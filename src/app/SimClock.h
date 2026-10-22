#pragma once

namespace webo::app {

// Drives `sim::World::tick()` off real wall-clock time, independent of the
// render frame rate -- the same shape as `FixedCameraClock` (CameraInput.h),
// but at the measured 200 ms / 5 Hz sim period (`sim::World::Rules::
// ticksPerSecond`, src/sim/World.h) rather than the 144 Hz camera period.
//
// `World::tick()` itself is unmodified and untouched by this class (brief
// d3b-app-sim-loop does not own src/sim/**): this is purely the accumulator
// that decides *how many times* to call it for a given slice of wall time.
//
// Design, to satisfy "whole ticks, with remainders carried, capped burst per
// frame": `advance()` never discards elapsed time (no clamp on the input), so
// a stalled frame's backlog is never silently lost -- only how fast it is
// drained is bounded. `takeStep()` returns at most `maxTicksPerFrame()` ticks
// per `advance()` call; anything beyond that stays in the carried remainder
// and is drained on later frames. The cap exists so a single very long frame
// (a breakpoint, a slow asset load, alt-tab) cannot force one render to run
// an unbounded burst of sim ticks; it does not exist to keep the sim in sync
// with real time at all costs, since `--sim-ticks N` is the deterministic,
// frame-independent path the D3b gate actually checks.
class SimClock {
 public:
  // MEASURED, World.h `Rules::ticksPerSecond` = 5.
  static constexpr double stepSeconds = 0.2;
  // The stated cap: 10 ticks (2.0 real seconds of sim) per `advance()` call.
  // Large enough that an ordinary frame hitch (anything under two seconds)
  // still catches up in the same frame it is noticed on; small enough that
  // one pathological frame cannot stall the renderer behind an unbounded
  // sim-tick loop. Not itself a measured constant -- a harness choice, named
  // so it can be changed in one place.
  static constexpr int kDefaultMaxTicksPerFrame = 10;

  explicit SimClock(int maxTicksPerFrame = kDefaultMaxTicksPerFrame)
      : maxTicksPerFrame_(maxTicksPerFrame) {}

  // Call once per render frame with that frame's elapsed real seconds.
  // Never clamps or discards: the remainder keeps every unconsumed fraction
  // of a second, including a whole backlog above the per-frame cap.
  void advance(double elapsedSeconds);

  // Call in a loop after advance(): true while a whole tick is owed AND the
  // per-frame cap has not yet been spent this frame. Each call that returns
  // true both subtracts one stepSeconds from the carried remainder and
  // counts against the per-frame cap; a false return means either "not
  // enough time yet" or "cap reached for this frame" -- remainder() /
  // ticksTakenThisFrame() tell the two apart.
  bool takeStep();

  double remainder() const { return remainder_; }
  int maxTicksPerFrame() const { return maxTicksPerFrame_; }
  int ticksTakenThisFrame() const { return ticksTakenThisFrame_; }

 private:
  double remainder_ = 0.0;
  int maxTicksPerFrame_;
  int ticksTakenThisFrame_ = 0;
};

}  // namespace webo::app
