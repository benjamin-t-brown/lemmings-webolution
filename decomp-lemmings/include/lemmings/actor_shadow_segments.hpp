#pragma once

#include "lemmings/collision_grid.hpp"

namespace lemmings {

// 0x459557's only effectful boundaries: resizing its caller's point list and
// obtaining a mutable entry. A bridge may use the original list handle.
struct ActorShadowSegmentList {
    virtual ~ActorShadowSegmentList() = default;
    virtual void resize_421260(std::int32_t count) = 0;
    virtual std::array<double, 3>& point_at_421240(std::int32_t index) = 0;
};

// 0x459557. Snapshot effect position +3B0, inspect the three integer columns
// around truncated X, and write terrain segment points with Z=0. The wrapped
// column is compared with the unwrapped bounds exactly as in the executable.
// Input list contents are retained when the signed loop does not execute.
// Requires a positive valid grid, masked FP exceptions, terminating X wrapping
// and signed iteration, valid callback entries, and nonthrowing callbacks.
void build_actor_shadow_segments(const std::array<double, 3>& position_3b0,
    double left, double right, CollisionGridHead& grid, ActorShadowSegmentList& list);

} // namespace lemmings
