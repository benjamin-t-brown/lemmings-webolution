#include "lemmings/actor_falling.hpp"
#include "lemmings/cylinder_coordinate.hpp"

#include <bit>

namespace lemmings {
namespace {
std::int32_t low_word_of_truncated_x87(double value) {
    // 0x0049C5AC uses FISTP qword with round-toward-zero. A masked invalid
    // conversion yields INT64_MIN, whose low 32 bits are zero.
    constexpr long double limit = 9223372036854775808.0L;
    if (!(value >= -limit && value < limit)) return 0;
    const auto integer = static_cast<std::int64_t>(value);
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(integer));
}
double speed(const FallingActor& actor) { return actor.floating_4b == 0 ? 20.0 : 10.0; }
double next_y(const FallingActor& actor, double distance) {
    return static_cast<double>(static_cast<long double>(actor.geometry.position[1]) -
        static_cast<long double>(actor.geometry.direction_44) * distance);
}
}

void enter_actor_falling(FallingActor& actor, ActorFallingCallbacks& callbacks) {
    if (callbacks.has_state_42ecc9(actor, 0x2B) == 0) {
        callbacks.select_animation_42fd25(actor, "fall");
    }
    actor.floating_4b = 0;
}

long double actor_fall_distance(const FallingActor& actor, double elapsed) {
    return static_cast<long double>(speed(actor)) * elapsed;
}

long double actor_fall_time_to_cell(const ActorFallingState& state,
                                    const FallingActor& actor) {
    const auto y = actor.geometry.position[1];
    const auto integer = low_word_of_truncated_x87(y);
    double time;
    if (actor.geometry.direction_44 == 1) {
        time = static_cast<double>((static_cast<long double>(y) - integer) / speed(actor));
    } else {
        const auto next_integer = std::bit_cast<std::int32_t>(
            static_cast<std::uint32_t>(integer) + 1u);
        time = static_cast<double>((static_cast<long double>(next_integer) - y) / speed(actor));
    }
    // FCOMP / TEST AH,0x40 treats unordered as equal here.
    if (state.last_cell_21 != 0 && !(time < 0.0 || time > 0.0)) {
        time = static_cast<double>(1.0L / speed(actor));
    }
    return time;
}

std::int32_t update_actor_falling(ActorFallingState& state, FallingActor& actor,
                                  double& remaining_time, const CollisionGridHead& grid,
                                  ActorFallingCallbacks& callbacks) {
    std::int32_t result = 0;
    while (remaining_time > 0.0 && result == 0) {
        const double cell_time = static_cast<double>(actor_fall_time_to_cell(state, actor));
        double umbrella_time = 100000000.0;
        if (actor.umbrella_49 != 0 && state.umbrella_open_20 != 0 && actor.floating_4b == 0) {
            const double duration = static_cast<double>(callbacks.model_duration_4c(actor.model_78));
            const long double elapsed = callbacks.model_elapsed_48(actor.model_78);
            constexpr double epsilon = std::bit_cast<double>(std::uint64_t{0x3E45798EE0000000});
            umbrella_time = static_cast<double>((static_cast<long double>(duration) - elapsed) + epsilon);
            callbacks.log_umbrella_time_40f860(umbrella_time);
        }
        bool crossed_cell = false;
        bool finished_umbrella = false;
        double step;
        // Preserve the original unordered branches, not std::min semantics.
        if (!(cell_time >= umbrella_time) && !(cell_time > remaining_time)) {
            crossed_cell = true;
            step = cell_time;
        } else if (!(umbrella_time > remaining_time)) {
            finished_umbrella = true;
            step = umbrella_time;
        } else {
            step = remaining_time;
        }
        const double distance = static_cast<double>(actor_fall_distance(actor, step));
        actor.geometry.position[2] = static_cast<double>(
            callbacks.height_43da38(actor.geometry.position, next_y(actor, distance)));
        if (!(actor.geometry.position[2] >= 0.0)) actor.geometry.position[2] = 0.0;
        // The original recomputes this argument after the height query.
        actor.geometry.position[1] = static_cast<double>(clamp_cylinder_y(grid, next_y(actor, distance)));
        callbacks.advance_model_2c(actor.model_78, step);
        remaining_time = static_cast<double>(static_cast<long double>(remaining_time) - step);
        if (finished_umbrella) {
            callbacks.select_animation_42fd25(actor, "float");
            actor.floating_4b = 1;
        }
        if (crossed_cell) {
            if (state.distance_18 >= 10.0 && actor.umbrella_49 != 0 && state.umbrella_open_20 == 0) {
                callbacks.select_animation_42fd25(actor, "umbopen");
                state.umbrella_open_20 = 1;
            }
            state.distance_18 = static_cast<double>(static_cast<long double>(state.distance_18) + 1.0L);
            result = callbacks.landing_416433(state, actor);
            state.last_cell_21 = 1;
        } else {
            state.last_cell_21 = 0;
        }
    }
    if (result == 0) remaining_time = 0.0;
    return result;
}

} // namespace lemmings
