#include "lemmings/actor_update.hpp"

#include <array>
#include <bit>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
using namespace lemmings;
void check(bool condition) {
    if (!condition) throw std::runtime_error("actor scheduler mismatch");
}
struct Fixture;
struct StateData {
    Fixture* fixture;
    unsigned id;
    unsigned calls = 0;
    std::uint32_t first_result = 3;
};
struct Fixture {
    ActorBehaviorUpdate actor;
    std::array<ActorBehavior, 4> states{};
    std::array<StateData, 4> data{};
    std::array<ActorBehaviorNode, 2> nodes{};
    std::vector<unsigned> trace;
    std::vector<std::uint64_t> times;
    ActorBehaviorHooks hooks;
    Fixture();
    void add(unsigned node, unsigned state) {
        auto* previous = actor.extra.end.previous;
        nodes[node] = {&states[state], &actor.extra.end, previous};
        previous->next = &nodes[node];
        actor.extra.end.previous = &nodes[node];
    }
};
void destroy(ActorBehavior& state, std::uint32_t flags) {
    auto& data = *static_cast<StateData*>(state.context);
    check(flags == 1);
    data.fixture->trace.push_back(100 + data.id);
}
std::uint32_t update(ActorBehavior& state, ActorBehaviorUpdate& actor, double& remaining) {
    auto& data = *static_cast<StateData*>(state.context);
    data.fixture->trace.push_back(data.id);
    data.fixture->times.push_back(std::bit_cast<std::uint64_t>(remaining));
    remaining -= 0.25;
    const auto result = data.calls++ == 0 ? data.first_result : 3;
    if (result == 1 && actor.primary == &state) actor.primary = &data.fixture->states[1];
    return result;
}
void leave(ActorBehavior& state, ActorBehaviorUpdate&) {
    auto& data = *static_cast<StateData*>(state.context);
    data.fixture->trace.push_back(200 + data.id);
}
void enter(ActorBehavior& state, ActorBehaviorUpdate&) {
    auto& data = *static_cast<StateData*>(state.context);
    data.fixture->trace.push_back(600 + data.id);
}
const ActorBehaviorOperations operations{destroy, update, enter, leave};
Fixture::Fixture() {
    for (unsigned i = 0; i < states.size(); ++i) {
        data[i] = {this, i};
        states[i] = {&operations, &data[i], 1};
    }
    actor.primary = &states[0];
    actor.identifier_3a = -123;
    hooks.context = this;
    hooks.forced_event_value = 456;
    hooks.forced_event = [](void* context, std::int16_t id, std::int32_t value, std::uint8_t flag) {
        check(id == -123 && value == 456 && flag == 0);
        static_cast<Fixture*>(context)->trace.push_back(300);
    };
    hooks.create_forced = [](void* context, ActorBehaviorUpdate&) {
        auto& fixture = *static_cast<Fixture*>(context);
        fixture.trace.push_back(301);
        return &fixture.states[1];
    };
    hooks.attach = [](void* context, ActorBehaviorUpdate& actor, ActorBehavior* state) {
        static_cast<Fixture*>(context)->trace.push_back(302);
        actor.primary = state;
    };
    hooks.release_node = [](void* context, ActorBehaviorNode& node) {
        auto& fixture = *static_cast<Fixture*>(context);
        check(node.previous->next == node.next && node.next->previous == node.previous);
        fixture.trace.push_back(400);
    };
    hooks.finalize = [](void* context, ActorBehaviorUpdate&) {
        static_cast<Fixture*>(context)->trace.push_back(500);
    };
}
}

