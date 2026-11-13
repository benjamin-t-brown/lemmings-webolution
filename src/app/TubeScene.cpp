#include "TubeScene.h"
#include "app/ActorVisibility.h"

#include "app/ActorSprite.h"
#include "app/LemmingAnim.h"
#include "app/LemmingMotion.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/LightMap.h"
#include "assets/LevelIndex.h"
#include "assets/PolyList.h"
#include "assets/SurfaceMesh.h"
#include "gfx/SpriteView.h"
#include "gfx/TilePosition.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <numbers>
#include <set>
#include <utility>

namespace webo::app {
namespace {

Texture toTexture(const Image& image) {
  Texture t;
  t.width = image.width;
  t.height = image.height;
  t.rgb.resize(static_cast<std::size_t>(image.width) * image.height * 3);
  for (std::size_t i = 0, n = static_cast<std::size_t>(image.width) * image.height; i < n; ++i) {
    t.rgb[i * 3] = image.rgba[i * 4];
    t.rgb[i * 3 + 1] = image.rgba[i * 4 + 1];
    t.rgb[i * 3 + 2] = image.rgba[i * 4 + 2];
  }
  return t;
}

bool project(const CameraBasis& c, const Vec3& v, std::array<double, 4>& out) {
  const auto& m = c.worldToView;
  const double px = m[0] * v.x + m[3] * v.y + m[6] * v.z + m[9];
  const double py = m[1] * v.x + m[4] * v.y + m[7] * v.z + m[10];
  const double pz = m[2] * v.x + m[5] * v.y + m[8] * v.z + m[11];
  if (pz <= 0.0 || !std::isfinite(pz))
    return false;
  const double inv = 1.0 / pz;
  out = {c.cx + c.focalX * px * inv, c.cy - c.focalY * py * inv, pz, inv};
  return true;
}

}  // namespace

std::expected<TubeScene, std::string> TubeScene::build(const AssetLibrary& lib,
                                                       const CylinderMesh& mesh,
                                                       std::span<const std::byte> levelBytes,
                                                       std::span<const std::uint32_t> grid0,
                                                       bool highDetailTiles) {
  if (mesh.vertices.empty() || mesh.ringCount < 2 || mesh.angularSteps == 0)
    return std::unexpected("scene: the cylinder mesh has no usable lattice");

  TubeScene scene;
  scene.mesh_ = mesh;
  if (auto grid = findCollisionGrid(levelBytes)) {
    scene.collision_ = std::move(*grid);
    scene.haveCollision_ = true;
    if (scene.collision_.cylinderExtent > 0.0)
      scene.cylinderExtent_ = scene.collision_.cylinderExtent;
  }

  const auto loadBlock = [&](const std::vector<std::string>& names, bool outer,
                             std::vector<Tile>& into, std::size_t& count) {
    for (const auto& name : names) {
      if (name.empty())
        continue;
      const gfx::TilePosition pos = gfx::parseTilePosition(name);
      if (!pos.valid || pos.outer != outer) {
        scene.missing_.push_back(std::format("{}: cannot read a position", name));
        continue;
      }
      auto assetName = highDetailTiles ? levelTextureAssetNameHi(name)
                                      : levelTextureAssetName(name + "Lo");
      // Constructor 0x0047218A restores the unsuffixed name after a failed
      // quality probe. Archive presence covers only the absent-entry case;
      // errors loading an existing HI/Lo entry must remain errors.
      if (!lib.contains(assetName))
        assetName = levelTextureAssetName(name);
      auto image = loadTexture(lib, assetName);
      if (!image) {
        scene.missing_.push_back(std::format("{}: {}", name, image.error()));
        continue;
      }
      into.push_back(Tile{toTexture(*image), pos.column, pos.row});
      ++count;
    }
  };
  loadBlock(mesh.tileTextures, true, scene.outer_, scene.outerTileCount_);
  loadBlock(mesh.innerTileTextures, false, scene.inner_, scene.innerTileCount_);

  // The per-cell mesh the engine builds at level load. A failure is not a scene
  // failure: it means this level's stored key table does not answer the rule.
  if (scene.haveCollision_ && !grid0.empty()) {
    if (auto built = buildCellMeshes(grid0, {}, scene.collision_, mesh.surface.size())) {
      scene.cellMesh_ = std::move(*built);
    } else {
      scene.missing_.push_back(std::format("cell mesh: {}", built.error()));
    }
  }

  // Optional cut-in geometry from the stored lattice. A failure here is not a
  // scene failure: it means this level has no reconstructible lattice faces.
  if (!grid0.empty()) {
    if (auto built = buildSurfaceMesh(mesh, grid0, {}); built && !built->quads.empty()) {
      for (const auto& q : built->quads) {
        SurfaceQuad quad;
        for (int i = 0; i < 4; ++i)
          quad.v[i] = q.v[i];
        double radius = 0;
        for (int i = 0; i < 4; ++i)
          radius += std::hypot(q.v[i].x, q.v[i].z);
        quad.outerFacing = radius / 4.0 >= mesh.innerRadius() + mesh.thickness * 0.5;
        scene.surface_.push_back(quad);
      }
      scene.surfaceQuads_ = scene.surface_.size();
    }
  }

  // Optional cut-in geometry from the stored PolyList 09 records. These carry
  // their own index into CylinderMesh::surface, a UV and an RGBA colour. They
  // are render attributes, not the runtime per-cell mesh (see the round note).
  if (!levelBytes.empty()) {
    if (auto lists = findPolyLists(levelBytes)) {
      for (const auto& block : lists->blocks) {
        for (const auto& record : block.records) {
          if (record.cornerCount < 3)
            continue;
          Vec3 corners[4];
          bool ok = true;
          for (std::uint32_t c = 0; c < record.cornerCount; ++c) {
            const auto& corner = record.corners[c];
            if (corner.index >= mesh.surface.size()) {
              ok = false;
              break;
            }
            corners[c] = mesh.surface[corner.index];
          }
          if (!ok)
            continue;
          const auto& first = record.corners[0];
          const std::array<double, 3> modulate = {
              std::clamp(first.r / 255.0, 0.0, 1.0),
              std::clamp(first.g / 255.0, 0.0, 1.0),
              std::clamp(first.b / 255.0, 0.0, 1.0)};
          const bool outerFacing =
              std::hypot(corners[0].x, corners[0].z) >= mesh.innerRadius() + mesh.thickness * 0.5;
          const std::array<std::array<int, 3>, 2> tris = {std::array<int, 3>{0, 1, 2},
                                                          std::array<int, 3>{0, 2, 3}};
          for (const auto& tri : tris) {
            if (record.cornerCount == 3 && tri[2] == 3)
              continue;
            PolyFace face;
            face.modulate = modulate;
            face.outerFacing = outerFacing;
            for (int i = 0; i < 3; ++i)
              face.v[i] = corners[tri[i]];
            scene.poly_.push_back(face);
          }
          ++scene.polyListRecords_;
        }
      }
    }
  }

  if (scene.haveCollision_ && !grid0.empty()) {
    scene.poseGrid0_.assign(grid0.begin(), grid0.end());
    scene.poseSurface_ = scene.mesh_.surface;
    scene.poseCylinderTemplate_ = scene.mesh_;
    scene.poseLevel_.gridHeight = scene.collision_.height;
    scene.poseLevel_.grid0 = scene.poseGrid0_;
    scene.poseLevel_.surface = scene.poseSurface_;
    scene.poseLevel_.cylinderTemplate = &scene.poseCylinderTemplate_;
    scene.havePoseLevel_ = true;
  }
  if (auto light = findActorLightMap(levelBytes))
    scene.actorLight_ = std::move(*light);
  if (auto sprites = loadSpriteSheet(lib, "SPRITES\\LEMMINGSHI.SPR")) {
    scene.lemmingSprites_ = std::move(*sprites);
    scene.haveLemmingSprites_ = true;
    if (scene.lemmingSprites_.find("walk") == nullptr)
      scene.missing_.push_back("compose: walk action missing from LEMMINGSHI.SPR");
  } else {
    scene.missing_.push_back(std::format("compose: {}", sprites.error()));
  }

  scene.rewirePoseLevel();
  return scene;
}

void TubeScene::rewirePoseLevel() const {
  if (!havePoseLevel_)
    return;
  auto* self = const_cast<TubeScene*>(this);
  self->poseLevel_.grid0 = poseGrid0_;
  self->poseLevel_.surface = poseSurface_;
  self->poseLevel_.cylinderTemplate = &poseCylinderTemplate_;
}

const TubeScene::Tile* TubeScene::findTile(bool outer, int column, int row) const {
  const auto& block = outer ? outer_ : inner_;
  for (const auto& t : block)
    if (t.column == column && t.row == row)
      return &t;
  return nullptr;
}

namespace {

void compositeActorOverlay(app::Raster& raster, const Image& overlay,
                           bool legacyNonZeroAlpha) {
  const app::Framebuffer& srcFb = raster.framebuffer();
  auto& rgb = const_cast<std::vector<std::uint8_t>&>(srcFb.rgb);
  const int width = srcFb.width;
  const int height = srcFb.height;
  if (overlay.width != width || overlay.height != height ||
      overlay.rgba.size() != static_cast<std::size_t>(width) * height * 4)
    return;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const std::size_t i = (static_cast<std::size_t>(y) * width + x) * 4;
      const std::uint8_t a = overlay.rgba[i + 3];
      if (legacyNonZeroAlpha) {
        if (a == 0)
          continue;
      } else if (!webo::actorSpriteAlphaTestPasses(a)) {
        continue;
      }
      const std::size_t o = (static_cast<std::size_t>(y) * width + x) * 3;
      rgb[o] = overlay.rgba[i];
      rgb[o + 1] = overlay.rgba[i + 1];
      rgb[o + 2] = overlay.rgba[i + 2];
    }
  }
}

