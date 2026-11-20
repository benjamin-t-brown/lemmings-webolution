#include "app/FrameProfile.h"
#include "assets/TextureCache.h"
#include "TubeScene.h"
#include "app/ActorVisibility.h"

#include "app/ActorSprite.h"
#include "app/ActorOcclusion.h"
#include "app/BuilderBrick.h"
#include "app/CrtRand.h"
#include "app/HoverFeedback.h"
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
#include "lemmings/compiled_level.hpp"

#include <algorithm>
#include <cmath>
#include <format>
#include <numbers>
#include <numeric>
#include <set>
#include <utility>

namespace webo::app {
namespace {

constexpr std::uint64_t kForcedDeathFlightLifetimeMs = 4000;

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
  if (!levelBytes.empty()) try {
    const auto level = lemmings::read_compiled_level(
        {reinterpret_cast<const std::uint8_t*>(levelBytes.data()), levelBytes.size()}, false);
    // 4082B6 -> 43B31E selects index 0 of the fifth name table. The second
    // entry (~splat2) has no traced producer; never substitute it.
    if (!level.prefix.fifth_names.empty()) {
      const auto name = levelTextureAssetName(level.prefix.fifth_names.front());
      if (auto image = loadTexture(lib, name))
        scene.explosionTexture_ = {image->width, image->height, std::move(image->rgba)};
      else scene.missing_.push_back(name + ": " + image.error());
    }
    for (const auto& t : level.world.integer_triples)
      scene.counterGateCoordinates_.push_back({static_cast<int>(t[0]), static_cast<int>(t[1]), static_cast<int>(t[2])});
    const auto points = [](int first, int last, int stride) {
      std::vector<int> out{first};
      for (int p = (first / stride + 1) * stride; p < last; p += stride) out.push_back(p);
      if (last != first) out.push_back(last);
      return out;
    };
    // 0x42727C -> 0x43A1C6 resolves each gate (x,y,pool-index) reference,
    // creating a record only if absent. Installed gate levels supply the records.
    for (const auto& e : level.gadgets.entities_424187) {
      const int left = static_cast<int>(e.values[0]), right = static_cast<int>(e.values[1]);
      const int top = static_cast<int>(e.values[2]), bottom = static_cast<int>(e.values[3]);
      for (const int x : points(left, right, 8)) for (const int y : points(bottom, top, 16)) {
        const int cx = x & 255;
        auto it = std::find_if(scene.counterGateCoordinates_.begin(), scene.counterGateCoordinates_.end(),
            [&](const auto& t) { return t[0] == cx && t[1] == y; });
        std::int32_t index;
        if (it != scene.counterGateCoordinates_.end()) index = (*it)[2];
        else {
          const auto v = cylinderTemplateVertex(mesh, cx, y, 1, static_cast<int>(mesh.faceCount));
          if (!v) continue;
          index = static_cast<std::int32_t>(scene.mesh_.surface.size());
          scene.mesh_.surface.push_back(*v);
          scene.counterGateCoordinates_.push_back({cx, y, index});
          // The clone's mesh key at the cylinder's right edge is 256.
          // Alias it to the same movable vertex as wrapped column zero.
        }
        if (cx == 0 && std::none_of(scene.counterGateCoordinates_.begin(), scene.counterGateCoordinates_.end(),
            [&](const auto& t) { return t[0] == 256 && t[1] == y; }))
          scene.counterGateCoordinates_.push_back({256, y, index});
      }
      // Keep the gate topology even when its live collision is clear. At
      // progress zero these faces retract onto the inner cylinder.
      for (int x = left; x < right; ++x) for (int y = bottom; y < top; ++y) {
        if (y >= 0 && y < scene.collision_.height && scene.haveCollision_)
          scene.collision_.cells[static_cast<std::size_t>(y) * scene.collision_.width + (x & 255)] |= 0x403u;
      }
    }
  } catch (const std::invalid_argument& e) {
    // Accept partial diagnostic inputs here. RetailLevel validates playable inputs.
    scene.missing_.push_back(std::string("gate coordinates: ") + e.what());
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
      // 0x0047218A restores the unsuffixed name after a failed detail probe.
      // An existing HI/Lo entry must load successfully.
      if (!lib.contains(assetName))
        assetName = levelTextureAssetName(name);
      auto image = loadTexture(lib, assetName);
      if (!image) {
        scene.missing_.push_back(std::format("{}: {}", name, image.error()));
        continue;
      }
      // Clamp tube-skin sampling at tile edges, as at 0x43A6BB.
      const bool clampRoute = assetName.starts_with(R"(TEXTURES\CYLINDER\E22_KF_1\)") ||
                              assetName.starts_with(R"(TEXTURES\CYLINDER\E11_KF_1\)");
      const auto routeAddress = clampRoute ? SceneAddress::Clamp : SceneAddress::Wrap;
      into.push_back(Tile{toTexture(*image), pos.column, pos.row, routeAddress});
      ++count;
    }
  };
  loadBlock(mesh.tileTextures, true, scene.outer_, scene.outerTileCount_);
  loadBlock(mesh.innerTileTextures, false, scene.inner_, scene.innerTileCount_);

  // Build the load-time cell mesh when stored keys support it.
  if (scene.haveCollision_ && !grid0.empty()) {
    if (auto built = buildCellMeshes(grid0, scene.counterGateCoordinates_, scene.collision_, scene.mesh_.surface.size())) {
      scene.cellMesh_ = std::move(*built);
    } else {
      scene.missing_.push_back(std::format("cell mesh: {}", built.error()));
    }
  }

  // Reconstruct lattice faces when the optional lattice is present.
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

  // Optional PolyList 09 records carry their own CylinderMesh::surface index, UV and RGBA.
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
    // 0x43D56C clamps against the serialized height (112, world+0x28).
    // Cylinder stores this word as faceCount. The collision mask has 96 rows.
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
                                "SPRITES\\LEMMINGSACIDHI.SPR", "SPRITES\\EVILRATHI.SPR"};
  for (int type = 0; type < 4; ++type) {
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
  // READ 496E40 binds 4E331C; 4196C0 -> 493761 submits the live child.
  if (auto axe = loadSpriteSheet(lib, "SPRITES\\AXE.SPR"))
    scene.axeSprites_ = std::move(*axe);

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

std::array<double, 3> TubeScene::soundWorldPosition(
    const std::array<double, 3>& position, int terrainHeight) const {
  rewirePoseLevel();
  return actorPoseFromCylinderSample(poseLevel_,
      {position[0], terrainHeight - 1.0 - position[1], position[2]}, 1).modelPosition;
}

void TubeScene::drawHoveredSensor(Image& image, const CameraBasis& camera, const Raster& raster,
    const sim::OccupancySensor& sensor) const {
  if (!havePoseLevel_ || image.width != camera.width || image.height != camera.height) return;
  // Provisional clone contact marker for a tagged occupancy volume. It has no
  // normal draw path. Place the ring at the volume's centre in world space.
  const auto& low = sensor.min;
  const auto& high = sensor.max;
  constexpr double width = 256.0;
  const double x = std::midpoint(low[0], high[0] < low[0] ? high[0] + width : high[0]);
  const double y = std::midpoint(low[1], high[1]);
  const double slab = std::midpoint(low[2], high[2]);
  if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(slab)) return;
  rewirePoseLevel();
  const auto at = actorPoseFromCylinderSample(poseLevel_, {std::fmod(x, width), y, slab}, 1).modelPosition;
  std::array<double, 4> p;
  if (!project(camera, {at[0], at[1], at[2]}, p)) return;
  constexpr int radius = 5;
  if (!(p[0] >= -radius && p[0] < image.width + radius &&
        p[1] >= -radius && p[1] < image.height + radius)) return;
  const ActorOcclusionPlane plane(camera, at);
  constexpr std::array<std::uint8_t, 3> light{128, 96, 48};
  const int cx = int(std::floor(p[0])), cy = int(std::floor(p[1]));
  for (int py = std::max(0, cy - radius); py < std::min(image.height, cy + radius + 1); ++py)
    for (int px = std::max(0, cx - radius); px < std::min(image.width, cx + radius + 1); ++px) {
      const double dx = px + 0.5 - p[0], dy = py + 0.5 - p[1];
      const double r2 = dx * dx + dy * dy;
      if (r2 > radius * radius || r2 < 9.0) continue;
      const auto pixel = std::size_t(py) * image.width + px;
      const double z = plane.depthAt(px + 0.5, py + 0.5);
      if (z < raster.depthBuffer()[pixel] && z < raster.actorDepth().depthAt(pixel))
        addHoverRgb(image.rgba.data() + pixel * 4, light.data());
    }
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
      const auto pixel = static_cast<std::size_t>(y) * width + x;
      const auto o = pixel * 3;
      raster.coverSceneHighlight(pixel);
      rgb[o] = overlay.rgba[i];
      rgb[o + 1] = overlay.rgba[i + 1];
      rgb[o + 2] = overlay.rgba[i + 2];
    }
  }
}

