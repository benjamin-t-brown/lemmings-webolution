// Maps simulation states to retail .SPR clips. Splatter provisionally uses "cartwheel"
// because there is no "splat" clip. The inferred Hoister clip is "flip" for the pull-up above a climb.
// Exited has no clip.

#pragma once

#include "sim/World.h"

#include <string_view>

namespace webo::gfx {

constexpr std::string_view spriteNameForState(sim::State state) {
  switch (state) {
    case sim::State::Faller: return "fall";
    // Bounce entry 0x0040ecca pushes "fall" at 0x004bf184.
    case sim::State::Bounce: return "fall";
    case sim::State::Walker: return "walk";
    case sim::State::Climber: return "climb";
    case sim::State::Hoister: return "flip";
    case sim::State::Floater: return "float";
    case sim::State::Blocker: return "block";
    case sim::State::Builder: return "build";
    // The entry hook of 0x004B0C78 sets "shrug", strip 10, for a spent builder.
    case sim::State::Shrugger: return "shrug";
    case sim::State::Basher: return "bash";
    case sim::State::Miner: return "mine";
    case sim::State::Digger: return "dig";
    case sim::State::Exploder: return "explode";
    case sim::State::Splatter: return "cartwheel";  // provisional clip
    // Drown entry 0x0040874c sets "drown" at 0x004BF130.
    case sim::State::Drown: return "drown";
    case sim::State::AntiGrav: return "cartwheel";
    case sim::State::Teleporting: return {};
    case sim::State::Exited: return {};
    case sim::State::Dead: return {};
  }
  return {};
}

}  // namespace webo::gfx
