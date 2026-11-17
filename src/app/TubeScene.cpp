#include "app/FrameProfile.h"
#include "TubeScene.h"
#include "app/ActorVisibility.h"

#include "app/ActorSprite.h"
#include "app/ActorOcclusion.h"
#include "app/BuilderBrick.h"
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
#include <numeric>
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
    // 0x43D56C clamps against world+0x28, the serialized coordinate-mesh
    // height (112 in the installed levels), NOT the collision mask's 96 rows.
    // `faceCount` is Cylinder's historical name for that first stored word.
    scene.poseLevel_.gridHeight = static_cast<int>(mesh.faceCount);
    scene.poseLevel_.grid0 = scene.poseGrid0_;
    scene.poseLevel_.surface = scene.poseSurface_;
    scene.poseLevel_.cylinderTemplate = &scene.poseCylinderTemplate_;
    scene.havePoseLevel_ = true;
  }
  if (auto light = findActorLightMap(levelBytes))
    scene.actorLight_ = std::move(*light);
  // READ: loader 4437F6..44389D registers these categories in this order;
  // constructor 42DF75..42DF89 binds 4E335C[sheetType] to actor+128.
  constexpr const char* names[]{"SPRITES\\LEMMINGSHI.SPR", "SPRITES\\LEMMINGSWATERHI.SPR",
                                "SPRITES\\LEMMINGSACIDHI.SPR"};
  for (int type = 0; type < 3; ++type) {
    if (auto sprites = loadSpriteSheet(lib, names[type])) {
      scene.lemmingSprites_[type] = std::move(*sprites);
      scene.haveLemmingSprites_[type] = true;
      if (scene.lemmingSprites_[type].find("walk") == nullptr)
        scene.missing_.push_back(std::format("compose: walk missing from {}", names[type]));
    } else {
      scene.missing_.push_back(std::format("compose: {}", sprites.error()));
    }
  }

  if (auto bubbles = loadSpriteSheet(lib, "SPRITES\\BUBBLES.SPR"))
    scene.bubbleSprites_ = std::move(*bubbles);

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

