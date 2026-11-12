// Offline comparison of 0x0040DF50 against the recovered Y clamp.
#include "lemmings/cylinder_coordinate.hpp"
#include "lemmings/collision_grid.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>

namespace {
constexpr std::uintptr_t image_base = 0x00400000;
constexpr std::size_t image_size = 0xC0000;
using NativeY = long double (__attribute__((thiscall)) *)(
    const lemmings::CollisionGridHead*, double);

template<std::size_t Size>
bool read_at(std::uintptr_t address) {
    std::array<char, Size> bytes{};
    std::cin.read(bytes.data(), bytes.size());
    if (std::cin.gcount() != static_cast<std::streamsize>(bytes.size())) return false;
    std::memcpy(reinterpret_cast<void*>(address), bytes.data(), bytes.size());
    return true;
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
    if (!read_at<20>(0x0040BAA0) || !read_at<85>(0x0040DF50) ||
        !read_at<8>(0x004B0338)) {
        std::cerr << "expected verified code and constant bytes on stdin\n";
        return 2;
    }
    DWORD old_protection = 0;
    if (!VirtualProtect(image, image_size, PAGE_EXECUTE_READ, &old_protection)) {
        std::cerr << "VirtualProtect failed\n";
        return 2;
    }
    FlushInstructionCache(GetCurrentProcess(), image, image_size);
    auto native = reinterpret_cast<NativeY>(0x0040DF50);

    lemmings::CollisionGridHead grid{};
    int cases = 0;
    auto compare = [&](double y) {
        const long double original = native(&grid, y);
        const long double recovered = lemmings::clamp_cylinder_y(grid, y);
        ++cases;
        if (original == recovered &&
            (original != 0.0L || std::signbit(original) == std::signbit(recovered))) {
            return true;
        }
        std::cerr << "Y mismatch height=" << grid.height << " y=" << y
                  << " original=" << static_cast<double>(original)
                  << " recovered=" << static_cast<double>(recovered) << '\n';
        return false;
    };

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const double special[] = {-inf, -100.0, -1.0, -0.0, 0.0,
                              1.0, 95.0, 96.0, 100.0, inf, nan};
    for (int height : {-2, 0, 1, 96}) {
        grid.height = height;
        for (double y : special) {
            if (!compare(y)) return 1;
        }
    }
    std::mt19937_64 random(0x40DF50);
    std::uniform_real_distribution<double> values(-1000.0, 1000.0);
    for (int trial = 0; trial != 10000; ++trial) {
        grid.height = static_cast<std::int32_t>(random() % 97);
        if (!compare(values(random))) return 1;
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native Y comparison passed " << cases << " cases\n";
}
