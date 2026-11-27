#pragma once

#include "app/WindowScale.h"

#include <cstdint>
#include <vector>

namespace webo::app {

struct TouchPoint {
  double x = 0, y = 0;
};

struct TouchId {
  std::int64_t device, finger;
  bool operator==(const TouchId&) const = default;
};

enum class TouchTarget { Tap, Scene, DragOnly, AssignableActor, Holdable, Hatch };
enum class TouchHatchSlide { Neutral, Up, Down };
enum class TouchActionKind {
  None, Tap, Assign, BeginDrag, Drag, EndDrag, CancelDrag,
  BeginHold, MoveHold, EndHold, CancelHold,
  Precision, EndPrecision, CancelPrecision, HatchSlide, EndHatchHold
};

struct TouchAction {
  TouchActionKind kind = TouchActionKind::None;
  TouchPoint point;
  TouchPoint delta;
  TouchHatchSlide hatchSlide = TouchHatchSlide::Neutral;
  // EndPrecision keeps the aimed cursor in point and the raw lift contact here.
  TouchPoint releasePoint{};
};

// SDL-free recognition. Positions are in presentation pixels and time
// is in unsigned milliseconds from the same monotonic clock. Advance to each
// event's timestamp before dispatching it, and advance during idle polls.
class TouchInput {
 public:
  TouchAction down(TouchId id, TouchPoint point, TouchTarget target, std::uint32_t now);
  TouchAction motion(TouchId id, TouchPoint point);
  TouchAction up(TouchId id, TouchPoint point, std::uint32_t now);
  TouchAction advance(std::uint32_t now);
  bool idle() const { return contacts_.empty(); }
  bool tracking(TouchId id) const;
  void reset();
  void setViewport(int width, int height);

 private:
  enum class Phase { Candidate, Cursor, Hold, HatchHold, Rotate, Moved, Suppressed };
  struct Contact { TouchId id; TouchPoint point; };

  TouchPoint clampPoint(TouchPoint point) const;
  TouchPoint cursorPoint(TouchPoint point) const;

  int width_ = kGameWidth, height_ = kGameHeight;
  double gestureScale_ = 1;
  std::vector<Contact> contacts_;
  Phase phase_ = Phase::Suppressed;
  TouchTarget target_ = TouchTarget::Tap;
  TouchPoint origin_;
  std::uint32_t downAt_ = 0;
  TouchHatchSlide hatchSlide_ = TouchHatchSlide::Neutral;
};

}  // namespace webo::app
