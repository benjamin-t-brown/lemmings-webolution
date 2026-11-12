// fastforward-impl-001 public-application demonstration. One driver source is built twice:
// against the authentic task-entry main.cpp/RuntimeHud/RuntimeSkillPanel frozen under the ignored
// build/fastforward-impl-001/frozen/ (FASTFORWARD_FROZEN=1) and against the current sources.
// It runs the public runApplication on MM_CLIMB_002 (`--level MM_CLIMB_002 --sim`) under the SDL
// dummy drivers with ApplicationRunOptions::frameSeconds = 0.4 s, and reads World::tickCount()
// from the existing level-frame observer.
//
// SOURCE-TEST SETUP, NOT NATURAL PLAY: every mouse/key event below is injected with SDL_PushEvent
// from the observer; each run ends with an observer-queued SDL_QUIT. --frames and the CTest
// timeout are failure guards only.
//
// Timing model (hand computed from SimClock: 0.2 s steps, 10 per advance): one pass of 0.4 s is
// exactly 2 ticks, so an unlatched frame is +2 ticks and a latched frame is 7 passes x 2 = +14.
// An event pushed by the observer at frame f is processed at the start of frame f+1, before that
// frame's advance. 7 is a pass budget (dword 0x4B15F8), not a measured speedup.
#include "app/AppEntry.h"
#include "app/RuntimeSkillPanel.h"
#include "sim/World.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#if WEBOLUTION_HAVE_SDL2
namespace {
using namespace webo;

#ifdef FASTFORWARD_FROZEN
constexpr bool kFrozen = true;
constexpr const char* kSide = "before";
#else
constexpr bool kFrozen = false;
constexpr const char* kSide = "after";
#endif

constexpr int kPlate8X = 50, kPlate8Y = 348;  // inside plate 8 (y 332..363)
constexpr int kPlate9X = 50, kPlate9Y = 388;  // inside plate 9 (y 372..403)

void pushKey(SDL_Scancode scancode, SDL_Keycode keycode) {
  SDL_Event event{};
  event.type = SDL_KEYDOWN;
  event.key.type = SDL_KEYDOWN;
  event.key.state = SDL_PRESSED;
  event.key.keysym.sym = keycode;
  event.key.keysym.scancode = scancode;
  SDL_PushEvent(&event);
}

void pushButton(Uint8 button, int x, int y) {
  SDL_Event event{};
  event.type = SDL_MOUSEBUTTONDOWN;
  event.button.type = SDL_MOUSEBUTTONDOWN;
  event.button.button = button;
  event.button.state = SDL_PRESSED;
  event.button.x = x;
  event.button.y = y;
  SDL_PushEvent(&event);
}

void pushMotion(Uint32 buttonState, int x, int y) {
  SDL_Event event{};
  event.type = SDL_MOUSEMOTION;
  event.motion.type = SDL_MOUSEMOTION;
  event.motion.state = buttonState;
  event.motion.x = x;
  event.motion.y = y;
  SDL_PushEvent(&event);
}

void pushQuit() {
  SDL_Event event{};
  event.type = SDL_QUIT;
  SDL_PushEvent(&event);
}

struct Scenario {
  const char* name;
  int frames;
  std::function<void(int)> act;          // runs in the observer after frame f
  std::vector<int> latchedTicks;         // hand-computed cumulative ticks, current build
  std::vector<int> unlatchedTicks;       // hand-computed cumulative ticks, frozen build
  std::vector<bool> paused;              // expected frame.paused, both builds (empty = skip)
};

std::string join(const std::vector<int>& v) {
  std::string s;
  for (std::size_t i = 0; i < v.size(); ++i) s += (i ? "," : "") + std::to_string(v[i]);
  return s;
}

bool runScenario(const Scenario& sc, const char* gameDir, std::ostream& log) {
  std::vector<int> ticks;
  std::vector<bool> paused;
  std::array<int, 9> stock0{};
  bool stockStable = true, selectionStable = true;
  const auto observer = [&](const app::LevelFrameView& frame) {
    ticks.push_back(static_cast<int>(frame.world.tickCount()));
    paused.push_back(frame.paused);
    const auto& skills = frame.world.setup().skills;
    if (frame.frame == 0) for (std::size_t i = 0; i < 8; ++i) stock0[i] = skills[i];
    for (std::size_t i = 0; i < 8; ++i) if (skills[i] != stock0[i]) stockStable = false;
    if (frame.selectedSkillIndex != 0) selectionStable = false;
    sc.act(frame.frame);
    if (frame.frame == sc.frames - 1) pushQuit();
  };
  std::vector<std::string> args{"fastforward-app-test", "--level", "MM_CLIMB_002", "--sim",
      "--frames", std::to_string(sc.frames + 10), "--game-dir", gameDir};
  std::vector<char*> raw;
  for (auto& a : args) raw.push_back(a.data());
  app::ApplicationRunOptions options;
  options.frameSeconds = 0.4;
  const auto start = std::chrono::steady_clock::now();
  const int status = app::runApplication(static_cast<int>(raw.size()), raw.data(), {}, observer, options);
  const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
  const auto& want = kFrozen ? sc.unlatchedTicks : sc.latchedTicks;
  bool ok = status == 0 && ticks == want && stockStable && selectionStable;
  if (!sc.paused.empty()) ok = ok && paused == sc.paused;
  log << (ok ? "ok   " : "FAIL ") << kSide << ' ' << sc.name << ": ticks=[" << join(ticks) << "] want=["
      << join(want) << "] stock-unchanged=" << stockStable << " selection-unchanged=" << selectionStable
      << " status=" << status << " seconds=" << seconds << '\n';
  return ok;
}

// Assignment after latching through the existing path: find an active non-climber lemming whose
// projected box is the first hit under its centre (mirrors test_levelinteraction), click it, and
// require one climber stock to be spent. Frame count is data dependent, so it quits itself.
bool runAssignAfterLatch(const char* gameDir, std::ostream& log) {
  std::vector<int> ticks;
  int target = -1, tx = 0, ty = 0, clickedFrame = -1, stockBefore = -1, frames = 0, attempts = 0;
  bool assigned = false, stockOk = false, selectionOk = true, done = false;
  const auto observer = [&](const app::LevelFrameView& frame) {
    ++frames;
    ticks.push_back(static_cast<int>(frame.world.tickCount()));
    const int climbers = frame.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)];
    if (frame.selectedSkillIndex != 0) selectionOk = false;
    if (frame.frame == 0) { pushButton(SDL_BUTTON_LEFT, kPlate8X, kPlate8Y); return; }
    if (clickedFrame >= 0) {
      if (frame.frame == clickedFrame + 1) {
        assigned = target >= 0 && frame.world.lemmings()[static_cast<std::size_t>(target)].canClimb;
        stockOk = stockBefore >= 0 && climbers == stockBefore - 1;
        if (assigned || ++attempts >= 6) { done = true; pushQuit(); return; }
        clickedFrame = -1;  // the pick rules rejected this lemming (not a Walker yet): try again
      } else {
        return;
      }
    }
    if (frame.frame >= 2) {
      const auto& lemmings = frame.world.lemmings();
      for (const auto& view : frame.actors) {
        const auto& l = lemmings[view.index];
        // Climber picks reject Fallers (RuntimePick.cpp passesKnownPickExclusions): wait for a Walker.
        if (!l.active() || l.canClimb || l.state != sim::State::Walker) continue;
        const int x = static_cast<int>((view.placement.left + view.placement.right) / 2);
        const int y = static_cast<int>((view.placement.top + view.placement.bottom) / 2);
        if (x < 100 || x >= 640 || y < 0 || y >= 400) continue;
        // Accept only the first box under the cursor so the pick cannot choose another actor.
        bool earlier = false;
        for (const auto& prior : frame.actors) {
          if (&prior == &view) break;
          const auto cursorL = x - 16, cursorR = x + 16, cursorT = y - 16, cursorB = y + 16;
          if (prior.placement.left < cursorR && prior.placement.right > cursorL &&
              prior.placement.top < cursorB && prior.placement.bottom > cursorT) earlier = true;
        }
        if (earlier) continue;
        target = static_cast<int>(view.index); tx = x; ty = y;
        stockBefore = climbers; clickedFrame = frame.frame;
        pushButton(SDL_BUTTON_LEFT, tx, ty);
        return;
      }
    }
    if (frame.frame >= 30) { pushQuit(); done = true; }
  };
  std::vector<std::string> args{"fastforward-app-test", "--level", "MM_CLIMB_002", "--sim",
      "--frames", "40", "--game-dir", gameDir};
  std::vector<char*> raw;
  for (auto& a : args) raw.push_back(a.data());
  app::ApplicationRunOptions options;
  options.frameSeconds = 0.4;
  const int status = app::runApplication(static_cast<int>(raw.size()), raw.data(), {}, observer, options);
  const bool ok = status == 0 && done && assigned && stockOk && selectionOk;
  log << (ok ? "ok   " : "FAIL ") << kSide << " assign-after-latch: ticks=[" << join(ticks)
      << "] attempts=" << attempts + (assigned ? 1 : 0) << " target=" << target << " assigned=" << assigned << " climber-stock-spent=" << stockOk
      << " selection-unchanged=" << selectionOk << " status=" << status << '\n';
  return ok;
}
}  // namespace
#endif

