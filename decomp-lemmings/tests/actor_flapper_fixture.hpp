#pragma once
#include "lemmings/actor_flapper.hpp"
#include <bit>
#include <cmath>
#include <string_view>
#include <vector>

namespace flapper_test {
using namespace lemmings;
inline std::uint64_t bits(double v) {
    return std::isnan(v) ? 0x7ff8000000000000ULL : std::bit_cast<std::uint64_t>(v);
}
struct Event {
    int kind;
    std::array<std::uint64_t, 10> values;
    bool operator==(const Event&) const = default;
};
struct Fixture : ActorFlapperCallbacks {
    ActorFlapperState state;
    FlapperActor actor;
    ActorBehavior next;
    double remaining = 0.125;
    std::uint32_t global = 123;
    unsigned script = 0;
    std::uint8_t has = 0, finished = 0;
    bool fail = false, valid = true;
    long double model_elapsed = 0.03125L;
    std::array<std::int32_t, 16 * 256> cells{};
    CollisionGridHead grid{};
    std::vector<Event> trace;
    Fixture() { grid.height = 16; grid.cells.data = cells.data(); actor.geometry.position = {10.5, 8.5, 0.5}; }
    void event(int kind, std::uint64_t argument = 0) {
        trace.push_back({kind, {bits(state.elapsed_08), bits(remaining),
            bits(actor.geometry.position[0]), bits(actor.geometry.position[1]),
            static_cast<std::uint32_t>(actor.geometry.direction_44), actor.floating_4b,
            static_cast<std::uint16_t>(actor.identifier_3a), global,
            static_cast<std::uint32_t>(actor.geometry.category_4c), argument}});
    }
    void notice_enter(std::int16_t id, std::uint32_t context) override {
        valid &= id == actor.identifier_3a && context == global; event(1);
        if (script & 1) actor.geometry.category_4c = 3;
    }
    void select_animation_42fd25(FlapperActor& a, const char* name) override {
        valid &= &a == &actor && std::string_view(name) == "explode"; event(2);
    }
    void advance_model_2c(FlapperActor& a, double dt) override {
        valid &= &a == &actor; event(3, bits(dt));
        if (script & 1) { remaining *= 0.5; state.elapsed_08 += 1; actor.geometry.position[1] += 2; }
    }
    std::uint8_t has_state_42ecc9(FlapperActor& a, std::int32_t type) override {
        valid &= &a == &actor && type == 0; event(4);
        if (script & 2) { actor.geometry.direction_44 = -actor.geometry.direction_44; actor.floating_4b ^= 1; }
        return has;
    }
    std::uint8_t finished_model_44(FlapperActor& a) override {
        valid &= &a == &actor; event(5);
        if (script & 4) { remaining = 11; ++global; actor.identifier_3a = -23; }
        return finished;
    }
    ActorBehavior* create_state_4081b0() override {
        event(6); if (script & 8) actor.geometry.position[1] += 3;
        return fail ? nullptr : &next;
    }
    long double elapsed_model_48(FlapperActor& a) override {
        valid &= &a == &actor; event(7);
        if (script & 16) { remaining = -99; actor.geometry.category_4c = 3; }
        return model_elapsed;
    }
    void notice_finished(std::int16_t id, std::uint32_t context, double dt) override {
        valid &= id == actor.identifier_3a && context == global && bits(dt) == bits(remaining); event(8, bits(dt));
        if (script & 32) actor.geometry.direction_44 = -2;
    }
    void attach_42e696(FlapperActor& a, ActorBehavior* value) override {
        valid &= &a == &actor && value == (fail ? nullptr : &next); event(9, value != nullptr);
        if (script & 64) { actor.geometry.position[1] = 7; remaining = 0.125; }
    }
};
} // namespace flapper_test
