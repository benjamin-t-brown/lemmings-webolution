#include "lemmings/actor_shadow.hpp"
#include "lemmings/actor_terrain_probe.hpp"
#include "lemmings/cylinder_coordinate.hpp"

#include <bit>
#include <cmath>

namespace lemmings {
namespace {
std::int32_t low_integer(long double x) {
    constexpr long double limit = 9223372036854775808.0L;
    if (!(x >= -limit && x < limit)) return 0;
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::int64_t>(x)));
}
}

void update_actor_shadow(ActorShadow& shadow, ActorShadowGlobals& globals,
    CollisionGridHead& grid, const WorldCoordinateMesh& mesh, ActorShadowCallbacks& cb) {
    void* list = cb.create_point_list_4229f0();
    cb.resize_point_list_421260(list, 1);
    shadow.position_3b0 = shadow.actor_3a8->geometry.position;
    if (shadow.actor_3a8->geometry.direction_44 == -1 || shadow.actor_3a8->gadget_54) {
        shadow.visible_3ad = 0;
        cb.destroy_point_list_422a30(list);
        return;
    }
    if (cb.has_state_42ecc9(*shadow.actor_3a8, 0x23)) {
        shadow.visible_3ad = 0;
        cb.destroy_point_list_422a30(list);
        return;
    }
    if (!actor_has_terrain_contact(shadow.actor_3a8->geometry, shadow.actor_3a8->primary_type_14, grid)) {
        const double distance = static_cast<double>(actor_distance_to_terrain(shadow.actor_3a8->geometry, grid));
        if (!(distance > 0 && distance < 10)) {
            shadow.visible_3ad = 0;
            cb.destroy_point_list_422a30(list);
            return;
        }
        shadow.visible_3ad = 1;
        shadow.position_3b0[1] = static_cast<double>(static_cast<long double>(shadow.position_3b0[1]) - distance);
        shadow.position_3b0[1] = static_cast<double>(static_cast<long double>(shadow.position_3b0[1]) + 1.0L);
        shadow.intensity_3c8 = low_integer(255.0L - (static_cast<long double>(distance) / 10.0L) * 255.0L);
    } else {
        globals.width_4c05c8 = 2.5;
        shadow.visible_3ad = 1;
        shadow.intensity_3c8 = 255;
    }
    const auto position = shadow.actor_3a8->geometry.position;
    const double left = static_cast<double>(wrap_cylinder_x(static_cast<double>(
        static_cast<long double>(position[0]) - static_cast<long double>(globals.width_4c05c8) / 2.0L)));
    const double right = static_cast<double>(wrap_cylinder_x(static_cast<double>(
        static_cast<long double>(globals.width_4c05c8) / 2.0L + position[0])));
    cb.build_segments_459557(shadow, list, left, right);
    const auto count = cb.point_count_459990(list);
    const auto& last = cb.point_at_421240(list, count - 1);
    const auto& first = cb.point_at_421240(list, 0);
    const double span = std::fabs(static_cast<double>(static_cast<long double>(last[0]) - first[0]));
    std::array<double, 4> u{};
    for (std::int32_t i = 0; i < count; ++i) {
        const auto& point = cb.point_at_421240(list, i);
        const auto& origin = cb.point_at_421240(list, 0);
        u[i] = static_cast<double>((static_cast<long double>(point[0]) - origin[0]) / span);
    }
    void* resource = cb.lookup_resource_414a10(globals.resource_id_4c05c0);
    cb.activate_resource_47261f(resource);

    // The original evaluates depth first, then reads the Y entry, then X.
    const auto vertex = [&](unsigned output, int x_index, int y_index, bool front) {
        const double z = static_cast<double>(static_cast<long double>(shadow.actor_3a8->geometry.position[2]) +
                                             (front ? 0.25L : -0.25L));
        constexpr double lift = std::bit_cast<double>(std::uint64_t{0x3fc851eb851eb852});
        const double y = static_cast<double>(static_cast<long double>(lift) + cb.point_at_421240(list, y_index)[1]);
        const double x = cb.point_at_421240(list, x_index)[0];
        shadow.vertices_000[output] = map_world_coordinates(mesh, {x, y, z});
    };
    const auto uv = [&](unsigned group, double first_u, double last_u) {
        shadow.uv_128[group][0] = {first_u, 0};
        shadow.uv_128[group][1] = {first_u, 1};
        shadow.uv_128[group][2] = {last_u, 1};
        shadow.uv_128[group][3] = {last_u, 0};
    };
    if (cb.point_count_459990(list) == 3) {
        shadow.mode_120 = 1;
        constexpr std::array<std::array<int, 2>, 4> indices{{{0,0},{1,0},{1,2},{2,2}}};
        for (unsigned i = 0; i < indices.size(); ++i) {
            vertex(i * 2, indices[i][0], indices[i][1], true);
            vertex(i * 2 + 1, indices[i][0], indices[i][1], false);
        }
        uv(0, u[0], u[1]);
    } else if (cb.point_count_459990(list) == 4) {
        shadow.mode_120 = 5;
        constexpr std::array<std::array<int, 2>, 6> indices{{{0,0},{1,0},{1,1},{2,2},{2,3},{3,3}}};
        for (unsigned i = 0; i < indices.size(); ++i) {
            vertex(i * 2, indices[i][0], indices[i][1], true);
            vertex(i * 2 + 1, indices[i][0], indices[i][1], false);
        }
        uv(0, u[0], u[1]);
        uv(1, static_cast<double>(static_cast<long double>(u[1]) - 1.0L),
              static_cast<double>(1.0L + u[1]));
        uv(2, u[1], u[2]);
        uv(3, static_cast<double>(static_cast<long double>(u[2]) - 1.0L),
              static_cast<double>(1.0L + u[2]));
        uv(4, u[2], u[3]);
    }
    cb.destroy_point_list_422a30(list);
}

} // namespace lemmings
