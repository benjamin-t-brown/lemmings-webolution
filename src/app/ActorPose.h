#pragma once

#include "assets/Cylinder.h"
#include "sim/World.h"

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace webo {

// Level geometry for native cell-mesh interpolation at `0x0043D56C`.
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

// Native actor `+0x08` triple. Y is up and Z blends the two shells.
// The clone's Y conversion and Z slab are provisional.
ActorTerrainSample terrainSampleFromLemming(const sim::Lemming& lemming, int terrainHeight,
                                            double zSlab = 0.75);

// Native `0x0043D2B8` supplies missing mesh vertices for `0x0043A31D` at `0x43A3AE`.
// Bilinear interpolation uses 8-column by 16-row buckets on shell 0 (inner) or
// 1 (outer). Columns wrap with `& 0x1F` and rows clamp to `gridHeight / 16 - 1`.
std::optional<Vec3> cylinderTemplateVertex(const CylinderMesh& cylinder, int x, int y, int layer,
                                           int gridHeight);

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

// `0x43D56C` anchors live actors on the cylinder template (world+0x18).
// The deformed pool (+0x148) can flatten their feet at water boundaries.
// gridHeight is the serialized coordinate height (112).
ActorPoseResult actorPoseFromCylinderSample(const ActorPoseLevel& level,
                                            const ActorTerrainSample& plus8, int facing);

}  // namespace webo
