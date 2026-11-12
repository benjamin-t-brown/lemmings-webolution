#pragma once

#include "lemmings/world_geometry_prefix.hpp"
#include <span>

namespace lemmings {

// Original world +0x28 and the 0x600-byte rows at +0x18/+0x0C.
// Each row stores 32 inner points followed by 32 outer points.
struct WorldCoordinateMesh {
    std::int32_t height = 0;
    std::span<const WorldGeometryPrefix::TripleBlock> rows;
};

// New view adapter over the already recovered serialized fields.
WorldCoordinateMesh world_coordinate_mesh(const WorldGeometryPrefix& world);

// 0x0043D56C: interpolate radial depth, X sector, then Y row, in that order.
// Requires height >= 16, enough rows and a Y selecting a valid row. Invalid
// mesh/index inputs throw; the original would access memory unchecked.
std::array<double,3> map_world_coordinates(
    const WorldCoordinateMesh& mesh, const std::array<double,3>& point);

// 0x0043DA38: depth at a new Y relative to the old Y's radial segment.
// This is not a terrain collision test. Degenerate segments divide by zero.
double world_depth_at_y(const WorldCoordinateMesh& mesh,
                        const std::array<double,3>& point, double new_y);

} // namespace lemmings
