#include "lemmings/actor_terrain_probe.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <limits>

using namespace lemmings;

int main() {
    std::array<std::int32_t, 8 * 256> cells{};
    CollisionGridHead grid{};
    grid.height = 8;
    grid.cells.data = cells.data();
    ActorGeometryState actor;
    actor.position = {257.75, 7.5, 0};
    actor.pair_360 = {9, 0.5};
    assert(actor_sample_y_in_world(actor, grid)); // Inclusive height, no truncation.
    actor.pair_360[1] = 0.75;
    assert(!actor_sample_y_in_world(actor, grid));
    actor.position[1] = -0.5;
    actor.pair_360[1] = 0.5;
    assert(actor_sample_y_in_world(actor, grid));
    actor.position[1] = std::numeric_limits<double>::quiet_NaN();
    assert(!actor_sample_y_in_world(actor, grid));

    actor.position[1] = 6.5;
    actor.pair_360 = {100, 100}; // Deliberately ignored by the distance scan.
    cells[6 * 256 + 1] = 0x280;
    cells[5 * 256 + 1] = 0x201;
    cells[4 * 256 + 1] = 0x81;
    cells[3 * 256 + 1] = 0x1000; // Trampoline bit does not make this passable.
    assert(actor_distance_to_terrain(actor, grid) == 3.5L);
    cells.fill(0);
    assert(actor_distance_to_terrain(actor, grid) == 7.5L); // Row -1, not row zero.
    actor.direction_44 = -1;
    assert(actor_distance_to_terrain(actor, grid) == -1.5L); // Row height.
    actor.direction_44 = 2;
    cells[3 * 256 + 1] = 1; // Odd row is skipped.
    assert(actor_distance_to_terrain(actor, grid) == 8.5L);
    actor.direction_44 = 0;
    cells[6 * 256 + 1] = 1; // Zero direction is valid if the first row stops.
    assert(actor_distance_to_terrain(actor, grid) == 0.5L);
    actor.position[1] = -1.5;
    assert(actor_distance_to_terrain(actor, grid) == -0.5L);
    actor.position[1] = 4294967296.5;
    actor.direction_44 = 1;
    cells[1] = 1;
    assert(actor_distance_to_terrain(actor, grid) == 4294967296.5L);
}
