#include "emitter_update_fixture.hpp"

#include <limits>
#include <source_location>
#include <stdexcept>

namespace {
void check(bool condition, std::source_location where = std::source_location::current()) {
    if (!condition) throw std::runtime_error("emitter update mismatch at line " +
        std::to_string(where.line()));
}
}

int main() {
    using namespace emitter_test;
    {
        Fixture f;
        f.emitter.remaining_98 = 3;
        f.emitter.countdown_c0 = 10;
        check(f.run(2));
        check(f.events() == std::vector<std::uint64_t>{effect, model_update, model_finished});
        check(f.emitter.effect_started_ae == 1 && f.emitter.countdown_c0 == 10);
        f.trace.clear();
        f.run(2);
        check(f.events() == std::vector<std::uint64_t>{model_update, model_finished});
    }
    {
        Fixture f;
        f.world.suppress_effect_4e32a0 = 255;
        f.finished_result = 255;
        f.remainder = 0.25;
        f.emitter.remaining_98 = 2;
        f.run(100); // The model's remaining time replaces the supplied step.
        check(f.emitter.opened_ac == 1 && f.emitter.effect_started_ae == 0);
        check(bits(f.emitter.countdown_c0) == 0x3FF0CCCCCCCCCCCDULL);
        check(f.events() == std::vector<std::uint64_t>{model_update, model_finished, model_remainder});
    }
    {
        Fixture f;
        f.emitter.opened_ac = 255;
        f.emitter.effect_started_ae = 255;
        f.emitter.remaining_98 = 3;
        f.emitter.coordinate_9a = -32768;
        f.emitter.coordinate_9c = 32767;
        f.emitter.actor_type_b0 = -8;
        f.emitter.flip_vertical_ad = 255;
        f.emitter.direction_b4 = -1;
        f.emitter.alternate_b6 = 1;
        f.emitter.countdown_c0 = 0.5;
        f.emitter.interval_b8 = 0.25;
        f.counts[3] = 0;
        f.world.remaining_total_4cbaa0 = 0;
        f.run(5); // Only one actor, even though the timer remains negative.
        check(f.emitter.remaining_98 == 2 && f.emitter.countdown_c0 == -4.25);
        check(f.emitter.direction_b4 == 1);
        check(f.counts[3] == 65535 && f.world.remaining_total_4cbaa0 == 65535);
        check(f.events() == std::vector<std::uint64_t>{allocation, construction, vertical,
            horizontal, actor_update, insert_false, registration, refresh});
        check(f.trace[0][4] == 2); // Decrement precedes allocation.
        check(f.trace[1][1] == bits(-32768.0) && f.trace[1][2] == bits(32767.0));
        check(f.trace[4][2] == bits(4.5));
        check(f.trace.back()[8] == bits(-4.25)); // Refill precedes UI callback.
    }
    {
        Fixture f;
        f.emitter.effect_started_ae = f.emitter.opened_ac = 1;
        f.emitter.remaining_98 = 1;
        f.emitter.direction_b4 = 1;
        f.emitter.alternate_b6 = 2; // Exactly one, rather than nonzero, toggles.
        f.actor_result = 255;
        f.replace_on_insert = true;
        f.run(0);
        check(f.emitter.remaining_98 == 0 && f.emitter.direction_b4 == 1);
        check(f.counts[4] == 7 && f.counts[3] == 7);
        check(f.events() == std::vector<std::uint64_t>{allocation, construction,
            actor_update, insert_true, refresh});
        check(f.trace[2][2] == bits(-0.0));
    }
    for (std::int16_t remaining : {std::int16_t{0}, std::int16_t{-1}, std::int16_t{-32768}}) {
        Fixture f;
        f.emitter.effect_started_ae = f.emitter.opened_ac = 1;
        f.emitter.remaining_98 = remaining;
        f.emitter.countdown_c0 = -5;
        f.run(100);
        check(f.trace.empty() && f.emitter.countdown_c0 == -5);
    }
    for (double timer : {std::numeric_limits<double>::quiet_NaN(),
                         -std::numeric_limits<double>::infinity(), -0.0}) {
        Fixture f;
        f.emitter.effect_started_ae = f.emitter.opened_ac = 1;
        f.emitter.remaining_98 = 1;
        f.emitter.countdown_c0 = timer;
        f.run(0);
        check(f.emitter.remaining_98 == 0);
    }
    {
        Fixture f;
        f.emitter.effect_started_ae = f.emitter.opened_ac = 1;
        f.emitter.remaining_98 = 1;
        f.emitter.countdown_c0 = std::numeric_limits<double>::infinity();
        f.run(1);
        check(f.trace.empty() && f.emitter.remaining_98 == 1);
    }
}
