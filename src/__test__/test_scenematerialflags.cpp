#include "app/SceneMaterialFlags.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

using namespace webo::app;

int main() {
  int checks = 0, failures = 0;
  const auto check = [&](bool ok, const char* label) {
    ++checks;
    if (!ok) { ++failures; std::cerr << "FAIL " << label << '\n'; }
  };

  SceneRasterOptions base{SceneFilter::Point, SceneAddress::Clamp,
      SceneAlphaFunction::StrictGreater, 239, true, SceneBlendFactor::SrcAlpha,
      SceneBlendFactor::InvSrcAlpha, SceneDepthFunction::Less, true,
      SceneShade::Gouraud, SceneRgbOperation::Modulate,
      SceneAlphaOperation::Modulate, .5, .5};
  const auto policy = conditionalCaretFlags8Policy(base);
  check(policy.alphaFunction == SceneAlphaFunction::Disabled &&
        policy.alphaOperation == SceneAlphaOperation::SelectTexture,
        "caret flags8 selects disabled alpha test and texture alpha");
  check(policy.filter == base.filter && policy.address == base.address &&
        policy.alphaReference == base.alphaReference &&
        policy.blendEnabled == base.blendEnabled &&
        policy.sourceBlend == base.sourceBlend &&
        policy.destinationBlend == base.destinationBlend &&
        policy.depthFunction == base.depthFunction && policy.depthWrite == base.depthWrite &&
        policy.shade == base.shade && policy.rgbOperation == base.rgbOperation &&
        policy.sampleOffsetX == base.sampleOffsetX && policy.sampleOffsetY == base.sampleOffsetY,
        "caret flags8 changes only the two accepted alpha fields");

  // One covered destination pixel; asymmetric diffuse alpha distinguishes
  // SELECT_TEXTURE (128) from MODULATE (128*64/255).
  const SceneRgbaImage tex{1, 1, {200, 100, 50, 128}};
  std::array<SceneRasterVertex, 3> tri{{
      {-1, -1, .25, 1, {255, 255, 255, 64}, .5, .5},
      { 3, -1, .25, 1, {255, 255, 255, 64}, .5, .5},
      {-1,  3, .25, 1, {255, 255, 255, 64}, .5, .5}}};
  const auto render = [&](SceneRasterOptions opts) {
    SceneRgbaFramebuffer dst{1, 1, {20, 40, 60, 80}};
    std::vector<double> z{1.0};
    const auto result = rasterizeSceneTriangle(dst, z, tex, tri, opts);
    check(bool(result), "rasterize 1x1 destination fixture");
    return dst.rgba;
  };
  const std::array<std::uint8_t, 4> expected{110, 70, 55, 104};
  const auto actual = render(policy);
  check(std::equal(actual.begin(), actual.end(), expected.begin()),
        "intermediate texture alpha 128 survives as literal blended destination pixel");

  // Mutate alpha-test alone: SELECT_TEXTURE alpha 128 is rejected by the old
  // strict >239 test. No other field is changed.
  auto oldTestOnly = policy;
  oldTestOnly.alphaFunction = SceneAlphaFunction::StrictGreater;
  const auto rejected = render(oldTestOnly);
  check(rejected == std::vector<std::uint8_t>({20, 40, 60, 80}),
        "isolated old strict alpha-test field rejects the destination pixel");

  // Mutate alpha-op alone: alpha becomes 128*64/255, producing a distinct
  // destination despite the alpha test remaining disabled.
  auto oldOpOnly = policy;
  oldOpOnly.alphaOperation = SceneAlphaOperation::Modulate;
  const auto modulated = render(oldOpOnly);
  check(modulated == std::vector<std::uint8_t>({43, 48, 59, 74}),
        "isolated old alpha-op field has its independently predicted pixel");

  std::cout << checks << " checks, " << failures << " failures\n";
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
