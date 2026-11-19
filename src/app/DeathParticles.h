#pragma once

#include "app/ActorPose.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <optional>

namespace webo::app {

// Presentation only. No particle feeds a position, random value or collision
// back into World. See docs/scratch/death-particles-001.md for evidence/limits.
struct DeathParticle {
  ActorTerrainSample origin{};
  double vx = 0, vy = 0, vz = 0, gravity = 0;
  std::uint64_t bornMs = 0;
  double lifetime = 0, size = 0;
  std::array<std::uint8_t, 3> colour{255, 255, 255};
  int piece = -1; // BITS.SPR clip index; -1 is a coloured chip/droplet
  int verticalDir = 1;
  bool minerAxe = false; // installed Axe child, separate from BITS/debris
  int facing = 1;       // snapshotted before refusal turns the miner
  // Terrain fragments retain the material/UVs of the cell that was removed.
  // Solid-colour flecks have no material.
  std::optional<std::uint32_t> terrainMaterial;
  std::array<double, 4> terrainUv{}; // left, top, right, bottom
  std::array<double, 2> rotation{}, angularVelocity{};

  void setChunkSpin(unsigned seed, int index) {
    constexpr double turn = 2.0 * std::numbers::pi;
    rotation = {(seed & 255u) * (turn / 256.0), ((seed >> 3) & 255u) * (turn / 256.0)};
    angularVelocity = {(index & 1 ? -1 : 1) * (2.5 + (seed % 7u) * 0.4),
                       (index & 2 ? -1 : 1) * (3.0 + (seed % 5u) * 0.5)};
  }

  // Provisional rigid chunk in a local orthonormal tangent/up/radial frame.
  // Two age-driven angles follow the native 495E2C/495EBA presentation path;
  // exact native shapes, random rates and shrink envelope are not reproduced.
  std::array<std::array<double, 3>, 8> chunkOffsets(std::uint64_t clockMs) const {
    const double t = (clockMs - bornMs) / 1000.0;
    const double a = rotation[0] + angularVelocity[0] * t;
    const double b = rotation[1] + angularVelocity[1] * t;
    const double ca = std::cos(a), sa = std::sin(a), cb = std::cos(b), sb = std::sin(b);
    std::array<std::array<double, 3>, 8> out;
    for (int i = 0; i < 8; ++i) {
      const double x = ((i & 1) ? 1 : -1) * size;
      const double y = ((i & 2) ? 1 : -1) * size * 0.65;
      const double z = ((i & 4) ? 1 : -1) * size * 0.55;
      const double ry = ca * y - sa * z, rz = sa * y + ca * z;
      out[i] = {cb * x + sb * rz, ry, -sb * x + cb * rz};
    }
    return out;
  }

  ActorTerrainSample position(std::uint64_t clockMs) const {
    const double t = (clockMs - bornMs) / 1000.0;
    return {origin.x + vx * t, origin.y + vy * t + gravity * t * t * 0.5,
            origin.z + vz * t};
  }
};

} // namespace webo::app