const Texture* TubeScene::builderBrickTexture(const SceneOptions::InjectedNativeWalker& w) const {
  if (builderBrickShape(w.builderPhase).scale <= 0.0 || !haveCollision_ ||
      collision_.width <= 0) return nullptr;
  const int x = (w.builderX + (w.facing > 0 ? 1 : -w.builderLength) +
                 collision_.width) % collision_.width;
  const int band = std::clamp(w.builderY / 16, 0, static_cast<int>(mesh_.bandCount()) - 1);
  const auto* tile = findTile(true, x / 32,
      (static_cast<int>(mesh_.bandCount()) - 1 - band) / 2);
  return tile ? &tile->texture : nullptr;
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
// placement failure). Drawing also rasterizes into depth-clipped scratch
// storage and skips wholly covered figures. Picking needs only the placement:
// a submitted actor keeps its box even when no body pixels are exposed.
enum class ActorPreparation { ClippedSprite, PlacementOnly };

struct PreparedInjectedWalker {
  webo::SpriteFrame frame;
  webo::Image texture;
  ActorSpritePlacement placement{};
  ActorSpritePlacement composedPlacement{}; // body plus render-only attachment
  bool mirrored = false;
  bool inverted = false;
  std::array<std::uint8_t, 3> tint{255, 255, 255};
  Image clippedSprite;
  std::array<double, 3> worldPosition{};
  std::vector<double> attachmentDepth;
};

// The native mesh's world-space lift does not track the clone's fixed-size
// SPR at every zoom. PROVISIONAL attachment from the selected directional
// image: the red pack's top edge, mirrored with exactly the body placement.
// Water/acid or replacement art without a red pack uses a body-relative anchor.
std::array<double, 2> builderPackAnchor(const SpriteFrame& frame, const Image& sheet,
    const ActorSpritePlacement& p, bool mirrored, bool inverted) {
  double sumX = 0.0;
  int count = 0;
  unsigned top = frame.height;
  for (unsigned y = 0; y < frame.height && frame.top + y < unsigned(sheet.height); ++y)
    for (unsigned x = 0; x < frame.width && frame.left + x < unsigned(sheet.width); ++x) {
      const auto at = (std::size_t(frame.top + y) * sheet.width + frame.left + x) * 4;
      const auto* c = sheet.rgba.data() + at;
      if (actorSpriteAlphaTestPasses(c[3]) && c[0] > 95 &&
          c[0] > c[1] * 1.6 && c[0] > c[2] * 1.3) {
        sumX += x + 0.5;
        top = std::min(top, y);
        ++count;
      }
    }
  double u = count ? sumX / count / frame.width : 0.4;
  double v = count ? double(top) / frame.height : 0.45;
  if (mirrored) u = 1.0 - u;
  if (inverted) v = 1.0 - v;
  return {std::lerp(p.left, p.right, u), std::lerp(p.top, p.bottom, v)};
}

// Rasterise the separate brick before the body so the builder's hands cover it.
// Native 410C0E/410E78 builds a terrain-textured mesh, not a SPR attachment.
// PROVISIONAL geometry/attachment: cuboid expanding at the pack, moving to the
// exact clone cell footprint. No terrain/depth writes escape this scratch raster.
void drawBuilderBrick(Image& image, ActorSpritePlacement& bounds,
    const ActorPoseLevel& level, const CameraBasis& camera,
    const SceneOptions::InjectedNativeWalker& w, const Texture& texture,
    const std::array<std::uint8_t, 3>& tint, const std::array<double, 2>& pack,
    std::vector<double>& attachmentDepth) {
  const auto shape = builderBrickShape(w.builderPhase);
  if (shape.scale <= 0.0 || w.builderLength <= 0) return;
  const double x0 = w.builderX + (w.facing > 0 ? 1 : -w.builderLength);
  const double targetX = x0 + w.builderLength * 0.5;
  const double targetY = w.builderY + 0.5;
  const double centreX = std::lerp(w.plus8X, targetX, shape.travel);
  const double centreY = std::lerp(targetY, w.plus8Y + 5.0 * w.verticalDir, shape.lift);
  const double centreZ = std::lerp(w.plus8Z, 0.5, shape.travel);
  std::array<std::array<double, 4>, 8> vertices;
  for (int i = 0; i < 8; ++i) {
    const ActorTerrainSample sample{
        centreX + ((i & 1) ? 0.5 : -0.5) * w.builderLength * shape.scale,
        centreY + ((i & 2) ? 0.5 : -0.5) * shape.scale,
        centreZ + ((i & 4) ? 0.5 : -0.5) * shape.scale};
    const auto p = actorPoseFromCylinderSample(level, sample, w.facing).modelPosition;
    if (!project(camera, {p[0], p[1], p[2]}, vertices[i])) return;
  }
  const auto packWorld = actorPoseFromCylinderSample(level,
      {w.plus8X, w.plus8Y + 5.0 * w.verticalDir, w.plus8Z}, w.facing).modelPosition;
  std::array<double, 4> projectedPack;
  if (!project(camera, {packWorld[0],packWorld[1],packWorld[2]}, projectedPack)) return;
  double minY = vertices[0][1], maxY = minY;
  for (const auto& v : vertices) { minY = std::min(minY,v[1]); maxY = std::max(maxY,v[1]); }
  // Keep the emerging brick against the pack rather than floating above it.
  // The correction vanishes on delivery, leaving the exact terrain footprint.
  const double dx = (pack[0] - projectedPack[0]) * (1.0 - shape.travel);
  const double dy = (pack[1] - w.verticalDir * (maxY-minY) * 0.4 - projectedPack[1]) * shape.lift;
  for (auto& v : vertices) { v[0] += dx; v[1] += dy; }
  // Use the same tile-local UV footprint as the forthcoming two terrain cells.
  const double u0 = (static_cast<int>(x0) & 31) / 32.0;
  const double u1 = u0 + w.builderLength / 32.0;
  const double v0 = 1.0 - (w.builderY & 31) / 32.0, v1 = v0 - 1.0 / 32.0;
  const std::array<std::array<double, 2>, 4> uv{{{u0,v0},{u0,v1},{u1,v1},{u1,v0}}};
  constexpr int faces[6][4]{{0,2,3,1},{4,6,7,5},{0,4,5,1},
                           {2,6,7,3},{0,2,6,4},{1,3,7,5}};
  Raster brick(image.width, image.height);
  for (const auto& face : faces)
    for (const auto& tri : {std::array{0,1,2}, std::array{0,2,3}}) {
      std::array<std::array<double,4>,3> p;
      std::array<std::array<double,2>,3> t;
      for (int i = 0; i < 3; ++i) { p[i] = vertices[face[tri[i]]]; t[i] = uv[tri[i]]; }
      brick.triangle(p, t, texture, {tint[0]/255.0,tint[1]/255.0,tint[2]/255.0});
    }
  for (const auto& v : vertices) {
    bounds.left = std::min(bounds.left, std::floor(v[0]));
    bounds.right = std::max(bounds.right, std::ceil(v[0]));
    bounds.top = std::min(bounds.top, std::floor(v[1]));
    bounds.bottom = std::max(bounds.bottom, std::ceil(v[1]));
  }
  const auto& rgb = brick.framebuffer().rgb;
  attachmentDepth = brick.depthBuffer();
  for (int y = std::max(0, int(std::floor(bounds.top)));
       y < std::min(image.height, int(std::ceil(bounds.bottom))); ++y)
    for (int x = std::max(0, int(std::floor(bounds.left)));
         x < std::min(image.width, int(std::ceil(bounds.right))); ++x) {
      const auto at = std::size_t(y) * image.width + x;
      if (!std::isfinite(brick.depthBuffer()[at])) continue;
      std::copy_n(rgb.begin() + at * 3, 3, image.rgba.begin() + at * 4);
      image.rgba[at * 4 + 3] = 255;
    }
}

void clipBodyFootprint(Image& body, const ActorSpritePlacement& p,
    const ActorOcclusionPlane& plane, const CameraBasis& camera,
    const CylinderMesh& mesh, const CollisionGrid& collision) {
  for (int y = std::max(0, int(std::ceil(p.top)));
       y < std::min(body.height, int(std::ceil(p.bottom))); ++y)
    for (int x = std::max(0, int(std::ceil(p.left)));
         x < std::min(body.width, int(std::ceil(p.right))); ++x) {
      auto& alpha = body.rgba[(std::size_t(y) * body.width + x) * 4 + 3];
      if (actorSpriteAlphaTestPasses(alpha) &&
          actorTexelInsideTerrain(mesh, collision, plane.worldAt(camera,x+.5,y+.5))) alpha = 0;
    }
}

// Smaller physical view Z wins. Effects without a world-upright body retain
// an origin-depth billboard; ordinary figures supply the same plane to drawing,
// highlight via prepareInjectedWalker. This never writes scenery depth.
bool clipActorToScene(Image& sprite, const ActorSpritePlacement& p,
                      std::span<const double> depth,
                      const ActorOcclusionPlane* plane = nullptr,
                      std::span<const double> pipeDepth = {}) {
  bool visible = false;
  const double originZ = actorPhysicalDepth(p, sprite.width);
  for (int y = std::max(0, int(std::ceil(p.top)));
       y < std::min(sprite.height, int(std::ceil(p.bottom))); ++y) {
    for (int x = std::max(0, int(std::ceil(p.left)));
         x < std::min(sprite.width, int(std::ceil(p.right))); ++x) {
      const auto at = std::size_t(y) * sprite.width + x;
      auto& alpha = sprite.rgba[at * 4 + 3];
      if (!actorSpriteAlphaTestPasses(alpha)) continue;
      const double z = plane ? plane->depthAt(x + 0.5, y + 0.5) : originZ;
      if ((at < depth.size() && !(z < depth[at])) ||
          (at < pipeDepth.size() && !(z < pipeDepth[at]))) alpha = 0;
      else visible = true;
    }
  }
  return visible;
}

std::optional<PreparedInjectedWalker> prepareInjectedWalker(
    const ActorPoseLevel& poseLevel, const SpriteSheet& sprites, const ActorLightMap& light,
    const CameraBasis& camera, const SceneOptions& options,
    const SceneOptions::InjectedNativeWalker& injected, const AssetLibrary& lib,
    const TubePassBounds& bounds, std::span<const double> depth,
    ActorPreparation preparation = ActorPreparation::ClippedSprite,
    const Texture* brickTexture = nullptr,
    const CylinderMesh* footprintMesh = nullptr, const CollisionGrid* footprintGrid = nullptr) {
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
    pose = actorPoseFromCylinderSample(poseLevel, plus8, injected.facing);
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

  const auto scores = gfx::spriteViewScores(cameraToActor, basisVec,
      options.composeActorPhase + injected.rotationPhase);
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
  *placement = actorSpriteGravityPlacement(*placement, injected.verticalDir);

  std::array<std::uint8_t, 3> tint{255, 255, 255};
  if (!light.empty()) {
    if (auto looked = actorLightTintAt(light, worldPosition[0], worldPosition[1]))
      tint = *looked;
  }

  PreparedInjectedWalker prepared;
  prepared.frame = frame;
  prepared.texture = std::move(*texture);
  prepared.placement = *placement;
  prepared.composedPlacement = *placement;
  prepared.mirrored = selection.mirrored;
  prepared.inverted = injected.verticalDir < 0;
  prepared.tint = tint;
  prepared.worldPosition = worldPosition;
  // Keep the explicit visibility diagnostic able to reveal rejected actors,
  // including actors hidden by scene depth. Production never sets this flag.
  const bool sceneClip = !options.composeSkipActorVisibility && !options.composeSkipActorPose;
  if (preparation == ActorPreparation::ClippedSprite && (sceneClip || brickTexture)) {
    auto& sprite = prepared.clippedSprite;
    sprite.width = camera.width;
    sprite.height = camera.height;
    sprite.alpha = AlphaMode::Embedded;
    sprite.rgba.assign(std::size_t(sprite.width) * sprite.height * 4, 0);
    const ActorOcclusionPlane plane(camera, worldPosition);
    if (brickTexture)
      drawBuilderBrick(sprite, prepared.composedPlacement, poseLevel, camera, injected,
          *brickTexture, tint, builderPackAnchor(frame, prepared.texture, *placement,
                                                selection.mirrored, prepared.inverted),
          prepared.attachmentDepth);
    // Keep the carried mesh beneath the hands, with its own physical depth
    // against other actors. The body's terrain footprint cannot erase it.
    Image body;
    if (brickTexture) {
      body = sprite;
      std::fill(body.rgba.begin(), body.rgba.end(), 0);
    }
    auto& bodyTarget = brickTexture ? body : sprite;
    if (!drawActorSprite(bodyTarget, frame, prepared.texture, *placement, selection.mirrored, tint,
                         true, prepared.inverted)) return std::nullopt;
    if (sceneClip && footprintMesh && footprintGrid)
      clipBodyFootprint(bodyTarget, *placement, plane, camera, *footprintMesh, *footprintGrid);
    if (brickTexture) {
      for (int y = std::max(0,int(std::ceil(placement->top)));
           y < std::min(body.height,int(std::ceil(placement->bottom))); ++y)
        for (int x = std::max(0,int(std::ceil(placement->left)));
             x < std::min(body.width,int(std::ceil(placement->right))); ++x) {
          const auto at = std::size_t(y)*body.width+x;
          if (!actorSpriteAlphaTestPasses(body.rgba[at*4+3])) continue;
          std::copy_n(body.rgba.begin()+at*4,4,sprite.rgba.begin()+at*4);
          if (!prepared.attachmentDepth.empty())
            prepared.attachmentDepth[at] = plane.depthAt(x+.5,y+.5);
        }
    }
    if (sceneClip && !clipActorToScene(sprite, prepared.composedPlacement,
            injected.drownFluidKind >= 0 && !options.fluidDrownSceneDepth.empty() ?
                options.fluidDrownSceneDepth : depth,
            &plane, options.pipeSceneDepth)) return std::nullopt;
  }
  return prepared;
}

}  // namespace

