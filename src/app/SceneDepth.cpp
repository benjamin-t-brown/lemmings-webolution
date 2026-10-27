#include "app/SceneDepth.h"
#include "app/SceneVertexPack.h"

#include <bit>
#include <cmath>

namespace webo::app {

double billboardPhysicalComparisonDepth(double nativeSlotZ,
                                         const BillboardCameraChain& chain) {
  // In the supported diagonal, zero-translation chain, center view depth is
  // multiplied by parent P, source S, then final P. Generated source-space
  // delta is added after S and receives only final P. Inverting slot Z by k
  // therefore preserves that offset's physical equivalent; it is not k times
  // the source delta and is not a fitted function.
  // parentCamera contains the view transform as well as P, so use the explicit
  // diagonal P operand supplied in projectionTransform for the scalar factor.
  const double parentPz = chain.projectionTransform[10];
  const double sourceSz = chain.sourceTransform[10];
  const double finalPz = chain.projectionTransform[10];
  const double coefficient = parentPz * sourceSz * finalPz;
  if (!std::isfinite(nativeSlotZ) || !std::isfinite(coefficient) || coefficient == 0.0)
    return NAN;
  return nativeSlotZ / coefficient;
}

SceneRasterVertex billboardRasterVertex(const BillboardVertex& slot,
                                        const BillboardCameraChain& chain,
                                        std::array<double, 4> diffuse) {
  // RHW intentionally remains derived from the original native slot Z.
  return {slot.x, slot.y, billboardPhysicalComparisonDepth(slot.z, chain),
          0.25 / slot.z, diffuse, slot.u, slot.v};
}

std::expected<SceneRasterVertex, std::string> adaptPackedSceneMeshVertex(
    double x, double y, double z, double scale, double bias,
    std::array<double, 4> diffuse, double u, double v) {
  if (!std::isfinite(scale) || scale == 0.0 || !std::isfinite(bias))
    return std::unexpected("mesh SOURCE projection scale/bias invalid");
  std::array<std::byte, 32> bytes{};
  SceneVertexPackInput input{x, y, scale * z - bias, diffuse, u, v, 0.0, 0.0};
  if (auto packed = packSceneVertex(input, bytes); !packed)
    return std::unexpected(packed.error());
  auto readF32 = [&](std::size_t offset) {
    std::uint32_t bits = 0;
    for (unsigned i = 0; i != 4; ++i)
      bits |= std::uint32_t(std::to_integer<unsigned char>(bytes[offset + i])) << (8 * i);
    return static_cast<double>(std::bit_cast<float>(bits));
  };
  const double nativeZ = readF32(8);
  std::uint32_t word = 0;
  for (unsigned i = 0; i != 4; ++i)
    word |= std::uint32_t(std::to_integer<unsigned char>(bytes[0x10 + i])) << (8 * i);
  const std::array<double, 4> rasterDiffuse{
      static_cast<double>((word >> 16) & 0xffu),
      static_cast<double>((word >> 8) & 0xffu),
      static_cast<double>(word & 0xffu),
      static_cast<double>(word >> 24)};
  return SceneRasterVertex{readF32(0), readF32(4), nativeZ / scale,
                           readF32(12), rasterDiffuse, readF32(24), readF32(28)};
}

}  // namespace webo::app
