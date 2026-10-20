#pragma once

#include "app/ActorPose.h"

#include <cstdint>

namespace webo {

// Hatch spawn words at native `+0x9A` / `+0x9C` (signed int16 from level stream).
struct HatchSpawnCells {
  std::int16_t x = 0;
  std::int16_t y = 0;
};

// First actor `+0x08` / `+0x10` / `+0x18` sample on hatch release, before steppers.
// READ: `0x00425C04`/`0x00425C1A` sign-extend hatch words to doubles;
// `0x0042DE6F` pushes `0x3FE00000` (0.5); `0x004230F9` `rep movsd` copies 24 B to `actor+8`.
ActorTerrainSample terrainSampleFromHatchSpawn(const HatchSpawnCells& hatch);

}  // namespace webo
