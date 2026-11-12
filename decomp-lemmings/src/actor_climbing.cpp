#include "lemmings/actor_climbing.hpp"
#include "lemmings/cylinder_coordinate.hpp"
#include <bit>
#include <cmath>

namespace lemmings {
namespace {
std::int32_t integer_low(long double value) {
    constexpr long double limit = 9223372036854775808.0L;
    if (!(value >= -limit && value < limit)) return 0;
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
}
}
void enter_actor_climbing(ClimbingActor& actor, const ActorClimbingHooks& hooks) {
    hooks.set_animation(hooks.context, actor, "climb");
}
double climbing_boundary_time(const ActorClimbingState& state, const ClimbingActor& actor) {
    const double y = actor.geometry.position[1];
    auto integer = integer_low(y);
    double result;
    if (actor.geometry.direction_44 == 1) {
        integer = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(integer) + 1u);
        result = static_cast<double>((static_cast<long double>(integer) - y) / 5.0L);
    } else result = static_cast<double>((static_cast<long double>(y) - integer) / 5.0L);
    if (state.reached_boundary_18 && (result == 0 || std::isnan(result))) result = 0.2;
    return result;
}
std::int32_t resolve_actor_climbing_collision(ClimbingActor& actor, CollisionGridHead& grid,
                                             const ActorClimbingHooks& hooks) {
    auto& geometry = actor.geometry;
    const auto point = actor_sample_position_350(geometry);
    auto x = integer_low(wrap_cylinder_x(static_cast<double>(static_cast<long double>(point[0]) - geometry.direction_40)));
    auto y = integer_low(point[1]);
    const auto first_y = integer_low(clamp_cylinder_y(grid, static_cast<double>(y)));
    const auto first_x = integer_low(wrap_cylinder_x(static_cast<double>(x)));
    const auto cell = *collision_cell(grid, static_cast<std::uint32_t>(first_x), first_y);
    if (cell != 0 || y > grid.height) {
        geometry.position[0] = static_cast<double>(wrap_cylinder_x(
            static_cast<double>(static_cast<long double>(geometry.position[0]) - geometry.direction_40)));
        hooks.notice(hooks.context, actor, ClimbingNotice::hit_head);
        reverse_actor_direction_40(geometry);
        hooks.attach_state(hooks.context, actor, hooks.create_state(hooks.context, ClimbingTransition::falling_415fa0));
        return 1;
    }
    x = integer_low(point[0]);
    y = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(integer_low(point[1])) -
                                   static_cast<std::uint32_t>(geometry.direction_44));
    y = integer_low(clamp_cylinder_y(grid, static_cast<double>(y)));
    if (*collision_cell(grid, static_cast<std::uint32_t>(x), y) == 0) {
        hooks.notice(hooks.context, actor, ClimbingNotice::topping_out);
        hooks.notice(hooks.context, actor, ClimbingNotice::topping_position);
        hooks.attach_state(hooks.context, actor, hooks.create_state(hooks.context, ClimbingTransition::topping_417c80));
        hooks.notice(hooks.context, actor, ClimbingNotice::climb_over);
        return 1;
    }
    return 0;
}
std::int32_t update_actor_climbing(ActorClimbingState& state, ClimbingActor& actor, double& remaining,
                                  CollisionGridHead& grid, const ActorClimbingHooks& hooks) {
    if (actor.mode_50 == 3) remaining = static_cast<double>(static_cast<long double>(remaining) / 2.0L);
    std::int32_t result = 0;
    while (remaining > 0 && result == 0) {
        const double boundary = climbing_boundary_time(state, actor);
        const bool reached = !(boundary > remaining);
        const double step = reached ? boundary : remaining;
        const double y = static_cast<double>(static_cast<long double>(actor.geometry.direction_44) * 5.0L * step + actor.geometry.position[1]);
        actor.geometry.position[1] = static_cast<double>(clamp_cylinder_y(grid, y));
        hooks.advance_animation(hooks.context, actor, step);
        remaining = static_cast<double>(static_cast<long double>(remaining) - step);
        if (reached) {
            hooks.notice(hooks.context, actor, ClimbingNotice::boundary_position);
            result = resolve_actor_climbing_collision(actor, grid, hooks);
            state.reached_boundary_18 = 1;
        } else state.reached_boundary_18 = 0;
    }
    if (result == 0) remaining = 0.0;
    return result;
}
} // namespace lemmings
