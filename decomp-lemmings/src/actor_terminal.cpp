#include "lemmings/actor_terminal.hpp"

#include <bit>
#include <cmath>
#include <limits>

namespace lemmings {
namespace {
double rounded(long double value) { volatile double result = static_cast<double>(value); return result; }
std::uint32_t ftol_low(double value) {
    if (!std::isfinite(value) || static_cast<long double>(value) < -9223372036854775808.0L ||
        static_cast<long double>(value) >= 9223372036854775808.0L) return 0;
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
}
std::array<double, 3> direction(const std::array<double, 3>& position,
                               const std::array<double, 3>& offset,
                               ActorTerminalEntryCallbacks& callbacks) {
    // Native argument evaluation transforms the original position first.
    const auto first = callbacks.world_position_43d56c(position);
    const auto second = callbacks.world_position_43d56c(offset);
    std::array<double, 3> result;
    for (unsigned i = 0; i < 3; ++i)
        result[i] = rounded(static_cast<long double>(second[i]) - first[i]);
    return result;
}
}
void initialize_actor_terminal(ActorTerminalState& state, ActorBehavior& behavior) {
    state.elapsed_08 = 0.0;
    behavior.phase_10 = 0;
    behavior.extra_11 = 0;
    behavior.type_14 = 0x1a;
}
void enter_actor_terminal(TerminalActor& actor, std::int32_t world_height,
                          std::int32_t event_value, ActorTerminalEntryCallbacks& callbacks) {
    auto position = actor.position_08;
    // TEST AH,1 treats unordered like below zero.
    if (!(position[1] >= 0.0)) position[1] = 0.0;
    if (world_height < position[1]) position[1] = world_height;
    auto offset = position;
    const auto shift = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(actor.direction_44) << 2);
    offset[1] = rounded(static_cast<long double>(offset[1]) + shift);
    actor.flag_28 = 0;
    callbacks.event_44535b(actor, actor.identifier_3a, event_value, 0);

    auto* first = callbacks.create_effect(actor.category_4c == 3 ?
        TerminalEffectKind::effect_4072a0 : TerminalEffectKind::effect_4018e0);
    callbacks.set_count(first, 6);
    callbacks.set_position_08(first, position);
    callbacks.set_pair(first, 4.0, 0.5);
    callbacks.set_mode(first, 5);
    callbacks.set_direction_54(first, direction(position, offset, callbacks));
    const auto triple = callbacks.sample_triple_403f40(ftol_low(offset[0]),
                         std::bit_cast<std::int32_t>(ftol_low(offset[1])));
    for (std::int32_t i = 0; i < callbacks.child_count_4018d0(first); ++i) {
        auto* child = callbacks.child_4018b0(first, i);
        callbacks.set_child_triple_403820(child, triple);
    }
    auto* entity = callbacks.create_entity_432080(position, first, 1);
    callbacks.insert_entity_401890(entity);

    auto* second = callbacks.create_effect(TerminalEffectKind::effect_402870);
    callbacks.set_count(second, 10);
    callbacks.set_triple_4017d0(second, {255.0, 127.0, 127.0});
    callbacks.set_position_08(second, offset);
    callbacks.set_pair(second, 4.0, 0.5);
    callbacks.set_mode(second, 5);
    callbacks.set_direction_54(second, direction(position, offset, callbacks));
    entity = callbacks.create_entity_432080(position, second, 1);
    callbacks.insert_entity_401890(entity);
}
std::uint32_t update_actor_terminal(ActorTerminalState& state, TerminalActor& actor,
                                   double& remaining, ActorTerminalUpdateCallbacks& callbacks) {
    callbacks.advance_model_2c(actor, remaining);
    state.elapsed_08 = rounded(static_cast<long double>(state.elapsed_08) + remaining);
    remaining = 0.0;
    return callbacks.finished_model_44(actor) ? 2 : 0;
}
void leave_actor_terminal() {}
} // namespace lemmings
