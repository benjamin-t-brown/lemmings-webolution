#include "lemmings/model_update.hpp"

#include <cmath>
#include <cstdint>

namespace lemmings {
namespace {
bool even_truncated_x87(long double value) {
    // 0x0049C5AC changes RC to truncate and executes FISTP qword. Invalid
    // masked conversion produces INT64_MIN, whose low bit is zero.
    constexpr long double limit = 9223372036854775808.0L;
    if (!(value >= -limit && value < limit)) return true;
    const auto converted = static_cast<std::int64_t>(value);
    return (static_cast<std::uint64_t>(converted) & 1u) == 0;
}
}

bool update_model_entity(ModelUpdateState& state, double elapsed,
                         const ModelUpdateCallbacks& callbacks) {
    if (state.model != nullptr) {
        callbacks.advance(state.model, elapsed);
        if (state.remove_when_finished != 0) {
            return callbacks.finished(state.model) == 0;
        }
    }
    return true; // 0x00423470.
}

bool update_blinking_model(ModelUpdateState& state, double elapsed,
                           const ModelUpdateCallbacks& callbacks) {
    if (state.blink_mode != 0) {
        bool visible = state.blink_mode == 1;
        state.blink_remaining = static_cast<double>(
            static_cast<long double>(state.blink_remaining) -
            static_cast<long double>(elapsed));
        if (!(state.blink_remaining >= 0.0)) {
            // FCOMP / TEST AH,1 also takes this path for unordered values.
            state.blink_remaining = 0.0;
            state.blink_mode = 0;
        } else if (even_truncated_x87(
                       static_cast<long double>(state.blink_remaining) /
                       static_cast<long double>(0.2))) {
            visible = !visible;
        }
        const double value = visible ? callbacks.full_value(state.model) : 0.0;
        callbacks.set_value(state.model, value);
    }
    return update_model_entity(state, elapsed, callbacks);
}

void enable_blinking_model(ModelUpdateState& state) {
    state.blink_mode = 1;
    state.blink_remaining = 1.0;
    state.enabled = 1;
}
void disable_blinking_model(ModelUpdateState& state) {
    state.blink_mode = 2;
    state.blink_remaining = 1.0;
    state.enabled = 0;
}

} // namespace lemmings
