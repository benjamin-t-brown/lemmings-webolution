#include "hook_config.hpp"
#include "hook_engine.hpp"
#include "hook_log.hpp"

#include "lemmings/collision_grid.hpp"

#include <cstdint>

// Phase 1 target: the collision grid cell accessor at 0x0040DE60.
//
// Chosen because it is a leaf routine that allocates nothing, dereferences
// nothing it returns, and already has 20,248 offline original-byte
// comparisons behind it (docs/evidence/0040de60.md). If the detour mechanism,
// the thiscall boundary and the struct offsets are right, this hook runs
// thousands of times per level with zero observable effect on the game.

namespace lrhook {
namespace {

constexpr std::uintptr_t kTarget = 0x0040DE60u;

// push ebp / mov ebp,esp / push ecx / mov [ebp-4],ecx
// Instruction boundaries sit at 4 and 7, so a five-byte jump must relocate 7.
constexpr std::uint8_t kPrologue[] = {0x55, 0x8B, 0xEC, 0x51, 0x89, 0x4D, 0xFC};
constexpr std::size_t kPrologueLength = sizeof(kPrologue);

// `this` in ECX, two stack arguments, callee pops: RET 8.
using CollisionCellFn =
    std::int32_t* (__attribute__((thiscall)) *)(void* self, std::uint32_t x,
                                                std::int32_t y);

CollisionCellFn g_original = nullptr;
Mode g_mode = Mode::Shadow;

unsigned long long g_calls = 0;
unsigned long long g_mismatches = 0;
unsigned g_logged_mismatches = 0;

constexpr unsigned kMaxLoggedMismatches = 20;

std::int32_t* __attribute__((thiscall))
hook_collision_cell(void* self, std::uint32_t x, std::int32_t y) {
    auto& grid = *static_cast<lemmings::CollisionGridHead*>(self);
    ++g_calls;

    // Terminating the game from outside skips DLL_PROCESS_DETACH, so record a
    // running tally at powers of two. That bounds the log to a few dozen lines
    // while still leaving evidence behind after an unclean exit.
    if (g_calls >= 1024 && (g_calls & (g_calls - 1)) == 0) {
        log_line("collision_cell: %llu calls, %llu mismatches so far", g_calls,
                 g_mismatches);
    }

    if (g_mode == Mode::Replace) {
        return lemmings::collision_cell(grid, x, y);
    }

    std::int32_t* const original = g_original(self, x, y);
    std::int32_t* const recovered = lemmings::collision_cell(grid, x, y);

    if (original != recovered) {
        ++g_mismatches;
        if (g_logged_mismatches < kMaxLoggedMismatches) {
            ++g_logged_mismatches;
            log_line("collision_cell MISMATCH call %llu: self=0x%08lx x=%lu y=%ld "
                     "height=%ld data=0x%08lx original=0x%08lx recovered=0x%08lx",
                     g_calls,
                     static_cast<unsigned long>(reinterpret_cast<std::uintptr_t>(self)),
                     static_cast<unsigned long>(x), static_cast<long>(y),
                     static_cast<long>(grid.height),
                     static_cast<unsigned long>(reinterpret_cast<std::uintptr_t>(grid.cells.data)),
                     static_cast<unsigned long>(reinterpret_cast<std::uintptr_t>(original)),
                     static_cast<unsigned long>(reinterpret_cast<std::uintptr_t>(recovered)));
        }
    }

    return g_mode == Mode::Verify ? recovered : original;
}

} // namespace

bool install_collision_grid_hook(Mode mode) {
    g_mode = mode;

    if (!verify_bytes(kTarget, kPrologue, kPrologueLength)) {
        return false;
    }

    void* const trampoline = install_detour(
        kTarget, reinterpret_cast<const void*>(&hook_collision_cell), kPrologueLength);
    if (trampoline == nullptr) {
        return false;
    }

    g_original = reinterpret_cast<CollisionCellFn>(trampoline);
    log_line("collision_cell hook active in %s mode", mode_name(mode));
    return true;
}

void report_collision_grid_hook() {
    log_line("collision_cell: %llu calls, %llu mismatches (%s mode)", g_calls,
             g_mismatches, mode_name(g_mode));
}

bool selftest_collision_grid() {
    if (g_original == nullptr) {
        log_line("selftest skipped: no trampoline");
        return false;
    }

    // The accessor never dereferences the cell pointer it returns, so a fixed
    // non-null data pointer is enough to compare the computed addresses. This
    // keeps the sweep off both heaps.
    alignas(8) static unsigned char storage[sizeof(lemmings::CollisionGridHead)] = {};
    auto& grid = *reinterpret_cast<lemmings::CollisionGridHead*>(storage);
    grid.cells.data = reinterpret_cast<std::int32_t*>(0x10000000u);

    const std::int32_t heights[] = {-2, -1, 0, 1, 2, 3, 5, 17, 64, 255, 256, 1024};
    const std::int32_t edge_y[] = {-4096, -2, -1, 0, 1, 2, 16, 254, 255, 256, 257, 4096};
    const std::uint32_t edge_x[] = {0u,   1u,   15u,        16u,        254u,
                                    255u, 256u, 257u,       4095u,      0x7FFFFFFFu,
                                    0x80000000u,            0xFFFFFFFFu};

    unsigned long long cases = 0;
    unsigned long long mismatches = 0;
    unsigned logged = 0;

    const auto check = [&](std::uint32_t x, std::int32_t y) {
        ++cases;
        std::int32_t* const original = g_original(&grid, x, y);
        std::int32_t* const recovered = lemmings::collision_cell(grid, x, y);
        if (original == recovered) {
            return;
        }
        ++mismatches;
        if (logged < 10) {
            ++logged;
            log_line("selftest MISMATCH height=%ld x=%lu y=%ld original=0x%08lx "
                     "recovered=0x%08lx",
                     static_cast<long>(grid.height), static_cast<unsigned long>(x),
                     static_cast<long>(y),
                     static_cast<unsigned long>(reinterpret_cast<std::uintptr_t>(original)),
                     static_cast<unsigned long>(reinterpret_cast<std::uintptr_t>(recovered)));
        }
    };

    for (const std::int32_t height : heights) {
        grid.height = height;
        for (const std::uint32_t x : edge_x) {
            for (const std::int32_t y : edge_y) {
                check(x, y);
            }
        }
        // Deterministic xorshift sweep over the wider input space.
        std::uint32_t state = 0x1DE60000u ^ static_cast<std::uint32_t>(height);
        for (int i = 0; i < 1500; ++i) {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            const std::uint32_t x = state;
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            const auto y = static_cast<std::int32_t>(state % 2048u) - 512;
            check(x, y);
        }
    }

    log_line("selftest: %llu in-process comparisons against the original bytes, "
             "%llu mismatches",
             cases, mismatches);
    return mismatches == 0;
}

} // namespace lrhook
