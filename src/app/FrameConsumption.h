#pragma once

#include <functional>
#include <cstdint>
#include <span>
#include <vector>
#include <utility>

namespace webo::app {
struct LevelFrameView;

// Automation opts in to deferred frame inspection. In the callback, call
// inspectLevelFrame() before reading pixels or raster-derived fields.
class ScopedFrameConsumption {
 public:
  explicit ScopedFrameConsumption(bool skipUnconsumed = true)
      : previous_(active), skip_(skipUnconsumed) { active = this; }
  ~ScopedFrameConsumption() { active = previous_; }
  ScopedFrameConsumption(const ScopedFrameConsumption&) = delete;
  ScopedFrameConsumption& operator=(const ScopedFrameConsumption&) = delete;
  static bool skipUnconsumed() { return active && active->skip_; }
  static bool observing() { return active != nullptr; }
 private:
  inline static thread_local ScopedFrameConsumption* active = nullptr;
  ScopedFrameConsumption* previous_;
  bool skip_;
};

namespace detail {
// Observe branch-1 sprite color stores without affecting drawing or RNG.
inline std::vector<std::uint8_t>& frameSpriteCoverage() {
  static thread_local std::vector<std::uint8_t> coverage;
  return coverage;
}
// Valid during the synchronous level callback. Nested scopes restore the outer request.
inline thread_local const std::function<void()>* pendingFrameInspection = nullptr;
class PendingFrameInspection {
 public:
  explicit PendingFrameInspection(const std::function<void()>& render)
      : previous_(std::exchange(pendingFrameInspection, &render)) {}
  ~PendingFrameInspection() { pendingFrameInspection = previous_; }
 private:
  const std::function<void()>* previous_;
};
}  // namespace detail

inline const LevelFrameView& inspectLevelFrame(const LevelFrameView& frame) {
  if (detail::pendingFrameInspection) (*detail::pendingFrameInspection)();
  return frame;
}
inline std::span<const std::uint8_t> levelFrameSpriteCoverage() {
  return detail::frameSpriteCoverage();
}
}  // namespace webo::app
