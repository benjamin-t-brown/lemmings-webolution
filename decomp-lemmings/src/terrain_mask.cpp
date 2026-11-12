#include "lemmings/terrain_mask.hpp"
#include "lemmings/cylinder_coordinate.hpp"

namespace lemmings {
namespace {
// Literal byte masks at 0x004B1528 and 0x004B155C in the reference EXE.
constexpr char small_mask[7][8] = {
    "0011100", "0111110", "1111111", "1111111",
    "1111111", "0111110", "0011100"
};
constexpr char large_mask[9][10] = {
    "000111000", "011111110", "011111110", "111111111",
    "111111111", "111111111", "011111110", "011111110",
    "000111000"
};
}

void clear_terrain_blast_cells(CollisionGridHead& grid,
                               const double (&position)[3], bool large,
                               TerrainObjectSite on_object_site,
                               void* site_context) {
    if (grid.height <= 0 || grid.cells.data == nullptr) {
        return;
    }
    const int width = large ? 9 : 7;
    const double half_width = static_cast<double>(width) / 2.0;
    const double y_input = position[1] - half_width;
    const double x_input = position[0] - half_width;
    std::int32_t y = static_cast<std::int32_t>(
        clamp_cylinder_y(grid, y_input));
    const auto first_x = static_cast<std::uint32_t>(
        static_cast<std::int32_t>(wrap_cylinder_x(x_input)));
    for (int row = 0; row < width; ++row) {
        std::uint32_t x = first_x;
        for (int column = 0; column < width; ++column) {
            const bool masked = large
                ? large_mask[row][column] != '0'
                : small_mask[row][column] != '0';
            if (masked) {
                auto* cell = collision_cell(grid, x, y);
                const auto flags = static_cast<std::uint32_t>(*cell);
                if ((flags & 0xA02u) == 0) {
                    if ((flags & 1u) != 0 && (x & 3u) == 0 && (y & 3) == 0 &&
                        on_object_site != nullptr) {
                        on_object_site(x, y, site_context);
                    }
                    *cell = 0;
                }
            }
            x = (x + 1) & 0xffu;
        }
        ++y;
        if (y == grid.height) {
            break;
        }
    }
}

} // namespace lemmings
