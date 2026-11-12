#include "lemmings/actor_walking.hpp"
#include "lemmings/cylinder_coordinate.hpp"
#include <bit>
#include <cmath>
#include <limits>

namespace lemmings {
namespace {
std::int32_t integer_low(double value) {
    const long double wide = value;
    if (!(wide >= -9223372036854775808.0L && wide < 9223372036854775808.0L))
        return 0;
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::int64_t>(wide)));
}
}
void enter_actor_walking(WalkingActor& actor, const ActorWalkingHooks& hooks) {
    hooks.set_animation(hooks.context, actor, "walk");
}
double walking_boundary_time(const ActorWalkingState& state, const WalkingActor& actor) {
    const auto x = actor.geometry.position[0];
    auto integer = integer_low(x);
    double result;
    if (actor.geometry.direction_40 == 1) {
        integer = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(integer) + 1u);
        result = static_cast<double>((static_cast<long double>(integer) - x) / 5.0L);
    } else {
        result = static_cast<double>((static_cast<long double>(x) - integer) / 5.0L);
    }
    if (state.reached_boundary_18 && (result == 0 || std::isnan(result))) result = 0.2;
    return result;
}
std::int32_t update_actor_walking(ActorWalkingState& state, WalkingActor& actor,
                                double& remaining, const ActorWalkingHooks& hooks) {
    if (actor.mode_50 == 3) remaining = static_cast<double>(static_cast<long double>(remaining) / 2.0L);
    std::int32_t result = 0;
    while (remaining > 0 && result == 0) {
        const double boundary = walking_boundary_time(state, actor);
        const bool reached = !(boundary > remaining);
        const double step = reached ? boundary : remaining;
        const double x = static_cast<double>(5.0L * step * actor.geometry.direction_40 + actor.geometry.position[0]);
        actor.geometry.position[0] = static_cast<double>(wrap_cylinder_x(x));
        hooks.advance_animation(hooks.context, actor, step);
        remaining = static_cast<double>(static_cast<long double>(remaining) - step);
        if (reached) {
            actor.geometry.position[0] = static_cast<double>(integer_low(actor.geometry.position[0]));
            result = hooks.collision(hooks.context, state, actor);
            state.reached_boundary_18 = 1;
        } else state.reached_boundary_18 = 0;
        hooks.surface(hooks.context, state, actor);
        if (actor.gadget_54 && !hooks.gadget_contact(hooks.context, actor.gadget_54, actor))
            actor.gadget_54 = nullptr;
    }
    if (result == 0) remaining = 0.0;
    hooks.finish(hooks.context, actor);
    return result;
}
void track_actor_walking_surface(WalkingActor& actor, CollisionGridHead& grid,
                                const ActorWalkingSurfaceHooks& hooks) {
    auto point = actor_sample_position_360(actor.geometry);
    actor.special_4a = 0;
    point[1] = static_cast<double>(static_cast<long double>(point[1]) - actor.geometry.direction_44);
    point[0] = static_cast<double>(static_cast<long double>(point[0]) +
                                  (actor.geometry.direction_40 == 1 ? 1.0L : -1.0L));
    point[0] = static_cast<double>(wrap_cylinder_x(point[0]));
    point[1] = static_cast<double>(clamp_cylinder_y(grid, point[1]));
    const auto x = static_cast<std::uint32_t>(integer_low(point[0]));
    const auto y = integer_low(point[1]);
    const auto next_y = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(y) +
                                                   static_cast<std::uint32_t>(actor.geometry.direction_44));
    const auto first = static_cast<std::uint32_t>(*collision_cell(grid, x, y));
    const auto second = static_cast<std::uint32_t>(*collision_cell(grid, x, next_y));
    if (((first | second) & 0x6200u) == 0) { actor.gadget_54 = nullptr; return; }
    const auto next_point = [&] {
        point[1] = static_cast<double>(static_cast<long double>(actor.geometry.direction_44) + point[1]);
    };
    if (actor.gadget_54 == nullptr) {
        const auto scan = [&] {
            for (std::int32_t index = 0; index < hooks.gadget_count(hooks.context); ++index) {
                void* gadget = hooks.gadget_at(hooks.context, index);
                if (hooks.gadget_contains(hooks.context, gadget, point)) {
                    actor.gadget_54 = gadget;
                    actor.gadget_58 = gadget;
                    break;
                }
            }
        };
        scan();
        if (actor.gadget_54 == nullptr) { next_point(); scan(); }
    } else if (!hooks.gadget_contains(hooks.context, actor.gadget_54, point)) {
        next_point();
        if (!hooks.gadget_contains(hooks.context, actor.gadget_54, point)) actor.gadget_54 = nullptr;
    }
}
void approach_actor_walking_depth(WalkingActor& actor, double target_direction_minus_one, double target_other) {
    if (actor.depth_adjust_5e == 0) return;
    constexpr double step = std::bit_cast<double>(std::uint64_t{0x3F947AE147AE147B});
    const double target = actor.geometry.direction_40 == -1 ? target_direction_minus_one : target_other;
    auto& z = actor.geometry.position[2];
    if (!(z >= target)) {
        z = static_cast<double>(static_cast<long double>(step) + z);
        if (z > target) z = target;
    } else if (z > target) {
        z = static_cast<double>(static_cast<long double>(z) - static_cast<long double>(step));
        if (!(z >= target)) z = target;
    }
}
} // namespace lemmings
