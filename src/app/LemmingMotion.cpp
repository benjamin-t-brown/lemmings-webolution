#include "app/LemmingMotion.h"
#include "app/ExitBalloon.h"

#include <algorithm>
#include <cmath>

namespace webo::app {
namespace {
constexpr double kCenterLane = 0.5;

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
double walkingLane(sim::Facing facing) {
  // READ 0x42F93C / 0x42F977: direction -1 -> 0.25, otherwise 0.75.
  return facing == sim::Facing::Left ? 0.25 : 0.75;
}
double initialLane(const sim::Lemming& l) {
  // READ 0x42DE51: hatch releases start halfway across the walkway.
  if (l.releasedFromHatch || l.state == sim::State::Blocker) return kCenterLane;
  return l.state == sim::State::Walker ? walkingLane(l.facing) : 0.75;
}
}

void LemmingMotionTracker::observe(const sim::World& world) {
  const auto now = world.levelClockMs();
  entries_.resize(world.lemmings().size());
  for (std::size_t i = 0; i < entries_.size(); ++i) {
    auto& e = entries_[i];
    const auto& l = world.lemmings()[i];
    if (l.state == sim::State::Exited) {
      if (!e.valid || e.state != sim::State::Exited || now < e.observedMs) {
        const double lane = e.valid ? e.laneZ :
            (l.releasedFromHatch ? kCenterLane : walkingLane(l.facing));
        e = {};
        e.valid = true;
        e.state = sim::State::Exited;
        e.laneZ = lane;
        e.observedMs = e.exitUpdatedMs = now;
      } else if (now > e.exitUpdatedMs) {
        e.exitStepSeconds = (now - e.exitUpdatedMs) / 1000.0;
        e.exitUpdatedMs = now;
      }
      continue;
    }
    if (!l.alive()) { e = {}; continue; }
    if (!e.valid || now < e.observedMs) {
      e.laneZ = initialLane(l);
      e.firstHatchDrawPending = l.releasedFromHatch;
    } else if (e.state == sim::State::Walker || e.state == sim::State::Blocker) {
      // READ 0x42F977: walking approaches its facing's lane by 0.02 per update.
      // READ 0x40E053 -> 0x42EDCD: blocking approaches the midpoint.
      // Provisional clone rate for both: one slab unit per level second.
      // Use the previous confirmed state and facing for the elapsed interval.
      // Falling and other states hold their lane until walking resumes.
      const double target = e.state == sim::State::Blocker ? kCenterLane : walkingLane(e.facing);
      const double step = (now - e.observedMs) / 1000.0;
      e.laneZ += std::clamp(target - e.laneZ, -step, step);
    }
    observePosition(e, l, world, false);
  }

  const auto& rats = world.rats();
  ratEntries_.resize(rats.size());
  for (std::size_t i = 0; i < rats.size(); ++i) {
    const auto& rat = rats[i];
    auto& e = ratEntries_[i];
    if (!rat.actor.alive()) { e = {}; continue; }
    // The rat lane starts at its stored Z and already eases while walking.
    // Punches keep the sim's movement state, so explicitly pin them at contact.
    e.laneZ = rat.lane;
    observePosition(e, rat.actor, world, rat.pendingVictim >= 0);
  }
}

void LemmingMotionTracker::frameDrawn() {
  for (auto& e : entries_) {
    if (!e.firstHatchDrawPending) continue;
    // Start the visible transition from the sample just presented. Until then,
    // laneZ still tracks the depth used by an exit jump before a live draw.
    e.laneZ = kCenterLane;
    e.firstHatchDrawPending = false;
  }
}

void LemmingMotionTracker::observePosition(Entry& e, const sim::Lemming& l,
                                           const sim::World& world, bool snapToCurrent) {
  const auto now = world.levelClockMs(), interval = intervalMs(world);
  const int width = world.terrain().width();
  int dx = l.x - e.x;
  if (width > 0) {
    if (dx > width / 2) dx -= width;
    if (dx < -width / 2) dx += width;
  }
  const int dy = l.y - e.y;
  // Even adjacent teleport arrivals snap. Compatible moving-state changes
  // retain the confirmed path so a landing doesn't skip a buffered fall.
  const auto elapsed = now >= e.observedMs ? now - e.observedMs : 0;
  const double intervals = std::max(1.0, static_cast<double>(elapsed) / interval);
  const auto& rules = world.rules();
  const double limitX = (l.state == sim::State::Bounce ? rules.bounceSpeed :
      std::max(2, rules.walkSpeed * 2)) * intervals;
  const double limitY = std::max({rules.fallSpeed, rules.climbSpeed, rules.maxStepUp,
      l.state == sim::State::Bounce ? rules.bounceSpeed : 2}) * intervals;
  const bool stateBreak = e.state != l.state && !(movingState(e.state) && movingState(l.state));
  const bool snap = snapToCurrent || !e.valid || now < e.observedMs || stateBreak ||
      e.facing != l.facing || e.gravity != l.verticalDir ||
      l.state == sim::State::Teleporting || std::abs(dx) > limitX || std::abs(dy) > limitY ||
      (l.state == sim::State::Walker && dx * static_cast<int>(l.facing) < 0);
  if (snap) {
    e.path.clear();
    e.path.push_back({now, static_cast<double>(l.x), static_cast<double>(l.y)});
  } else if (dx != 0 || dy != 0) {
    // A cell step has no authoritative within-update timestamp. Spread it over
    // at most the preceding interval. All endpoints are confirmed positions.
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

double LemmingMotionTracker::exitSeconds(std::size_t index, const sim::World& world) const {
  if (index >= entries_.size() || index >= world.lemmings().size() ||
      world.lemmings()[index].state != sim::State::Exited) return -1.0;
  const auto& e = entries_[index];
  const auto now = world.levelClockMs();
  if (!e.valid || e.state != sim::State::Exited || now < e.observedMs) return -1.0;
  const double age = (now - e.observedMs) / 1000.0;
  return age < kExitJumpSeconds ? age : -1.0;
}

double LemmingMotionTracker::exitBalloonSwayScale(const sim::World& world) const {
  const auto now = world.levelClockMs();
  double scale = 1.0;
  for (const auto& e : entries_) {
    if (!e.valid || e.state != sim::State::Exited || now < e.observedMs) continue;
    const double age = (now - e.observedMs) / 1000.0;
    // A timeout can start departure before a rescue. Ease back to center
    // during lift-off, hold through landing, then restore the sway. The
    // strongest restraint wins when jumps overlap.
    const double fadeOut = 1.0 - age / kExitBalloonSwayBlendSeconds;
    const double fadeIn = (age - kExitJumpSeconds) / kExitBalloonSwayBlendSeconds;
    scale = std::min(scale, std::max({0.0, fadeOut, fadeIn}));
  }
  return scale;
}

LemmingDrawPosition LemmingMotionTracker::positionOf(std::size_t index, const sim::World& world) const {
  if (index >= world.lemmings().size()) return {};
  const auto& l = world.lemmings()[index];
  const double exitTime = exitSeconds(index, world);
  if (exitTime >= 0.0) {
    const auto& e = entries_[index];
    const int width = world.terrain().width();
    for (const auto& balloon : world.setup().exits) {
      const double contactX = wrapped(l.x - balloon.x + width / 2, width) - width / 2;
      if (contactX < 0 || contactX > balloon.width || l.y < balloon.y ||
          l.y > balloon.y + balloon.height) continue;
      const double dx = wrapped(balloon.x + balloon.width * 0.5 - l.x + width * 0.5, width) - width * 0.5;
      // Invert the accepted retail trigger adapter: box top is model Y - 6.
      // PROVISIONAL: rounded static pose rather than the exact serialized root;
      // all exit themes use this box. Native target adds (0,2,0) to the root;
      // Y-down therefore subtracts another 2. The exit root's radial Z is 1.5.
      const double dy = balloon.y - 8.0 - l.y;
      const double dz = 1.5 - e.laneZ;
      const double speed = std::sqrt(dx * dx + dy * dy + dz * dz);
      // READ 417FE4/41818A: fall clip, length(delta) impulse, -12.5*t*t,
      // then delta*dt. Y is reset from entry every update before that addition;
      // it deliberately uses dy*dt, not dy*t. X/Z accumulate to the target.
      const double lift = speed * exitTime - 12.5 * exitTime * exitTime;
      // The original jump captures the bobbing target on entry (0x41803B).
      // PROVISIONAL presentation adapter: blend the same current bob used by
      // BackdropScene into the accepted jump. Feet start at their unchanged
      // contact position and converge on the moving basket, including inverted
      // actors (the model's world-Y bob does not reverse with actor gravity).
      const double bob = exitBalloonBob(world.levelClockMs() / 1000.0) * exitTime;
      return {wrapped(l.x + dx * exitTime, width),
          l.y - l.verticalDir * lift + dy * e.exitStepSeconds - bob,
          e.laneZ + dz * exitTime};
    }
  }
  return samplePosition(entries_, index, world, {static_cast<double>(l.x), static_cast<double>(l.y),
      l.alive() ? initialLane(l) : 0.75});
}

LemmingDrawPosition LemmingMotionTracker::ratPositionOf(std::size_t index, const sim::World& world) const {
  if (index >= world.rats().size()) return {};
  const auto& rat = world.rats()[index];
  return samplePosition(ratEntries_, index, world,
      {static_cast<double>(rat.actor.x), static_cast<double>(rat.actor.y), rat.lane});
}

LemmingDrawPosition LemmingMotionTracker::samplePosition(const std::vector<Entry>& entries,
    std::size_t index, const sim::World& world, LemmingDrawPosition confirmed) {
  if (index >= entries.size() || !entries[index].valid || entries[index].path.empty())
    return confirmed;
  const auto& entry = entries[index];
  const auto& path = entry.path;
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
  return {wrapped(p.x, world.terrain().width()), p.y,
          entry.firstHatchDrawPending ? kCenterLane : entry.laneZ};
}

}  // namespace webo::app
