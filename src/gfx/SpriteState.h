// Which .SPR sprite a simulation state is drawn with.
//
// Renderer-independent so it can be tested against the retail `.SPR` names
// without dragging in a GL context or an SDL window. Restored as production
// code in A4 from old/src/gfx/LemmingSprites.h; sprite *rasterisation* is out
// of scope for this round and lives in a future presentation layer.
//
// Most states are named directly by the sheet. The two that are not:
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

#pragma once

#include "sim/World.h"

#include <string_view>

namespace webo::gfx {

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
    case sim::State::Teleporting: return {};
    case sim::State::Exited: return {};
    case sim::State::Dead: return {};
  }
  return {};
}

}  // namespace webo::gfx