void TubeScene::drawInjectedWalkers(Raster& raster, const CameraBasis& camera,
                                    const SceneOptions& options,
                                    const AssetLibrary& lib) const {
  profile::Scope profileActors(profile::Actors);
  if (options.injectedWalkers.empty() || !havePoseLevel_) {
    raster.mutableActorDepth().clear();
    return;
  }
  raster.mutableActorDepth().reset(raster.framebuffer().rgb.size() / 3);
  rewirePoseLevel();

  Image overlay;
  overlay.width = raster.framebuffer().width;
  overlay.height = raster.framebuffer().height;
  overlay.alpha = AlphaMode::Embedded;
  overlay.rgba.assign(static_cast<std::size_t>(overlay.width) * overlay.height * 4, 0);

  const auto bounds = tubePassBounds(camera.selectorTurns, cylinderExtent_);
  std::vector<std::size_t> order(options.injectedWalkers.size());
  std::iota(order.rbegin(), order.rend(), 0);
  const bool physicalActors = !options.composeSkipActorVisibility && !options.composeSkipActorPose;
  if (physicalActors) {
    // Upright body planes are parallel: sort their horizontal camera distance,
    // not radius, column, height or creation order. Far-to-near also preserves
    // the existing additive drowning blend over bodies behind it.
    std::vector<double> distance(order.size());
    const auto& m = camera.worldToView;
    for (std::size_t i = 0; i < order.size(); ++i) {
      const auto& w = options.injectedWalkers[i];
      const auto p = actorPoseFromCylinderSample(poseLevel_,
          {w.plus8X,w.plus8Y,w.plus8Z}, w.facing).modelPosition;
      distance[i] = m[2]*p[0] + m[8]*p[2];
    }
    std::sort(order.begin(),order.end(),[&](auto a,auto b) {
      return distance[a] != distance[b] ? distance[a] > distance[b] : a > b;
    });
  }
  for (const auto owner : order) {
    const auto it = options.injectedWalkers.begin() + owner;
    const auto* sprites = lemmingSprites(it->sheetType);
    if (!sprites) continue;
    auto prepared = prepareInjectedWalker(poseLevel_, *sprites, actorLight_, camera,
        options, *it, lib, bounds, raster.depthBuffer(), ActorPreparation::ClippedSprite,
        builderBrickTexture(*it), &mesh_, haveCollision_ ? &collision_ : nullptr);
    if (!prepared)
      continue;
    if (it->drownFluidKind >= 0 && !prepared->clippedSprite.rgba.empty()) {
      // READ 4043F3..404434 sets selector 2 (ONE/ONE) and equal RGB
      // proportional to height above the fluid bottom. Preserve ordinary
      // actor composition; flush its pending layer before this additive one.
      compositeActorOverlay(raster, overlay, options.overlayCompositeLegacyNonZero);
      std::fill(overlay.rgba.begin(), overlay.rgba.end(), 0);
      auto add = [&](const Image& sprite, double strength, const ActorOcclusionPlane* plane,
                     double originZ) {
        auto& rgb = raster.mutableFramebuffer().rgb;
        for (std::size_t p = 0; p < sprite.pixelCount(); ++p) {
          if (!actorSpriteAlphaTestPasses(sprite.rgba[p * 4 + 3])) continue;
          const double z = plane ? plane->depthAt(p % sprite.width + .5, p / sprite.width + .5) : originZ;
          if (physicalActors && !raster.mutableActorDepth().submit(p,z,owner)) continue;
          for (int c = 0; c < 3; ++c)
            rgb[p * 3 + c] = std::min(255, rgb[p * 3 + c] +
                int(std::lround(sprite.rgba[p * 4 + c] * strength)));
        }
      };
      // The fluid state supplies neutral brightness rather than the light map.
      Image figure = std::move(prepared->clippedSprite);
      std::fill(figure.rgba.begin(), figure.rgba.end(), 0);
      const auto grey = std::uint8_t(std::clamp(it->drownBrightness, 0.0, 1.0) * 255.0);
      if (!drawActorSprite(figure, prepared->frame, prepared->texture, prepared->placement,
          prepared->mirrored, {grey, grey, grey}, true, prepared->inverted)) continue;
      const auto pose = actorPoseFromCylinderSample(poseLevel_,
          {it->plus8X, it->plus8Y, it->plus8Z}, it->facing);
      const ActorOcclusionPlane plane(camera, pose.modelPosition);
      if (physicalActors && haveCollision_)
        clipBodyFootprint(figure,prepared->placement,plane,camera,mesh_,collision_);
      clipActorToScene(figure, prepared->placement,
          options.fluidDrownSceneDepth.empty() ? raster.depthBuffer() : options.fluidDrownSceneDepth,
          &plane, options.pipeSceneDepth);
      add(figure, 1.0, &plane, 0);

      // READ: 404372 creates Bubbles FX once submerged by 5..10 units.
      // PROVISIONAL deterministic presentation: two installed bubble sprites
      // rise from the body; small early ones stand in for splash droplets.
      // Original randomized emitter/lifetimes are not reconstructed.
      if (it->drownFluidKind == 0) {
        const Sprite* bubbles = bubbleSprites_.find("bubbles");
        if (bubbles && !bubbles->animations.empty()) {
          const double t = it->drownSeconds;
          for (int b = 0; b < 2; ++b) {
            const bool splash = t < 0.6;
            const double phase = std::fmod(std::max(0.0, t - 0.5) + b * 0.21, 0.48);
            const std::size_t n = std::min(bubbles->animations.size() - 1,
                std::size_t(phase / 0.06));
            if (bubbles->animations[n].frames.empty()) continue;
            const auto& frame = bubbles->animations[n].frames[0];
            auto texture = loadTexture(lib, sheetAssetName(frame.sheet));
            if (!texture) continue;
            ActorTerrainSample sample{it->plus8X + (b ? 1.5 : -1.5),
                splash ? it->drownSurfaceY + 2.0 + 5.0 * std::sin(t / 0.6 * std::numbers::pi) :
                    std::min(it->drownSurfaceY, it->plus8Y + 5.0 + phase * 12.0), it->plus8Z};
            const auto pose = actorPoseFromCylinderSample(poseLevel_, sample, it->facing);
            auto box = actorSpritePlacement(frame, pose.modelPosition, camera, false, -1);
            if (!box) continue;
            const double scale = splash ? 0.45 : 0.8;
            box->left = box->originX + (box->left - box->originX) * scale;
            box->right = box->originX + (box->right - box->originX) * scale;
            box->top = box->originY + (box->top - box->originY) * scale;
            box->bottom = box->originY + (box->bottom - box->originY) * scale;
            Image effect = figure;
            std::fill(effect.rgba.begin(), effect.rgba.end(), 0);
            if (!drawActorSprite(effect, frame, *texture, *box, false)) continue;
            clipActorToScene(effect, *box, options.fluidDrownSceneDepth.empty() ?
                raster.depthBuffer() : options.fluidDrownSceneDepth);
            add(effect, splash ? 0.8 : it->drownBrightness, nullptr,
                actorPhysicalDepth(*box,camera.width));
          }
        }
      }
      continue;
    }
    if (!prepared->clippedSprite.rgba.empty()) {
      const auto& sprite = prepared->clippedSprite;
      const auto& p = prepared->composedPlacement;
      const ActorOcclusionPlane plane(camera, prepared->worldPosition);
      for (int y = std::max(0, int(std::ceil(p.top)));
           y < std::min({overlay.height, sprite.height, int(std::ceil(p.bottom))}); ++y)
        for (int x = std::max(0, int(std::ceil(p.left)));
             x < std::min({overlay.width, sprite.width, int(std::ceil(p.right))}); ++x) {
          const auto at = (std::size_t(y) * overlay.width + x) * 4;
          const auto source = (std::size_t(y) * sprite.width + x) * 4;
          if (!actorSpriteAlphaTestPasses(sprite.rgba[source + 3])) continue;
          const auto pixel = source / 4;
          const double z = prepared->attachmentDepth.empty() ? plane.depthAt(x+.5,y+.5) :
              prepared->attachmentDepth[pixel];
          if (!physicalActors || raster.mutableActorDepth().submit(pixel,z,owner))
            std::copy_n(sprite.rgba.begin()+source,4,overlay.rgba.begin()+at);
        }
      continue;
    }
    if (!app::drawActorSprite(overlay, prepared->frame, prepared->texture, prepared->placement,
                              prepared->mirrored, prepared->tint, true, prepared->inverted)
             .has_value())
      continue;
  }

  compositeActorOverlay(raster, overlay, options.overlayCompositeLegacyNonZero);
}

