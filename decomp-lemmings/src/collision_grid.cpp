#include "lemmings/collision_grid.hpp"

namespace lemmings {

std::int32_t* collision_cell(CollisionGridHead& grid,
                             std::uint32_t x, std::int32_t y) {
    if (y >= grid.height) {
        y = grid.height - 1;
    }
    if (y < 0) {
        y = 0;
    }
    const std::uint32_t index = (x & 0xffu) +
                                (static_cast<std::uint32_t>(y) << 8);
    return grid.cells.data + index;
}

} // namespace lemmings
