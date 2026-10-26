#include "app/SceneDepth.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

using namespace webo::app;

namespace {
void check(bool ok, const char* message) {
  if (!ok) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
SceneRasterOptions options() {
  return {SceneFilter::Point, SceneAddress::Clamp, SceneAlphaFunction::Disabled, 0,
          false, SceneBlendFactor::One, SceneBlendFactor::One,
          SceneDepthFunction::Less, true, SceneShade::Flat,
          SceneRgbOperation::Modulate, SceneAlphaOperation::SelectTexture, .5, .5};
}
void draw(SceneRgbaFramebuffer& fb, std::vector<double>& depth,
          const SceneRgbaImage& tex, const std::array<SceneRasterVertex,3>& v,
          const SceneRasterOptions& state);
std::array<SceneRasterVertex, 3> triangle(double z) {
  return {{{0,0,z,1,{255,255,255,255},.5,.5},
           {1,0,z,1,{255,255,255,255},.5,.5},
           {0,1,z,1,{255,255,255,255},.5,.5}}};
}
std::array<SceneRasterVertex, 3> billboardTriangle(double slotZ,
                                                   const BillboardCameraChain& chain) {
  const std::array<BillboardVertex,3> slots{{{0,0,slotZ,.5,.5},
                                             {1,0,slotZ,.5,.5},
                                             {0,1,slotZ,.5,.5}}};
  return {{billboardRasterVertex(slots[0],chain,{255,255,255,255}),
           billboardRasterVertex(slots[1],chain,{255,255,255,255}),
           billboardRasterVertex(slots[2],chain,{255,255,255,255})}};
}
void sourceAndSubmitOffsetCase() {
  // Custom exact diagonal transforms; generated source-space offsets are
  // deliberately asymmetric and nonzero (a=4, b=8), unlike center-only tests.
  webo::SceneNode node;
  node.type = 2;
  node.type2.branch = 0;
  node.type2.leading = {4,8,8};
  node.type2.branchDoubles = {.1,.8,.2,.9};
  node.matrix = {1,0,0, 0,1,0, 0,0,1, 0,0,40};
  BillboardSourceInputs inputs;
  inputs.parentCamera.values = {1,0,0,0, 0,1,0,0, 0,0,.25,0};
  inputs.sourceTransform.values = {1,0,0,0, 0,1,0,0, 0,0,8,0};
  inputs.projectionTransform.values = {20,0,0,0, 0,20,0,0, 0,0,.25,0};
  inputs.center = {16,16};
  BillboardCameraChain chain{inputs.parentCamera.values,inputs.sourceTransform.values,
                             inputs.projectionTransform.values};
  const auto source = makeSceneBillboardSource(node,inputs);
  check(bool(source), "actual source helper accepts asymmetric synthetic input");
  if (!source) return;

  constexpr std::array<double,4> expectedSlotZ{19.5,19,19,19.5};
  constexpr std::array<double,4> expectedPhysicalZ{39,38,38,39};
  constexpr std::array<std::array<double,2>,4> expectedUv{{{.1,.9},{.1,.2},{.8,.2},{.8,.9}}};
  std::array<SceneRasterVertex,4> submitted{};
  for (std::size_t i=0;i<source->slots.size();++i) {
    const auto& slot=source->slots[i];
    check(slot.z==expectedSlotZ[i], "actual source helper retains each generated point's own slot Z");
    const auto& point=source->generatedPoints[i];
    const double delta=(i==0||i==3)?-2.0:-4.0;
    check(point[2]==80.0+delta, "actual source helper adds source offset after P and S");
    check(slot.u==expectedUv[i][0]&&slot.v==expectedUv[i][1], "native source UV tuple unchanged");
    submitted[i]=billboardRasterVertex(slot,chain,{255,255,255,255});
    check(submitted[i].screenZ==expectedPhysicalZ[i], "actual submission helper yields each equivalent physical corner depth");
    check(submitted[i].screenX==slot.x&&submitted[i].screenY==slot.y&&submitted[i].u==slot.u&&submitted[i].v==slot.v,
          "actual submission helper preserves source slot XY and UV");
    check(submitted[i].rhw==.25/slot.z, "actual submission helper retains RHW from native slot Z");
  }
  check(submitted[0].screenZ!=submitted[1].screenZ, "source-to-submission preserves distinct a/b corner depths");

  // Submit the actual helper-produced vertices against an opaque 38.5-depth
  // screen. Near (38) fragments pass, far (39) fragments fail. A raw normalized
  // slot-Z regression instead paints both sides.
  SceneRgbaFramebuffer fb{32,32,std::vector<std::uint8_t>(32*32*4,0)};
  std::vector<double> depth(32*32,std::numeric_limits<double>::infinity());
  for(std::size_t p=0;p<32*32;++p){fb.rgba[p*4+1]=80;fb.rgba[p*4+3]=255;}
  auto state=options();
  const SceneRgbaImage green{1,1,{0,80,0,255}};
  const std::array<SceneRasterVertex,3> bg1{{{0,0,38.5,1,{255,255,255,255},.5,.5},
                                             {32,0,38.5,1,{255,255,255,255},.5,.5},
                                             {32,32,38.5,1,{255,255,255,255},.5,.5}}};
  const std::array<SceneRasterVertex,3> bg2{{{0,0,38.5,1,{255,255,255,255},.5,.5},
                                             {32,32,38.5,1,{255,255,255,255},.5,.5},
                                             {0,32,38.5,1,{255,255,255,255},.5,.5}}};
  draw(fb,depth,green,bg1,state);draw(fb,depth,green,bg2,state);
  const SceneRgbaImage red{1,1,{220,10,5,255}};
  const auto drawBillboardTri=[&](int i,int j,int k){
    draw(fb,depth,red,{submitted[i],submitted[j],submitted[k]},state);
  };
  drawBillboardTri(0,3,2);drawBillboardTri(0,2,1);
  const auto pixel=[&](int x,int y){return fb.rgba.data()+(static_cast<std::size_t>(y)*32+x)*4;};
  check(pixel(16,12)[0]==0&&pixel(16,12)[1]==80,
        "source-to-submission far side remains behind opaque 38.5 reference");
  check(pixel(16,19)[0]==220&&pixel(16,19)[1]==10,
        "source-to-submission near side occludes opaque 38.5 reference");
}
void draw(SceneRgbaFramebuffer& fb, std::vector<double>& depth,
          const SceneRgbaImage& tex, const std::array<SceneRasterVertex,3>& v,
          const SceneRasterOptions& state) {
  const auto result = rasterizeSceneTriangle(fb, depth, tex, v, state);
  check(bool(result), "rasterize value fixture");
}
}

int main() {
  sourceAndSubmitOffsetCase();
  // Independent accepted Z operands, binary32-context values from the
  // derivation packet. Do not calculate expected values from the helper.
  BillboardCameraChain chain{};
  constexpr double pz = 0.0004687500186264515;
  constexpr double sz = 2133.333251953125;
  chain.sourceTransform[10] = sz;
  chain.projectionTransform[10] = pz;
  constexpr double k = 0.0004687500193715089;
  const SceneRgbaImage opaque{1,1,{20,30,40,255}};
  const SceneRgbaImage billboard{1,1,{220,10,5,255}};
  SceneRgbaFramebuffer fb{1,1,{0,0,0,255}};
  std::vector<double> depth{std::numeric_limits<double>::infinity()};
  auto state = options();
  draw(fb, depth, opaque, triangle(5.0), state);
  const auto far = billboardRasterVertex({0,0,k*10,.5,.5}, chain,
                                          {255,255,255,255});
  draw(fb, depth, billboard, billboardTriangle(k*10,chain), state);
  check(fb.rgba == std::vector<std::uint8_t>({20,30,40,255}) && depth[0] == 5.0,
        "far normalized billboard cannot overpaint nearer opaque reference");
  check(far.screenZ == 10.0 && far.rhw == 0.25/(k*10),
        "integration adapter converts comparison Z but retains native-slot RHW");

  fb.rgba = {0,0,0,255}; depth[0] = std::numeric_limits<double>::infinity();
  draw(fb, depth, opaque, triangle(5.0), state);
  draw(fb, depth, billboard, billboardTriangle(k*2,chain), state);
  check(fb.rgba == std::vector<std::uint8_t>({220,10,5,255}) && depth[0] == 2.0,
        "near billboard overpaints farther opaque reference and writes physical depth");

  check(std::abs(billboardPhysicalComparisonDepth(k, chain) - 1.0) < 1e-12,
        "exact-scale slot value converts to independent physical depth 1");
  check(std::abs(billboardPhysicalComparisonDepth(0.004687500193715089, chain) - 10.0) < 1e-11,
        "exact-scale slot value converts to independent physical depth 10");
  check(std::abs(billboardPhysicalComparisonDepth(0.04687500193715089, chain) - 100.0) < 1e-10,
        "exact-scale slot value converts to independent physical depth 100");

  // Mutation discriminator: submitting the same native normalized slot Z
  // directly instead of the adapter's physical comparison Z reverses ordering.
  fb.rgba = {0,0,0,255}; depth[0] = std::numeric_limits<double>::infinity();
  draw(fb, depth, opaque, triangle(5.0), state);
  auto rawNormalized = far;
  rawNormalized.screenZ = k*10;
  auto raw = billboardTriangle(k*10,chain);
  for(auto& v:raw) v.screenZ=rawNormalized.screenZ;
  draw(fb, depth, billboard, raw, state);
  check(fb.rgba == std::vector<std::uint8_t>({220,10,5,255}),
        "raw normalized-Z mutation demonstrably fails the far-occlusion expectation");
  std::cout << "SceneDepth exact-scale and shared-depth occlusion checks passed\n";
}