void TubeScene::advanceDeathParticles(std::uint64_t clockMs) {
  if (deathParticles_.empty()) return;
  deathClockMs_ = clockMs;
  std::erase_if(deathParticles_, [&](const DeathParticle& p) {
    return clockMs >= p.bornMs && (clockMs - p.bornMs) / 1000.0 >= p.lifetime;
  });
}

std::array<std::uint8_t, 3> TubeScene::terrainDebrisColour(
    const std::array<int, 2>& cell, int terrainHeight) const {
  const int row = terrainHeight - 1 - cell[1];
  std::array<std::uint8_t, 3> colour{128, 104, 72};
  // PROVISIONAL chips sample their destroyed block's installed OUT texture.
  // The exact original fragment model/UV selection (40C960) is unresolved.
  // Reuse the cell renderer's material descriptor and reversed tile rows.
  const auto material = cellMesh_.buckets > unsigned(cell[0] / 8) &&
      cellMesh_.bands > unsigned(row / 16) ?
      cellMesh_.at(cell[0] / 8, row / 16)->material :
      std::uint32_t(cell[0] / 32 + cell[1] / 32 * 8);
  if (const auto* tile = findTile(true, material % 8, material / 8);
      tile && !tile->texture.empty()) {
    const auto& t = tile->texture;
    const int px = int(((cell[0] & 31) + 0.5) * t.width / 32);
    const int py = int((31.5 - (row & 31)) * t.height / 32);
    const auto at = (std::size_t(py) * t.width + px) * 3;
    std::copy_n(t.rgb.begin() + at, 3, colour.begin());
  }
  return colour;
}

