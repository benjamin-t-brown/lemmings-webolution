// Offline differential check for 0x00430C9C and its two relocatable helpers.
// The Python wrapper verifies user-owned executable bytes before sending them.
#include "lemmings/gadget_subboxes.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>

namespace {
constexpr std::uintptr_t image_base = 0x00400000;
constexpr std::size_t image_size = 0x40000;
constexpr std::uintptr_t count_address = 0x00428670;
constexpr std::uintptr_t index_address = 0x00431370;
constexpr std::uintptr_t subbox_address = 0x00430C9C;
using NativeSubbox = int (__attribute__((thiscall)) *)(
    const lemmings::GadgetSubboxHead*, const double*,
    const lemmings::ActorHead*);

template<std::size_t Size>
bool read_at(std::uintptr_t address) {
    std::array<char, Size> bytes{};
    std::cin.read(bytes.data(), bytes.size());
    if (std::cin.gcount() != static_cast<std::streamsize>(bytes.size())) {
        return false;
    }
    std::memcpy(reinterpret_cast<void*>(address), bytes.data(), bytes.size());
    return true;
}

bool matches(NativeSubbox native, const lemmings::GadgetSubboxHead& gadget,
             const lemmings::ActorHead& actor, int case_number) {
    const int original = native(&gadget, actor.position, &actor);
    const int recovered = lemmings::containing_subbox_index(gadget, actor);
    if (original == recovered) return true;
    std::cerr << "subbox mismatch at case " << case_number
              << ": original=" << original << " recovered=" << recovered
              << " count=" << gadget.local_lower.count
              << " facing=" << actor.facing << '\n';
    for (int axis = 0; axis != 3; ++axis) {
        std::cerr << " axis " << axis << " origin=" << gadget.bounds.minimum[axis]
                  << " point=" << actor.position[axis] << '\n';
    }
    return false;
}
} // namespace

int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    void* image = VirtualAlloc(reinterpret_cast<void*>(image_base), image_size,
                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(image_base)) {
        std::cerr << "could not map original function addresses\n";
        return 3;
    }
    if (!read_at<16>(count_address) || !read_at<27>(index_address) ||
        !read_at<584>(subbox_address)) {
        std::cerr << "expected verified function bytes on stdin\n";
        return 2;
    }
    DWORD previous_protection = 0;
    if (!VirtualProtect(image, image_size, PAGE_EXECUTE_READ,
                        &previous_protection)) {
        std::cerr << "VirtualProtect failed\n";
        return 2;
    }
    FlushInstructionCache(GetCurrentProcess(), image, image_size);
    auto native = reinterpret_cast<NativeSubbox>(subbox_address);

    lemmings::GadgetSubboxHead gadget{};
    lemmings::Vec3d lower[2]{};
    lemmings::Vec3d upper[2]{};
    gadget.local_lower.data = lower;
    gadget.local_upper.data = upper;
    lemmings::ActorHead actor{};
    int case_number = 0;

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const double special[] = {-inf, -1.0, -0.0, 0.0, 1.0, inf, nan};
    gadget.local_lower.count = 1;
    gadget.local_upper.count = 1;
    for (int facing : {-1, 0, 1, 2}) {
        actor.facing = facing;
        for (int axis = 0; axis != 3; ++axis) {
            for (double low : special) {
                for (double point : special) {
                    for (double high : special) {
                        for (int i = 0; i != 3; ++i) {
                            gadget.bounds.minimum[i] = 0.0;
                            lower[0].value[i] = -2.0;
                            upper[0].value[i] = 2.0;
                            actor.position[i] = 0.0;
                        }
                        lower[0].value[axis] = low;
                        upper[0].value[axis] = high;
                        actor.position[axis] = point;
                        if (!matches(native, gadget, actor, ++case_number)) return 1;
                    }
                }
            }
        }
    }

    std::mt19937_64 random(0x430C9C);
    std::uniform_real_distribution<double> values(-1000.0, 1000.0);
    for (int trial = 0; trial != 10000; ++trial) {
        gadget.local_lower.count = trial % 3;
        gadget.local_upper.count = gadget.local_lower.count;
        actor.facing = trial % 4 == 0 ? 0 : (trial % 2 == 0 ? -1 : 1);
        for (int axis = 0; axis != 3; ++axis) {
            gadget.bounds.minimum[axis] = values(random);
            actor.position[axis] = values(random);
            for (int box = 0; box != 2; ++box) {
                lower[box].value[axis] = values(random);
                upper[box].value[axis] = values(random);
            }
        }
        if (trial % 7 == 0 && gadget.local_lower.count != 0) {
            for (int axis = 0; axis != 3; ++axis) {
                gadget.bounds.minimum[axis] = 0.0;
                lower[0].value[axis] = -1.0;
                upper[0].value[axis] = 1.0;
                actor.position[axis] = values(random) / 1000.0;
            }
        }
        if (!matches(native, gadget, actor, ++case_number)) return 1;
    }

    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native subbox comparison passed " << case_number << " cases\n";
}
