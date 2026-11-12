#pragma once

#include "lemmings/actor_falling.hpp"

#include <bit>
#include <functional>
#include <vector>

namespace falling_test {
inline std::uint64_t bits(double value) { return std::bit_cast<std::uint64_t>(value); }
enum Event : std::uint64_t { query = 1, animation, duration, elapsed, log_event,
                           height, advance, landing };
using Trace = std::vector<std::vector<std::uint64_t>>;

struct Fixture : lemmings::ActorFallingCallbacks {
    lemmings::ActorFallingState state;
    lemmings::FallingActor actor;
    lemmings::CollisionGridHead grid{};
    double remaining = 0;
    long double model_duration = 0.5L;
    long double model_elapsed = 0.0L;
    long double height_result = 2.0L;
    double height_mutation = 0;
    std::uint8_t query_result = 0;
    std::int32_t landing_result = 17;
    std::size_t transition_on_cell = 3;
    std::size_t cells = 0;
    Trace trace;
    std::function<void()> sync;

    Fixture() {
        grid.height = 96;
        actor.geometry.position = {1.5, 20.5, 3};
    }
    void record(Event event, std::uint64_t argument = 0) {
        if (sync) sync();
        trace.push_back({event, argument, bits(remaining), bits(state.distance_18),
            state.umbrella_open_20, state.last_cell_21, actor.umbrella_49, actor.floating_4b,
            bits(actor.geometry.position[0]), bits(actor.geometry.position[1]),
            bits(actor.geometry.position[2])});
    }
    std::uint8_t has_state_42ecc9(lemmings::FallingActor&, std::int32_t id) override {
        record(query, id);
        return query_result;
    }
    void select_animation_42fd25(lemmings::FallingActor&, std::string_view name) override {
        record(animation, name == "fall" ? 1 : name == "float" ? 2 : name == "umbopen" ? 3 : 0);
        model_elapsed = 0;
    }
    long double model_duration_4c(void*) override { record(duration); return model_duration; }
    long double model_elapsed_48(void*) override { record(elapsed); return model_elapsed; }
    void log_umbrella_time_40f860(double value) override { record(log_event, bits(value)); }
    long double height_43da38(std::array<double, 3>& position, double y) override {
        record(height, bits(y));
        if (height_mutation != 0) position[1] += height_mutation;
        return height_result;
    }
    void advance_model_2c(void*, double step) override {
        record(advance, bits(step));
        model_elapsed += static_cast<long double>(step);
    }
    std::int32_t landing_416433(lemmings::ActorFallingState&, lemmings::FallingActor&) override {
        record(landing);
        return ++cells >= transition_on_cell ? landing_result : 0;
    }
    std::int32_t run() {
        return lemmings::update_actor_falling(state, actor, remaining, grid, *this);
    }
    std::vector<std::uint64_t> events() const {
        std::vector<std::uint64_t> result;
        for (const auto& item : trace) result.push_back(item[0]);
        return result;
    }
};
}
