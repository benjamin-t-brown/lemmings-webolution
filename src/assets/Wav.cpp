#include "Wav.h"

#include "Box.h"

#include <algorithm>
#include <cstring>
#include <format>

namespace webo {
namespace {

std::uint32_t read32(std::span<const std::byte> d, std::size_t at) {
  std::uint32_t v = 0;
  std::memcpy(&v, d.data() + at, sizeof v);
  return v;
}

std::uint16_t read16(std::span<const std::byte> d, std::size_t at) {
  std::uint16_t v = 0;
  std::memcpy(&v, d.data() + at, sizeof v);
  return v;
}

bool tagIs(std::span<const std::byte> d, std::size_t at, const char (&tag)[5]) {
  for (int i = 0; i < 4; ++i)
    if (static_cast<char>(d[at + i]) != tag[i])
      return false;
  return true;
}

constexpr std::uint16_t kPcm = 1;

}  // namespace

std::expected<Sound, std::string> decodeWav(std::span<const std::byte> data) {
  if (data.size() < 12)
    return std::unexpected("wav: too short to be RIFF");
  if (!tagIs(data, 0, "RIFF") || !tagIs(data, 8, "WAVE"))
    return std::unexpected("wav: not a RIFF/WAVE file");

  std::uint16_t format = 0;
  std::uint16_t channels = 0;
  std::uint32_t sampleRate = 0;
  std::uint16_t bits = 0;
  std::span<const std::byte> pcm;
  bool haveFmt = false;

  // Walk the chunk list rather than assuming an order. Chunks are padded to
  // an even size in RIFF, and skipping that padding is what keeps every chunk
  // after the first odd one aligned.
  std::size_t at = 12;
  while (at + 8 <= data.size()) {
    const std::uint32_t size = read32(data, at + 4);
    const std::size_t body = at + 8;
    if (size > data.size() - body)
      break;  // a truncated tail; keep whatever was already found

    if (tagIs(data, at, "fmt ") && size >= 16) {
      format = read16(data, body);
      channels = read16(data, body + 2);
      sampleRate = read32(data, body + 4);
      bits = read16(data, body + 14);
      haveFmt = true;
    } else if (tagIs(data, at, "data")) {
      pcm = data.subspan(body, size);
    }

    at = body + size + (size & 1u);
  }

  if (!haveFmt)
    return std::unexpected("wav: no fmt chunk");
  if (format != kPcm)
    return std::unexpected(std::format("wav: format {} is not PCM", format));
  if (channels < 1 || channels > 2)
    return std::unexpected(std::format("wav: {} channels is not mono or stereo", channels));
  if (sampleRate == 0 || sampleRate > 192000)
    return std::unexpected(std::format("wav: implausible sample rate {}", sampleRate));
  if (bits != 8 && bits != 16)
    return std::unexpected(std::format("wav: {}-bit samples are not supported", bits));
  if (pcm.empty())
    return std::unexpected("wav: no data chunk");

  Sound sound;
  sound.channels = channels;
  sound.sampleRate = static_cast<int>(sampleRate);

  if (bits == 8) {
    // 8-bit RIFF samples are unsigned and centred on 128. Shifting rather
    // than reinterpreting matters: reinterpreting turns quiet audio into a
    // loud buzz, which is audible but easy to mistake for a bad asset.
    sound.samples.resize(pcm.size());
    for (std::size_t i = 0; i < pcm.size(); ++i) {
      const int v = static_cast<int>(std::to_integer<std::uint8_t>(pcm[i])) - 128;
      sound.samples[i] = static_cast<std::int16_t>(v * 256);
    }
  } else {
    const std::size_t count = pcm.size() / 2;
    sound.samples.resize(count);
    std::memcpy(sound.samples.data(), pcm.data(), count * 2);
  }

  // A partial final frame would desynchronise the channels, so drop it.
  const std::size_t perFrame = static_cast<std::size_t>(channels);
  sound.samples.resize(sound.samples.size() - sound.samples.size() % perFrame);
  return sound;
}

Sound resampleTo(const Sound& source, int sampleRate, int channels) {
  Sound out;
  if (sampleRate <= 0 || channels < 1 || channels > 2 || source.empty() ||
      source.sampleRate <= 0 || source.channels < 1)
    return out;

  out.sampleRate = sampleRate;
  out.channels = channels;

  const std::size_t inFrames = source.frameCount();
  if (inFrames == 0)
    return out;

  const double ratio = static_cast<double>(sampleRate) / source.sampleRate;
  const auto outFrames =
      static_cast<std::size_t>(static_cast<double>(inFrames) * ratio + 0.5);
  if (outFrames == 0)
    return out;
  out.samples.resize(outFrames * static_cast<std::size_t>(channels));

  // Reads one source channel of a frame, mixing down when the source has more
  // channels than the destination.
  const auto sampleAt = [&](std::size_t frame, int channel) -> double {
    frame = std::min(frame, inFrames - 1);
    const std::size_t base = frame * static_cast<std::size_t>(source.channels);
    if (source.channels == channels)
      return source.samples[base + static_cast<std::size_t>(channel)];
    if (source.channels == 1)
      return source.samples[base];  // mono fans out to every destination channel
    // Stereo down to mono: average, so a hard-panned sound does not vanish.
    return 0.5 * (static_cast<double>(source.samples[base]) +
                  static_cast<double>(source.samples[base + 1]));
  };

  for (std::size_t f = 0; f < outFrames; ++f) {
    const double src = static_cast<double>(f) / ratio;
    const auto i0 = static_cast<std::size_t>(src);
    const double t = src - static_cast<double>(i0);
    for (int c = 0; c < channels; ++c) {
      const double a = sampleAt(i0, c);
      const double b = sampleAt(i0 + 1, c);
      const double v = a + (b - a) * t;
      const double clamped = std::clamp(v, -32768.0, 32767.0);
      out.samples[f * static_cast<std::size_t>(channels) + static_cast<std::size_t>(c)] =
          static_cast<std::int16_t>(clamped);
    }
  }
  return out;
}

std::expected<Sound, std::string> loadSound(const AssetLibrary& lib, std::string_view name) {
  auto blob = lib.read(name);
  if (!blob)
    return std::unexpected(blob.error());
  return decodeWav(*blob);
}

std::vector<std::string> soundNames(const AssetLibrary& lib) {
  std::vector<std::string> out;
  for (const auto& name : lib.names())
    if (name.size() > 4 && normaliseAssetName(name).ends_with(".WAV"))
      out.push_back(name);
  return out;
}

std::string soundAssetName(std::string_view cue) {
  std::string out = "SOUNDS\\";
  out += cue;
  out += ".WAV";
  return normaliseAssetName(out);
}

}  // namespace webo
