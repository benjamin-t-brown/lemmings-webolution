#pragma once

#include "lemmings/collision_grid.hpp"

#include <cstdint>

namespace lemmings {

using TerrainObjectSite = void (*)(std::uint32_t x, std::int32_t y,
                                   void* context);

// Grid-clearing loop recovered from 0x0043F510. `large` selects its 9x9
// mask; false selects 7x7. The larger routine also creates objects and
// performs redraw/visual work, which this function does not model.
void clear_terrain_blast_cells(CollisionGridHead& grid,
                               const double (&position)[3], bool large,
                               TerrainObjectSite on_object_site = nullptr,
                               void* site_context = nullptr);

} // namespace lemmings
