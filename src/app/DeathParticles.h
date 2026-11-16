#pragma once

#include "app/ActorPose.h"

#include <array>
#include <cstdint>

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

  ActorTerrainSample position(std::uint64_t clockMs) const {
    const double t = (clockMs - bornMs) / 1000.0;
    return {origin.x + vx * t, origin.y + vy * t + gravity * t * t * 0.5,
            origin.z + vz * t};
  }
};

} // namespace webo::app
