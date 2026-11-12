#include "lemmings/actor_bouncing.hpp"
#include "lemmings/actor_motion.hpp"
#include "lemmings/cylinder_coordinate.hpp"

#include <bit>
#include <cmath>

namespace lemmings {
namespace {
std::int32_t low_integer(long double x) {
    constexpr long double limit = 9223372036854775808.0L;
    if (!(x >= -limit && x < limit)) return 0;
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::int64_t>(x)));
}
std::int32_t signed_bits(std::uint32_t x) { return std::bit_cast<std::int32_t>(x); }
bool equal_or_unordered(double a, double b) { return !(a < b || a > b); }
}

void initialize_bounce_times(BounceTimeTable& table) {
    for (std::int32_t height = 0; height <= 24; ++height) {
        const double discriminant = static_cast<double>(35.0L * 35.0L + 2.0L * -25.0L * height);
        const double root = static_cast<double>(std::sqrt(static_cast<long double>(discriminant)));
        const double first = static_cast<double>((-35.0L + root) / -25.0L);
        const double second = static_cast<double>((-35.0L - root) / -25.0L);
        if (!(first >= table.apex_time)) {
            table.times[height] = first;
            table.times[49 - height] = second;
        } else {
            table.times[height] = second;
            table.times[49 - height] = first;
        }
    }
}

long double actor_bounce_time_to_x(const ActorBouncingState& state, const BouncingActor& actor) {
    const double x = actor.geometry.position[0];
    const auto cell = low_integer(x);
    double time = actor.geometry.direction_40 == 1
        ? static_cast<double>((static_cast<long double>(signed_bits(static_cast<std::uint32_t>(cell) + 1)) - x) / 5.0L)
        : static_cast<double>((static_cast<long double>(x) - cell) / 5.0L);
    if (state.reached_boundary_28 && equal_or_unordered(time, 0))
        time = static_cast<double>(1.0L / 5.0L);
    return time;
}

long double actor_bounce_time_to_y(const ActorBouncingState& state, const BounceTimeTable& table,
                                  std::int32_t& next_height) {
    double time = 0;
    std::int32_t index = 1;
    for (; index < 50; ++index) {
        if (state.elapsed_08 >= table.times[index - 1] &&
            !(state.elapsed_08 >= table.times[index])) {
            time = static_cast<double>(static_cast<long double>(table.times[index]) - state.elapsed_08);
            next_height = index < 25 ? index : 49 - index;
            break;
        }
    }
    if (state.reached_boundary_28 && equal_or_unordered(time, 0) && index + 1 < 50) {
        time = static_cast<double>(static_cast<long double>(table.times[index + 1]) - table.times[index]);
        next_height = index + 1 < 25 ? index + 1 : 48 - index;
    }
    return time;
}

void enter_actor_bouncing(ActorBouncingState& state, BouncingActor& actor,
                          const CollisionGridHead& grid, ActorBouncingCallbacks& cb) {
    auto foot = actor_sample_position_360(actor.geometry);
    foot[1] = static_cast<double>(static_cast<long double>(foot[1]) - actor.geometry.direction_44);
    state.origin_y_18 = low_integer(foot[1]);
    state.origin_x_20 = foot[0];
    const auto id = cb.actor_id_406a00(actor);
    cb.notice_40f860(BounceNotice::started, foot[0], foot[1], id);
    cb.select_animation_42fd25(actor, "fall");
    const auto y = low_integer(clamp_cylinder_y(grid, low_integer(
        static_cast<long double>(foot[1]) - actor.geometry.direction_44)));
    const auto x = low_integer(wrap_cylinder_x(low_integer(foot[0])));
    BounceNearbyActor* nearest = nullptr;
    std::int32_t minimum = 0x20000;
    for (std::int32_t i = 0; i < cb.nearby_count_404d30(); ++i) {
        const auto nearby_id = cb.nearby_id_404d10(i);
        auto* nearby = cb.find_actor_444ba1(nearby_id);
        if (!nearby) continue;
        const auto dx = static_cast<std::uint32_t>(low_integer(nearby->position[0])) - static_cast<std::uint32_t>(x);
        const auto dy = static_cast<std::uint32_t>(low_integer(nearby->position[1])) - static_cast<std::uint32_t>(y);
        const auto distance = signed_bits(dx * dx + dy * dy);
        if (distance < minimum) { minimum = distance; nearest = nearby; }
    }
    if (nearest && nearest->primary_type_14 == 0xE) nearest->primary_flag_18 = 1;
}

void leave_actor_bouncing(BouncingActor& actor, ActorBouncingCallbacks& cb) {
    if (cb.has_state_42ecc9(actor, 0x2D)) cb.remove_state_42e7f3(actor, 0x2D);
}

