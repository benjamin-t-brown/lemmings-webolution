#include "lemmings/terrain_mask.hpp"

#include <array>
#include <stdexcept>

namespace {
void check(bool value) {
    if (!value) throw std::runtime_error("terrain mask mismatch");
}
void count_site(std::uint32_t, std::int32_t, void* context) {
    ++*static_cast<int*>(context);
}
}

int main() {
    std::array<std::int32_t, 20 * 256> cells{};
    cells.fill(1);
    lemmings::CollisionGridHead grid{};
    grid.height = 20;
    grid.cells.data = cells.data();
    cells[8 * 256 + 8] = 0x201; // protected despite the occupied bit
    const double center[3] = {10.0, 10.0, 0.5};
    int sites = 0;
    lemmings::clear_terrain_blast_cells(grid, center, false,
                                        count_site, &sites);
    check(sites == 2);
    check(cells[8 * 256 + 8] == 0x201);
    check(cells[8 * 256 + 9] == 0);
    check(cells[6 * 256 + 6] == 1); // outside the first mask row

    cells.fill(1);
    sites = 0;
    lemmings::clear_terrain_blast_cells(grid, center, true,
                                        count_site, &sites);
    int cleared = 0;
    for (const auto cell : cells) cleared += cell == 0;
    check(cleared == 61);

    cells.fill(1);
    const double wrapped[3] = {255.0, 2.0, 0.5};
    lemmings::clear_terrain_blast_cells(grid, wrapped, false);
    check(cells[0 * 256 + 254] == 0);
    check(cells[2 * 256 + 0] == 0);
}
