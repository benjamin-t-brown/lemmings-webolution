#include "app/ActorSprite.h"
#include "assets/LightMap.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {
void require(bool ok, const char* message) {
  if (!ok) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}
void nearNativeFloat(double actual, double expected, const char* message) {
  const float got = static_cast<float>(actual);
  const float want = static_cast<float>(expected);
  const float next = std::nextafter(want, std::numeric_limits<float>::infinity());
  require(std::abs(got - want) <= next - want, message);
}
}  // namespace

int main() {
  webo::app::CameraParams params;
  params.width = 640;
  params.height = 480;
  params.aspect = 4.0 / 3.0;
  const auto camera = webo::app::buildCamera(params);
  constexpr std::array<double, 3> world{-56.99833297729492, 32.0, 6.069479465484619};
  const auto projectionFromCamera = webo::app::actorProjectionInput(camera);
  require(std::abs(projectionFromCamera.worldToLensedView[0] -
                   0.3 * camera.focalX / camera.width * camera.worldToView[0]) < 1e-15 &&
              std::abs(projectionFromCamera.worldToLensedView[1] -
                       0.3 * camera.focalY / camera.width * camera.worldToView[1]) < 1e-15 &&
              std::abs(projectionFromCamera.worldToLensedView[2] -
                       0.3 * camera.worldToView[2] / camera.width) < 1e-15,
          "synthetic camera applies uniform 0.3 projection scale on x, y and z");
  webo::app::CameraBasis synthetic;
  synthetic.worldToView = {1,0,0, 0,1,0, 0,0,1, 0,0,0};
  synthetic.focalX = 80;
  synthetic.focalY = 120;
  synthetic.width = 640;
  const auto syntheticProjection = webo::app::actorProjectionInput(synthetic);
  require(std::abs(syntheticProjection.worldToLensedView[0] - 0.0375) < 1e-15 &&
              std::abs(syntheticProjection.worldToLensedView[4] - 0.05625) < 1e-15 &&
              std::abs(syntheticProjection.worldToLensedView[8] - 0.3 / 640.0) < 1e-15,
          "known synthetic lens ratios retain uniform 0.3 on each axis");

  webo::SpriteFrame productionFrame;
  productionFrame.width = 2;
  productionFrame.height = 2;
  const auto productionPlacement = webo::app::actorSpritePlacement(
      productionFrame, world, camera, true, -1);
  require(productionPlacement.has_value() && std::isfinite(productionPlacement->originX),
          "A1 camera builder converts to normalized actor projection units");

  // The selected five-direction record from live-pairs-4 (action13/time8/dir1).
  webo::SpriteFrame walk;
  walk.left = 53; walk.top = 0; walk.right = 79; walk.bottom = 38;
  walk.uLeft = 53.0 / 256.0; walk.uTop = 0.0;
  walk.uRight = 79.0 / 256.0; walk.uBottom = 38.0 / 226.0;
  walk.unknown0 = 13; walk.unknown1 = 0; walk.width = 26; walk.height = 38;
  const webo::app::ActorProjectionInput nativeProjection{
      {-0.4746057093143463, 0.09290599077939987, 0.00032504371483810246,
       0.0, 0.6582083702087402, -9.175968443742022e-05,
       -0.47460585832595825, -0.09290596097707748, -0.00032504359842278063,
       0.0, -25.03416633605957, 0.1839417815208435}, 319.0, 239.0};
  auto five = webo::app::actorSpritePlacement(walk, world, nativeProjection, true, -1);
  require(five.has_value(), "native five-direction semantic matrix computes");
  nearNativeFloat(five->left, 463.3755798339844, "native five left corner within one float ULP");
  nearNativeFloat(five->top, 281.36407470703125, "native five top corner within one float ULP");
  nearNativeFloat(five->right, 476.3755798339844, "native five right corner within one float ULP");
  nearNativeFloat(five->bottom, 300.36407470703125, "native five bottom corner within one float ULP");

  // The selected eight-direction record from live-pairs-4 (action1/time0/dir7).
  webo::SpriteFrame block;
  block.left = 210; block.top = 152; block.right = 240; block.bottom = 190;
  block.uLeft = 210.0 / 256.0; block.uTop = 152.0 / 228.0;
  block.uRight = 240.0 / 256.0; block.uBottom = 190.0 / 228.0;
  block.unknown0 = 14; block.unknown1 = 2; block.width = 30; block.height = 38;
  auto eight = webo::app::actorSpritePlacement(block, world, nativeProjection, true, -1);
  require(eight.has_value(), "native eight-direction placement computes");
  nearNativeFloat(eight->left, 461.8755798339844, "native eight left corner within one float ULP");
  nearNativeFloat(eight->top, 280.36407470703125, "native eight top corner within one float ULP");
  nearNativeFloat(eight->right, 476.8755798339844, "native eight right corner within one float ULP");
  nearNativeFloat(eight->bottom, 299.36407470703125, "native eight bottom corner within one float ULP");

  auto unmirroredHalf = webo::app::actorSpritePlacement(block, world, nativeProjection, false, -1);
  require(unmirroredHalf.has_value(), "unmirrored half-scale placement computes");
  nearNativeFloat(unmirroredHalf->top,
                  unmirroredHalf->originY - block.unknown1 - block.height * 0.5,
                  "unmirrored half-scale keeps the initial full unknown1 subtraction");
  auto unmirroredDouble = webo::app::actorSpritePlacement(block, world, nativeProjection, false, 1);
  require(unmirroredDouble.has_value(), "unmirrored double-scale placement computes");
  nearNativeFloat(unmirroredDouble->top,
                  unmirroredDouble->originY - block.unknown1 - block.height * 2.0,
                  "unmirrored double-scale keeps the initial full unknown1 subtraction");

  // Literal 2x2 source: opaque RGB values plus one keyed transparent texel.
  webo::Image sheet;
  sheet.width = 2; sheet.height = 2;
  sheet.rgba = {255,0,0,255, 0,255,0,255,
                0,0,255,255, 64,64,64,0};
  webo::Image overlay;
  overlay.width = 8; overlay.height = 8; overlay.alpha = webo::AlphaMode::Embedded;
  overlay.rgba.assign(8 * 8 * 4, 0);
  webo::SpriteFrame tiny;
  tiny.left = 0; tiny.top = 0; tiny.right = 2; tiny.bottom = 2;
  tiny.width = 2; tiny.height = 2;
  tiny.uLeft = 0; tiny.uTop = 0; tiny.uRight = 1; tiny.uBottom = 1;
  webo::app::ActorSpritePlacement quad{0,0,1,2,2,4,4};
  require(webo::app::drawActorSprite(overlay, tiny, sheet, quad, false).has_value(),
          "opaque and alpha-key pixels draw");
  const auto pixel = [&](int x, int y, int c) { return overlay.rgba[(y * 8 + x) * 4 + c]; };
  require(pixel(2,2,0)==255 && pixel(2,2,3)==255, "top-left source texel maps to destination");
  require(pixel(3,2,1)==255 && pixel(3,2,3)==255, "top-right source texel maps to destination");
  require(pixel(2,3,2)==255 && pixel(2,3,3)==255, "bottom-left source texel maps to destination");
  require(pixel(3,3,3)==0, "$ gray-key alpha remains transparent");

  overlay.rgba.assign(8 * 8 * 4, 0);
  require(webo::app::drawActorSprite(overlay, tiny, sheet, quad, true).has_value(),
          "mirrored source draws");
  require(pixel(2,2,1)==255 && pixel(2,2,3)==255,
          "horizontal mirror reverses UV selection without moving the quad");
  require(!webo::app::actorSpritePlacement(tiny, world, nativeProjection, false, 2).has_value(),
          "unsupported scale delta rejected");

  require(webo::actorLightMapIndex(world[0], world[1]) == 4196,
          "A4f fixture world X/Y maps to index 4196");
  require(webo::actorLightMapIndex(-57.0, 32.0) == 4195,
          "truncated X=-57 maps to index 4195");
  require(webo::actorLightMapIndex(-0.25, 32.0) == webo::actorLightMapIndex(0.0, 32.0),
          "X in (-1,0) truncates toward zero like native _ftol");

  const auto modulated =
      webo::modulateActorSpritePixel({200, 100, 50, 128}, {152, 152, 152});
  require(modulated[0] == static_cast<std::uint8_t>((200 * 152) / 255) &&
              modulated[1] == static_cast<std::uint8_t>((100 * 152) / 255) &&
              modulated[2] == static_cast<std::uint8_t>((50 * 152) / 255) &&
              modulated[3] == 128,
          "MODULATE scales RGB and preserves alpha");
  require(webo::actorLightMapIndex(0.0, -1.0) == -128,
          "negative Y uses signed shift semantics in the index");

  webo::Image tintSheet;
  tintSheet.width = 1;
  tintSheet.height = 1;
  tintSheet.rgba = {200, 0, 0, 255};
  webo::Image tintOverlay;
  tintOverlay.width = 4;
  tintOverlay.height = 4;
  tintOverlay.alpha = webo::AlphaMode::Embedded;
  tintOverlay.rgba.assign(4 * 4 * 4, 0);
  webo::SpriteFrame tintFrame;
  tintFrame.left = 0;
  tintFrame.top = 0;
  tintFrame.right = 1;
  tintFrame.bottom = 1;
  tintFrame.width = 1;
  tintFrame.height = 1;
  tintFrame.uLeft = 0;
  tintFrame.uTop = 0;
  tintFrame.uRight = 1;
  tintFrame.uBottom = 1;
  webo::app::ActorSpritePlacement tintQuad{0, 0, 1, 2, 2, 4, 4};
  const auto tintedPixel = [&](int x, int y, int c) {
    return tintOverlay.rgba[(y * 4 + x) * 4 + c];
  };
  require(webo::app::drawActorSprite(tintOverlay, tintFrame, tintSheet, tintQuad, false,
                                     {255, 255, 255})
              .has_value(),
          "draw with white tint");
  require(tintedPixel(2, 2, 0) == 200, "white tint leaves red channel unchanged");
  tintOverlay.rgba.assign(4 * 4 * 4, 0);
  const std::array<std::uint8_t, 3> mapTint{152, 152, 152};
  require(webo::app::drawActorSprite(tintOverlay, tintFrame, tintSheet, tintQuad, false, mapTint)
              .has_value(),
          "draw with map tint");
  require(tintedPixel(2, 2, 0) == static_cast<std::uint8_t>((200 * 152) / 255),
          "draw path applies MODULATE");

  std::cout << "PASS: actor sprite placement, UV mirror, light tint and alpha key\n";
  return 0;
}
