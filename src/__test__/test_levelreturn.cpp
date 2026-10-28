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

  // Separate end-to-end loss route: start from a fresh application/session so
  // every progress bit and map state is genuinely new-game state.
  struct LossTrace {
    bool ok = true;
    std::string why;
    int stage = 0;
    bool loss7 = false, loss9 = false, map11 = false, menu12 = false;
    bool pending7 = false, pending9 = false, retry = false;
    bool settleQueued = false;
    int levelCalls = 0;
    std::array<std::int32_t, 102> freshMapStates{};
    void fail(const char* s) { if (ok) why = s; ok = false; }
  } loss;
  const auto nativeFreshStates = [] {
    std::array<std::int32_t, 102> states;
    states.fill(1);
    states[0] = states[1] = states[2] = 3;
    return states;
  }();
  const auto exportSettled = [&](const char* name, const app::FrontendSession& s) {
    if (s.pointerX() != 320 || s.pointerY() != 240 || s.leftHeld() || s.rightHeld()) return false;
    return exportFrameIfRequested(name, s.composeFrame());
  };
  const auto lossFrontend = [&](const app::FrontendSession& s, int) {
    using shell::Screen;
    if (!loss.ok) { pushQuit(); return; }
    const auto zeroFlags = std::all_of(s.completionFlags().begin(), s.completionFlags().end(),
                                       [](auto flag) { return flag == 0; });
    const auto freshStates = s.mapStates() == loss.freshMapStates;
    switch (loss.stage) {
      case 0:
        if (s.screen() != Screen::Menu || s.constructionCounter() != 3 || !pushKey()) { loss.fail("fresh app did not start at Menu3"); pushQuit(); return; }
        loss.stage = 1; break;
      case 1:
        if (s.screen() != Screen::Confirm || s.constructionCounter() != 4 || !pushKey()) { loss.fail("ordinary New Game confirmation failed"); pushQuit(); return; }
        loss.stage = 2; break;
      case 2:
        if (s.screen() != Screen::Map || s.constructionCounter() != 5 || !zeroFlags || s.mapStates() != nativeFreshStates ||
            !pushClick(206, 204)) { loss.fail("fresh New Game did not reach Map5 with zero progress"); pushQuit(); return; }
        loss.freshMapStates = s.mapStates();
        loss.stage = 3; break;
      case 3:
        if (s.screen() != Screen::Results || !s.outcome() || s.outcome()->success ||
            s.outcome()->level != "MM_CLIMB_002" || s.outcome()->saved != 0 ||
            s.outcome()->required != 1 || s.constructionCounter() != 7 || !zeroFlags ||
            !freshStates || !loss.pending7) { loss.fail("first natural timeout was not fresh loss Results7"); pushQuit(); return; }
        if (!loss.settleQueued) {
          if (!pushMotion(320, 240) || !pushButtonUp(320, 240)) { loss.fail("cannot queue first loss screenshot settle events"); pushQuit(); }
          loss.settleQueued = true;
          break;
        }
        if (!exportSettled("nr-loss-native-loss7.png", s)) { loss.fail("first loss screenshot did not export at released center pointer"); pushQuit(); return; }
        loss.loss7 = true;
        if (!pushClick(320, 360)) { loss.fail("Retry click on Results7 failed"); pushQuit(); return; }
        loss.settleQueued = false; loss.stage = 4; break;
      case 4:
        if (s.screen() != Screen::Results || !s.outcome() || s.outcome()->success ||
            s.outcome()->level != "MM_CLIMB_002" || s.outcome()->saved != 0 || s.outcome()->required != 1 || s.constructionCounter() != 9 ||
            !zeroFlags || !freshStates || s.levelIntroEnabled() || !loss.retry || !loss.pending9) {
          loss.fail("Retry did not produce fresh stock/intro-disabled timeout Results9"); pushQuit(); return;
        }
        if (!loss.settleQueued) {
          if (!pushMotion(320, 240) || !pushButtonUp(320, 240)) { loss.fail("cannot queue retry loss screenshot settle events"); pushQuit(); }
          loss.settleQueued = true;
          break;
        }
        if (!exportSettled("nr-loss-native-loss9.png", s)) { loss.fail("retry loss screenshot did not export at released center pointer"); pushQuit(); return; }
        loss.loss9 = true;
        if (!pushClick(320, 420)) { loss.fail("Quit click on Results9 failed"); pushQuit(); return; }
        loss.settleQueued = false; loss.stage = 5; break;
      case 5:
        if (s.screen() != Screen::Map || s.constructionCounter() != 11 || s.selectedLevel() || s.outcome() ||
            !s.levelIntroEnabled() || !zeroFlags || !freshStates) { loss.fail("Quit did not return to fresh Map11"); pushQuit(); return; }
        if (!loss.settleQueued) {
          if (!pushMotion(320, 240) || !pushButtonUp(320, 240)) { loss.fail("cannot queue returned map screenshot settle events"); pushQuit(); }
          loss.settleQueued = true;
          break;
        }
        if (!exportSettled("nr-loss-native-returnedmap11.png", s)) { loss.fail("returned map screenshot did not export at released center pointer"); pushQuit(); return; }
        loss.map11 = true;
        if (!pushClick(320, 455)) { loss.fail("map footer Menu click failed"); pushQuit(); return; }
        loss.settleQueued = false; loss.stage = 6; break;
      case 6:
        if (s.screen() != Screen::Menu || s.constructionCounter() != 12 || !zeroFlags) {
          loss.fail("Map footer did not reach Menu12"); pushQuit(); return;
        }
        if (!loss.settleQueued) {
          if (!pushMotion(320, 240) || !pushButtonUp(320, 240)) { loss.fail("cannot queue final menu screenshot settle events"); pushQuit(); }
          loss.settleQueued = true;
          break;
        }
        if (!exportSettled("nr-loss-native-finalmenu12.png", s)) { loss.fail("final menu screenshot did not export at released center pointer"); pushQuit(); return; }
        loss.menu12 = true;
        pushQuit(); loss.stage = 7; break;
      default: pushQuit(); break;
    }
  };
  const auto lossLevel = [&](const app::LevelFrameView& f) {
    ++loss.levelCalls;
    if (!loss.ok) { pushQuit(); return; }
    const auto& w = f.world;
    if (loss.stage == 3 || loss.stage == 4) {
      if (f.levelName != "MM_CLIMB_002" || f.frontendConstructionCounter != (loss.stage == 3 ? 5 : 7)) {
        loss.fail("loss route has wrong level identity or construction counter"); pushQuit(); return;
      }
      if (w.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)] != 10) {
        loss.fail("natural timeout level did not use stock skill inventory"); pushQuit(); return;
      }
      if (loss.stage == 3 && f.levelName != "MM_CLIMB_002") {
        loss.fail("Map5 did not enter MM_CLIMB_002"); pushQuit(); return;
      }
      if (loss.stage == 3 && f.frame == 0 && !f.cameraIntroActive) {
        loss.fail("Map5 level entry skipped its ordinary camera intro"); pushQuit(); return;
      }
      if (loss.stage == 3 && f.endPhase == app::LevelEndPhase::Pending && w.timedOut()) loss.pending7 = true;
      if (loss.stage == 4) {
        if (f.frame == 0 && (w.tickCount() != 10 || w.saved() != 0 || f.endPhase != app::LevelEndPhase::Running ||
                             f.cameraIntroActive || f.entryCueSeconds != -0.5 || !f.entryCueFrame || !f.entryCueFrame->additive)) {
          loss.fail("Retry did not restart stock world with intro disabled"); pushQuit(); return;
        }
        if (w.timedOut() && f.endPhase == app::LevelEndPhase::Pending) loss.pending9 = true;
        loss.retry = true;
      }
    }
  };
  std::vector<std::string> lossArgs{"levelreturn-loss-test", "--frontend", "--frames", "220", "--game-dir", gameDir};
  std::vector<char*> lossRaw; for (auto& a : lossArgs) lossRaw.push_back(a.data());
  const int lossResult = app::runApplication(static_cast<int>(lossRaw.size()), lossRaw.data(), lossFrontend, lossLevel,
                                             app::ApplicationRunOptions{.frameSeconds = 2.0});
  if (lossResult != 0) loss.fail("loss-only runApplication returned failure");
  if (loss.stage != 7 || !loss.loss7 || !loss.loss9 || !loss.map11 || !loss.menu12)
    loss.fail("loss-only route did not export/verify Results7, Results9, Map11 and Menu12");
  if (!loss.ok) { std::cerr << "FAIL: LevelReturn loss route: " << loss.why << " (level callbacks=" << loss.levelCalls << ")\n"; return 1; }
  std::cout << "PASS: natural success/failure results return to and preserve map progress\n";
  return 0;
#endif
}
