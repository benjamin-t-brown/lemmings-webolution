// Legacy audio units: real callback PCM and installed WAV loading, no level
// route, recording, save, physical output or original-game execution.
#include "assets/Box.h"
#include "audio/CuePlayback.h"
#include "sim/World.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <vector>

namespace webo::audio {
struct MusicTestAccess {
  static Mixer& mixer(CuePlayback& p) { return p.mixer_; }
  static void pause(Mixer& m) { SDL_PauseAudioDevice(m.device_, 1); }
  static const auto& voices(const Mixer& m) { return m.voices_; }
  static const auto& track(const CuePlayback& p) { return p.currentMusic_; }
  static const auto& attempts(const CuePlayback& p) { return p.attemptedMusic_; }
  static void bind(CuePlayback& p, const AssetLibrary& lib) { p.library_ = &lib; }
  static void select(CuePlayback& p, std::string_view name) { p.playMusic(name, true); }
  static std::vector<std::int16_t> render(Mixer& m, int frames) {
    std::vector<std::int16_t> out(static_cast<std::size_t>(frames) * m.channels());
    m.fill(out.data(), frames);
    return out;
  }
};
}  // namespace webo::audio

using namespace webo;
using Access = audio::MusicTestAccess;
namespace {
int failures = 0;
void check(bool passed, const char* message) {
  if (!passed) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}

void mixAndLoop() {
  audio::Mixer mixer;
  auto opened = mixer.open(22050, 2);
  check(opened.has_value(), "dummy device opens");
  if (!opened) return;
  Access::pause(mixer);
  mixer.setMusicVolume(0.3f);
  const Sound bed{22050, 1, {1000, -1000, 2000}};
  const Sound effect{22050, 1, std::vector<std::int16_t>(64, 3000)};
  check(mixer.load("bed", bed).has_value(), "synthetic bed loads");
  check(mixer.load("effect", effect).has_value(), "synthetic effect loads");
  mixer.playMusic("bed");
  const auto looped = Access::render(mixer, 10);
  for (std::size_t i = 0; i < looped.size(); ++i)
    check(looped[i] == static_cast<std::int16_t>(bed.samples[(i / 2) % 3] * 0.3f),
          "music loops through multiple boundaries with stereo fan-out and lower gain");
  check(Access::voices(mixer).size() == 1 && Access::voices(mixer)[0].cursor == 1,
        "callback advances the music cursor independently of any simulation updates");
  mixer.play("effect");
  const auto mixed = Access::render(mixer, 1);
  check(mixed[0] == 2700 && mixed[1] == 2700,
        "SFX attack is mixed immediately at full gain over the continuing bed");
  mixer.playMusic("bed");
  check(Access::voices(mixer).size() == 2 && Access::voices(mixer)[0].cursor == 1,
        "restarting music preserves the SFX tail and its cursor");
  mixer.stopMusic();
  check(Access::voices(mixer).size() == 1 && !Access::voices(mixer)[0].music,
        "stopping music preserves effects");
  mixer.stopAll();
  mixer.playMusic("bed");
  for (int n = 0; n < 32; ++n) mixer.play("effect");
  check(Access::voices(mixer).size() == 33,
        "music does not steal any of the 32 additive SFX slots");
  mixer.play("effect");
  check(Access::voices(mixer).size() == 33 && Access::voices(mixer)[0].music,
        "overflow keeps the bed and the existing bounded SFX capacity");
}

void failuresAndDisabled() {
  AssetLibrary empty;
  audio::CuePlayback missing;
  auto& mixer = Access::mixer(missing);
  const auto opened = mixer.open();
  check(opened.has_value(), "missing-asset dummy device opens");
  if (!opened) return;
  Access::pause(mixer);
  Access::bind(missing, empty);
  int diagnostics = 0;
  missing.setDiagnosticSink([&](const auto&) { ++diagnostics; });
  missing.playFrontendMusic(); missing.playFrontendMusic();
  check(diagnostics == 1 && Access::voices(mixer).empty(),
        "a missing track reports once and continues silently");
  missing.disable();
  missing.playLevelMusic();
  check(Access::attempts(missing).size() == 1, "disabled music makes no load attempts");

  audio::CuePlayback disabled;
  disabled.disable(); disabled.start(empty, true);
  disabled.playFrontendMusic(); disabled.playLevelMusic();
  check(!disabled.deviceReady() && Access::attempts(disabled).empty(),
        "--no-audio opens no device and never reads a music asset");
  audio::CuePlayback noDevice;
  noDevice.start(empty, false);
  noDevice.playFrontendMusic(); noDevice.playLevelMusic();
  check(!noDevice.deviceReady() && Access::attempts(noDevice).empty(),
        "audio off test mode remains device-free");
}

void installed(const AssetLibrary& library) {
  audio::CuePlayback playback;
  playback.start(library, true);
  check(playback.deviceReady(), "installed playback opens a dummy device");
  if (!playback.deviceReady()) return;
  auto& mixer = Access::mixer(playback);
  Access::pause(mixer);
  int effects = 0;
  playback.setObserver([&](auto) { ++effects; });
  playback.playFrontendMusic();
  check(Access::track(playback) == "lemming01" && Access::voices(mixer).size() == 1,
        "installed frontend track starts as the only looping voice");
  const auto out = Access::render(mixer, 10000);
  check(std::any_of(out.begin(), out.end(), [](auto s) { return s != 0; }),
        "installed frontend music produces nonzero PCM through the mixer");
  playback.playFrontendMusic();
  check(Access::voices(mixer).size() == 1 && Access::voices(mixer)[0].cursor == 10000,
        "frontend/map page entry does not restart the current track");
  playback.play(sim::SoundCue::Assign);
  check(effects == 1 && playback.routed() == 1 && playback.drained() == 1,
        "music never enters SFX routing counters or the SFX observer");
  playback.playLevelMusic();
  const auto track = Access::track(playback);
  check(track >= "lemming02" && track <= "lemming13", "level choice is within the installed table");
  check(Access::voices(mixer).size() == 2 && Access::voices(mixer).back().music &&
            Access::voices(mixer).back().cursor == 0,
        "level load replaces the bed and preserves an assignment effect");
  Access::render(mixer, 100);
  playback.playLevelMusic();
  check(Access::voices(mixer).back().music && Access::voices(mixer).back().cursor == 0,
        "Retry starts a fresh bed even when its random draw repeats");
  playback.playFrontendMusic();
  check(Access::track(playback) == "lemming01" && Access::voices(mixer).back().cursor == 0,
        "returning from a level restarts the frontend bed");
  std::cout << "installed frontend and level/Retry PCM checked; first level track " << track << '\n';

  // A random draw must never hide an unreadable installed track. Select each
  // explicitly in this unit, without consuming or changing the production RNG.
  mixer.stopAll();
  for (int n = 1; n <= 13; ++n) {
    const auto name = std::format("lemming{:02}", n);
    Access::select(playback, name);
    const auto& voices = Access::voices(mixer);
    check(voices.size() == 1 && voices[0].music && voices[0].loop,
          "each installed track replaces only the previous looping voice");
    const auto pcm = Access::render(mixer, 10000);
    check(std::any_of(pcm.begin(), pcm.end(), [](auto s) { return s != 0; }),
          "every installed track decodes, resamples and produces nonzero PCM");
    std::cout << name << ": " << (voices.empty() ? 0 : voices[0].clip->samples.size() / mixer.channels())
              << " output frames\n";
  }
  check(Access::attempts(playback).size() == 13, "each installed track is loaded at most once");
  Access::select(playback, "lemming01");
  check(Access::attempts(playback).size() == 13 && Access::voices(mixer).size() == 1 &&
            Access::voices(mixer)[0].cursor == 0,
        "a cached track can restart without reloading or stacking music");
}
}  // namespace

int main() {
  SDL_SetMainReady();
  SDL_SetHintWithPriority(SDL_HINT_AUDIODRIVER, "dummy", SDL_HINT_OVERRIDE);
  mixAndLoop();
  failuresAndDisabled();
  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR"); dir && *dir) {
    const auto root = std::filesystem::path(dir);
    auto lib = AssetLibrary::open(root / "lemmings.box", root / "patch.box");
    check(lib.has_value(), "installed overlay opens read-only");
    if (lib) installed(*lib);
  }
  std::cout << "music: " << failures << " failures\n";
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
