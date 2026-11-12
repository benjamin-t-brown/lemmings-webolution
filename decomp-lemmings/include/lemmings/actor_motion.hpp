#pragma once

#include "lemmings/actor_geometry.hpp"

namespace lemmings {

// 0x0042EEA3: position X = wrap(target sample X - pair_360[0]).
void move_actor_sample_x(ActorGeometryState& actor, double sample_x);

// 0x0042EE74: position Y = clamp(target sample Y - pair_360[1]).
// Original stores the subtraction to binary64 before calling the Y clamp.
void move_actor_sample_y(ActorGeometryState& actor, double sample_y,
                         const CollisionGridHead& grid);

} // namespace lemmings
