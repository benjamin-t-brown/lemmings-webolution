#pragma once

#include "Camera.h"
#include "assets/Bitmap.h"
#include "assets/Sprite.h"

#include <array>
#include <cstdint>
#include <expected>
#include <string>

namespace webo::app {

struct ActorSpritePlacement {
  double originX = 0.0;
  double originY = 0.0;
  double depth = 0.0;
  double left = 0.0;
  double top = 0.0;
  double right = 0.0;
  double bottom = 0.0;
};

// Recover physical origin view Z for scene occlusion. The traced normalized
// 0.0003 bias remains in sprite projection/placement, but is undone here:
// pulling comparison depth toward the camera exposes actors through thin
// walls and bridge floors (playtest 9 Y9-Y11). No contact allowance.
// This is provisional clone presentation, not original device depth policy.
inline double actorPhysicalDepth(const ActorSpritePlacement& p, int viewportWidth) {
  return (p.depth + 0.0003) * viewportWidth / 0.3;
}

// A lensed view-space input for the actor draw path. Its x/y/z operands are
// already in the native projection's normalized units (lens Z is 0.3/width).
struct ActorProjectionInput {
  std::array<double, 12> worldToLensedView{};
  double cx = 319.0;
  double cy = 239.0;
};

ActorProjectionInput actorProjectionInput(const CameraBasis& camera);

// Actor draw path 0x43CC68. The CameraBasis overload converts its un-lensed
// view transform to the native lens vector, uniformly scaled by 0.3 with Z
// coefficient 0.3/width.
// The semantic-input overload accepts that lensed transform directly.
std::expected<ActorSpritePlacement, std::string> actorSpritePlacement(
    const webo::SpriteFrame& frame, const std::array<double, 3>& worldPosition,
    const CameraBasis& camera, bool horizontalMirror, int scaleDelta = -1);

std::expected<ActorSpritePlacement, std::string> actorSpritePlacement(
    const webo::SpriteFrame& frame, const std::array<double, 3>& worldPosition,
    const ActorProjectionInput& projection, bool horizontalMirror, int scaleDelta = -1);

// Candidate CPU raster for the selected SPR rectangle. RGB is MODULATE(texture,
// vertexRgb). With alphaTest (default, A4u) a texel is written only when its
// alpha passes the material row 3 test (see kActorAlphaTestReference in
// LightMap.h; A4x/A4y in-draw reference 0xEF, tested encoded path) and is written
// opaquely (overlay alpha 255, no blend, so the harness composite becomes a pure
// replace). alphaTest=false is the pre-A4u diagnostic: every texel is written
// keeping texture alpha.
std::expected<void, std::string> drawActorSprite(
    webo::Image& overlay, const webo::SpriteFrame& frame, const webo::Image& sheet,
    const ActorSpritePlacement& placement, bool horizontalMirror,
    std::array<std::uint8_t, 3> vertexRgb = {255, 255, 255}, bool alphaTest = true);

// R8: original orientation+2D8 reverses V independently of horizontal mirror
// (42E5D4, 47A7F1/47A999). Preserve the old overload for upright consumers.
std::expected<void, std::string> drawActorSprite(
    webo::Image& overlay, const webo::SpriteFrame& frame, const webo::Image& sheet,
    const ActorSpritePlacement& placement, bool horizontalMirror,
    std::array<std::uint8_t, 3> vertexRgb, bool alphaTest, bool verticalMirror);

// PROVISIONAL clone support-point adapter: reflect the sprite box around the
// projected feet. Native origin/+368 collision bias is a different convention;
// an original inverted contact-frame comparison must settle exact anchoring.
ActorSpritePlacement actorSpriteGravityPlacement(ActorSpritePlacement placement, int verticalDir);

}  // namespace webo::app
