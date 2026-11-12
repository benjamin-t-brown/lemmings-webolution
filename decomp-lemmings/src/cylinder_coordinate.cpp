#include "lemmings/cylinder_coordinate.hpp"
#include "lemmings/collision_grid.hpp"

#include <bit>
#include <cmath>
#include <cstdint>

namespace lemmings {

long double wrap_cylinder_x(double x) {
    constexpr double epsilon = std::bit_cast<double>(
        std::uint64_t{0x3e45798ee0000000});
    if (!(std::fabs(x) >= epsilon)) {
        return 0.0L;
    }
    if (x < 0.0) {
        return static_cast<long double>(x) + 256.0L;
    }
    while (x >= 256.0) {
        x -= 256.0; // The original stores the result to a double each pass.
    }
    return static_cast<long double>(x);
}

long double clamp_cylinder_y(const CollisionGridHead& grid, double y) {
    if (!(y >= 0.0)) {
        return 0.0L;
    }
    if (y >= static_cast<double>(grid.height)) {
        return static_cast<long double>(grid.height - 1);
    }
    return static_cast<long double>(y);
}

} // namespace lemmings
