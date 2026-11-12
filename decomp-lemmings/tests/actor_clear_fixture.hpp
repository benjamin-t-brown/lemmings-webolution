#pragma once

#include "lemmings/actor_update.hpp"

#include <array>
#include <functional>
#include <vector>

namespace actor_clear_test {
struct Fixture;
struct StateData { Fixture* fixture; std::size_t index; };
struct Fixture {
    lemmings::ActorBehaviorUpdate actor;
    std::array<lemmings::ActorBehavior, 18> states{};
    std::array<StateData, 18> data{};
    std::array<lemmings::ActorBehaviorNode, 18> nodes{};
    std::vector<std::array<std::size_t, 5>> trace;
    std::function<void()> sync;
    std::size_t size = 0;
    unsigned mode = 0;
    Fixture();
    void initialize(std::size_t count, unsigned changes) {
        size = count; mode = changes;
        actor.primary = &states[17];
        actor.extra.begin.behavior = &states[16];
        actor.extra.end.behavior = &states[17];
        auto* previous = &actor.extra.begin;
        for (std::size_t i = 0; i < count; ++i) {
            nodes[i] = {&states[i], &actor.extra.end, previous};
            previous->next = &nodes[i];
            previous = &nodes[i];
        }
        previous->next = &actor.extra.end;
        actor.extra.end.previous = previous;
    }
    std::size_t node_id(const lemmings::ActorBehaviorNode* node) const {
        if (node == &actor.extra.begin) return 100;
        if (node == &actor.extra.end) return 101;
        for (std::size_t i = 0; i < nodes.size(); ++i) if (node == &nodes[i]) return i;
        return 102;
    }
    std::size_t state_id(const lemmings::ActorBehavior* state) const {
        for (std::size_t i = 0; i < states.size(); ++i) if (state == &states[i]) return i;
        return 102;
    }
    void record(std::size_t kind, std::size_t index, std::size_t argument = 0) {
        if (sync) sync();
        trace.push_back({kind, index, argument, node_id(actor.extra.begin.next), state_id(actor.primary)});
    }
    void leave(std::size_t index) {
        record(1, index);
        if (mode == 1) nodes[index].behavior = nullptr;
        if (mode == 2 && index == 0) {
            nodes[size] = {&states[size], actor.extra.begin.next, &actor.extra.begin};
            nodes[size].next->previous = &nodes[size];
            actor.extra.begin.next = &nodes[size];
        }
        if ((mode == 11 || mode == 16) && index == 17) {
            nodes[size] = {&states[17], actor.extra.begin.next, &actor.extra.begin};
            nodes[size].next->previous = &nodes[size];
            actor.extra.begin.next = &nodes[size];
            actor.primary = mode == 11 ? &states[16] : nullptr;
        }
        if (mode == 11 && index < size) states[index].type_14 = 99;
        if (mode == 12 && index < size) actor.primary = &states[index];
        if (mode == 15 && index < size) {
            auto& node = nodes[index];
            node.previous->next = node.next; node.next->previous = node.previous;
            node.next = &actor.extra.end; node.previous = actor.extra.end.previous;
            node.previous->next = &node; actor.extra.end.previous = &node;
        }
    }
    void destroy(std::size_t index, std::uint32_t flags) {
        record(2, index, flags);
        if (mode == 3 && index == 0 && size >= 3) nodes[0].next = &nodes[2];
        if (mode == 13 && index < size) {
            auto& node = nodes[index];
            nodes[size] = {&states[16], node.next, &node};
            node.next->previous = &nodes[size]; node.next = &nodes[size];
        }
        if (mode == 13 && index == 17) actor.primary = &states[16];
        if (mode == 17 && index < size) nodes[index].behavior = nullptr;
    }
    void release(lemmings::ActorBehaviorNode& node) {
        const auto index = node_id(&node);
        record(3, index);
        if (mode == 4) node.next = &actor.extra.end; // Next must already be cached.
        if (mode == 14) node.next = &actor.extra.end;
    }
    bool reset_correctly() const {
        return actor.extra.begin.next == &actor.extra.end &&
            actor.extra.begin.previous == nullptr && actor.extra.end.next == nullptr &&
            actor.extra.end.previous == &actor.extra.begin && actor.primary == &states[17] &&
            actor.extra.begin.behavior == &states[16] && actor.extra.end.behavior == &states[17];
    }
    void run() {
        lemmings::ActorBehaviorClearHooks hooks{this, [](void* context, lemmings::ActorBehaviorNode& node) {
            static_cast<Fixture*>(context)->release(node);
        }};
        lemmings::clear_actor_extra_behaviors(actor, hooks);
    }
    void initialize_removal(std::size_t count, unsigned changes) {
        initialize(count, changes + 10);
        for (std::size_t i=0;i<states.size();++i) states[i].type_14=static_cast<std::int32_t>(i%3);
        states[17].type_14=3;
    }
    bool remove(std::int32_t type) {
        return lemmings::remove_actor_behavior(actor, type, {this, [](void* p, lemmings::ActorBehaviorNode& node) {
            static_cast<Fixture*>(p)->release(node);
        }});
    }
    std::vector<std::size_t> snapshot() const {
        std::vector<std::size_t> result{state_id(actor.primary)};
        for (auto* n : {&actor.extra.begin, &actor.extra.end}) {
            result.insert(result.end(), {state_id(n->behavior),node_id(n->next),node_id(n->previous)});
        }
        for (const auto& n : nodes) result.insert(result.end(), {state_id(n.behavior),node_id(n.next),node_id(n.previous)});
        for (const auto& state : states) result.push_back(static_cast<std::uint32_t>(state.type_14));
        return result;
    }
};
inline const lemmings::ActorBehaviorOperations operations{
    [](lemmings::ActorBehavior& state, std::uint32_t flags) {
        auto& data = *static_cast<StateData*>(state.context);
        data.fixture->destroy(data.index, flags);
    }, nullptr, nullptr,
    [](lemmings::ActorBehavior& state, lemmings::ActorBehaviorUpdate&) {
        auto& data = *static_cast<StateData*>(state.context);
        data.fixture->leave(data.index);
    }
};
inline Fixture::Fixture() {
    for (std::size_t i = 0; i < states.size(); ++i) {
        data[i] = {this, i};
        states[i].operations = &operations;
        states[i].context = &data[i];
    }
}
}
