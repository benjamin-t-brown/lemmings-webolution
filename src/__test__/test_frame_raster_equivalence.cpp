// Differential test against this task's authentic source-built clone baseline.
#include "app/Raster.h"
#include "app/SceneRaster.h"
#include "frame_profile_reference.h"
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

namespace now = webo::app;
namespace old = webo_frame_reference::app;
int main() {
  std::mt19937 random(0x50495845);
  const auto unit = [&] { return double(random() % 65536) / 65535.0; };
  for (int trial = 0; trial < 640; ++trial) {
    const int widths[]{1,2,3,7,16,31,64};
    const int tw = widths[trial % 7], th = widths[(trial / 7) % 7];
    now::SceneRgbaImage tex{tw,th,std::vector<std::uint8_t>(tw*th*4)};
    for (auto& c : tex.rgba) c = random() & 255;
    old::SceneRgbaImage refTex{tw,th,tex.rgba};
    now::SceneRgbaFramebuffer image{48,32,std::vector<std::uint8_t>(48*32*4)};
    for (auto& c : image.rgba) c = random() & 255;
    old::SceneRgbaFramebuffer refImage{48,32,image.rgba};
    std::vector<double> depth(48*32, (trial & 1) ? 10 : std::numeric_limits<double>::infinity());
    auto refDepth = depth;
    std::array<now::SceneRasterVertex,3> vertices{};
    std::array<old::SceneRasterVertex,3> refVertices{};
    for (int i = 0; i < 3; ++i) {
      auto& v = vertices[i];
      v.screenX = unit()*96-24; v.screenY = unit()*64-16;
      v.screenZ = unit()*12+1; v.rhw = 1.0/v.screenZ;
      for (double& c : v.diffuse) c = unit()*255;
      // Exercise exact edges, negative wrap, NPOT and the supported UV limits.
      const double uv[]{-1e6,-1,-.5,0,.5,1,1e6};
      v.u = trial % 3 ? unit()*12-6 : uv[(trial+i)%7];
      v.v = trial % 4 ? unit()*12-6 : uv[(trial+2*i)%7];
      refVertices[i] = {v.screenX,v.screenY,v.screenZ,v.rhw,v.diffuse,v.u,v.v};
    }
    now::SceneRasterOptions o{
      now::SceneFilter(trial%2),now::SceneAddress((trial/2)%2),
      now::SceneAlphaFunction((trial/4)%2),127,bool((trial/8)%2),
      now::SceneBlendFactor((trial/16)%3),now::SceneBlendFactor((trial/48)%3),
      now::SceneDepthFunction((trial/9)%8),bool((trial/5)%2),now::SceneShade((trial/7)%2),
      now::SceneRgbOperation::Modulate,now::SceneAlphaOperation((trial/11)%2),.5,.5};
    old::SceneRasterOptions ro{
      old::SceneFilter(o.filter),old::SceneAddress(o.address),old::SceneAlphaFunction(o.alphaFunction),
      o.alphaReference,o.blendEnabled,old::SceneBlendFactor(o.sourceBlend),old::SceneBlendFactor(o.destinationBlend),
      old::SceneDepthFunction(o.depthFunction),o.depthWrite,old::SceneShade(o.shade),
      old::SceneRgbOperation(o.rgbOperation),old::SceneAlphaOperation(o.alphaOperation),o.sampleOffsetX,o.sampleOffsetY};
    const auto a = now::rasterizeSceneTriangle(image,depth,tex,vertices,o);
    const auto b = old::rasterizeSceneTriangle(refImage,refDepth,refTex,refVertices,ro);
    if (bool(a)!=bool(b) || image.rgba!=refImage.rgba || depth!=refDepth)
      throw std::runtime_error("SceneRaster differs at trial " + std::to_string(trial));

    now::Raster tube(48,32); old::Raster refTube(48,32);
    tube.clear(17,29,41); refTube.clear(17,29,41);
    tube.setDepthEnabled(trial&1); refTube.setDepthEnabled(trial&1);
    now::Texture tile{tw,th,std::vector<std::uint8_t>(tw*th*3)};
    for (auto& c : tile.rgb) c = random() & 255;
    old::Texture refTile{tw,th,tile.rgb};
    std::array<std::array<double,4>,3> p{};
    std::array<std::array<double,2>,3> uv{};
    for (int i=0;i<3;++i) {
      const auto& v=vertices[i]; p[i]={v.screenX,v.screenY,v.screenZ,v.rhw}; uv[i]={v.u,v.v};
    }
    const std::array<double,3> tint{unit(),unit(),unit()};
    tube.triangle(p,uv,tile,tint); refTube.triangle(p,uv,refTile,tint);
    if (tube.framebuffer().rgb!=refTube.framebuffer().rgb || tube.depthBuffer()!=refTube.depthBuffer())
      throw std::runtime_error("Raster differs at trial " + std::to_string(trial));
  }
  std::cout << "640 SceneRaster + 640 tube triangles: exact baseline colour/depth equality\n";
}
