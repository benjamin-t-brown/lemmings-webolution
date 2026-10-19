// Restored asset-side coverage (PLAN.md Phase A, debt 2).
//
// Which .SPR sprite each simulation state is drawn with. The mapping lives in
// a renderer-independent header so it can be checked without a GL context or
// an SDL window; sprite drawing itself is a later presentation task.
//
// Most states name a sprite directly. The two that do not are deliberate and
// marked as such in the header: Splatter ("cartwheel") is an interpretation,
// and Exited has no artwork. The retail sweep confirms every non-empty name
// actually exists in the shipped LEMMINGSLO.SPR, which is the part that cannot
// be inferred from the state enum alone.

#include "assets/Box.h"
#include "assets/Sprite.h"
#include "gfx/SpriteState.h"
#include "sim/World.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string_view what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

constexpr std::array kStates = {
    webo::sim::State::Faller,   webo::sim::State::Walker,   webo::sim::State::Climber,
    webo::sim::State::Hoister,  webo::sim::State::Floater,  webo::sim::State::Blocker,
    webo::sim::State::Builder,  webo::sim::State::Shrugger, webo::sim::State::Basher,
    webo::sim::State::Miner,    webo::sim::State::Digger,   webo::sim::State::Exploder,
    webo::sim::State::Splatter, webo::sim::State::Drown,    webo::sim::State::Bounce,
    webo::sim::State::Teleporting, webo::sim::State::Exited, webo::sim::State::Dead,
};

void testNames() {
  std::cout << "state -> sprite names\n";

  check(webo::gfx::spriteNameForState(webo::sim::State::Exited).empty(), "Exited has no sprite");
  check(webo::gfx::spriteNameForState(webo::sim::State::Dead).empty(), "Dead has no sprite");
  check(webo::gfx::spriteNameForState(webo::sim::State::Teleporting).empty(),
        "Teleporting has no lemming sprite");
  check(webo::gfx::spriteNameForState(webo::sim::State::Walker) == "walk", "Walker -> walk");
  check(webo::gfx::spriteNameForState(webo::sim::State::Hoister) == "flip", "Hoister -> flip");
  check(webo::gfx::spriteNameForState(webo::sim::State::Drown) == "drown", "Drown -> drown");
  check(webo::gfx::spriteNameForState(webo::sim::State::Bounce) == "fall", "Bounce -> fall");

  int named = 0;
  for (auto s : kStates)
    if (!webo::gfx::spriteNameForState(s).empty())
      ++named;
  check(named == 15, std::format("15 of the 18 states are drawn ({} are)", named));
}

void testRetail(const fs::path& gameDir) {
  std::cout << "state -> sprite names (retail)\n";

  auto lib = webo::AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!lib) {
    std::cerr << "  FAIL  could not open the installation: " << lib.error() << "\n";
    ++g_failures;
    return;
  }
  auto sheet = webo::loadSpriteSheet(*lib, "SPRITES\\LEMMINGSLO.SPR");
  if (!sheet) {
    std::cerr << "  FAIL  LEMMINGSLO.SPR: " << sheet.error() << "\n";
    ++g_failures;
    return;
  }

  int matched = 0;
  for (auto s : kStates) {
    const auto name = webo::gfx::spriteNameForState(s);
    if (name.empty())
      continue;
    const webo::Sprite* sprite = sheet->find(name);
    check(sprite != nullptr, std::format("sprite '{}' exists in LEMMINGSLO.SPR", name));
    if (sprite) {
      ++matched;
      check(!sprite->animations.empty() && !sprite->animations.front().frames.empty(),
            std::format("sprite '{}' has at least one frame", name));
    }
  }
  std::cout << std::format("    {} states mapped onto {} sprites\n", matched, sheet->sprites.size());
}

}  // namespace

int main() {
  testNames();

  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR")) {
    if (fs::exists(fs::path(dir) / "lemmings.box"))
      testRetail(dir);
    else
      std::cout << "state -> sprite names (retail): skipped, no lemmings.box in " << dir << "\n";
  } else {
    std::cout << "state -> sprite names (retail): skipped, WEBOLUTION_GAME_DIR is not set\n";
  }

  std::cout << std::format("{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