// Prepare pose, view, sheet, placement and tint for one submitted actor.
// Drawing depth-tests body pixels. Picking keeps the box even when fully covered.
struct PreparedInjectedWalker {
  webo::SpriteFrame frame;
  webo::Image texture;
  // Borrow images only for the draw/pick call. Cache insertions preserve references.
  const webo::Image* borrowedTexture = nullptr;
  const webo::Image& sheet() const { return borrowedTexture ? *borrowedTexture : texture; }
  ActorSpritePlacement placement{};
  ActorSpritePlacement composedPlacement{}; // body plus render-only attachment
  bool mirrored = false;
  bool inverted = false;
  std::array<std::uint8_t, 3> tint{255, 255, 255};
  std::array<double, 3> worldPosition{};
  std::vector<double> attachmentDepth;
};

// Provisional brick anchor follows the red pack's top edge in the selected
// SPR image, mirrored with the body. Art without a red pack uses a body-relative anchor.
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

// Rasterize the brick before the body so the hands cover it. Native
// 410C0E/410E78 build a terrain-textured mesh. The provisional clone cuboid
// grows at the pack and moves to the target cells in a scratch raster.
void drawBuilderBrick(Image& image, ActorSpritePlacement& bounds,
    const ActorPoseLevel& level, const CameraBasis& camera,
    const SceneOptions::InjectedNativeWalker& w, const Texture& texture,
    const std::array<std::uint8_t, 3>& tint, const std::array<double, 2>& pack,
    std::vector<double>& attachmentDepth) {
  profile::Scope profileBrick(profile::BuilderBrick);
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
  profile::Scope profileFootprint(profile::BodyFootprint);
  for (int y = std::max(0, int(std::ceil(p.top)));
       y < std::min(body.height, int(std::ceil(p.bottom))); ++y)
    for (int x = std::max(0, int(std::ceil(p.left)));
         x < std::min(body.width, int(std::ceil(p.right))); ++x) {
      auto& alpha = body.rgba[(std::size_t(y) * body.width + x) * 4 + 3];
      if (actorSpriteAlphaTestPasses(alpha) &&
          actorTexelInsideTerrain(mesh, collision, plane.worldAt(camera,x+.5,y+.5))) alpha = 0;
    }
}

// Smaller view Z wins. Upright body planes are shared by drawing and
// highlight. Effects without a body plane use origin depth.
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

// Identical half-open texel bounds to drawActorSprite, in screen coordinates.
struct SpriteRect { int left, right, top, bottom; };
SpriteRect spriteRect(const Image& image, const ActorSpritePlacement& p) {
  const int left = std::clamp(int(std::ceil(p.left)), 0, image.width);
  const int top = std::clamp(int(std::ceil(p.top)), 0, image.height);
  return {left, std::clamp(int(std::ceil(p.right)), left, image.width),
          top, std::clamp(int(std::ceil(p.bottom)), top, image.height)};
}

void clearSpriteRect(Image& image, const SpriteRect& rect) {
  for (int y = rect.top; y < rect.bottom; ++y) {
    auto row = image.rgba.begin() + std::size_t(y) * image.width * 4;
    std::fill(row + rect.left * 4, row + rect.right * 4, 0);
  }
}