void TubeScene::emitTerrainDebris(const sim::Lemming& lemming,
    std::span<const std::array<int, 2>> removedCells, std::uint64_t clockMs, int terrainHeight) {
  if (removedCells.empty()) return;
  advanceDeathParticles(clockMs);
  deathClockMs_ = clockMs;
  const int gravityDir = lemming.verticalDir < 0 ? -1 : 1;
  const int facing = static_cast<int>(lemming.facing);
  // READ: bash 40C1C5 (subpasses 0/3), mine 418B69 (rows -1/+1),
  // dig 415BE8 all construct 40C960; 40C9FE..40CA03 creates four fragments.
  // PROVISIONAL: four texture-coloured chips per actually removed hybrid cell,
  // thrown back from the work face (Digger fans both ways). No native subpass
  // clock or physics is transplanted. Life overlaps successive 1.2..1.5s bites;
  // deterministic gravity-only arcs use no simulation RNG or collision.
  for (const auto& cell : removedCells) {
    constexpr std::size_t maxParticles = 4096;
    if (deathParticles_.size() > maxParticles - 4)
      deathParticles_.erase(deathParticles_.begin(), deathParticles_.begin() +
          (deathParticles_.size() - (maxParticles - 4)));
    const auto colour = terrainDebrisColour(cell, terrainHeight);
    for (int i = 0; i < 4; ++i) {
      DeathParticle p;
      p.origin = {double(cell[0]), double(terrainHeight - 1 - cell[1]) + 0.5,
                  gravityDir < 0 ? 0.55 : 0.75};
      p.vx = lemming.state == sim::State::Digger ? (i - 1.5) * 3.0 : -facing * (3.0 + i);
      p.vy = gravityDir * (9.0 + i * 1.5);
      p.vz = (i - 1.5) * 0.04;
      p.gravity = -gravityDir * 16.0;
      p.bornMs = clockMs;
      p.lifetime = 1.6 + i * 0.1;
      p.size = 0.65 + (i % 2) * 0.2;
      p.colour = colour;
      p.verticalDir = gravityDir;
      deathParticles_.push_back(p);
    }
  }
}

