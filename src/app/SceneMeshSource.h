#pragma once

#include "assets/SceneNode.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace webo {

// Raw source-pose candidate. This is not a draw command: no visibility, pass,
// texture-object, clipping, device-state, or raster decision is made here.
struct SceneMeshCornerSource {
  std::size_t sourceOffset = 0;
  std::uint32_t vertexIndex = 0;
  std::array<double, 3> localPosition{};
  std::array<double, 3> worldPosition{};
  std::array<double, 3> viewPosition{};
  std::array<double, 3> localNormal{};
  std::uint32_t normalIndex = 0;
  // Original corner values remain linked even when another alias wins.
  std::array<double, 6> sourceAttributes{}; // UV, RGBA
  std::array<double, 6> attributes{};       // per-record last-store winner
};

struct SceneMeshRecordSource {
  std::size_t nodeSourceOffset = 0;
  std::size_t blockSourceOffset = 0;
  std::size_t recordSourceOffset = 0;
  std::uint32_t kind = 0;
  std::uint32_t materialSlot = 0;
  std::string materialName; // empty remains a logical empty name, not a texture handle
  std::uint32_t blendSelector = 0;
  std::vector<SceneMeshCornerSource> corners;
};

// Matrices use the accepted 12-double row-vector affine representation:
// out[j]=sum(in[i]*m[3*i+j])+m[9+j]. Required caller matrices, no defaults.
std::expected<std::vector<SceneMeshRecordSource>, std::string> buildSceneMeshSource(
    const SceneNode& root, const std::array<double, 12>& parentWorld,
    const std::array<double, 12>& view);

}  // namespace webo
