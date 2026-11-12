// Offline comparison of original gadget-bounds setters against recovered code.
#include "lemmings/gadget_bounds.hpp"
#include "lemmings/collision_grid.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <random>

namespace {
constexpr std::uintptr_t image_base = 0x00400000;
constexpr std::size_t image_size = 0xD0000;
using NativeXY = void (__attribute__((thiscall)) *)(
    lemmings::GadgetBoundsHead*, double, double, double, double);
using NativeXYZ = void (__attribute__((thiscall)) *)(
    lemmings::GadgetBoundsHead*, double, double, double,
    double, double, double);
using NativeMode = void (__attribute__((thiscall)) *)(
    lemmings::GadgetBoundsHead*, const double*, const double*, int);

template<std::size_t Size>
bool read_at(std::uintptr_t address) {
    std::array<char, Size> bytes{};
    std::cin.read(bytes.data(), bytes.size());
    if (std::cin.gcount() != static_cast<std::streamsize>(bytes.size())) return false;
    std::memcpy(reinterpret_cast<void*>(address), bytes.data(), bytes.size());
    return true;
}

bool same_bounds(const lemmings::GadgetBoundsHead& native,
                 const lemmings::GadgetBoundsHead& recovered) {
    return std::memcmp(native.minimum, recovered.minimum,
                       sizeof(native.minimum) + sizeof(native.maximum)) == 0;
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
    if (!read_at<60>(0x004017F0) || !read_at<20>(0x0040BAA0) ||
        !read_at<110>(0x0040DEC0) || !read_at<21>(0x0040DF30) ||
        !read_at<85>(0x0040DF50) || !read_at<223>(0x00430FE1) ||
        !read_at<219>(0x004310C0) || !read_at<314>(0x004338C2) ||
        !read_at<181>(0x0049C8F4) || !read_at<35>(0x0049FC56) ||
        !read_at<8>(0x004B0338) || !read_at<8>(0x004B0698) ||
        !read_at<8>(0x004B07A8)) {
        std::cerr << "expected verified code and constant bytes on stdin\n";
        return 2;
    }
    lemmings::CollisionGridHead grid{};
    grid.height = 96;
    std::memcpy(reinterpret_cast<void*>(0x004C6A10), &grid, sizeof(grid));
    DWORD old_protection = 0;
    if (!VirtualProtect(image, image_size, PAGE_EXECUTE_READ, &old_protection)) {
        std::cerr << "VirtualProtect failed\n";
        return 2;
    }
    FlushInstructionCache(GetCurrentProcess(), image, image_size);
    auto xy = reinterpret_cast<NativeXY>(0x00430FE1);
    auto xyz = reinterpret_cast<NativeXYZ>(0x004310C0);
    auto mode = reinterpret_cast<NativeMode>(0x004338C2);

    int cases = 0;
    auto compare = [&](double x0, double y0, double x1, double y1,
                       double z0, double z1, int mode_value) {
        lemmings::GadgetBoundsHead original{};
        lemmings::GadgetBoundsHead recovered{};
        xy(&original, x0, y0, x1, y1);
        lemmings::set_gadget_bounds_xy(recovered, grid, x0, y0, x1, y1);
        ++cases;
        if (!same_bounds(original, recovered)) {
            std::cerr << "XY setter mismatch at case " << cases << '\n';
            return false;
        }
        original = {};
        recovered = {};
        xyz(&original, x0, y0, z0, x1, y1, z1);
        lemmings::set_gadget_bounds_xyz(recovered, grid,
                                        x0, y0, z0, x1, y1, z1);
        ++cases;
        if (!same_bounds(original, recovered)) {
            std::cerr << "XYZ setter mismatch at case " << cases << '\n';
            return false;
        }
        original = {};
        recovered = {};
        const double first[2] = {x0, y0};
        const double second[2] = {x1, y1};
        mode(&original, first, second, mode_value);
        lemmings::set_gadget_bounds_for_mode(recovered, grid,
                                             x0, y0, x1, y1, mode_value);
        ++cases;
        if (!same_bounds(original, recovered)) {
            std::cerr << "mode setter mismatch at case " << cases
                      << " mode=" << mode_value << '\n';
            return false;
        }
        return true;
    };

    for (int mode_value : {-1, 0, 1, 2, 3}) {
        if (!compare(-1.0, 30.0, 257.0, 10.0, -2.0, 3.0,
                     mode_value)) return 1;
        if (!compare(255.5, -3.0, -257.0, 120.0, 0.5, 1.0,
                     mode_value)) return 1;
    }
    std::mt19937_64 random(0x4338C2);
    std::uniform_real_distribution<double> x_values(-600.0, 600.0);
    std::uniform_real_distribution<double> y_values(-100.0, 200.0);
    std::uniform_real_distribution<double> z_values(-2.0, 3.0);
    for (int trial = 0; trial != 5000; ++trial) {
        if (!compare(x_values(random), y_values(random),
                     x_values(random), y_values(random),
                     z_values(random), z_values(random),
                     static_cast<int>(random() % 5) - 1)) return 1;
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native bounds setters passed " << cases << " comparisons\n";
}
