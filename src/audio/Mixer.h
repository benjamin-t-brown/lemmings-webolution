// SDL2 sound playback for the retail `.WAV` cues.
//
// The decoder lives in assets/Wav and stays free of SDL: it turns archive
// entries into signed 16-bit PCM at whatever rate they arrived. This mixer
// owns the device, resamples every loaded cue to one output format once, and
// mixes concurrent voices in the audio callback.
//
// Opening the device is optional. The game shell treats failure the same way
// it treats missing sprites -- stderr and keep going -- so a headless CI box
// without an audio backend is not a hard error.
//
// Threading: play() is called from the main thread; the callback runs on
// SDL's audio thread. Shared state is guarded with SDL_LockAudioDevice, which
// is the simplest correct answer and cheap at the rates these cues need.

#pragma once

#include "assets/Wav.h"

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
  Mixer() = default;
  ~Mixer();
  Mixer(const Mixer&) = delete;
  Mixer& operator=(const Mixer&) = delete;

  // Opens the default output device. sampleRate/channels are the mix format;
  // every cue is resampled to them on load. 22050 stereo matches the plurality
  // of the retail files, so most loads are a channel fan-out rather than a
  // rate conversion.
  std::expected<void, std::string> open(int sampleRate = 22050, int channels = 2);
  void close();
  bool ready() const { return device_ != 0; }

  int sampleRate() const { return sampleRate_; }
  int channels() const { return channels_; }

  // Decode + resample into the mix format and keep by cue name ("Letsgo").
  // Replaces a previous load of the same name.
  std::expected<void, std::string> load(std::string_view cue, const Sound& source);
  std::expected<void, std::string> loadFrom(const AssetLibrary& lib, std::string_view cue);

  bool has(std::string_view cue) const;
  void play(std::string_view cue, float volume = 1.0f);

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
  struct Clip {
    std::vector<std::int16_t> samples;  // interleaved, mix format
  };

  struct Voice {
    const Clip* clip = nullptr;
    std::size_t cursor = 0;
    float volume = 1.0f;
    bool loop = false;
    bool music = false;
  };

  static void callback(void* userdata, std::uint8_t* stream, int len);
  void fill(std::int16_t* out, int frames);

  std::uint32_t device_ = 0;
  int sampleRate_ = 0;
  int channels_ = 0;
  float musicVolume_ = 0.6f;
  float sfxVolume_ = 1.0f;

  std::unordered_map<std::string, Clip> clips_;
  std::vector<Voice> voices_;
};

}  // namespace webo::audio
