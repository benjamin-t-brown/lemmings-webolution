#include "lemmings/actor_walking_collision.hpp"
#include "lemmings/cylinder_coordinate.hpp"

#include <bit>

namespace lemmings {
namespace {
std::int32_t x87_low_integer(long double value) {
    constexpr long double limit = 9223372036854775808.0L;
    if (!(value >= -limit && value < limit)) return 0;
    return std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
}
std::int32_t add_wrapping(std::int32_t first, std::int32_t second) {
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(first) +
                                      static_cast<std::uint32_t>(second));
}
std::int32_t subtract_wrapping(std::int32_t first, std::int32_t second) {
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(first) -
                                      static_cast<std::uint32_t>(second));
}
}

std::int32_t resolve_actor_walking_collision(WalkingActor& actor,
    CollisionGridHead& grid, ActorWalkingCollisionCallbacks& callbacks) {
    auto& geometry = actor.geometry;
    if (!(geometry.position[1] > 0.0) ||
        !(static_cast<long double>(grid.height) > geometry.position[1])) return 2;
    const auto point = actor_sample_position_360(geometry);
    actor.special_4a = 0;
    const auto x = static_cast<std::uint32_t>(x87_low_integer(
        wrap_cylinder_x(static_cast<double>(x87_low_integer(point[0])))));
    const auto clamped_integer = [&](std::int32_t value) {
        return x87_low_integer(clamp_cylinder_y(grid, static_cast<double>(value)));
    };
    const auto cell = [&](std::int32_t y) {
        return static_cast<std::uint32_t>(*collision_cell(grid, x, y));
    };
    const auto below_y = clamped_integer(x87_low_integer(
        static_cast<long double>(point[1]) - geometry.direction_44));
    const auto below_cell = cell(below_y);
    auto y = clamped_integer(x87_low_integer(point[1]));
    const auto current_cell = cell(y);
    const auto attach = [&](WalkingCollisionTransition type) {
        callbacks.attach_42e696(actor, callbacks.create_state(type));
    };
    if ((below_cell & 0x8000u) != 0 && callbacks.has_state_42ecc9(actor, 0x2B) == 0) {
        void* gadget = nullptr;
        for (std::int32_t i = 0; i < callbacks.gadget_count_41cb60(); ++i) {
            gadget = callbacks.gadget_at_41cb40(i);
            if (callbacks.gadget_contains_430ba1(gadget, actor_sample_position_360(geometry)) != 0) break;
        }
        if (callbacks.gadget_enabled_41cba0(gadget) != 0) {
            attach(WalkingCollisionTransition::gadget_41ad90);
            callbacks.set_gadget_actor_a4(gadget, actor);
            callbacks.activate_gadget_20(gadget);
            attach(WalkingCollisionTransition::falling_415fa0);
            return 1;
        }
    }
    if (actor.gadget_54 != nullptr) {
        const auto mode = callbacks.gadget_mode_128(actor.gadget_54);
        if (mode == 1 || !((geometry.category_4c == 1 && mode == 0) ||
                          (geometry.category_4c == 2 && mode == 2))) {
            const auto name = callbacks.gadget_animation_406500(actor.gadget_54);
            callbacks.select_animation_42fd25(actor, name);
            callbacks.clear_states_42e78a(actor);
            attach(WalkingCollisionTransition::gadget_4041fd);
            return 1;
        }
        actor.special_4a = 1;
    }
    const auto blocked = [&](std::uint32_t value) {
        return (value & 0x800u) != 0 && geometry.category_4c != 3;
    };
    const auto reverse = [&] { reverse_actor_direction_40(geometry); };
    if (blocked(current_cell)) {
        reverse();
        return 0;
    }
    if ((current_cell & 1u) != 0) {
        y = clamped_integer(add_wrapping(y, geometry.direction_44));
        auto upper_cell = cell(y);
        if (blocked(upper_cell)) {
            reverse();
            return 0;
        }
        if ((upper_cell & 1u) != 0) {
            y = clamped_integer(add_wrapping(y, geometry.direction_44));
            upper_cell = cell(y);
            if (blocked(upper_cell)) {
                reverse();
                return 0;
            }
            if ((upper_cell & 1u) != 0) {
                if (actor.climbable_48 != 0) {
                    actor.primary_340 = callbacks.create_state(WalkingCollisionTransition::climbing_413750);
                    callbacks.enter_state_08(actor.primary_340, actor);
                    return 1;
                }
                reverse();
                return 0;
            }
        }
        callbacks.move_y_42ee74(actor, static_cast<double>(y));
        return 0;
    }
    if (below_cell == 0) {
        y = clamped_integer(subtract_wrapping(y, geometry.direction_44));
        if (cell(y) == 0) {
            y = clamped_integer(subtract_wrapping(y, geometry.direction_44));
            if (cell(y) == 0) {
                attach(WalkingCollisionTransition::falling_415fa0);
                return 1;
            }
        }
        if (actor.special_4a == 0) {
            callbacks.move_y_42ee74(actor, static_cast<double>(add_wrapping(y, geometry.direction_44)));
        }
    }
    return 0;
}

} // namespace lemmings
