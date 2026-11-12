#include "lemmings/actor_terrain_probe.hpp"

#include <bit>

namespace lemmings {
namespace {
std::int32_t low_integer(long double value) {
    constexpr long double limit = 9223372036854775808.0L;
    if (!(value >= -limit && value < limit)) return 0;
    return std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
}
}

bool actor_sample_y_in_world(const ActorGeometryState& actor,
                             const CollisionGridHead& grid) {
    const auto point = actor_sample_position_360(actor);
    return point[1] >= 0 && static_cast<long double>(grid.height) >= point[1];
}

long double actor_distance_to_terrain(const ActorGeometryState& actor,
                                      CollisionGridHead& grid) {
    const double start_y = actor.position[1];
    auto row = low_integer(start_y);
    while (row >= 0 && row < grid.height) {
        // Preserve the original short-circuited three queries. The accessor
        // wraps X and returns a cell reference; it performs no terrain filter.
        if (*collision_cell(grid, static_cast<std::uint32_t>(low_integer(actor.position[0])), row) != 0 &&
            (*collision_cell(grid, static_cast<std::uint32_t>(low_integer(actor.position[0])), row) & 0x200) == 0 &&
            (*collision_cell(grid, static_cast<std::uint32_t>(low_integer(actor.position[0])), row) & 0x80) == 0)
            break;
        row = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(row) -
                                        static_cast<std::uint32_t>(actor.direction_44));
    }
    return static_cast<long double>(start_y) - row;
}

} // namespace lemmings
