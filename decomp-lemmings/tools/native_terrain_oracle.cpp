// Compare the original terrain mask loop with the recovered grid update.
// The loop-exit instruction is redirected to the routine's own epilogue so
// later visual and object work is deliberately outside this oracle.
#include "lemmings/terrain_mask.hpp"

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
constexpr std::size_t image_size = 0xD0000;
constexpr int max_height = 96;
constexpr int cell_count = max_height * 256;
using NativeBlast = void (__cdecl *)(const double*, int, int, int);

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
    if (!read_at<25>(0x00404D10) || !read_at<20>(0x0040BAA0) ||
        !read_at<92>(0x0040DE60) || !read_at<110>(0x0040DEC0) ||
        !read_at<21>(0x0040DF30) || !read_at<85>(0x0040DF50) ||
        !read_at<1308>(0x0043F510) || !read_at<39>(0x0049C5AC) ||
        !read_at<181>(0x0049C8F4) || !read_at<35>(0x0049FC56) ||
        !read_at<8>(0x004B0338) || !read_at<8>(0x004B03F8) ||
        !read_at<8>(0x004B0698) || !read_at<8>(0x004B07A8) ||
        !read_at<49>(0x004B1528) || !read_at<81>(0x004B155C)) {
        std::cerr << "expected verified code and data bytes on stdin\n";
        return 2;
    }
    // At loop exit, skip unrelated redraw/object work using the original
    // routine's own stack and SEH unwind sequence.
    auto* exit = reinterpret_cast<std::uint8_t*>(0x0043F830);
    exit[0] = 0xE9;
    const std::int32_t displacement = 0x0043FA1C - (0x0043F830 + 5);
    std::memcpy(exit + 1, &displacement, sizeof(displacement));
    DWORD old_protection = 0;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &old_protection)) {
        std::cerr << "VirtualProtect failed\n";
        return 2;
    }
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);

    auto native = reinterpret_cast<NativeBlast>(0x0043F510);
    std::array<std::int32_t, cell_count> original_cells{};
    std::array<std::int32_t, cell_count> recovered_cells{};
    lemmings::CollisionGridHead original_grid{};
    original_grid.height = max_height;
    original_grid.cells.data = original_cells.data();
    lemmings::CollisionGridHead recovered_grid{};
    recovered_grid.height = max_height;
    recovered_grid.cells.data = recovered_cells.data();
    std::memcpy(reinterpret_cast<void*>(0x004C6A10),
                &original_grid, sizeof(original_grid));

    std::mt19937 random(0x43F510);
    constexpr std::int32_t flags[] = {0, 1, 2, 5, 0x200, 0x800, 4, 8, 0x100, 0x400};
    for (int trial = 0; trial != 3000; ++trial) {
        const int height = 1 + static_cast<int>(random() % max_height);
        recovered_grid.height = height;
        *reinterpret_cast<std::int32_t*>(0x004C6A10 + 0x418) = height;
        for (int cell = 0; cell != cell_count; ++cell) {
            auto value = flags[random() % std::size(flags)];
            // An aligned occupied site enters object construction, which is
            // outside the grid-only slice. Keep bit 1 on other sites.
            if ((cell % 256 & 3) == 0 && (cell / 256 & 3) == 0) value &= ~1;
            original_cells[cell] = value;
        }
        recovered_cells = original_cells;
        const double position[3] = {
            static_cast<double>(static_cast<int>(random() % 1200) - 600) +
                static_cast<double>(random() % 4) / 4.0,
            static_cast<double>(static_cast<int>(random() % 120) - 10) +
                static_cast<double>(random() % 4) / 4.0,
            0.0
        };
        const int mode = trial & 1;
        native(position, mode, 0, 0);
        lemmings::clear_terrain_blast_cells(recovered_grid, position, mode != 0);
        if (original_cells != recovered_cells) {
            for (int cell = 0; cell != cell_count; ++cell) {
                if (original_cells[cell] != recovered_cells[cell]) {
                    std::cerr << "terrain grid mismatch at trial " << trial
                              << " cell " << cell << " x=" << position[0]
                              << " y=" << position[1] << " mode=" << mode
                              << " original=" << original_cells[cell]
                              << " recovered=" << recovered_cells[cell] << '\n';
                    break;
                }
            }
            return 1;
        }
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native terrain loop passed 3000 grid comparisons\n";
}
