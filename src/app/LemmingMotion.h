#pragma once

#include "sim/World.h"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

namespace webo::app {

struct LemmingDrawPosition {
  double x = 0.0;
  double y = 0.0;  // terrain feet, Y down
  double z = 0.75;  // radial blend: 0 inner shell, 1 outer shell
};

// P7, PROVISIONAL presentation only. D1m (0x41BDD4) establishes continuous
// original movement, not this reconstruction's interpolation policy. Buffer
// confirmed cell positions by one bookkeeping interval, then linearly sample
// that path on every frame. Never extrapolate. Sparse skill steps occupy at
// most the preceding interval; their intervening stationary time stays still.
// Turns, gravity changes, teleport/stationary transitions and implausible jumps
// reset the path immediately; compatible moving states retain it through landing.
// An original per-frame actor-position/render trace would settle the policy.
// World is read-only; its level clock supplies pause, stall cap and fast-forward.
class LemmingMotionTracker {
 public:
  // Call before advancing and after EACH World update, including fast-forward
  // passes. Identity is the stable append-only World::lemmings() index.
  void observe(const sim::World& world);
  LemmingDrawPosition positionOf(std::size_t index, const sim::World& world) const;
  // Presentation only: saved actors retain a one-second jump, without becoming
  // alive/assignable again. Negative outside that interval. Uses level time.
  double exitSeconds(std::size_t index, const sim::World& world) const;

 private:
  struct Point {
    std::uint64_t ms = 0;
    double x = 0.0, y = 0.0;  // X unwrapped across the cylinder seam
  };
  struct Entry {
    bool valid = false;
    int x = 0, y = 0, gravity = 1;
    sim::Facing facing = sim::Facing::Right;
    sim::State state = sim::State::Faller;
    std::uint64_t observedMs = 0;
    // Q1: keep radial presentation separate from the discontinuity-reset X/Y path.
    double laneZ = 0.75;
    double exitStepSeconds = 0.0;
    std::uint64_t exitUpdatedMs = 0;
    std::deque<Point> path;
  };
  std::vector<Entry> entries_;
};

}  // namespace webo::app
