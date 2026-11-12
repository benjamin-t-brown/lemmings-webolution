#pragma once

#include "lemmings/emitter_update.hpp"

#include <array>
#include <bit>
#include <cstring>
#include <functional>
#include <vector>

// Test adapters expose boundary calls and their state without emulating the
// original animation, actor construction, sound, container, or UI systems.
namespace emitter_test {
template<class T> T load(const void* memory, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(memory) + offset, sizeof(T));
    return value;
}
template<class T> void store(void* memory, std::size_t offset, T value) {
    std::memcpy(static_cast<std::uint8_t*>(memory) + offset, &value, sizeof(T));
}
inline std::uint64_t bits(double value) { return std::bit_cast<std::uint64_t>(value); }
enum EventId : std::uint64_t {
    effect = 1, model_update, model_finished, model_remainder, allocation,
    construction, vertical, horizontal, actor_update, insert_true,
    insert_false, registration, refresh
};
using Trace = std::vector<std::vector<std::uint64_t>>;

struct Fixture : lemmings::EmitterCallbacks {
    lemmings::EmitterState emitter;
    std::array<std::uint16_t, 5> counts{4, 5, 6, 7, 8};
    lemmings::EmitterWorld world{0, 0x1234, counts, 30};
    std::array<std::uint8_t, 0x3A0> actor{}, replacement{};
    std::uint8_t finished_result = 0;
    std::uint8_t actor_result = 0;
    double remainder = 0;
    bool replace_on_insert = false;
    std::int32_t constructed_type = 2;
    std::int32_t updated_type = 3;
    Trace trace;
    std::function<void()> sync;

    Fixture() {
        emitter.model_44 = reinterpret_cast<void*>(std::uintptr_t{0x11223344});
        store(replacement.data(), 0x4C, std::int32_t{4});
    }
    std::uint64_t identity(void* handle) const {
        return handle == actor.data() ? 1 : handle == replacement.data() ? 2 : 0;
    }
    void record(EventId event, std::uint64_t first = 0, std::uint64_t second = 0,
                std::uint64_t third = 0) {
        if (sync) sync();
        trace.push_back({event, first, second, third,
            static_cast<std::uint16_t>(emitter.remaining_98),
            emitter.opened_ac, emitter.effect_started_ae,
            static_cast<std::uint16_t>(emitter.direction_b4), bits(emitter.countdown_c0),
            counts[0], counts[1], counts[2], counts[3], counts[4],
            world.remaining_total_4cbaa0});
    }
    void effect_44521b(std::uint32_t value, std::uint8_t flag) override {
        record(effect, value, flag);
    }
    void update_model_432383(lemmings::EmitterState&, double elapsed) override {
        record(model_update, bits(elapsed));
    }
    std::uint8_t model_finished_44(void*) override {
        record(model_finished);
        return finished_result;
    }
    double model_remainder_48(void*) override {
        record(model_remainder);
        return remainder;
    }
    void* allocate_actor_49c1a8(std::uint32_t size) override {
        record(allocation, size);
        return actor.data();
    }
    void* construct_actor_42de51(void* storage, double x, double y,
                               std::int32_t type) override {
        record(construction, bits(x), bits(y), static_cast<std::uint32_t>(type));
        store(storage, 0x4C, constructed_type);
        return storage;
    }
    void flip_vertical_42f5e3(void* handle) override { record(vertical, identity(handle)); }
    void flip_horizontal_42f66b(void* handle) override { record(horizontal, identity(handle)); }
    std::uint8_t update_actor_14(void* handle, double elapsed) override {
        record(actor_update, identity(handle), bits(elapsed));
        store(handle, 0x4C, updated_type);
        return actor_result;
    }
    void insert_426180(void*& handle) override {
        record(insert_true, identity(handle));
        if (replace_on_insert) handle = replacement.data();
    }
    void insert_4090b0(void* handle) override { record(insert_false, identity(handle)); }
    void register_43b536(void* handle) override { record(registration, identity(handle)); }
    std::int32_t actor_type_4c(void* handle) override { return load<std::int32_t>(handle, 0x4C); }
    void refresh_444abc() override { record(refresh); }
    bool run(double elapsed) { return lemmings::update_emitter(emitter, elapsed, world, *this); }
    std::vector<std::uint64_t> events() const {
        std::vector<std::uint64_t> result;
        for (const auto& event : trace) result.push_back(event[0]);
        return result;
    }
};
} // namespace emitter_test
