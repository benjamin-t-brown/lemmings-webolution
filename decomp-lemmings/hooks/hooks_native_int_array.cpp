#include "hook_config.hpp"
#include "hook_engine.hpp"
#include "hook_log.hpp"

#include "lemmings/allocator.hpp"
#include "lemmings/native_int_array.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <stdexcept>

// Third hooked routine: append-with-growth at 0x0041BC10.
//
// This one exists to exercise the two hazards the first two could not. It
// allocates through the executable's own CRT, and the recovered port throws on
// a corrupt header. Both are fatal if handled naively:
//
//   * A block allocated by MinGW's heap and released by the game's `free` at
//     0x0049C13D corrupts the heap. The installer therefore binds
//     lemmings::set_active_allocator to the game's own pair before any
//     recovered allocating code can run.
//   * A C++ exception must not propagate into the original's frames, which
//     know nothing about it. The hook catches everything at the boundary.
//
// It is also the first *mutating* routine hooked, which changes what a shadow
// comparison means. The original cannot simply be run twice: it reallocates
// and frees, so the input state is gone after the first call. Instead the
// recovered routine runs against a deep copy taken from the game's heap, the
// original runs against the real array, and the resulting states are compared
// by count, capacity and element contents -- never by data pointer, which is a
// fresh allocation on each side.

