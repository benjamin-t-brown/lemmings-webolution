// Provisional clone policy for exit cheers: average concurrent voices, then
// fit that bus into the headroom left by existing effects. No hard clipping,
// allocation or device access. This does not model the retail DirectSound mix.
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

namespace webo::audio {

class OverlapMix {
public:
  // One interleaved frame (mono or stereo), with the voice's existing gain.
  void add(std::span<const std::int16_t> frame, float volume,
           float left = 1.0f, float right = 1.0f) {
    for (std::size_t c = 0; c < frame.size(); ++c)
      sum_[c] += static_cast<double>(frame[c]) * volume *
          (frame.size() == 1 ? (left + right)*0.5f : (c == 0 ? left : right));
    ++voices_;
  }

  void mixInto(std::span<std::int16_t> frame) const {
    if (voices_ == 0)
      return;
    // One gain for both channels preserves the stereo image. Existing sounds
    // keep their samples; only the new bus is attenuated if room is limited.
    double gain = 1.0;
    for (std::size_t c = 0; c < frame.size(); ++c) {
      const double sample = sum_[c] / voices_;
      if (sample > 0)
        gain = std::min(gain, (32767.0 - frame[c]) / sample);
      else if (sample < 0)
        gain = std::min(gain, (-32768.0 - frame[c]) / sample);
    }
    for (std::size_t c = 0; c < frame.size(); ++c)
      frame[c] = static_cast<std::int16_t>(
          std::clamp(frame[c] + gain * (sum_[c] / voices_), -32768.0, 32767.0));
  }

private:
  std::array<double, 2> sum_{};
  unsigned voices_ = 0;
};

}  // namespace webo::audio
