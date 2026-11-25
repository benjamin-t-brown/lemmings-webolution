#pragma once

#include "app/BackdropScene.h"
#include "app/FrameProfile.h"
#include "app/PreparedLevelLayers.h"
#include "app/RasterRows.h"
#include "app/TubeScene.h"

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

namespace webo::app {

enum class LevelScenePass {
  Primary, LegacyUnattributed, Tube, TubeBeforeRoute, EarlyActors, TubeRoute,
  GadgetEffects, Pipes, Fluid, Secondary, CounterGates, BlueSky, CompleteDrowningDepth,
  ExplosionMarks, Actors, Particles, TeleportEffects
};

// Geometry and material handles are valid for the owning level leg. Each
// pass names its draw payload and the operation on the scene or auxiliary depth.
struct PreparedLevelScene {
  CameraBasis camera;
  std::vector<LevelScenePass> passes;
  std::shared_ptr<const BackdropScene::PreparedStage> primary;
  BackdropScene::PreparedStage legacy, gadgetEffects, pipes, fluid, secondary, teleportEffects;
  std::shared_ptr<const TubeScene::PreparedTube> tube;
  std::vector<PreparedSceneTriangle> counterGates, explosionMarks;
  TubeScene::PreparedActors actors;
  std::vector<PreparedHatchQuery> hatchQueries;
  TubeScene::PreparedParticles particles;
  bool trackHighlights = false;
  bool cachePose = false;
  bool cacheTube = false;
  bool hasPipes = false;
  bool hasFluidDrowning = false;
};

inline std::vector<LevelScenePass> prepareLevelPassOrder(bool drawActors, bool actorsBeforeRoute,
                                                         bool blueSky, bool fluidDrowning) {
  using Pass = LevelScenePass;
  std::vector<Pass> passes;
  passes.reserve(15);
  passes.insert(passes.end(), {Pass::Primary, Pass::LegacyUnattributed});
  if (actorsBeforeRoute) {
    // The diagnostic actor draw splits the tube at PreparedTube::beforeRoute.
    passes.insert(passes.end(), {Pass::TubeBeforeRoute, Pass::EarlyActors, Pass::TubeRoute});
  } else {
    passes.push_back(Pass::Tube);
  }
  passes.insert(passes.end(), {Pass::GadgetEffects, Pass::Pipes, Pass::Fluid,
                               Pass::Secondary, Pass::CounterGates});
  if (blueSky) passes.push_back(Pass::BlueSky);
  if (fluidDrowning) passes.push_back(Pass::CompleteDrowningDepth);
  passes.push_back(Pass::ExplosionMarks);
  if (drawActors && !actorsBeforeRoute) passes.push_back(Pass::Actors);
  passes.insert(passes.end(), {Pass::Particles, Pass::TeleportEffects});
  return passes;
}

struct PreparedLevelFrame {
  PreparedLevelScene scene;
  PreparedLevelLayers layers;
};

inline constexpr std::array<std::uint8_t, 3> kBlueSkyRgb{4, 8, 24};

inline void fillBlueSky(Raster& target) {
  // One Way Ticket fills untouched black after water blends over its clear.
  auto& rgb = target.mutableFramebuffer().rgb;
  const auto& depth = target.depthBuffer();
  for (std::size_t p = 0; p < depth.size(); ++p) {
    const auto i = 3 * p;
    if (depth[p] != std::numeric_limits<double>::infinity() ||
        rgb[i] != 0 || rgb[i+1] != 0 || rgb[i+2] != 0) continue;
    std::copy(kBlueSkyRgb.begin(), kBlueSkyRgb.end(), rgb.begin() + i);
  }
}

inline void patchDrowningDepth(std::span<double> beforeFluid, std::span<const double> afterFluid,
                               std::span<const double> completedScene) {
  // Later opaque scenery can occlude the additive swimmer through water.
  for (std::size_t i = 0; i < completedScene.size(); ++i)
    if (completedScene[i] != afterFluid[i]) beforeFluid[i] = completedScene[i];
}

// The software consumer owns its cache and intermediate depth planes. All
// simulation, clocks, RNG and audio stay outside this object.
class SoftwareLevelRaster {
 public:
  void draw(Raster& target, const PreparedLevelScene& frame, BackdropStats& stats,
      const SceneRasterObserver* primaryObserver = nullptr,
      const SceneRasterObserver* secondaryObserver = nullptr) {
    const auto& camera = frame.camera;
    pipeDepth_.clear();
    drowningDepth_.clear();
    using Pass = LevelScenePass;
    for (const auto pass : frame.passes) {
      switch (pass) {
        case Pass::Primary:
          target.trackSceneHighlights(false);
          if (frame.primary && primary_.draws == frame.primary && !primaryObserver) {
            target = *primary_.raster;
            stats = primary_.stats;
            break;
          }
          target.clear(0, 0, 0);
          if (frame.primary) {
            BackdropScene::drawStage(target, *frame.primary, stats, primaryObserver);
            if (frame.cachePose && !primaryObserver) {
              primary_.draws = frame.primary;
              primary_.raster = target;
              primary_.stats = stats;
            }
          }
          break;
        case Pass::LegacyUnattributed:
          target.trackSceneHighlights(frame.trackHighlights);
          BackdropScene::drawStage(target, frame.legacy, stats);
          break;
        case Pass::Tube:
          drawTubePass(target, frame);
          break;
        case Pass::TubeBeforeRoute:
          target.setDepthEnabled(frame.tube->depthTest);
          for (const auto& t : std::span(frame.tube->triangles).first(frame.tube->beforeRoute)) t.draw(target);
          break;
        case Pass::EarlyActors:
          TubeScene::drawActors(target, camera, frame.actors);
          break;
        case Pass::TubeRoute:
          for (const auto& t : std::span(frame.tube->triangles).subspan(frame.tube->beforeRoute)) t.draw(target);
          break;
        case Pass::GadgetEffects:
          BackdropScene::drawStage(target, frame.gadgetEffects, stats);
          break;
        case Pass::Pipes:
          if (frame.hasPipes)
            pipeDepth_.assign(target.depthBuffer().size(), std::numeric_limits<double>::infinity());
          BackdropScene::drawStage(target, frame.pipes, stats, nullptr, pipeDepth_);
          break;
        case Pass::Fluid:
          if (frame.hasFluidDrowning) drowningDepth_ = target.depthBuffer();
          BackdropScene::drawStage(target, frame.fluid, stats);
          if (frame.hasFluidDrowning) afterFluidDepth_ = target.depthBuffer();
          break;
        case Pass::Secondary:
          BackdropScene::drawStage(target, frame.secondary, stats, secondaryObserver);
          break;
        case Pass::CounterGates:
          TubeScene::drawSceneTriangles(target, frame.counterGates);
          break;
        case Pass::BlueSky:
          fillBlueSky(target);
          break;
        case Pass::CompleteDrowningDepth:
          patchDrowningDepth(drowningDepth_, afterFluidDepth_, target.depthBuffer());
          break;
        case Pass::ExplosionMarks:
          TubeScene::drawSceneTriangles(target, frame.explosionMarks);
          break;
        case Pass::Actors:
          TubeScene::drawActors(target, camera, frame.actors, drowningDepth_, pipeDepth_);
          break;
        case Pass::Particles:
          TubeScene::drawParticles(target, camera, frame.particles, pipeDepth_);
          break;
        case Pass::TeleportEffects:
          BackdropScene::drawStage(target, frame.teleportEffects, stats);
          break;
      }
    }
  }

