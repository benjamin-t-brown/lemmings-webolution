#include "lemmings/actor_geometry.hpp"
#include "lemmings/cylinder_coordinate.hpp"
#include <bit>

namespace lemmings {
namespace {
std::int32_t negate_wrapping(std::int32_t value) {
    return std::bit_cast<std::int32_t>(0u - static_cast<std::uint32_t>(value));
}
std::int32_t x87_low_integer(long double value) {
    constexpr long double limit = 9223372036854775808.0L;
    if (!(value >= -limit && value < limit)) return 0;
    return std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
}
}
void reverse_actor_direction_40(ActorGeometryState& actor) {
    actor.direction_40 = negate_wrapping(actor.direction_40);
    const double x = actor.direction_40 == -1 ? -1.0 : 0.0;
    actor.pair_350[0] = x;
    actor.pair_360[0] = x;
}
void reverse_actor_direction_44(ActorGeometryState& actor) {
    actor.direction_44 = negate_wrapping(actor.direction_44);
    actor.flag_5c = 1;
    actor.pair_350[1] = actor.direction_44 == 1 ? 3.0 : 0.0;
    actor.pair_360[1] = actor.direction_44 == 1 ? 0.0 : 3.0;
}
std::array<double,3> actor_sample_position_350(const ActorGeometryState& actor) {
    return {
        static_cast<double>(static_cast<long double>(actor.pair_350[0]) + actor.position[0]),
        static_cast<double>(static_cast<long double>(actor.pair_350[1]) + actor.position[1]),
        actor.position[2]
    };
}
std::array<double,3> actor_sample_position_360(const ActorGeometryState& actor) {
    return {
        static_cast<double>(static_cast<long double>(actor.position[0]) + actor.pair_360[0]),
        static_cast<double>(static_cast<long double>(actor.position[1]) + actor.pair_360[1]),
        actor.position[2]
    };
}
bool actor_accepts_terrain_value(std::int32_t category, std::uint32_t cell) {
    if (cell == 0x200) return category == 1;
    if (cell == 0x4000) return category == 2;
    if (cell == 0x2000) return false;
    return true;
}
bool actor_has_terrain_contact(const ActorGeometryState& actor,
                               std::int32_t primary_state_id,
                               CollisionGridHead& grid) {
    auto point = actor_sample_position_360(actor);
    if (actor.direction_40 == -1 && primary_state_id != 4 && primary_state_id != 5) {
        point[0] = static_cast<double>(static_cast<long double>(point[0]) + 1.0L);
    }
    point[1] = static_cast<double>(static_cast<long double>(point[1]) - actor.direction_44);
    const auto y = x87_low_integer(clamp_cylinder_y(grid,point[1]));
    const auto x = x87_low_integer(wrap_cylinder_x(point[0]));
    const auto cell = static_cast<std::uint32_t>(
        *collision_cell(grid,static_cast<std::uint32_t>(x),y));
    return cell != 0 && (cell & 0x80u) == 0 && (cell & 0x1000u) == 0 &&
           actor_accepts_terrain_value(actor.category_4c,cell);
}
bool actor_has_wall_contact(const ActorGeometryState& actor, CollisionGridHead& grid) {
    const auto point = actor_sample_position_360(actor);
    bool contact = true;
    for (std::uint32_t i = 0; i < 4; ++i) {
        const auto offset = std::bit_cast<std::int32_t>(
            static_cast<std::uint32_t>(actor.direction_44) * i);
        const double sample_y = static_cast<double>(static_cast<long double>(offset) + point[1]);
        const auto y = x87_low_integer(clamp_cylinder_y(grid, sample_y));
        const double sample_x = static_cast<double>(static_cast<long double>(actor.direction_40) + point[0]);
        const auto x = x87_low_integer(wrap_cylinder_x(sample_x));
        const auto cell = static_cast<std::uint32_t>(*collision_cell(grid, static_cast<std::uint32_t>(x), y));
        if ((cell & 0x80u) == 0) contact = contact & (cell != 0);
    }
    return contact;
}
bool actor_contains_xy_point(const ActorGeometryState& actor,
                             const std::array<double,3>& point) {
    const long double x = actor.position[0], y = actor.position[1];
    if (x - 1.0L > point[0] || y - 1.0L > point[1]) return false;
    return x + 1.0L >= point[0] && y + 1.0L >= point[1];
}
} // namespace lemmings
