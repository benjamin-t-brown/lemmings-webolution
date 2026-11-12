#pragma once

#include "lemmings/actor_shadow.hpp"

namespace lemmings {

// 0x457F40, including the pure quad/index/UV/color/alpha initialization chain.
// Mutates an existing logical view: the original does NOT initialize vertices,
// visible_3ad, intensity_3c8, object padding, or corner padding. Default member
// values in ActorShadow are conveniences for callers, not original ctor writes.
// The actor pointer is stored without dereferencing it; null is accepted.
ActorShadow& initialize_actor_shadow(ActorShadow& shadow, ShadowActor* actor);

} // namespace lemmings
