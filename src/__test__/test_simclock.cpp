#include "app/SimClock.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

using namespace webo::app;

namespace {
void check(bool condition, const char* message) {
  if (!condition) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
}  // namespace

int main() {
  // Sub-step elapsed time produces no tick.
  {
    SimClock clock;
    clock.advance(0.5 * SimClock::stepSeconds);
    check(!clock.takeStep(), "sub-step elapsed time produces no tick");
  }

  // Carry remainders between advances to accumulate a whole tick.
  {
    SimClock clock;
    clock.advance(0.5 * SimClock::stepSeconds);
    clock.advance(0.5 * SimClock::stepSeconds);
    check(clock.takeStep(), "two half-steps combine into one whole tick");
    check(!clock.takeStep(), "one frame's worth of accumulated time yields only one tick");
  }

  // A multi-tick backlog inside the cap is drained in one frame.
  {
    SimClock clock;
    clock.advance(3.0 * SimClock::stepSeconds);
    int steps = 0;
    while (clock.takeStep()) ++steps;
    check(steps == 3, "a backlog within the cap drains fully in one frame");
    check(std::abs(clock.remainder()) < 1e-9, "draining a whole-tick backlog leaves no remainder");
  }

  // Partitioning elapsed time preserves the tick count within the per-call cap.
  {
    SimClock grouped(100), partitioned(100);
    grouped.advance(4.0 * SimClock::stepSeconds);
    for (int i = 0; i < 4; ++i) partitioned.advance(SimClock::stepSeconds);
    int groupedSteps = 0, partitionedSteps = 0;
    while (grouped.takeStep()) ++groupedSteps;
    while (partitioned.takeStep()) ++partitionedSteps;
    check(groupedSteps == 4 && partitionedSteps == 4,
         "equal elapsed time partitioned differently produces equal tick counts");
  }

  // One second accumulates five ticks at 5 Hz.
  {
    SimClock clock;
    for (int i = 0; i < 4; ++i) clock.advance(0.25);
    int steps = 0;
    while (clock.takeStep()) ++steps;
    check(steps == 5, "one real second produces 5 ticks at the measured 200 ms period");
  }

  // Limit a single frame to maxTicksPerFrame() and carry the excess.
  {
    SimClock clock(10);
    clock.advance(100.0 * SimClock::stepSeconds);  // 100 ticks owed
    int firstFrameSteps = 0;
    while (clock.takeStep()) ++firstFrameSteps;
    check(firstFrameSteps == 10, "one catastrophic frame takes at most the stated cap of ticks");
    check(clock.ticksTakenThisFrame() == 10, "ticksTakenThisFrame reports the capped count");
    check(std::abs(clock.remainder() - 90.0 * SimClock::stepSeconds) < 1e-9,
         "time beyond the per-frame cap is carried, not discarded");

    // The carried backlog is NOT lost: it keeps draining on later frames
    // (each still capped) until it is gone, recovering every owed tick.
    int totalSteps = firstFrameSteps;
    int frames = 1;
    while (clock.remainder() + 1e-9 >= SimClock::stepSeconds && frames < 50) {
      clock.advance(0.0);  // no new time this frame, just drain the backlog
      int frameSteps = 0;
      while (clock.takeStep()) ++frameSteps;
      check(frameSteps <= 10, "every later frame draining the backlog also stays within the cap");
      totalSteps += frameSteps;
      ++frames;
    }
    check(totalSteps == 100, "every owed tick from the catastrophic frame is eventually taken, none lost");
  }

  // Repeated advances accumulate time even without draining ticks.
  {
    SimClock clock;
    clock.advance((1.0 / 3.0) * SimClock::stepSeconds);
    clock.advance((1.0 / 3.0) * SimClock::stepSeconds);
    clock.advance((1.0 / 3.0) * SimClock::stepSeconds);
    check(clock.takeStep(), "three uncapped partial advances combine into a whole tick");
    check(!clock.takeStep(), "no second tick from three thirds of a step");
  }

  std::puts("SimClock: all checks passed");
  return 0;
}
