#include "app/ActorTerrain.h"

namespace webo {

namespace {

constexpr double kNativeSpawnZSlab = 0.5;

}  // namespace

ActorTerrainSample terrainSampleFromHatchSpawn(const HatchSpawnCells& hatch) {
  ActorTerrainSample out;
  out.x = static_cast<double>(hatch.x);
  out.y = static_cast<double>(hatch.y);
  out.z = kNativeSpawnZSlab;
  return out;
}

}  // namespace webo