int main() {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "FastForwardApp: requires SDL2\n";
  return 1;
#else
  const char* driver = std::getenv("SDL_VIDEODRIVER");
  if (!driver || std::string(driver) != "dummy") {
    std::cerr << "FastForwardApp: refusing to run without SDL_VIDEODRIVER=dummy\n";
    return 1;
  }
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir ||
      !std::filesystem::is_regular_file(std::filesystem::path(gameDir) / "lemmings.box")) {
    std::cout << "FastForwardApp: SKIPPED (retail assets unavailable)\n";
    return 0;
  }
  // Keep the geometry used here tied to the production hit test.
  if (app::runtimeSkillPanelIndex(kPlate8X, kPlate8Y) != 8 || app::runtimeSkillPanelIndex(kPlate9X, kPlate9Y) != 9) {
    std::cerr << "FAIL: plate coordinates are not runtimeSkillPanelIndex-consistent\n";
    return 1;
  }

  const std::vector<int> plain5{2, 4, 6, 8, 10};
  const std::vector<Scenario> scenarios{
      {"click-plate-8", 5, [](int f) { if (f == 0) pushButton(SDL_BUTTON_LEFT, kPlate8X, kPlate8Y); },
       {2, 16, 30, 44, 58}, plain5, {}},
      {"drag-onto-plate-8", 5, [](int f) {
         if (f == 0) pushMotion(SDL_BUTTON_LMASK, 300, 200);        // button held, off the plate
         if (f == 1) pushMotion(SDL_BUTTON_LMASK, kPlate8X, kPlate8Y);  // then onto plate 8
       }, {2, 4, 18, 32, 46}, plain5, {}},
      {"click-plate-9", 5, [](int f) { if (f == 0) pushButton(SDL_BUTTON_LEFT, kPlate9X, kPlate9Y); },
       plain5, plain5, {}},
      {"click-non-plate", 5, [](int f) { if (f == 0) pushButton(SDL_BUTTON_LEFT, 5, 5); },
       plain5, plain5, {}},
      {"hover-plate-8-no-button", 5, [](int f) { if (f == 0) pushMotion(0, kPlate8X, kPlate8Y); },
       plain5, plain5, {}},
      {"keyboard-8", 5, [](int f) { if (f == 0) pushKey(SDL_SCANCODE_8, SDLK_8); },
       plain5, plain5, {}},
      {"right-press-on-plate-8", 5, [](int f) { if (f == 0) pushButton(SDL_BUTTON_RIGHT, kPlate8X, kPlate8Y); },
       plain5, plain5, {}},
      {"right-held-then-left-click-plate-8", 5, [](int f) {
         if (f == 0) pushButton(SDL_BUTTON_RIGHT, 320, 240);
         if (f == 1) pushButton(SDL_BUTTON_LEFT, kPlate8X, kPlate8Y);
       }, plain5, plain5, {}},
      // P at f0 (paused from f1), plate-8 click at f1, P again at f3 (running from f4). The latch is
      // set while paused with no ticks, and the first running frame is 7 passes x 2 = 14 more.
      {"paused-click-plate-8", 5, [](int f) {
         if (f == 0) pushKey(SDL_SCANCODE_P, SDLK_p);
         if (f == 1) pushButton(SDL_BUTTON_LEFT, kPlate8X, kPlate8Y);
         if (f == 3) pushKey(SDL_SCANCODE_P, SDLK_p);
       }, {2, 2, 2, 2, 16}, {2, 2, 2, 2, 4}, {false, true, true, true, false}},
  };

  const std::filesystem::path outDir = std::filesystem::path(FASTFORWARD_IMPL_DIR);
  std::error_code ec;
  std::filesystem::create_directories(outDir, ec);
  std::ofstream file(outDir / (std::string("app_results_") + kSide + ".txt"));
  struct Tee : std::streambuf {
    std::streambuf* a; std::streambuf* b;
    Tee(std::streambuf* x, std::streambuf* y) : a(x), b(y) {}
    int overflow(int c) override { a->sputc(static_cast<char>(c)); return b->sputc(static_cast<char>(c)); }
  } tee(std::cout.rdbuf(), file.rdbuf());
  std::ostream log(&tee);
  log << "fastforward-impl-001 " << kSide << " (injected SDL events = source-test setup, not natural play)\n";

  bool ok = true;
  for (const auto& sc : scenarios) ok = runScenario(sc, gameDir, log) && ok;
  ok = runAssignAfterLatch(gameDir, log) && ok;
  log << (ok ? "PASS" : "FAIL") << ": fastforward app " << kSide << '\n';
  return ok ? 0 : 1;
#endif
}
