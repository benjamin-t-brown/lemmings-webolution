#pragma once

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
  Precision, EndPrecision, CancelPrecision, HatchSlide, EndHatchHold, ZoomIn, ZoomOut
};

struct TouchAction {
  TouchActionKind kind = TouchActionKind::None;
  TouchPoint point;
  TouchPoint delta;
  TouchHatchSlide hatchSlide = TouchHatchSlide::Neutral;
};

// SDL-free recognition. Positions are in 640x480 presentation pixels and time
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
  void reset() { *this = {}; }

 private:
  enum class Phase { Candidate, Drag, Hold, Precision, HatchHold, Pinch, Moved, Suppressed };
  struct Contact { TouchId id; TouchPoint point; };

  TouchAction pinchMotion();

  std::vector<Contact> contacts_;
  Phase phase_ = Phase::Suppressed;
  TouchTarget target_ = TouchTarget::Tap;
  TouchPoint origin_;
  std::uint32_t downAt_ = 0;
  TouchHatchSlide hatchSlide_ = TouchHatchSlide::Neutral;
  double pinchReference_ = 0;
  TouchActionKind pinchDirection_ = TouchActionKind::None;
};

}  // namespace webo::app
