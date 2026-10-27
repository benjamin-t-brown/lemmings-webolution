#include "app/SceneDepth.h"
#include "app/SceneDiffuse.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>

using namespace webo::app;

int main() {
  // Asymmetric SOURCE fixture. Independent expectations are binary32 rounding
  // followed by the documented native-to-physical decode.
  constexpr double scale = 0.00046874998952262104;
  constexpr double z = 1733.2718281828459;
  constexpr double x = 17.123456789, y = 91.987654321;
  const std::array<double, 4> color{203.75, 31.25, 117.875, 255.0};
  auto packed = adaptPackedSceneMeshVertex(x, y, z, scale, 0.0, color,
                                            0.123456789, 0.876543211);
  assert(packed);
  const float expectedX = static_cast<float>(x);
  const float expectedY = static_cast<float>(y);
  const float expectedNativeZ = static_cast<float>(scale * z);
  const float expectedRhw = 0.25f / expectedNativeZ;
  const float expectedU = static_cast<float>(0.123456789);
  const float expectedV = static_cast<float>(0.876543211);
  assert(packed->screenX == expectedX && packed->screenY == expectedY);
  assert(packed->screenZ == static_cast<double>(expectedNativeZ) / scale);
  assert(packed->rhw == expectedRhw);
  assert(packed->u == expectedU && packed->v == expectedV);
  auto priorColor = adaptSceneMeshVertex(x, y, z, 0.25 / z, 0.123456789,
                                         0.876543211, color);
  assert(priorColor && packed->diffuse == priorColor->diffuse);

  constexpr double bias = 0.00003125;
  auto biased = adaptPackedSceneMeshVertex(x, y, z, scale, bias, color,
                                           0.123456789, 0.876543211);
  assert(biased);
  const float expectedBiasedNativeZ = static_cast<float>(scale * z - bias);
  assert(biased->screenZ == static_cast<double>(expectedBiasedNativeZ) / scale);
  assert(biased->screenZ !=
         static_cast<double>(expectedBiasedNativeZ + static_cast<float>(bias)) / scale);

  // Behavioral output oracle through the exact adapter consumed by
  // BackdropScene: packed geometry/depth must differ from the former doubles.
  const SceneRasterVertex old{x, y, z, 0.25 / z, color, 0.123456789, 0.876543211};
  assert(old.screenX != packed->screenX || old.screenY != packed->screenY ||
         old.screenZ != packed->screenZ || old.rhw != packed->rhw ||
         old.u != packed->u || old.v != packed->v);
  const SceneRasterOptions options{SceneFilter::Point, SceneAddress::Clamp,
      SceneAlphaFunction::Disabled, 0, false, SceneBlendFactor::One,
      SceneBlendFactor::One, SceneDepthFunction::Always, true,
      SceneShade::Gouraud, SceneRgbOperation::Modulate,
      SceneAlphaOperation::Modulate, 0.5, 0.5};
  const SceneRgbaImage tex{1, 1, {255, 255, 255, 255}};
  auto render = [&](const SceneRasterVertex& center) {
    SceneRgbaFramebuffer fb{128, 128, std::vector<std::uint8_t>(128 * 128 * 4, 0)};
    std::vector<double> depth(128 * 128, 1e9);
    std::array<SceneRasterVertex, 3> tri{center, center, center};
    tri[0].screenX -= 19.125; tri[0].screenY -= 13.75;
    tri[1].screenX += 21.375; tri[1].screenY -= 9.25;
    tri[2].screenX += 3.625; tri[2].screenY += 24.875;
    assert(rasterizeSceneTriangle(fb, depth, tex, tri, options));
    return fb.rgba;
  };
  assert(render(old) != render(*packed));
  std::cout << "production mesh pack adapter fields and physical depth pass\n";
}
