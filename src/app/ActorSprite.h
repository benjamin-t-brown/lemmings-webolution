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

}  // namespace webo::app
