#include "app/TouchInput.h"

#include <algorithm>
#include <cmath>

namespace webo::app {
namespace {
// Provisional clone thresholds in presentation pixels and elapsed milliseconds.
constexpr double slop = 8.0;
constexpr std::int32_t holdMs = 180;
constexpr std::uint32_t tapMs = 300;
// The visible cursor sits one fingertip above the contact so it stays visible.
constexpr double fingerLengthPixels = 28.0;
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

TouchPoint centre(TouchPoint a, TouchPoint b) {
  return {(a.x + b.x) / 2.0, (a.y + b.y) / 2.0};
}
}  // namespace

void TouchInput::reset() {
  contacts_.clear();
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
  return {contact.x, std::max(0.0, contact.y - fingerLengthPixels * gestureScale_)};
}

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

  // A second finger turns the gesture into camera rotation. Any leftover
  // contact keeps it suppressed until every finger lifts.
  if (contacts_.size() == 2 && phase_ != Phase::Suppressed) {
    phase_ = Phase::Rotate;
    return {TouchActionKind::BeginDrag, centre(contacts_.front().point, point), {}};
  }
  phase_ = Phase::Suppressed;
  return {};
}

TouchAction TouchInput::motion(TouchId id, TouchPoint point) {
  const bool rotating = phase_ == Phase::Rotate && contacts_.size() == 2;
  const TouchPoint previousCentre = rotating ?
      centre(contacts_.front().point, contacts_[1].point) : TouchPoint{};

  const auto contact = std::ranges::find(contacts_, id, &Contact::id);
  if (contact == contacts_.end()) return {};
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
    case Phase::Rotate: {
      if (contacts_.size() != 2) return {};
      const auto currentCentre = centre(contacts_.front().point, contacts_[1].point);
      return {TouchActionKind::Drag, currentCentre, difference(currentCentre, previousCentre)};
    }
    case Phase::Moved:
    case Phase::Suppressed: return {};
  }
  return {};
}

TouchAction TouchInput::up(TouchId id, TouchPoint point, std::uint32_t now) {
  const auto contact = std::ranges::find(contacts_, id, &Contact::id);
  if (contact == contacts_.end()) return {};
  const auto phase = phase_;
  contacts_.erase(contact);
  phase_ = Phase::Suppressed;
  if (phase == Phase::Rotate) return {TouchActionKind::EndDrag, point, {}};
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
