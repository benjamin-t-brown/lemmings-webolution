#pragma once

#include "lemmings/actor_walking_collision.hpp"

#include <bit>
#include <functional>
#include <vector>

namespace walking_collision_test {
inline std::uint64_t bits(double value) { return std::bit_cast<std::uint64_t>(value); }
enum Event : std::uint64_t { query = 1, count, at, contains, enabled, activate,
                           name, animation, clear, allocation, construct, attach, enter, move_y };
struct Gadget {
    std::int32_t mode = 0;
    std::uint8_t enabled = 0;
    std::uint8_t contains = 0;
    bool associated = false;
};
struct Fixture : lemmings::ActorWalkingCollisionCallbacks {
    lemmings::WalkingActor actor;
    std::array<std::int32_t, 8 * 256> cells{};
    lemmings::CollisionGridHead grid{};
    std::array<Gadget, 3> gadgets{};
    std::int32_t gadget_count = 3;
    std::uint8_t query_result = 0;
    std::array<lemmings::ActorBehavior, 4> states{};
    std::array<lemmings::WalkingCollisionTransition, 4> types{};
    std::size_t created = 0;
    std::vector<std::vector<std::uint64_t>> trace;
    std::function<void()> sync;

    Fixture() {
        actor.geometry.position = {2.5, 3.5, 0};
        grid.height = 8;
        grid.cells.data = cells.data();
    }
    std::uint64_t gadget_id(void* gadget) const {
        for (std::size_t i = 0; i < gadgets.size(); ++i) if (gadget == &gadgets[i]) return i + 1;
        return 0;
    }
    std::uint64_t state_id(lemmings::ActorBehavior* state) const {
        for (std::size_t i = 0; i < states.size(); ++i) if (state == &states[i]) return i + 1;
        return 0;
    }
    void record(Event event, std::uint64_t first = 0, std::uint64_t second = 0,
                std::uint64_t third = 0) {
        if (sync) sync();
        trace.push_back({event, first, second, third, actor.special_4a,
            static_cast<std::uint32_t>(actor.geometry.direction_40),
            bits(actor.geometry.position[1]), state_id(actor.primary_340),
            gadgets[0].associated, gadgets[1].associated, gadgets[2].associated});
    }
    std::uint8_t has_state_42ecc9(lemmings::WalkingActor&, std::int32_t id) override {
        record(query, id); return query_result;
    }
    std::int32_t gadget_count_41cb60() override { record(count); return gadget_count; }
    void* gadget_at_41cb40(std::int32_t index) override { record(at, index); return &gadgets[index]; }
    std::uint8_t gadget_contains_430ba1(void* gadget, const std::array<double, 3>& point) override {
        record(contains, gadget_id(gadget), bits(point[0]), bits(point[1]));
        return static_cast<Gadget*>(gadget)->contains;
    }
    std::uint8_t gadget_enabled_41cba0(void* gadget) override {
        record(enabled, gadget_id(gadget)); return static_cast<Gadget*>(gadget)->enabled;
    }
    void activate_gadget_20(void* gadget) override { record(activate, gadget_id(gadget)); }
    void set_gadget_actor_a4(void* gadget, lemmings::WalkingActor&) override {
        static_cast<Gadget*>(gadget)->associated = true;
    }
    std::int32_t gadget_mode_128(void* gadget) override { return static_cast<Gadget*>(gadget)->mode; }
    std::string_view gadget_animation_406500(void* gadget) override {
        record(name, gadget_id(gadget)); return "gadget-animation";
    }
    void select_animation_42fd25(lemmings::WalkingActor&, std::string_view value) override {
        record(animation, value == "gadget-animation");
    }
    void clear_states_42e78a(lemmings::WalkingActor&) override { record(clear); }
    lemmings::ActorBehavior* create_state(lemmings::WalkingCollisionTransition type) override {
        record(allocation, type == lemmings::WalkingCollisionTransition::falling_415fa0 ? 0x28 : 0x20);
        record(construct, static_cast<std::uint64_t>(type));
        types[created] = type;
        return &states[created++];
    }
    void attach_42e696(lemmings::WalkingActor& value, lemmings::ActorBehavior* state) override {
        record(attach, state_id(state)); value.primary_340 = state;
    }
    void enter_state_08(lemmings::ActorBehavior* state, lemmings::WalkingActor&) override {
        record(enter, state_id(state));
    }
    void move_y_42ee74(lemmings::WalkingActor& value, double y) override {
        record(move_y, bits(y)); value.geometry.position[1] = y;
    }
    std::int32_t run() { return lemmings::resolve_actor_walking_collision(actor, grid, *this); }
    std::vector<std::uint64_t> events() const {
        std::vector<std::uint64_t> result;
        for (const auto& item : trace) result.push_back(item[0]);
        return result;
    }
    void row(std::size_t y, std::uint32_t value) {
        for (std::size_t x = 0; x < 256; ++x) cells[y * 256 + x] = static_cast<std::int32_t>(value);
    }
};
}
