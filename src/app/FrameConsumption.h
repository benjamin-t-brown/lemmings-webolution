#pragma once

#include <functional>
#include <cstdint>
#include <span>
#include <vector>
#include <utility>

namespace webo::app {
struct LevelFrameView;

// Explicit owned-process automation option. Ordinary application entry never
// creates this scope. Only observers that consume simulation/presentation state
// and selected images may opt in; per-frame depth/hover/actor consumers must not.
// Uninspected views have an empty image and no raster-derived fields. Call
// inspectLevelFrame() within the observer before reading those fields or pixels.
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
// Actual branch-1 sprite color stores for scoped image comparisons. This mask
// is diagnostic only; it never changes pixels, depth, RNG or the seeding clock.
inline std::vector<std::uint8_t>& frameSpriteCoverage() {
  static thread_local std::vector<std::uint8_t> coverage;
  return coverage;
}
// Callback lifetime is precisely the synchronous level observer invocation.
// Nesting restores the caller's request, including on an exception.
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
