// native-023: the public application route for the installed MM_TELE_003,
//   runApplication(--level MM_TELE_003 --sim) -> sim::loadRetailLevel ->
//   LevelSetup::teleports -> World,
// observed through the read-only per-frame view. The run ends through an
// observer-queued SDL_QUIT under the SDL dummy drivers; --frames is only a
// failure guard that is never reached.
//
// Literal expectations: the four pads of MM_TELE_003 in locator order stand on
// columns 219, 241, 102 and 79 (step-1 measurement; the retained live table of
// round 16 has the same columns), each with the closed 12-wide box
// [col-6, col+6]. Ids pair 3<->5 and 4<->6, i.e. locator pads 0<->2 and 1<->3, so
// every destination column is the partner pad's column. This level pairs
// identically under the old file-order rule; the level-changing evidence is in
// teleportdemo_before/after. No original code runs.
#include "app/AppEntry.h"
#include "sim/World.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace webo;

namespace {
bool pushQuit() {
  SDL_Event e{};
  e.type = SDL_QUIT;
  return SDL_PushEvent(&e) == 1;
}
}  // namespace

int main() {
  const char* gameDirC = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDirC || !*gameDirC) {
    std::cout << "TeleportApp: SKIPPED (set WEBOLUTION_GAME_DIR to retail assets)\n";
    return 0;
  }
  const char* video = std::getenv("SDL_VIDEODRIVER");
  const char* audio = std::getenv("SDL_AUDIODRIVER");
  if (!video || std::string(video) != "dummy" || !audio || std::string(audio) != "dummy") {
    std::cerr << "TeleportApp: refusing to run without SDL dummy video and audio drivers\n";
    return 1;
  }
  const std::string gameDir = gameDirC;
  std::vector<std::string> args{"teleportapp", "--game-dir", gameDir, "--level", "MM_TELE_003",
                                "--sim", "--native", "--frames", "40", "--no-audio"};
  std::vector<char*> argv;
  for (auto& a : args) argv.push_back(a.data());

  constexpr int kLastFrame = 3;
  constexpr std::array<int, 4> kCol{219, 241, 102, 79};
  constexpr std::array<int, 4> kPartner{2, 3, 0, 1};

  int failures = 0;
  const auto expect = [&](bool ok, const std::string& what) {
    if (!ok) {
      ++failures;
      std::cerr << "FAIL: " << what << '\n';
    }
  };

  int callbacks = 0;
  bool queued = false;
  std::size_t teleportCount = 0;
  std::string level;
  std::uint64_t firstTick = 0, lastTick = 0;
  std::array<sim::Teleport, 4> seen{};
  const int result = app::runApplication(
      static_cast<int>(argv.size()), argv.data(), {},
      [&](const app::LevelFrameView& f) {
        ++callbacks;
        const auto& tele = f.world.setup().teleports;
        if (f.frame == 0) {
          level = std::string(f.levelName);
          teleportCount = tele.size();
          firstTick = f.world.tickCount();
          for (std::size_t i = 0; i < tele.size() && i < seen.size(); ++i) seen[i] = tele[i];
        }
        lastTick = f.world.tickCount();
        if (f.frame >= kLastFrame && !queued) queued = pushQuit();
      },
      app::ApplicationRunOptions{.frameSeconds = 0.25});

  expect(result == 0, "exit status " + std::to_string(result));
  expect(queued, "observer queued SDL_QUIT");
  expect(callbacks == kLastFrame + 1, "callbacks " + std::to_string(callbacks));
  expect(level == "MM_TELE_003", "observed level '" + level + "'");
  expect(teleportCount == 4, "World built with " + std::to_string(teleportCount) + " teleports");
  expect(lastTick >= firstTick, "tick count does not run backwards");
  if (teleportCount == 4) {
    for (std::size_t i = 0; i < 4; ++i) {
      expect(seen[i].x == kCol[i] - 6 && seen[i].width == 12 && seen[i].height == 8,
             "pad " + std::to_string(i) + " box at column " + std::to_string(kCol[i]));
      expect(seen[i].destX == kCol[static_cast<std::size_t>(kPartner[i])],
             "pad " + std::to_string(i) + " destination column is pad " +
                 std::to_string(kPartner[i]) + "'s column");
    }
  }
  std::cout << (failures == 0 ? "PASS" : "FAIL") << ": TeleportApp MM_TELE_003 --sim, " << callbacks
            << " frames, ticks " << firstTick << ".." << lastTick << ", " << teleportCount
            << " teleports from the loader\n";
  return failures == 0 ? 0 : 1;
}