// Pose, view selection, sheet load, placement and tint for one injected
// walker: the steps `drawInjectedWalkers` already ran immediately before
// `drawActorSprite`. nullopt is one of that loop's `continue`s (temporal
// index, empty direction list, forced view out of range, sheet-load failure,
// placement failure). P8 additionally draws far-facing sprites into scratch
// storage and returns nullopt if the cylinder masks every opaque texel. Callers
// own iteration order; partially visible actors keep their original pick box.
struct PreparedInjectedWalker {
  webo::SpriteFrame frame;
  webo::Image texture;
  ActorSpritePlacement placement{};
  bool mirrored = false;
  std::array<std::uint8_t, 3> tint{255, 255, 255};
  Image clippedSprite;
};

// P8, provisional playable-v1 policy, not recovered D3D device state. A6c/A6d16
// leave effective actor depth unresolved. Treat the cylinder's outer envelope
// as opaque to FAR-facing actors, clipping their billboard pixels at its actual
// polygonal silhouette. Near-facing actors keep the accepted overlay path: their
// origin lies within the shell (Z=.75), including inside excavated tunnels, so a
// blanket depth test against the outer skin would incorrectly bury them.
// This is deliberately independent of terrain edits, camera and simulation.
class ActorCylinderOccluder {
 public:
  ActorCylinderOccluder(const CylinderMesh& mesh, const CameraBasis& camera) : camera_(camera) {
    if (mesh.ringCount < 2 || mesh.angularSteps < 3) return;
    for (std::uint32_t i = 0; i < mesh.angularSteps; ++i) {
      const auto& a = mesh.vertex(0, i, true);
      const auto& b = mesh.vertex(0, (i + 1) % mesh.angularSteps, true);
      double nx = b.z - a.z, nz = a.x - b.x;
      double limit = nx * a.x + nz * a.z;
      if (limit < 0) { nx = -nx; nz = -nz; limit = -limit; }
      planes_.push_back({nx, 0, nz, limit});
    }
    const double y0 = mesh.vertex(0, 0, true).y;
    const double y1 = mesh.vertex(mesh.ringCount - 1, 0, true).y;
    planes_.push_back({0, -1, 0, -std::min(y0, y1)});
    planes_.push_back({0, 1, 0, std::max(y0, y1)});
  }

