#include "lemmings/terrain_blast.hpp"
#include "lemmings/cylinder_coordinate.hpp"
#include <bit>

namespace lemmings {
namespace {
double rounded(long double value) {
    volatile double result = static_cast<double>(value);
    return result;
}
void set_direction(TerrainBlastCallbacks& callbacks, void* effect,
                   const std::array<double, 3>& position,
                   const std::array<double, 3>& offset) {
    // Mapping position first matters when a callback changes the live position.
    const auto first = callbacks.world_position_43d56c(position);
    const auto second = callbacks.world_position_43d56c(offset);
    std::array<double, 3> direction;
    for (unsigned i = 0; i < 3; ++i)
        direction[i] = rounded(static_cast<long double>(second[i]) - first[i]);
    callbacks.set_direction_54(effect, direction);
}
struct DebrisContext {
    std::array<double, 3>& position;
    TerrainBlastCallbacks& callbacks;
};
void debris_site(std::uint32_t, std::int32_t, void* context) {
    auto& self = *static_cast<DebrisContext*>(context);
    auto& position = self.position;
    auto& callbacks = self.callbacks;
    auto* effect = callbacks.create_debris_40c960(position);
    callbacks.set_time_40c920(effect, 0.01);
    callbacks.reset_3c(effect, 0);
    callbacks.set_debris_count_40dbf0(effect, 4);
    auto offset = position;
    offset[2] = rounded(static_cast<long double>(offset[2]) + 2.0L);
    offset[1] = rounded(static_cast<long double>(offset[1]) + 10.0L);
    set_direction(callbacks, effect, position, offset);
    auto* entity = callbacks.create_entity_432080(position, effect, 1);
    callbacks.insert_entity_401890(entity);
    // The grid loop clears its previously selected cell after this returns.
}
}

void apply_terrain_blast(CollisionGridHead& grid, std::array<double, 3>& position,
    std::int32_t mode, double elapsed, TerrainBlastCallbacks& callbacks) {
    const std::int32_t width = mode ? 9 : 7;
    const auto first_y = static_cast<std::int32_t>(clamp_cylinder_y(grid,
        rounded(static_cast<long double>(position[1]) - width / 2.0L)));
    const auto first_x = static_cast<std::uint32_t>(static_cast<std::int32_t>(
        wrap_cylinder_x(rounded(static_cast<long double>(position[0]) - width / 2.0L))));
    // The grid rectangle stays fixed while the effect callbacks use live input.
    const double input[3] = {position[0], position[1], position[2]};
    DebrisContext context{position, callbacks};
    clear_terrain_blast_cells(grid, input, mode != 0, debris_site, &context);
    const auto end_y = std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(first_y) + static_cast<std::uint32_t>(width));
    callbacks.notify_rectangle_43b0a1(first_x, first_y,
        first_x + static_cast<std::uint32_t>(width), end_y);

    // Capture the offset before creating the final particle object. Later
    // callbacks may change the live position used for its position/direction.
    auto offset = position;
    offset[1] = rounded(static_cast<long double>(offset[1]) + 40.0L);
    offset[2] = rounded(static_cast<long double>(offset[2]) + 10.0L);
    auto* effect = callbacks.create_particles_402870();
    callbacks.set_color_43fa30(effect, {255, 32, 32});
    callbacks.set_particle_count_402ce0(effect, 100);
    callbacks.set_position_08(effect, position);
    callbacks.set_pair_402d80(effect, 2, 0.5);
    set_direction(callbacks, effect, position, offset);
    callbacks.set_flag_403e10(effect, 1);
    callbacks.set_mode_402dc0(effect, 7);
    callbacks.advance_2c(effect, elapsed);
    auto* entity = callbacks.create_entity_432080(position, effect, 1);
    callbacks.insert_entity_401890(entity);
}

} // namespace lemmings
