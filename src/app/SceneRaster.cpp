#include "app/SceneRaster.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

namespace webo::app {
namespace {

bool dimensionsValid(int w, int h, std::size_t channels, std::size_t size) {
  if (w <= 0 || h <= 0) return false;
  const auto uw = static_cast<std::size_t>(w), uh = static_cast<std::size_t>(h);
  return uw <= std::numeric_limits<std::size_t>::max() / uh &&
         uw * uh <= std::numeric_limits<std::size_t>::max() / channels &&
         size == uw * uh * channels;
}

bool validFactor(SceneBlendFactor f) {
  return f == SceneBlendFactor::One || f == SceneBlendFactor::SrcAlpha ||
         f == SceneBlendFactor::InvSrcAlpha;
}

std::uint8_t unorm8(double v) {
  if (!(v > 0.0)) return 0;
  if (v >= 255.0) return 255;
  return static_cast<std::uint8_t>(std::floor(v + 0.5));
}

int addressIndex(double i, int n, SceneAddress mode) {
  if (mode == SceneAddress::Clamp)
    return static_cast<int>(std::clamp(i, 0.0, static_cast<double>(n - 1)));
  double r = std::fmod(i, static_cast<double>(n));
  if (r < 0) r += n;
  return static_cast<int>(r);
}

std::array<double, 4> sample(const SceneRgbaImage& image, double u, double v,
                             SceneFilter filter, SceneAddress address) {
  if (filter == SceneFilter::Point) {
    const int x = addressIndex(std::floor(u * image.width), image.width, address);
    const int y = addressIndex(std::floor(v * image.height), image.height, address);
    const auto at = (static_cast<std::size_t>(y) * image.width + x) * 4;
    return {double(image.rgba[at]), double(image.rgba[at + 1]),
            double(image.rgba[at + 2]), double(image.rgba[at + 3])};
  }
  const double fx = u * image.width - 0.5, fy = v * image.height - 0.5;
  const double floorX = std::floor(fx), floorY = std::floor(fy);
  // UV inputs are range checked to keep these conversions representable.
  const double x0 = floorX, y0 = floorY;
  const double tx = fx - floorX, ty = fy - floorY;
  std::array<double, 4> out{};
  for (int j = 0; j < 2; ++j) for (int i = 0; i < 2; ++i) {
    const int x = addressIndex(x0 + i, image.width, address);
    const int y = addressIndex(y0 + j, image.height, address);
    const auto at = (static_cast<std::size_t>(y) * image.width + x) * 4;
    const double weight = (i ? tx : 1.0 - tx) * (j ? ty : 1.0 - ty);
    for (int c = 0; c < 4; ++c) out[c] += image.rgba[at + c] * weight;
  }
  return out;
}

double factorValue(SceneBlendFactor f, double sourceAlpha) {
  if (f == SceneBlendFactor::One) return 1.0;
  const double a = sourceAlpha / 255.0;
  return f == SceneBlendFactor::SrcAlpha ? a : 1.0 - a;
}

bool depthPass(SceneDepthFunction f, double source, double destination) {
  switch (f) {
    case SceneDepthFunction::Never: return false;
    case SceneDepthFunction::Less: return source < destination;
    case SceneDepthFunction::LessEqual: return source <= destination;
    case SceneDepthFunction::Equal: return source == destination;
    case SceneDepthFunction::GreaterEqual: return source >= destination;
    case SceneDepthFunction::Greater: return source > destination;
    case SceneDepthFunction::NotEqual: return source != destination;
    case SceneDepthFunction::Always: return true;
  }
  return false;
}

}  // namespace

std::expected<void, std::string> rasterizeSceneTriangle(
    SceneRgbaFramebuffer& target, std::vector<double>& depth,
    const SceneRgbaImage& texture, const std::array<SceneRasterVertex, 3>& p,
    const SceneRasterOptions& o, const SceneRasterObserver* observer) {
  if (!dimensionsValid(target.width, target.height, 4, target.rgba.size()) ||
      depth.size() != static_cast<std::size_t>(target.width) * target.height)
    return std::unexpected("invalid target image/depth dimensions or storage");
  if (observer && (!observer->mask || observer->mask->size() != depth.size() || !observer->accepted))
    return std::unexpected("invalid masked raster observer");
  if (!dimensionsValid(texture.width, texture.height, 4, texture.rgba.size()))
    return std::unexpected("invalid RGBA8 texture dimensions or storage");
  if ((o.filter != SceneFilter::Point && o.filter != SceneFilter::Linear) ||
      (o.address != SceneAddress::Wrap && o.address != SceneAddress::Clamp) ||
      (o.alphaFunction != SceneAlphaFunction::Disabled && o.alphaFunction != SceneAlphaFunction::StrictGreater) ||
      !validFactor(o.sourceBlend) || !validFactor(o.destinationBlend) ||
      (o.depthFunction < SceneDepthFunction::Never || o.depthFunction > SceneDepthFunction::Always) ||
      (o.shade != SceneShade::Flat && o.shade != SceneShade::Gouraud) ||
      o.rgbOperation != SceneRgbOperation::Modulate ||
      (o.alphaOperation != SceneAlphaOperation::SelectTexture && o.alphaOperation != SceneAlphaOperation::Modulate) ||
      !std::isfinite(o.sampleOffsetX) || !std::isfinite(o.sampleOffsetY) ||
      o.sampleOffsetX < 0 || o.sampleOffsetX >= 1 || o.sampleOffsetY < 0 || o.sampleOffsetY >= 1)
    return std::unexpected("invalid/unsupported raster state");
  for (const auto& v : p) {
    if (!std::isfinite(v.screenX) || !std::isfinite(v.screenY) || !std::isfinite(v.screenZ) ||
        !std::isfinite(v.rhw) || v.rhw <= 0 || !std::isfinite(v.u) || !std::isfinite(v.v) ||
        std::abs(v.u) > 1.0e6 || std::abs(v.v) > 1.0e6)
      return std::unexpected("non-finite, non-positive RHW, or unbounded vertex input");
    for (double c : v.diffuse) if (!std::isfinite(c) || c < 0 || c > 255)
      return std::unexpected("diffuse channel outside finite 0..255 range");
  }
  const auto& a = p[0]; const auto& b = p[1]; const auto& c = p[2];
  const double area = (b.screenX - a.screenX) * (c.screenY - a.screenY) -
                      (b.screenY - a.screenY) * (c.screenX - a.screenX);
  if (!std::isfinite(area)) return std::unexpected("non-finite triangle area");
  if (std::abs(area) < 1e-12) return {};

  const double minX = std::min({a.screenX, b.screenX, c.screenX});
  const double maxX = std::max({a.screenX, b.screenX, c.screenX});
  const double minY = std::min({a.screenY, b.screenY, c.screenY});
  const double maxY = std::max({a.screenY, b.screenY, c.screenY});
  if (maxX < o.sampleOffsetX || maxY < o.sampleOffsetY ||
      minX > target.width - 1 + o.sampleOffsetX || minY > target.height - 1 + o.sampleOffsetY) return {};
  const int xmin = static_cast<int>(std::max(0.0, std::ceil(minX - o.sampleOffsetX)));
  const int xmax = static_cast<int>(std::min(double(target.width - 1), std::floor(maxX - o.sampleOffsetX)));
  const int ymin = static_cast<int>(std::max(0.0, std::ceil(minY - o.sampleOffsetY)));
  const int ymax = static_cast<int>(std::min(double(target.height - 1), std::floor(maxY - o.sampleOffsetY)));
  for (int y = ymin; y <= ymax; ++y) for (int x = xmin; x <= xmax; ++x) {
    const double sx = x + o.sampleOffsetX, sy = y + o.sampleOffsetY;
    const double w0 = ((b.screenX - sx) * (c.screenY - sy) - (b.screenY - sy) * (c.screenX - sx)) / area;
    const double w1 = ((c.screenX - sx) * (a.screenY - sy) - (c.screenY - sy) * (a.screenX - sx)) / area;
    const double w2 = 1.0 - w0 - w1;
    if (std::min({w0, w1, w2}) < 0) continue;
    const double q = w0*a.rhw + w1*b.rhw + w2*c.rhw;
    if (!(q > 0) || !std::isfinite(q)) continue;
    const double z = w0*a.screenZ + w1*b.screenZ + w2*c.screenZ;
    const std::size_t pos = static_cast<std::size_t>(y) * target.width + x;
    if (!depthPass(o.depthFunction, z, depth[pos])) continue;
    const double u = (w0*a.u*a.rhw + w1*b.u*b.rhw + w2*c.u*c.rhw) / q;
    const double v = (w0*a.v*a.rhw + w1*b.v*b.rhw + w2*c.v*c.rhw) / q;
    if (!std::isfinite(u) || !std::isfinite(v) || std::abs(u) > 1.0e6 || std::abs(v) > 1.0e6)
      return std::unexpected("interpolated UV outside supported range");
    const auto texel = sample(texture, u, v, o.filter, o.address);
    std::array<double, 4> diffuse = a.diffuse;
    if (o.shade == SceneShade::Gouraud)
      for (int k=0;k<4;++k) diffuse[k] = (w0*a.diffuse[k]*a.rhw + w1*b.diffuse[k]*b.rhw + w2*c.diffuse[k]*c.rhw) / q;
    std::array<double, 4> src{};
    for (int k=0;k<3;++k) src[k] = texel[k] * diffuse[k] / 255.0;
    src[3] = o.alphaOperation == SceneAlphaOperation::SelectTexture ? texel[3] : texel[3] * diffuse[3] / 255.0;
    if (o.alphaFunction == SceneAlphaFunction::StrictGreater && !(src[3] > o.alphaReference)) continue;
    // Mask membership precedes all diagnostic event construction/storage.
    const bool observe = observer && (*observer->mask)[pos] != 0;
    SceneRasterWriteEvent event{};
    if (observe) {
      event.source=observer->source; event.triangle=observer->triangle; event.pixel=pos;
      for (int k=0;k<4;++k) event.colorBefore[k]=target.rgba[pos*4+k];
      event.depthBefore=depth[pos];
    }
    const auto at = pos * 4;
    for (int k=0;k<4;++k) {
      const double out = o.blendEnabled
          ? src[k] * factorValue(o.sourceBlend, src[3]) + target.rgba[at+k] * factorValue(o.destinationBlend, src[3])
          : src[k];
      target.rgba[at+k] = unorm8(out);
    }
    if (o.depthWrite) depth[pos] = z;
    if (observe) {
      for (int k=0;k<4;++k) event.colorAfter[k]=target.rgba[at+k];
      event.depthAfter=depth[pos]; event.depthAssigned=o.depthWrite;
      observer->accepted(event);
    }
  }
  return {};
}

}  // namespace webo::app
