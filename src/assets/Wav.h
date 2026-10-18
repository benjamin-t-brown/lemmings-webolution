// The game's sound, as RIFF/WAVE.
//
// Unlike the textures and levels, this format is not custom: the 73 `.WAV`
// entries are ordinary RIFF PCM and any player opens them. What is *not*
// ordinary is how much they vary. An earlier note in this project described
// them as "22050 Hz", which is true of 38 of the 73 and wrong about the rest:
//
//   sample rates   3179 to 44100, and seventeen distinct values
//   bit depth      8 and 16
//   channels       mono and stereo
//
// Several of the odd rates (7418, 5563, 8363, 3709) are tracker-era sample
// rates, so these are plainly converted assets rather than anything recorded
// to a house standard. Any mixer therefore has to resample and convert rather
// than assume, which is the reason this header reports the format instead of
// hiding it.
//
// Chunk order is not assumed either. Most files are `fmt ` then `data`, but
// two carry a trailing `smpl` and one a `LIST`/`DISP` tail, so the reader
// walks the chunk list and ignores what it does not know. Odd-sized chunks
// are padded to even in RIFF, which is handled; getting that wrong
// desynchronises every chunk after the first odd one.

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

  // Decoded to signed 16-bit, interleaved, whatever the file's bit depth was.
  // 8-bit RIFF samples are *unsigned* and centred on 128, so they are shifted
  // here rather than reinterpreted -- reinterpreting them is the classic way
  // to turn quiet audio into a loud buzz.
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

// Converts a sound to a given rate and channel count.
//
// Done once at load rather than in the audio callback: the retail sounds span
// seventeen sample rates, so *something* has to resample, and doing it per
// callback would put a resampler on the real-time thread for no benefit.
//
// Linear interpolation, which is more than the source material warrants --
// several of these are 8-bit at 3-8 kHz. Mono to stereo duplicates; stereo to
// mono averages, rather than dropping a channel, so nothing panned hard to one
// side vanishes.
Sound resampleTo(const Sound& source, int sampleRate, int channels);
std::expected<Sound, std::string> loadSound(const AssetLibrary&, std::string_view name);

// Every "*.WAV" in the archive.
std::vector<std::string> soundNames(const AssetLibrary&);

// "Letsgo" -> "SOUNDS\\LETSGO.WAV". Level files name sound cues bare, the way
// they name textures bare.
std::string soundAssetName(std::string_view cue);

// "LEMMING01" -> "MUSIC\\LEMMING01.WAV".
std::string musicAssetName(std::string_view cue);

}  // namespace webo
