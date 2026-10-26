#include "app/SceneDepth.h"

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

}  // namespace webo::app
