#include "hook_config.hpp"
#include "hook_engine.hpp"
#include "hook_log.hpp"

#include "lemmings/cylinder_coordinate.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

// Second hooked routine: cylinder X wrapping at 0x0040DEC0.
//
// Chosen specifically to exercise the x87 hazard. The original is entirely
// floating point, returns its result on the register stack, and rounds through
// memory on every pass of its subtraction loop (`fstpl 0x8(%ebp)` at
// 0x0040DF20) because it writes back to its own argument slot. A port that
// keeps the running value in a register instead would diverge for large
// inputs, and only an in-process comparison under the game's own FPU control
// word can show whether it does.

namespace lrhook {
namespace {

constexpr std::uintptr_t kTarget = 0x0040DEC0u;

// push ebp / mov ebp,esp / push ecx / mov [ebp-4],ecx -- identical shape to
// 0x0040DE60, so again 7 bytes relocate. The earliest internal branch target
// is 0x0040DEEC, well past the relocated range.
constexpr std::uint8_t kPrologue[] = {0x55, 0x8B, 0xEC, 0x51, 0x89, 0x4D, 0xFC};
constexpr std::size_t kPrologueLength = sizeof(kPrologue);

// `this` arrives in ECX (stored at [ebp-4] and never read), the double sits at
// [ebp+8], and the routine ends in RET 8. The result is left in ST0.
using WrapCylinderXFn =
    long double (__attribute__((thiscall)) *)(void* self, double x);

WrapCylinderXFn g_original = nullptr;
Mode g_mode = Mode::Shadow;

unsigned long long g_calls = 0;
unsigned long long g_mismatches = 0;
unsigned g_logged_mismatches = 0;

constexpr unsigned kMaxLoggedMismatches = 20;

bool same_result(long double a, long double b) {
    if (a == b) {
        return true;
    }
    return std::isnan(a) && std::isnan(b);
}

long double __attribute__((thiscall)) hook_wrap_cylinder_x(void* self, double x) {
    ++g_calls;
    if (g_calls >= 1024 && (g_calls & (g_calls - 1)) == 0) {
        log_line("wrap_cylinder_x: %llu calls, %llu mismatches so far", g_calls,
                 g_mismatches);
    }

    if (g_mode == Mode::Replace) {
        return lemmings::wrap_cylinder_x(x);
    }

    const long double original = g_original(self, x);
    const long double recovered = lemmings::wrap_cylinder_x(x);

    if (!same_result(original, recovered)) {
        ++g_mismatches;
        if (g_logged_mismatches < kMaxLoggedMismatches) {
            ++g_logged_mismatches;
            log_line("wrap_cylinder_x MISMATCH call %llu: x=%.17g "
                     "original=%.17g recovered=%.17g",
                     g_calls, x, static_cast<double>(original),
                     static_cast<double>(recovered));
        }
    }

    return g_mode == Mode::Verify ? recovered : original;
}

} // namespace

bool install_cylinder_coordinate_hook(Mode mode) {
    g_mode = mode;

    if (!verify_bytes(kTarget, kPrologue, kPrologueLength)) {
        return false;
    }

    void* const trampoline =
        install_detour(kTarget, reinterpret_cast<const void*>(&hook_wrap_cylinder_x),
                       kPrologueLength);
    if (trampoline == nullptr) {
        return false;
    }

    g_original = reinterpret_cast<WrapCylinderXFn>(trampoline);
    log_line("wrap_cylinder_x hook active in %s mode", mode_name(mode));
    return true;
}

void report_cylinder_coordinate_hook() {
    log_line("wrap_cylinder_x: %llu calls, %llu mismatches (%s mode)", g_calls,
             g_mismatches, mode_name(g_mode));
}

bool selftest_cylinder_coordinate() {
    if (g_original == nullptr) {
        log_line("cylinder selftest skipped: no trampoline");
        return false;
    }

    unsigned long long cases = 0;
    unsigned long long mismatches = 0;
    unsigned logged = 0;

    const auto check = [&](double x) {
        ++cases;
        const long double original = g_original(&cases, x);
        const long double recovered = lemmings::wrap_cylinder_x(x);
        if (same_result(original, recovered)) {
            return;
        }
        ++mismatches;
        if (logged < 10) {
            ++logged;
            log_line("cylinder selftest MISMATCH x=%.17g original=%.17g "
                     "recovered=%.17g",
                     x, static_cast<double>(original),
                     static_cast<double>(recovered));
        }
    };

    // Positive infinity is excluded deliberately: the original's subtraction
    // loop never terminates for it (0x0040DF07 through 0x0040DF23), so feeding
    // it here would hang the game. The port documents the same exclusion.
    const double edges[] = {
        0.0,
        -0.0,
        9.99999993922529e-9,   // the threshold constant at 0x004B0698
        9.9e-9,
        1.0e-8,
        -9.99999993922529e-9,
        -1.0e-8,
        1.0,
        -1.0,
        0.5,
        255.0,
        255.999,
        256.0,
        256.000001,
        257.0,
        511.0,
        512.0,
        768.0,
        1023.75,
        -255.0,
        -256.0,
        -257.0,
        -1000.0,
        4.9e-324,              // smallest denormal
        -4.9e-324,
        std::nan(""),
        -std::numeric_limits<double>::infinity(),
    };
    for (const double x : edges) {
        check(x);
    }

    // Deterministic sweep. Positive magnitudes stay bounded because the
    // original iterates once per 256 units, so large values are slow rather
    // than interesting.
    std::uint64_t state = 0x40DEC0ULL * 6364136223846793005ULL + 1442695040888963407ULL;
    for (int i = 0; i < 4000; ++i) {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        const auto unit =
            static_cast<double>((state >> 11) & 0x1FFFFFFFFFFFFFULL) / 9007199254740992.0;
        const double magnitude = unit * 100000.0;
        check((i % 2) == 0 ? magnitude : -magnitude);
    }

    // Values just under and just over each multiple of 256, where the loop's
    // exit condition and its memory round-trip matter most.
    for (int multiple = 1; multiple <= 64; ++multiple) {
        const double centre = 256.0 * multiple;
        check(std::nextafter(centre, 0.0));
        check(centre);
        check(std::nextafter(centre, 1.0e300));
    }

    log_line("cylinder selftest: %llu in-process comparisons against the "
             "original bytes, %llu mismatches",
             cases, mismatches);
    return mismatches == 0;
}

} // namespace lrhook
