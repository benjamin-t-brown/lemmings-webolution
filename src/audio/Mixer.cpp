#include "Mixer.h"

#include "assets/Box.h"

#include <utility>

#ifndef __EMSCRIPTEN__
#include "audio/OverlapMix.h"

#include <SDL.h>

#include <algorithm>
#include <cstring>
#include <format>
#endif

namespace webo::audio {

std::string Mixer::cueKey(std::string_view cue) {
  std::string out(cue);
  for (char& c : out)
    if (c >= 'a' && c <= 'z')
      c = static_cast<char>(c - 'a' + 'A');
  return out;
}

Mixer::~Mixer() { close(); }

std::expected<void, std::string> Mixer::load(std::string_view cue, const Sound& source) {
  if (!ready())
    return std::unexpected("mixer: not open");
  if (source.empty())
    return std::unexpected("mixer: empty sound");

  int clipChannels = channels_;
#ifdef __EMSCRIPTEN__
  // Web Audio expands mono at playback, keeping mono music clips compact in the AudioBuffer cache.
  if (source.channels == 1) clipChannels = 1;
#endif
  Sound mixed = resampleTo(source, sampleRate_, clipChannels);
  if (mixed.empty())
    return std::unexpected("mixer: resample produced nothing");
  return storeClip(cueKey(cue), std::move(mixed));
}

std::expected<void, std::string> Mixer::loadFrom(const AssetLibrary& lib, std::string_view cue) {
  auto sound = loadSound(lib, soundAssetName(cue));
  if (!sound)
    return std::unexpected(sound.error());
  return load(cue, *sound);
}

std::expected<void, std::string> Mixer::loadMusicFrom(const AssetLibrary& lib, std::string_view cue) {
  auto sound = loadSound(lib, musicAssetName(cue));
  if (!sound)
    return std::unexpected(sound.error());
  return load(cue, *sound);
}

#ifndef __EMSCRIPTEN__
std::expected<void, std::string> Mixer::open(int sampleRate, int channels) {
  if (device_)
    close();
  if (sampleRate <= 0 || channels < 1 || channels > 2)
    return std::unexpected("mixer: bad output format");

  // SDL_MAIN_HANDLED builds must notify SDL that main() ran before subsystem initialization.
  SDL_SetMainReady();
  if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
      return std::unexpected(std::format("mixer: SDL_InitSubSystem: {}", SDL_GetError()));
    ownsAudioSubsystem_ = true;
  }

  SDL_AudioSpec want{};
  want.freq = sampleRate;
  want.format = AUDIO_S16SYS;
  want.channels = static_cast<std::uint8_t>(channels);
  want.samples = 1024;
  want.callback = callback;
  want.userdata = this;

  SDL_AudioSpec have{};
  // An exact device format keeps resampling out of the callback.
  const SDL_AudioDeviceID id = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
  if (id == 0) {
    std::string message = std::format("mixer: SDL_OpenAudioDevice: {}", SDL_GetError());
    releaseSubsystem();
    return std::unexpected(std::move(message));
  }

  if (have.freq != sampleRate || have.channels != channels || have.format != AUDIO_S16SYS) {
    SDL_CloseAudioDevice(id);
    releaseSubsystem();
    return std::unexpected(std::format(
        "mixer: device opened as {} Hz {} ch format {:#x}, wanted {} Hz {} ch S16", have.freq,
        have.channels, have.format, sampleRate, channels));
  }

  device_ = id;
  sampleRate_ = sampleRate;
  channels_ = channels;
  SDL_PauseAudioDevice(device_, 0);
  return {};
}

