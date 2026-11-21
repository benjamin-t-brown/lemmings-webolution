#pragma once

namespace webo::app {

// Accumulate wall time for sim::World::tick at 5 Hz (200 ms).
// Cap ticks per frame to bound stalls and carry any backlog.
class SimClock {
 public:
  static constexpr double stepSeconds = 0.2;
  // A burst of ten ticks drains up to 2.0 s of backlog per frame.
  static constexpr int kDefaultMaxTicksPerFrame = 10;

  explicit SimClock(int maxTicksPerFrame = kDefaultMaxTicksPerFrame)
      : maxTicksPerFrame_(maxTicksPerFrame) {}

  // Add the frame's elapsed seconds to the unconsumed time.
  void advance(double elapsedSeconds);

  // Call after advance(). Each successful call consumes stepSeconds
  // and one slot in the frame's tick budget.
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
