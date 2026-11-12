#pragma once

#include "lemmings/collision_grid.hpp"
#include <array>
#include <cstdint>

namespace lemmings {
struct ActorGeometryState {
    std::array<double,3> position{}; // +8
    std::int32_t direction_40 = 1;
    std::int32_t direction_44 = 1;
    std::int32_t category_4c = 0;
    std::uint8_t flag_5c = 0;
    std::array<double,2> pair_350{};
    std::array<double,2> pair_360{};
};

// Names retain offsets until both directions' complete semantics are recovered.
void reverse_actor_direction_40(ActorGeometryState& actor); // 0x0042F66B
void reverse_actor_direction_44(ActorGeometryState& actor); // 0x0042F5E3
std::array<double,3> actor_sample_position_360(const ActorGeometryState& actor);
std::array<double,3> actor_sample_position_350(const ActorGeometryState& actor); // 0x0042F35F

// 0x0042E8C7 compares complete cell values, not individual mask bits.
bool actor_accepts_terrain_value(std::int32_t category, std::uint32_t cell);

// 0x0042E913: valid positive grid dimensions and backing storage required.
// primary_state_id is read from the actor's primary state at +0x340/+0x14.
// X must be in the terminating domain of wrap_cylinder_x.
bool actor_has_terrain_contact(const ActorGeometryState& actor,
                               std::int32_t primary_state_id,
                               CollisionGridHead& grid);

// 0x0042EB48: four cells ahead, spaced by direction_44 from the foot sample.
// Bit 0x80 leaves the accumulated result unchanged; any empty cell clears it.
// Same valid-grid/terminating-X requirements as the contact query above.
bool actor_has_wall_contact(const ActorGeometryState& actor, CollisionGridHead& grid);

// 0x0042FAD7: inclusive one-unit XY bounds; the third coordinate is ignored.
bool actor_contains_xy_point(const ActorGeometryState& actor,
                             const std::array<double,3>& point);
} // namespace lemmings
