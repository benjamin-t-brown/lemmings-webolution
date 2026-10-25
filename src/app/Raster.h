// A small CPU triangle rasteriser for the 3D-first frame.
//
// Deliberately a straight port of the accepted A2 fixture's inner loop
// (tools/harness/render_tube_fixture.py, triangle()): screen-space linear
// depth with a smaller-is-nearer keep test, perspective-correct UVs via 1/z,
// bilinear sampling with wrap. It is not a device-state model: no blend state,
// no scissor, no guard band, no mip selection. The A2 note already limits the
// pass to the fixed ROI and to the declared software depth ordering.
//
// Colours are 8-bit triplets; the PNG writer converts. One cell is one world
// unit throughout, so no PIXELS_PER_UNIT scaling appears anywhere.

#pragma once

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

  // p[i] = {screenX, screenY, viewDepth - context, 1 / viewDepth}.
  // uv[i] = texture coordinates in tile units (wrapped at sample time).
  void triangle(const std::array<std::array<double, 4>, 3>& p,
                const std::array<std::array<double, 2>, 3>& uv, const Texture& texture,
                const std::array<double, 3>& modulate);

  const Framebuffer& framebuffer() const { return color_; }
  Framebuffer& mutableFramebuffer() { return color_; }
  std::vector<double>& mutableDepthBuffer() { return depth_; }
  std::size_t drawnTriangles() const { return triangles_; }
  std::size_t drawnPixels() const { return pixels_; }

 private:
  int width_ = 0;
  int height_ = 0;
  bool depthEnabled_ = true;
  Framebuffer color_;
  std::vector<double> depth_;
  std::size_t triangles_ = 0;
  std::size_t pixels_ = 0;
};

}  // namespace webo::app
