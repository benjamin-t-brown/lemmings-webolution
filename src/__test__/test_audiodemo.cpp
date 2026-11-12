// native-022 source-built before/after demonstration. The included application
// source is either authentic HEAD frozen under build/native-022 or current.
// Only loadRetailLevel is substituted in this test translation unit. Both sides
// receive the SAME synthetic setup on the real installed MM_CLIMB_002 terrain.
// No production observer mutates a World, and no original code is executed.
#include "app/AppEntry.h"
#include "assets/Box.h"
#include "sim/RetailLevel.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace audiodemo {
bool longFall = false;
}

namespace webo::sim {
std::expected<LevelSetup, std::string> loadAudioDemoLevel(
    const AssetLibrary& library, std::string_view id) {
  auto retail = loadRetailLevel(library, id);
  if (!retail) return std::unexpected(retail.error());
  // Preserve the installed collision terrain, including its authored flags.
  // Synthetic population, stock, releases and exits isolate each cue. These
  // are test inputs, not a claim about retail level play or original timing.
  LevelSetup setup;
  setup.terrain = retail->terrain;
  setup.entrances = retail->entrances;
  setup.totalLemmings = 1;
  setup.releaseIntervalTicks = 1;
  // The UI clamps unlimited (-1) stock to zero; use one real selectable bomb.
  setup.skills.fill(0);
  setup.skills[static_cast<std::size_t>(Skill::Bomber)] = 1;
  if (audiodemo::longFall) {
    const auto& terrain = setup.terrain;
    bool found = false;
    for (int x = 0; x < terrain.width() && !found; ++x) {
      for (int start = 0; start < terrain.height() && !found; ++start) {
        if (terrain.solid(x, start) || terrain.waterHazard(x, start)) continue;
        int y = start;
        while (y < terrain.height() && !terrain.solid(x, y) &&
               !terrain.waterHazard(x, y)) ++y;
        if (y < terrain.height() && y - start > World::Rules{}.splatDistance + 2 &&
            !terrain.waterHazard(x, y) && !terrain.trampoline(x, y)) {
          setup.entrances = {{x, start}};
          found = true;
        }
        start = y;
      }
    }
    if (!found) return std::unexpected("audio demo: no long drop on installed terrain");
  }
  std::cout << "synthetic MM_CLIMB_002 " << (audiodemo::longFall ? "long-fall" : "bomber")
            << " entrance=" << setup.entrances.front().x << ',' << setup.entrances.front().y
            << " terrain=" << setup.terrain.width() << 'x' << setup.terrain.height() << '\n';
  return setup;
}
}

// Headers above are already guarded. Do not rewrite or instrument the frozen
// main.cpp; this sole loader seam is applied equally to frozen and current.
#define loadRetailLevel loadAudioDemoLevel
#include WEBOLUTION_AUDIO_DEMO_MAIN
#undef loadRetailLevel

