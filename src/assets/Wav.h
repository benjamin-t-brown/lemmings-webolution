// Retail .WAV entries use RIFF/WAVE PCM, mono or stereo, at 8 or 16 bits and 3179..44100 Hz.
// Walk chunks in file order, skip unsupported chunks (e.g. `smpl`, `LIST` or `DISP`)
// and account for even-byte padding. `fmt ` and `data` need not be adjacent.

#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace webo {

class AssetLibrary;

struct Sound {
  int sampleRate = 0;
  int channels = 0;

  // Interleaved signed 16-bit samples. Convert unsigned 8-bit input by subtracting 128 and scaling.
  std::vector<std::int16_t> samples;

  std::size_t frameCount() const {
    return channels > 0 ? samples.size() / static_cast<std::size_t>(channels) : 0;
  }
  double seconds() const {
    return sampleRate > 0 ? static_cast<double>(frameCount()) / sampleRate : 0.0;
  }
  bool empty() const { return samples.empty(); }
};

std::expected<Sound, std::string> decodeWav(std::span<const std::byte>);

// Convert at load time to keep resampling out of the audio callback. Use linear interpolation,
// duplicate mono channels and average stereo to mono.
Sound resampleTo(const Sound& source, int sampleRate, int channels);
std::expected<Sound, std::string> loadSound(const AssetLibrary&, std::string_view name);

// Every "*.WAV" in the archive.
std::vector<std::string> soundNames(const AssetLibrary&);

// Cue names are bare: "Letsgo" -> "SOUNDS\\LETSGO.WAV".
std::string soundAssetName(std::string_view cue);

// "LEMMING01" -> "MUSIC\\LEMMING01.WAV".
std::string musicAssetName(std::string_view cue);

}  // namespace webo
