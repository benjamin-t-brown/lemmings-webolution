// Explicit-input software scene triangle rasterizer. It intentionally has no
// device-state defaults and is separate from the legacy Raster path.
#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace webo::app {

struct SceneRgbaImage {
  int width = 0;
  int height = 0;
  std::vector<std::uint8_t> rgba;  // tightly packed top-down RGBA8
};

struct SceneRasterVertex {
  double screenX = 0;
  double screenY = 0;
  double screenZ = 0;
  double rhw = 0;
  std::array<double, 4> diffuse{};  // 0..255 values supplied by caller
  double u = 0;
  double v = 0;
};

enum class SceneFilter { Point, Linear };
enum class SceneAddress { Wrap, Clamp };
enum class SceneAlphaFunction { Disabled, StrictGreater };
enum class SceneBlendFactor { One, SrcAlpha, InvSrcAlpha };
enum class SceneDepthFunction { Never, Less, LessEqual, Equal, GreaterEqual, Greater, NotEqual, Always };
enum class SceneShade { Flat, Gouraud };
enum class SceneRgbOperation { Modulate };
enum class SceneAlphaOperation { SelectTexture, Modulate };

struct SceneRasterOptions {
  // Every state that affects supported output is mandatory at the call site.
  SceneFilter filter;
  SceneAddress address;
  SceneAlphaFunction alphaFunction;
  std::uint8_t alphaReference;
  bool blendEnabled;
  SceneBlendFactor sourceBlend;
  SceneBlendFactor destinationBlend;
  SceneDepthFunction depthFunction;
  bool depthWrite;
  SceneShade shade;
  SceneRgbOperation rgbOperation;
  SceneAlphaOperation alphaOperation;
  double sampleOffsetX;
  double sampleOffsetY;
};

struct SceneRgbaFramebuffer {
  int width = 0;
  int height = 0;
  std::vector<std::uint8_t> rgba;
};

// Software conventions, not asserted native D3D measurements:
// samples are at pixel+(sampleOffsetX,sampleOffsetY); coverage includes all
// triangle edges (shared-edge double hits are possible); screen Z is affine;
// UV and Gouraud diffuse are perspective-correct using caller RHW; flat diffuse
// is vertex 0; texture point uses floor(U*size), linear uses U*size-.5; WRAP is
// modulo (including negative U/V), CLAMP clamps texel indices. UNORM output is
// rounded nearest with halfway values away from zero. No quantizer is applied
// to supplied texture/diffuse inputs beyond this RGBA8 output conversion.
// Fog, specular, mipmaps, higher texture stages, culling and clipping are not
// implemented; inputs must already be clipped, and those states are outside
// this API rather than implicitly selected.
std::expected<void, std::string> rasterizeSceneTriangle(
    SceneRgbaFramebuffer& target, std::vector<double>& depth,
    const SceneRgbaImage& texture,
    const std::array<SceneRasterVertex, 3>& vertices,
    const SceneRasterOptions& options);

}  // namespace webo::app
