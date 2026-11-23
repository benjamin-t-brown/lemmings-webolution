#include "app/TouchInput.h"
#include "app/WindowScale.h"

#include <algorithm>
#include <cmath>

namespace webo::app {
namespace {
// Provisional clone thresholds in presentation pixels and elapsed milliseconds.
constexpr double slop = 8.0;
constexpr double pinchRatio = 1.20;
constexpr std::int32_t holdMs = 180;
constexpr std::uint32_t tapMs = 300;
constexpr double precisionOffset = 28.0;
constexpr double hatchSlideThreshold = 12.0;

TouchPoint precisionPoint(TouchPoint point) {
  return {std::clamp(point.x, 0.0, double(kGameWidth - 1)),
          std::clamp(point.y - precisionOffset, 0.0, double(kGameHeight - 1))};
}

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

bool TouchInput::tracking(TouchId id) const {
  return std::ranges::find(contacts_, id, &Contact::id) != contacts_.end();
}

TouchAction TouchInput::down(TouchId id, TouchPoint point, TouchTarget target, std::uint32_t now) {
  if (tracking(id)) return {};
  contacts_.push_back({id, point});
  if (contacts_.size() == 1) {
    phase_ = Phase::Candidate;
    target_ = target;
    origin_ = point;
    downAt_ = now;
    if (target_ == TouchTarget::AssignableActor) return {TouchActionKind::Assign, point, {}};
    return {};
  }

  TouchActionKind cancellation = TouchActionKind::None;
  if (phase_ == Phase::Drag) cancellation = TouchActionKind::CancelDrag;
  if (phase_ == Phase::Hold) cancellation = TouchActionKind::CancelHold;
  if (phase_ == Phase::Precision) cancellation = TouchActionKind::CancelPrecision;
  if (phase_ == Phase::HatchHold) cancellation = TouchActionKind::EndHatchHold;
  // A third contact or a finger left over from a pinch keeps the gesture
  // suppressed until every finger lifts.
  if (contacts_.size() == 2 && phase_ != Phase::Suppressed) {
    phase_ = Phase::Pinch;
    pinchReference_ = std::max(slop, distance(contacts_.front().point, point));
    pinchDirection_ = TouchActionKind::None;
  } else {
    phase_ = Phase::Suppressed;
  }
  return {cancellation, point, {}};
}

TouchAction TouchInput::motion(TouchId id, TouchPoint point) {
  const auto contact = std::ranges::find(contacts_, id, &Contact::id);
  if (contact == contacts_.end()) return {};
  const TouchPoint previous = contact->point;
  contact->point = point;
  switch (phase_) {
    case Phase::Candidate:
      if (distance(point, origin_) <= slop) return {};
      if (target_ == TouchTarget::Scene || target_ == TouchTarget::DragOnly ||
          target_ == TouchTarget::AssignableActor) {
        phase_ = Phase::Drag;
        return {TouchActionKind::BeginDrag, point, difference(point, origin_)};
      }
      phase_ = Phase::Moved;
      return {};
    case Phase::Drag: return {TouchActionKind::Drag, point, difference(point, previous)};
    case Phase::Hold: return {TouchActionKind::MoveHold, point, {}};
    case Phase::Precision: return {TouchActionKind::Precision, precisionPoint(point), {}};
    case Phase::HatchHold: {
      const auto slide = hatchSlide(point.y - origin_.y);
      if (slide == hatchSlide_) return {};
      hatchSlide_ = slide;
      return {TouchActionKind::HatchSlide, origin_, {}, slide};
    }
    case Phase::Pinch: return pinchMotion();
    case Phase::Moved:
    case Phase::Suppressed: return {};
  }
  return {};
}

TouchAction TouchInput::up(TouchId id, TouchPoint point, std::uint32_t now) {
  const auto contact = std::ranges::find(contacts_, id, &Contact::id);
  if (contact == contacts_.end()) return {};
  const auto phase = phase_;
  const TouchPoint previous = contact->point;
  contacts_.erase(contact);
  phase_ = Phase::Suppressed;
  if (phase == Phase::Drag) return {TouchActionKind::EndDrag, point, difference(point, previous)};
  if (phase == Phase::Hold) return {TouchActionKind::EndHold, point, {}};
  if (phase == Phase::Precision) return {TouchActionKind::EndPrecision, precisionPoint(point), {}};
  if (phase == Phase::HatchHold) return {TouchActionKind::EndHatchHold, origin_, {}};
  if (phase == Phase::Candidate && (target_ == TouchTarget::Tap || target_ == TouchTarget::Holdable) &&
      now - downAt_ <= tapMs && distance(point, origin_) <= slop)
    return {TouchActionKind::Tap, origin_, {}};
  return {};
}

TouchAction TouchInput::advance(std::uint32_t now) {
  // The idle sample can predate a down event delivered during the poll.
  // Signed elapsed time handles that ordering and the SDL tick wrap.
  if (phase_ != Phase::Candidate || static_cast<std::int32_t>(now - downAt_) < holdMs) return {};
  switch (target_) {
    case TouchTarget::Scene:
      phase_ = Phase::Precision;
      return {TouchActionKind::Precision, precisionPoint(contacts_.front().point), {}};
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

TouchAction TouchInput::pinchMotion() {
  const auto a = contacts_[0].point, b = contacts_[1].point;
  const double spread = std::max(slop, distance(a, b));
  auto direction = TouchActionKind::None;
  if (pinchDirection_ != TouchActionKind::ZoomIn && spread >= pinchReference_ * pinchRatio)
    direction = TouchActionKind::ZoomIn;
  if (pinchDirection_ != TouchActionKind::ZoomOut && spread <= pinchReference_ / pinchRatio)
    direction = TouchActionKind::ZoomOut;
  if (direction != TouchActionKind::None) {
    pinchDirection_ = direction;
    pinchReference_ = spread;
    return {direction, {(a.x + b.x) / 2, (a.y + b.y) / 2}, {}};
  }

  // Follow the extreme in the active direction so a reversal can trigger
  // anywhere in the pinch, rather than requiring a return to its start.
  if (pinchDirection_ == TouchActionKind::ZoomIn)
    pinchReference_ = std::max(pinchReference_, spread);
  if (pinchDirection_ == TouchActionKind::ZoomOut)
    pinchReference_ = std::min(pinchReference_, spread);
  return {};
}

}  // namespace webo::app
