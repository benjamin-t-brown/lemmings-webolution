#include "app/AppEntry.h"
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
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {

#if WEBOLUTION_HAVE_SDL2
using namespace webo;

bool pushKey(SDL_Scancode scancode, SDL_Keycode keycode) {
  SDL_Event event{};
  event.type = SDL_KEYDOWN;
  event.key.type = SDL_KEYDOWN;
  event.key.state = SDL_PRESSED;
  event.key.keysym.sym = keycode;
  event.key.keysym.scancode = scancode;
  return SDL_PushEvent(&event) == 1;
}

bool pushClick(int x, int y) {
  SDL_Event event{};
  event.type = SDL_MOUSEBUTTONDOWN;
  event.button.type = SDL_MOUSEBUTTONDOWN;
  event.button.button = SDL_BUTTON_LEFT;
  event.button.state = SDL_PRESSED;
  event.button.x = x;
  event.button.y = y;
  return SDL_PushEvent(&event) == 1;
}

bool pushQuit() {
  SDL_Event event{};
  event.type = SDL_QUIT;
  return SDL_PushEvent(&event) == 1;
}

webo::app::PickBox boxFor(const webo::app::ActorSpritePlacement& p) {
  return {static_cast<int>(p.left), static_cast<int>(p.top),
          static_cast<int>(p.right), static_cast<int>(p.bottom)};
}

bool imageEqualInCountRoi(const webo::Image& a, const webo::Image& b) {
  if (a.width != b.width || a.height != b.height || a.rgba.size() != b.rgba.size()) return false;
  for (int y = 20; y < 35; ++y)
    for (int x = 66; x < 79; ++x) {
      const auto offset = (static_cast<std::size_t>(y) * a.width + x) * 4;
      for (int c = 0; c < 4; ++c)
        if (a.rgba[offset + c] != b.rgba[offset + c]) return false;
    }
  return true;
}

bool findEmptyClick(const std::span<const app::LevelActorView> actors, int& outX, int& outY) {
  // Source pick geometry uses a 32x32 cursor centered on the click. Stay out
  // of the fixed panel/HUD plates and out of every projected actor box.
  for (int y = 100; y < 445; y += 5) {
    for (int x = 100; x < 635; x += 5) {
      const app::PickBox cursor{x - 16, y - 16, x + 16, y + 16};
      bool overlaps = false;
      for (const auto& actor : actors)
        if (app::pickBoxOverlaps(boxFor(actor.placement), cursor)) { overlaps = true; break; }
      if (!overlaps) { outX = x; outY = y; return true; }
    }
  }
  return false;
}
#endif

}  // namespace

