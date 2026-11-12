#include "lemmings/actor_flapper.hpp"
#include "lemmings/cylinder_coordinate.hpp"
#include <bit>
#include <cmath>

namespace lemmings {
namespace {
double rounded(long double value) {
    volatile double result = static_cast<double>(value);
    return result;
}
std::int32_t ftol(long double value) {
    // Original signed64 FISTP, projected to the low 32 bits. Masked invalid
    // conversion produces INT64_MIN, whose low word is zero.
    constexpr long double limit = 9223372036854775808.0L;
    if (!std::isfinite(value) || value < -limit || value >= limit) return 0;
    return std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
}
}

void initialize_actor_flapper(ActorFlapperState& state, ActorBehavior& behavior) {
    state.elapsed_08 = 0;
    behavior.phase_10 = 0;
    behavior.extra_11 = 0;
    behavior.type_14 = 0x20;
}

void enter_actor_flapper(FlapperActor& actor, const std::uint32_t& global,
                          ActorFlapperCallbacks& callbacks) {
    callbacks.notice_enter(actor.identifier_3a, global);
    if (actor.geometry.category_4c != 3)
        callbacks.select_animation_42fd25(actor, "explode");
}

void leave_actor_flapper() {}

std::uint32_t update_actor_flapper(ActorFlapperState& state, FlapperActor& actor,
    double& remaining, CollisionGridHead& grid, const std::uint32_t& global,
    ActorFlapperCallbacks& callbacks) {
    callbacks.advance_model_2c(actor, remaining);
    state.elapsed_08 = rounded(static_cast<long double>(state.elapsed_08) + remaining);
    const auto point = actor_sample_position_360(actor.geometry);
    // Integer conversion precedes wrapping/clamping. The Y subtraction remains
    // extended until FISTP; it does not round to binary64 first.
    const auto x = ftol(wrap_cylinder_x(static_cast<double>(ftol(point[0]))));
    const auto y = ftol(clamp_cylinder_y(grid, static_cast<double>(
        ftol(static_cast<long double>(point[1]) - actor.geometry.direction_44))));
    if (*collision_cell(grid, static_cast<std::uint32_t>(x), y) == 0 &&
        callbacks.has_state_42ecc9(actor, 0) == 0) {
        const auto distance = rounded(actor_fall_distance(actor, remaining) *
                                      actor.geometry.direction_44);
        actor.geometry.position[1] = rounded(
            static_cast<long double>(actor.geometry.position[1]) - distance);
    }
    remaining = 0;
    if (!callbacks.finished_model_44(actor)) return 0;
    auto* next = callbacks.create_state_4081b0();
    remaining = rounded(callbacks.elapsed_model_48(actor));
    callbacks.notice_finished(actor.identifier_3a, global, remaining);
    callbacks.attach_42e696(actor, next);
    // Attachment may change both fields; the original reloads them here.
    actor.geometry.position[1] = rounded(
        static_cast<long double>(actor.geometry.position[1]) - actor.geometry.direction_44);
    return 1;
}

} // namespace lemmings