void TubeScene::emitDeathParticles(const sim::Lemming& lemming, bool explosion,
    std::span<const std::array<int, 2>> removedCells, std::uint64_t clockMs,
    int terrainHeight, const AssetLibrary& lib) {
  advanceDeathParticles(clockMs);
  deathClockMs_ = clockMs;
  if (!deathSpritesLoaded_) {
    deathSpritesLoaded_ = true;
    if (auto bits = loadSpriteSheet(lib, "SPRITES\\BITS.SPR")) deathSprites_ = std::move(*bits);
  }
  // READ: 408227 creates six actor pieces; 40B3EC does the same on splat.
  // 401C50 selects piece indices 0,1,2,2,3,3. BITS.SPR/$LemBits0 is the
  // installed piece artwork. Its binding and animation sampling are provisional.
  // PROVISIONAL physics throughout: deterministic terrain-coordinate ballistics,
  // gravity 32 units/s^2, no collision/bounce, finite 0.55..1.6 second life.
  // No World RNG, state, counters, timers or terrain are written here.
  auto origin = terrainSampleFromLemming(lemming, terrainHeight,
                                         lemming.verticalDir < 0 ? 0.55 : 0.75);
  const int gravityDir = lemming.verticalDir < 0 ? -1 : 1;
  origin.y += gravityDir * (explosion ? 3.0 : 1.0);
  const auto add = [&](ActorTerrainSample at, int index, int count, bool piece,
                       std::array<std::uint8_t, 3> colour, double life, double size,
                       double spread, double rise) {
    // Deterministic radial fans do not consume simulation/scenery randomness.
    const double angle = (index + 0.5) * 2.0 * std::numbers::pi / count;
    DeathParticle p;
    p.origin = at;
    p.vx = std::cos(angle) * spread;
    p.vy = gravityDir * (rise + std::sin(angle) * rise * 0.35);
    p.vz = std::sin(angle * 3.0) * 0.10;
    p.gravity = -gravityDir * 32.0;
    p.bornMs = clockMs;
    p.lifetime = life;
    p.size = size;
    p.colour = colour;
    constexpr int clips[]{0, 1, 2, 2, 3, 3};
    p.piece = piece ? clips[index % 6] : -1;
    p.verticalDir = gravityDir;
    deathParticles_.push_back(p);
  };
  // Bound presentation memory under repeated Nuke/retry stress. At capacity
  // evict oldest particles only; every new death still gets its complete burst.
  constexpr std::size_t maxParticles = 4096;
  if (deathParticles_.size() > maxParticles - 160)
    deathParticles_.erase(deathParticles_.begin(), deathParticles_.begin() +
        (deathParticles_.size() - (maxParticles - 160)));
  for (int i = 0; i < 6; ++i)
    add(origin, i, 6, true, {255,255,255}, 1.6, 2.0, explosion ? 15.0 : 12.0,
        explosion ? 17.0 : 10.0);
  // READ 43F510: count 100 and RGB(255,32,32). READ 40B3EC:
  // count 10, RGB doubles (255,127,127). Size/motion/life are provisional;
  // plain coloured chips stand in for the unrecovered particle renderer.
  const int count = explosion ? 100 : 10;
  for (int i = 0; i < count; ++i)
    add(origin, i, count, false, explosion ? std::array<std::uint8_t,3>{255,32,32} :
        std::array<std::uint8_t,3>{255,127,127}, 0.55 + (i % 7) * 0.07,
        0.45, 8.0 + (i % 11), explosion ? 12.0 : 5.0);

  if (!explosion || removedCells.empty()) return;
  const auto debris = [&](const std::array<int, 2>& cell) {
    const int row = terrainHeight - 1 - cell[1];
    const auto colour = terrainDebrisColour(cell, terrainHeight);
    for (int i = 0; i < 4; ++i)
      add({double(cell[0]), double(row), origin.z}, i, 4, false, colour,
          1.2 + i * 0.1, 1.4, 9.0 + i, 12.0);
  };
  bool emitted = false;
  // READ 43F510: four fragments at eligible 4x4-aligned removed cells.
  for (const auto& cell : removedCells)
    if ((cell[0] & 3) == 0 && ((terrainHeight - 1 - cell[1]) & 3) == 0) {
      debris(cell);
      emitted = true;
    }
  // PROVISIONAL: a thin destroyed ledge can miss every original grid site.
  // Keep that terrain removal visible with one four-chip burst in this case.
  if (!emitted) debris(removedCells.front());
}

