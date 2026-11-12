#pragma once
#include "lemmings/actor_geometry.hpp"
#include "lemmings/gadget_bounds.hpp"
#include <span>

namespace lemmings {
struct GadgetSurfaceView {
    const GadgetBoundsHead* bounds = nullptr;
    // Receiver +158 points to a model; its +118 array contains 24-byte points.
    std::span<const std::array<double,3>> model_points;
};
// 0x42C714. Requires a valid bounds pointer and at least six model points.
// Preserves the extended result; a zero X extent follows floating division.
long double gadget_surface_y(const GadgetSurfaceView&, double x);
// 0x42C78F. contact_bounds is actor +54, which can differ from the receiver.
// Samples below the actor first, then at the actor, and moves sample Y on hit.
bool adjust_actor_gadget_contact(const GadgetSurfaceView& receiver,
    const GadgetBoundsHead& contact_bounds, ActorGeometryState& actor,
    const CollisionGridHead& grid);
} // namespace lemmings
