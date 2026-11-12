#pragma once

#include "lemmings/actor_geometry.hpp"
#include "lemmings/actor_update.hpp"

namespace lemmings {

enum class ActorLandingTransition {
    state_40b3a0, // Constructor sets type ID 0x1A; original allocation is 0x18 bytes.
    bounce_40ec7e, // Constructor sets type ID 0x21; original allocation is 0x30 bytes.
    landed_41bd60  // Constructor sets type ID 0x12; original allocation is 0x20 bytes.
};
struct ActorLandingHooks {
    void* context = nullptr;
    // Original diagnostic call 0x0040F860, before bounce-state creation.
    void (*bounce_notice)(void*, std::int32_t x, std::int32_t y) = nullptr;
    ActorBehavior* (*create_state)(void*, ActorLandingTransition) = nullptr;
    void (*attach_state)(void*, ActorBehavior*) = nullptr;
    // May delegate to select_actor_landed_state below.
    void (*select_landed_state)(void*) = nullptr;
};

// 0x00416433. Samples actor position + pair_360, truncates BEFORE coordinate
// clamp/wrap, then resolves out-of-world, bounce and ordinary terrain paths.
// distance_18 belongs to the falling behavior; umbrella_49 belongs to actor.
// Positive grid height and correctly sized backing cells are required.
bool resolve_actor_landing(const ActorGeometryState& actor, double distance_18,
                           std::uint8_t umbrella_49, CollisionGridHead& grid,
                           const ActorLandingHooks& hooks);

// 0x0042F8B9 always creates the 0x0041BD60 state and attaches it.
void select_actor_landed_state(const ActorLandingHooks& hooks);

} // namespace lemmings
