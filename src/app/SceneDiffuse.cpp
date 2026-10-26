#include "app/SceneDiffuse.h"

#include <cmath>
#include <limits>

namespace webo::app {
namespace {
bool asTruncatedI32(double value, std::int32_t& out) {
  if (!std::isfinite(value) || value < static_cast<double>(INT32_MIN) ||
      value >= -static_cast<double>(INT32_MIN)) return false;
  out = static_cast<std::int32_t>(value);
  return true;
}
}

std::expected<std::uint32_t, std::string> packSceneDiffuseWord(
    const std::array<double, 4>& raw) {
  std::array<std::int32_t, 4> channel{};
  for (std::size_t i = 0; i < channel.size(); ++i)
    if (!asTruncatedI32(raw[i], channel[i]))
      return std::unexpected("diffuse channel outside finite signed-32 domain");
  return (static_cast<std::uint32_t>(channel[3]) << 24) |
         (static_cast<std::uint32_t>(channel[0]) << 16) |
         (static_cast<std::uint32_t>(channel[1]) << 8) |
         static_cast<std::uint32_t>(channel[2]);
}

std::expected<SceneRasterVertex, std::string> adaptSceneMeshVertex(
    double x, double y, double z, double rhw, double u, double v,
    const std::array<double, 4>& raw) {
  auto word = packSceneDiffuseWord(raw);
  if (!word) return std::unexpected(word.error());
  const auto packed = *word;
  return SceneRasterVertex{x, y, z, rhw,
      {static_cast<double>((packed >> 16) & 0xffu),
       static_cast<double>((packed >> 8) & 0xffu),
       static_cast<double>(packed & 0xffu),
       static_cast<double>(packed >> 24)}, u, v};
}

}  // namespace webo::app
