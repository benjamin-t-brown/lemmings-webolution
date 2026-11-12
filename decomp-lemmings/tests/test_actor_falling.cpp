#include "actor_falling_fixture.hpp"

#include <cmath>
#include <limits>
#include <source_location>
#include <stdexcept>

namespace {
void check(bool condition, std::source_location where = std::source_location::current()) {
    if (!condition) throw std::runtime_error("actor falling mismatch at line " +
        std::to_string(where.line()));
}
}

int main() {
    using namespace falling_test;
    using lemmings::actor_fall_distance;
    using lemmings::actor_fall_time_to_cell;
    for (const std::uint8_t result : {0, 1, 255}) {
        Fixture f;
        f.actor.floating_4b = 255;
        f.query_result = result;
        lemmings::enter_actor_falling(f.actor, f);
        check(f.actor.floating_4b == 0);
        check(f.trace[0][1] == 0x2B && f.trace[0][7] == 255);
        check(f.trace.size() == (result == 0 ? 2u : 1u));
        if (result == 0) check(f.trace[1][1] == 1 && f.trace[1][7] == 255);
    }
    {
        Fixture f;
        check(actor_fall_distance(f.actor, 0.25) == 5.0L);
        f.actor.floating_4b = 255;
        check(actor_fall_distance(f.actor, 0.25) == 2.5L);
        f.actor.geometry.position[1] = 20.25;
        check(bits(static_cast<double>(actor_fall_time_to_cell(f.state, f.actor))) == bits(0.025));
        f.actor.geometry.direction_44 = -1;
        check(bits(static_cast<double>(actor_fall_time_to_cell(f.state, f.actor))) == bits(0.075));
        f.actor.geometry.position[1] = -0.25;
        f.actor.geometry.direction_44 = 1;
        check(bits(static_cast<double>(actor_fall_time_to_cell(f.state, f.actor))) == bits(-0.025));
        f.actor.geometry.position[1] = 20;
        f.state.last_cell_21 = 0;
        check(actor_fall_time_to_cell(f.state, f.actor) == 0);
        f.state.last_cell_21 = 255;
        check(bits(static_cast<double>(actor_fall_time_to_cell(f.state, f.actor))) == bits(0.1));
        f.actor.geometry.position[1] = std::numeric_limits<double>::quiet_NaN();
        check(bits(static_cast<double>(actor_fall_time_to_cell(f.state, f.actor))) == bits(0.1));
        f.actor.geometry.position[1] = 4294967296.25;
        check(actor_fall_time_to_cell(f.state, f.actor) > 100000000.0L);
        f.actor.geometry.position[1] = 2147483647.0;
        f.actor.geometry.direction_44 = -1;
        check(actor_fall_time_to_cell(f.state, f.actor) < -100000000.0L);
    }
    {
        Fixture f;
        f.remaining = 0.01;
        f.height_result = -1;
        check(f.run() == 0 && f.remaining == 0);
        check(f.events() == std::vector<std::uint64_t>{height, advance});
        check(f.actor.geometry.position[2] == 0 && f.state.last_cell_21 == 0);
        check(f.state.distance_18 == 0);
    }
    {
        Fixture f;
        f.remaining = 0.5;
        f.transition_on_cell = 1;
        f.state.distance_18 = 10;
        f.actor.umbrella_49 = 1;
        check(f.run() == 17);
        check(f.events() == std::vector<std::uint64_t>{height, advance, animation, landing});
        check(f.trace[2][1] == 3 && f.trace[2][4] == 0);
        check(f.trace[3][3] == bits(11) && f.trace[3][4] == 1 && f.trace[3][5] == 0);
        check(f.state.last_cell_21 == 1 && f.remaining > 0);
    }
    {
        Fixture f;
        f.remaining = 0.01;
        f.actor.umbrella_49 = 1;
        f.state.umbrella_open_20 = 1;
        f.model_duration = f.model_elapsed = 0.25L;
        check(f.run() == 0 && f.actor.floating_4b == 1);
        check(f.trace[0][0] == duration && f.trace[1][0] == elapsed && f.trace[2][0] == log_event);
        check(f.trace[2][1] == 0x3E45798EE0000000ULL);
        check(f.trace[5][0] == animation && f.trace[5][1] == 2 && f.trace[5][7] == 0);
    }
    for (double remaining : {-1.0, -0.0, std::numeric_limits<double>::quiet_NaN()}) {
        Fixture f;
        f.remaining = remaining;
        check(f.run() == 0 && bits(f.remaining) == 0 && f.trace.empty());
    }
    {
        Fixture f;
        f.remaining = 0.1;
        f.actor.geometry.position[1] = 20;
        f.actor.umbrella_49 = f.state.umbrella_open_20 = 1;
        f.model_duration = 0;
        f.model_elapsed = std::bit_cast<double>(std::uint64_t{0x3E45798EE0000000});
        f.transition_on_cell = 1;
        check(f.run() == 17);
        check(f.trace[2][0] == log_event && f.trace[2][1] == bits(0));
        check(f.trace[5][0] == animation && f.trace[5][1] == 2);
        check(f.trace[8][0] == landing); // Umbrella completion wins an exact tie.
    }
    {
        Fixture f;
        f.remaining = 0.01;
        f.height_result = std::numeric_limits<long double>::quiet_NaN();
        f.height_mutation = 1;
        check(f.run() == 0 && f.actor.geometry.position[2] == 0);
        check(f.actor.geometry.position[1] > 21); // Recompute after height callback.
    }
    {
        Fixture f;
        f.remaining = 0.3;
        check(f.run() == 17 && f.cells == 3 && f.remaining > 0);
        check(f.state.distance_18 == 3 && f.state.last_cell_21 == 1);
    }
}
