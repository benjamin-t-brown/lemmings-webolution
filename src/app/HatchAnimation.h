#pragma once

#include "assets/SceneNode.h"
#include <expected>
#include <span>
#include <string>

namespace webo::app {
// Transform tracks retained by the Object 09 auxiliary reader. Key time is
// the END of the interval to the following key (0x477130), not its start.
struct HatchAnimation {
  struct Key {
    double end = 0;
    std::array<double, 3> scale{}, position{};
    std::array<double, 4> quaternion{};  // w, x, y, z (0x4817DA)
    bool linear = true;
  };
  struct Track {
    std::size_t node = 0;
    std::vector<Key> keys;
  };
  double duration = 0;
  std::vector<Track> tracks;
};
std::expected<HatchAnimation, std::string> readHatchAnimation(
    std::span<const std::uint8_t>);
void applyHatchAnimation(SceneNode&, const HatchAnimation&, double progress);
}  // namespace webo::app
