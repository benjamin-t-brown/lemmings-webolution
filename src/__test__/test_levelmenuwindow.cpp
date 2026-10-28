#include "app/AppEntry.h"
#include "app/LevelMenu.h"
#include "app/RuntimeCursor.h"
#include "assets/Box.h"
#include "sim/World.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace webo;
namespace {
bool key(SDL_Scancode code) {
  SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.keysym.scancode = code;
  e.key.keysym.sym = SDL_GetKeyFromScancode(code);
  return SDL_PushEvent(&e) == 1;
}
bool click(int x, int y, Uint8 button = SDL_BUTTON_LEFT) {
  SDL_Event e{}; e.type = SDL_MOUSEBUTTONDOWN; e.button.button = button;
  e.button.x = x; e.button.y = y; return SDL_PushEvent(&e) == 1;
}
bool quit() { SDL_Event e{}; e.type = SDL_QUIT; return SDL_PushEvent(&e) == 1; }
}

int main() {
  const char* driver = std::getenv("SDL_VIDEODRIVER");
  if (!driver || std::string(driver) != "dummy") return 1;
  const char* game = std::getenv("WEBOLUTION_GAME_DIR");
  if (!game || !*game) { std::cout << "LevelMenuWindow: SKIPPED (retail assets unavailable)\n"; return 0; }
  auto library = AssetLibrary::open(std::filesystem::path(game) / "lemmings.box",
                                   std::filesystem::path(game) / "patch.box");
  if (!library) return 1;
  auto map = app::MapScene::build(*library, {});
  if (!map) return 1;
  auto menuImage = app::LevelMenu::load(*library);
  auto cursor = app::RuntimeCursor::load(*library);
  if (!menuImage || !cursor) return 1;
  int otherX = -1, otherY = -1;
  for (int y = 0; y < 480 && otherX < 0; ++y) for (int x = 0; x < 640; ++x)
    if (map->pickNode(x, y) == 1) { otherX = x; otherY = y; break; }
  const auto otherName = map->levelForNode(1);
  if (otherX < 0 || !otherName || *otherName == "MM_CLIMB_002") return 1;

  bool ok = true;
  int phase = 0, callbacks = 0;
  std::uint64_t frozenTick = 0;
  double frozenCue = 0;
  Image retryFrame;
  const auto front = [&](const app::FrontendSession& s, int) {
    if (!ok) { quit(); return; }
    if (s.screen() == shell::Screen::Menu || s.screen() == shell::Screen::Confirm) {
      ok &= phase == 0 && key(SDL_SCANCODE_RETURN);
    } else if (s.screen() == shell::Screen::Map) {
      ok &= !s.selectedLevel() && !s.outcome() && s.completionFlags() == std::array<std::int32_t, 102>{};
      if (phase == 0) ok &= click(206, 204);
      else if (phase == 2) { phase = 3; ok &= click(otherX, otherY); }
      else if (phase == 5) { phase = 6; ok &= quit(); }
      else { ok = false; quit(); }
    } else { ok = false; quit(); } // No fabricated Results on abandonment.
  };
  const auto level = [&](const app::LevelFrameView& f) {
    ++callbacks;
    if (f.levelMenuOpen) {
      const auto pixel = [&](int x, int y, std::array<int, 3> rgb) {
        const auto i = (y * f.image.width + x) * 4;
        return f.image.rgba[i] == rgb[0] && f.image.rgba[i + 1] == rgb[1] && f.image.rgba[i + 2] == rgb[2];
      };
      ok &= pixel(262, 101, {64, 64, 64}) && pixel(263, 102, {224, 128, 0}) &&
            pixel(253, 131, {224, 10, 0});
    }
    if (phase == 0) {
      const int n = f.frame;
      ok &= f.levelName == "MM_CLIMB_002";
      const bool menu = n == 1 || n == 2 || n == 5 || n == 7 || n == 10;
      const bool paused = menu || n == 4 || n == 8;
      ok &= f.levelMenuOpen == menu && f.paused == paused;
      if (n == 0) {
        frozenTick = f.world.tickCount(); frozenCue = f.entryCueSeconds;
        ok &= key(SDL_SCANCODE_ESCAPE);
      } else if (n == 1) {
        ok &= f.world.tickCount() == frozenTick && f.entryCueSeconds == frozenCue;
        // Menu misses, Options and gameplay bindings must not leak through.
        ok &= click(10, 130) && click(320, 200) && click(320, 240, SDL_BUTTON_RIGHT);
        ok &= key(SDL_SCANCODE_2) && key(SDL_SCANCODE_SPACE) && key(SDL_SCANCODE_P);
      } else if (n == 2) {
        ok &= f.world.tickCount() == frozenTick && f.entryCueSeconds == frozenCue &&
              f.selectedSkillIndex == 0 && !f.hoveredActor && f.cursorState == app::CursorState::NoLemming;
        ok &= f.world.setup().skills[0] == 10 && f.endTimerSeconds == 7.0;
        ok &= key(SDL_SCANCODE_RETURN); // Initial focus Resume survived unsupported clicks.
        ok &= key(SDL_SCANCODE_P); // Same-batch menu input must not leak after dismissal.
      } else if (n == 3) {
        ok &= f.world.tickCount() > frozenTick && f.entryCueSeconds < frozenCue && f.selectedSkillIndex == 0;
        frozenTick = f.world.tickCount(); ok &= key(SDL_SCANCODE_P);
      } else if (n == 4) {
        ok &= f.world.tickCount() == frozenTick; ok &= key(SDL_SCANCODE_O);
      } else if (n == 5) {
        ok &= f.world.tickCount() == frozenTick; ok &= key(SDL_SCANCODE_RETURN);
      } else if (n == 6) {
        ok &= f.world.tickCount() > frozenTick;
        frozenTick = f.world.tickCount(); ok &= key(SDL_SCANCODE_ESCAPE);
      } else if (n == 7) {
        ok &= key(SDL_SCANCODE_O) && key(SDL_SCANCODE_P); // closes to paused, unlike Resume
      }
      else if (n == 8) {
        ok &= f.world.tickCount() == frozenTick && f.selectedSkillIndex == 0;
        ok &= key(SDL_SCANCODE_ESCAPE);
      } else if (n == 9) ok &= key(SDL_SCANCODE_ESCAPE);
      else if (n == 10) {
        ok &= f.entryCueFrame.has_value(); // drawable cue must survive menu ownership
        phase = 1; ok &= click(320, 140);
      } // Retry
      else ok = false;
    } else if (phase == 1 || phase == 3 || phase == 4) {
      const bool retry = phase != 3;
      ok &= f.levelName == (phase == 1 ? "MM_CLIMB_002" : *otherName);
      if (f.frame == 0) {
        ok &= !f.levelMenuOpen && !f.paused && f.world.tickCount() == 2 && f.world.saved() == 0 &&
              f.endPhase == app::LevelEndPhase::Running && f.entryCueSeconds == (retry ? 1.0 : 3.0);
        if (retry) ok &= !f.cameraIntroActive;
        if (phase == 1) retryFrame = f.image;
        ok &= key(SDL_SCANCODE_ESCAPE);
      } else if (f.frame == 1) {
        ok &= f.levelMenuOpen && f.paused && f.world.tickCount() == 2;
        if (phase == 1) {
          // Retry has no camera intro. Opening menu must leave the exact scene,
          // cue and HUD beneath it, without a pause caption or full backdrop.
          menuImage->open(); menuImage->draw(retryFrame);
          cursor->draw(retryFrame, app::CursorState::NoLemming, f.cursorX, f.cursorY);
          ok &= retryFrame.rgba == f.image.rgba;
        }
        if (phase == 3) { phase = 4; ok &= click(320, 140); } // non-first-level Retry
        else { phase = phase == 1 ? 2 : 5; ok &= click(320, 170); } // Quit to map
      } else ok = false;
    } else ok = false;
    if (!ok) { std::cerr << "menu mismatch phase=" << phase << " frame=" << f.frame << '\n'; quit(); }
  };
  std::vector<std::string> args{"level-menu-test", "--frontend", "--frames", "20", "--game-dir", game};
  std::vector<char*> argv; for (auto& a : args) argv.push_back(a.data());
  const int result = app::runApplication(static_cast<int>(argv.size()), argv.data(), front, level,
                                        {.frameSeconds = 0.5});
  if (result || !ok || phase != 6 || callbacks != 17) return 1;
  std::cout << "PASS: menu ownership, Resume/O-close, fresh retries and no-result map abandonment\n";
  return 0;
}
