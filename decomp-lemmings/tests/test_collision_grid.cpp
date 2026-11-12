#include "lemmings/collision_grid.hpp"

#include <array>
#include <stdexcept>

namespace {
void check(bool condition) {
    if (!condition) {
        throw std::runtime_error("collision grid index mismatch");
    }
}
}

int main() {
    std::array<std::int32_t, 4 * 256> cells{};
    lemmings::CollisionGridHead grid{};
    grid.height = 4;
    grid.cells.data = cells.data();

    check(lemmings::collision_cell(grid, 0, 0) == &cells[0]);
    check(lemmings::collision_cell(grid, 255, 3) == &cells[1023]);
    check(lemmings::collision_cell(grid, 256, 1) == &cells[256]);
    check(lemmings::collision_cell(grid, 0xffffffffu, 2) == &cells[767]);
    check(lemmings::collision_cell(grid, 7, -100) == &cells[7]);
    check(lemmings::collision_cell(grid, 7, 100) == &cells[3 * 256 + 7]);

    grid.height = 0;
    check(lemmings::collision_cell(grid, 19, 100) == &cells[19]);
}
