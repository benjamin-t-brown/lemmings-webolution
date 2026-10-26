// Raw diffuse packing shared by the final vertex record and software scene adapter.
#pragma once

#include "app/SceneRaster.h"

#include <array>
#include <cstdint>
#include <expected>
#include <string>

namespace webo::app {

// Software-supported signed-32 input domain; native overflow behavior is not implied.
std::expected<std::uint32_t, std::string> packSceneDiffuseWord(
    const std::array<double, 4>& rawChannels);

// Adapt source order (channel0, channel1, channel2, alpha) through that packed
// word into raster order (R,G,B,A). Other submitted fields pass through unchanged.
std::expected<SceneRasterVertex, std::string> adaptSceneMeshVertex(
    double screenX, double screenY, double screenZ, double rhw,
    double u, double v, const std::array<double, 4>& rawChannels);

}  // namespace webo::app
