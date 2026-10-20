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

  // A4y: material row 3 alpha test, GREATER against the in-draw reference 0xEF
  // (A4x: 0x47A75D calls 0x470F29(0) at 0x47A776 before the draw; the writer
  // 0x470F39 sets [0x4C24E0]=0xEF; reader 0x472896). 4x1 sheet of alpha
  // 0xA1 / 0xEF / 0xF0 / 0x00 on a sentinel overlay. The raster assertions run
  // first and read destination pixels, so reverting the reference or the
  // strictness fails on pixels, not on the helper or the literal constant.
  webo::Image aSheet;
  aSheet.width = 4;
  aSheet.height = 1;
  aSheet.rgba = {200, 100, 50, 0xA1, 200, 100, 50, 0xEF, 200, 100, 50, 0xF0, 200, 100, 50, 0x00};
  webo::Image aOverlay;
  aOverlay.width = 8;
  aOverlay.height = 8;
  aOverlay.alpha = webo::AlphaMode::Embedded;
  const auto resetOverlay = [&] {
    aOverlay.rgba.assign(8 * 8 * 4, 0);
    for (std::size_t i = 0; i < aOverlay.rgba.size(); i += 4) {
      aOverlay.rgba[i] = 9; aOverlay.rgba[i + 1] = 8; aOverlay.rgba[i + 2] = 7;
      aOverlay.rgba[i + 3] = 6;
    }
  };
  resetOverlay();
  webo::SpriteFrame aFrame;
  aFrame.left = 0; aFrame.top = 0; aFrame.right = 4; aFrame.bottom = 1;
  aFrame.width = 4; aFrame.height = 1;
  aFrame.uLeft = 0; aFrame.uTop = 0; aFrame.uRight = 1; aFrame.uBottom = 1;
  webo::app::ActorSpritePlacement aQuad{0, 0, 1, 0, 0, 4, 1};
  const auto ap = [&](int x, int c) { return aOverlay.rgba[(0 * 8 + x) * 4 + c]; };
  const std::array<std::uint8_t, 3> aTint{152, 152, 152};
  require(webo::app::drawActorSprite(aOverlay, aFrame, aSheet, aQuad, false, aTint).has_value(),
          "alpha-test draw");
  require(ap(0, 0) == 9 && ap(0, 1) == 8 && ap(0, 2) == 7 && ap(0, 3) == 6,
          "alpha 0xA1 below the reference leaves the destination untouched");
  require(ap(1, 0) == 9 && ap(1, 1) == 8 && ap(1, 2) == 7 && ap(1, 3) == 6,
          "alpha 0xEF at the reference is rejected (strictly greater) and leaves the destination untouched");
  require(ap(2, 0) == 119 && ap(2, 3) == 255,
          "alpha 0xF0 above the reference is drawn (modulated, opaque)");
  require(ap(3, 0) == 9 && ap(3, 3) == 6, "alpha 0x00 texel leaves the destination untouched");

  // Helper and its literal reference describe the same threshold; they come after
  // the raster checks so a mutation fails on destination pixels first.
  require(webo::kActorAlphaTestReference == 0xEF, "alpha reference is 0xEF");
  require(!webo::actorSpriteAlphaTestPasses(0xEF), "alpha 0xEF is rejected (strictly greater)");
  require(webo::actorSpriteAlphaTestPasses(0xF0), "alpha 0xF0 is accepted");
  require(!webo::actorSpriteAlphaTestPasses(0xEE), "alpha 0xEE is rejected");
  require(!webo::actorSpriteAlphaTestPasses(0x00) && webo::actorSpriteAlphaTestPasses(0xFF),
          "alpha 0x00 rejected, 0xFF accepted");

  // Diagnostic path (alphaTest=false) is the pre-A4u write: every texel, texture alpha kept.
  resetOverlay();
  require(webo::app::drawActorSprite(aOverlay, aFrame, aSheet, aQuad, false, aTint, false)
              .has_value(),
          "keep-alpha diagnostic draw");
  require(ap(0, 3) == 0xA1 && ap(1, 3) == 0xEF && ap(2, 3) == 0xF0 && ap(3, 3) == 0x00,
          "diagnostic path keeps texture alpha on every texel");

  // A4v: integer pixel-centre raster (pixel x covered iff left <= x < right, UV
  // (x-left)/(right-left)). 8x1 opaque sheet drawn at 2:1 into [1.3,5.3) x [1.3,2.3)
  // on a sentinel overlay. The old x+0.5 rule covered x=1..4 / y=1 and sampled texels
  // 0/2/4/6; the integer-centre rule covers x=2..5 / y=2 with texels 1/3/5/7.
  webo::Image cSheet;
  cSheet.width = 8;
  cSheet.height = 1;
  for (std::uint8_t i = 0; i < 8; ++i) {
    const std::uint8_t px[4] = {static_cast<std::uint8_t>(10 * (i + 1)), 0, 0, 255};
    cSheet.rgba.insert(cSheet.rgba.end(), px, px + 4);
  }
  webo::Image cOverlay;
  cOverlay.width = 8;
  cOverlay.height = 8;
  cOverlay.alpha = webo::AlphaMode::Embedded;
  cOverlay.rgba.assign(8 * 8 * 4, 0);
  webo::SpriteFrame cFrame;
  cFrame.left = 0; cFrame.top = 0; cFrame.right = 8; cFrame.bottom = 1;
  cFrame.width = 8; cFrame.height = 1;
  cFrame.uLeft = 0; cFrame.uTop = 0; cFrame.uRight = 1; cFrame.uBottom = 1;
  webo::app::ActorSpritePlacement cQuad{0, 0, 1, 1.3, 1.3, 5.3, 2.3};
  require(webo::app::drawActorSprite(cOverlay, cFrame, cSheet, cQuad, false).has_value(),
          "fractional-edge draw");
  const auto cp = [&](int x, int y, int c) { return cOverlay.rgba[(y * 8 + x) * 4 + c]; };
  // Footprint: x in [ceil(1.3), ceil(5.3)) = 2..5, y in [ceil(1.3), ceil(2.3)) = 2.
  require(cp(1, 2, 3) == 0 && cp(6, 2, 3) == 0, "integer-centre coverage leaves x=1 and x=6 empty");
  require(cp(2, 2, 3) == 255 && cp(5, 2, 3) == 255, "integer-centre coverage draws x=2..5");
  require(cp(2, 1, 3) == 0 && cp(2, 3, 3) == 0, "integer-centre coverage draws only y=2");
  // Sampling: texel = floor((x-1.3)/4*8) = 1, 3, 5, 7 for x = 2..5 (R = 10*(i+1)).
  require(cp(2, 2, 0) == 20 && cp(3, 2, 0) == 40 && cp(4, 2, 0) == 60 && cp(5, 2, 0) == 80,
          "integer-centre UV selects texels 1,3,5,7");

  // Same rule on the V axis. The 8x1 sheet above cannot select by V, so use a 1x8
  // sheet with distinct rows (R = 10*(i+1)) drawn at 2:1 into x [1.3,2.3) x y [1.3,5.3).
  // Integer centre covers y=2..5 with v = (y-1.3)/4 -> row floor(v*8) = 1,3,5,7; the
  // old y+0.5 centre would sample rows 2,4,6,7 (v = 0.3,0.55,0.8,1.05 clamped).
  webo::Image vSheet;
  vSheet.width = 1;
  vSheet.height = 8;
  for (std::uint8_t i = 0; i < 8; ++i) {
    const std::uint8_t px[4] = {static_cast<std::uint8_t>(10 * (i + 1)), 0, 0, 255};
    vSheet.rgba.insert(vSheet.rgba.end(), px, px + 4);
  }
  webo::Image vOverlay;
  vOverlay.width = 8;
  vOverlay.height = 8;
  vOverlay.alpha = webo::AlphaMode::Embedded;
  vOverlay.rgba.assign(8 * 8 * 4, 0);
  webo::SpriteFrame vFrame;
  vFrame.left = 0; vFrame.top = 0; vFrame.right = 1; vFrame.bottom = 8;
  vFrame.width = 1; vFrame.height = 8;
  vFrame.uLeft = 0; vFrame.uTop = 0; vFrame.uRight = 1; vFrame.uBottom = 1;
  webo::app::ActorSpritePlacement vQuad{0, 0, 1, 1.3, 1.3, 2.3, 5.3};
  require(webo::app::drawActorSprite(vOverlay, vFrame, vSheet, vQuad, false).has_value(),
          "fractional-top V draw");
  const auto vp = [&](int x, int y, int c) { return vOverlay.rgba[(y * 8 + x) * 4 + c]; };
  require(vp(2, 1, 3) == 0 && vp(2, 2, 3) == 255 && vp(2, 5, 3) == 255 && vp(2, 6, 3) == 0,
          "integer-centre V coverage draws y=2..5 only");
  require(vp(2, 2, 0) == 20 && vp(2, 3, 0) == 40 && vp(2, 4, 0) == 60 && vp(2, 5, 0) == 80,
          "integer-centre V selects rows 1,3,5,7");

  std::cout << "PASS: actor sprite placement, UV mirror, light tint, alpha test and alpha key\n";
  return 0;
}