std::optional<PreparedInjectedWalker> prepareInjectedWalker(
    const ActorPoseLevel& poseLevel, const SpriteSheet& sprites, const ActorLightMap& light,
    const CameraBasis& camera, const SceneOptions& options,
    const SceneOptions::InjectedNativeWalker& injected, const AssetLibrary& lib,
    const TubePassBounds& bounds, std::span<const double> depth,
    Image* spriteTarget = nullptr,
    const Texture* brickTexture = nullptr,
    const CylinderMesh* footprintMesh = nullptr, const CollisionGrid* footprintGrid = nullptr,
    const ActorPoseResult* airbornePose = nullptr) {
  if (!airbornePose && !options.composeSkipActorVisibility &&
      !actorSubmissionVisible(injected.plus8X, injected.plus8Z, injected.renderEnabled, bounds))
    return std::nullopt;
  // Fall back to "walk" when the sheet lacks the actor's clip.
  const Sprite* walkSprite = injected.clip != nullptr ? sprites.find(injected.clip) : nullptr;
  if (walkSprite == nullptr)
    walkSprite = sprites.find("walk");
  if (walkSprite == nullptr)
    return std::nullopt;

  ActorTerrainSample plus8{injected.plus8X, injected.plus8Y, injected.plus8Z};
  ActorPoseResult pose;
  if (airbornePose) {
    pose = *airbornePose;
  } else if (options.composeSkipActorPose) {
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
  gfx::SpriteViewVector basisVec{pose.basis[0], pose.basis[1], pose.basis[2]};
  double phase = options.composeActorPhase + injected.rotationPhase;
  if (walkSprite->name == "dig") {
    // Provisional dig pose uses the smooth cylinder tangent. Native 4159B4
    // sets phase 0.75. The shared view chooser uses the camera angle.
    const double radius = std::hypot(pose.modelPosition[0], pose.modelPosition[2]);
    if (radius > 0.0)
      basisVec = {-pose.modelPosition[2] / radius, 0.0, pose.modelPosition[0] / radius};
    // Centre the front view on the outward normal. Half its 33.75 degree sector is 3/64 turn.
    phase += 3.0 / 64.0;
    // Face leftward diggers into the cylinder. Keep the body rectangle for picking.
    if (spriteTarget && injected.facing == static_cast<int>(sim::Facing::Left))
      phase += 0.5;
  }
  std::size_t temporalIndex = static_cast<std::size_t>(std::max(0, injected.walkTemporalIndex));
  if (injected.clipSeconds >= 0.0) {
    ActorAnimClockState clock;
    clock.loopAtEnd = injected.clipLoops;
    const double duration = 0.06 * walkSprite->animations.size() / walkSprite->rate;
    double seconds = injected.clipSeconds;
    if (duration > 0.0 && std::isfinite(duration)) {
      if (clock.loopAtEnd && seconds > duration) {
        seconds = std::fmod(seconds, duration);
        if (seconds == 0.0) seconds = duration;
      } else if (!clock.loopAtEnd) seconds = std::min(seconds, duration + 0.06);
    }
    temporalIndex = advanceActorAnimClock(clock,
        {int(walkSprite->animations.size()), walkSprite->rate}, seconds).temporalIndex;
  }
  if (temporalIndex >= walkSprite->animations.size())
    return std::nullopt;
  const auto& temporal = walkSprite->animations[temporalIndex];
  const std::size_t dirCount = temporal.frames.size();
  if (dirCount == 0)
    return std::nullopt;

  const auto scores = gfx::spriteViewScores(cameraToActor, basisVec, phase);
  auto selection = gfx::selectSpriteView(scores[0], scores[1], dirCount, false);
  if (injected.forcedViewIndex >= 0)
    selection.index = static_cast<std::size_t>(injected.forcedViewIndex);
  if (injected.forcedMirror >= 0)
    selection.mirrored = injected.forcedMirror != 0;
  if (selection.index >= dirCount)
    return std::nullopt;

  const auto& frame = temporal.frames[selection.index];
  const auto asset = sheetAssetName(frame.sheet);
  const Image* borrowedTexture = nullptr;
  if (auto* cache = activeTextureCache; cache && cache->library == &lib) {
    if (const auto found = cache->images.find(asset); found != cache->images.end()) {
      borrowedTexture = &found->second;
      ++cache->hits;
    }
  }
  Image texture;
  if (!borrowedTexture) {
    auto loaded = loadTexture(lib, asset);
    if (!loaded) return std::nullopt;
    texture = std::move(*loaded);
  }

  const std::array<double, 3> worldPosition{pose.modelPosition[0], pose.modelPosition[1],
                                            pose.modelPosition[2]};
  auto placement = app::actorSpritePlacement(frame, worldPosition, camera, selection.mirrored, -1);
  if (!placement)
    return std::nullopt;
  int spriteGravity = injected.verticalDir;
  if (injected.antiGravTurn >= 0.0 && walkSprite->name == "cartwheel") {
    // The art contains the intermediate rotations. Move its centre around
    // the projected feet over the same half-turn. A return lift reflects
    // the whole sequence about the feet on its source ceiling.
    const double angle = std::numbers::pi * injected.antiGravTurn;
    const double offset = (placement->originY - (placement->top + placement->bottom) * 0.5)
        * (1.0 - std::cos(angle));
    placement->top += offset;
    placement->bottom += offset;
    spriteGravity = -spriteGravity;
  }
  *placement = actorSpriteGravityPlacement(*placement, spriteGravity);

  std::array<std::uint8_t, 3> tint{255, 255, 255};
  if (!light.empty()) {
    // 0x403F40 receives terrain X and file-row Y from actor+0x08/+0x10.
    if (auto looked = actorLightTintAt(light, plus8.x, plus8.y))
      tint = *looked;
  }

  PreparedInjectedWalker prepared;
  prepared.frame = frame;
  prepared.texture = std::move(texture);
  prepared.borrowedTexture = borrowedTexture;
  prepared.placement = *placement;
  prepared.composedPlacement = *placement;
  prepared.mirrored = selection.mirrored;
  prepared.inverted = spriteGravity < 0;
  prepared.tint = tint;
  prepared.worldPosition = worldPosition;
  // The visibility diagnostic bypasses both submission and scene-depth rejection.
  const bool sceneClip = !options.composeSkipActorVisibility && !options.composeSkipActorPose;
  if (spriteTarget && (sceneClip || brickTexture)) {
    auto& sprite = *spriteTarget;
    sprite.width = camera.width;
    sprite.height = camera.height;
    sprite.alpha = AlphaMode::Embedded;
    sprite.rgba.resize(std::size_t(sprite.width) * sprite.height * 4);
    if (brickTexture) {
      // The attachment can expand the bounds beyond the body.
      std::fill(sprite.rgba.begin(), sprite.rgba.end(), 0);
    } else {
      // Share the half-open screen rectangle across sampling, clipping and composition.
      clearSpriteRect(sprite, spriteRect(sprite, *placement));
    }
    const ActorOcclusionPlane plane(camera, worldPosition);
    if (brickTexture)
      drawBuilderBrick(sprite, prepared.composedPlacement, poseLevel, camera, injected,
          *brickTexture, tint, builderPackAnchor(frame, prepared.sheet(), *placement,
                                                selection.mirrored, prepared.inverted),
          prepared.attachmentDepth);
    // The brick depth-tests independently of the hands and the body footprint.
    Image body;
    if (brickTexture) {
      body = sprite;
      std::fill(body.rgba.begin(), body.rgba.end(), 0);
    }
    auto& bodyTarget = brickTexture ? body : sprite;
    if (!drawActorSprite(bodyTarget, frame, prepared.sheet(), *placement, selection.mirrored, tint,
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

  const bool physicalActors = !options.composeSkipActorVisibility && !options.composeSkipActorPose;
  Image sprite, overlay;
  overlay.width = raster.framebuffer().width;
  overlay.height = raster.framebuffer().height;
  overlay.alpha = AlphaMode::Embedded;
  if (!physicalActors)
    overlay.rgba.assign(static_cast<std::size_t>(overlay.width) * overlay.height * 4, 0);

  const auto bounds = tubePassBounds(camera.selectorTurns, cylinderExtent_);
  std::vector<std::size_t> order(options.injectedWalkers.size());
  std::iota(order.rbegin(), order.rend(), 0);
  if (physicalActors) {
    // Sort parallel upright body planes far to near in horizontal view depth.
    // That order also blends additive drowning over bodies behind it.
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
    const auto* brickTexture = builderBrickTexture(*it);
    const bool clipped = physicalActors || brickTexture;
    auto prepared = prepareInjectedWalker(poseLevel_, *sprites, actorLight_, camera,
        options, *it, lib, bounds, raster.depthBuffer(), &sprite,
        brickTexture, &mesh_, haveCollision_ ? &collision_ : nullptr);
    if (!prepared)
      continue;
    if (it->drownFluidKind >= 0 && clipped) {
      // 4043F3..404434 selects ONE/ONE with neutral brightness proportional to
      // height above the fluid bottom. Flush pending actor coverage before blending it.
      if (!physicalActors) {
        compositeActorOverlay(raster, overlay, options.overlayCompositeLegacyNonZero);
        std::fill(overlay.rgba.begin(), overlay.rgba.end(), 0);
      }
      auto add = [&](const SpriteRect& rect, double strength, const ActorOcclusionPlane* plane,
                     double originZ) {
        auto& rgb = raster.mutableFramebuffer().rgb;
        for (int y = rect.top; y < rect.bottom; ++y) for (int x = rect.left; x < rect.right; ++x) {
          const auto p = std::size_t(y) * sprite.width + x;
          if (!actorSpriteAlphaTestPasses(sprite.rgba[p * 4 + 3])) continue;
          const double z = plane ? plane->depthAt(x + .5, y + .5) : originZ;
          if (physicalActors && !raster.mutableActorDepth().submit(p,z,owner)) continue;
          bool changed = false;
          for (int c = 0; c < 3; ++c) {
            auto& channel = rgb[p * 3 + c];
            const auto out = std::min(255, channel + int(std::lround(sprite.rgba[p * 4 + c] * strength)));
            changed |= out != channel;
            channel = out;
          }
          if (changed) raster.coverSceneHighlight(p);
        }
      };
      // The fluid state supplies neutral brightness rather than the light map.
      const auto bodyRect = spriteRect(sprite, prepared->placement);
      clearSpriteRect(sprite, bodyRect);
      const auto grey = std::uint8_t(std::clamp(it->drownBrightness, 0.0, 1.0) * 255.0);
      if (!drawActorSprite(sprite, prepared->frame, prepared->sheet(), prepared->placement,
          prepared->mirrored, {grey, grey, grey}, true, prepared->inverted)) continue;
      const auto pose = actorPoseFromCylinderSample(poseLevel_,
          {it->plus8X, it->plus8Y, it->plus8Z}, it->facing);
      const ActorOcclusionPlane plane(camera, pose.modelPosition);
      if (physicalActors && haveCollision_)
        clipBodyFootprint(sprite,prepared->placement,plane,camera,mesh_,collision_);
      clipActorToScene(sprite, prepared->placement,
          options.fluidDrownSceneDepth.empty() ? raster.depthBuffer() : options.fluidDrownSceneDepth,
          &plane, options.pipeSceneDepth);
      add(bodyRect, 1.0, &plane, 0);

      // 404372 creates Bubbles FX at 5..10 units submerged. Provisional clone
      // feedback uses two rising bubble sprites and smaller early splashes.
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
            const auto effectRect = spriteRect(sprite, *box);
            clearSpriteRect(sprite, effectRect);
            if (!drawActorSprite(sprite, frame, *texture, *box, false)) continue;
            clipActorToScene(sprite, *box, options.fluidDrownSceneDepth.empty() ?
                raster.depthBuffer() : options.fluidDrownSceneDepth);
            add(effectRect, splash ? 0.8 : it->drownBrightness, nullptr,
                actorPhysicalDepth(*box,camera.width));
          }
        }
      }
      continue;
    }
    if (clipped) {
      const auto& p = prepared->composedPlacement;
      const ActorOcclusionPlane plane(camera, prepared->worldPosition);
      for (int y = std::max(0, int(std::ceil(p.top)));
           y < std::min({overlay.height, sprite.height, int(std::ceil(p.bottom))}); ++y)
        for (int x = std::max(0, int(std::ceil(p.left)));
             x < std::min({overlay.width, sprite.width, int(std::ceil(p.right))}); ++x) {
          const auto targetPixel = std::size_t(y) * overlay.width + x;
          const auto pixel = std::size_t(y) * sprite.width + x;
          const auto source = pixel * 4;
          if (!actorSpriteAlphaTestPasses(sprite.rgba[source + 3])) continue;
          const double z = prepared->attachmentDepth.empty() ? plane.depthAt(x+.5,y+.5) :
              prepared->attachmentDepth[pixel];
          if (!physicalActors) {
            std::copy_n(sprite.rgba.begin()+source, 4, overlay.rgba.begin()+targetPixel*4);
          } else if (raster.mutableActorDepth().submit(pixel,z,owner)) {
            // Store accepted body and brick RGB opaquely at their sorted slot before later FX.
            raster.coverSceneHighlight(targetPixel);
            std::copy_n(sprite.rgba.begin()+source, 3, raster.mutableFramebuffer().rgb.begin()+targetPixel*3);
          }
        }
      continue;
    }
    if (!app::drawActorSprite(overlay, prepared->frame, prepared->sheet(), prepared->placement,
                              prepared->mirrored, prepared->tint, true, prepared->inverted)
             .has_value())
      continue;
  }

  if (!physicalActors)
    compositeActorOverlay(raster, overlay, options.overlayCompositeLegacyNonZero);
}

void TubeScene::advanceDeathParticles(std::uint64_t clockMs) {
  deathClockMs_ = clockMs;
  std::erase_if(explosionDecals_, [&](const ExplosionDecal& d) {
    return clockMs >= d.bornMs && clockMs - d.bornMs >= d.lifetimeMs;
  });
  std::erase_if(deathParticles_, [&](const DeathParticle& p) {
    return clockMs >= p.bornMs && (clockMs - p.bornMs) / 1000.0 >= p.lifetime;
  });
  std::erase_if(forcedDeathFlights_, [&](const ForcedDeathFlight& flight) {
    return clockMs >= flight.bornMs && clockMs - flight.bornMs >= kForcedDeathFlightLifetimeMs;
  });
}

void TubeScene::emitForcedDeathFlight(const sim::Lemming& actor, ActorTerrainSample sample,
                                      std::uint64_t clockMs) {
  if (!havePoseLevel_) return;
  rewirePoseLevel();
  const int facing = static_cast<int>(actor.facing);
  const auto launch = actorPoseFromCylinderSample(poseLevel_, sample, facing);
  // 416C49 uses a cylinder-space offset for its direction. Use the central
  // values of the Y/Z offsets and the upward kick from 493473.
  const auto target = actorPoseFromCylinderSample(poseLevel_,
      {sample.x, sample.y + 1.5, sample.z + 2.0}, facing);
  const auto& p = launch.modelPosition;
  const auto& q = target.modelPosition;
  // 496D5B cubes one CRT unit draw and uses a second for the direction.
  // The scene owns this stream so only forced-death visuals consume it.
  const double u = forcedDeathSpinRandom_.unit();
  const double direction = forcedDeathSpinRandom_.unit() < 0.5 ? 1.0 : -1.0;
  forcedDeathFlights_.push_back({sample, launch, {q[0] - p[0], q[1] - p[1] + 10.0, q[2] - p[2]},
                                 clockMs, actor.sheetType, facing, direction * 16.0 * u * u * u});
}

void TubeScene::emitExplosionDecal(const sim::Lemming& lemming,
    std::uint64_t clockMs, int terrainHeight, bool nukeActive) {
  if (explosionTexture_.rgba.empty() || !havePoseLevel_) return;
  rewirePoseLevel();
  const auto origin = terrainSampleFromLemming(lemming, terrainHeight);
  // 408227 adds (1,1.5); 43B31E uses (-5.5,-4.5)..(2.5,3.5).
  constexpr double size = 8.0;
  const double left = origin.x - 4.5, bottom = origin.y - 3.0;
  const double right = left + size, top = bottom + size;
  const double clippedBottom = std::max(0.0, bottom);
  const double clippedTop = std::min(double(mesh_.faceCount), top);
  if (clippedBottom >= clippedTop) return;
  const auto split = [](double lo, double hi, int stride) {
    const double seam = (std::floor(lo / stride) + 1) * stride;
    return std::array{lo, std::min(seam, hi), hi};
  };
  const auto xs = split(left, right, 8);
  const auto ys = split(clippedBottom, clippedTop, 16);
  ExplosionDecal decal;
  decal.bornMs = clockMs;
  // 43B31E reads the level's Nuke flag (4CBB40, set at 444AB1).
  // Sampling it at creation preserves the lifetime of marks already drawn.
  decal.lifetimeMs = nukeActive ? 3000 : 20000;
  for (std::size_t y = 1; y < ys.size(); ++y) {
    if (ys[y] == ys[y - 1]) continue;
    for (std::size_t x = 1; x < xs.size(); ++x) {
      if (xs[x] == xs[x - 1]) continue;
      auto& patch = decal.patches[decal.patchCount++];
      const std::array<std::array<double, 2>, 4> corners{{
          {xs[x - 1], ys[y - 1]}, {xs[x - 1], ys[y]},
          {xs[x], ys[y]}, {xs[x], ys[y - 1]}}};
      for (std::size_t i = 0; i < corners.size(); ++i) {
        // 43FCAA -> 43D56C maps to the cylinder template at Z=0 (inner
        // shell), split at 8x16 seams by 441E26. No terrain is modified.
        const auto [column, row] = corners[i];
        const double wrappedColumn = column - std::floor(column / 256.0) * 256.0;
        const auto p = actorPoseFromCylinderSample(poseLevel_, {wrappedColumn, row, 0}, 1);
        patch.corners[i] = {p.modelPosition[0], p.modelPosition[1], p.modelPosition[2]};
        patch.uv[i] = {(column - left) / size, (top - row) / size};
      }
    }
  }
  // Provisional clone cap counts whole marks (one per explosion). Native
  // 441B17 shares a 128-segment pool. Prepare geometry and textures before drawing.
  constexpr std::size_t maxDecals = 256;
  if (explosionDecals_.size() == maxDecals) explosionDecals_.erase(explosionDecals_.begin());
  explosionDecals_.push_back(decal);
}

void TubeScene::drawExplosionDecals(Raster& raster, const CameraBasis& camera) const {
  if (explosionDecals_.empty()) return;
  SceneRgbaFramebuffer fb;
  fb.width = camera.width;
  fb.height = camera.height;
  fb.rgb = raster.mutableFramebuffer().rgb;
  fb.highlights = raster.highlightPixels();
  // 424290 mode 2 -> 44026F -> 414D80: ONE/ONE. ~ texture flags disable
  // alpha test (47261F). PROVISIONAL clone Less/no-write surface policy:
  // scenery occludes the mark; faded/black texels never occlude later actors.
  const SceneRasterOptions options{SceneFilter::Linear, SceneAddress::Clamp,
      SceneAlphaFunction::Disabled, 0, true, SceneBlendFactor::One,
      SceneBlendFactor::One, SceneDepthFunction::Less, false,
      SceneShade::Flat, SceneRgbOperation::Modulate, SceneAlphaOperation::SelectTexture, 0.5, 0.5};
  for (const auto& decal : explosionDecals_) {
    const auto age = deathClockMs_ >= decal.bornMs ? deathClockMs_ - decal.bornMs : 0;
    if (age >= decal.lifetimeMs) continue;
    // 43B419 gives every vertex the same RGB = 32 * remaining / initial.
    const double fade = 32.0 * (decal.lifetimeMs - age) / decal.lifetimeMs;
    for (std::size_t q = 0; q < decal.patchCount; ++q) {
      const auto& patch = decal.patches[q];
      std::array<SceneRasterVertex, 4> v;
      bool visible = true;
      for (std::size_t i = 0; i < v.size(); ++i) {
        std::array<double, 4> p;
        if (!project(camera, patch.corners[i], p)) {
          visible = false;
          break;
        }
        // 43FA93 temporarily adds 4B15B8 to software bias. Provisional clone
        // conversion separates the mark from the coplanar skin.
        v[i] = {p[0], p[1], p[2] - 0.0012, p[3], {fade, fade, fade, 255},
                patch.uv[i][0], patch.uv[i][1]};
      }
      if (!visible) continue;
      for (const auto ids : {std::array<int,3>{0,1,2}, std::array<int,3>{0,2,3}})
        (void)rasterizeSceneTriangle(fb, raster.mutableDepthBuffer(), explosionTexture_,
            {v[ids[0]],v[ids[1]],v[ids[2]]}, options);
    }
  }
}

void TubeScene::emitMinerAxe(const sim::Lemming& lemming,
    std::uint64_t clockMs, int terrainHeight) {
  // READ 418D75..418E5B constructs one Axe only after a protected-cell
  // refusal. Normal mine SPR frames already include the held pickaxe.
  if (axeSprites_.find("axe") == nullptr) return;
  advanceDeathParticles(clockMs);
  deathClockMs_ = clockMs;
  constexpr std::size_t maxParticles = 4096;
  if (deathParticles_.size() >= maxParticles) deathParticles_.erase(deathParticles_.begin());
  DeathParticle p;
  p.origin = terrainSampleFromLemming(lemming, terrainHeight);
  p.verticalDir = lemming.verticalDir < 0 ? -1 : 1;
  p.facing = int(lemming.facing);
  // PROVISIONAL clone-local ballistic adapter. Native 49356B uses random
  // (6*r, 12+5*r, 6*r) transformed by the emitter basis; 419C50 advances
  // position/velocity. Do not transplant its model units into terrain cells.
  p.origin.y += p.verticalDir;
  p.vx = p.facing * 3.0;
  p.vy = p.verticalDir * 12.0;
  p.gravity = -p.verticalDir * 32.0;
  p.bornMs = clockMs;
  // 418EC9..418EDA -> 419590 sets base life 4.5, variation 0;
  // 419B70 stores it in child+30, 419C50 subtracts elapsed seconds.
  p.lifetime = 4.5;
  p.minerAxe = true;
  deathParticles_.push_back(p);
}

std::uint32_t TubeScene::terrainDebrisMaterial(
    const std::array<int, 2>& cell, int terrainHeight) const {
  const int row = terrainHeight - 1 - cell[1];
  return cellMesh_.buckets > unsigned(cell[0] / 8) &&
      cellMesh_.bands > unsigned(row / 16) ?
      cellMesh_.at(cell[0] / 8, row / 16)->material :
      std::uint32_t(cell[0] / 32 + cell[1] / 32 * 8);
}

static std::array<double, 4> terrainChunkUv(int x, int row, int index) {
  const double u = ((x & 31) + (index & 1) * 0.5) / 32.0;
  const double v = (32.0 - (row & 31) - (index >> 1) * 0.5) / 32.0;
  return {u, v - 0.5 / 32.0, u + 0.5 / 32.0, v};
}

static constexpr std::array<std::uint8_t, 3> kTerrainChunkFallback{128, 104, 72};

void TubeScene::emitTerrainDebris(const sim::Lemming& lemming,
    std::span<const std::array<int, 2>> removedCells, std::uint64_t clockMs, int terrainHeight) {
  if (removedCells.empty()) return;
  advanceDeathParticles(clockMs);
  deathClockMs_ = clockMs;
  const int gravityDir = lemming.verticalDir < 0 ? -1 : 1;
  const int facing = static_cast<int>(lemming.facing);
  // Bash 40C1C5 (passes 0/3), mine 418B69 (rows -1/+1) and dig 415BE8
  // construct 40C960. 40DBF0 selects velocity mode 4; 40C9B5 sets 12 slots.
  // Provisional clone arcs use four chunks per removed cell and local
  // deterministic gravity. Digger debris fans both ways.
  for (const auto& cell : removedCells) {
    constexpr std::size_t maxParticles = 4096;
    if (deathParticles_.size() > maxParticles - 4)
      deathParticles_.erase(deathParticles_.begin(), deathParticles_.begin() +
          (deathParticles_.size() - (maxParticles - 4)));
    const auto material = terrainDebrisMaterial(cell, terrainHeight);
    const int row = terrainHeight - 1 - cell[1];
    for (int i = 0; i < 4; ++i) {
      DeathParticle p;
      p.origin = {double(cell[0]), double(row) + 0.5,
                  gravityDir < 0 ? 0.55 : 0.75};
      p.vx = lemming.state == sim::State::Digger ? (i - 1.5) * 3.0 : -facing * (3.0 + i);
      p.vy = gravityDir * (9.0 + i * 1.5);
      p.vz = (i - 1.5) * 0.04;
      p.gravity = -gravityDir * 16.0;
      p.bornMs = clockMs;
      p.lifetime = 1.6 + i * 0.1;
      p.size = 0.65 + (i % 2) * 0.2;
      p.colour = kTerrainChunkFallback;
      p.verticalDir = gravityDir;
      p.terrainMaterial = material;
      p.terrainUv = terrainChunkUv(cell[0], row, i);
      // Deterministic variation is presentation-only; never consume World RNG.
      const unsigned seed = unsigned(cell[0]) * 73u + unsigned(cell[1]) * 151u + i * 47u;
      p.setChunkSpin(seed, i);
      deathParticles_.push_back(p);
    }
  }
}

void TubeScene::emitDeathParticles(const sim::Lemming& lemming, bool explosion,
    std::span<const std::array<int, 2>> removedCells, std::uint64_t clockMs,
    int terrainHeight, const AssetLibrary& lib, bool nukeActive) {
  advanceDeathParticles(clockMs);
  if (explosion) emitExplosionDecal(lemming, clockMs, terrainHeight, nukeActive);
  if (!deathSpritesLoaded_) {
    deathSpritesLoaded_ = true;
    if (auto bits = loadSpriteSheet(lib, "SPRITES\\BITS.SPR")) deathSprites_ = std::move(*bits);
  }
  // 408227 and 40B3EC create six pieces. 401C50 selects Bits clips
  // 0,1,2,2,3,3 for normal lemmings. Provisional clone ballistics use
  // 32 terrain units/s^2, no collision, 0.55..1.6 s life and local RNG.
  auto origin = terrainSampleFromLemming(lemming, terrainHeight,
                                         lemming.verticalDir < 0 ? 0.55 : 0.75);
  const int gravityDir = lemming.verticalDir < 0 ? -1 : 1;
  origin.y += gravityDir * (explosion ? 3.0 : 1.0);
  const auto fan = [&](int index, int count, double spread, double rise) {
    const double angle = (index + 0.5) * 2.0 * std::numbers::pi / count;
    return std::array{std::cos(angle) * spread, gravityDir * (rise + std::sin(angle) * rise * 0.35),
                      std::sin(angle * 3.0) * 0.10};
  };
  const auto add = [&](ActorTerrainSample at, int index, int clip,
                       std::array<std::uint8_t, 3> colour, double life, double size,
                       const std::array<double, 3>& velocity) -> DeathParticle& {
    DeathParticle p;
    p.origin = at;
    p.vx = velocity[0];
    p.vy = velocity[1];
    p.vz = velocity[2];
    p.gravity = -gravityDir * 32.0;
    p.bornMs = clockMs;
    p.lifetime = life;
    p.size = size;
    p.colour = colour;
    p.piece = clip;
    p.verticalDir = gravityDir;
    const unsigned seed = unsigned(int(at.x)) * 73u + unsigned(int(at.y)) * 151u + index * 47u;
    p.setChunkSpin(seed, index);
    deathParticles_.push_back(p);
    return deathParticles_.back();
  };
  // Evict the oldest particles at capacity so each new death keeps its full burst.
  constexpr std::size_t maxParticles = 4096;
  if (deathParticles_.size() > maxParticles - 160)
    deathParticles_.erase(deathParticles_.begin(), deathParticles_.begin() +
        (deathParticles_.size() - (maxParticles - 160)));
  constexpr int clips[]{0, 1, 2, 2, 3, 3};
  for (int i = 0; i < 6; ++i)
    add(origin, i, clips[i], {255, 255, 255}, 1.6, 2.0,
        fan(i, 6, explosion ? 15.0 : 12.0, explosion ? 17.0 : 10.0));
  // READ 43F510/40B3EC and 492E33: the 100 red blast particles and
  // 10 pink splat particles share a point-list renderer. Use small tumbling
  // solid flecks as a provisional clone presentation for both bursts.
  const int count = explosion ? 100 : 10;
  // The blast scatters in three axes. Keep the generator local to this burst
  // so camera, scenery and simulation cannot affect its launch or palette.
  CrtRand burstRandom(std::uint32_t(clockMs) ^ (unsigned(lemming.x) * 73u + unsigned(lemming.y) * 151u));
  for (int i = 0; i < count; ++i) {
    std::array<double, 3> velocity;
    std::array<std::uint8_t, 3> colour;
    if (explosion) {
      velocity = {36.0 * (burstRandom.unit() - .5),
                  gravityDir * (5.0 + 14.0 * burstRandom.unit()),
                  0.32 * (burstRandom.unit() - .5)};
      colour = {std::uint8_t(96.0 + 159.0 * burstRandom.unit()),
                std::uint8_t(16.0 + 16.0 * burstRandom.unit()),
                std::uint8_t(16.0 + 16.0 * burstRandom.unit())};
    } else {
      velocity = fan(i, count, 8.0 + (i % 11), 5.0);
      colour = {255, 127, 127};
    }
    add(origin, i, -1, colour, 0.55 + (i % 7) * 0.07, 0.33 + (i % 4) * 0.07, velocity);
  }

  if (!explosion || removedCells.empty()) return;
  const auto debris = [&](const std::array<int, 2>& cell) {
    const int row = terrainHeight - 1 - cell[1];
    const auto material = terrainDebrisMaterial(cell, terrainHeight);
    for (int i = 0; i < 4; ++i) {
      auto& p = add({double(cell[0]), double(row), origin.z}, i, -1,
          kTerrainChunkFallback, 1.2 + i * 0.1, 0.85 + (i % 2) * 0.20,
          fan(i, 4, 9.0 + i, 12.0));
      p.terrainMaterial = material;
      p.terrainUv = terrainChunkUv(cell[0], row, i);
    }
  };
  bool emitted = false;
  // 43F510 emits at eligible 4x4 sites. The clone uses four fragments
  // per removed cell. At 40DBF0, 4 selects the velocity mode.
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
                                   const AssetLibrary& lib, std::span<const double> pipeDepth) const {
  profile::Scope profileParticles(profile::Particles);
  // No allocations, asset reads, depth copies or scans on an empty frame.
  if ((deathParticles_.empty() && forcedDeathFlights_.empty()) || !havePoseLevel_) return;
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
    const auto pose = actorPoseFromCylinderSample(poseLevel_, sample, p.facing);
    const Sprite* particleClip = p.minerAxe ? axeSprites_.find("axe") :
        (p.piece >= 0 && std::size_t(p.piece) < deathSprites_.sprites.size() ?
            &deathSprites_.sprites[p.piece] : nullptr);
    if (particleClip) {
      const auto& clip = *particleClip;
      if (!clip.animations.empty()) {
        // PROVISIONAL: use stored strip rate when positive, otherwise 12 fps.
        const double rate = clip.rate > 0 && std::isfinite(clip.rate) ? clip.rate : 12.0;
        const double seconds = (deathClockMs_ - p.bornMs) / 1000.0;
        // Axe uses the 60 ms strip clock (419780 -> 496C84) with provisional
        // phase and spin axis. Both effects advance on level time.
        const auto index = std::size_t(seconds * rate / (p.minerAxe ? .06 : 1.0)) %
            clip.animations.size();
        const auto& entry = clip.animations[index];
        if (!entry.frames.empty()) {
          gfx::SpriteViewSelection selection{};
          if (p.minerAxe) {
            const auto scores = gfx::spriteViewScores(
                {pose.modelPosition[0]-camera.eyeWorld[0],pose.modelPosition[1]-camera.eyeWorld[1],
                 pose.modelPosition[2]-camera.eyeWorld[2]},
                {pose.basis[0],pose.basis[1],pose.basis[2]},0.0);
            selection = gfx::selectSpriteView(scores[0],scores[1],entry.frames.size(),false);
          }
          if (selection.index >= entry.frames.size()) continue;
          auto frame = entry.frames[selection.index];
          auto texture = loadTexture(lib, sheetAssetName(frame.sheet));
          auto box = actorSpritePlacement(frame, pose.modelPosition, camera, selection.mirrored, -1);
          if (texture && box) {
            *box = actorSpriteGravityPlacement(*box, p.verticalDir);
            if (p.minerAxe && (frame.uRight > 1.0 || frame.uBottom > 1.0)) {
              // Convert the 16 pixel-UV records in AXE.SPR using atlas dimensions.
              // The other 24 records are normalized.
              frame.uLeft = double(frame.left) / texture->width;
              frame.uRight = double(frame.right) / texture->width;
              frame.uTop = double(frame.top) / texture->height;
              frame.uBottom = double(frame.bottom) / texture->height;
            }
            std::fill(overlay.rgba.begin(), overlay.rgba.end(), 0);
            auto colour = p.colour;
            if (p.minerAxe) {
              if (auto light = actorLightTintAt(actorLight_, sample.x, sample.y))
                colour = *light;
            }
            const ActorOcclusionPlane plane(camera,pose.modelPosition);
            const auto drawn = drawActorSprite(overlay, frame, *texture, *box,
                selection.mirrored, colour, true, p.verticalDir < 0);
            if (drawn &&
                clipActorToScene(overlay, *box, depth, p.minerAxe ? &plane : nullptr)) {
              if (p.minerAxe && haveCollision_)
                clipBodyFootprint(overlay,*box,plane,camera,mesh_,collision_);
              const double z = actorPhysicalDepth(*box,camera.width);
              for (int y = std::max(0,int(std::ceil(box->top)));
                   y < std::min(buffer.height,int(std::ceil(box->bottom))); ++y)
                for (int x = std::max(0,int(std::ceil(box->left)));
                     x < std::min(buffer.width,int(std::ceil(box->right))); ++x) {
                  const auto at = std::size_t(y)*buffer.width+x;
                  auto& alpha = overlay.rgba[at*4+3];
                  const double pixelZ = p.minerAxe ? plane.depthAt(x+.5,y+.5) : z;
                  if (actorSpriteAlphaTestPasses(alpha) && !actors.submit(at,pixelZ,owner)) alpha=0;
                }
              compositeActorOverlay(raster, overlay, false);
            }
            continue;
          }
        }
      }
    }
    if (p.minerAxe) continue; // The axe requires its sprite art.

    // Rigid rotation in model units: rotating the slab parameter itself
    // would stretch the piece because one slab is the whole tube thickness.
    const Vec3 tangent{pose.basis[0], pose.basis[1], pose.basis[2]};
    const Vec3 radial{-tangent.z, 0, tangent.x};
    const auto offsets = p.chunkOffsets(deathClockMs_);
    std::array<Vec3, 8> model;
    std::array<std::array<double, 4>, 8> projected;
    bool visible = true;
    for (int j = 0; j < 8; ++j) {
      const auto& d = offsets[j];
      model[j] = {pose.modelPosition[0] + tangent.x*d[0] + radial.x*d[2],
                  pose.modelPosition[1] + d[1],
                  pose.modelPosition[2] + tangent.z*d[0] + radial.z*d[2]};
      visible &= project(camera, model[j], projected[j]);
    }
    if (!visible) continue;
    const auto* tile = p.terrainMaterial
        ? findTile(true, *p.terrainMaterial % 8, *p.terrainMaterial / 8)
        : nullptr;
    const bool textured = tile && !tile->texture.empty();
    static const Texture white{1, 1, {255, 255, 255}};
    const auto& texture = textured ? tile->texture : white;
    const auto tint = textured ? std::array{1.0, 1.0, 1.0}
        : std::array{p.colour[0] / 255.0, p.colour[1] / 255.0, p.colour[2] / 255.0};
    const auto& u = p.terrainUv;
    const std::array<std::array<double, 2>, 4> uv{{{u[0],u[1]},{u[0],u[3]},
                                                 {u[2],u[3]},{u[2],u[1]}}};
    constexpr int faces[6][4]{{0,2,3,1},{4,5,7,6},{0,1,5,4},
                             {2,6,7,3},{0,4,6,2},{1,3,7,5}};
    for (const auto& face : faces) {
      const auto& a = model[face[0]];
      const auto& b = model[face[1]];
      const auto& c = model[face[2]];
      const Vec3 ab{b.x-a.x,b.y-a.y,b.z-a.z}, ac{c.x-a.x,c.y-a.y,c.z-a.z};
      const Vec3 n{ab.y*ac.z-ab.z*ac.y,ab.z*ac.x-ab.x*ac.z,ab.x*ac.y-ab.y*ac.x};
      const double length = std::sqrt(n.x*n.x+n.y*n.y+n.z*n.z);
      // Provisional fixed ambient/directional light makes tumbling legible.
      const double shade = 0.5 + 0.5 * std::max(0.0,(-0.3*n.x+0.8*n.y-0.5*n.z) /
                                                  std::max(length,1e-12));
      for (const auto& ids : {std::array{0,1,2},std::array{0,2,3}}) {
        std::array<std::array<double,4>,3> triangle;
        std::array<std::array<double,2>,3> texcoords;
        for (int k = 0; k < 3; ++k) {
          triangle[k] = projected[face[ids[k]]];
          texcoords[k] = uv[ids[k]];
        }
        raster.triangle(triangle, texcoords, texture,
                        {shade * tint[0], shade * tint[1], shade * tint[2]}, owner);
      }
    }
  }

  SceneOptions flightOptions;
  flightOptions.pipeSceneDepth = pipeDepth;
  for (std::size_t i = 0; i < forcedDeathFlights_.size(); ++i) {
    const auto& flight = forcedDeathFlights_[i];
    const auto* sprites = lemmingSprites(flight.sheetType);
    if (!sprites || deathClockMs_ < flight.bornMs) continue;
    const double seconds = (deathClockMs_ - flight.bornMs) / 1000.0;
    auto pose = flight.launchPose;
    for (std::size_t axis = 0; axis < flight.velocity.size(); ++axis)
      pose.modelPosition[axis] += flight.velocity[axis] * seconds;
    // Provisional continuous approximation of 4178B0's per-update -1400*dt²
    // velocity change, using 60 Hz as a reference. World gravity is always down.
    pose.modelPosition[1] -= 0.5 * (1400.0 / 60.0) * seconds * seconds;

    SceneOptions::InjectedNativeWalker sprite;
    sprite.plus8X = flight.sample.x;
    sprite.plus8Y = flight.sample.y;
    sprite.plus8Z = flight.sample.z;
    sprite.facing = flight.facing;
    // Advance the cartwheel strip alongside the 496C84 view phase, in turns.
    sprite.clip = "cartwheel";
    sprite.clipSeconds = seconds;
    sprite.clipLoops = true;
    const double phase = flight.spinTurnsPerSecond * seconds;
    sprite.rotationPhase = phase - std::floor(phase);
    const auto prepared = prepareInjectedWalker(poseLevel_, *sprites, actorLight_, camera,
        flightOptions, sprite, lib, bounds, depth, &overlay,
        nullptr, &mesh_, haveCollision_ ? &collision_ : nullptr, &pose);
    if (!prepared) continue;
    const ActorOcclusionPlane plane(camera, pose.modelPosition);
    const auto rect = spriteRect(overlay, prepared->placement);
    const auto owner = ActorDepth::noOwner / 2 + deathParticles_.size() + i;
    for (int y = rect.top; y < rect.bottom; ++y)
      for (int x = rect.left; x < rect.right; ++x) {
        const auto pixel = std::size_t(y) * buffer.width + x;
        const auto* rgba = overlay.rgba.data() + pixel * 4;
        if (actorSpriteAlphaTestPasses(rgba[3]) &&
            actors.submit(pixel, plane.depthAt(x + .5, y + .5), owner)) {
          raster.coverSceneHighlight(pixel);
          std::copy_n(rgba, 3, buffer.rgb.data() + pixel * 3);
        }
      }
  }
}

