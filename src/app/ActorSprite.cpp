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
  auto placement = actorSpritePlacement(frame, worldPosition, projection, horizontalMirror, scaleDelta);
  if (!placement || camera.defaultZoomDepthOffset == 0.0)
    return placement;
  const double scale = camera.zoomScaleAtDepth(actorPhysicalDepth(*placement, camera.width));
  if (!(scale > 0.0) || !std::isfinite(scale))
    return std::unexpected("actor sprite: invalid zoom reference depth");
  // Provisional clone zoom. Native 47A75D uses global half/full/double extents.
  // Scale the zoom-0 rectangle by local terrain magnification and retain its
  // origin bias. Drawing, highlighting and picking share this placement.
  placement->left = nativeFloat(placement->originX + (placement->left - placement->originX) * scale);
  placement->right = nativeFloat(placement->originX + (placement->right - placement->originX) * scale);
  placement->top = nativeFloat(placement->originY + (placement->top - placement->originY) * scale);
  placement->bottom = nativeFloat(placement->originY + (placement->bottom - placement->originY) * scale);
  return placement;
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
  // 43CDA0's 0.0003 bias uses normalized lens Z (Z/width).
  const double biasedDepth = z - 0.0003;
  if (!(biasedDepth > 0.0) || !std::isfinite(biasedDepth))
    return std::unexpected("actor sprite: origin is behind the biased near plane");

  const double scale = scaleDelta == 0 ? 1.0 : (scaleDelta < 0 ? 0.5 : 2.0);
  const double originX = projection.cx + x / biasedDepth;
  const double originY = projection.cy - y / biasedDepth;
  // In the 0x47A75D delta=-1 path, add the half-scaled frame offsets and
  // subtract the half extents. The reverse side adds the same extents.
  double left = horizontalMirror
                    ? originX + (static_cast<double>(frame.unknown0) - frame.width) * scale
                    : originX - frame.unknown0 * scale;
  double top = originY - frame.unknown1 * scale - frame.height * scale;
  // At 0x47AA59 and 0x47AAAF, scaling affects the frame height. The initial
  // unmirrored subtraction of unknown1 is at full scale.
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
  return drawActorSprite(overlay, frame, sheet, p, horizontalMirror, vertexRgb, alphaTest, false);
}

ActorSpritePlacement actorSpriteGravityPlacement(ActorSpritePlacement p, int verticalDir) {
  if (verticalDir < 0) {
    const double top = 2.0 * p.originY - p.bottom;
    p.bottom = nativeFloat(2.0 * p.originY - p.top);
    p.top = nativeFloat(top);
  }
  return p;
}

std::expected<void, std::string> drawActorSprite(webo::Image& overlay,
                                                 const webo::SpriteFrame& frame,
                                                 const webo::Image& sheet,
                                                 const ActorSpritePlacement& p,
                                                 bool horizontalMirror,
                                                 std::array<std::uint8_t, 3> vertexRgb,
                                                 bool alphaTest, bool verticalMirror) {
  if (overlay.width <= 0 || overlay.height <= 0 ||
      overlay.rgba.size() != static_cast<std::size_t>(overlay.width) * overlay.height * 4)
    return std::unexpected("actor sprite: invalid overlay storage");
  if (sheet.width <= 0 || sheet.height <= 0 ||
      sheet.rgba.size() != static_cast<std::size_t>(sheet.width) * sheet.height * 4)
    return std::unexpected("actor sprite: invalid sheet storage");
  if (frame.width == 0 || frame.height == 0 || p.right <= p.left || p.bottom <= p.top)
    return std::unexpected("actor sprite: empty frame or destination");

  // Software coverage and UVs use integer pixel coordinates and nearest sampling.
  // 0x470F29(linear) sets stage-0 TSS MAG/MIN (0x10 and 0x11). In D3D7,
  // the default 1 is POINT. LINEAR (2) requires a nonzero argument and [this+0x3a1]
  // (0x470F32/0x470F5E, pushes at 0x470F72/0x470F8F).
  // 0x47A766 passes 0 when [0x58D53C]==0. The active actor sampler is unconfirmed.
  const int x0 = std::max(0, static_cast<int>(std::ceil(p.left)));
  const int x1 = std::min(overlay.width, static_cast<int>(std::ceil(p.right)));
  const int y0 = std::max(0, static_cast<int>(std::ceil(p.top)));
  const int y1 = std::min(overlay.height, static_cast<int>(std::ceil(p.bottom)));
  for (int y = y0; y < y1; ++y) {
    double v = (y - p.top) / (p.bottom - p.top);
    if (verticalMirror) v = 1.0 - v;
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
