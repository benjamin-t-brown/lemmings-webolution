#include "actor_walking_collision_fixture.hpp"

#include <limits>
#include <source_location>
#include <stdexcept>

namespace {
void check(bool condition, std::source_location where = std::source_location::current()) {
    if (!condition) throw std::runtime_error("walking collision mismatch at line " +
        std::to_string(where.line()));
}
}
int main() {
    using namespace walking_collision_test;
    using Type = lemmings::WalkingCollisionTransition;
    for (double y : {0.0, -1.0, 8.0, std::numeric_limits<double>::quiet_NaN()}) {
        Fixture f;
        f.actor.geometry.position[1] = y;
        f.actor.special_4a = 255;
        check(f.run() == 2 && f.trace.empty() && f.actor.special_4a == 255);
    }
    {
        Fixture f;
        check(f.run() == 1 && f.created == 1 && f.types[0] == Type::falling_415fa0);
        check(f.events() == std::vector<std::uint64_t>{allocation, construct, attach});
    }
    {
        Fixture f;
        f.row(2, 1);
        check(f.run() == 0 && f.trace.empty());
    }
    {
        Fixture f;
        f.row(1, 1);
        check(f.run() == 0 && f.actor.geometry.position[1] == 2);
        check(f.events() == std::vector<std::uint64_t>{move_y});
    }
    for (std::size_t barrier : {3u, 4u, 5u}) {
        Fixture f;
        f.row(3, 1); f.row(4, 1); f.row(barrier, 0x800);
        check(f.run() == 0 && f.actor.geometry.direction_40 == -1 && f.trace.empty());
        check(f.actor.geometry.pair_350[0] == -1 && f.actor.geometry.pair_360[0] == -1);
    }
    {
        Fixture f;
        f.row(3, 0x800); f.actor.geometry.category_4c = 3; f.row(2, 1);
        check(f.run() == 0 && f.actor.geometry.direction_40 == 1);
    }
    {
        Fixture f;
        f.row(3, 1); f.row(4, 1); f.row(5, 1); f.actor.climbable_48 = 255;
        check(f.run() == 1 && f.types[0] == Type::climbing_413750);
        check(f.events() == std::vector<std::uint64_t>{allocation, construct, enter});
        check(f.trace.back()[7] == 1); // Primary replaced before enter, no attach call.
    }
    {
        Fixture f;
        f.row(2, 0x8000);
        f.gadgets[2].enabled = 1; // No contains match: last visited gadget still wins.
        check(f.run() == 1 && f.created == 2 && f.gadgets[2].associated);
        check(f.types[0] == Type::gadget_41ad90 && f.types[1] == Type::falling_415fa0);
        auto event = f.events();
        check(event[0] == query && event[11] == enabled);
        check(event[event.size() - 4] == activate);
        check(f.trace[event.size() - 4][10] == 1);
    }
    for (int category : {0, 1, 2, 3}) {
        for (int mode : {0, 1, 2, 3}) {
            Fixture f;
            f.actor.gadget_54 = &f.gadgets[0];
            f.actor.geometry.category_4c = category;
            f.gadgets[0].mode = mode;
            f.row(1, 1);
            const bool pass = (category == 1 && mode == 0) || (category == 2 && mode == 2);
            check(f.run() == (pass ? 0 : 1));
            if (pass) check(f.actor.special_4a == 1 && f.trace.empty());
            else {
                check(f.events() == std::vector<std::uint64_t>{name, animation, clear,
                    allocation, construct, attach});
                check(f.types[0] == Type::gadget_4041fd);
            }
        }
    }
}
