#include "lemmings/cylinder_coordinate.hpp"
#include "lemmings/collision_grid.hpp"

#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace {
void check(bool condition) {
    if (!condition) {
        throw std::runtime_error("cylinder coordinate mismatch");
    }
}
}

int main() {
    constexpr double threshold = std::bit_cast<double>(
        std::uint64_t{0x3e45798ee0000000});
    check(lemmings::wrap_cylinder_x(0.0) == 0.0L);
    check(lemmings::wrap_cylinder_x(-0.0) == 0.0L);
    check(lemmings::wrap_cylinder_x(std::nextafter(threshold, 0.0)) == 0.0L);
    check(lemmings::wrap_cylinder_x(threshold) == threshold);
    check(lemmings::wrap_cylinder_x(-threshold) ==
          256.0L - static_cast<long double>(threshold));
    check(lemmings::wrap_cylinder_x(255.75) == 255.75L);
    check(lemmings::wrap_cylinder_x(256.0) == 0.0L);
    check(lemmings::wrap_cylinder_x(512.5) == 0.5L);
    check(lemmings::wrap_cylinder_x(-1.0) == 255.0L);
    check(lemmings::wrap_cylinder_x(-256.0) == 0.0L);
    check(lemmings::wrap_cylinder_x(-257.0) == -1.0L);
    check(lemmings::wrap_cylinder_x(std::numeric_limits<double>::quiet_NaN()) == 0.0L);

    lemmings::CollisionGridHead grid{};
    grid.height = 96;
    check(lemmings::clamp_cylinder_y(grid, -1.0) == 0.0L);
    check(lemmings::clamp_cylinder_y(grid, 0.0) == 0.0L);
    check(lemmings::clamp_cylinder_y(grid, 95.5) == 95.5L);
    check(lemmings::clamp_cylinder_y(grid, 96.0) == 95.0L);
    check(lemmings::clamp_cylinder_y(grid, 1000.0) == 95.0L);
    check(lemmings::clamp_cylinder_y(grid,
          std::numeric_limits<double>::quiet_NaN()) == 0.0L);
    grid.height = 0;
    check(lemmings::clamp_cylinder_y(grid, 0.0) == -1.0L);
}