  bool farFacing(const std::array<double, 3>& p) const {
    return !planes_.empty() &&
        p[0] * (camera_.eyeWorld[0] - p[0]) +
        p[2] * (camera_.eyeWorld[2] - p[2]) < 0;
  }

  bool occludes(int x, int y, double lensedDepth) const {
    const auto& m = camera_.worldToView;
    const auto& eye = camera_.eyeWorld;
    const double vx = (x - camera_.cx) / camera_.focalX;
    const double vy = (camera_.cy - y) / camera_.focalY;
    // Inverse rigid view basis. Ray parameter is physical view Z; actor depth
    // already carries the accepted .0003 bias in normalized (0.3/width) units.
    const std::array<double, 3> ray{
        m[0] * vx + m[1] * vy + m[2],
        m[3] * vx + m[4] * vy + m[5],
        m[6] * vx + m[7] * vy + m[8]};
    double enter = 0, exit = lensedDepth * camera_.width / 0.3;
    for (const auto& p : planes_) {
      const double speed = p[0] * ray[0] + p[1] * ray[1] + p[2] * ray[2];
      const double room = p[3] - p[0] * eye[0] - p[1] * eye[1] - p[2] * eye[2];
      if (std::abs(speed) < 1e-12) {
        if (room < 0) return false;
      } else if (speed < 0) {
        enter = std::max(enter, room / speed);
      } else {
        exit = std::min(exit, room / speed);
      }
      if (enter >= exit) return false;
    }
    return enter < exit;
  }

 private:
  const CameraBasis& camera_;
  std::vector<std::array<double, 4>> planes_;
};

bool clipFarActor(Image& sprite, const ActorSpritePlacement& p,
                  const ActorCylinderOccluder& cylinder) {
  bool visible = false;
  for (int y = std::max(0, int(std::ceil(p.top)));
       y < std::min(sprite.height, int(std::ceil(p.bottom))); ++y) {
    for (int x = std::max(0, int(std::ceil(p.left)));
         x < std::min(sprite.width, int(std::ceil(p.right))); ++x) {
      auto& alpha = sprite.rgba[(std::size_t(y) * sprite.width + x) * 4 + 3];
      if (!actorSpriteAlphaTestPasses(alpha)) continue;
      if (cylinder.occludes(x, y, p.depth)) alpha = 0;
      else visible = true;
    }
  }
  return visible;
}

std::optional<PreparedInjectedWalker> prepareInjectedWalker(
    const ActorPoseLevel& poseLevel, const SpriteSheet& sprites, const ActorLightMap& light,
    const CameraBasis& camera, const SceneOptions& options,
    const SceneOptions::InjectedNativeWalker& injected, const AssetLibrary& lib,
    const TubePassBounds& bounds, const ActorCylinderOccluder& cylinder) {
  if (!options.composeSkipActorVisibility &&
      !actorSubmissionVisible(injected.plus8X, injected.plus8Z, injected.renderEnabled, bounds))
    return std::nullopt;
  // actor-anim-001: the walker's own clip; the walk clip when the sheet lacks it.
  const Sprite* walkSprite = injected.clip != nullptr ? sprites.find(injected.clip) : nullptr;
  if (walkSprite == nullptr)
    walkSprite = sprites.find("walk");
  if (walkSprite == nullptr)
    return std::nullopt;

  ActorTerrainSample plus8{injected.plus8X, injected.plus8Y, injected.plus8Z};
  ActorPoseResult pose;
  if (options.composeSkipActorPose) {
    pose.modelPosition = {plus8.x, plus8.y, plus8.z};
    pose.basis = {1.0, 0.0, 0.0};
  } else {
    pose = actorPoseFromNativeSample(poseLevel, plus8, injected.facing);
  }

  const gfx::SpriteViewVector cameraToActor{
      pose.modelPosition[0] - camera.eyeWorld[0],
      pose.modelPosition[1] - camera.eyeWorld[1],
      pose.modelPosition[2] - camera.eyeWorld[2],
  };
  const gfx::SpriteViewVector basisVec{pose.basis[0], pose.basis[1], pose.basis[2]};
  const std::size_t temporalIndex =
      static_cast<std::size_t>(std::max(0, injected.walkTemporalIndex));
  if (temporalIndex >= walkSprite->animations.size())
    return std::nullopt;
  const auto& temporal = walkSprite->animations[temporalIndex];
  const std::size_t dirCount = temporal.frames.size();
  if (dirCount == 0)
    return std::nullopt;

  const auto scores = gfx::spriteViewScores(cameraToActor, basisVec, options.composeActorPhase);
  auto selection = gfx::selectSpriteView(scores[0], scores[1], dirCount, false);
  if (injected.forcedViewIndex >= 0)
    selection.index = static_cast<std::size_t>(injected.forcedViewIndex);
  if (injected.forcedMirror >= 0)
    selection.mirrored = injected.forcedMirror != 0;
  if (selection.index >= dirCount)
    return std::nullopt;

  const auto& frame = temporal.frames[selection.index];
  auto texture = loadTexture(lib, sheetAssetName(frame.sheet));
  if (!texture)
    return std::nullopt;

  const std::array<double, 3> worldPosition{pose.modelPosition[0], pose.modelPosition[1],
                                            pose.modelPosition[2]};
  auto placement = app::actorSpritePlacement(frame, worldPosition, camera, selection.mirrored, -1);
  if (!placement)
    return std::nullopt;

  std::array<std::uint8_t, 3> tint{255, 255, 255};
  if (!light.empty()) {
    if (auto looked = actorLightTintAt(light, worldPosition[0], worldPosition[1]))
      tint = *looked;
  }

  PreparedInjectedWalker prepared;
  prepared.frame = frame;
  prepared.texture = std::move(*texture);
  prepared.placement = *placement;
  prepared.mirrored = selection.mirrored;
  prepared.tint = tint;
  // Keep the explicit visibility diagnostic able to reveal rejected actors,
  // including actors hidden by the P8 mask. Production never sets this flag.
  if (!options.composeSkipActorVisibility && !options.composeSkipActorPose &&
      cylinder.farFacing(worldPosition)) {
    auto& sprite = prepared.clippedSprite;
    sprite.width = camera.width;
    sprite.height = camera.height;
    sprite.alpha = AlphaMode::Embedded;
    sprite.rgba.assign(std::size_t(sprite.width) * sprite.height * 4, 0);
    if (!drawActorSprite(sprite, frame, prepared.texture, *placement, selection.mirrored, tint) ||
        !clipFarActor(sprite, *placement, cylinder)) return std::nullopt;
  }
  return prepared;
}

}  // namespace

