#include "app/TouchInput.h"

#include <algorithm>
#include <cmath>

namespace webo::app {
namespace {
// Provisional clone thresholds in presentation pixels and elapsed milliseconds.
constexpr double slop = 8.0;
constexpr std::int32_t holdMs = 180;
constexpr std::uint32_t tapMs = 300;
// Presentation pixels above the contact. A phone fingertip does not shrink
// with the portrait viewport, so this offset stays in screen pixels.
constexpr double fingerLengthPixels = 80.0;
constexpr double hatchSlideThreshold = 12.0;

TouchHatchSlide hatchSlide(double vertical) {
  if (vertical < -hatchSlideThreshold) return TouchHatchSlide::Up;
  if (vertical > hatchSlideThreshold) return TouchHatchSlide::Down;
  return TouchHatchSlide::Neutral;
}

TouchPoint difference(TouchPoint a, TouchPoint b) {
  return {a.x - b.x, a.y - b.y};
}

double distance(TouchPoint a, TouchPoint b) {
  return std::hypot(a.x - b.x, a.y - b.y);
}
}  // namespace

void TouchInput::reset() {
  contacts_.clear();
  steeringFinger_.reset();
  phase_ = Phase::Suppressed;
}

void TouchInput::setViewport(int width, int height) {
  reset();
  width_ = width;
  height_ = height;
  gestureScale_ = double(width) / kGameWidth;
}

TouchPoint TouchInput::clampPoint(TouchPoint point) const {
  return {std::clamp(point.x, 0.0, double(width_ - 1)),
          std::clamp(point.y, 0.0, double(height_ - 1))};
}

TouchPoint TouchInput::cursorPoint(TouchPoint point) const {
  const auto contact = clampPoint(point);
  return {contact.x, std::max(0.0, contact.y - fingerLengthPixels)};
}

bool TouchInput::tracking(TouchId id) const {
  return std::ranges::find(contacts_, id, &Contact::id) != contacts_.end();
}

bool TouchInput::rotationPadHeld() const {
  return target_ == TouchTarget::RotatePad && (phase_ == Phase::RotateReady || phase_ == Phase::Rotate);
}

TouchAction TouchInput::down(TouchId id, TouchPoint point, TouchTarget target, std::uint32_t now) {
  if (tracking(id)) return {};
  contacts_.push_back({id, point});
  if (contacts_.size() == 1) {
    target_ = target;
    phase_ = target_ == TouchTarget::RotatePad ? Phase::RotateReady : Phase::Candidate;
    origin_ = point;
    downAt_ = now;
    if (target_ == TouchTarget::AssignableActor) return {TouchActionKind::Assign, point, {}};
    return {};
  }

  // The first contact anchors rotation. A new steering finger can pick up
  // where the last one left off as long as that anchor is still down.
  if (phase_ == Phase::RotateReady || (contacts_.size() == 2 && phase_ != Phase::Suppressed)) {
    phase_ = Phase::Rotate;
    steeringFinger_ = id;
    return {TouchActionKind::BeginDrag, point, {}};
  }
  return {};
}

TouchAction TouchInput::motion(TouchId id, TouchPoint point) {
  const auto contact = std::ranges::find(contacts_, id, &Contact::id);
  if (contact == contacts_.end()) return {};
  const auto previous = contact->point;
  contact->point = point;

  switch (phase_) {
    case Phase::Candidate:
      if (distance(point, origin_) <= slop * gestureScale_) return {};
      if (target_ == TouchTarget::Scene || target_ == TouchTarget::DragOnly) {
        phase_ = Phase::Cursor;
        return {TouchActionKind::Precision, cursorPoint(point), {}};
      }
      // An assignable actor was already assigned on contact; do not repeat it.
      phase_ = Phase::Moved;
      return {};
    case Phase::Cursor: return {TouchActionKind::Precision, cursorPoint(point), {}};
    case Phase::Hold: return {TouchActionKind::MoveHold, point, {}};
    case Phase::HatchHold: {
      const auto slide = hatchSlide((point.y - origin_.y) / gestureScale_);
      if (slide == hatchSlide_) return {};
      hatchSlide_ = slide;
      return {TouchActionKind::HatchSlide, origin_, {}, slide};
    }
    case Phase::Rotate:
      if (steeringFinger_ == id) return {TouchActionKind::Drag, point, difference(point, previous)};
      return {};
    case Phase::RotateReady:
    case Phase::Moved:
    case Phase::Suppressed: return {};
  }
  return {};
}

TouchAction TouchInput::up(TouchId id, TouchPoint point, std::uint32_t now) {
  const auto contact = std::ranges::find(contacts_, id, &Contact::id);
  if (contact == contacts_.end()) return {};
  const auto phase = phase_;
  const bool anchor = contact == contacts_.begin();
  contacts_.erase(contact);
  if (phase == Phase::Rotate || phase == Phase::RotateReady) {
    if (!anchor && steeringFinger_ != id) return {};
    steeringFinger_.reset();
    phase_ = anchor ? Phase::Suppressed : Phase::RotateReady;
    if (phase == Phase::Rotate) return {TouchActionKind::EndDrag, point, {}};
    return {};
  }
  phase_ = Phase::Suppressed;
  if (phase == Phase::Cursor)
    return {TouchActionKind::EndPrecision, cursorPoint(point), {}, TouchHatchSlide::Neutral,
             clampPoint(point)};
  if (phase == Phase::Hold) return {TouchActionKind::EndHold, point, {}};
  if (phase == Phase::HatchHold) return {TouchActionKind::EndHatchHold, origin_, {}};
  if (phase == Phase::Candidate &&
      (target_ == TouchTarget::Tap || target_ == TouchTarget::Holdable ||
       target_ == TouchTarget::Scene) &&
      now - downAt_ <= tapMs && distance(point, origin_) <= slop * gestureScale_)
    return {TouchActionKind::Tap, origin_, {}};
  return {};
}

TouchAction TouchInput::advance(std::uint32_t now) {
  // The idle sample can predate a down event delivered during the poll.
  // Signed elapsed time handles that ordering and the SDL tick wrap.
  if (phase_ != Phase::Candidate || static_cast<std::int32_t>(now - downAt_) < holdMs) return {};
  switch (target_) {
    case TouchTarget::Hatch:
      phase_ = Phase::HatchHold;
      hatchSlide_ = TouchHatchSlide::Neutral;
      return {TouchActionKind::HatchSlide, origin_, {}};
    case TouchTarget::Holdable:
      phase_ = Phase::Hold;
      return {TouchActionKind::BeginHold, origin_, {}};
    default: return {};
  }
}

}  // namespace webo::app
