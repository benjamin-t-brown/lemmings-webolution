#include "app/LemmingMotion.h"

#include <algorithm>
#include <cmath>

namespace webo::app {
namespace {
std::uint64_t intervalMs(const sim::World& world) {
  return static_cast<std::uint64_t>(std::max(1, 1000 / std::max(1, world.rules().ticksPerSecond)));
}
std::uint64_t drawTime(const sim::World& world) {
  const auto now = world.levelClockMs(), delay = intervalMs(world);
  return now > delay ? now - delay : 0;
}
double wrapped(double x, int width) {
  if (width <= 0) return x;
  x = std::fmod(x, static_cast<double>(width));
  return x < 0.0 ? x + width : x;
}
bool movingState(sim::State state) {
  using sim::State;
  switch (state) {
    case State::Walker: case State::Faller: case State::Floater:
    case State::Climber: case State::Hoister: case State::Builder:
    case State::Basher: case State::Miner: case State::Digger:
    case State::Bounce: return true;
    default: return false;
  }
}
}

void LemmingMotionTracker::observe(const sim::World& world) {
  const auto now = world.levelClockMs(), interval = intervalMs(world);
  entries_.resize(world.lemmings().size());
  for (std::size_t i = 0; i < entries_.size(); ++i) {
    auto& e = entries_[i];
    const auto& l = world.lemmings()[i];
    if (!l.alive()) { e = {}; continue; }
    const int width = world.terrain().width();
    int dx = l.x - e.x;
    if (width > 0) {
      if (dx > width / 2) dx -= width;
      if (dx < -width / 2) dx += width;
    }
    const int dy = l.y - e.y;
    // Conservative discontinuity guard, not a new simulation speed rule.
    // Even adjacent teleport arrivals snap. Compatible moving-state changes
    // retain the confirmed path: resetting a landing would jump over the
    // buffered fall. Sampling each update also retains fast-forward turns.
    const auto elapsed = now >= e.observedMs ? now - e.observedMs : 0;
    const double intervals = std::max(1.0, static_cast<double>(elapsed) / interval);
    const auto& rules = world.rules();
    const double limitX = (l.state == sim::State::Bounce ? rules.bounceSpeed :
        std::max(2, rules.walkSpeed * 2)) * intervals;
    const double limitY = std::max({rules.fallSpeed, rules.climbSpeed, rules.maxStepUp,
        l.state == sim::State::Bounce ? rules.bounceSpeed : 2}) * intervals;
    const bool stateBreak = e.state != l.state && !(movingState(e.state) && movingState(l.state));
    const bool snap = !e.valid || now < e.observedMs || stateBreak ||
        e.facing != l.facing || e.gravity != l.verticalDir ||
        l.state == sim::State::Teleporting || std::abs(dx) > limitX || std::abs(dy) > limitY ||
        (l.state == sim::State::Walker && dx * static_cast<int>(l.facing) < 0);
    if (snap) {
      e.path.clear();
      e.path.push_back({now, static_cast<double>(l.x), static_cast<double>(l.y)});
    } else if (dx != 0 || dy != 0) {
      // A cell step has no authoritative within-update timestamp. Spread it
      // over at most the preceding interval, with a hold before a sparse step.
      // All endpoints are confirmed positions; interpolation cannot overshoot.
      const auto last = e.path.back();
      const auto start = std::max(last.ms, now > interval ? now - interval : 0);
      if (start > last.ms) e.path.push_back({start, last.x, last.y});
      e.path.push_back({now, last.x + dx, static_cast<double>(l.y)});
    }
    e.valid = true; e.x = l.x; e.y = l.y; e.gravity = l.verticalDir;
    e.state = l.state; e.facing = l.facing; e.observedMs = now;
    const auto sample = drawTime(world);
    while (e.path.size() > 1 && e.path[1].ms <= sample) e.path.pop_front();
  }
}

LemmingDrawPosition LemmingMotionTracker::positionOf(std::size_t index, const sim::World& world) const {
  if (index >= world.lemmings().size()) return {};
  const auto& l = world.lemmings()[index];
  if (index >= entries_.size() || !entries_[index].valid || entries_[index].path.empty())
    return {static_cast<double>(l.x), static_cast<double>(l.y)};
  const auto& path = entries_[index].path;
  const auto t = drawTime(world);
  Point p = path.front();
  for (std::size_t j = 1; j < path.size() && t > p.ms; ++j) {
    const auto& next = path[j];
    if (t >= next.ms) { p = next; continue; }
    const double alpha = std::clamp(static_cast<double>(t - p.ms) / (next.ms - p.ms), 0.0, 1.0);
    p.x += (next.x - p.x) * alpha;
    p.y += (next.y - p.y) * alpha;
    break;
  }
  return {wrapped(p.x, world.terrain().width()), p.y};
}

}  // namespace webo::app