bool TubeScene::drawHoveredWalker(Image& target, std::size_t walker, const CameraBasis& camera,
    const SceneOptions& options, const AssetLibrary& lib, const Raster& sceneRaster) const {
  if (walker >= options.injectedWalkers.size() || !havePoseLevel_ ||
      target.width != camera.width || target.height != camera.height)
    return false;
  const auto& injected = options.injectedWalkers[walker];
  const auto* sprites = lemmingSprites(injected.sheetType);
  if (!sprites) return false;
  rewirePoseLevel();

  // 454ACA..454B5C copies the body quad and uses ONE/ONE with neutral RGB 255.
  // Keep the clone's physical depth and body mask for the late redraw.
  Image sprite;
  const auto bounds = tubePassBounds(camera.selectorTurns, cylinderExtent_);
  const auto prepared = prepareInjectedWalker(poseLevel_, *sprites, ActorLightMap{}, camera,
      options, injected, lib, bounds, sceneRaster.depthBuffer(), &sprite,
      nullptr, &mesh_, haveCollision_ ? &collision_ : nullptr);
  if (!prepared || sprite.rgba.empty()) return false;

  const ActorOcclusionPlane plane(camera, prepared->worldPosition);
  const auto& actorDepth = sceneRaster.actorDepth();
  const auto rect = spriteRect(sprite, prepared->placement);
  bool marked = false;
  for (int y = rect.top; y < rect.bottom; ++y) {
    for (int x = rect.left; x < rect.right; ++x) {
      const auto pixel = std::size_t(y) * sprite.width + x;
      const auto at = pixel * 4;
      if (!actorSpriteAlphaTestPasses(sprite.rgba[at + 3]) ||
          !actorDepth.owns(pixel, walker) ||
          actorDepth.depthAt(pixel) != plane.depthAt(x + .5, y + .5)) continue;
      addHoverRgb(target.rgba.data() + at, sprite.rgba.data() + at);
      marked = true;
    }
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
          options, options.injectedWalkers[i], lib, bounds, sceneRaster.depthBuffer()))
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
      if (tile == nullptr)
        continue;
      std::array<std::array<double, 2>, 4> uv = {{{0, 0}, {0, 1}, {1, 1}, {1, 0}}};
      const std::array<double, 3> mod = {1.0, 1.0, 1.0};
      for (const auto& ids : {std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3}}) {
        std::array<std::array<double, 4>, 3> p{};
        std::array<std::array<double, 2>, 3> t{};
        for (int i = 0; i < 3; ++i) {
          p[i] = projected[ids[i]];
          t[i] = uv[ids[i]];
        }
        // Deferred raster commands borrow this scene-owned texture through flush.
        raster.triangle(p, t, tile->texture, mod);
      }
    }
  };

  const auto drawWall = [&](bool outer) {
    for (int band = 0; band < bands; ++band) {
      // Reverse tile rows relative to band order. Each band uses half the tile's V span.
      const int row = options.tileRowsFlipped ? (bands - 1 - band) / bandsPerRow
                                              : band / bandsPerRow;
      const int withinRow = options.tileRowsFlipped ? (bands - 1 - band) % bandsPerRow
                                                    : band % bandsPerRow;
      for (int step = 0; step < steps; ++step) {
        const int column = step / stepsPerCol;
        if (!options.fullWall && haveCollision_) {
          // The solid-only wall needs at least one solid collision cell in the
          // corresponding 8-column by 16-row block of the 256 x 96 grid.
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
    // 0x43A4EC centres the bucket range from the camera turn fraction
    // (`0x43c990 -> fsub 0.5, fmul 32.0`). Bracket it by the extent and
    // 0.7 of the extent, truncate with 0x49C5AC, then shift together by 32s.
    const auto [centre, a, b, mid, c, d] = tubePassBounds(camera.selectorTurns, cylinderExtent_);
    // `0x462465` compares each quad's child coordinate against the centre
    // bucket in eighths, i.e. the camera's own collision column.
    const double centreColumn = centre * 8.0;

    // 0x460A2D and 0x43AB3A use the cell descriptor as material.
    // Material +24 selects the inner bank. Tile coordinates are (material % 8, material / 8).
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
      SceneHighlightId highlight;
      if (quad.gate && !raster.highlightPixels().empty()) {
        for (const auto vertex : quad.index) {
          if (vertex < counterGateVertexOwners_.size() &&
              counterGateVertexOwners_[vertex].kind == SceneHighlightKind::CounterGate) {
            highlight = counterGateVertexOwners_[vertex];
            break;
          }
        }
      }
      for (const auto& ids : {std::array<int, 3>{0, 1, 2}, std::array<int, 3>{0, 2, 3}}) {
        std::array<std::array<double, 4>, 3> p{};
        std::array<std::array<double, 2>, 3> t{};
        for (int i = 0; i < 3; ++i) {
          p[i] = projected[ids[i]];
          t[i] = quad.uv[ids[i]];
        }
        raster.triangle(p, t, tile->texture, modulate, ActorDepth::noOwner, tile->routeAddress, highlight);
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

// Missing keys use bilinear cylinder vertices (0x43A31D, 0x43D2B8).
// The native per-block scan (0x4615E9) tests kCellFlag0x400, the gate and door stamp bit.
// FUN_0043f510 clears cells; its mesh handoff is unresolved.
// Provisional clone skill edits rebuild touched blocks with a one-cell
// margin at sync time. Builder cells use the block material.
void TubeScene::syncTerrain(const sim::Terrain& terrain, std::span<const sim::CounterGate> gates) {
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
      bool now = terrain.solid(x, h - 1 - y);
      const bool gateCell = std::any_of(gates.begin(), gates.end(),
          [&](const auto& g) { return g.containsCell(x, y, w); });
      // Gates move the existing surface pool, independently of the collision
      // mask (0x42727C/0x426FEC). Keep their closed topology so opening can
      // retract it to the inner cylinder instead of deleting/re-creating it.
      if (gateCell) now = !(collision_.at(x, y) & kCellNotLandable);
      std::uint8_t& was = meshedSolid_[static_cast<std::size_t>(y) * w + x];
      if (now == (was != 0))
        continue;
      was = now ? 1 : 0;
      ++changed;
      collision_.cells[static_cast<std::size_t>(y) * w + x] =
          gateCell ? (collision_.at(x, y) | 0x403u) : now ? kCellSolid : 0u;
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
  // Supply corner keys (x0..x0+8, y0..y0+16) plus a margin for neighbour reads.
  for (const auto& [bucket, band] : dirty) {
    const int x0 = bucket * kColumnsPerBlock;
    const int y0 = band * kRowsPerBlock;
    for (int layer = 0; layer < 2; ++layer)
      for (int y = std::max(0, y0 - 2); y <= std::min(h, y0 + kRowsPerBlock + 2); ++y)
        for (int x = std::max(0, x0 - 2); x <= std::min(w, x0 + kColumnsPerBlock + 2); ++x) {
          const std::size_t key = static_cast<std::size_t>(layer) + (x + y * 0x101) * 2;
          if (key >= editKeys_.size() || editKeys_[key] != 0xFFFFFFFFu)
            continue;
          // 43D2B8 clamps with coordinate height world+28 (112, 43D2F9/43D334).
          const auto v = cylinderTemplateVertex(mesh_, x, y, layer, static_cast<int>(mesh_.faceCount));
          if (!v)
            continue;
          editKeys_[key] = static_cast<std::uint32_t>(mesh_.surface.size());
          mesh_.surface.push_back(*v);
          ++terrainVerticesAdded_;
        }
  }

  const std::vector<std::pair<int, int>> blocks(dirty.begin(), dirty.end());
  if (auto status = rebuildCellMeshCells(cellMesh_, editKeys_, counterGateCoordinates_, collision_,
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

void TubeScene::syncCounterGates(std::span<const sim::CounterGate> gates) {
  if (gates.empty()) return;
  std::vector<double> fractions;
  for (const auto& g : gates) fractions.push_back(g.surfaceFraction());
  if (fractions == counterGateFractions_ && counterGateMeshedBlocks_ == terrainBlocksRebuilt_) return;
  counterGateFractions_ = fractions;
  counterGateMeshedBlocks_ = terrainBlocksRebuilt_;
  ++counterGateRevision_;
  counterGateVertexOwners_.assign(mesh_.surface.size(), {});
  for (std::size_t i = 0; i < gates.size(); ++i) {
    const auto& g = gates[i];
    const auto inside = [&](int x, int y) {
      return y >= g.bottom && y <= g.top &&
          collision_.width > 0 && (x - g.left + collision_.width) % collision_.width <= g.right - g.left;
    };
    const auto place = [&](std::uint32_t index, int x, int y) {
      if (index >= mesh_.surface.size() || !inside(x, y)) return;
      const auto a = cylinderTemplateVertex(mesh_, x, y, 0, static_cast<int>(mesh_.faceCount));
      const auto b = cylinderTemplateVertex(mesh_, x, y, 1, static_cast<int>(mesh_.faceCount));
      if (!a || !b) return;
      mesh_.surface[index] = {a->x + (b->x - a->x) * fractions[i],
                              a->y + (b->y - a->y) * fractions[i],
                              a->z + (b->z - a->z) * fractions[i]};
      counterGateVertexOwners_[index] = {SceneHighlightKind::CounterGate, static_cast<std::uint32_t>(i)};
    };
    // The original binds these exact extra-vertex indices. They can alias
    // keys used by several faces: mutate the shared vertex, not a decal.
    for (const auto& t : counterGateCoordinates_)
      place(t[2], static_cast<int>(t[0]), static_cast<int>(t[1]));
  }
}

void TubeScene::renderCounterGates(Raster& raster, const CameraBasis& camera,
    const AssetLibrary& lib, std::span<const sim::CounterGate> gates) {
  if (std::none_of(gates.begin(), gates.end(), [](const auto& g) { return g.hasCounter(); })) return;
  if (!counterTexturesLoaded_) {
    counterTexturesLoaded_ = true;
    for (int i = 0; i < 11; ++i) {
      const auto name = i == 10 ? std::string("TEXTURES\\^LIMIT_PANEL.BMP") :
                                std::format("TEXTURES\\_LIMIT{}.BMP", i);
      if (auto image = loadTexture(lib, name))
        counterTextures_[i] = {image->width, image->height, std::move(image->rgba)};
      else missing_.push_back(name + ": " + image.error());
    }
  }
  rewirePoseLevel();
  SceneRgbaFramebuffer fb;
  fb.width = camera.width; fb.height = camera.height; fb.rgb = raster.mutableFramebuffer().rgb;
  fb.highlights = raster.highlightPixels();
  SceneRasterOptions options{SceneFilter::Linear, SceneAddress::Clamp,
      SceneAlphaFunction::StrictGreater, 0, true, SceneBlendFactor::SrcAlpha,
      SceneBlendFactor::InvSrcAlpha, SceneDepthFunction::Less, true,
      SceneShade::Gouraud, SceneRgbOperation::Modulate, SceneAlphaOperation::SelectTexture, 0.5, 0.5};
  const auto quad = [&](double x, double y, double w, double h, int texture, double slab) {
    const auto& image = counterTextures_[texture];
    if (image.rgba.empty()) return;
    std::array<SceneRasterVertex, 4> v;
    const std::array<std::array<double, 2>, 4> uv{{{0,1},{1,1},{1,0},{0,0}}};
    for (int i = 0; i < 4; ++i) {
      const auto at = actorPoseFromCylinderSample(poseLevel_,
          {x + (i == 1 || i == 2 ? w : 0.0), y + (i >= 2 ? h : 0.0), slab}, 1);
      std::array<double, 4> p;
      if (!project(camera, {at.modelPosition[0], at.modelPosition[1], at.modelPosition[2]}, p)) return;
      // PROVISIONAL clone decal separation. The original's inherited device
      // depth state is unproved; offset the cylinder slab slightly outward.
      v[i] = {p[0], p[1], p[2], p[3], {255,255,255,255}, uv[i][0], uv[i][1]};
    }
    for (const auto ids : {std::array<int,3>{0,1,2}, std::array<int,3>{0,2,3}})
      (void)rasterizeSceneTriangle(fb, raster.mutableDepthBuffer(), image,
          {v[ids[0]],v[ids[1]],v[ids[2]]}, options);
  };
  for (std::uint32_t i = 0; i < gates.size(); ++i) {
    const auto& g = gates[i];
    if (!g.hasCounter()) continue;
    options.highlight = {SceneHighlightKind::CounterGate, i};
    options.highlightTextureRgb = true;
    // 0x423E69: panel 8.2x6.1; digits 2.1x3.5 separated by 2.7.
    quad(g.counterX, g.counterY - 1.3, 8.2, 6.1, 10, 1.005);
    quad(g.counterX + 1.3, g.counterY + 0.35, 2.1, 3.5, g.remaining() / 10, 1.01);
    quad(g.counterX + 4.0, g.counterY + 0.35, 2.1, 3.5, g.remaining() % 10, 1.01);
  }
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
    walker.renderEnabled = lemming.state != sim::State::Splatter && !lemming.ratVictim;
    walker.plus8X = sample.x;
    walker.plus8Y = sample.y;
    walker.plus8Z = sample.z;
    if (motion) {
      const auto drawn = motion->positionOf(i, world);
      walker.plus8X = drawn.x;
      walker.plus8Y = terrainHeight - 1.0 - drawn.y;
      // Reuse the displayed radial lane for placement, occlusion and picking.
      walker.plus8Z = drawn.z;
    }
    walker.facing = static_cast<int>(lemming.facing);
    walker.verticalDir = lemming.verticalDir;
    walker.sheetType = lemming.sheetType;
    // Use the tracked clip and entry, or walk entry 0 without a tracker.
    if (anim != nullptr) {
      const LemmingAnimPose pose = anim->poseOf(i);
      walker.clip = pose.clip;
      walker.walkTemporalIndex = pose.temporalIndex;
      walker.drownSeconds = pose.drownSeconds;
      // READ 40E20C/40E247: block entry sets controller phase to 0.25;
      // 40E0DF/40E12D resets it. A fresh render sample restores zero on leave.
      if (lemming.state == sim::State::Blocker)
        walker.rotationPhase = 0.25;
      // READ 4159B4/4159DA -> 4683E9: dig entry sets phase 0.75;
      // 415995/4159A9 resets it. Simulation walking facing stays untouched.
      if (lemming.state == sim::State::Digger)
        walker.rotationPhase = 0.75;
      if (lemming.state == sim::State::Builder) {
        walker.builderPhase = builderPresentationPhase(world, lemming);
        walker.builderX = lemming.x;
        walker.builderY = terrainHeight - 1 - lemming.y;
        walker.builderLength = world.rules().brickLength;
        // Sample the hands and brick at the same phase.
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
          // 404655..404675 uses Y -= verticalDir * 20 * dt / 2.
          // Provisional clone pose clamps at the receiver bottom. The original
          // removes type 0x23 there; World still owns the clone's death timing.
          walker.plus8Y = std::clamp(row - lemming.verticalDir * 10.0 * pose.drownSeconds,
              g.minY, g.minY + g.height);
          walker.drownBrightness = std::clamp((walker.plus8Y - g.minY) /
              (g.high > 0.0 ? g.high : g.height), 0.0, 1.0);
          break;
        }
      }
    }
    if (lemming.state == sim::State::AntiGrav) {
      walker.antiGravTurn = lemming.antiGravTurn;
      walker.clip = lemmingClipName(lemming);
      // The installed 16-entry cartwheel completes its first half at entry 8.
      // Retiming that half to the lift is provisional. The native controller
      // runs its own clip timer alongside the fall (41AE95).
      walker.walkTemporalIndex = static_cast<int>(std::lround(8.0 * walker.antiGravTurn));
    }
    out.push_back(walker);
  }
  // Append noninteractive exit tails after live actors to preserve pick indices.
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
  // Rats share actor rendering and stay outside the pickable prefix.
  constexpr const char* punches[]{"punch0", "punch1", "punch2", "punch3"};
  const auto& rats = world.rats();
  for (std::size_t i = 0; i < rats.size(); ++i) {
    const auto& rat = rats[i];
    const auto& l = rat.actor;
    if (!l.alive()) continue;
    const auto drawn = motion ? motion->ratPositionOf(i, world) :
        LemmingDrawPosition{static_cast<double>(l.x), static_cast<double>(l.y), rat.lane};
    SceneOptions::InjectedNativeWalker w;
    w.plus8X = drawn.x;
    w.plus8Y = terrainHeight - 1.0 - drawn.y;
    w.plus8Z = drawn.z;
    w.facing = int(l.facing);
    w.verticalDir = l.verticalDir;
    w.sheetType = 3;
    w.renderEnabled = rat.renderEnabled && l.state != sim::State::Splatter;
    w.clip = lemmingClipName(l);
    w.clipSeconds = rat.animationSeconds;
    w.clipLoops = lemmingClipLoops(w.clip);
    if (l.state == sim::State::Floater && rat.animationSeconds > rat.openDuration) {
      w.clip = "float";
      w.clipSeconds -= rat.openDuration;
      w.clipLoops = true;
    }
    if (rat.pendingVictim >= 0) {
      w.clip = punches[rat.punch];
      w.clipLoops = false;
    }
    out.push_back(w);
  }
  return out;
}

}  // namespace webo::app
