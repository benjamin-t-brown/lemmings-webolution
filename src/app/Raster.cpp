#include "app/FrameProfile.h"
#include "Raster.h"
#include "app/RasterRows.h"

#include <algorithm>
#include <cmath>
#include <cstring>
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
  if (r == g && r == b) {
    std::fill(color_.rgb.begin(), color_.rgb.end(), r);
  } else {
    for (std::size_t i = 0; i < color_.rgb.size(); i += 3) {
      color_.rgb[i] = r;
      color_.rgb[i + 1] = g;
      color_.rgb[i + 2] = b;
    }
  }
  std::fill(depth_.begin(), depth_.end(), std::numeric_limits<double>::infinity());
  actorDepth_.clear();
}

void Raster::compositeOpaque(const Raster& layer) {
  if (width_ != layer.width_ || height_ != layer.height_) return;
  for (int y = 0; y < height_; ++y) {
    const auto end = std::size_t(y + 1) * width_;
    auto p = std::size_t(y) * width_;
    while (p < end) {
      if (!(layer.depth_[p] < depth_[p])) { ++p; continue; }
      const auto first = p++;
      while (p < end && layer.depth_[p] < depth_[p]) ++p;
      std::copy(layer.depth_.begin()+first, layer.depth_.begin()+p, depth_.begin()+first);
      std::memcpy(color_.rgb.data()+first*3, layer.color_.rgb.data()+first*3, (p-first)*3);
      pixels_ += p-first;
    }
  }
  triangles_ += layer.triangles_;
}

void Raster::triangle(const std::array<std::array<double, 4>, 3>& p,
                      const std::array<std::array<double, 2>, 3>& uv, const Texture& texture,
                      const std::array<double, 3>& modulate, std::size_t actorOwner,
                      SceneAddress address) {
  profile::Scope profileRaster(profile::active &&
      (actorOwner != ActorDepth::noOwner || profile::active->cost == profile::BuilderBrick) ?
      profile::active->cost : profile::TubeRaster);
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
  const bool wrap = address == SceneAddress::Wrap;

  const auto drawRows = [this, p, uv, &texture, actorOwner, area, xmin, xmax, tw, th, dst, mr, mg, mb, wrap](int first, int last) {
    const auto& a = p[0]; const auto& b = p[1]; const auto& c = p[2];
    // The normalized coordinate bounds the bilinear taps to [-1, size].
    const auto addressTap = [wrap](int i, int size) {
      if (!wrap) return std::clamp(i, 0, size - 1);
      return i < 0 ? i + size : i >= size ? i - size : i;
    };
    std::size_t count = 0;
    for (int y = first; y <= last; ++y) {
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
        if ((depthEnabled_ || actorOwner != ActorDepth::noOwner) && z >= depth_[pos])
          continue;
        if (actorOwner != ActorDepth::noOwner && !actorDepth_.visible(pos, z, actorOwner))
          continue;
        const double u = (w0 * uv[0][0] * a[3] + w1 * uv[1][0] * b[3] + w2 * uv[2][0] * c[3]) / q;
        const double v = (w0 * uv[0][1] * a[3] + w1 * uv[1][1] * b[3] + w2 * uv[2][1] * c[3]) / q;

        const double sx = (wrap ? u - std::floor(u) : std::clamp(u, 0.0, 1.0)) * tw - 0.5;
        const double sy = (wrap ? v - std::floor(v) : std::clamp(v, 0.0, 1.0)) * th - 0.5;
        int ix = static_cast<int>(std::floor(sx));
        int iy = static_cast<int>(std::floor(sy));
        const double dx = sx - ix;
        const double dy = sy - iy;

        double acc[3] = {0.0, 0.0, 0.0};
        const double wx[2] = {1.0 - dx, dx};
        const double wy[2] = {1.0 - dy, dy};
        const int xs[]{addressTap(ix, tw), addressTap(ix + 1, tw)};
        const int ys[]{addressTap(iy, th), addressTap(iy + 1, th)};
        for (int j = 0; j < 2; ++j) {
          const int ty = ys[j];
          for (int i = 0; i < 2; ++i) {
            const int tx = xs[i];
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
        if (actorOwner == ActorDepth::noOwner) depth_[pos] = z;
        else actorDepth_.submit(pos, z, actorOwner);
        ++count;
      }
    }
    return count;
  };
  if (actorOwner == ActorDepth::noOwner && detail::commandBatch) {
    detail::commandBatch->add(ymin, ymax, [drawRows, ymin, ymax](int first, int last, std::size_t& pixels, std::string&) {
      pixels = drawRows(std::max(first, ymin), std::min(last, ymax));
    }, [this](std::size_t pixels) { pixels_ += pixels; ++triangles_; });
    return;
  }
  pixels_ += drawRows(ymin, ymax);
  ++triangles_;
}

}  // namespace webo::app