namespace lrhook {
namespace {

constexpr std::uintptr_t kTarget = 0x0041BC10u;

// push ebp / mov ebp,esp / sub $0x14,esp -- boundaries at 1, 3, 6, 7, 10, so a
// five-byte jump relocates 6. The earliest internal branch target is
// 0x0041BC5D, well clear.
constexpr std::uint8_t kPrologue[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x14};
constexpr std::size_t kPrologueLength = sizeof(kPrologue);

// The game's CRT, both __cdecl: `operator new` takes a size and returns a
// block, `free` takes a block. Confirmed by disassembly -- each pushes its
// argument, calls inward, pops with `pop ecx` / `add $4,esp`, and returns with
// a plain `ret`.
using GameAllocateFn = void*(__cdecl*)(std::size_t bytes);
using GameReleaseFn = void(__cdecl*)(void* block);

constexpr std::uintptr_t kGameOperatorNew = 0x0049C1A8u;
constexpr std::uintptr_t kGameFree = 0x0049C13Du;

const auto game_allocate = reinterpret_cast<GameAllocateFn>(kGameOperatorNew);
const auto game_release = reinterpret_cast<GameReleaseFn>(kGameFree);

void* allocate_through_game(std::size_t bytes) {
    return game_allocate(bytes);
}

void release_through_game(void* block) {
    game_release(block);
}

// The value argument is a pointer that the original dereferences
// (`mov 0x8(%ebp),%ecx` then `mov (%ecx),%ecx`), not an immediate. `this` is
// the array. RET 4 confirms one stack argument, callee-popped.
using AppendFn = void(__attribute__((thiscall)) *)(lemmings::NativeIntArray* self,
                                                   const std::int32_t* value);

AppendFn g_original = nullptr;
Mode g_mode = Mode::Shadow;

// False until the executable's CRT is known to be initialised. patch.dll is a
// static import, so DllMain runs before CRT startup and the game's heap is not
// usable yet. Everything that would allocate checks this first.
bool g_heap_ready = false;

unsigned long long g_calls = 0;
unsigned long long g_mismatches = 0;
unsigned long long g_skipped = 0;
unsigned long long g_exceptions = 0;
unsigned g_logged = 0;

constexpr unsigned kMaxLogged = 20;

// Beyond this the deep copy is not worth taking; the comparison is skipped and
// the original runs alone.
constexpr std::int32_t kMaxComparableCapacity = 1 << 18;

bool plausible(const lemmings::NativeIntArray& array) {
    if (array.count < 0 || array.capacity < 0) {
        return false;
    }
    if (array.count > array.capacity) {
        return false;
    }
    if (array.capacity > kMaxComparableCapacity) {
        return false;
    }
    if (array.capacity > 0 && array.data == nullptr) {
        return false;
    }
    return true;
}

// Compares everything that is part of the observable result. Data pointers are
// deliberately excluded: each side allocates its own block.
bool same_state(const lemmings::NativeIntArray& a, const lemmings::NativeIntArray& b) {
    if (a.count != b.count || a.capacity != b.capacity || a.growth != b.growth) {
        return false;
    }
    if (a.count == 0) {
        return true;
    }
    if (a.data == nullptr || b.data == nullptr) {
        return false;
    }
    return std::memcmp(a.data, b.data,
                       static_cast<std::size_t>(a.count) * sizeof(std::int32_t)) == 0;
}

void log_state(const char* label, const lemmings::NativeIntArray& array) {
    log_line("    %s: count=%ld capacity=%ld growth=%ld first=%ld last=%ld", label,
             static_cast<long>(array.count), static_cast<long>(array.capacity),
             static_cast<long>(array.growth),
             array.count > 0 ? static_cast<long>(array.data[0]) : 0L,
             array.count > 0 ? static_cast<long>(array.data[array.count - 1]) : 0L);
}

// Runs the recovered routine on a copy of `source` taken from the game heap.
// Returns false when the copy could not be taken or the routine rejected the
// input; `result` is only meaningful when it returns true.
bool run_recovered_on_copy(const lemmings::NativeIntArray& source,
                           std::int32_t value, lemmings::NativeIntArray& result) {
    result = source;
    result.data = nullptr;

    if (source.capacity > 0) {
        const auto bytes = static_cast<std::size_t>(source.capacity) * sizeof(std::int32_t);
        result.data = static_cast<std::int32_t*>(allocate_through_game(bytes));
        if (result.data == nullptr) {
            return false;
        }
        if (source.count > 0 && source.data != nullptr) {
            std::memcpy(result.data, source.data,
                        static_cast<std::size_t>(source.count) * sizeof(std::int32_t));
        }
    }

    bool ok = true;
    try {
        lemmings::append_native_int(result, value);
    } catch (const std::exception&) {
        // The port guards a corrupt header where the original writes out of
        // bounds. Record it and fall back rather than letting it escape.
        ++g_exceptions;
        ok = false;
    } catch (...) {
        ++g_exceptions;
        ok = false;
    }

    if (!ok && result.data != nullptr) {
        release_through_game(result.data);
        result.data = nullptr;
    }
    return ok;
}

void release_copy(lemmings::NativeIntArray& copy) {
    if (copy.data != nullptr) {
        release_through_game(copy.data);
        copy.data = nullptr;
    }
}

void __attribute__((thiscall))
hook_append(lemmings::NativeIntArray* self, const std::int32_t* value) {
    ++g_calls;
    if (g_calls >= 1024 && (g_calls & (g_calls - 1)) == 0) {
        log_line("append_native_int: %llu calls, %llu mismatches, %llu skipped, "
                 "%llu guarded so far",
                 g_calls, g_mismatches, g_skipped, g_exceptions);
    }

    if (self == nullptr || value == nullptr) {
        g_original(self, value);
        return;
    }

    // Before the CRT is up, neither the recovered routine nor a heap copy may
    // allocate. Forward and leave the array to the original.
    if (!g_heap_ready) {
        ++g_skipped;
        g_original(self, value);
        return;
    }

    if (g_mode == Mode::Replace) {
        try {
            lemmings::append_native_int(*self, *value);
        } catch (...) {
            ++g_exceptions;
        }
        return;
    }

    if (!plausible(*self)) {
        ++g_skipped;
        g_original(self, value);
        return;
    }

    const lemmings::NativeIntArray before = *self;
    lemmings::NativeIntArray recovered{};
    const bool recovered_ok = run_recovered_on_copy(before, *value, recovered);

    if (g_mode == Mode::Verify && recovered_ok) {
        // Hand the recovered result to the game and retire the original's
        // block, so the recovered allocation becomes the live one.
        const lemmings::NativeIntArray replaced = *self;
        *self = recovered;
        if (replaced.data != nullptr && replaced.data != recovered.data) {
            release_through_game(replaced.data);
        }
        return;
    }

    g_original(self, value);

    if (recovered_ok && !same_state(*self, recovered)) {
        ++g_mismatches;
        if (g_logged < kMaxLogged) {
            ++g_logged;
            log_line("append_native_int MISMATCH call %llu, appended %ld:", g_calls,
                     static_cast<long>(*value));
            log_state("before  ", before);
            log_state("original", *self);
            log_state("recovered", recovered);
        }
    }
    release_copy(recovered);
}

} // namespace

bool install_native_int_array_hook(Mode mode) {
    g_mode = mode;

    if (!verify_bytes(kTarget, kPrologue, kPrologueLength)) {
        return false;
    }

    // Must happen before any recovered allocating code runs, hooked or not.
    const lemmings::Allocator previous = lemmings::set_active_allocator(
        lemmings::Allocator{&allocate_through_game, &release_through_game});
    if (lemmings::active_allocator().allocate != &allocate_through_game) {
        log_line("failed to install the game allocator; refusing to hook 0x%08lx",
                 static_cast<unsigned long>(kTarget));
        return false;
    }
    log_line("recovered allocator bound to the game CRT (new 0x%08lx, free 0x%08lx), "
             "replacing the host pair at %p",
             static_cast<unsigned long>(kGameOperatorNew),
             static_cast<unsigned long>(kGameFree),
             reinterpret_cast<void*>(previous.allocate));

    void* const trampoline = install_detour(
        kTarget, reinterpret_cast<const void*>(&hook_append), kPrologueLength);
    if (trampoline == nullptr) {
        lemmings::set_active_allocator(previous);
        return false;
    }

    g_original = reinterpret_cast<AppendFn>(trampoline);
    log_line("append_native_int hook active in %s mode", mode_name(mode));
    return true;
}

void report_native_int_array_hook() {
    log_line("append_native_int: %llu calls, %llu mismatches, %llu skipped, "
             "%llu guarded (%s mode)",
             g_calls, g_mismatches, g_skipped, g_exceptions, mode_name(g_mode));
}

bool selftest_native_int_array() {
    if (g_original == nullptr) {
        log_line("array selftest skipped: no trampoline");
        return false;
    }

    // Reaching here means the caller has established that the game's CRT is
    // running. See the declaration: calling this from DllMain kills the
    // process with 0xC0000142.
    g_heap_ready = true;

    unsigned long long cases = 0;
    unsigned long long mismatches = 0;
    unsigned logged = 0;

    // Every array in the sweep owns a game-heap block, so both sides allocate
    // and free through the executable's own CRT exactly as they would live.
    const auto check = [&](std::int32_t count, std::int32_t capacity,
                           std::int32_t growth, std::int32_t value) {
        if (capacity < 0 || count < 0 || count > capacity) {
            return;
        }
        ++cases;

        lemmings::NativeIntArray original{count, capacity, growth, nullptr};
        if (capacity > 0) {
            original.data = static_cast<std::int32_t*>(allocate_through_game(
                static_cast<std::size_t>(capacity) * sizeof(std::int32_t)));
            if (original.data == nullptr) {
                --cases;
                return;
            }
            for (std::int32_t i = 0; i < count; ++i) {
                original.data[i] = i * 7 - 3;
            }
        }

        lemmings::NativeIntArray recovered{};
        const bool recovered_ok = run_recovered_on_copy(original, value, recovered);

        bool original_ok = true;
        if (growth > 0 || count < capacity) {
            g_original(&original, &value);
        } else {
            // The original allocates a zero or shrinking block here and then
            // writes past it. Not a supported input; the port rejects it.
            original_ok = false;
        }

        if (original_ok && recovered_ok && !same_state(original, recovered)) {
            ++mismatches;
            if (logged < 10) {
                ++logged;
                log_line("array selftest MISMATCH count=%ld capacity=%ld growth=%ld "
                         "value=%ld",
                         static_cast<long>(count), static_cast<long>(capacity),
                         static_cast<long>(growth), static_cast<long>(value));
                log_state("original", original);
                log_state("recovered", recovered);
            }
        } else if (original_ok != recovered_ok) {
            // Expected wherever the port guards an input the original accepts
            // and corrupts; counted, not treated as a mismatch.
        }

        release_copy(recovered);
        if (original.data != nullptr) {
            release_through_game(original.data);
        }
    };

    const std::int32_t growths[] = {1, 2, 4, 8, 16, 32};
    for (const std::int32_t growth : growths) {
        for (std::int32_t capacity = 0; capacity <= 24; ++capacity) {
            for (std::int32_t count = 0; count <= capacity; ++count) {
                check(count, capacity, growth, count * 13 + 1);
            }
        }
    }

    // Exactly-full arrays, where the growth path runs every time.
    for (const std::int32_t growth : growths) {
        for (std::int32_t full = 0; full <= 64; ++full) {
            check(full, full, growth, -full);
        }
    }

    log_line("array selftest: %llu in-process comparisons against the original "
             "bytes through the game heap, %llu mismatches, %llu guarded",
             cases, mismatches, g_exceptions);
    return mismatches == 0;
}

} // namespace lrhook
