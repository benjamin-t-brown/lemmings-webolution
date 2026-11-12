// Offline comparison of 0x0040DE60 and its array-index helper 0x00404D10.
#include "lemmings/collision_grid.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <random>

namespace {
constexpr std::uintptr_t image_base = 0x00400000;
constexpr std::size_t image_size = 0x10000;
using NativeCell = std::int32_t* (__attribute__((thiscall)) *)(
    lemmings::CollisionGridHead*, std::uint32_t, std::int32_t);

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
    if (!read_at<25>(0x00404D10) || !read_at<92>(0x0040DE60)) {
        std::cerr << "expected verified function bytes on stdin\n";
        return 2;
    }
    DWORD old_protection = 0;
    if (!VirtualProtect(image, image_size, PAGE_EXECUTE_READ, &old_protection)) {
        std::cerr << "VirtualProtect failed\n";
        return 2;
    }
    FlushInstructionCache(GetCurrentProcess(), image, image_size);
    auto native = reinterpret_cast<NativeCell>(0x0040DE60);

    std::array<std::int32_t, 8 * 256> cells{};
    lemmings::CollisionGridHead grid{};
    grid.cells.data = cells.data();
    int cases = 0;
    auto compare = [&](std::uint32_t x, std::int32_t y) {
        auto* original = native(&grid, x, y);
        auto* recovered = lemmings::collision_cell(grid, x, y);
        ++cases;
        if (original == recovered) return true;
        std::cerr << "grid mismatch height=" << grid.height << " x=" << x
                  << " y=" << y << " original=" << original
                  << " recovered=" << recovered << '\n';
        return false;
    };

    for (int height = -2; height <= 5; ++height) {
        grid.height = height;
        for (int x = -512; x <= 512; x += 17) {
            for (int y = -10; y <= 10; ++y) {
                if (!compare(static_cast<std::uint32_t>(x), y)) return 1;
            }
        }
    }
    std::mt19937 random(0x40DE60);
    for (int i = 0; i != 10000; ++i) {
        grid.height = 1 + static_cast<std::int32_t>(random() % 6);
        const auto x = random();
        const auto y = static_cast<std::int32_t>(random() % 201) - 100;
        if (!compare(x, y)) return 1;
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native grid comparison passed " << cases << " cases\n";
}