void TubeScene::drawInjectedWalkers(Raster& raster, const CameraBasis& camera,
                                    const SceneOptions& options,
                                    const AssetLibrary& lib) const {
  if (options.injectedWalkers.empty() || !haveLemmingSprites_ || !havePoseLevel_)
    return;
  rewirePoseLevel();
  const Sprite* walkSprite = lemmingSprites_.find("walk");
  if (walkSprite == nullptr)
    return;

  Image overlay;
  overlay.width = raster.framebuffer().width;
  overlay.height = raster.framebuffer().height;
  overlay.alpha = AlphaMode::Embedded;
  overlay.rgba.assign(static_cast<std::size_t>(overlay.width) * overlay.height * 4, 0);

  const auto bounds = tubePassBounds(camera.selectorTurns, cylinderExtent_);
  const ActorCylinderOccluder cylinder(mesh_, camera);
  for (auto it = options.injectedWalkers.rbegin(); it != options.injectedWalkers.rend(); ++it) {
    // Reverse order, same skips, and the same frame/texture/placement/mirror/tint
    // fed to drawActorSprite. alphaTest stays true; only far-facing sprites
    // arrive already rasterised and clipped in scratch storage.
    auto prepared = prepareInjectedWalker(poseLevel_, lemmingSprites_, actorLight_, camera,
                                          options, *it, lib, bounds, cylinder);
    if (!prepared)
      continue;
    if (!prepared->clippedSprite.rgba.empty()) {
      const auto& sprite = prepared->clippedSprite;
      const auto& p = prepared->placement;
      for (int y = std::max(0, int(std::ceil(p.top)));
           y < std::min({overlay.height, sprite.height, int(std::ceil(p.bottom))}); ++y)
        for (int x = std::max(0, int(std::ceil(p.left)));
             x < std::min({overlay.width, sprite.width, int(std::ceil(p.right))}); ++x) {
          const auto at = (std::size_t(y) * overlay.width + x) * 4;
          const auto source = (std::size_t(y) * sprite.width + x) * 4;
          if (actorSpriteAlphaTestPasses(sprite.rgba[source + 3]))
            std::copy_n(sprite.rgba.begin() + source, 4, overlay.rgba.begin() + at);
        }
      continue;
    }
    if (!app::drawActorSprite(overlay, prepared->frame, prepared->texture, prepared->placement,
                              prepared->mirrored, prepared->tint, true)
             .has_value())
      continue;
  }

  compositeActorOverlay(raster, overlay, options.overlayCompositeLegacyNonZero);
}

std::vector<std::optional<ActorSpritePlacement>> TubeScene::injectedWalkerScreenBoxes(
    const CameraBasis& camera, const SceneOptions& options, const AssetLibrary& lib) const {
  std::vector<std::optional<ActorSpritePlacement>> boxes(options.injectedWalkers.size());
  if (options.injectedWalkers.empty() || !haveLemmingSprites_ || !havePoseLevel_)
    return boxes;
  rewirePoseLevel();
  if (lemmingSprites_.find("walk") == nullptr)
    return boxes;
  const auto bounds = tubePassBounds(camera.selectorTurns, cylinderExtent_);
  const ActorCylinderOccluder cylinder(mesh_, camera);
  for (std::size_t i = 0; i < options.injectedWalkers.size(); ++i) {
    if (auto prepared = prepareInjectedWalker(poseLevel_, lemmingSprites_, actorLight_, camera,
                                              options, options.injectedWalkers[i], lib, bounds, cylinder))
      boxes[i] = prepared->placement;
  }
  return boxes;
}