void TubeScene::drawDeathParticles(Raster& raster, const CameraBasis& camera,
                                   const AssetLibrary& lib) const {
  // No allocations, asset reads, depth copies or scans on an empty frame.
  if (deathParticles_.empty() || !havePoseLevel_) return;
  rewirePoseLevel();
  auto& buffer = raster.mutableFramebuffer();
  const auto& depth = raster.depthBuffer();
  auto& actors = raster.mutableActorDepth();
  if (actors.size() != depth.size()) actors.reset(depth.size());
  const auto bounds = tubePassBounds(camera.selectorTurns, cylinderExtent_);
  Image overlay;
  overlay.width = buffer.width; overlay.height = buffer.height;
  overlay.alpha = AlphaMode::Embedded;
  overlay.rgba.resize(std::size_t(buffer.width) * buffer.height * 4);
  for (std::size_t i = 0; i < deathParticles_.size(); ++i) {
    const auto& p = deathParticles_[i];
    const auto owner = ActorDepth::noOwner / 2 + i;
    auto sample = p.position(deathClockMs_);
    sample.x = std::fmod(sample.x, 256.0);
    if (sample.x < 0) sample.x += 256.0;
    if (sample.y < 0 || sample.y >= poseLevel_.gridHeight ||
        !actorSubmissionVisible(sample.x, sample.z, true, bounds)) continue;
    const auto pose = actorPoseFromCylinderSample(poseLevel_, sample, 1);
    if (p.piece >= 0 && std::size_t(p.piece) < deathSprites_.sprites.size()) {
      const auto& clip = deathSprites_.sprites[p.piece];
      if (!clip.animations.empty()) {
        // PROVISIONAL: use stored strip rate when positive, otherwise 12 fps.
        const double rate = clip.rate > 0 && std::isfinite(clip.rate) ? clip.rate : 12.0;
        const auto index = std::size_t((deathClockMs_ - p.bornMs) / 1000.0 * rate) % clip.animations.size();
        const auto& entry = clip.animations[index];
        if (!entry.frames.empty()) {
          const auto& frame = entry.frames.front();
          auto texture = loadTexture(lib, sheetAssetName(frame.sheet));
          auto box = actorSpritePlacement(frame, pose.modelPosition, camera, false, -1);
          if (texture && box) {
            *box = actorSpriteGravityPlacement(*box, p.verticalDir);
            std::fill(overlay.rgba.begin(), overlay.rgba.end(), 0);
            if (drawActorSprite(overlay, frame, *texture, *box, false, p.colour, true,
                                p.verticalDir < 0) &&
                clipActorToScene(overlay, *box, depth)) {
              const double z = actorPhysicalDepth(*box,camera.width);
              for (int y = std::max(0,int(std::ceil(box->top)));
                   y < std::min(buffer.height,int(std::ceil(box->bottom))); ++y)
                for (int x = std::max(0,int(std::ceil(box->left)));
                     x < std::min(buffer.width,int(std::ceil(box->right))); ++x) {
                  const auto at = std::size_t(y)*buffer.width+x;
                  auto& alpha = overlay.rgba[at*4+3];
                  if (actorSpriteAlphaTestPasses(alpha) && !actors.submit(at,z,owner)) alpha=0;
                }
              compositeActorOverlay(raster, overlay, false);
            }
            continue;
          }
        }
      }
    }
    std::array<double, 4> projected;
    if (!project(camera, {pose.modelPosition[0], pose.modelPosition[1], pose.modelPosition[2]}, projected))
      continue;
    const double r = std::clamp(camera.focalY * p.size / projected[2], 0.6, 5.0);
    // Physical view Z, matching body planes and body-piece sprites. The old
    // 0.64-unit chip pull could paint a farther chip over a nearer lemming.
    const double z = projected[2];
    for (int y = std::max(0, int(std::ceil(projected[1] - r)));
         y < std::min(buffer.height, int(std::ceil(projected[1] + r))); ++y)
      for (int x = std::max(0, int(std::ceil(projected[0] - r)));
           x < std::min(buffer.width, int(std::ceil(projected[0] + r))); ++x) {
        const auto at = std::size_t(y) * buffer.width + x;
        if (z < depth[at] && actors.submit(at,z,owner))
          std::copy(p.colour.begin(), p.colour.end(), buffer.rgb.begin()+at*3);
      }
  }
}

bool TubeScene::drawHoveredWalker(Image& target, std::size_t walker,
    const CameraBasis& camera, const SceneOptions& options,
    const AssetLibrary& lib, const Raster& sceneRaster) const {
  if (walker >= options.injectedWalkers.size() || !havePoseLevel_ ||
      target.width != camera.width || target.height != camera.height) return false;
  rewirePoseLevel();
  const auto* sprites = lemmingSprites(options.injectedWalkers[walker].sheetType);
  if (!sprites) return false;
  auto prepared = prepareInjectedWalker(poseLevel_, *sprites, actorLight_, camera,
      options, options.injectedWalkers[walker], lib,
      tubePassBounds(camera.selectorTurns, cylinderExtent_), sceneRaster.depthBuffer(),
      ActorPreparation::ClippedSprite, builderBrickTexture(options.injectedWalkers[walker]),
      &mesh_, haveCollision_ ? &collision_ : nullptr);
  if (!prepared) return false;
  auto sprite = std::move(prepared->clippedSprite);
  if (sprite.rgba.empty()) return false;
  // READ 454ACA..454B5C copies the selected sprite and redraws it with selector
  // 2 (ONE/ONE), equal RGB from runtime 4C6988 and an extra depth adjustment.
  // PROVISIONAL: half-strength additive sprite, no pulse. Runtime intensity,
  // effective original depth state and pixel blend need original observation.
  const auto& p = prepared->composedPlacement;
  bool marked = false;
  for (int y = std::max(0, int(std::ceil(p.top))); y < std::min(target.height, int(std::ceil(p.bottom))); ++y)
    for (int x = std::max(0, int(std::ceil(p.left))); x < std::min(target.width, int(std::ceil(p.right))); ++x) {
      const auto at = (std::size_t(y) * target.width + x) * 4;
      if (!actorSpriteAlphaTestPasses(sprite.rgba[at + 3])) continue;
      if (!sceneRaster.actorDepth().owns(at / 4,walker)) continue;
      for (int c = 0; c < 3; ++c)
        target.rgba[at + c] = std::min(255, target.rgba[at + c] + (sprite.rgba[at + c] + 1) / 2);
      marked = true;
    }
  return marked;
}

