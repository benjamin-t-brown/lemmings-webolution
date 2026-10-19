#include "Raster.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace webo::app {
namespace {
constexpr double kDegenerateArea = 1e-12;

std::uint8_t clamp8(double v) {
  if (v <= 0.0) return 0;
  if (v >= 255.0) return 255;
  return static_cast<std::uint8_t>(std::lround(v));
}
}  // namespace

Raster::Raster(int width, int height)
    : width_(std::max(1, width)), height_(std::max(1, height)) {
  color_.width = width_;
  color_.height = height_;
  color_.rgb.assign(static_cast<std::size_t>(width_) * height_ * 3, 0);
  depth_.assign(static_cast<std::size_t>(width_) * height_, std::numeric_limits<double>::infinity());
}

void Raster::clear(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
  for (std::size_t i = 0; i < color_.rgb.size(); i += 3) {
    color_.rgb[i] = r;
    color_.rgb[i + 1] = g;
    color_.rgb[i + 2] = b;
  }
  std::fill(depth_.begin(), depth_.end(), std::numeric_limits<double>::infinity());
}

void Raster::triangle(const std::array<std::array<double, 4>, 3>& p,
                      const std::array<std::array<double, 2>, 3>& uv, const Texture& texture,
                      const std::array<double, 3>& modulate) {
  if (texture.width <= 0 || texture.height <= 0 || texture.rgb.empty())
    return;

  const auto& a = p[0];
  const auto& b = p[1];
  const auto& c = p[2];
  const double area = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]);
  if (std::abs(area) < kDegenerateArea || !std::isfinite(area))
    return;

  const int xmin = std::max(0, static_cast<int>(std::floor(std::min({a[0], b[0], c[0]}))));
  const int xmax = std::min(width_ - 1, static_cast<int>(std::ceil(std::max({a[0], b[0], c[0]}))));
  const int ymin = std::max(0, static_cast<int>(std::floor(std::min({a[1], b[1], c[1]}))));
  const int ymax = std::min(height_ - 1, static_cast<int>(std::ceil(std::max({a[1], b[1], c[1]}))));

  const int tw = texture.width;
  const int th = texture.height;
  std::uint8_t* dst = color_.rgb.data();
  const double mr = modulate[0], mg = modulate[1], mb = modulate[2];

  for (int y = ymin; y <= ymax; ++y) {
    const std::size_t rowAt = static_cast<std::size_t>(y) * width_;
    for (int x = xmin; x <= xmax; ++x) {
      const double xx = x + 0.5;
      const double yy = y + 0.5;
      const double w0 = ((b[0] - xx) * (c[1] - yy) - (b[1] - yy) * (c[0] - xx)) / area;
      const double w1 = ((c[0] - xx) * (a[1] - yy) - (c[1] - yy) * (a[0] - xx)) / area;
      const double w2 = 1.0 - w0 - w1;
      if (std::min({w0, w1, w2}) < 0.0)
        continue;
      const double q = w0 * a[3] + w1 * b[3] + w2 * c[3];
      if (q <= 0.0 || !std::isfinite(q))
        continue;
      const double z = w0 * a[2] + w1 * b[2] + w2 * c[2];
      const std::size_t pos = rowAt + static_cast<std::size_t>(x);
      if (depthEnabled_ && z >= depth_[pos])
        continue;
      const double u = (w0 * uv[0][0] * a[3] + w1 * uv[1][0] * b[3] + w2 * uv[2][0] * c[3]) / q;
      const double v = (w0 * uv[0][1] * a[3] + w1 * uv[1][1] * b[3] + w2 * uv[2][1] * c[3]) / q;

      double sx = (u - std::floor(u)) * tw - 0.5;
      double sy = (v - std::floor(v)) * th - 0.5;
      int ix = static_cast<int>(std::floor(sx));
      int iy = static_cast<int>(std::floor(sy));
      const double dx = sx - ix;
      const double dy = sy - iy;

      double acc[3] = {0.0, 0.0, 0.0};
      const double wx[2] = {1.0 - dx, dx};
      const double wy[2] = {1.0 - dy, dy};
      for (int j = 0; j < 2; ++j) {
        int ty = (iy + j) % th;
        if (ty < 0) ty += th;
        for (int i = 0; i < 2; ++i) {
          int tx = (ix + i) % tw;
          if (tx < 0) tx += tw;
          const double w = wx[i] * wy[j];
          const std::uint8_t* texel =
              texture.rgb.data() + (static_cast<std::size_t>(ty) * tw + tx) * 3;
          acc[0] += texel[0] * w;
          acc[1] += texel[1] * w;
          acc[2] += texel[2] * w;
        }
      }
      const std::size_t o = pos * 3;
      dst[o] = clamp8(acc[0] * mr);
      dst[o + 1] = clamp8(acc[1] * mg);
      dst[o + 2] = clamp8(acc[2] * mb);
      depth_[pos] = z;
      ++pixels_;
    }
  }
  ++triangles_;
}

}  // namespace webo::app
