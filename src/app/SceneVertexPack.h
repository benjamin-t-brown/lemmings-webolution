// Explicit-input packing for one already-projected/clipped scene vertex.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>

namespace webo::app {

struct SceneVertexPackInput {
  double screenX;
  double screenY;
  double screenZ;
  std::array<double, 4> diffuse;  // raw channel0, channel1, channel2, alpha
  double u;
  double v;
  double materialUOffset;
  double materialVOffset;
};

// Writes known fields of the final 32-byte little-endian record. Bytes +0x14..+0x17
// (specular) are intentionally untouched. This is a software component contract,
// not a claim about native x87 precision/control-word or GPU behavior.
// Inputs must be finite; values and truncated diffuse channels must be representable
// as finite binary32 / signed 32-bit integers, and rounded screenZ must be nonzero.
std::expected<void, std::string> packSceneVertex(
    const SceneVertexPackInput& input, std::span<std::byte, 32> destination);

}  // namespace webo::app
