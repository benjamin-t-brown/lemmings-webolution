#include "app/HatchAnimation.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <stdexcept>

namespace webo::app {
std::expected<HatchAnimation, std::string> readHatchAnimation(
    std::span<const std::uint8_t> bytes) {
  std::size_t at = 0;
  const auto integer = [&](std::size_t size) {
    if (size > bytes.size() - at)
      throw std::invalid_argument("truncated hatch transform track");
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < size; ++i)
      value |= std::uint64_t(bytes[at++]) << (8 * i);
    return value;
  };
  const auto number = [&] {
    const double value = std::bit_cast<double>(integer(8));
    if (!std::isfinite(value))
      throw std::invalid_argument("nonfinite hatch transform key");
    return value;
  };
  try {
    HatchAnimation animation;
    animation.duration = number();
    if (animation.duration <= 0)
      throw std::invalid_argument("nonpositive hatch animation duration");
    const auto count = integer(4);
    if (count > (bytes.size() - at) / 8)
      throw std::invalid_argument("invalid hatch track count");
    for (std::size_t i = 0; i < count; ++i) {
      HatchAnimation::Track track;
      track.node = integer(4);
      const auto keys = integer(4);
      if (!keys || keys > (bytes.size() - at) / 89)
        throw std::invalid_argument("invalid hatch key count");
      double previous = 0;
      for (std::size_t k = 0; k < keys; ++k) {
        HatchAnimation::Key key;
        key.end = number();
        if (key.end < previous)
          throw std::invalid_argument("unordered hatch keys");
        previous = key.end;
        for (auto& x : key.scale)
          x = number();
        for (auto& x : key.position)
          x = number();
        double norm = 0;
        for (auto& x : key.quaternion) {
          x = number();
          norm += x * x;
        }
        if (norm <= 1e-20)
          throw std::invalid_argument("zero hatch quaternion");
        key.linear = integer(1) != 0;
        if (!key.linear)
          throw std::invalid_argument("unsupported spherical hatch key");
        track.keys.push_back(key);
      }
      animation.tracks.push_back(std::move(track));
    }
    // Hatch scope only: colour/event tracks remain with their existing reader.
    if (integer(4) != 0 || integer(4) != 0)
      throw std::invalid_argument("unsupported hatch colour/event tracks");
    return animation;
  } catch (const std::invalid_argument& e) {
    return std::unexpected(e.what());
  }
}

void applyHatchAnimation(SceneNode& root, const HatchAnimation& animation,
                         double progress) {
  std::vector<SceneNode*> nodes;
  const auto collect = [&](auto& self, SceneNode& node) -> void {
    nodes.push_back(&node);
    for (auto& c : node.children)
      self(self, c);
  };
  collect(collect, root);
  const double time = std::clamp(progress, 0.0, 1.0) * animation.duration;
  for (const auto& track : animation.tracks) {
    if (track.node >= nodes.size() || track.keys.empty())
      continue;
    std::size_t i = 0;
    while (i + 1 < track.keys.size() && time > track.keys[i].end)
      ++i;
    const auto& a = track.keys[i];
    const auto& b = track.keys[std::min(i + 1, track.keys.size() - 1)];
    const double start = i ? track.keys[i - 1].end : 0.0;
    const double t = &a == &b || a.end <= start
                         ? 0.0
                         : std::clamp((time - start) / (a.end - start), 0.0, 1.0);
    auto q = a.quaternion;
    double dot = 0;
    for (int k = 0; k < 4; ++k)
      dot += a.quaternion[k] * b.quaternion[k];
    // PROVISIONAL numerical adapter: shortest-arc normalized lerp for the
    // installed linear door keys. Native 0x476B40 also has a spherical branch.
    for (int k = 0; k < 4; ++k)
      q[k] = (1 - t) * a.quaternion[k] + t * b.quaternion[k] * (dot < 0 ? -1 : 1);
    double norm = 0;
    for (double x : q)
      norm += x * x;
    const double f = 2.0 / norm, w = q[0], x = q[1], y = q[2], z = q[3];
    auto& m = nodes[track.node]->matrix;
    // 0x4817DA, row-vector rotation; 0x4764C4 then scales rows and
    // replaces the translation using the interpolated key vectors.
    m = {1 - f * (y * y + z * z),
         f * (x * y + w * z),
         f * (x * z - w * y),
         f * (x * y - w * z),
         1 - f * (x * x + z * z),
         f * (y * z + w * x),
         f * (x * z + w * y),
         f * (y * z - w * x),
         1 - f * (x * x + y * y),
         0,
         0,
         0};
    for (int k = 0; k < 3; ++k) {
      const double scale = a.scale[k] + t * (b.scale[k] - a.scale[k]);
      for (int j = 0; j < 3; ++j)
        m[3 * k + j] *= scale;
      m[9 + k] = a.position[k] + t * (b.position[k] - a.position[k]);
    }
  }
}
}  // namespace webo::app
