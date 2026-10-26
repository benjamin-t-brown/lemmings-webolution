#pragma once

#include "app/Camera.h"
#include "app/SceneBillboardSource.h"
#include "app/SceneRaster.h"

#include <array>

namespace webo::app {

// Software inverse-stage conversion for production's diagonal billboard P/S
// transforms with zero transform translation. A source-space generated Z offset
// is added after source S, so slotZ = k*centerViewZ + finalPz*deltaSourceZ,
// where k=parentPz*sourceSz*finalPz. Dividing by k yields the equivalent
// physical-view comparison depth centerViewZ + deltaSourceZ/(parentPz*sourceSz).
// This supported domain is the accepted diagonal source chain, not arbitrary
// affine matrices or a claim about effective native device depth state. Only
// comparison Z is converted; the source slot and RHW remain native.
double billboardPhysicalComparisonDepth(double nativeSlotZ,
                                         const BillboardCameraChain& chain);

// Production adapter used by BackdropScene's actual billboard submission path.
SceneRasterVertex billboardRasterVertex(const BillboardVertex& slot,
                                        const BillboardCameraChain& chain,
                                        std::array<double, 4> diffuse);

}  // namespace webo::app
