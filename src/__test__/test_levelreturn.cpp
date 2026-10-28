#include "app/AppEntry.h"
#include "app/RuntimePick.h"
#include "assets/Png.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {
#if WEBOLUTION_HAVE_SDL2
using namespace webo;

bool pushKey(SDL_Keycode key = SDLK_RETURN) {
  SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.type = SDL_KEYDOWN;
  e.key.keysym.sym = key; e.key.keysym.scancode = SDL_GetScancodeFromKey(key);
  return SDL_PushEvent(&e) == 1;
}
bool pushClick(int x, int y) {
  SDL_Event e{}; e.type = SDL_MOUSEBUTTONDOWN; e.button.type = SDL_MOUSEBUTTONDOWN;
  e.button.button = SDL_BUTTON_LEFT; e.button.x = x; e.button.y = y;
  return SDL_PushEvent(&e) == 1;
}
bool pushMotion(int x, int y) {
  SDL_Event e{}; e.type = SDL_MOUSEMOTION; e.motion.type = SDL_MOUSEMOTION;
  e.motion.x = x; e.motion.y = y;
  return SDL_PushEvent(&e) == 1;
}
bool pushButtonUp(int x, int y) {
  SDL_Event e{}; e.type = SDL_MOUSEBUTTONUP; e.button.type = SDL_MOUSEBUTTONUP;
  e.button.button = SDL_BUTTON_LEFT; e.button.x = x; e.button.y = y;
  return SDL_PushEvent(&e) == 1;
}
bool pushQuit() { SDL_Event e{}; e.type = SDL_QUIT; return SDL_PushEvent(&e) == 1; }

struct Trace {
  bool ok = true;
  std::string why;
  int frontCalls = 0, levelCalls = 0;
  int stage = 0;
  bool sawFailureResult = false;
  bool checkedUnfocusedReturn = false;
  bool winPendingSeen = false, lossPendingSeen = false;
  bool retrySeen = false, reentrySeen = false;
  bool winMotionQueued = false, winReleaseQueued = false, winFrameExported = false;
  bool mapMotionQueued = false, mapReleaseQueued = false, mapFrameExported = false;
  int pendingMenuStage = 0;
  double pendingMenuTimer = 0.0;
  Image initialMap, clearedMap;
  void fail(const char* message) { if (ok) why = message; ok = false; }
};

bool exportFrameIfRequested(const char* filename, const Image& image) {
  const char* output = std::getenv("WEBOLUTION_TEST_OUTPUT_DIR");
  if (!output || !*output) return true;
  std::error_code ec;
  const std::filesystem::path directory(output);
  std::filesystem::create_directories(directory, ec);
  if (ec) return false;
  return webo::writePng(directory / filename, image).has_value();
}
std::array<std::int32_t, 102> expectedCompletionFlags() {
  std::array<std::int32_t, 102> expected{};
  expected[0] = 1;
  return expected;
}
std::array<std::int32_t, 102> expectedCompletedMapStates() {
  std::array<std::int32_t, 102> expected{};
  expected.fill(1);
  expected[0] = 2;
  for (std::size_t i = 1; i <= 4; ++i) expected[i] = 3;
  return expected;
}
#endif
}