void TubeScene::render(Raster& raster, const CameraBasis& camera,
                       const SceneOptions& options, const AssetLibrary* composeAssets) const {
  raster.setDepthEnabled(options.depthTest);
  const int steps = static_cast<int>(mesh_.angularSteps);
  const int bands = static_cast<int>(mesh_.bandCount());
  const int cols = static_cast<int>(CylinderMesh::kTileColumns);
  const int rows = static_cast<int>(CylinderMesh::kTileRows);
  const int stepsPerCol = std::max(1, steps / cols);
  const int bandsPerRow = std::max(1, bands / rows);

  if (options.flat) {
    // Debug unwrap: one screen-space cell per angular step and ring band, with
    // the outer tile's own UV quadrant. No camera, no depth.
    const double cellW = static_cast<double>(camera.width) / steps;
    const double cellH = static_cast<double>(camera.height) / bands;
    for (int band = 0; band < bands; ++band) {
      const int row = (bands - 1 - band) / bandsPerRow;
      const int withinRow = (bands - 1 - band) % bandsPerRow;
      for (int step = 0; step < steps; ++step) {
        const Tile* tile = findTile(true, step / stepsPerCol, row);
        if (tile == nullptr)
          continue;
        const int withinCol = step % stepsPerCol;
        const double x0 = step * cellW, y0 = (bands - 1 - band) * cellH;
        const double u0 = static_cast<double>(withinCol) / stepsPerCol;
        const double u1 = static_cast<double>(withinCol + 1) / stepsPerCol;
        const double v0 = static_cast<double>(withinRow) / bandsPerRow;
        const double v1 = static_cast<double>(withinRow + 1) / bandsPerRow;
        std::array<std::array<double, 4>, 3> p = {
            {{x0, y0, 0, 1}, {x0, y0 + cellH, 0, 1}, {x0 + cellW, y0 + cellH, 0, 1}}};
        std::array<std::array<double, 2>, 3> uv = {{{u0, v0}, {u0, v1}, {u1, v1}}};
        raster.triangle(p, uv, tile->texture, {1.0, 1.0, 1.0});
        std::array<std::array<double, 4>, 3> p2 = {
            {{x0, y0, 0, 1}, {x0 + cellW, y0 + cellH, 0, 1}, {x0 + cellW, y0, 0, 1}}};
        std::array<std::array<double, 2>, 3> uv2 = {{{u0, v0}, {u1, v1}, {u1, v0}}};
        raster.triangle(p2, uv2, tile->texture, {1.0, 1.0, 1.0});
      }
    }
    return;
  }

  const auto drawQuad = [&](const Vec3 corners[4], const Tile* tile, double context,
                            const std::array<std::array<double, 2>, 4>& uv) {
    if (tile == nullptr)
      return;
    std::array<std::array<double, 4>, 4> projected{};
    for (int i = 0; i < 4; ++i) {
      if (!project(camera, corners[i], projected[i]))
        return;
      projected[i][2] -= context;
    }
    const std::array<double, 3> modulate = {1.0, 1.0, 1.0};
    for (const auto& ids : {std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3}}) {
      std::array<std::array<double, 4>, 3> p{};
      std::array<std::array<double, 2>, 3> t{};
      for (int i = 0; i < 3; ++i) {
        p[i] = projected[ids[i]];
        t[i] = uv[ids[i]];
      }
      raster.triangle(p, t, tile->texture, modulate);
    }
  };

  const auto drawSurface = [&]() {
    if (surface_.empty())
      return;
    for (const auto& quad : surface_) {
      std::array<std::array<double, 4>, 4> projected{};
      bool visible = true;
      for (int i = 0; i < 4 && visible; ++i)
        visible = project(camera, quad.v[i], projected[i]);
      if (!visible)
        continue;
      // Tile chosen from the first corner's angular step and height.
      const double angle = std::atan2(-quad.v[0].x, quad.v[0].z) / (2.0 * std::numbers::pi);
      int step = static_cast<int>(std::floor(angle * steps));
      if (step < 0) step += steps;
      step %= steps;
      int band = static_cast<int>(std::floor(quad.v[0].y / std::max(1e-9, mesh_.ringSpacing())));
      band = std::clamp(band, 0, bands - 1);
      const Tile* tile = findTile(quad.outerFacing, step / stepsPerCol, band / bandsPerRow);
      std::array<std::array<double, 2>, 4> uv = {{{0, 0}, {0, 1}, {1, 1}, {1, 0}}};
      const std::array<double, 3> mod = {1.0, 1.0, 1.0};
      for (const auto& ids : {std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3}}) {
        std::array<std::array<double, 4>, 3> p{};
        std::array<std::array<double, 2>, 3> t{};
        for (int i = 0; i < 3; ++i) {
          p[i] = projected[ids[i]];
          t[i] = uv[ids[i]];
        }
        raster.triangle(p, t, tile ? tile->texture : Texture{}, mod);
      }
    }
  };

  const auto drawWall = [&](bool outer) {
    for (int band = 0; band < bands; ++band) {
      // Vertical tile row, with the A2 flip: the source-derived row is
      // (bands-1-band)/bandsPerRow, which reverses the tile rows against the
      // band order. Within a row the two bands split the tile's V in half,
      // reversed the same way so the art stays continuous up the tube.
      const int row = options.tileRowsFlipped ? (bands - 1 - band) / bandsPerRow
                                              : band / bandsPerRow;
      const int withinRow = options.tileRowsFlipped ? (bands - 1 - band) % bandsPerRow
                                                    : band % bandsPerRow;
      for (int step = 0; step < steps; ++step) {
        const int column = step / stepsPerCol;
        if (!options.fullWall && haveCollision_) {
          // File-side presence rule measured in the round note: a runtime
          // outer-wall cell is drawn exactly when at least one collision cell
          // of its (step,band) box is solid. The collision grid is 256 x 96,
          // so one angular step is 8 columns and one ring band is 16 rows.
          const int columnsPerStep = CollisionGrid::kWidth / steps;
          const int rowsPerBand = collision_.height > 0 ? collision_.height / bands : 0;
          bool solid = false;
          for (int dx = 0; dx < columnsPerStep && !solid; ++dx)
            for (int dy = 0; dy < rowsPerBand && !solid; ++dy)
              if (collision_.solid(step * columnsPerStep + dx, band * rowsPerBand + dy))
                solid = true;
          if (!solid)
            continue;
        }
        const Tile* tile = findTile(outer, column, row);
        if (tile == nullptr)
          continue;
        const int withinCol = step % stepsPerCol;
        const double u0 = static_cast<double>(withinCol) / stepsPerCol;
        const double u1 = static_cast<double>(withinCol + 1) / stepsPerCol;
        const double v0 = static_cast<double>(withinRow) / bandsPerRow;
        const double v1 = static_cast<double>(withinRow + 1) / bandsPerRow;
        // Corners: (band,step) (band+1,step) (band+1,step+1) (band,step+1).
        const std::array<std::array<double, 2>, 4> uv = {
            {{u0, v0}, {u0, v1}, {u1, v1}, {u1, v0}}};
        const int next = (step + 1) % steps;
        Vec3 corners[4] = {
            mesh_.vertex(band, step, outer),
            mesh_.vertex(band + 1, step, outer),
            mesh_.vertex(band + 1, next, outer),
            mesh_.vertex(band, next, outer),
        };
        drawQuad(corners, tile, outer ? options.outerContext : options.innerContext, uv);
      }
    }
  };

  if (options.drawWall) {
    if (options.innerFirst && options.drawInner)
      drawWall(false);
    if (options.drawOuter)
      drawWall(true);
    if (!options.innerFirst && options.drawInner)
      drawWall(false);
  }

  if (options.drawSurface)
    drawSurface();

  if (options.drawCellMesh) {
    // The runtime tube topology: for every 8x16 cell, the solid rectangle's
    // outer patch plus the wall-thickness ribbons on its open sides. UVs are
    // tile-local (u over 32 columns, v over 32 rows), so the texture is the
    // outer tile at (material % 8, material / 8).
    for (const auto& cell : cellMesh_.cells) {
      const Tile* tile = findTile(true, static_cast<int>(cell.material % 8),
                                  static_cast<int>(cell.material / 8));
      if (tile == nullptr)
        continue;
      const auto drawBank = [&](const std::vector<CellMeshQuad>& bank) {
        for (const auto& quad : bank) {
          std::array<std::array<double, 4>, 4> projected{};
          bool ok = true;
          for (int i = 0; i < 4 && ok; ++i) {
            const std::uint32_t index = quad.index[i];
            if (index >= mesh_.surface.size())
              ok = false;
            else
              ok = project(camera, mesh_.surface[index], projected[i]);
          }
          if (!ok)
            continue;
          const double c = std::clamp(quad.colour / 255.0, 0.0, 1.0);
          const std::array<double, 3> modulate = {c, c, c};
          for (const auto& ids : {std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3}}) {
            std::array<std::array<double, 4>, 3> p{};
            std::array<std::array<double, 2>, 3> t{};
            for (int i = 0; i < 3; ++i) {
              p[i] = projected[ids[i]];
              t[i] = quad.uv[ids[i]];
            }
            raster.triangle(p, t, tile->texture, modulate);
          }
        }
      };
      drawBank(cell.mesh0);
      drawBank(cell.mesh4);
    }
  }

  const auto maybeCompose = [&]() {
    if (!options.drawCellRoute || options.injectedWalkers.empty() || composeAssets == nullptr)
      return;
    drawInjectedWalkers(raster, camera, options, *composeAssets);
  };

  if (options.drawCellRoute && options.composeActorsBeforeRoute)
    maybeCompose();

  if (options.drawCellRoute && cellMesh_.buckets > 0 && cellMesh_.bands > 0) {
    // The original's tube submission, `0x43A4EC`. Everything here is a port of
    // that walker and its four callees; see the round note for the addresses.
    //
    // The pass bounds. `0x43A4EC` composes the camera's own turn fraction into
    // a bucket centre (`0x43c990 -> fsub 0.5, fmul 32.0`), then brackets it by
    // the stored extent and 0.7 of it, chopping each bound toward zero with the
    // engine's float-to-int helper `0x49C5AC`, then shifting the whole set into
    // [0, 32] in 32s.
    const auto [centre, a, b, mid, c, d] = tubePassBounds(camera.selectorTurns, cylinderExtent_);
    // `0x462465` compares each quad's child coordinate against the centre
    // bucket in eighths, i.e. the camera's own collision column.
    const double centreColumn = centre * 8.0;

    // The bank texture: A2's addressing (`0x460A2D`, `0x43AB3A`). The material
    // index is the cell's stored descriptor; +24 selects the inner bank, and
    // the tile position is (material % 8, material / 8) within that bank.
    const auto tileFor = [&](std::uint32_t materialIndex) -> const Tile* {
      const std::uint32_t tile = materialIndex % 24;
      const bool outer = (materialIndex < 24) != options.routeTexBankSwap;
      return findTile(outer, static_cast<int>(tile % 8), static_cast<int>(tile / 8));
    };

    const auto drawRouteQuad = [&](const CellMeshQuad& quad, std::uint32_t materialIndex,
                                   double context) {
      const Tile* tile = tileFor(materialIndex);
      if (tile == nullptr)
        return;
      std::array<std::array<double, 4>, 4> projected{};
      for (int i = 0; i < 4; ++i) {
        const std::uint32_t index = quad.index[i];
        if (index >= mesh_.surface.size())
          return;
        if (!project(camera, mesh_.surface[index], projected[i]))
          return;
        projected[i][2] -= context;
      }
      const double cc = std::clamp(quad.colour / 255.0, 0.0, 1.0);
      const std::array<double, 3> modulate = {cc, cc, cc};
      for (const auto& ids : {std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3}}) {
        std::array<std::array<double, 4>, 3> p{};
        std::array<std::array<double, 2>, 3> t{};
        for (int i = 0; i < 3; ++i) {
          p[i] = projected[ids[i]];
          t[i] = quad.uv[ids[i]];
        }
        raster.triangle(p, t, tile->texture, modulate);
      }
    };

    const int bands = static_cast<int>(cellMesh_.bands);
    const bool swap = options.routeSwapBanks;
    const auto cellAt = [&](int bucket, int band) {
      return cellMesh_.at(bucket & 31, band);
    };
    // The side routes draw their bank from the last element back to the first
    // (`0x4623A5` / `0x462404`); the centre route walks both banks forward.
    const auto drawBankReversed = [&](const CellMeshCell& cell, bool useMesh4) {
      const auto& bank = useMesh4 ? cell.mesh4 : cell.mesh0;
      for (auto it = bank.rbegin(); it != bank.rend(); ++it)
        drawRouteQuad(*it, cell.material, options.outerContext);
    };

    // 1. INN (`0x4622FD`): buckets c..b descending, all bands, the entry+8
    //    whole-block quad, material + 24.
    if (!options.routeNoInner) {
      for (int i = c; i >= b; --i)
        for (int j = 0; j < bands; ++j) {
          const CellMeshCell* cell = cellAt(i, j);
          if (cell != nullptr && cell->hasInner)
            drawRouteQuad(cell->inner, cell->material + 24, options.innerContext);
        }
    }

    // 2. OUT right (`0x462404`): buckets d-1..mid+1 descending, mesh4.
    for (int i = d - 1; i > mid; --i)
      for (int j = 0; j < bands; ++j) {
        const CellMeshCell* cell = cellAt(i, j);
        if (cell != nullptr)
          drawBankReversed(*cell, !swap);
      }

    // 3. OUT left (`0x4623A5`): buckets a..mid-1, mesh0.
    for (int i = a; i < mid; ++i)
      for (int j = 0; j < bands; ++j) {
        const CellMeshCell* cell = cellAt(i, j);
        if (cell != nullptr)
          drawBankReversed(*cell, swap);
      }

    // 4. OUT centre (`0x462465`): the centre bucket, mesh0 forward where the
    //    child coordinate is below the camera column, then mesh4 forward where
    //    it is at or above it. Each record is drawn by exactly one of the two.
    for (int j = 0; j < bands; ++j) {
      const CellMeshCell* cell = cellAt(mid, j);
      if (cell == nullptr)
        continue;
      const auto& first = swap ? cell->mesh4 : cell->mesh0;
      const auto& second = swap ? cell->mesh0 : cell->mesh4;
      for (const auto& quad : first)
        if (options.routeFlipCentre ? (quad.coord >= centreColumn)
                                    : (quad.coord < centreColumn))
          drawRouteQuad(quad, cell->material, options.outerContext);
      for (const auto& quad : second)
        if (options.routeFlipCentre ? (quad.coord < centreColumn)
                                    : (quad.coord >= centreColumn))
          drawRouteQuad(quad, cell->material, options.outerContext);
    }
  }

  if (options.drawCellRoute && !options.composeActorsBeforeRoute)
    maybeCompose();

  if (options.drawPolyList) {
    for (const auto& face : poly_) {
      std::array<std::array<double, 4>, 3> p{};
      bool ok = true;
      for (int i = 0; i < 3 && ok; ++i)
        ok = project(camera, face.v[i], p[i]);
      if (!ok)
        continue;
      const double angle = std::atan2(-face.v[0].x, face.v[0].z) / (2.0 * std::numbers::pi);
      int step = static_cast<int>(std::floor(angle * steps));
      if (step < 0) step += steps;
      step %= steps;
      int band = static_cast<int>(std::floor(face.v[0].y / std::max(1e-9, mesh_.ringSpacing())));
      band = std::clamp(band, 0, bands - 1);
      const Tile* tile = findTile(face.outerFacing, step / stepsPerCol, band / bandsPerRow);
      if (tile == nullptr)
        continue;
      std::array<std::array<double, 2>, 3> t = {{{0, 0}, {0.5, 1}, {1, 0}}};
      raster.triangle(p, t, tile->texture, face.modulate);
    }
  }
}

