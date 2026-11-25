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
  bool operator==(const ActorSpritePlacement&) const = default;
};

// Provisional clone occlusion uses physical view Z. Undo the normalized
// 0.0003 placement bias so thin walls and floors can hide the actor.
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

// Actor draw path 0x43CC68. The CameraBasis overload applies the native
// lens (0.3 scale, Z coefficient 0.3/width), then scales the rectangle
// about its origin by the terrain zoom depth ratio. The other overload
// accepts a lensed transform and uses the native fixed-pixel extents.
std::expected<ActorSpritePlacement, std::string> actorSpritePlacement(
    const webo::SpriteFrame& frame, const std::array<double, 3>& worldPosition,
    const CameraBasis& camera, bool horizontalMirror, int scaleDelta = -1);

std::expected<ActorSpritePlacement, std::string> actorSpritePlacement(
    const webo::SpriteFrame& frame, const std::array<double, 3>& worldPosition,
    const ActorProjectionInput& projection, bool horizontalMirror, int scaleDelta = -1);

// The SPR rectangle uses MODULATE(texture RGB, vertexRgb). With alphaTest,
// material row 3 rejects texels that fail the 0xEF reference (kActorAlphaTestReference
// in LightMap.h) and writes passing texels opaquely. With it off, write
// every texel with its texture alpha.
std::expected<void, std::string> drawActorSprite(
    webo::Image& overlay, const webo::SpriteFrame& frame, const webo::Image& sheet,
    const ActorSpritePlacement& placement, bool horizontalMirror,
    std::array<std::uint8_t, 3> vertexRgb = {255, 255, 255}, bool alphaTest = true);

// Native orientation+2D8 reverses V independently of mirror (42E5D4, 47A7F1/47A999).
std::expected<void, std::string> drawActorSprite(
    webo::Image& overlay, const webo::SpriteFrame& frame, const webo::Image& sheet,
    const ActorSpritePlacement& placement, bool horizontalMirror,
    std::array<std::uint8_t, 3> vertexRgb, bool alphaTest, bool verticalMirror);

// Provisional clone anchoring reflects the box around the projected feet.
// The native sprite origin and +368 collision bias use a different anchor.
ActorSpritePlacement actorSpriteGravityPlacement(ActorSpritePlacement placement, int verticalDir);

}  // namespace webo::app
