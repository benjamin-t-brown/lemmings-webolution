#include "ActorSprite.h"

#include "assets/LightMap.h"

#include <algorithm>
#include <cmath>

namespace webo::app {
namespace {

double nativeFloat(double value) {
  return static_cast<double>(static_cast<float>(value));
}

}  // namespace

ActorProjectionInput actorProjectionInput(const CameraBasis& camera) {
  ActorProjectionInput projection;
  const auto& m = camera.worldToView;
  const double lensX = camera.focalX / camera.width;
  const double lensY = camera.focalY / camera.width;
  projection.worldToLensedView = {
      0.3*lensX*m[0], 0.3*lensY*m[1], 0.3*m[2]/camera.width,
      0.3*lensX*m[3], 0.3*lensY*m[4], 0.3*m[5]/camera.width,
      0.3*lensX*m[6], 0.3*lensY*m[7], 0.3*m[8]/camera.width,
      0.3*lensX*m[9], 0.3*lensY*m[10], 0.3*m[11]/camera.width};
  projection.cx = camera.cx;
  projection.cy = camera.cy;
  return projection;
}

std::expected<ActorSpritePlacement, std::string> actorSpritePlacement(
    const webo::SpriteFrame& frame, const std::array<double, 3>& worldPosition,
    const CameraBasis& camera, bool horizontalMirror, int scaleDelta) {
  if (frame.width == 0 || frame.height == 0)
    return std::unexpected("actor sprite: degenerate frame");
  if (scaleDelta < -1 || scaleDelta > 1)
    return std::unexpected("actor sprite: unsupported native scale delta");
  const auto projection = actorProjectionInput(camera);
  return actorSpritePlacement(frame, worldPosition, projection, horizontalMirror, scaleDelta);
}

std::expected<ActorSpritePlacement, std::string> actorSpritePlacement(
    const webo::SpriteFrame& frame, const std::array<double, 3>& worldPosition,
    const ActorProjectionInput& projection, bool horizontalMirror, int scaleDelta) {
  if (frame.width == 0 || frame.height == 0)
    return std::unexpected("actor sprite: degenerate frame");
  if (scaleDelta < -1 || scaleDelta > 1)
    return std::unexpected("actor sprite: unsupported native scale delta");
  const auto& m = projection.worldToLensedView;
  const double x = m[0] * worldPosition[0] + m[3] * worldPosition[1] +
                   m[6] * worldPosition[2] + m[9];
  const double y = m[1] * worldPosition[0] + m[4] * worldPosition[1] +
                   m[7] * worldPosition[2] + m[10];
  const double z = m[2] * worldPosition[0] + m[5] * worldPosition[1] +
                   m[8] * worldPosition[2] + m[11];
  // Native lens vector uses Z/width. 43CDA0's 0.0003 bias is in that same
  // normalized unit; applying it to physical world-space Z is incorrect.
  const double biasedDepth = z - 0.0003;
  if (!(biasedDepth > 0.0) || !std::isfinite(biasedDepth))
    return std::unexpected("actor sprite: origin is behind the biased near plane");

  const double scale = scaleDelta == 0 ? 1.0 : (scaleDelta < 0 ? 0.5 : 2.0);
  const double originX = projection.cx + x / biasedDepth;
  const double originY = projection.cy - y / biasedDepth;
  // Raw 0x47A75D delta=-1 path: add the two record offsets at half scale,
  // then subtract half the source width/height. The reverse side supplies
  // right/bottom by adding the same half extents. The other two literal
  // branches are the full and doubled versions of this arithmetic.
  double left = horizontalMirror
                    ? originX + (static_cast<double>(frame.unknown0) - frame.width) * scale
                    : originX - frame.unknown0 * scale;
  double top = originY - frame.unknown1 * scale - frame.height * scale;
  // The unmirrored branches at 0x47AA59 and 0x47AAAF both keep the initial
  // full unknown1 subtraction; only the frame-height extent changes scale.
  if (!horizontalMirror)
    top = originY - frame.unknown1 - frame.height * scale;
  double right = left + frame.width * scale;
  double bottom = top + frame.height * scale;

  // For mirrored frames the reverse destination edge is selected and U is
  // reversed at sample time; the unmirrored branch anchors by unknown0 alone.
  left = nativeFloat(left);
  top = nativeFloat(top);
  right = nativeFloat(right);
  bottom = nativeFloat(bottom);
  return ActorSpritePlacement{originX, originY, biasedDepth, left, top, right, bottom};
}

std::expected<void, std::string> drawActorSprite(webo::Image& overlay,
                                                 const webo::SpriteFrame& frame,
                                                 const webo::Image& sheet,
                                                 const ActorSpritePlacement& p,
                                                 bool horizontalMirror,
                                                 std::array<std::uint8_t, 3> vertexRgb,
                                                 bool alphaTest) {
  if (overlay.width <= 0 || overlay.height <= 0 ||
      overlay.rgba.size() != static_cast<std::size_t>(overlay.width) * overlay.height * 4)
    return std::unexpected("actor sprite: invalid overlay storage");
  if (sheet.width <= 0 || sheet.height <= 0 ||
      sheet.rgba.size() != static_cast<std::size_t>(sheet.width) * sheet.height * 4)
    return std::unexpected("actor sprite: invalid sheet storage");
  if (frame.width == 0 || frame.height == 0 || p.right <= p.left || p.bottom <= p.top)
    return std::unexpected("actor sprite: empty frame or destination");

  // A4v (MEASURED offline on the two retained fixtures; no exe byte states the
  // pixel-centre convention): the native raster matches as if the pixel centre
  // is the INTEGER coordinate, not x+0.5. Pixel x is covered when
  // left <= x < right and its UV is (x - left) / (right - left); likewise for
  // y. The earlier x+0.5 centre was this project's choice and sat half a pixel
  // up-left of native (support MAE 0.0819/0.0778 before, 0.0014/0.0015 after).
  // Sampling stays nearest-texel. READ: 0x470F29(linear) writes TSS stage-0
  // 0x10 and 0x11 (MAG/MIN) with value 1 by default and 2 only when its arg and
  // byte [this+0x3a1] are both nonzero (0x470F32/0x470F5E, pushes at
  // 0x470F72/0x470F8F); 0x47A766 calls it with 0 when [0x58D53C]==0. That the
  // value 1 is D3D7 POINT (2 = LINEAR) is header knowledge, not an exe byte;
  // bilinear scores worse here (support MAE 0.027). Whether this call is the one
  // in force for actor sprites is not traced.
  const int x0 = std::max(0, static_cast<int>(std::ceil(p.left)));
  const int x1 = std::min(overlay.width, static_cast<int>(std::ceil(p.right)));
  const int y0 = std::max(0, static_cast<int>(std::ceil(p.top)));
  const int y1 = std::min(overlay.height, static_cast<int>(std::ceil(p.bottom)));
  for (int y = y0; y < y1; ++y) {
    const double v = (y - p.top) / (p.bottom - p.top);
    const double sourceV = frame.uTop + v * (frame.uBottom - frame.uTop);
    const int sy = std::clamp(static_cast<int>(std::floor(sourceV * sheet.height)),
                              static_cast<int>(frame.top), static_cast<int>(frame.bottom) - 1);
    for (int x = x0; x < x1; ++x) {
      double u = (x - p.left) / (p.right - p.left);
      if (horizontalMirror)
        u = 1.0 - u;
      const double sourceU = frame.uLeft + u * (frame.uRight - frame.uLeft);
      const int sx = std::clamp(static_cast<int>(std::floor(sourceU * sheet.width)),
                                static_cast<int>(frame.left), static_cast<int>(frame.right) - 1);
      const std::size_t src = (static_cast<std::size_t>(sy) * sheet.width + sx) * 4;
      const std::size_t dst = (static_cast<std::size_t>(y) * overlay.width + x) * 4;
      const std::array<std::uint8_t, 4> tex{sheet.rgba[src], sheet.rgba[src + 1],
                                            sheet.rgba[src + 2], sheet.rgba[src + 3]};
      if (alphaTest && !actorSpriteAlphaTestPasses(tex[3]))
        continue;
      auto out = modulateActorSpritePixel(tex, vertexRgb);
      if (alphaTest)
        out[3] = 255;  // row 3 / selector 0: opaque replace, no blend
      for (int c = 0; c < 4; ++c)
        overlay.rgba[dst + c] = out[static_cast<std::size_t>(c)];
    }
  }
  return {};
}

}  // namespace webo::app