namespace audiodemo {
bool key(SDL_Scancode scan, SDL_Keycode sym) {
  SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.keysym.scancode = scan; e.key.keysym.sym = sym;
  return SDL_PushEvent(&e) == 1;
}
bool click(int x, int y) {
  SDL_Event e{}; e.type = SDL_MOUSEBUTTONDOWN; e.button.button = SDL_BUTTON_LEFT;
  e.button.x = x; e.button.y = y; return SDL_PushEvent(&e) == 1;
}
bool quit() {
  SDL_Event e{}; e.type = SDL_QUIT; return SDL_PushEvent(&e) == 1;
}

bool run(const std::string& gameDir, std::ostream& report) {
  bool ok = true, reached = false, assigned = false, quitSent = false;
  bool device = false;
  std::size_t pending = 0, drained = 0;
  std::uint64_t ticks = 0;
  std::vector<std::string> heard;
  webo::app::ApplicationRunOptions options;
  options.frameSeconds = 0.2;
  options.enableAudio = true;
  options.cueObserver = [&](std::string_view name) { heard.emplace_back(name); };
  const auto level = [&](const webo::app::LevelFrameView& f) {
    const auto& lemmings = f.world.lemmings();
    device = f.audioDeviceReady;
    if (!longFall && !lemmings.empty()) {
      assigned |= lemmings[0].fuseMs >= 0;
      if (!assigned) {
        for (const auto& actor : f.actors) {
          if (actor.index != 0) continue;
          const int x = static_cast<int>((actor.placement.left + actor.placement.right) / 2);
          const int y = static_cast<int>((actor.placement.top + actor.placement.bottom) / 2);
          if (x >= 100 && x < 640 && y >= 0 && y < 480) {
            ok &= key(SDL_SCANCODE_3, SDLK_3) && click(x, y);
            break;
          }
        }
      }
    }
    reached = !lemmings.empty() && (longFall ?
        lemmings[0].state == webo::sim::State::Splatter :
        assigned && lemmings[0].state == webo::sim::State::Exploder);
    if (reached) {
      // Inspect a COPY: the driver never consumes/mutates the app's World.
      auto snapshot = f.world;
      const auto left = snapshot.takeCues();
      pending = left.size(); drained = f.audioCuesDrained; ticks = f.world.tickCount();
#ifdef WEBOLUTION_AUDIO_DEMO_BEFORE
      const auto expected = longFall ? webo::sim::SoundCue::Splat : webo::sim::SoundCue::OhNo;
      ok &= std::count(left.begin(), left.end(), expected) == 1 && heard.empty() &&
            pending > 0 && drained == 0 && !device;
#else
      // Literal expectations, not another recovered invocation's output.
      ok &= heard == std::vector<std::string>{longFall ? "FallSplat" : "Ohno"} &&
            pending == 0 && drained > 0 && device;
#endif
      quitSent = quit(); ok &= quitSent;
    } else if (f.frame >= 78) {
      ok = false; quit();  // failure guard, never success
    }
  };
  std::vector<std::string> args{"audiodemo", "--level", "MM_CLIMB_002", "--sim", "--native", "--scene-off",
      "--frames", "80", "--game-dir", gameDir};
  std::vector<char*> argv; for (auto& a : args) argv.push_back(a.data());
  const int result = webo::app::runApplication(static_cast<int>(argv.size()), argv.data(), {}, level, options);
  ok &= result == 0 && reached && quitSent;
#ifdef WEBOLUTION_AUDIO_DEMO_BEFORE
  constexpr auto side = "frozen HEAD";
#else
  constexpr auto side = "current";
#endif
  report << side << ' ' << (longFall ? "long-fall" : "bomber-fuse")
         << ": result=" << result << " reached=" << reached << " assigned=" << assigned
         << " tick=" << ticks << " device=" << device << " appConsumed=" << drained
         << " pending=" << pending << " observer=";
  for (const auto& name : heard) report << name << ' ';
  report << " quit=" << quitSent << " => " << (ok ? "PASS" : "FAIL") << '\n';
  return ok;
}
}

int main() {
  for (const auto* var : {"SDL_VIDEODRIVER", "SDL_AUDIODRIVER"}) {
    const char* value = std::getenv(var);
    if (!value || std::string(value) != "dummy") {
      std::cerr << "audio demo requires dummy video/audio\n"; return 1;
    }
  }
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir) { std::cerr << "audio demo requires installed assets\n"; return 1; }
  std::filesystem::create_directories(WEBOLUTION_AUDIO_DEMO_DIR);
#ifdef WEBOLUTION_AUDIO_DEMO_BEFORE
  std::ofstream report(std::filesystem::path(WEBOLUTION_AUDIO_DEMO_DIR) / "before.txt");
#else
  std::ofstream report(std::filesystem::path(WEBOLUTION_AUDIO_DEMO_DIR) / "after.txt");
#endif
  bool ok = audiodemo::run(gameDir, report);
  audiodemo::longFall = true;
  ok = audiodemo::run(gameDir, report) && ok;
  report.flush();
  return ok && report ? 0 : 1;
}
