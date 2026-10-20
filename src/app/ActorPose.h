#pragma once

#include "assets/Cylinder.h"
#include "sim/World.h"

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace webo {

// Level data needed to reproduce native `0x0043D56C` cell-mesh interpolation
// (same key table and surface pool the loader builds at level load).
struct ActorPoseLevel {
  int gridHeight = 0;
  std::span<const std::uint32_t> grid0{};
  std::span<const Vec3> surface{};
  // Constant retail cylinder template (`world+0x18` bands); used when a
  // `grid0` key is `-1` and native `0x0043A31D` would call `0x0043D2B8`.
  const CylinderMesh* cylinderTemplate = nullptr;
};

struct ActorTerrainSample {
  double x = 0;
  double y = 0;
  double z = 0;
};

struct ActorPoseResult {
  std::array<double, 3> modelPosition{};
  std::array<double, 3> basis{};
};

// Native actor `+0x08` triple (file Y-up X/Y, Z blends inner/outer shell).
// **Unsettled:** no live sim/native pair was measured for the Y flip or Z slab;
// `terrainHeight - 1 - lemming.y` follows other webolution terrain adapters.
ActorTerrainSample terrainSampleFromLemming(const sim::Lemming& lemming, int terrainHeight,
                                            double zSlab = 0.75);

// `0x0043D56C` on the stored surface pool: X buckets every 8 world units,
// Y buckets every 16, Z at `+0x18` selects between layer 0 and 1 vertices.
std::optional<std::array<double, 3>> interpolateCellMeshPosition(
    const ActorPoseLevel& level, const ActorTerrainSample& sample);

// Full adapter: sim lemming -> model `+0x108` and stored basis (`0x0042EEF2`
// path: facing `+0x40` adds `0.1` to X before interpolation for basis only).
ActorPoseResult actorPoseFromLemming(const ActorPoseLevel& level, const sim::Lemming& lemming,
                                     double zSlab = 0.75);

ActorPoseResult actorPoseFromNativeSample(const ActorPoseLevel& level,
                                          const ActorTerrainSample& plus8, int facing);

}  // namespace webo
