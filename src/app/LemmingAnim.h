// actor-anim-001 (playtest P4-P6): per-lemming animation for the level view.
//
// Every live lemming used to be drawn as the first temporal entry of the "walk" clip. This
// module gives each simulated lemming its own clip, chosen from its sim::State, and its own
// clock, advanced with level time through the recovered controller clock (ActorAnim.h,
// 0x00468833: 0.06 s per temporal entry, scaled by the clip's rate).
//
// Evidence, kept apart:
//   * Clip names and order in SPRITES\LEMMINGSHI.SPR: READ (docs/scratch/phase-c1p-spr-sheet-read.md):
//     bash, block, build, climb, dig, drown, explode, fall, flip, mine, shrug, float, umbopen,
//     walk, cartwheel. The live controller indices match: walk 13 (test_actoranim), bash 0,
//     build 2, dig 4, flip 8 (hoist), mine 9, shrug 10, umbopen 12 (docs/lessons/data-and-simulation.md).
//   * Clip lengths and rates are read from the sheet at run time, never hard-coded.
//   * State -> clip is evidenced for walker, climber, hoister (flip), builder, shrugger, basher,
//     miner, digger, drowning (the state's enter sets "drown", World.h Drown) and the teleport pad
//     ("cartwheel", World.h Teleporting). Retained actor_falling/climbing/topping/drowning/
//     bouncing/flapper evidence also establishes those names and the umbopen -> float sequence.
//     Blocker and splatter are PROVISIONAL mappings; the frame-end state adapter and umbrella
//     handover are provisional, rather than original within-frame model callbacks.
//   * Looping versus holding the last entry per clip is PROVISIONAL (lemmingClipLoops).
// Presentation only: nothing here feeds back into the simulation.
#pragma once

#include "app/ActorAnim.h"
#include "assets/Bitmap.h"
#include "assets/Sprite.h"
#include "sim/World.h"

#include <cstddef>
#include <span>
#include <vector>

namespace webo::app {

struct LevelActorView;

// The clip a lemming in this state draws. Returns a literal with static storage.
const char* lemmingClipName(const sim::Lemming& lemming);

// Whether a clip repeats (true) or holds its last temporal entry after one pass (false).
bool lemmingClipLoops(const char* clip);

struct LemmingAnimPose {
  const char* clip = "walk";
  int temporalIndex = 0;
};

class LemmingAnimTracker {
 public:
  // Advance every live lemming's clock by `elapsedSeconds` of level time (0 while paused: the
  // clock then does not move, but a state change still restarts the new clip at entry 0).
  // A lemming whose state changed since the last call starts its new clip from temporal 0.
  // Floating plays "umbopen" once, then "float" (the open animation finishes first; World.h
  // floatOpenMs). Index i is World::lemmings()[i].
  void advance(const sim::World& world, const SpriteSheet& sprites, double elapsedSeconds);

  // The pose of lemming `index` as of the last advance(); walk entry 0 for an unseen lemming.
  LemmingAnimPose poseOf(std::size_t index) const;

 private:
  struct Entry {
    bool valid = false;
    sim::State state = sim::State::Faller;
    const char* clip = "walk";
    ActorAnimClockState clock;
  };
  std::vector<Entry> entries_;
};

// The decimal bomber countdown for a lit fuse: whole seconds remaining, rounded up, so the
// 5 s fuse reads 5, 4, 3, 2, 1. 0 for a lemming with no lit fuse (fuseMs <= 0).
int bomberCountdownDigit(const sim::Lemming& lemming);

// Draw the countdown above every live lemming with a lit fuse, using the ^Font sheet and
// glyph table the HUD uses (shell::drawHud). `actors` gives each lemming's on-screen box.
// PROVISIONAL presentation: text above the sprite, at the actor billboard depth.
// An optional physical scene depth masks each glyph pixel (smaller is nearer).
void drawBomberCountdowns(Image& frame, const Image& font, const sim::World& world,
                          std::span<const LevelActorView> actors);
void drawBomberCountdowns(Image& frame, const Image& font, const sim::World& world,
                          std::span<const LevelActorView> actors, std::span<const double> sceneDepth);

}  // namespace webo::app
