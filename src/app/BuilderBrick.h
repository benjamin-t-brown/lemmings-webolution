#pragma once

#include "sim/World.h"

#include <algorithm>
#include <cmath>

namespace webo::app {

// Native 410302 shows the mesh at 0.12..0.78 s of a 16-entry strip
// at 0.06 s per entry. 4105B0 expands it toward the placed shape.
// Provisional clone sampling spans actual placement ticks so the carried
// brick retires when stepBuilder changes terrain.
inline double builderPresentationPhase(const sim::World& world, const sim::Lemming& l) {
  if (l.state != sim::State::Builder || l.bricksLeft <= 0) return -1.0;
  const auto& r = world.rules();
  const int tickMs = r.ticksPerSecond > 0 ? 1000 / r.ticksPerSecond : 200;
  if (tickMs <= 0 || r.builderCycleMs <= 0) return -1.0;
  const int laid = std::max(0, r.builderBricks - l.bricksLeft);
  const long long eventMs = static_cast<long long>(r.builderCycleMs) *
      (r.builderBrickPercent - r.builderEnterPercent) / 100;
  const auto placementTick = [&](int brick) {
    return (eventMs + static_cast<long long>(brick) * r.builderCycleMs) / tickMs + 1;
  };
  const auto previous = laid == 0 ? 0 : placementTick(laid - 1);
  const auto next = placementTick(laid);
  const double time = static_cast<double>(l.frame) +
      static_cast<double>(world.levelClockMs() % tickMs) / tickMs;
  const double between = std::clamp((time - previous) / std::max(1LL, next - previous),
                                    0.0, std::nextafter(1.0, 0.0));
  constexpr double placedPhase = 13.0 / 16.0; // native 0.78 / 0.96
  double phase = laid == 0 ? 1.0 / 16.0 + between * (placedPhase - 1.0 / 16.0) :
                             placedPhase + between;
  if (phase >= 1.0) phase -= 1.0;
  // Roundoff must not retire the last carried frame before the terrain callback.
  if (between > 0.5 && phase >= placedPhase)
    phase = std::nextafter(placedPhase, 0.0);
  return phase;
}

struct BuilderBrickShape {
  double scale = 0.0;
  double travel = 0.0;
  double lift = 0.0;
};

inline BuilderBrickShape builderBrickShape(double phase) {
  constexpr double begin = 2.0 / 16.0, end = 13.0 / 16.0;
  if (!std::isfinite(phase) || phase < begin || phase >= end) return {};
  const double progress = (phase - begin) / (end - begin);
  const double travel = std::max(0.0, progress * 2.0 - 1.0);
  // Native smaller geometry is made by /3 about its centre (410E78).
  // Native offset 4104B6 holds Y at +/-5 then brings it down to the map.
  // PROVISIONAL: a uniformly scaled cuboid replaces its folded 12-vertex mesh.
  return {1.0 / 3.0 + progress * 2.0 / 3.0, travel,
          std::min(1.0, (1.0 - travel) * 2.0)};
}

} // namespace webo::app
