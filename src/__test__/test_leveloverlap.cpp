#include "app/AppEntry.h"
#include "app/ActorPose.h"
#include "app/Pick.h"
#include "sim/World.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {
#if WEBOLUTION_HAVE_SDL2
using namespace webo;

bool pushKey(SDL_Scancode scancode, SDL_Keycode keycode) {
  SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.type = SDL_KEYDOWN;
  e.key.state = SDL_PRESSED; e.key.keysym.sym = keycode; e.key.keysym.scancode = scancode;
  return SDL_PushEvent(&e) == 1;
}
bool pushClick(int x, int y) {
  SDL_Event e{}; e.type = SDL_MOUSEBUTTONDOWN; e.button.type = SDL_MOUSEBUTTONDOWN;
  e.button.button = SDL_BUTTON_LEFT; e.button.state = SDL_PRESSED;
  e.button.x = x; e.button.y = y;
  return SDL_PushEvent(&e) == 1;
}
bool pushQuit() { SDL_Event e{}; e.type = SDL_QUIT; return SDL_PushEvent(&e) == 1; }
app::PickBox boxFor(const app::ActorSpritePlacement& p) {
  return {static_cast<int>(p.left), static_cast<int>(p.top),
          static_cast<int>(p.right), static_cast<int>(p.bottom)};
}

// Independent test oracle: among overlapping eligible walkers, rank by the
// raw actor sample's squared distance to the measured D2d reference point.
// Priority is equal (Walker=0); vector order is only the final tie breaker.
double distanceToReference(const sim::Lemming& l, int terrainHeight) {
  const auto p = terrainSampleFromLemming(l, terrainHeight);
  const double dx = p.x - 32.0, dy = p.y - 48.0, dz = p.z - 0.5;
  return dx * dx + dy * dy + dz * dz;
}

bool eligibleWalker(const sim::Lemming& l) {
  return l.active() && l.state == sim::State::Walker && !l.canClimb;
}

bool findUsefulCursor(const app::LevelFrameView& frame, int& outX, int& outY,
                      std::size_t& firstWinner, std::size_t& expected) {
  const auto& lemmings = frame.world.lemmings();
  for (int y = 40; y < 465; y += 2) for (int x = 100; x < 640; x += 2) {
    const app::PickBox cursor{x - 16, y - 16, x + 16, y + 16};
    std::vector<const app::LevelActorView*> hits;
    bool ineligibleHit = false;
    for (const auto& actor : frame.actors) {
      if (!app::pickBoxOverlaps(boxFor(actor.placement), cursor)) continue;
      if (actor.index >= lemmings.size() || !eligibleWalker(lemmings[actor.index])) {
        ineligibleHit = true; break;
      }
      hits.push_back(&actor);
    }
    if (ineligibleHit || hits.size() < 2) continue;
    const auto closest = *std::min_element(hits.begin(), hits.end(), [&](auto* a, auto* b) {
      const double da = distanceToReference(lemmings[a->index], frame.world.terrain().height());
      const double db = distanceToReference(lemmings[b->index], frame.world.terrain().height());
      return da < db;
    });
    if (closest->index == hits.front()->index) continue;
    outX = x; outY = y; firstWinner = hits.front()->index; expected = closest->index;
    return true;
  }
  return false;
}
#endif
}  // namespace

