// Select a SPRITES\LEMMINGSHI.SPR clip per state and advance each actor on
// level time. At 0x00468833 each entry consumes 0.06 s of rate-scaled
// time. Lengths and rates come from the sheet.
// Strip order: bash, block, build, climb, dig, drown, explode, fall, flip,
// mine, shrug, float, umbopen, walk, cartwheel.
// Provisional clone policies select Blocker/Splatter clips, looping, and
// frame-end transitions. Native actor_falling defines the umbopen-to-float
// transition. Animation state stays local to presentation.
#pragma once

#include "app/ActorAnim.h"
#include "assets/Bitmap.h"
#include "assets/Sprite.h"
#include "shell/GlyphDraw.h"
#include "sim/World.h"

#include <cstddef>
#include <array>
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
  double drownSeconds = -1.0;
};

class LemmingAnimTracker {
 public:
  // Advance live actors and saved exit tails in level seconds. At zero dt,
  // the clock holds but a new state starts at entry 0. Floating plays
  // "umbopen" before "float". Indices match World::lemmings().
  void advance(const sim::World& world, const SpriteSheet& sprites, double elapsedSeconds);
  void advance(const sim::World& world, const std::array<const SpriteSheet*, 3>& sprites,
               double elapsedSeconds);

  // The pose of lemming `index` as of the last advance(); walk entry 0 for an unseen lemming.
  LemmingAnimPose poseOf(std::size_t index) const;

 private:
  struct Entry {
    bool valid = false;
    sim::State state = sim::State::Faller;
    const char* clip = "walk";
    ActorAnimClockState clock;
    double seconds = 0.0;
  };
  std::vector<Entry> entries_;
};

// Lit fuses show whole seconds rounded up. An unlit fuse returns zero.
int bomberCountdownDigit(const sim::Lemming& lemming);
std::vector<shell::HudGlyph> prepareBomberCountdowns(
    const sim::World&, std::span<const LevelActorView>, int viewportWidth);

// Provisional countdown draws ^Font glyphs above the sprite. Optional
// scene depth masks glyph pixels against the actor billboard plane.
void drawBomberCountdowns(Image& frame, const Image& font, const sim::World& world,
                          std::span<const LevelActorView> actors);
void drawBomberCountdowns(Image& frame, const Image& font, const sim::World& world,
                          std::span<const LevelActorView> actors, std::span<const double> sceneDepth);

}  // namespace webo::app
