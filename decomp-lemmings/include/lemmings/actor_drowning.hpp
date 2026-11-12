#pragma once

#include "lemmings/actor_update.hpp"

namespace lemmings {
struct ActorDrowningState { double elapsed_08 = 0; };
struct ActorDrowningHooks {
    void* context = nullptr;
    void (*select_animation_42fd25)(void*, const char* name) = nullptr;
    void (*advance_model_2c)(void*, double elapsed) = nullptr;
};
// 0x408700: elapsed=0, phase=0, extra=0, type=0x22.
void initialize_actor_drowning(ActorDrowningState&, ActorBehavior&);
// 0x40874C selects the literal animation name "drown".
void enter_actor_drowning(const ActorDrowningHooks&);
// 0x408766 advances the inline model, rereads elapsed/remaining, accumulates,
// clears remaining to positive zero, and returns 2 only above 2.0 seconds.
std::uint32_t update_actor_drowning(ActorDrowningState&, double& remaining,
                                    const ActorDrowningHooks&);
void leave_actor_drowning(); // Shared empty behavior method 0x401320.
} // namespace lemmings
