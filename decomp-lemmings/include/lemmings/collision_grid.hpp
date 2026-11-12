#pragma once

#include "lemmings/native_int_array.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace lemmings {

// Known portion of the world object used by the collision-cell accessor.
struct CollisionGridHead {
    std::array<std::byte, 0x418> unknown_prefix;
    std::int32_t height;
    NativeIntArray cells;
};

static_assert(offsetof(CollisionGridHead, height) == 0x418);
#if defined(INTPTR_MAX) && defined(INT32_MAX) && INTPTR_MAX == INT32_MAX
static_assert(offsetof(CollisionGridHead, cells) == 0x41C);
static_assert(offsetof(CollisionGridHead, cells) +
              offsetof(NativeIntArray, data) == 0x428);
#endif

// Recovered from 0x0040DE60, which forwards the result of 0x00404D10.
// X wraps to eight bits. For positive height, Y clamps to [0, height-1].
// The native code performs no storage-size check. Invalid dimensions are
// outside the claimed behavior of this source-level port.
std::int32_t* collision_cell(CollisionGridHead& grid,
                             std::uint32_t x, std::int32_t y);

} // namespace lemmings
