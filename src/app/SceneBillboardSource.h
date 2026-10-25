#pragma once

#include "assets/SceneNode.h"

#include <array>
#include <expected>
#include <string>

namespace webo::app {

// Row-major 3x4 affine transform: rows are x, y, z; column 3 is translation.
struct BillboardAffine3x4 {
  std::array<double, 12> values{};
};

struct BillboardSourceInputs {
  // Applied in this order: node matrix, parent/camera, source transform,
  // projection transform. The first three locate the source node; projection
  // transforms generated local billboard points.
  BillboardAffine3x4 parentCamera;
  BillboardAffine3x4 sourceTransform;
  BillboardAffine3x4 projectionTransform;
  std::array<double, 2> center{};
};

struct BillboardVertex {
  double x = 0, y = 0, z = 0;
  double u = 0, v = 0;
};

struct SceneBillboardSource {
  std::size_t sourceOffset = 0;
  std::string sourceName;
  std::string textureName;
  std::uint32_t blendSelector = 0;
  std::uint8_t sourceVisibility = 0;
  std::array<std::array<double, 3>, 4> generatedPoints{};
  // Native storage slots, before any screen clipping or raster work.
  std::array<BillboardVertex, 4> slots{};
  std::array<unsigned, 4> adapterOrder{0, 3, 2, 1};
};

// Produces float64 preclip geometry only. No visibility/cull/pass/state decision.
std::expected<SceneBillboardSource, std::string> makeSceneBillboardSource(
    const SceneNode& node, const BillboardSourceInputs& inputs);

}  // namespace webo::app
