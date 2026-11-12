// native-022: the audio adapter through the real application path.
//
// Ordinary frontend entry (RETURN, RETURN, map click) reaches a simulated
// retail level with one cue adapter owned by runApplication across the frontend
// and gameplay legs. SDL_QUIT is pushed by the level observer; CTest's timeout
// is only a failure guard. Run under SDL_VIDEODRIVER=dummy and
// SDL_AUDIODRIVER=dummy: no real window or device is ever opened.
//
// Node1 is the installed MM_FLOAT. Let its unmodified World fall naturally:
// the cue observer must receive literal FallSplat and push SDL_QUIT. No skills,
// terrain, population or World state are changed. The production clock runs.
// Cue routing only; no claim about native timing, volume, mixing or resampling.

#include "app/AppEntry.h"
#include "sim/World.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

#if WEBOLUTION_HAVE_SDL2
bool pushKey() {
  SDL_Event event{};
  event.type = SDL_KEYDOWN;
  event.key.type = SDL_KEYDOWN;
  event.key.keysym.sym = SDLK_RETURN;
  return SDL_PushEvent(&event) == 1;
}

bool pushQuit() {
  SDL_Event event{};
  event.type = SDL_QUIT;
  return SDL_PushEvent(&event) == 1;
}

bool pushMapClick(int x, int y) {
  SDL_Event event{};
  event.type = SDL_MOUSEBUTTONDOWN;
  event.button.type = SDL_MOUSEBUTTONDOWN;
  event.button.button = SDL_BUTTON_LEFT;
  event.button.x = x;
  event.button.y = y;
  return SDL_PushEvent(&event) == 1;
}

struct Outcome {
  int result = -1;
  bool ok = true;
  int frontendFrames = 0;
  int levelFrames = 0;
  bool deviceReadyAtLevel = false;
  bool cueQuit = false;
  std::size_t drained = 0;
  std::uint64_t tick = 0;
  std::vector<std::string> cues;
};

Outcome run(const std::string& gameDir, bool enableAudio, bool noAudioFlag,
            bool observeFall = false) {
  Outcome out;
  std::vector<std::string> args{"audioapp-test", "--game-dir", gameDir};
  if (noAudioFlag) args.emplace_back("--no-audio");
  int mapX = 206, mapY = 204;
  if (observeFall) {
    auto library = webo::AssetLibrary::open(fs::path(gameDir) / "lemmings.box", fs::path(gameDir) / "patch.box");
    if (!library) { out.ok = false; return out; }
    auto map = webo::app::MapScene::build(*library, {});
    if (!map || map->levelForNode(1) != "MM_FLOAT") { out.ok = false; return out; }
    mapX = mapY = -1;
    for (int y = 0; y < 480 && mapX < 0; ++y)
      for (int x = 0; x < 640; ++x)
        if (map->pickNode(x, y) == 1) { mapX = x; mapY = y; break; }
    if (mapX < 0) { out.ok = false; return out; }
  }
  std::vector<char*> argv;
  for (auto& arg : args) argv.push_back(arg.data());

  webo::app::ApplicationRunOptions options;  // no saveStore: nothing is persisted
  options.enableAudio = enableAudio;
  options.cueObserver = [&](std::string_view name) {
    out.cues.emplace_back(name);
    if (observeFall) {
      out.ok &= name == "FallSplat";  // literal retained retail cue name
      out.cueQuit = pushQuit(); out.ok &= out.cueQuit;
    }
  };

  const auto frontend = [&](const webo::app::FrontendSession&, int frame) {
    ++out.frontendFrames;
    if (frame == 0 || frame == 1) out.ok &= pushKey();
    else if (frame == 2) out.ok &= pushMapClick(mapX, mapY);
    else { out.ok = false; pushQuit(); }
  };
  const auto level = [&](const webo::app::LevelFrameView& view) {
    ++out.levelFrames;
    out.ok &= !view.levelName.empty() && view.frontendConstructionCounter == 5;
    out.deviceReadyAtLevel = view.audioDeviceReady;
    out.drained = view.audioCuesDrained; out.tick = view.world.tickCount();
    if (observeFall) {
      out.ok &= view.levelName == "MM_FLOAT" && !view.paused;
      if (out.cueQuit) out.ok &= view.world.dead() > 0 && out.drained > 0;
      if (view.world.tickCount() >= 108) { out.ok = false; pushQuit(); }
    } else out.ok &= view.frame == 0 && pushQuit();
  };
  out.result = webo::app::runApplication(static_cast<int>(argv.size()), argv.data(), frontend,
                                         level, options);
  return out;
}
#endif

}  // namespace

