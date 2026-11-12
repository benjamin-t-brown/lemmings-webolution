#pragma once

#include <cstdint>
#include <span>

namespace lemmings {

// Logical view of fields used by 0x00425A95. Suffixes are original offsets;
// this portable structure deliberately does not reproduce the x86 layout.
struct EmitterState {
    void* model_44 = nullptr;
    std::int16_t remaining_98 = 0;
    std::int16_t coordinate_9a = 0;
    std::int16_t coordinate_9c = 0;
    std::uint8_t opened_ac = 0;
    std::uint8_t flip_vertical_ad = 0;
    std::uint8_t effect_started_ae = 0;
    std::int32_t actor_type_b0 = 0;
    std::int16_t direction_b4 = 0;
    std::uint8_t alternate_b6 = 0;
    double interval_b8 = 0;
    double countdown_c0 = 0;
};

struct EmitterWorld {
    std::uint8_t suppress_effect_4e32a0 = 0;
    std::uint32_t effect_value_4e32b4 = 0;
    std::span<std::uint16_t> remaining_by_type_4cba8c;
    std::uint16_t remaining_total_4cbaa0 = 0;
};

// Explicit boundaries of this recovered routine, not implementations of the
// called animation, actor, sound, container, or UI systems. Callbacks can observe
// and update the surrounding state just as the original callees can.
struct EmitterCallbacks {
    virtual ~EmitterCallbacks() = default;
    virtual void effect_44521b(std::uint32_t value, std::uint8_t flag) = 0;
    virtual void update_model_432383(EmitterState& emitter, double elapsed) = 0;
    virtual std::uint8_t model_finished_44(void* model) = 0;
    virtual double model_remainder_48(void* model) = 0;
    virtual void* allocate_actor_49c1a8(std::uint32_t bytes) = 0;
    virtual void* construct_actor_42de51(void* storage, double coordinate_9a,
                                       double coordinate_9c, std::int32_t type) = 0;
    virtual void flip_vertical_42f5e3(void* actor) = 0;
    virtual void flip_horizontal_42f66b(void* actor) = 0;
    virtual std::uint8_t update_actor_14(void* actor, double elapsed) = 0;
    virtual void insert_426180(void*& actor) = 0;
    virtual void insert_4090b0(void* actor) = 0;
    virtual void register_43b536(void* actor) = 0;
    // Projects the actor's signed 32-bit field at +0x4C; not an original call.
    virtual std::int32_t actor_type_4c(void* actor) = 0;
    virtual void refresh_444abc() = 0;
};

// Always returns true (original AL=1). Emits at most one actor per update.
// Valid-object contract: actor allocation/construction succeed, callback actor
// handles are valid, and actor_type_4c is within the counter span. The original
// has no recovery from invalid handles/indices; native fault behavior is outside
// this source model. Floating point exceptions must be masked, as in the tests.
bool update_emitter(EmitterState& emitter, double elapsed, EmitterWorld& world,
                    EmitterCallbacks& callbacks);

} // namespace lemmings
