#pragma once

#include <cstdint>

namespace lemmings {

// Known state used by 0x00432383 and the blinking subclass 0x0043416C.
// Pointer layout is host-native; comments retain original x86 offsets.
struct ModelUpdateState {
    std::uint8_t enabled = 0;              // +0x38
    std::uint8_t remove_when_finished = 0; // +0x40
    void* model = nullptr;                // +0x44
    double blink_remaining = 0.0;         // +0x48
    std::int32_t blink_mode = 0;           // +0x50
};

// Explicit boundaries for unrecovered model vtable methods.
struct ModelUpdateCallbacks {
    void (*advance)(void* model, double elapsed);  // +0x2C
    std::uint8_t (*finished)(void* model);         // +0x44
    double (*full_value)(void* model);            // +0x4C
    void (*set_value)(void* model, double value);  // +0x38
};

bool update_model_entity(ModelUpdateState& state, double elapsed,
                         const ModelUpdateCallbacks& callbacks);
bool update_blinking_model(ModelUpdateState& state, double elapsed,
                           const ModelUpdateCallbacks& callbacks);

// 0x00434261 / 0x00434290: start a one-second transition and set enabled.
void enable_blinking_model(ModelUpdateState& state);
void disable_blinking_model(ModelUpdateState& state);

} // namespace lemmings
