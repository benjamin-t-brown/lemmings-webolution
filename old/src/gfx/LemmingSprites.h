// Draws lemmings with the original's own artwork.
//
// `LEMMINGSLO.SPR` carries 15 named sprites -- walk, fall, climb, build, bash,
// mine, dig, block, float, explode and so on -- and each one holds several
// *animations*. Those are not variations: they are views. A lemming on the
// inside of the tube is seen from a different angle depending on where it
// stands, so `walk` has 12 views, `block` has 8, `fall` has 7. The flat
// unwrapped view this renderer draws needs only one of them, so view 0 is
// used throughout and the rest are left for when the cylinder projection
// arrives.
//
// Frames reference several different sheet textures (`$LemSheet2` through
// `$LemSheet10`), so this loads each sheet once on demand and keeps them by
// name. Sheet pixel sizes come from SpriteFrame's recovered dimensions rather
// than from the decoded image, so a mismatch between the two shows up as a
// visibly sliced lemming rather than passing silently -- which is how the
// transposed axes in .SPR were caught in the first place.
//
// The state-to-sprite mapping is by name and mostly obvious. The two that are
// not are called out at spriteNameForState below, where they are made.

#pragma once

#include "assets/Sprite.h"
#include "sim/World.h"

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace webo {
class AssetLibrary;
}

namespace webo::gfx {

class Renderer;

// The sprite each simulation state is drawn with, or empty for states with no
// artwork. Defined here rather than in the .cpp so it can be tested against
// the retail .SPR without dragging in the renderer and a GL context.
//
// Most are named directly by the sheet. Two are not, and guessing quietly
// would put the wrong animation on screen with nothing to mark it as a guess:
//
//   Splatter  there is no "splat" sprite. "cartwheel" is the only candidate
//             that reads as a lemming losing control, but that is an
//             interpretation of a name, not a verified mapping.
//   Exited    nothing corresponds; the lemming is gone. Left empty so the
//             caller draws nothing.
//
// Hoister maps to "flip", the pull-up at the top of a climb. That one is
// confident: it is the only sprite left once every other state is accounted
// for, and no other state claims it.
constexpr std::string_view spriteNameForState(sim::State state) {
  switch (state) {
    case sim::State::Faller: return "fall";
    // MEASURED, round 13: bounce enter 0x0040ecca pushes "fall" at 0x004bf184.
    case sim::State::Bounce: return "fall";
    case sim::State::Walker: return "walk";
    case sim::State::Climber: return "climb";
    case sim::State::Hoister: return "flip";
    case sim::State::Floater: return "float";
    case sim::State::Blocker: return "block";
    case sim::State::Builder: return "build";
    // MEASURED, round 6: a spent builder's animation strip is index 10, and
    // `shrug` is the strip name the enter hook of 0x004B0C78 sets.
    case sim::State::Shrugger: return "shrug";
    case sim::State::Basher: return "bash";
    case sim::State::Miner: return "mine";
    case sim::State::Digger: return "dig";
    case sim::State::Exploder: return "explode";
    case sim::State::Splatter: return "cartwheel";  // unverified, see above
    // MEASURED, round 14: drown enter 0x0040874c sets "drown" at 0x004BF130.
    case sim::State::Drown: return "drown";
    // MEASURED, round 16: pad FX is teleport_fx.a3d; tele-out/tele-snap are
    // object states (vtables 0x004B0C1C / 0x004B0CE8), not a lemming strip.
    case sim::State::Teleporting: return {};
    case sim::State::Exited: return {};
    case sim::State::Dead: return {};
  }
  return {};
}

class LemmingSprites {
public:
  // Loads the sprite definitions and every sheet they reference, uploading
  // each sheet to the renderer once.
  std::expected<void, std::string> load(const AssetLibrary& lib, Renderer& renderer,
                                        std::string_view sprFile = "SPRITES\\LEMMINGSLO.SPR");

  bool ready() const { return ready_; }

  // Draws one lemming. `tick` advances the animation; `height` is the
  // simulation's lemming height in world units, which the frame is scaled to
  // so the artwork lines up with the collision box whatever its pixel size.
  // Returns false when the state has no artwork, so the caller can fall back.
  bool draw(Renderer& renderer, sim::State state, float x, float y, float height, int tick,
            bool faceLeft) const;

  const SpriteSheet& definitions() const { return sheet_; }

  // Sheets that were referenced but could not be loaded, for diagnostics.
  const std::vector<std::string>& missingSheets() const { return missing_; }

private:
  struct Sheet {
    unsigned texture = 0;
    int width = 0;
    int height = 0;
  };

  const Sheet* sheetFor(const std::string& name) const;

  SpriteSheet sheet_;
  std::unordered_map<std::string, Sheet> sheets_;
  std::vector<std::string> missing_;
  bool ready_ = false;
};

}  // namespace webo::gfx
