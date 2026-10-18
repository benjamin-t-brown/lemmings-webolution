#include "Mixer.h"

#include "assets/Box.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <cstring>
#include <format>

namespace webo::audio {
namespace {

std::string upperCue(std::string_view cue) {
  std::string out(cue);
  for (char& c : out)
    if (c >= 'a' && c <= 'z')
      c = static_cast<char>(c - 'a' + 'A');
  return out;
}

}  // namespace

Mixer::~Mixer() { close(); }

std::expected<void, std::string> Mixer::open(int sampleRate, int channels) {
  if (device_)
    close();
  if (sampleRate <= 0 || channels < 1 || channels > 2)
    return std::unexpected("mixer: bad output format");

  if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
      return std::unexpected(std::format("mixer: SDL_InitSubSystem: {}", SDL_GetError()));
  }

  SDL_AudioSpec want{};
  want.freq = sampleRate;
  want.format = AUDIO_S16SYS;
  want.channels = static_cast<std::uint8_t>(channels);
  want.samples = 1024;
  want.callback = callback;
  want.userdata = this;

  SDL_AudioSpec have{};
  // Exact format: a negotiated rate would need a second resampler in the
  // callback, which is what load-time resampling exists to avoid.
  const SDL_AudioDeviceID id = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
  if (id == 0)
    return std::unexpected(std::format("mixer: SDL_OpenAudioDevice: {}", SDL_GetError()));

  if (have.freq != sampleRate || have.channels != channels || have.format != AUDIO_S16SYS) {
    SDL_CloseAudioDevice(id);
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

void Mixer::close() {
  if (!device_)
    return;
  // Closing the device stops the callback before we free anything it might
  // have been reading.
  SDL_CloseAudioDevice(device_);
  device_ = 0;
  voices_.clear();
  clips_.clear();
  sampleRate_ = 0;
  channels_ = 0;
}

std::expected<void, std::string> Mixer::load(std::string_view cue, const Sound& source) {
  if (!ready())
    return std::unexpected("mixer: not open");
  if (source.empty())
    return std::unexpected("mixer: empty sound");

  Sound mixed = resampleTo(source, sampleRate_, channels_);
  if (mixed.empty())
    return std::unexpected("mixer: resample produced nothing");

  const std::string key = upperCue(cue);
  Clip clip;
  clip.samples = std::move(mixed.samples);

  SDL_LockAudioDevice(device_);
  // Drop voices that still point at the clip we are about to replace -- the
  // pointer would otherwise dangle the moment the map entry is overwritten.
  if (const auto it = clips_.find(key); it != clips_.end()) {
    const Clip* dying = &it->second;
    voices_.erase(std::remove_if(voices_.begin(), voices_.end(),
                                 [dying](const Voice& v) { return v.clip == dying; }),
                  voices_.end());
  }
  clips_[key] = std::move(clip);
  SDL_UnlockAudioDevice(device_);
  return {};
}

std::expected<void, std::string> Mixer::loadFrom(const AssetLibrary& lib, std::string_view cue) {
  auto sound = loadSound(lib, soundAssetName(cue));
  if (!sound)
    return std::unexpected(sound.error());
  return load(cue, *sound);
}

bool Mixer::has(std::string_view cue) const {
  if (!ready())
    return false;
  SDL_LockAudioDevice(device_);
  const bool found = clips_.contains(upperCue(cue));
  SDL_UnlockAudioDevice(device_);
  return found;
}

void Mixer::play(std::string_view cue, float volume) {
  if (!ready() || volume <= 0.0f || sfxVolume_ <= 0.0f)
    return;

  SDL_LockAudioDevice(device_);
  const auto it = clips_.find(upperCue(cue));
  if (it != clips_.end() && !it->second.samples.empty()) {
    // Cap concurrent voices so a long nuke cannot grow without bound. Drop
    // the oldest non-music when full -- short SFX recover.
    constexpr std::size_t kMaxVoices = 32;
    if (voices_.size() >= kMaxVoices) {
      for (auto vit = voices_.begin(); vit != voices_.end(); ++vit) {
        if (!vit->music) {
          voices_.erase(vit);
          break;
        }
      }
    }
    const float v = std::clamp(volume, 0.0f, 1.0f) * sfxVolume_;
    voices_.push_back(Voice{&it->second, 0, v, false, false});
  }
  SDL_UnlockAudioDevice(device_);
}

std::expected<void, std::string> Mixer::loadMusicFrom(const AssetLibrary& lib,
                                                      std::string_view cue) {
  auto sound = loadSound(lib, musicAssetName(cue));
  if (!sound)
    return std::unexpected(sound.error());
  return load(cue, *sound);
}

void Mixer::playMusic(std::string_view cue, float volume) {
  if (!ready())
    return;
  SDL_LockAudioDevice(device_);
  voices_.erase(std::remove_if(voices_.begin(), voices_.end(),
                               [](const Voice& v) { return v.music; }),
                voices_.end());
  const auto it = clips_.find(upperCue(cue));
  if (it != clips_.end() && !it->second.samples.empty() && musicVolume_ > 0.0f) {
    const float v = std::clamp(volume, 0.0f, 1.0f) * musicVolume_;
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
  musicVolume_ = std::clamp(volume, 0.0f, 1.0f);
  if (!ready())
    return;
  SDL_LockAudioDevice(device_);
  for (Voice& v : voices_)
    if (v.music)
      v.volume = musicVolume_;
  SDL_UnlockAudioDevice(device_);
}

void Mixer::setSfxVolume(float volume) {
  sfxVolume_ = std::clamp(volume, 0.0f, 1.0f);
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
    if (!v.clip)
      continue;
    const auto& samples = v.clip->samples;
    const std::size_t totalFrames = samples.size() / ch;
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
            static_cast<double>(samples[v.cursor * ch + c]) * static_cast<double>(v.volume);
        const double mixed = static_cast<double>(out[static_cast<std::size_t>(f) * ch + c]) + s;
        out[static_cast<std::size_t>(f) * ch + c] =
            static_cast<std::int16_t>(std::clamp(mixed, -32768.0, 32767.0));
      }
      ++v.cursor;
    }
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

}  // namespace webo::audio
