#include "lemmings/actor_spawn_monitor.hpp"
#include "lemmings/cylinder_coordinate.hpp"

#include <bit>
#include <cmath>

namespace lemmings {
namespace {
std::int32_t ftol_low(long double value) {
    if (!std::isfinite(value) || value < -9223372036854775808.0L || value >= 9223372036854775808.0L) return 0;
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
}
double rounded(long double value) { volatile double result = static_cast<double>(value); return result; }
bool ordered_unequal(double first, double second) { return !std::isnan(first) && !std::isnan(second) && first != second; }
}
void initialize_actor_spawn_monitor(ActorSpawnMonitorState& state, ActorBehavior& behavior) {
    state.elapsed_08 = 0;
    behavior.phase_10 = 1; behavior.extra_11 = 1; behavior.type_14 = 0x2f;
}
void enter_actor_spawn_monitor() {}
void leave_actor_spawn_monitor() {}
std::array<double, 3> actor_monitor_sample_position(const ActorGeometryState& actor) {
    return {rounded(static_cast<long double>(ftol_low(actor.position[0])) + actor.pair_360[0]),
            rounded(static_cast<long double>(ftol_low(actor.position[1])) + actor.pair_360[1]),
            actor.position[2]};
}
bool monitor_pairs_differ(const std::array<double, 2>& first, const std::array<double, 2>& second) {
    return ordered_unequal(first[0], second[0]) || ordered_unequal(first[1], second[1]);
}
std::uint32_t update_actor_spawn_monitor(SpawnMonitorActor& actor, double& remaining,
    CollisionGridHead& grid, void* request_context, ActorSpawnMonitorCallbacks& callbacks) {
    const auto sample = actor_monitor_sample_position(actor.geometry);
    const auto x = ftol_low(wrap_cylinder_x(static_cast<double>(ftol_low(sample[0]))));
    const auto shifted_y = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(ftol_low(sample[1])) -
                                                    static_cast<std::uint32_t>(actor.geometry.direction_44));
    const auto y = ftol_low(clamp_cylinder_y(grid, static_cast<double>(shifted_y)));
    remaining = 0.0;
    if (!callbacks.can_continue_42f6ec(actor)) return 2;
    const std::array<double, 2> pair{static_cast<double>(x), static_cast<double>(y)};
    const auto cell = static_cast<std::uint32_t>(*collision_cell(grid, static_cast<std::uint32_t>(x), y));
    if (cell & 0x40) {
        bool found = false;
        for (std::int32_t index = 0; index < callbacks.region_count_4157f0(); ++index) {
            if (callbacks.region_contains_420a52(index, {pair[0], pair[1], 0.0})) { found = true; break; }
        }
        const auto type = found ? 0x2e : 0x22;
        if (!callbacks.has_state_42ecc9(actor, type)) {
            if (!found) callbacks.clear_primary_42e725(actor);
            auto* state = callbacks.create_state(found ? SpawnMonitorTransition::state_406c20 : SpawnMonitorTransition::state_408700);
            callbacks.attach_state_42e696(actor, state);
        }
    } else if ((cell & 0x100) && monitor_pairs_differ(pair, actor.last_pair_68)) {
        auto* request = callbacks.create_request_415710(request_context);
        request->pair_10 = pair;
        callbacks.enqueue_request_43f0c0(*request);
        actor.last_pair_68 = pair;
        actor.flag_5f = 1;
    }
    return 0;
}
std::uint32_t clear_spawn_monitor_request(const SpawnMonitorRequest& request,
    CollisionGridHead& grid, const SpawnMonitorClearHooks& hooks) {
    *collision_cell(grid, static_cast<std::uint32_t>(ftol_low(request.pair_10[0])), ftol_low(request.pair_10[1])) = 0;
    const auto x = ftol_low(request.pair_10[0]); const auto y = ftol_low(request.pair_10[1]);
    hooks.notify_43b0a1(hooks.context, x, y, x, y);
    return 0;
}
} // namespace lemmings