// terrain-render-001. METHOD, with its evidence level:
//   READ (0x43A31D, 0x43D2B8, static): a vertex the key table lacks is created on demand and
//     filled by bilinear interpolation of the cylinder template (`cylinderTemplateVertex`).
//   READ (docs: kCellFlag0x400, CollisionGrid.h): the original already rebuilds the per-cell
//     mesh after a runtime collision change, by re-running the 0x4615E9 scan for the blocks
//     concerned; gates and doors do it. This function does the same for skill edits.
//   PROVISIONAL (not read in the binary): that the original's Basher/Miner/Digger/Bomber/
//     Builder cell writers (FUN_0043f510 is the cell-clear function; its mesh hand-off was not
//     traced) queue a rebuild of exactly the touched blocks padded by one cell, that a Builder
//     brick is drawn with its block's tile material, and the frame on which the original
//     redraws. What would settle it: an original capture of a bomber crater and a builder
//     staircase compared with the clone frame (not run here).
void TubeScene::syncTerrain(const sim::Terrain& terrain) {
  if (terrainFailed_ || !haveCollision_ || cellMesh_.cells.empty() || poseGrid0_.empty() ||
      terrain.empty() || terrain.width() != collision_.width ||
      terrain.height() != collision_.height)
    return;
  if (terrainSeen_ && terrain.version() == terrainVersion_)
    return;
  const int w = collision_.width;
  const int h = collision_.height;
  if (!terrainSeen_) {
    meshedSolid_.assign(static_cast<std::size_t>(w) * h, 0);
    for (int y = 0; y < h; ++y)
      for (int x = 0; x < w; ++x)
        meshedSolid_[static_cast<std::size_t>(y) * w + x] = collision_.solid(x, y) ? 1 : 0;
    terrainSeen_ = true;
  }
  terrainVersion_ = terrain.version();

  constexpr int kColumnsPerBlock = 8;
  constexpr int kRowsPerBlock = 16;
  std::set<std::pair<int, int>> dirty;
  std::size_t changed = 0;
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      const bool now = terrain.solid(x, h - 1 - y);
      std::uint8_t& was = meshedSolid_[static_cast<std::size_t>(y) * w + x];
      if (now == (was != 0))
        continue;
      was = now ? 1 : 0;
      ++changed;
      collision_.cells[static_cast<std::size_t>(y) * w + x] = now ? kCellSolid : 0u;
      // A block's gap lists read its neighbours, so the cells around the edit are queued too.
      for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx) {
          const int cx = (x + dx) & (w - 1);
          const int cy = std::clamp(y + dy, 0, h - 1);
          dirty.insert({cx / kColumnsPerBlock, cy / kRowsPerBlock});
        }
    }
  }
  if (changed == 0)
    return;

  if (editKeys_.empty())
    editKeys_ = poseGrid0_;
  // Every key a rebuilt block can ask for: its own corners (x0..x0+8, y0..y0+16) with a
  // margin for the neighbour reads. Keys the stored table holds are left alone.
  for (const auto& [bucket, band] : dirty) {
    const int x0 = bucket * kColumnsPerBlock;
    const int y0 = band * kRowsPerBlock;
    for (int layer = 0; layer < 2; ++layer)
      for (int y = std::max(0, y0 - 2); y <= std::min(h, y0 + kRowsPerBlock + 2); ++y)
        for (int x = std::max(0, x0 - 2); x <= std::min(w, x0 + kColumnsPerBlock + 2); ++x) {
          const std::size_t key = static_cast<std::size_t>(layer) + (x + y * 0x101) * 2;
          if (key >= editKeys_.size() || editKeys_[key] != 0xFFFFFFFFu)
            continue;
          const auto v = cylinderTemplateVertex(mesh_, x, y, layer, h);
          if (!v)
            continue;
          editKeys_[key] = static_cast<std::uint32_t>(mesh_.surface.size());
          mesh_.surface.push_back(*v);
          ++terrainVerticesAdded_;
        }
  }

  const std::vector<std::pair<int, int>> blocks(dirty.begin(), dirty.end());
  if (auto status = rebuildCellMeshCells(cellMesh_, editKeys_, {}, collision_,
                                         mesh_.surface.size(), blocks);
      !status) {
    terrainError_ = status.error();
    terrainFailed_ = true;
    return;
  }
  terrainUnresolved_ = cellMesh_.rebuildUnresolved;
  terrainCellsChanged_ += changed;
  terrainBlocksRebuilt_ += blocks.size();
}

