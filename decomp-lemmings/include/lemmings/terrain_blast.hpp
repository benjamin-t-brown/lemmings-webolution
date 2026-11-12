#pragma once

#include "lemmings/terrain_mask.hpp"
#include <array>

namespace lemmings {

struct TerrainBlastCallbacks {
    virtual ~TerrainBlastCallbacks() = default;
    // Combined allocation/constructor boundaries. Effect factories must return
    // valid handles; the original dereferences them without a null guard.
    virtual void* create_debris_40c960(const std::array<double, 3>&) = 0; // 0x748 bytes
    virtual void set_time_40c920(void*, double) = 0;
    virtual void reset_3c(void*, std::int32_t) = 0;
    virtual void set_debris_count_40dbf0(void*, std::int32_t) = 0;
    virtual std::array<double, 3> world_position_43d56c(const std::array<double, 3>&) = 0;
    virtual void set_direction_54(void*, const std::array<double, 3>&) = 0;
    // Entity allocation (0x48 bytes) may fail; insertion still receives null.
    virtual void* create_entity_432080(const std::array<double, 3>&, void*, std::uint8_t) = 0;
    virtual void insert_entity_401890(void*&) = 0;
    virtual void notify_rectangle_43b0a1(std::uint32_t x, std::int32_t y,
                                        std::uint32_t end_x, std::int32_t end_y) = 0;
    virtual void* create_particles_402870() = 0; // 0x158 bytes
    virtual void set_color_43fa30(void*, const std::array<double, 3>&) = 0;
    virtual void set_particle_count_402ce0(void*, std::int32_t) = 0;
    virtual void set_position_08(void*, const std::array<double, 3>&) = 0;
    virtual void set_pair_402d80(void*, double, double) = 0;
    virtual void set_flag_403e10(void*, std::int32_t) = 0;
    virtual void set_mode_402dc0(void*, std::int32_t) = 0;
    virtual void advance_2c(void*, double) = 0;
};

// Full normal-return 0x43F510: grid mask, object sites and effect orchestration.
// Caller position is live and may change through callbacks. Requires positive
// stable height, stable valid cell storage and finite XY whose wrapped/clamped values
// fit signed32 conversion. Renderer/resource/allocation effects remain explicit.
// Callback exceptions and original compiler SEH behavior are not modeled.
void apply_terrain_blast(CollisionGridHead&, std::array<double, 3>& position,
    std::int32_t mode, double elapsed, TerrainBlastCallbacks&);

} // namespace lemmings
