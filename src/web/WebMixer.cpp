#include "audio/Mixer.h"

#include <algorithm>
#include <span>

extern "C" {
int weboAudioOpen(const void* id, int channels, int maxVoices, float musicVolume, float sfxVolume);
void weboAudioClose(const void* id);
int weboAudioLoad(const void* id, const char* key, const std::int16_t* pcm,
                  int frames, int channels, int sampleRate);
int weboAudioHas(const void* id, const char* key);
void weboAudioPlay(const void* id, const char* key, float volume, float left, float right,
                   int average, int skipPlaying, int restartPlaying, int addWithoutEviction);
void weboAudioPlayMusic(const void* id, const char* key, float volume);
void weboAudioStopMusic(const void* id);
void weboAudioStopAll(const void* id);
void weboAudioVolumes(const void* id, float music, float sfx);
}

namespace webo::audio {

std::expected<void, std::string> Mixer::open(int sampleRate, int channels) {
  close();
  if (sampleRate <= 0 || channels < 1 || channels > 2)
    return std::unexpected("mixer: bad output format");
  if (!weboAudioOpen(this, channels, kMaxAdditiveVoices, musicVolume_, sfxVolume_))
    return std::unexpected("mixer: cannot open Web Audio");
  webReady_ = true;
  sampleRate_ = sampleRate;
  channels_ = channels;
  return {};
}

void Mixer::close() {
  if (!ready()) return;
  weboAudioClose(this);
  webReady_ = false;
  sampleRate_ = 0;
  channels_ = 0;
}

std::expected<void, std::string> Mixer::storeClip(std::string key, Sound clip) {
  const auto channels = static_cast<std::size_t>(clip.channels);
  std::span<const std::int16_t> pcm(clip.samples);
  if (key == "ACTION") {
    // Playtest 20 K3. The installed Action clip stays below -48 dBFS for its first 53 ms.
    // Skip that lead on the web, leaving the attack and any louder variant intact.
    const auto leadSamples = static_cast<std::size_t>(sampleRate_) * 53 / 1000 * channels;
    constexpr int quietPeak = 128; // About -48 dBFS.
    if (leadSamples < pcm.size() && std::ranges::all_of(pcm.first(leadSamples),
        [](int sample) { return sample >= -quietPeak && sample <= quietPeak; })) {
      pcm = pcm.subspan(leadSamples);
    }
  }
  const auto frames = static_cast<int>(pcm.size() / channels);
  if (!weboAudioLoad(this, key.c_str(), pcm.data(), frames, clip.channels, clip.sampleRate))
    return std::unexpected("mixer: cannot load Web Audio buffer");
  return {};
}

bool Mixer::has(std::string_view cue) const {
  return ready() && weboAudioHas(this, cueKey(cue).c_str());
}

void Mixer::play(std::string_view cue, float volume, Overlap overlap, StereoGains gains) {
  if (!ready() || volume <= 0.0f || sfxVolume_ <= 0.0f) return;
  weboAudioPlay(this, cueKey(cue).c_str(), std::clamp(volume, 0.0f, 1.0f),
                std::clamp(gains.left, 0.0f, 1.0f), std::clamp(gains.right, 0.0f, 1.0f),
                overlap == Overlap::Average, overlap == Overlap::SkipPlaying,
                overlap == Overlap::RestartPlaying, overlap == Overlap::AddWithoutEviction);
}

void Mixer::playMusic(std::string_view cue, float volume) {
  if (ready()) weboAudioPlayMusic(this, cueKey(cue).c_str(), std::clamp(volume, 0.0f, 1.0f));
}

void Mixer::stopMusic() {
  if (ready()) weboAudioStopMusic(this);
}

void Mixer::setMusicVolume(float volume) {
  musicVolume_ = std::clamp(volume, 0.0f, 1.0f);
  if (ready()) weboAudioVolumes(this, musicVolume_, sfxVolume_);
}

void Mixer::setSfxVolume(float volume) {
  sfxVolume_ = std::clamp(volume, 0.0f, 1.0f);
  if (ready()) weboAudioVolumes(this, musicVolume_, sfxVolume_);
}

void Mixer::stopAll() {
  if (ready()) weboAudioStopAll(this);
}

}  // namespace webo::audio