std::vector<SceneOptions::InjectedNativeWalker> defaultMmClimb002InjectedWalkers() {
  return {
      {60.244850158691406, 32.0, 0.75, 1},
      {45.812477111816406, 32.0, 0.75, 1},
      {32.0, 34.47958755493164, 0.5, 1},
  };
}

std::vector<SceneOptions::InjectedNativeWalker> injectedWalkersFromWorld(
    const sim::World& world, int terrainHeight, const LemmingAnimTracker* anim) {
  // Keep the pre-P7 signature for frozen source demonstrations/headers too.
  return injectedWalkersFromWorld(world, terrainHeight, anim, nullptr);
}

std::vector<SceneOptions::InjectedNativeWalker> injectedWalkersFromWorld(
    const sim::World& world, int terrainHeight, const LemmingAnimTracker* anim,
    const LemmingMotionTracker* motion) {
  std::vector<SceneOptions::InjectedNativeWalker> out;
  out.reserve(world.lemmings().size());
  const auto& lemmings = world.lemmings();
  for (std::size_t i = 0; i < lemmings.size(); ++i) {
    const sim::Lemming& lemming = lemmings[i];
    if (!lemming.alive())
      continue;
    const ActorTerrainSample sample = terrainSampleFromLemming(lemming, terrainHeight);
    SceneOptions::InjectedNativeWalker walker;
    walker.plus8X = sample.x;
    walker.plus8Y = sample.y;
    walker.plus8Z = sample.z;
    if (motion) {
      const auto drawn = motion->positionOf(i, world);
      walker.plus8X = drawn.x;
      walker.plus8Y = terrainHeight - 1.0 - drawn.y;
    }
    walker.facing = static_cast<int>(lemming.facing);
    // Without a tracker every state draws walk entry 0 (the D3b behaviour); with one, the
    // lemming's own clip and temporal entry (actor-anim-001).
    if (anim != nullptr) {
      const LemmingAnimPose pose = anim->poseOf(i);
      walker.clip = pose.clip;
      walker.walkTemporalIndex = pose.temporalIndex;
    }
    out.push_back(walker);
  }
  return out;
}

}  // namespace webo::app
