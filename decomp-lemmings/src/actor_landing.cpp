#include "lemmings/actor_landing.hpp"
#include "lemmings/cylinder_coordinate.hpp"

#include <bit>

namespace lemmings {
namespace {
// Original __ftol returns signed64 in EDX:EAX; the callers retain only EAX.
// Masked x87 invalid conversion returns INT64_MIN, whose low32 bits are zero.
std::int32_t x87_low_integer(long double value) {
    constexpr long double limit = 9223372036854775808.0L;
    if (!(value >= -limit && value < limit)) return 0;
    return std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
}
}

void select_actor_landed_state(const ActorLandingHooks& hooks) {
    auto* state = hooks.create_state(hooks.context, ActorLandingTransition::landed_41bd60);
    hooks.attach_state(hooks.context, state);
}

bool resolve_actor_landing(const ActorGeometryState& actor, double distance_18,
                           std::uint8_t umbrella_49, CollisionGridHead& grid,
                           const ActorLandingHooks& hooks) {
    const auto point = actor_sample_position_360(actor);
    const auto initial_y = x87_low_integer(
        static_cast<long double>(point[1]) - actor.direction_44);
    const auto y = x87_low_integer(clamp_cylinder_y(grid, static_cast<double>(initial_y)));
    const auto initial_x = x87_low_integer(point[0]);
    const auto x = x87_low_integer(wrap_cylinder_x(static_cast<double>(initial_x)));
    const auto cell = static_cast<std::uint32_t>(
        *collision_cell(grid, static_cast<std::uint32_t>(x), y));
    const auto transition = [&](ActorLandingTransition type) {
        auto* state = hooks.create_state(hooks.context, type);
        hooks.attach_state(hooks.context, state);
    };

    // The original x87 unordered comparison takes this path for NaN Y too.
    if (!(actor.position[1] > 0.0) ||
        !(static_cast<long double>(grid.height) > actor.position[1])) {
        transition(ActorLandingTransition::state_40b3a0);
        return true;
    }
    if ((cell & 0x1000u) != 0) {
        hooks.bounce_notice(hooks.context, x, y);
        transition(ActorLandingTransition::bounce_40ec7e);
        return true;
    }
    if (cell != 0 && (cell & 0x80u) == 0) {
        if (distance_18 > 30.0 && umbrella_49 == 0 &&
            ((cell & 0x200u) == 0 || actor.category_4c != 1) &&
            ((cell & 0x4000u) == 0 || actor.category_4c != 2)) {
            transition(ActorLandingTransition::state_40b3a0);
        } else {
            hooks.select_landed_state(hooks.context);
        }
        return true;
    }
    return false;
}

} // namespace lemmings
