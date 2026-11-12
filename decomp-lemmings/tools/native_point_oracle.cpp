// Offline differential check for the relocatable 0x00430BA1 function body.
// Compiled only as a 32-bit Windows process; original bytes arrive on stdin
// from run_native_point_oracle.py after digest verification.
#include "lemmings/gadget_bounds.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>

namespace {
constexpr std::size_t function_size = 146;
using NativePointTest = unsigned char (__attribute__((thiscall)) *)(
    const lemmings::GadgetBoundsHead*, const double*);

bool matches(NativePointTest native, const lemmings::GadgetBoundsHead& gadget,
             const double (&point)[3], int case_number) {
    const bool expected = native(&gadget, point) != 0;
    const bool recovered = lemmings::gadget_contains_point(gadget, point);
    if (expected == recovered) {
        return true;
    }
    std::cerr << "mismatch at case " << case_number << ": native=" << expected
              << " recovered=" << recovered << '\n';
    for (int axis = 0; axis != 3; ++axis) {
        std::cerr << " axis " << axis << " min=" << gadget.minimum[axis]
                  << " point=" << point[axis] << " max=" << gadget.maximum[axis]
                  << '\n';
    }
    return false;
}
} // namespace

int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    std::array<std::uint8_t, function_size> bytes{};
    std::cin.read(reinterpret_cast<char*>(bytes.data()), bytes.size());
    if (std::cin.gcount() != static_cast<std::streamsize>(bytes.size())) {
        std::cerr << "expected exactly 146 verified function bytes on stdin\n";
        return 2;
    }

    void* memory = VirtualAlloc(nullptr, 4096, MEM_RESERVE | MEM_COMMIT,
                                PAGE_EXECUTE_READWRITE);
    if (memory == nullptr) {
        std::cerr << "VirtualAlloc failed\n";
        return 2;
    }
    std::memcpy(memory, bytes.data(), bytes.size());
    FlushInstructionCache(GetCurrentProcess(), memory, bytes.size());
    auto native = reinterpret_cast<NativePointTest>(memory);

    lemmings::GadgetBoundsHead gadget{};
    const double quiet_nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    const double special[] = {-infinity, -1.0, -0.0, 0.0, 1.0,
                              infinity, quiet_nan};
    int case_number = 0;
    bool all_match = true;

    // Cover every axis and all lower/point/upper special-value combinations.
    for (int axis = 0; axis != 3; ++axis) {
        for (double lower : special) {
            for (double point_value : special) {
                for (double upper : special) {
                    for (int i = 0; i != 3; ++i) {
                        gadget.minimum[i] = -2.0;
                        gadget.maximum[i] = 2.0;
                    }
                    gadget.minimum[axis] = lower;
                    gadget.maximum[axis] = upper;
                    const double point[3] = {
                        axis == 0 ? point_value : 0.0,
                        axis == 1 ? point_value : 0.0,
                        axis == 2 ? point_value : 0.0
                    };
                    all_match &= matches(native, gadget, point, ++case_number);
                }
            }
        }
    }

    std::mt19937_64 random(0x430BA1);
    std::uniform_real_distribution<double> values(-1000.0, 1000.0);
    for (int trial = 0; trial != 10000; ++trial) {
        double point[3];
        for (int axis = 0; axis != 3; ++axis) {
            gadget.minimum[axis] = values(random);
            gadget.maximum[axis] = values(random);
            point[axis] = values(random);
            if (trial % 9 == 0) point[axis] = gadget.minimum[axis];
            if (trial % 11 == 0) point[axis] = gadget.maximum[axis];
        }
        all_match &= matches(native, gadget, point, ++case_number);
    }

    VirtualFree(memory, 0, MEM_RELEASE);
    if (!all_match) return 1;
    std::cout << "native byte comparison passed " << case_number << " cases\n";
}