int main() {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "LevelReturn requires SDL2\n";
  return 1;
#else
  const char* driver = std::getenv("SDL_VIDEODRIVER");
  if (!driver || std::string(driver) != "dummy") {
    std::cerr << "LevelReturn: refusing to run without SDL_VIDEODRIVER=dummy\n";
    return 1;
  }
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir ||
      !std::filesystem::is_regular_file(std::filesystem::path(gameDir) / "lemmings.box")) {
    std::cout << "LevelReturn: SKIPPED (retail assets unavailable)\n";
    return 0;
  }

  Trace t;
  const auto frontend = [&](const app::FrontendSession& s, int) {
    ++t.frontCalls;
    using shell::Screen;
    if (!t.ok) { pushQuit(); return; }
    switch (t.stage) {
      case 0:
        if (s.screen() != Screen::Menu || !pushKey()) { t.fail("new-game menu/Return sequence failed"); pushQuit(); return; }
        t.stage = 1; break;
      case 1:
        if (s.screen() != Screen::Confirm || !pushKey()) { t.fail("confirmation/Return sequence failed"); pushQuit(); return; }
        t.stage = 2; break;
      case 2:
        if (s.screen() != Screen::Map) { t.fail("new game did not reach map"); pushQuit(); return; }
        t.initialMap = s.image();
        if (!pushClick(206, 204)) { t.fail("could not select first map node"); pushQuit(); return; }
        t.stage = 3; break;
      case 3:
        if (s.screen() != Screen::Results || !s.outcome() || !s.outcome()->success ||
            s.outcome()->level != "MM_CLIMB_002" || s.outcome()->saved != 10 ||
            s.outcome()->total != 10 || s.outcome()->required != 1 ||
            s.outcome()->secondsRemaining <= 0 || s.constructionCounter() != 7 ||
            s.completionFlags() != expectedCompletionFlags() || s.selectedLevel() ||
            !t.winPendingSeen) {
          t.fail("first natural run did not produce the expected successful result/progress"); pushQuit(); return;
        }
        if (!t.winMotionQueued) {
          t.winMotionQueued = pushMotion(320, 240);
          t.winReleaseQueued = pushButtonUp(320, 240);
          if (!t.winMotionQueued || !t.winReleaseQueued) {
            t.fail("could not queue win-frame pointer settle events"); pushQuit(); return;
          }
          break;
        }
        if (s.pointerX() != 320 || s.pointerY() != 240 || s.leftHeld() || s.rightHeld()) break;
        if (!t.winFrameExported) {
          if (!exportFrameIfRequested("levelreturn-win.png", s.composeFrame())) {
            t.fail("could not export composed win frame"); pushQuit(); return;
          }
          t.winFrameExported = true;
        }
        std::cout << "win result: saved=" << s.outcome()->saved << " needed=" << s.outcome()->required
                  << " seconds-left=" << s.outcome()->secondsRemaining << '\n';
        if (!t.checkedUnfocusedReturn) {
          t.checkedUnfocusedReturn = true;
          // Initial result focus repair is unresolved. Exercise the actual
          // adapter's refusal to invent Return-to-Continue, then explicitly click.
          if (!pushKey()) { t.fail("could not queue unfocused results Return"); pushQuit(); }
          break;
        }
        if (!pushClick(320, 360)) { t.fail("could not continue from successful results"); pushQuit(); return; }
        t.stage = 4; break;
      case 4:
        if (s.screen() != Screen::Map || s.selectedLevel() ||
            s.constructionCounter() != 8 ||
            s.completionFlags() != expectedCompletionFlags() ||
            s.mapStates() != expectedCompletedMapStates() || s.image().rgba == t.initialMap.rgba) {
          t.fail("successful result did not persist on cleared map/unlock node"); pushQuit(); return;
        }
        t.clearedMap = s.image();
        if (!t.mapMotionQueued) {
          t.mapMotionQueued = pushMotion(320, 240);
          t.mapReleaseQueued = pushButtonUp(320, 240);
          if (!t.mapMotionQueued || !t.mapReleaseQueued) {
            t.fail("could not queue completed-map pointer settle events"); pushQuit(); return;
          }
          break;
        }
        if (s.pointerX() != 320 || s.pointerY() != 240 || s.leftHeld() || s.rightHeld()) break;
        if (!t.mapFrameExported) {
          if (!exportFrameIfRequested("levelreturn-map-complete.png", s.composeFrame())) {
            t.fail("could not export composed completed-map frame"); pushQuit(); return;
          }
          t.mapFrameExported = true;
        }
        if (!pushClick(206, 204)) { t.fail("could not reselect completed node"); pushQuit(); return; }
        t.stage = 5; break;
      case 5: {
        const bool extraFlags = std::any_of(s.completionFlags().begin() + 1,
                                            s.completionFlags().end(), [](auto flag) { return flag != 0; });
        if (s.screen() != Screen::Results || !s.outcome() || s.outcome()->success ||
            s.outcome()->saved != 0 || s.outcome()->total != 10 || s.outcome()->secondsRemaining != 0 ||
            s.completionFlags()[0] == 0 || extraFlags || !t.lossPendingSeen) {
          t.fail("second natural run did not time out without changing earned progress"); pushQuit(); return;
        }
        t.sawFailureResult = true;
        std::cout << "loss result: saved=" << s.outcome()->saved
                  << " seconds-left=" << s.outcome()->secondsRemaining << '\n';
        if (!pushClick(320, 360)) { t.fail("could not retry failed level"); pushQuit(); return; }
        t.stage = 6; break;
      }
      case 6:
        if (s.screen() != Screen::Results || !s.outcome() || s.outcome()->success ||
            !t.retrySeen || s.levelIntroEnabled() || s.completionFlags()[0] != 1 || t.pendingMenuStage != 2) {
          t.fail("retry did not return fresh loss preserving progress/intro preference"); pushQuit(); return;
        }
        if (!pushClick(320, 420)) { t.fail("could not Quit failed results"); pushQuit(); return; }
        t.stage = 7; break;
      case 7:
        if (s.screen() != Screen::Map || s.selectedLevel() || s.completionFlags()[0] == 0 ||
            s.outcome() || !s.levelIntroEnabled() ||
            s.mapStates()[0] != 2 || s.image().rgba != t.clearedMap.rgba) {
          t.fail("failed result did not retain prior map progress"); pushQuit(); return;
        }
        if (!pushClick(206, 204)) { t.fail("could not select after Quit"); pushQuit(); return; }
        t.stage = 8; break;
      default: pushQuit(); break;
    }
  };

  const auto level = [&](const app::LevelFrameView& f) {
    ++t.levelCalls;
    if (!t.ok) { pushQuit(); return; }
    if ((t.stage == 3 || t.stage == 5) && f.frame == 0 && !f.cameraIntroActive) {
      t.fail("map-selected first level skipped camera intro"); pushQuit(); return;
    }
    if (t.stage == 3) {
      if (f.endPhase == app::LevelEndPhase::Pending && f.world.finished() && f.world.saved() == 10)
        t.winPendingSeen = true;
      const auto& lem = f.world.lemmings();
      for (const auto& actor : f.actors) {
        if (actor.index >= lem.size() || !lem[actor.index].active() || lem[actor.index].canClimb ||
            !app::firstLevelClimberSupport(lem[actor.index], sim::Skill::Climber, f.world.terrain()).value_or(false)) continue;
        const int x = static_cast<int>((actor.placement.left + actor.placement.right) / 2);
        const int y = static_cast<int>((actor.placement.top + actor.placement.bottom) / 2);
        if (x >= 100 && x < 640 && y >= 0 && y < 480) { pushClick(x, y); break; }
      }
    } else if (t.stage == 5 || t.stage == 6) {
      const auto& world = f.world;
      if (f.endPhase == app::LevelEndPhase::Pending && world.timedOut()) t.lossPendingSeen = true;
      if (world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)] != 10) {
        t.fail("reselected level did not reset to stock climber inventory"); pushQuit(); return;
      }
      if (t.stage == 6) {
        t.retrySeen = true;
        if (f.cameraIntroActive || (f.frame == 0 &&
            (world.tickCount() != 10 || world.saved() != 0 || f.endPhase != app::LevelEndPhase::Running ||
             f.entryCueSeconds != -0.5 || !f.entryCueFrame || !f.entryCueFrame->additive))) {
          t.fail("retry replayed camera intro or reused old simulation/end state"); pushQuit(); return;
        }
        if (t.pendingMenuStage == 0 && f.endPhase == app::LevelEndPhase::Pending) {
          t.pendingMenuTimer = f.endTimerSeconds;
          if (!pushKey(SDLK_ESCAPE)) t.fail("could not open pending-level menu");
          t.pendingMenuStage = 1;
        } else if (t.pendingMenuStage == 1) {
          if (!f.levelMenuOpen || f.endTimerSeconds != t.pendingMenuTimer)
            t.fail("in-level menu failed to freeze pending completion timer");
          if (!pushKey(SDLK_ESCAPE)) t.fail("could not resume pending level");
          t.pendingMenuStage = 2;
        }
      }
      // Intentionally assign nobody; the application's bounded level clock lets
      // all ten natural lemmings time out and drives its normal failure path.
    } else if (t.stage == 8) {
      t.reentrySeen = true;
      if (f.frame != 0 || !f.cameraIntroActive) t.fail("map selection after Quit failed to restore intro");
      pushQuit(); t.stage = 9;
    }
  };

  std::vector<std::string> args{"levelreturn-test", "--frontend", "--frames", "220",
      "--game-dir", gameDir};
  std::vector<char*> raw; for (auto& a : args) raw.push_back(a.data());
  const int result = app::runApplication(static_cast<int>(raw.size()), raw.data(), frontend, level,
                                         app::ApplicationRunOptions{.frameSeconds = 2.0});
  if (result != 0) t.fail("runApplication returned failure");
  if (!t.sawFailureResult || !t.retrySeen || !t.reentrySeen || t.stage != 9 ||
      !t.winFrameExported || !t.mapFrameExported)
    t.fail("did not complete success, retry, Quit and map reentry cycles");
  if (!t.ok) { std::cerr << "FAIL: LevelReturn: " << t.why << " (frontend callbacks=" << t.frontCalls
                         << ", level callbacks=" << t.levelCalls << ")\n"; return 1; }
  std::cout << "PASS: natural success/failure results return to and preserve map progress\n";
  return 0;
#endif
}