std::vector<std::optional<ActorSpritePlacement>> TubeScene::injectedWalkerScreenBoxes(
    const CameraBasis& camera, const SceneOptions& options, const AssetLibrary& lib) const {
  Raster sceneRaster(camera.width, camera.height);
  auto geometry = options;
  geometry.injectedWalkers.clear();
  render(sceneRaster, camera, geometry, nullptr);
  return injectedWalkerScreenBoxes(camera, options, lib, sceneRaster);
}

std::vector<std::optional<ActorSpritePlacement>> TubeScene::injectedWalkerScreenBoxes(
    const CameraBasis& camera, const SceneOptions& options, const AssetLibrary& lib,
    const Raster& sceneRaster) const {
  std::vector<std::optional<ActorSpritePlacement>> boxes(options.injectedWalkers.size());
  if (options.injectedWalkers.empty() || !havePoseLevel_ ||
      sceneRaster.framebuffer().width != camera.width || sceneRaster.framebuffer().height != camera.height)
    return boxes;
  rewirePoseLevel();
  const auto bounds = tubePassBounds(camera.selectorTurns, cylinderExtent_);
  for (std::size_t i = 0; i < options.injectedWalkers.size(); ++i) {
    const auto* sprites = lemmingSprites(options.injectedWalkers[i].sheetType);
    if (!sprites) continue;
    if (auto prepared = prepareInjectedWalker(poseLevel_, *sprites, actorLight_, camera,
          options, options.injectedWalkers[i], lib, bounds, sceneRaster.depthBuffer(),
          ActorPreparation::PlacementOnly))
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
  if (options.drawCellRoute && !options.composeActorsBeforeRoute)
    maybeCompose();
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
    // READ 40B4AF hides the intact actor on splat entry. Retain its vector
    // slot so screen-box/hover indices for other live actors stay aligned.
    walker.renderEnabled = lemming.state != sim::State::Splatter;
    walker.plus8X = sample.x;
    walker.plus8Y = sample.y;
    walker.plus8Z = sample.z;
    if (motion) {
      const auto drawn = motion->positionOf(i, world);
      walker.plus8X = drawn.x;
      walker.plus8Y = terrainHeight - 1.0 - drawn.y;
      // Q1: same radial lane sample for actor placement, occlusion and picking.
      walker.plus8Z = drawn.z;
    }
    walker.facing = static_cast<int>(lemming.facing);
    walker.verticalDir = lemming.verticalDir;
    walker.sheetType = lemming.sheetType;
    // Without a tracker every state draws walk entry 0 (the D3b behaviour); with one, the
    // lemming's own clip and temporal entry (actor-anim-001).
    if (anim != nullptr) {
      const LemmingAnimPose pose = anim->poseOf(i);
      walker.clip = pose.clip;
      walker.walkTemporalIndex = pose.temporalIndex;
      walker.drownSeconds = pose.drownSeconds;
      if (lemming.state == sim::State::Builder) {
        walker.builderPhase = builderPresentationPhase(world, lemming);
        walker.builderX = lemming.x;
        walker.builderY = terrainHeight - 1 - lemming.y;
        walker.builderLength = world.rules().brickLength;
        // Both hands and brick use this phase. Other clips retain their existing clock.
        walker.walkTemporalIndex = std::clamp(int(walker.builderPhase * 16.0), 0, 15);
      }
      if (lemming.state == sim::State::Drown && pose.drownSeconds >= 0.0) {
        const double row = terrainHeight - 1.0 - lemming.y;
        const int width = world.terrain().width();
        for (const auto& g : world.setup().gates) {
          if (width <= 0 || g.kind < 0 || g.kind > 2 || g.height <= 0) continue;
          double span = g.maxX - g.minX;
          if (span < 0) span += width;
          double dx = std::fmod(lemming.x - g.minX, double(width));
          if (dx < 0) dx += width;
          if (dx >= span || row < g.minY || row > g.minY + g.height) continue;
          walker.drownFluidKind = g.kind;
          walker.drownSurfaceY = g.minY + g.height;
          // READ 404655..404675: Y -= verticalDir * 20 * dt / 2.
          // Presentation only: World position, death timer and counters stay
          // unchanged. PROVISIONAL clamp at the receiver bottom; the original
          // removes type 0x23 there, while the clone's existing timer owns death.
          walker.plus8Y = std::clamp(row - lemming.verticalDir * 10.0 * pose.drownSeconds,
              g.minY, g.minY + g.height);
          walker.drownBrightness = std::clamp((walker.plus8Y - g.minY) /
              (g.high > 0.0 ? g.high : g.height), 0.0, 1.0);
          break;
        }
      }
    }
    out.push_back(walker);
  }
  // Exit tails are render-only and follow the live prefix. Keep the established
  // alive-lemming -> screen-box indices used by hovering, skills and observers.
  if (motion) for (std::size_t i = 0; i < lemmings.size(); ++i) {
    const double t = motion->exitSeconds(i, world);
    if (t < 0.0) continue;
    const auto& lemming = lemmings[i];
    const auto p = motion->positionOf(i, world);
    SceneOptions::InjectedNativeWalker jumper;
    jumper.plus8X = p.x;
    jumper.plus8Y = terrainHeight - 1.0 - p.y;
    jumper.plus8Z = p.z;
    jumper.facing = static_cast<int>(lemming.facing);
    jumper.verticalDir = lemming.verticalDir;
    jumper.sheetType = lemming.sheetType;
    jumper.clip = "fall";  // 41800A: no separate exit strip
    jumper.rotationPhase = 0.25;  // 418017..418027 -> 4683E9
    if (anim) jumper.walkTemporalIndex = anim->poseOf(i).temporalIndex;
    out.push_back(jumper);
  }
  return out;
}

}  // namespace webo::app
