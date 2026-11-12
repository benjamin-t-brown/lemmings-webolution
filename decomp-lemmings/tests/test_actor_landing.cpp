#include "lemmings/actor_landing.hpp"

#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
using namespace lemmings;
void check(bool condition) {
    if (!condition) throw std::runtime_error("actor landing mismatch");
}
struct Fixture {
    std::array<std::int32_t, 8 * 256> cells{};
    CollisionGridHead grid{};
    ActorGeometryState actor{};
    ActorBehavior created;
    ActorLandingHooks hooks;
    std::vector<int> trace;
    std::int32_t bounce_x = -1, bounce_y = -1;
    Fixture() {
        grid.height = 8;
        grid.cells.data = cells.data();
        actor.position = {2.5, 3.5, 0};
        actor.direction_44 = 1;
        hooks.context = this;
        hooks.bounce_notice = [](void* context, std::int32_t x, std::int32_t y) {
            auto& self = *static_cast<Fixture*>(context);
            self.trace.push_back(1);
            self.bounce_x = x;
            self.bounce_y = y;
        };
        hooks.create_state = [](void* context, ActorLandingTransition type) {
            auto& self = *static_cast<Fixture*>(context);
            self.trace.push_back(type == ActorLandingTransition::state_40b3a0 ? 2 :
                type == ActorLandingTransition::bounce_40ec7e ? 3 : 6);
            return &self.created;
        };
        hooks.attach_state = [](void* context, ActorBehavior* state) {
            auto& self = *static_cast<Fixture*>(context);
            check(state == &self.created);
            self.trace.push_back(4);
        };
        hooks.select_landed_state = [](void* context) {
            static_cast<Fixture*>(context)->trace.push_back(5);
        };
    }
    bool run(std::uint32_t cell, double distance = 31.0, std::uint8_t umbrella = 0) {
        cells.fill(static_cast<std::int32_t>(cell));
        trace.clear();
        return resolve_actor_landing(actor, distance, umbrella, grid, hooks);
    }
};
}

int main() {
    Fixture fixture;
    select_actor_landed_state(fixture.hooks);
    check(fixture.trace == std::vector<int>{6, 4});
    for (auto cell : {0U, 0x80U, 0x2080U, 0x4080U}) {
        check(!fixture.run(cell) && fixture.trace.empty());
    }
    for (auto cell : {0x1000U, 0x1080U, 0xFFFFFFFFU}) {
        check(fixture.run(cell) && fixture.trace == std::vector<int>{1, 3, 4});
    }
    check(fixture.run(1) && fixture.trace == std::vector<int>{2, 4});
    check(fixture.run(1, 30.0) && fixture.trace == std::vector<int>{5});
    check(fixture.run(1, std::nextafter(30.0, 31.0)) && fixture.trace == std::vector<int>{2, 4});
    for (double distance : {0.0, -1.0, std::numeric_limits<double>::quiet_NaN()}) {
        check(fixture.run(1, distance) && fixture.trace == std::vector<int>{5});
    }
    for (std::uint8_t umbrella : {std::uint8_t{1}, std::uint8_t{0x80}, std::uint8_t{0xFF}}) {
        check(fixture.run(1, 100.0, umbrella) && fixture.trace == std::vector<int>{5});
    }
    fixture.actor.category_4c = 1;
    check(fixture.run(0x201) && fixture.trace == std::vector<int>{5});
    check(fixture.run(0x4001) && fixture.trace == std::vector<int>{2, 4});
    fixture.actor.category_4c = 2;
    check(fixture.run(0x4001) && fixture.trace == std::vector<int>{5});
    check(fixture.run(0x201) && fixture.trace == std::vector<int>{2, 4});
    fixture.actor.category_4c = 3;
    check(fixture.run(0x4201) && fixture.trace == std::vector<int>{2, 4});
    for (double y : {-1.0, -0.0, 0.0, 8.0, 9.0,
                     std::numeric_limits<double>::quiet_NaN(),
                     std::numeric_limits<double>::infinity()}) {
        fixture.actor.position[1] = y;
        // Bounds take precedence even over a bounce cell and umbrella flag.
        check(fixture.run(0x1080, 0.0, 1) && fixture.trace == std::vector<int>{2, 4});
    }
    fixture.actor.position = {-0.75, 3.5, 0};
    fixture.actor.category_4c = 0;
    fixture.cells.fill(0);
    fixture.cells[2 * 256] = 0x1000;
    fixture.trace.clear();
    check(resolve_actor_landing(fixture.actor, 0, 0, fixture.grid, fixture.hooks));
    check(fixture.bounce_x == 0 && fixture.bounce_y == 2 &&
          fixture.trace == std::vector<int>{1, 3, 4});
}