int main() {
    {
        Fixture fixture;
        check(!has_actor_behavior(fixture.actor, -1));
        fixture.states[0].type_14 = -1;
        check(has_actor_behavior(fixture.actor, -1));
        ActorBehaviorAttachmentHooks hooks;
        hooks.context = &fixture;
        hooks.allocate_node = [](void* context) {
            auto& fixture = *static_cast<Fixture*>(context);
            fixture.trace.push_back(700);
            return fixture.nodes[0].behavior == nullptr ? &fixture.nodes[0] : &fixture.nodes[1];
        };
        fixture.states[2].extra_11 = 0x80;
        fixture.states[2].type_14 = 42;
        fixture.states[3].extra_11 = 0xFF;
        fixture.states[3].type_14 = 43;
        attach_actor_behavior(fixture.actor, fixture.states[2], hooks);
        attach_actor_behavior(fixture.actor, fixture.states[3], hooks);
        check(fixture.trace == std::vector<unsigned>{602, 700, 603, 700});
        check(fixture.actor.extra.begin.next == &fixture.nodes[1] &&
              fixture.nodes[1].next == &fixture.nodes[0] &&
              fixture.nodes[0].next == &fixture.actor.extra.end &&
              fixture.actor.extra.end.previous == &fixture.nodes[0]);
        check(has_actor_behavior(fixture.actor, 42) && has_actor_behavior(fixture.actor, 43));
        fixture.trace.clear();
        fixture.states[1].type_14 = 99;
        attach_actor_behavior(fixture.actor, fixture.states[1], hooks);
        check(fixture.trace == std::vector<unsigned>{601, 200});
        check(fixture.actor.primary == &fixture.states[1] &&
              !has_actor_behavior(fixture.actor, -1) && has_actor_behavior(fixture.actor, 99));
        fixture.actor.primary = nullptr;
        fixture.trace.clear();
        attach_actor_behavior(fixture.actor, fixture.states[0], hooks);
        check(fixture.trace == std::vector<unsigned>{600});
    }
    {
        Fixture fixture;
        fixture.actor.mode_50 = 2;
        fixture.states[0].phase_10 = 0;
        fixture.states[2].phase_10 = 0;
        fixture.add(0, 2);
        fixture.add(1, 3);
        check(update_actor_behavior_schedule(fixture.actor, 1.0, fixture.hooks));
        check(fixture.trace == std::vector<unsigned>{3, 0, 2, 3, 500});
    }
    {
        Fixture fixture;
        fixture.actor.mode_50 = 2;
        fixture.data[0].first_result = 1;
        fixture.data[2].first_result = 1;
        fixture.add(0, 2);
        fixture.add(1, 3);
        check(update_actor_behavior_schedule(fixture.actor, 1.0, fixture.hooks));
        check(fixture.trace == std::vector<unsigned>{0, 100, 1, 2, 202, 102, 400, 3, 1, 3, 500});
        check(fixture.actor.primary == &fixture.states[1] &&
              fixture.actor.extra.begin.next == &fixture.nodes[1]);
    }
    {
        Fixture fixture;
        fixture.actor.forced_transition_5d = 0x80;
        check(update_actor_behavior_schedule(fixture.actor, 1.0, fixture.hooks));
        check(fixture.trace == std::vector<unsigned>{300, 301, 302, 100, 1, 500});
        check(fixture.actor.forced_transition_5d == 0);
    }
    for (const double elapsed : {0.0, -1.0, std::numeric_limits<double>::quiet_NaN()}) {
        Fixture fixture;
        fixture.actor.mode_50 = 2;
        fixture.actor.forced_transition_5d = 1;
        fixture.add(0, 2);
        check(update_actor_behavior_schedule(fixture.actor, elapsed, fixture.hooks));
        check(fixture.trace == std::vector<unsigned>{2, 2, 500});
        check(fixture.actor.forced_transition_5d == 1);
    }
    for (unsigned exit_state : {0U, 2U}) {
        Fixture fixture;
        fixture.data[exit_state].first_result = 2;
        fixture.add(0, 2);
        fixture.add(1, 3);
        check(!update_actor_behavior_schedule(fixture.actor, 1.0, fixture.hooks));
        check(fixture.trace == (exit_state == 0 ? std::vector<unsigned>{0} :
                                                std::vector<unsigned>{0, 2}));
    }
    {
        Fixture fixture;
        fixture.states[0].phase_10 = 2;
        fixture.states[2].phase_10 = 0xFF;
        fixture.add(0, 2);
        check(update_actor_behavior_schedule(fixture.actor, 1.0, fixture.hooks));
        check(fixture.trace == std::vector<unsigned>{500});
    }
    {
        Fixture fixture;
        fixture.data[0].first_result = 0;
        fixture.data[2].first_result = 4; // Original extra-state loop retries same node.
        fixture.add(0, 2);
        check(update_actor_behavior_schedule(fixture.actor, 1.0, fixture.hooks));
        check(fixture.trace == std::vector<unsigned>{0, 0, 2, 2, 500});
        check(fixture.times == std::vector<std::uint64_t>{
            std::bit_cast<std::uint64_t>(1.0), std::bit_cast<std::uint64_t>(0.75),
            std::bit_cast<std::uint64_t>(1.0), std::bit_cast<std::uint64_t>(1.0)});
    }
}