int main() {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "LevelOverlap: requires SDL2\n"; return 1;
#else
  const char* driver = std::getenv("SDL_VIDEODRIVER");
  if (!driver || std::string(driver) != "dummy") {
    std::cerr << "LevelOverlap: refusing to run without SDL_VIDEODRIVER=dummy\n"; return 1;
  }
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir ||
      !std::filesystem::is_regular_file(std::filesystem::path(gameDir) / "lemmings.box")) {
    std::cout << "LevelOverlap: SKIPPED (retail assets unavailable)\n"; return 0;
  }

  bool ok = true; std::string failure; int calls = 0;
  int clickX = 0, clickY = 0, initialClimbers = -1;
  std::size_t firstWinner = 0, firstExpected = 0, secondExpected = 0;
  const auto fail = [&](const char* s) { if (ok) failure = s; ok = false; };
  const auto observer = [&](const app::LevelFrameView& frame) {
    ++calls;
    if (frame.frame != calls - 1 || calls > 6) { fail("unexpected frame sequence"); return; }
    const auto& lemmings = frame.world.lemmings();
    const auto assignedCount = std::count_if(lemmings.begin(), lemmings.end(),
        [](const sim::Lemming& l) { return l.canClimb; });
    if (calls == 1) {
      if (assignedCount != 0) { fail("prepared actors already have Climber"); pushQuit(); return; }
      initialClimbers = frame.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)];
      if (initialClimbers <= 1) { fail("insufficient climber stock"); pushQuit(); return; }
      if (!pushKey(SDL_SCANCODE_P, SDLK_p)) fail("could not queue pause");
    } else if (calls == 2) {
      if (!frame.paused) { fail("simulation did not pause"); pushQuit(); return; }
      if (!findUsefulCursor(frame, clickX, clickY, firstWinner, firstExpected)) {
        fail("no overlapping eligible walker cursor with a non-first closest actor"); pushQuit(); return;
      }
      std::cout << "overlap cursor=" << clickX << ',' << clickY
                << " first-listed=" << firstWinner << " expected=" << firstExpected << '\n';
      SDL_Event motion{}; motion.type = SDL_MOUSEMOTION;
      motion.motion.x = clickX; motion.motion.y = clickY;
      if (SDL_PushEvent(&motion) != 1) fail("could not queue overlap hover");
    } else if (calls == 3) {
      if (!frame.paused || frame.hoveredActor != firstExpected ||
          frame.cursorState != app::CursorState::Lemming)
        fail("overlap hover did not identify the closest non-first walker");
      if (!pushClick(clickX, clickY)) fail("could not queue overlap click");
    } else if (calls == 4) {
      if (frame.paused || assignedCount != 1 || firstExpected >= lemmings.size() || !lemmings[firstExpected].canClimb ||
          lemmings[firstWinner].canClimb ||
          frame.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)] != initialClimbers - 1)
        fail("overlap click did not assign the closest non-first walker and resume");
      if (firstExpected >= lemmings.size() || !pushKey(SDL_SCANCODE_P, SDLK_p))
        fail("could not prepare duplicate-target rejection check");
    } else if (calls == 5) {
      if (!frame.paused) { fail("could not pause before same-cursor retry"); pushQuit(); return; }
      const app::PickBox cursor{clickX - 16, clickY - 16, clickX + 16, clickY + 16};
      double best = std::numeric_limits<double>::infinity();
      for (const auto& actor : frame.actors) {
        if (!app::pickBoxOverlaps(boxFor(actor.placement), cursor) || actor.index >= lemmings.size() ||
            !eligibleWalker(lemmings[actor.index])) continue;
        const double d = distanceToReference(lemmings[actor.index], frame.world.terrain().height());
        if (d < best) { best = d; secondExpected = actor.index; }
      }
      if (!std::isfinite(best) || secondExpected == firstExpected || !pushClick(clickX, clickY))
        fail("no distinct eligible overlap remained at the same cursor");
      std::cout << "same-cursor next expected=" << secondExpected << '\n';
    } else if (calls == 6) {
      if (frame.paused || assignedCount != 2 || secondExpected >= lemmings.size() ||
          !lemmings[firstExpected].canClimb || !lemmings[secondExpected].canClimb ||
          frame.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)] != initialClimbers - 2)
        fail("same-cursor retry did not select the next eligible overlapping walker");
      if (!pushQuit()) fail("could not queue quit");
    } else if (!pushQuit()) fail("could not queue quit");
  };
  // Prepared app simulation only; this is not a native executable behavior oracle.
  std::vector<std::string> args{"leveloverlap-test", "--native", "--level", "MM_CLIMB_002",
      "--sim-ticks", "100", "--frames", "12", "--game-dir", gameDir};
  std::vector<char*> raw; for (auto& arg : args) raw.push_back(arg.data());
  if (app::runApplication(static_cast<int>(raw.size()), raw.data(), {}, observer) != 0)
    fail("runApplication returned failure");
  if (calls != 6) fail("did not observe exactly six frames");
  if (!ok) { std::cerr << "FAIL: LevelOverlap: " << failure << '\n'; return 1; }
  std::cout << "PASS: prepared simulation overlapping walker selection and retry\n";
  return 0;
#endif
}
