#include "app/SceneVertexPack.h"
#include "app/SceneDiffuse.h"

#include <bit>
#include <cmath>
#include <limits>

namespace webo::app {
namespace {

bool asFiniteF32(double value, float& out) {
  if (!std::isfinite(value) || value > std::numeric_limits<float>::max() ||
      value < -std::numeric_limits<float>::max())
    return false;
  out = static_cast<float>(value);
  return std::isfinite(out);
}

void putU32(std::span<std::byte, 32> dst, std::size_t offset, std::uint32_t v) {
  for (unsigned i = 0; i != 4; ++i)
    dst[offset + i] = std::byte((v >> (i * 8)) & 0xffu);
}

void putF32(std::span<std::byte, 32> dst, std::size_t offset, float value) {
  putU32(dst, offset, std::bit_cast<std::uint32_t>(value));
}

}  // namespace

std::expected<void, std::string> packSceneVertex(
    const SceneVertexPackInput& in, std::span<std::byte, 32> dst) {
  float x, y, z, u, v;
  if (!asFiniteF32(in.screenX, x) || !asFiniteF32(in.screenY, y) ||
      !asFiniteF32(in.screenZ, z) || !asFiniteF32(in.u + in.materialUOffset, u) ||
      !asFiniteF32(in.v + in.materialVOffset, v) || z == 0.0f)
    return std::unexpected("vertex coordinate/UV outside finite binary32 domain");
  const float rhw = 0.25f / z;  // divide using the reloaded binary32 Z
  if (!std::isfinite(rhw)) return std::unexpected("RHW outside finite binary32 domain");
  auto packedDiffuse = packSceneDiffuseWord(in.diffuse);
  if (!packedDiffuse) return std::unexpected(packedDiffuse.error());

  putF32(dst, 0x00, x);
  putF32(dst, 0x04, y);
  putF32(dst, 0x08, z);
  putF32(dst, 0x0c, rhw);
  putU32(dst, 0x10, *packedDiffuse);
  // +0x14 specular deliberately retains the caller's arbitrary bytes.
  putF32(dst, 0x18, u);
  putF32(dst, 0x1c, v);
  return {};
}

}  // namespace webo::app