std::int32_t resolve_actor_bounce_collision(const ActorBouncingState& state, BouncingActor& actor,
    std::uint8_t horizontal, CollisionGridHead& grid, const BounceTimeTable& table,
    ActorBouncingCallbacks& cb) {
    const auto head = actor_sample_position_350(actor.geometry);
    const auto foot = actor_sample_position_360(actor.geometry);
    const auto head_cell = *collision_cell(grid, static_cast<std::uint32_t>(low_integer(head[0])), low_integer(head[1]));
    const auto y = low_integer(clamp_cylinder_y(grid, low_integer(
        static_cast<long double>(foot[1]) - actor.geometry.direction_44)));
    const auto foot_cell = static_cast<std::uint32_t>(*collision_cell(grid,
        static_cast<std::uint32_t>(low_integer(foot[0])), y));
    if (!horizontal) {
        cb.notice_40f860(BounceNotice::checking_floor);
        if (foot_cell && !(foot_cell & 0x80)) {
            cb.select_landed_42f8b9(actor);
            cb.notice_40f860(BounceNotice::hit_floor);
            return 1;
        }
        if ((foot_cell & 0x1000) && state.elapsed_08 > table.apex_time) {
            auto* next = cb.create_state(BounceStateKind::bounce_40ec7e);
            cb.attach_state_42e696(actor, next);
            cb.notice_40f860(BounceNotice::hit_trampoline);
            return 1;
        }
        if (head_cell) {
            auto* next = cb.create_state(BounceStateKind::falling_415fa0);
            cb.attach_state_42e696(actor, next);
            cb.notice_40f860(BounceNotice::hit_head);
            return 1;
        }
        if (equal_or_unordered(state.elapsed_08, table.times[49])) {
            auto* next = cb.create_state(BounceStateKind::falling_415fa0);
            cb.attach_state_42e696(actor, next);
            return 1;
        }
    } else {
        cb.notice_40f860(BounceNotice::checking_wall);
        if (cb.wall_contact_42eb48(actor)) {
            reverse_actor_direction_40(actor.geometry);
            cb.notice_40f860(BounceNotice::hit_wall);
        }
    }
    return 0;
}

std::int32_t update_actor_bouncing(ActorBouncingState& state, BouncingActor& actor,
    double& remaining, CollisionGridHead& grid, const BounceTimeTable& table,
    ActorBouncingCallbacks& cb) {
    cb.advance_model_2c(actor.model_78, remaining);
    if (!actor.effect_60) {
        auto* effect = cb.create_state(BounceStateKind::effect_40e3c0);
        cb.attach_state_42e696(actor, effect);
    }
    state.reached_boundary_28 = 0;
    std::int32_t result = 0;
    // The original leaves this stack local indeterminate until the Y helper
    // finds an interval. The documented update domain guarantees that write.
    std::int32_t next_height = 0;
    while (remaining > 0 && result == 0) {
        const double tx = static_cast<double>(actor_bounce_time_to_x(state, actor));
        const double ty = static_cast<double>(actor_bounce_time_to_y(state, table, next_height));
        const bool horizontal = !(tx >= ty); // Includes unordered.
        double step = horizontal ? tx : ty;
        const bool boundary = !(step > remaining); // Includes unordered.
        if (boundary) cb.notice_40f860(BounceNotice::updating_to_integer);
        else step = remaining;
        state.elapsed_08 = static_cast<double>(static_cast<long double>(step) + state.elapsed_08);
        if (!horizontal && boundary)
            state.elapsed_08 = table.times[state.elapsed_08 > table.apex_time ? 49 - next_height : next_height];
        remaining = static_cast<double>(static_cast<long double>(remaining) - step);
        cb.notice_40f860(BounceNotice::updated_by, step);
        double height = boundary && !horizontal ? static_cast<double>(next_height)
            : static_cast<double>(35.0L * state.elapsed_08 + -12.5L * state.elapsed_08 * state.elapsed_08);
        height = static_cast<double>(static_cast<long double>(actor.geometry.direction_44) * height);
        next_height = signed_bits(static_cast<std::uint32_t>(next_height) *
                                  static_cast<std::uint32_t>(actor.geometry.direction_44));
        const double y = static_cast<double>(static_cast<long double>(height) + state.origin_y_18);
        const double x = static_cast<double>(static_cast<long double>(actor.geometry.direction_40) *
                                             5.0L * step + actor.geometry.position[0]);
        // Original format receives Y first, then X.
        cb.notice_40f860(BounceNotice::new_position, y, x);
        const double wrapped_x = static_cast<double>(wrap_cylinder_x(x));
        const double clamped_y = static_cast<double>(clamp_cylinder_y(grid, y));
        move_actor_sample_x(actor.geometry, wrapped_x);
        move_actor_sample_y(actor.geometry, clamped_y, grid);
        if (boundary) {
            result = resolve_actor_bounce_collision(state, actor, horizontal, grid, table, cb);
            state.reached_boundary_28 = 1;
        } else state.reached_boundary_28 = 0;
    }
    if (result == 0) remaining = 0;
    return result;
}

} // namespace lemmings
