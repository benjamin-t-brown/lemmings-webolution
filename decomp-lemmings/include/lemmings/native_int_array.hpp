#pragma once

#include <cstddef>
#include <cstdint>

namespace lemmings {

// The game's array of 32-bit integers, used by 0x00430F4D.
struct NativeIntArray {
    std::int32_t count;
    std::int32_t capacity;
    std::int32_t growth;
    std::int32_t* data;
};

#if defined(INTPTR_MAX) && defined(INT32_MAX) && INTPTR_MAX == INT32_MAX
static_assert(offsetof(NativeIntArray, data) == 0xC);
static_assert(sizeof(NativeIntArray) == 0x10);
#endif

// Both routines take their memory from lemmings::active_allocator(), not from
// the global operators. Offline builds get the host pair by default; code
// running inside the game must install the game's own CRT pair first, because
// the original releases these blocks through its own `free`.

// Recovered from 0x00406650 / 0x00406AE0.
void clear_native_int_array(NativeIntArray& array);

// Recovered from 0x0041BC10. A positive growth step is required by the
// original allocation path whenever capacity is exhausted.
//
// Throws std::length_error on a corrupt array header, where the original would
// write out of bounds instead. A caller running inside the game must not let
// that escape into the original's frames.
void append_native_int(NativeIntArray& array, std::int32_t value);

} // namespace lemmings
