// Check work cues through CuePlayback -> Mixer::play -> Mixer::fill.
#include "assets/Box.h"
#include "assets/Wav.h"
#include "audio/CuePlayback.h"
#include "sim/World.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

namespace webo::audio {
// Pause the dummy callback before loading/playing. Manual fill then uses the
// real mixer with deterministic sample intervals and no callback race.
struct WorkSoundTestAccess {
  static Mixer& mixer(CuePlayback& playback) { return playback.mixer_; }
  static void pause(Mixer& mixer) { SDL_PauseAudioDevice(mixer.device_, 1); }
  static std::size_t voices(const Mixer& mixer) { return mixer.voices_.size(); }
  static std::vector<std::int16_t> render(Mixer& mixer, int frames) {
    std::vector<std::int16_t> out(static_cast<std::size_t>(frames) * mixer.channels());
    mixer.fill(out.data(), frames);
    return out;
  }
};
}  // namespace webo::audio

using namespace webo;
using Access = audio::WorkSoundTestAccess;
namespace {
int failures = 0;
void check(bool passed, const char* message) {
  if (!passed) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}

void attacks(const Sound& source) {
  audio::CuePlayback playback;
  auto& mixer = Access::mixer(playback);
  const auto opened = mixer.open(22050, 2);
  check(opened.has_value(), "dummy mixer opens");
  if (!opened) return;
  Access::pause(mixer);
  check(mixer.load("Mine", source).has_value(), "work sample loads through production resampler");
  const auto expected = resampleTo(source, 22050, 2);
  if (expected.frameCount() < 8192) {
    check(false, "test sample must outlast the fast-forward/worker intervals");
    return;
  }

  // A different effect must survive every Mine restart. Silent PCM lets us
  // compare the work output exactly while still checking its voice survives.
  Sound other{22050, 2, std::vector<std::int16_t>(expected.samples.size())};
  check(mixer.load("Other", other).has_value(), "unrelated effect loads");
  mixer.play("Other");

  // Intervals are shorter than MINE.WAV. 3780/4725 frames approximate Digger
  // and Miner fast-forward spacing. Each cue restarts at sample zero
  // independently of the previous tail (SkipPlaying or Add/Average).
  for (int frames : {256, 3780, 97, 4725, 1, 256}) {
    playback.play(sim::SoundCue::Work);
    const auto out = Access::render(mixer, frames);
    check(std::equal(out.begin(), out.end(), expected.samples.begin()),
          "every work request starts fresh PCM even while the prior tail is active");
    check(Access::voices(mixer) <= 2, "work cannot stack tails or erase the unrelated effect");
  }
  // The unrelated effect still has frames left after these requests.
  check(Access::voices(mixer) == 2, "restarting Mine leaves another SFX voice playing");

  mixer.stopAll();
  for (int group = 0; group < 4; ++group) {
    for (int worker = 0; worker < 100; ++worker) playback.play(sim::SoundCue::Work);
    check(Access::voices(mixer) == 1, "simultaneous work shares one bounded fresh attack");
    const auto out = Access::render(mixer, 256);
    check(std::equal(out.begin(), out.end(), expected.samples.begin()),
          "a work burst restarts its attack without clipping or summing tails");
  }
  Access::render(mixer, static_cast<int>(expected.frameCount()));
  check(Access::voices(mixer) == 0, "last one-shot finishes with no deferred work or loop");
  const auto silence = Access::render(mixer, 256);
  check(std::all_of(silence.begin(), silence.end(), [](auto s) { return s == 0; }),
        "no new work request means silence after the final tail");
}
}  // namespace

int main() {
  SDL_SetMainReady();
  SDL_SetHintWithPriority(SDL_HINT_AUDIODRIVER, "dummy", SDL_HINT_OVERRIDE);
  // Distinct positive and negative samples distinguish a clean restart from a stacked tail.
  Sound synthetic{22050, 1, std::vector<std::int16_t>(22050)};
  for (std::size_t i = 0; i < synthetic.samples.size(); ++i)
    synthetic.samples[i] = static_cast<std::int16_t>(static_cast<int>(i % 20001) - 10000);
  attacks(synthetic);

  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR")) {
    const auto root = std::filesystem::path(dir);
    auto lib = AssetLibrary::open(root / "lemmings.box", root / "patch.box");
    check(lib.has_value(), "installed overlay opens read-only");
    if (lib) {
      auto mine = loadSound(*lib, "SOUNDS\\MINE.WAV");
      check(mine.has_value(), "installed Mine decodes");
      if (mine) { attacks(*mine); std::cout << "installed Mine PCM checked\n"; }
    }
  }
  std::cout << "work sound PCM: " << failures << " failures\n";
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
