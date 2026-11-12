#pragma once

#include "lemmings/world_geometry_prefix.hpp"
#include "lemmings/collision_grid.hpp"

namespace lemmings {

// New ownership adapter for the recovered world +0x418/+0x41C fields.
// Views are obtained after copying/moving the storage to avoid stale pointers.
struct CollisionGridStorage {
    std::int32_t height = 0;
    std::vector<std::int32_t> cells;
    CollisionGridHead view();
};

// 0x439FD3 reads following_value through 0x43AA63 (height), then loads
// second_integer_array directly into the collision array at +0x41C.
CollisionGridStorage collision_grid_from_level(const WorldGeometryPrefix& world);

} // namespace lemmings