  std::span<const double> drowningDepth() const { return drowningDepth_; }
  std::span<const double> pipeDepth() const { return pipeDepth_; }

 private:
  void drawTubePass(Raster& target, const PreparedLevelScene& frame) {
    const auto& camera = frame.camera;
    if (!frame.cacheTube) {
      TubeScene::drawTube(target, *frame.tube);
    } else if (tube_.draws == frame.tube) {
      target.compositeOpaque(*tube_.raster);
    } else if (!frame.cachePose) {
      drawTubeBatch(target, frame);
    } else {
      if (!tube_.raster || tube_.raster->framebuffer().width != camera.width ||
          tube_.raster->framebuffer().height != camera.height)
        tube_.raster.emplace(camera.width, camera.height);
      tube_.raster->trackSceneHighlights(frame.trackHighlights);
      tube_.raster->clear(0, 0, 0);
      tube_.raster->resetDrawCounts();
      drawTubeBatch(*tube_.raster, frame);
      tube_.draws = frame.tube;
      target.compositeOpaque(*tube_.raster);
    }
  }

  static void drawTubeBatch(Raster& target, const PreparedLevelScene& frame) {
    detail::RasterCommandBatch batch(frame.camera.height);
    TubeScene::drawTube(target, *frame.tube);
    profile::Scope rasterTime(profile::TubeRaster);
    batch.flush();
  }

  struct {
    std::shared_ptr<const BackdropScene::PreparedStage> draws;
    std::optional<Raster> raster;
    BackdropStats stats;
  } primary_;
  struct {
    std::shared_ptr<const TubeScene::PreparedTube> draws;
    std::optional<Raster> raster;
  } tube_;
  std::vector<double> pipeDepth_, drowningDepth_, afterFluidDepth_;
};

// Native GL queries keep the completed frame's prepared inputs. Sample only
// the projected query rectangle for mesh bounds, in original screen coordinates.
class NativeLevelQueries {
 public:
  void nextFrame() { fullReady_ = false; }

  const Raster& full(const PreparedLevelScene& frame) {
    if (!fullReady_) {
      if (!full_) full_.emplace(frame.camera.width, frame.camera.height);
      detail::ScopedSerialRaster serial(true);
      BackdropStats stats;
      software_.draw(*full_, frame, stats);
      fullReady_ = true;
    }
    return *full_;
  }

