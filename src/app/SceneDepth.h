#pragma once

#include "app/Camera.h"
#include "app/SceneBillboardSource.h"
#include "app/SceneRaster.h"

#include <array>

namespace webo::app {

// Convert billboard slot Z to physical view depth for the diagonal,
// zero-translation P/S chain. slotZ = k*centerViewZ + finalPz*deltaSourceZ
// with k=parentPz*sourceSz*finalPz. Divide by k for comparison depth.
// Keep the native slot and RHW for projection and interpolation.
double billboardPhysicalComparisonDepth(double nativeSlotZ,
                                         const BillboardCameraChain& chain);

SceneRasterVertex billboardRasterVertex(const BillboardVertex& slot,
                                        const BillboardCameraChain& chain,
                                        std::array<double, 4> diffuse);

// Decode mesh depth to physical view units using the supplied projection scale and bias.
std::expected<SceneRasterVertex, std::string> adaptPackedSceneMeshVertex(
    double screenX, double screenY, double physicalViewZ,
    double projectionScale, double sourceBias,
    std::array<double, 4> diffuse, double u, double v,
    std::array<double, 2> materialUvOffset = {});

}  // namespace webo::app