int main() {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "LevelInteraction: requires SDL2\n";
  return 1;
#else
  // Check before any call into the app (which initializes SDL), never opening
  // a user's interactive desktop if this test was invoked outside CTest.
  const char* driver = std::getenv("SDL_VIDEODRIVER");
  if (!driver || std::string(driver) != "dummy") {
    std::cerr << "LevelInteraction: refusing to run without SDL_VIDEODRIVER=dummy\n";
    return 1;
  }
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir ||
      !std::filesystem::is_regular_file(std::filesystem::path(gameDir) / "lemmings.box")) {
    std::cout << "LevelInteraction: SKIPPED (retail assets unavailable)\n";
    return 0;
  }

  bool ok = true;
  int calls = 0;
  std::string failure;
  Image pausedImage;
  std::size_t targetIndex = static_cast<std::size_t>(-1);
  int targetX = 0, targetY = 0;
  int initialClimbers = -1;
  const auto fail = [&](const char* why) {
    if (ok) failure = why;
    ok = false;
  };

  const auto observer = [&](const app::LevelFrameView& frame) {
    ++calls;
    if (frame.frame != calls - 1 || calls > 5) { fail("unexpected frame sequence"); return; }
    const auto& lemmings = frame.world.lemmings();
    if (calls == 1) {
      if (lemmings.empty() || !lemmings[0].active() || frame.actors.empty()) {
        fail("prepared retail simulation has no live active visible actor");
        pushQuit(); return;
      }
      if (std::none_of(frame.actors.begin(), frame.actors.end(),
                       [](const app::LevelActorView& a) { return a.index == 0; })) {
        fail("actor index 0 is not visible"); pushQuit(); return;
      }
      initialClimbers = frame.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)];
      if (initialClimbers != 10) { fail("stock climber count is not 10"); pushQuit(); return; }
      if (!pushKey(SDL_SCANCODE_P, SDLK_p) || !pushClick(50, 25) || !pushClick(50, 65) ||
          !pushKey(SDL_SCANCODE_2, SDLK_2) || !pushClick(50, 345)) fail("could not queue paused-input events");
    } else if (calls == 2) {
      if (!frame.paused || frame.selectedSkillIndex != 0 ||
          frame.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)] != initialClimbers)
        fail("paused panel/key/command events changed selection, stock, or pause state");
      pausedImage = frame.image;
      int x = 0, y = 0;
      if (!findEmptyClick(frame.actors, x, y)) { fail("could not find empty in-screen cursor point"); pushQuit(); return; }
      if (!pushClick(x, y)) fail("could not queue empty click");
    } else if (calls == 3) {
      if (!frame.paused || frame.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)] != initialClimbers)
        fail("empty click changed pause state or stock");
      auto target = frame.actors.end();
      for (auto candidate = frame.actors.begin(); candidate != frame.actors.end(); ++candidate) {
        if (!lemmings[candidate->index].active() || lemmings[candidate->index].canClimb) continue;
        const int x = static_cast<int>((candidate->placement.left + candidate->placement.right) / 2);
        const int y = static_cast<int>((candidate->placement.top + candidate->placement.bottom) / 2);
        if (x < 100 || x >= 640 || y < 0 || y >= 480) continue;
        const app::PickBox cursor{x - 16, y - 16, x + 16, y + 16};
        if (!app::pickBoxOverlaps(boxFor(candidate->placement), cursor)) continue;
        bool earlierOverlap = false;
        for (auto prior = frame.actors.begin(); prior != candidate; ++prior)
          if (app::pickBoxOverlaps(boxFor(prior->placement), cursor)) { earlierOverlap = true; break; }
        if (!earlierOverlap) { target = candidate; break; }
      }
      if (target == frame.actors.end()) { fail("no valid active actor click target"); pushQuit(); return; }
      targetIndex = target->index;
      targetX = static_cast<int>((target->placement.left + target->placement.right) / 2);
      targetY = static_cast<int>((target->placement.top + target->placement.bottom) / 2);
      if (targetX < 100 || targetX >= 640 || targetY < 0 || targetY >= 480) { fail("target is outside viewport"); pushQuit(); return; }
      if (!pushClick(targetX, targetY)) fail("could not queue assignment click");
    } else if (calls == 4) {
      if (frame.paused || targetIndex >= lemmings.size() || !lemmings[targetIndex].canClimb ||
          frame.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)] != initialClimbers - 1)
        fail("valid actor click did not assign climber and resume");
      if (frame.selectedSkillIndex != 0) fail("assignment changed selected skill");
      if (imageEqualInCountRoi(pausedImage, frame.image)) fail("climber-count HUD ROI did not change");
      if (!pushKey(SDL_SCANCODE_P, SDLK_p) || !pushClick(targetX, targetY)) fail("could not queue duplicate rejection events");
    } else {
      if (!frame.paused || frame.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)] != initialClimbers - 1)
        fail("duplicate assignment changed pause state or stock");
      if (!pushQuit()) fail("could not queue SDL_QUIT");
    }
  };

  // --sim-ticks prepares a bounded retail simulation state. This is not a
  // natural frontend/spawn-timing run and is not a native behavior oracle.
  std::vector<std::string> args{"levelinteraction-test", "--level", "MM_CLIMB_002", "--native",
      "--sim-ticks", "20", "--frames", "12", "--game-dir", gameDir};
  std::vector<char*> raw;
  for (auto& arg : args) raw.push_back(arg.data());
  const int result = app::runApplication(static_cast<int>(raw.size()), raw.data(), {}, observer);
  if (result != 0) fail("runApplication returned failure");
  if (calls != 5) fail("did not observe exactly five frames");
  if (!ok) { std::cerr << "FAIL: LevelInteraction: " << failure << '\n'; return 1; }
  std::cout << "PASS: prepared retail simulation level interaction sequence\n";
  return 0;
#endif
}
