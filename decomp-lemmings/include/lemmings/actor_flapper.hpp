#pragma once

#include "lemmings/actor_falling.hpp"
#include "lemmings/actor_update.hpp"

namespace lemmings {

// Name comes from the original diagnostic strings. Type 0x20, vtable 4B0A58.
// Logical views: suffixes identify original offsets, not host member offsets.
struct ActorFlapperState { double elapsed_08 = 0; };
struct FlapperActor : FallingActor { std::int16_t identifier_3a = 0; };
struct ActorFlapperCallbacks {
    virtual ~ActorFlapperCallbacks() = default;
    virtual void notice_enter(std::int16_t id, std::uint32_t context_4cbf88) = 0;
    virtual void select_animation_42fd25(FlapperActor&, const char*) = 0;
    virtual void advance_model_2c(FlapperActor&, double) = 0;
    virtual std::uint8_t has_state_42ecc9(FlapperActor&, std::int32_t) = 0;
    virtual std::uint8_t finished_model_44(FlapperActor&) = 0;
    // Combined allocation of 0x18 bytes and constructor 4081B0; null is passed
    // on to attach exactly as in the original allocation-failure branch.
    virtual ActorBehavior* create_state_4081b0() = 0;
    virtual long double elapsed_model_48(FlapperActor&) = 0;
    virtual void notice_finished(std::int16_t id, std::uint32_t context_4cbf88,
                                  double remaining) = 0;
    virtual void attach_42e696(FlapperActor&, ActorBehavior*) = 0;
};

// 416910. Operations/context remain caller bindings; original vtable identity
// is recorded above. Initializes only elapsed, phase, extra flag and type.
void initialize_actor_flapper(ActorFlapperState&, ActorBehavior&);
void enter_actor_flapper(FlapperActor&, const std::uint32_t& context_4cbf88,
                          ActorFlapperCallbacks&); // 41695C

// 41699E. Empty terrain allows falling unless state 0 is present. A finished
// model supplies a new time budget, attaches 4081B0 and moves raw Y one direction
// unit. Callbacks may change live fields/time/context between these operations.
// Requires a valid grid and masked FP exceptions. Callback exception unwinding
// and aliases between independent state, actor and remaining fields are unclaimed.
std::uint32_t update_actor_flapper(ActorFlapperState&, FlapperActor&, double& remaining,
    CollisionGridHead&, const std::uint32_t& context_4cbf88, ActorFlapperCallbacks&);
void leave_actor_flapper(); // 401320: no side effects.

} // namespace lemmings
