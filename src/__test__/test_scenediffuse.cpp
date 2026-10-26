#include "app/SceneDiffuse.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace webo::app;

int main() {
  struct Case { std::array<double,4> raw; std::uint32_t word; };
  constexpr Case cases[] = {
      {{{0,0,0,0}}, 0x00000000u},
      {{{1,1,1,1}}, 0x01010101u},
      {{{0.5,0.5,0.5,0.5}}, 0x00000000u},
      {{{1,0.5,-1,1}}, 0xffffffffu},
      {{{256,0,0,1}}, 0x01000000u},
  };
  for (const auto& c : cases) {
    auto word = packSceneDiffuseWord(c.raw);
    assert(word && *word == c.word);
  }

  struct RasterCase {
    const char* name;
    std::array<double,4> raw;
    std::array<double,4> rgba;
  };
  constexpr RasterCase rasterCases[] = {
      {"zero", {0,0,0,0}, {0,0,0,0}},
      {"one", {1,1,1,1}, {1,1,1,1}},
      {"fraction", {0.5,0.5,0.5,0.5}, {0,0,0,0}},
      {"signed-spill", {1,0.5,-1,1}, {255,255,255,255}},
  };
  const SceneRasterOptions state{SceneFilter::Point, SceneAddress::Wrap,
      SceneAlphaFunction::Disabled, 0, false, SceneBlendFactor::One,
      SceneBlendFactor::InvSrcAlpha, SceneDepthFunction::Less,
      true, SceneShade::Gouraud, SceneRgbOperation::Modulate,
      SceneAlphaOperation::Modulate, 0.5, 0.5};
  const SceneRgbaImage white{1,1,{255,255,255,255}};
  bool allPass = true;
  for (const auto& c : rasterCases) {
    const auto v0 = adaptSceneMeshVertex(0,0,1,1,0.5,0.5,c.raw);
    const auto v1 = adaptSceneMeshVertex(2,0,1,1,0.5,0.5,c.raw);
    const auto v2 = adaptSceneMeshVertex(0,2,1,1,0.5,0.5,c.raw);
    assert(v0 && v1 && v2);
    const bool fieldsPass = v0->screenX == 0 && v0->screenY == 0 && v0->screenZ == 1 &&
        v0->rhw == 1 && v0->u == 0.5 && v0->v == 0.5;
    const bool adapterPass = fieldsPass && v0->diffuse == c.rgba;

    SceneRgbaFramebuffer target{2,2,std::vector<std::uint8_t>(16,0)};
    std::vector<double> depth(4, 100.0);
    auto rendered = rasterizeSceneTriangle(target, depth, white, {*v0,*v1,*v2}, state);
    assert(rendered);
    const std::array<std::uint8_t,4> expected{
        static_cast<std::uint8_t>(c.rgba[0]), static_cast<std::uint8_t>(c.rgba[1]),
        static_cast<std::uint8_t>(c.rgba[2]), static_cast<std::uint8_t>(c.rgba[3])};
    const bool pixelPass = std::array<std::uint8_t,4>{
        target.rgba[0],target.rgba[1],target.rgba[2],target.rgba[3]} == expected;
    std::cout << c.name << ": adapter=" << adapterPass << " destination_RGBA=" << pixelPass << '\n';
    allPass = allPass && adapterPass && pixelPass;
  }
  return allPass ? 0 : 1;
}
