// Sound playback for the retail `.WAV` cues.
//
// The decoder lives in assets/Wav and stays free of SDL: it turns archive
// entries into signed 16-bit PCM at whatever rate they arrived. Clips are
// converted to the mix format on load. Desktop uses an SDL audio callback.
// The browser schedules the clips directly on its Web Audio rendering clock.
//
// Opening the device is optional. The game shell treats failure the same way
// it treats missing sprites -- stderr and keep going -- so a headless CI box
// without an audio backend is not a hard error.
//
// The game calls play() on the main thread. Desktop's SDL callback shares
// the voice state under SDL_LockAudioDevice. Web Audio owns its rendered PCM.

#pragma once

#include "assets/Wav.h"
#include "audio/SpatialSound.h"

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace webo {
class AssetLibrary;
}

namespace webo::audio {

class Mixer {
public:
  // Average is an opt-in bus for overlapping exit cheers. Other SFX retain
  // the existing additive mix. The policy is provisional, not retail mixing.
  // SkipPlaying is an additive one-shot, suppressed while the same clip is
  // still audible. No delayed requests or looping voice are created.
  // RestartPlaying starts each work/warning attack without stacking tails.
  enum class Overlap { Add, Average, SkipPlaying, RestartPlaying };

  Mixer() = default;
  ~Mixer();
  Mixer(const Mixer&) = delete;
  Mixer& operator=(const Mixer&) = delete;

  // Opens the default output device. Clips use sampleRate/channels on load.
  // The browser renders them at the Web Audio device's own rate.
  std::expected<void, std::string> open(int sampleRate = 22050, int channels = 2);
  void close();
#ifdef __EMSCRIPTEN__
  bool ready() const { return webReady_; }
#else
  bool ready() const { return device_ != 0; }
#endif

  int sampleRate() const { return sampleRate_; }
  int channels() const { return channels_; }

  // Decode + resample into the mix format and keep by cue name ("Letsgo").
  // Replaces a previous load of the same name.
  std::expected<void, std::string> load(std::string_view cue, const Sound& source);
  std::expected<void, std::string> loadFrom(const AssetLibrary& lib, std::string_view cue);

  bool has(std::string_view cue) const;
  void play(std::string_view cue, float volume = 1.0f, Overlap overlap = Overlap::Add,
            StereoGains gains = {});

  // Looping bed (menu / in-level music). One music voice at a time.
  std::expected<void, std::string> loadMusicFrom(const AssetLibrary& lib, std::string_view cue);
  void playMusic(std::string_view cue, float volume = 1.0f);
  void stopMusic();
  void setMusicVolume(float volume);
  void setSfxVolume(float volume);
  float musicVolume() const { return musicVolume_; }
  float sfxVolume() const { return sfxVolume_; }

  // Stops every voice including music. Does not unload cues.
  void stopAll();

private:
  static std::string cueKey(std::string_view cue);
  std::expected<void, std::string> storeClip(std::string key, Sound clip);
  static constexpr int kMaxAdditiveVoices = 32;

#ifdef __EMSCRIPTEN__
  bool webReady_ = false;
#else
  friend struct WorkSoundTestAccess;
  friend struct PositionalSoundTestAccess;
  friend struct MusicTestAccess;
  struct Clip {
    std::vector<std::int16_t> samples;  // interleaved, mix format
  };

  struct Voice {
    const Clip* clip = nullptr;
    std::size_t cursor = 0;
    float volume = 1.0f;
    bool loop = false;
    bool music = false;
    Overlap overlap = Overlap::Add;
    StereoGains gains{};
  };

  static void callback(void* userdata, std::uint8_t* stream, int len);
  void fill(std::int16_t* out, int frames);
  void releaseSubsystem();

  std::uint32_t device_ = 0;
  bool ownsAudioSubsystem_ = false;
  std::unordered_map<std::string, Clip> clips_;
  std::vector<Voice> voices_;
#endif
  int sampleRate_ = 0;
  int channels_ = 0;
  float musicVolume_ = 0.6f;
  float sfxVolume_ = 1.0f;
};

}  // namespace webo::audio
