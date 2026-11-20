// CPU triangle rasterizer with linear view depth (smaller is nearer),
// perspective-correct UVs and bilinear sampling. RGB uses 8-bit triplets.
// One terrain cell is one world unit.

#pragma once

#include "app/ActorDepth.h"
#include "app/SceneRaster.h"

#include <array>
#include <cstdint>
#include <vector>

namespace webo::app {

struct Texture {
  int width = 0;
  int height = 0;
  std::vector<std::uint8_t> rgb;  // 3 bytes per pixel, top-down

  bool empty() const { return rgb.empty(); }
};

struct Framebuffer {
  int width = 0;
  int height = 0;
  std::vector<std::uint8_t> rgb;
};

class Raster {
 public:
  Raster(int width, int height);

  void clear(std::uint8_t r, std::uint8_t g, std::uint8_t b);
  void setDepthEnabled(bool on) { depthEnabled_ = on; }
  // Merge an opaque, depth-tested geometry layer rendered against infinity.
  // Equal depth keeps the earlier destination, as in triangle().
  void compositeOpaque(const Raster& layer);
  // Cached layers reuse storage but count only their current geometry build.
  void resetDrawCounts() { triangles_ = pixels_ = 0; }

  // p[i] = {screenX, screenY, viewDepth - context, 1 / viewDepth}.
  // uv[i] = texture coordinates in tile units.
  // An actor owner uses the shared actor coverage after the strict scenery
  // test, retaining per-face physical depth without writing scenery depth.
  // An active RasterCommandBatch borrows texture; keep it alive and unchanged
  // until the batch is flushed. Scene geometry uses its owning Tile's texture.
  void triangle(const std::array<std::array<double, 4>, 3>& p,
                const std::array<std::array<double, 2>, 3>& uv, const Texture& texture,
                const std::array<double, 3>& modulate,
                std::size_t actorOwner = ActorDepth::noOwner,
                SceneAddress address = SceneAddress::Wrap);
  // Reject temporaries even in synchronous calls so the same caller is safe
  // when enclosed in a deferred batch.
  void triangle(const std::array<std::array<double, 4>, 3>& p,
                const std::array<std::array<double, 2>, 3>& uv, const Texture&& texture,
                const std::array<double, 3>& modulate,
                std::size_t actorOwner = ActorDepth::noOwner,
                SceneAddress address = SceneAddress::Wrap) = delete;

  const Framebuffer& framebuffer() const { return color_; }
  Framebuffer& mutableFramebuffer() { return color_; }
  const std::vector<double>& depthBuffer() const { return depth_; }
  std::vector<double>& mutableDepthBuffer() { return depth_; }
  const ActorDepth& actorDepth() const { return actorDepth_; }
  ActorDepth& mutableActorDepth() { return actorDepth_; }
  std::size_t drawnTriangles() const { return triangles_; }
  std::size_t drawnPixels() const { return pixels_; }

 private:
  int width_ = 0;
  int height_ = 0;
  bool depthEnabled_ = true;
  Framebuffer color_;
  std::vector<double> depth_;
  ActorDepth actorDepth_;
  std::size_t triangles_ = 0;
  std::size_t pixels_ = 0;
};

}  // namespace webo::app
