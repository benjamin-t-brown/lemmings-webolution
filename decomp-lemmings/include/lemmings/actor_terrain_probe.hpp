#pragma once

#include "lemmings/actor_geometry.hpp"

namespace lemmings {

// 0x0042F6EC. Tests the pair_360 sample Y against [0, grid.height], inclusive.
// NaN is outside. Only the original return register's low byte is meaningful.
bool actor_sample_y_in_world(const ActorGeometryState& actor,
                             const CollisionGridHead& grid);

// 0x0042F788. Starts at trunc(raw position Y), then subtracts direction_44
// until the row leaves [0,height) or a nonzero cell without bits 0x200/0x80 is
// reached. X is truncated and wrapped to eight bits by collision_cell; neither
// pair_350 nor pair_360 is used. Returns raw Y minus the final signed row index
// in x87 extended precision, so upward scans can return a negative distance.
// Integer conversion uses the original FISTP signed64 low-32-bit projection;
// row subtraction wraps at 32 bits. Requires masked FP exceptions, valid cell
// storage for accessed rows, and a terminating scan. In particular direction
// zero with a passable starting cell does not terminate; no guard is invented.
long double actor_distance_to_terrain(const ActorGeometryState& actor,
                                      CollisionGridHead& grid);

} // namespace lemmings
