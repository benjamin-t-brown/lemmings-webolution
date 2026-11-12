#include "lemmings/actor_drowning.hpp"

namespace lemmings {
void initialize_actor_drowning(ActorDrowningState& state, ActorBehavior& behavior) {
    state.elapsed_08 = 0.0;
    behavior.phase_10 = 0; behavior.extra_11 = 0; behavior.type_14 = 0x22;
}
void enter_actor_drowning(const ActorDrowningHooks& hooks) {
    hooks.select_animation_42fd25(hooks.context, "drown");
}
std::uint32_t update_actor_drowning(ActorDrowningState& state, double& remaining,
                                    const ActorDrowningHooks& hooks) {
    hooks.advance_model_2c(hooks.context, remaining);
    volatile double elapsed = static_cast<double>(static_cast<long double>(state.elapsed_08) + remaining);
    state.elapsed_08 = elapsed;
    remaining = 0.0;
    return state.elapsed_08 > 2.0 ? 2 : 0;
}
void leave_actor_drowning() {}
} // namespace lemmings
