#include "app/LemmingAnim.h"

#include "app/ActorSprite.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>

namespace webo::app {

namespace {

bool isClip(const char* a, const char* b) { return std::strcmp(a, b) == 0; }

// The controller scales elapsed time by ActorAnimStrip::rate (strip +0x10).
// Missing or unusable speeds default to 1.0.
double clipRate(const Sprite& clip) {
  return std::isfinite(clip.rate) && clip.rate > 0.0 ? clip.rate : 1.0;
}

}  // namespace

const char* lemmingClipName(const sim::Lemming& lemming) {
  using sim::State;
  switch (lemming.state) {
    case State::Walker: return "walk";            // READ: walk strip, controller index 13
    case State::Climber: return "climb";
    case State::Hoister: return "flip";           // READ: hoist cycle is strip 8 "flip", 14 entries
    case State::Builder: return "build";          // READ: build strip 2, 16 entries
    case State::Shrugger: return "shrug";         // READ: strip 10
    case State::Basher: return "bash";            // READ: strip 0
    case State::Miner: return "mine";             // READ: strip 9
    case State::Digger: return "dig";             // READ: strip 4
    case State::Drown: return "drown";            // The state enters with "drown".
    case State::Teleporting: return "cartwheel";  // The pad state selects "cartwheel".
    case State::Floater: return "umbopen";        // Umbrella opening transitions to "float".
    case State::Blocker: return "block";          // PROVISIONAL: the only blocker-named strip
    case State::Exploder: return "explode";       // actor_flapper handles pre-blast type 0x20.
    case State::Faller: return "fall";            // actor_falling entry clip.
    case State::Bounce: return "fall";            // actor_bouncing entry clip.
    case State::Splatter:                         // PROVISIONAL: no splat strip; the fall pose, held
    case State::Exited:
    case State::Dead: return "fall";
  }
  return "walk";
}

bool lemmingClipLoops(const char* clip) {
  // PROVISIONAL: cyclic actions repeat; one-shot ones hold their last entry.
  return !(isClip(clip, "umbopen") || isClip(clip, "flip") || isClip(clip, "shrug") ||
           isClip(clip, "explode") || isClip(clip, "drown"));
}

void LemmingAnimTracker::advance(const sim::World& world, const SpriteSheet& sprites,
                                 double elapsedSeconds) {
  advance(world, std::array<const SpriteSheet*, 3>{&sprites, &sprites, &sprites}, elapsedSeconds);
}

void LemmingAnimTracker::advance(const sim::World& world,
                                 const std::array<const SpriteSheet*, 3>& sprites,
                                 double elapsedSeconds) {
  const auto& lemmings = world.lemmings();
  if (entries_.size() < lemmings.size())
    entries_.resize(lemmings.size());
  for (std::size_t i = 0; i < lemmings.size(); ++i) {
    const sim::Lemming& lemming = lemmings[i];
    if (!lemming.alive() && lemming.state != sim::State::Exited)
      continue;
    Entry& e = entries_[i];
    if (!e.valid || e.state != lemming.state) {
      e.valid = true;
      e.state = lemming.state;
      e.clip = lemmingClipName(lemming);
      e.clock = ActorAnimClockState{};
      e.seconds = 0.0;
      e.clock.loopAtEnd = lemmingClipLoops(e.clip) && lemming.state != sim::State::Splatter;
    }
    e.seconds += std::max(0.0, elapsedSeconds);
    const int type = lemming.sheetType;
    const SpriteSheet* sheet = type >= 0 && type < 3 ? sprites[type] : nullptr;
    const Sprite* clip = sheet ? sheet->find(e.clip) : nullptr;
    if (clip == nullptr || clip->animations.empty())
      continue;
    const ActorAnimStrip strip{static_cast<int>(clip->animations.size()), clipRate(*clip)};
    e.clock = advanceActorAnimClock(e.clock, strip, std::max(0.0, elapsedSeconds));
    // Native actor_falling umbrella completion selects "float". The provisional clone
    // handover occurs at frame end and discards excess opening time.
    if (isClip(e.clip, "umbopen") && e.clock.wrapped) {
      e.clip = "float";
      e.clock = ActorAnimClockState{};
      e.clock.loopAtEnd = true;
    }
  }
}

LemmingAnimPose LemmingAnimTracker::poseOf(std::size_t index) const {
  if (index >= entries_.size() || !entries_[index].valid)
    return {};
  const auto& e = entries_[index];
  return {e.clip, e.clock.temporalIndex, e.state == sim::State::Drown ? e.seconds : -1.0};
}

int bomberCountdownDigit(const sim::Lemming& lemming) {
  if (lemming.fuseMs <= 0)
    return 0;
  return (lemming.fuseMs + 999) / 1000;
}

}  // namespace webo::app