  std::vector<HatchHoverTarget> bounds(const PreparedLevelScene& frame,
      std::span<const PreparedHatchQuery> queries, std::optional<std::pair<int, int>> pointer) {
    std::vector<HatchHoverTarget> targets;
    for (const auto& query : queries) {
      const auto box = query.bounds(frame.camera);
      if (box.left >= box.right || box.top >= box.bottom ||
          (pointer && !pointerOverSceneBounds(pointer->first, pointer->second,
                                             box.left, box.top, box.right, box.bottom))) continue;
      const auto& raster = fullReady_ ? *full_ : depthRegion(frame, box);
      auto visible = BackdropScene::hatchHoverTargets(frame.camera, raster, std::span(&query, 1));
      targets.insert(targets.end(), visible.begin(), visible.end());
    }
    return targets;
  }

 private:
  const Raster& depthRegion(const PreparedLevelScene& frame, PickBox box) {
    if (!region_) region_.emplace(frame.camera.width, frame.camera.height);
    auto& target = *region_;
    target.setClip(box);
    target.clear(0, 0, 0);
    BackdropStats stats;
    const auto draw = [&](const BackdropScene::PreparedStage& stage) {
      BackdropScene::drawStage(target, stage, stats);
    };
    if (frame.primary) draw(*frame.primary);
    draw(frame.legacy);
    TubeScene::drawTube(target, *frame.tube);
    draw(frame.gadgetEffects);
    draw(frame.pipes);
    draw(frame.fluid);
    draw(frame.secondary);
    TubeScene::drawSceneTriangles(target, frame.counterGates);
    return target;
  }

  std::optional<Raster> region_, full_;
  SoftwareLevelRaster software_;
  bool fullReady_ = false;
};

inline Image sceneImage(const Framebuffer& fb) {
  Image image;
  image.width = fb.width;
  image.height = fb.height;
  image.rgba.resize(static_cast<std::size_t>(fb.width) * fb.height * 4);
  for (std::size_t i = 0, n = image.pixelCount(); i < n; ++i) {
    image.rgba[i * 4] = fb.rgb[i * 3];
    image.rgba[i * 4 + 1] = fb.rgb[i * 3 + 1];
    image.rgba[i * 4 + 2] = fb.rgb[i * 3 + 2];
    image.rgba[i * 4 + 3] = 255;
  }
  return image;
}

// Software query token. The leg keeps its target alive and untouched through
// event dispatch, including when preparation and materialization were deferred.
class DisplayedFrameQueries {
 public:
  using Point = std::function<std::pair<std::size_t, double>(int, int)>;
  DisplayedFrameQueries(CameraBasis camera, const Raster& raster)
      : camera_(std::move(camera)), raster_(&raster) {}
  DisplayedFrameQueries(CameraBasis camera, Point point)
      : camera_(std::move(camera)), point_(std::move(point)) {}

  void setSceneResults(std::vector<std::optional<ActorSpritePlacement>> actors,
                       std::vector<HatchHoverTarget> hatches) {
    actors_ = std::move(actors);
    hatches_ = std::move(hatches);
    sceneResultsReady_ = true;
  }
  bool sceneResultsReady() const { return sceneResultsReady_; }

  void useSoftwareRaster(const Raster& raster) {
    raster_ = &raster;
    point_ = {};
    sceneResultsReady_ = false;
    image_ = {};
  }

  const CameraBasis& camera() const { return camera_; }
  std::span<const std::optional<ActorSpritePlacement>> actorBoxes() const { return actors_; }
  std::span<const HatchHoverTarget> hatches() const { return hatches_; }
  std::size_t actorOwnerAt(int x, int y) const {
    if (!pointerOverSceneBounds(x, y, 0, 0, camera_.width, camera_.height)) return ActorDepth::noOwner;
    if (point_) return point_(x, y).first;
    return raster_->actorDepth().ownerAt(std::size_t(y) * camera_.width + x);
  }
  std::optional<double> pointDepth(int x, int y) const {
    if (x < 0 || y < 0 || x >= camera_.width || y >= camera_.height) return std::nullopt;
    if (point_) return point_(x, y).second;
    const auto pixel = std::size_t(y) * camera_.width + x;
    const auto actor = raster_->actorDepth().depthAt(pixel);
    return std::isfinite(actor) ? actor : raster_->depthBuffer()[pixel];
  }
  const Image& composedImage() const {
    if (image_.empty() && raster_) {
      image_ = sceneImage(raster_->framebuffer());
      if (layers_) layers_->draw(image_, raster_->depthBuffer());
    }
    return image_;
  }
  void setLayers(const PreparedLevelLayers& layers) {
    layers_ = &layers;
    image_ = {};
  }
  // Automation holds a reference to this storage before it requests pixels.
  const Image& imageStorage() const { return image_; }
  void setImage(Image image) { image_ = std::move(image); }

 private:
  CameraBasis camera_;
  const Raster* raster_ = nullptr;
  Point point_;
  std::vector<std::optional<ActorSpritePlacement>> actors_;
  std::vector<HatchHoverTarget> hatches_;
  bool sceneResultsReady_ = false;
  const PreparedLevelLayers* layers_ = nullptr;
  mutable Image image_;
};

}  // namespace webo::app
