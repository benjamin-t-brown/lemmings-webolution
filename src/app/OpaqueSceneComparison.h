#pragma once

#include "app/PreparedLevelFrame.h"
#include "app/PresentationBackend.h"

#include <filesystem>
#include <span>

namespace webo::app {

struct OpaqueDraw {
  std::array<SceneRasterVertex, 3> vertices;
  SceneRasterOptions material;
  const SceneRgbaImage* backdrop = nullptr;
  const PreparedRasterTriangle* tube = nullptr;
  std::uint64_t source = 0;
  std::uint64_t ordinal = 0;
  std::string_view materialName;
  std::string_view kind;
  const BackdropStats::SpriteSubmission* sprite = nullptr;
};

OpaqueDraw opaqueRasterDraw(const PreparedRasterTriangle&, bool depthTest);

struct OpaqueDrawRange {
  // Node offset, or a tube block's bucket, band and bank. Ranges are in
  // submission order. An identity may occur in several disjoint ranges.
  std::uint64_t source;
  int band, bank;
  std::span<const OpaqueDraw> draws;
};

struct OpaqueGpuReadback {
  Image colour;
  std::vector<std::uint32_t> owners;
  std::vector<float> depth;
  double milliseconds = 0;
};

struct OpaqueUpload {
  std::uint64_t source;
  int band, bank;
  std::size_t triangles, vertexBytes, indexBytes;
};

struct SceneDepthSnapshots {
  std::vector<float> pipes, beforeFluid, afterFluid;
};

struct GlActorPixel {
  double depth = std::numeric_limits<double>::infinity();
  std::size_t owner = ActorDepth::noOwner;
  enum class Kind { Scene, Body, Brick, Drowning, Bubble, Particle } kind = Kind::Scene;
};

struct GlActorReadback {
  Image colour;
  std::vector<GlActorPixel> actor;
  std::vector<double> sceneDepth, pipeDepth, drowningDepth;
  double milliseconds = 0;
};

struct GlPointQuery {
  GlActorPixel pixel;
  double milliseconds;
};

struct GlHatchQueries {
  std::vector<HatchHoverTarget> targets;
  double readbackMilliseconds = 0;
};

class OpaqueGpu {
 public:
  virtual ~OpaqueGpu() = default;
  virtual std::string_view precision() const = 0;
  virtual std::string_view driver() const = 0;
  virtual std::expected<void, std::string> begin(int width, int height,
                                               double minimum, double maximum, bool trackHighlights = false) = 0;
  virtual std::expected<std::vector<OpaqueUpload>, std::string> draw(
      LevelScenePass pass, std::span<const OpaqueDrawRange>) = 0;
  virtual std::expected<OpaqueGpuReadback, std::string> read() = 0;
  virtual std::expected<SceneDepthSnapshots, std::string> readDepthSnapshots() = 0;
  virtual std::expected<void, std::string> drawActors(const CameraBasis&,
      const TubeScene::PreparedActors&, bool patchFluidDepth) = 0;
  virtual std::expected<void, std::string> drawParticles(const CameraBasis&,
      const TubeScene::PreparedParticles&) = 0;
  virtual std::expected<GlActorReadback, std::string> readActors(bool readDrowningDepth = false) = 0;
  virtual std::expected<GlHatchQueries, std::string> readHatches(
      std::span<const PreparedHatchQuery>) = 0;
  virtual std::expected<GlPointQuery, std::string> readPoint(int x, int y) = 0;
  virtual std::expected<SceneHighlightId, std::string> readHighlight(int x, int y) = 0;
  virtual std::expected<void, std::string> clipSensor(std::span<const SensorFragment>,
                                                     std::vector<HoverFragment>&) = 0;
  virtual std::expected<bool, std::string> markActor(const CameraBasis&,
      const TubeScene::PreparedActors&, std::size_t walker, bool patchFluidDepth, bool needCoverage) = 0;
  virtual GlSceneImage sceneImage() const = 0;
  virtual void end() = 0;
};

std::expected<std::unique_ptr<OpaqueGpu>, std::string> createOpaqueGpu();

class OpaqueSceneComparison {
 public:
  OpaqueSceneComparison(std::unique_ptr<OpaqueGpu> gpu, std::filesystem::path directory)
      : gpu_(std::move(gpu)), directory_(std::move(directory)) {}

  std::expected<void, std::string> draw(const PreparedLevelScene&, std::string_view level,
      int frame, std::uint64_t levelClockMs, std::size_t terrainCellsChanged,
      std::size_t terrainBlocksRebuilt, bool checkpoint,
      const DisplayedFrameQueries* softwareQueries = nullptr, int pointerX = -1, int pointerY = -1);

  bool hasDisplayedFrame() const { return displayed_; }
  auto actorBoxes() const { return std::span(boxes_); }
  auto hatches() const { return std::span(hatches_); }
  std::expected<GlPointQuery, std::string> pointDepth(int x, int y) { return gpu_->readPoint(x, y); }
  GlSceneImage sceneImage() const { return gpu_->sceneImage(); }
  auto readBounds(std::span<const PreparedHatchQuery> queries) { return gpu_->readHatches(queries); }
  auto readHighlight(int x, int y) { return gpu_->readHighlight(x, y); }
  auto clipSensor(std::span<const SensorFragment> candidates, std::vector<HoverFragment>& output) {
    return gpu_->clipSensor(candidates, output);
  }
  auto markActor(const PreparedLevelScene& scene, std::size_t walker, bool needCoverage) {
    return gpu_->markActor(scene.camera, scene.actors, walker, scene.hasFluidDrowning, needCoverage);
  }

 private:
  std::unique_ptr<OpaqueGpu> gpu_;
  std::filesystem::path directory_, levelDirectory_;
  bool displayed_ = false;
  std::vector<std::optional<ActorSpritePlacement>> boxes_;
  std::vector<HatchHoverTarget> hatches_;
};

}  // namespace webo::app