// Stop only the audio subsystem owned by this mixer. SDL_Quit would also stop the window subsystem.
void Mixer::releaseSubsystem() {
  if (!ownsAudioSubsystem_)
    return;
  ownsAudioSubsystem_ = false;
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void Mixer::close() {
  if (!device_) {
    releaseSubsystem();
    return;
  }
  // Close the device to stop the callback before freeing its inputs.
  SDL_CloseAudioDevice(device_);
  device_ = 0;
  voices_.clear();
  clips_.clear();
  sampleRate_ = 0;
  channels_ = 0;
  releaseSubsystem();
}

std::expected<void, std::string> Mixer::storeClip(std::string key, Sound clip) {
  SDL_LockAudioDevice(device_);
  // Stop voices that reference the clip before overwriting it.
  if (const auto it = clips_.find(key); it != clips_.end()) {
    const Clip* dying = &it->second;
    voices_.erase(std::remove_if(voices_.begin(), voices_.end(),
                                 [dying](const Voice& v) { return v.clip == dying; }),
                  voices_.end());
  }
  clips_[std::move(key)] = Clip{std::move(clip.samples)};
  SDL_UnlockAudioDevice(device_);
  return {};
}

bool Mixer::has(std::string_view cue) const {
  if (!ready())
    return false;
  SDL_LockAudioDevice(device_);
  const bool found = clips_.contains(cueKey(cue));
  SDL_UnlockAudioDevice(device_);
  return found;
}

void Mixer::play(std::string_view cue, float volume, Overlap overlap, StereoGains gains) {
  if (!ready() || volume <= 0.0f || sfxVolume_ <= 0.0f)
    return;

  SDL_LockAudioDevice(device_);
  const auto it = clips_.find(cueKey(cue));
  if (it != clips_.end() && !it->second.samples.empty()) {
    if (overlap == Overlap::SkipPlaying) {
      const Clip* clip = &it->second;
      const bool playing = std::any_of(voices_.begin(), voices_.end(),
          [clip, this](const Voice& v) {
            return v.clip == clip && v.cursor < clip->samples.size() /
                static_cast<std::size_t>(channels_);
          });
      if (playing) {
        SDL_UnlockAudioDevice(device_);
        return;
      }
      overlap = Overlap::Add;
    }
    if (overlap == Overlap::RestartPlaying) {
      const Clip* clip = &it->second;
      voices_.erase(std::remove_if(voices_.begin(), voices_.end(),
          [clip](const Voice& v) { return v.clip == clip; }), voices_.end());
      overlap = Overlap::Add;
    }
    // Exit cheers and flipper one-shots bypass the capped pool. Capped voices evict only other capped voices.
    if (overlap == Overlap::Add) {
      const auto inCappedPool = [](const Voice& v) { return !v.music && v.overlap == Overlap::Add; };
      if (std::count_if(voices_.begin(), voices_.end(), inCappedPool) >= kMaxAdditiveVoices)
        voices_.erase(std::find_if(voices_.begin(), voices_.end(), inCappedPool));
    }
    const float v = std::clamp(volume, 0.0f, 1.0f);
    gains.left = std::clamp(gains.left, 0.0f, 1.0f);
    gains.right = std::clamp(gains.right, 0.0f, 1.0f);
    voices_.push_back(Voice{&it->second, 0, v, false, false, overlap, gains});
  }
  SDL_UnlockAudioDevice(device_);
}

void Mixer::playMusic(std::string_view cue, float volume) {
  if (!ready())
    return;
  SDL_LockAudioDevice(device_);
  voices_.erase(std::remove_if(voices_.begin(), voices_.end(),
                               [](const Voice& v) { return v.music; }),
                voices_.end());
  const auto it = clips_.find(cueKey(cue));
  if (it != clips_.end() && !it->second.samples.empty()) {
    // A bed selected while muted keeps its playhead for a later live unmute.
    const float v = std::clamp(volume, 0.0f, 1.0f);
    voices_.push_back(Voice{&it->second, 0, v, true, true});
  }
  SDL_UnlockAudioDevice(device_);
}

void Mixer::stopMusic() {
  if (!ready())
    return;
  SDL_LockAudioDevice(device_);
  voices_.erase(std::remove_if(voices_.begin(), voices_.end(),
                               [](const Voice& v) { return v.music; }),
                voices_.end());
  SDL_UnlockAudioDevice(device_);
}

void Mixer::setMusicVolume(float volume) {
  if (ready()) SDL_LockAudioDevice(device_);
  musicVolume_ = std::clamp(volume, 0.0f, 1.0f);
  if (ready()) SDL_UnlockAudioDevice(device_);
}

void Mixer::setSfxVolume(float volume) {
  if (ready()) SDL_LockAudioDevice(device_);
  sfxVolume_ = std::clamp(volume, 0.0f, 1.0f);
  if (ready()) SDL_UnlockAudioDevice(device_);
}

void Mixer::stopAll() {
  if (!ready())
    return;
  SDL_LockAudioDevice(device_);
  voices_.clear();
  SDL_UnlockAudioDevice(device_);
}

void Mixer::callback(void* userdata, std::uint8_t* stream, int len) {
  auto* self = static_cast<Mixer*>(userdata);
  std::memset(stream, 0, static_cast<std::size_t>(len));
  if (!self || self->channels_ <= 0)
    return;
  const int frames = len / (static_cast<int>(sizeof(std::int16_t)) * self->channels_);
  self->fill(reinterpret_cast<std::int16_t*>(stream), frames);
}

void Mixer::fill(std::int16_t* out, int frames) {
  // Called only from the audio callback, which already holds the device lock.
  if (frames <= 0)
    return;

  const std::size_t ch = static_cast<std::size_t>(channels_);
  for (Voice& v : voices_) {
    if (!v.clip || v.overlap == Overlap::Average)
      continue;
    const auto& samples = v.clip->samples;
    const std::size_t totalFrames = samples.size() / ch;
    const float gain = v.volume * (v.music ? musicVolume_ : sfxVolume_);
    if (totalFrames == 0)
      continue;
    for (int f = 0; f < frames; ++f) {
      if (v.cursor >= totalFrames) {
        if (v.loop)
          v.cursor = 0;
        else
          break;
      }
      for (std::size_t c = 0; c < ch; ++c) {
        const double s =
            static_cast<double>(samples[v.cursor * ch + c]) * static_cast<double>(gain) *
            (ch == 1 ? (v.gains.left + v.gains.right)*0.5f :
                       (c == 0 ? v.gains.left : v.gains.right));
        const double mixed = static_cast<double>(out[static_cast<std::size_t>(f) * ch + c]) + s;
        out[static_cast<std::size_t>(f) * ch + c] =
            static_cast<std::int16_t>(std::clamp(mixed, -32768.0, 32767.0));
      }
      ++v.cursor;
    }
  }

  // Average opt-in exit cheers per frame so ended voices stop attenuating the others immediately.
  const bool hasExitVoices = std::any_of(voices_.begin(), voices_.end(),
      [](const Voice& v) { return v.overlap == Overlap::Average; });
  for (int f = 0; hasExitVoices && f < frames; ++f) {
    OverlapMix mix;
    for (Voice& v : voices_) {
      if (!v.clip || v.overlap != Overlap::Average)
        continue;
      const auto& samples = v.clip->samples;
      if (v.cursor >= samples.size() / ch)
        continue;
      mix.add(std::span<const std::int16_t>(samples).subspan(v.cursor * ch, ch), v.volume * sfxVolume_,
              v.gains.left, v.gains.right);
      ++v.cursor;
    }
    mix.mixInto(std::span<std::int16_t>(out + static_cast<std::size_t>(f) * ch, ch));
  }

  voices_.erase(std::remove_if(voices_.begin(), voices_.end(),
                               [ch](const Voice& v) {
                                 if (!v.clip)
                                   return true;
                                 if (v.loop)
                                   return false;
                                 return v.cursor >= v.clip->samples.size() / ch;
                               }),
                voices_.end());
}
#endif

}  // namespace webo::audio