int main() {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "AudioApp: requires SDL2\n";
  return 1;
#else
  const char* video = std::getenv("SDL_VIDEODRIVER");
  const char* audio = std::getenv("SDL_AUDIODRIVER");
  if (!video || std::string(video) != "dummy" || !audio || std::string(audio) != "dummy") {
    std::cerr << "AudioApp: refusing to run without SDL_VIDEODRIVER=dummy and SDL_AUDIODRIVER=dummy\n";
    return 1;
  }
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir || !fs::is_regular_file(fs::path(gameDir) / "lemmings.box")) {
    std::cout << "AudioApp: SKIPPED (retail assets unavailable)\n";
    return 0;
  }

  bool passed = true;

  const Outcome fall = run(gameDir, true, false, true);
  std::cout << "ordinary Menu3 -> Confirm4 -> Map5 -> MM_FLOAT: result=" << fall.result
            << " frontendFrames=" << fall.frontendFrames << " levelFrames=" << fall.levelFrames
            << " tick=" << fall.tick << " device=" << fall.deviceReadyAtLevel
            << " drained=" << fall.drained << " cueQuit=" << fall.cueQuit << " observer=";
  for (const auto& name : fall.cues) std::cout << name << ' ';
  std::cout << '\n';
  if (fall.result != 0 || !fall.ok || fall.frontendFrames != 3 ||
      !fall.deviceReadyAtLevel || !fall.cueQuit ||
      fall.cues != std::vector<std::string>{"FallSplat"}) {
    std::cerr << "FAIL: ordinary frontend must observe natural MM_FLOAT FallSplat\n";
    passed = false;
  }

  // Audio on: one device, open at the level, after the frontend leg tore down
  // its window. The cue observer is armed and no cue is expected at frame 0.
  const Outcome on = run(gameDir, true, false);
  std::cout << "audio on: result=" << on.result << " frontendFrames=" << on.frontendFrames
            << " levelFrames=" << on.levelFrames << " deviceReadyAtLevel="
            << on.deviceReadyAtLevel << " routedCues=" << on.cues.size() << '\n';
  if (on.result != 0 || !on.ok || on.frontendFrames != 3 || on.levelFrames != 1 ||
      !on.deviceReadyAtLevel) {
    std::cerr << "FAIL: audio-enabled frontend -> level route\n";
    passed = false;
  }

  // Default options (every other test): no device is opened.
  const Outcome off = run(gameDir, false, false);
  if (off.result != 0 || !off.ok || off.levelFrames != 1 || off.deviceReadyAtLevel) {
    std::cerr << "FAIL: default options must not open an audio device\n";
    passed = false;
  }

  // --no-audio wins over an enabled adapter.
  const Outcome flagged = run(gameDir, true, true);
  if (flagged.result != 0 || !flagged.ok || flagged.levelFrames != 1 ||
      flagged.deviceReadyAtLevel || !flagged.cues.empty()) {
    std::cerr << "FAIL: --no-audio must open no device\n";
    passed = false;
  }

  if (passed) std::cout << "PASS: one audio device across frontend and level legs; --no-audio and defaults silent\n";
  return passed ? 0 : 1;
#endif
}
