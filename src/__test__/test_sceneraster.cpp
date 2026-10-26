#include "app/SceneRaster.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using namespace webo::app;

namespace {
void check(bool ok, const std::string& message) {
  if (!ok) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
SceneRasterOptions options() {
  return {SceneFilter::Point, SceneAddress::Clamp, SceneAlphaFunction::Disabled, 0,
          false, SceneBlendFactor::One, SceneBlendFactor::One,
          SceneDepthFunction::Always, true, SceneShade::Flat,
          SceneRgbOperation::Modulate,
          SceneAlphaOperation::SelectTexture, .5, .5};
}
std::array<SceneRasterVertex,3> triangle(double u=0.25, double v=0.5) {
  SceneRasterVertex a{0,0,2,1,{255,255,255,255},u,v};
  SceneRasterVertex b{2,0,2,1,{255,255,255,255},u,v};
  SceneRasterVertex c{0,2,2,1,{255,255,255,255},u,v};
  return {a,b,c};
}
SceneRgbaImage tex(std::vector<std::uint8_t> pixels, int w, int h=1) {
  return {w,h,std::move(pixels)};
}
void draw(SceneRgbaFramebuffer& fb, std::vector<double>& z, const SceneRgbaImage& t,
          const std::array<SceneRasterVertex,3>& p, const SceneRasterOptions& o) {
  auto result = rasterizeSceneTriangle(fb,z,t,p,o);
  check(bool(result), result ? "" : result.error());
}
}

int main() {
  const auto two = tex({10,20,30,40, 210,120,60,200},2);
  {
    auto o=options(); SceneRgbaFramebuffer fb{1,1,{0,0,0,0}}; std::vector<double> z{9};
    draw(fb,z,two,triangle(.25),o);
    check(fb.rgba==std::vector<std::uint8_t>({10,20,30,40}),"POINT picks floor(U*width)");
    o.filter=SceneFilter::Linear;
    draw(fb,z,two,triangle(.5),o);
    check(fb.rgba==std::vector<std::uint8_t>({110,70,45,120}),"LINEAR midpoint bilinear RGBA");
    o.filter=SceneFilter::Point; o.address=SceneAddress::Wrap;
    draw(fb,z,two,triangle(-.25),o);
    check(fb.rgba[0]==210,"negative U wraps by mathematical modulo");
    o.address=SceneAddress::Clamp;
    draw(fb,z,two,triangle(-.25),o);
    check(fb.rgba[0]==10,"negative U clamps to edge");
  }
  {
    auto o=options(); o.alphaFunction=SceneAlphaFunction::StrictGreater; o.alphaReference=127;
    const auto one=tex({90,80,70,127},1); SceneRgbaFramebuffer fb{1,1,{3,4,5,6}};
    std::vector<double> z{7}; draw(fb,z,one,triangle(.5),o);
    check(fb.rgba==std::vector<std::uint8_t>({3,4,5,6}) && z[0]==7,"alpha equality rejects and never writes depth");
    const auto above=tex({90,80,70,128},1); draw(fb,z,above,triangle(.5),o);
    check(fb.rgba[0]==90 && z[0]==2,"alpha strictly above ref accepted and writes depth");
  }
  {
    auto o=options(); const auto one=tex({100,90,80,128},1);
    SceneRgbaFramebuffer fb{1,1,{1,2,3,4}}; std::vector<double> z{8}; o.depthWrite=false;
    draw(fb,z,one,triangle(.5),o);
    check(z[0]==8,"depth write disabled");
    o.depthWrite=true; draw(fb,z,one,triangle(.5),o);
    check(z[0]==2,"depth write enabled");
    o.depthFunction=SceneDepthFunction::Less; draw(fb,z,one,triangle(.5),o);
    check(fb.rgba[0]==100,"equal depth fails LESS");
    o.depthFunction=SceneDepthFunction::LessEqual; draw(fb,z,one,triangle(.5),o);
    check(fb.rgba[0]==100,"equal depth passes LESS_EQUAL");
  }
  {
    auto o=options(); o.blendEnabled=true; o.sourceBlend=SceneBlendFactor::SrcAlpha;
    o.destinationBlend=SceneBlendFactor::InvSrcAlpha;
    const auto one=tex({200,100,50,128},1); SceneRgbaFramebuffer fb{1,1,{20,40,60,80}};
    std::vector<double> z{8}; draw(fb,z,one,triangle(.5),o);
    check(fb.rgba==std::vector<std::uint8_t>({110,70,55,104}),"SRCALPHA/INVSRCALPHA blends RGBA independently");
    o.sourceBlend=SceneBlendFactor::One; o.destinationBlend=SceneBlendFactor::One;
    draw(fb,z,one,triangle(.5),o);
    check(fb.rgba==std::vector<std::uint8_t>({255,170,105,232}),"ONE/ONE additive blend clamps/rounds");
  }
  {
    auto o=options(); const auto one=tex({200,100,50,128},1);
    auto p=triangle(.5); p[0].diffuse={255,255,255,255}; p[1].diffuse={0,0,0,0}; p[2].diffuse={0,0,0,0};
    SceneRgbaFramebuffer flat{1,1,{0,0,0,0}}; std::vector<double> z{8};
    draw(flat,z,one,p,o);
    check(flat.rgba==std::vector<std::uint8_t>({200,100,50,128}),"flat diffuse uses vertex zero; alpha SELECTTEXTURE");
    o.shade=SceneShade::Gouraud;
    SceneRgbaFramebuffer gouraud{1,1,{0,0,0,0}}; z[0]=8; draw(gouraud,z,one,p,o);
    check(gouraud.rgba==std::vector<std::uint8_t>({100,50,25,128}),"Gouraud weights at center are .5/.25/.25");
    o.alphaOperation=SceneAlphaOperation::Modulate;
    z[0]=8; draw(gouraud,z,one,p,o);
    check(gouraud.rgba[3]==64,"alpha MODULATE uses interpolated diffuse alpha");
    // Perspective UV: weighted u*rhw / weighted rhw = .125/.6875 ~= .1818,
    // while affine UV would be .25 (the adjacent point-sampled texel).
    auto q=triangle(0); q[0].u=0; q[1].u=1; q[1].rhw=.5; q[2].rhw=.25;
    o.alphaOperation=SceneAlphaOperation::SelectTexture; o.shade=SceneShade::Flat;
    o.filter=SceneFilter::Point; o.address=SceneAddress::Clamp;
    const auto uvTex=tex({10,0,0,255, 40,0,0,255, 100,0,0,255, 200,0,0,255},4);
    SceneRgbaFramebuffer perspective{1,1,{0,0,0,0}}; z[0]=8;
    draw(perspective,z,uvTex,q,o);
    check(perspective.rgba[0]==10,"supplied RHW drives perspective-correct UV");
    q[0].screenZ=1; q[1].screenZ=5; q[2].screenZ=9;
    o.depthFunction=SceneDepthFunction::Always; z[0]=100;
    draw(perspective,z,uvTex,q,o);
    check(std::abs(z[0]-4.0)<1e-12,"depth is screen-linear, not perspective transformed");
  }
  {
    auto o=options(); auto p=triangle(.5); SceneRgbaFramebuffer fb{1,1,{1,2,3,4}};
    std::vector<double> z{9}; auto one=tex({5,6,7,8},1);
    auto bad=rasterizeSceneTriangle(fb,z,one,p,o); check(bool(bad),"valid baseline");
    auto malformed=one; malformed.rgba.pop_back();
    check(!rasterizeSceneTriangle(fb,z,malformed,p,o),"reject malformed texture storage");
    malformed=one; malformed.width=0;
    check(!rasterizeSceneTriangle(fb,z,malformed,p,o),"reject zero-sized texture");
    std::vector<double> badDepth;
    check(!rasterizeSceneTriangle(fb,badDepth,one,p,o),"reject mismatched depth storage");
    SceneRgbaFramebuffer badTarget{0,1,{}};
    check(!rasterizeSceneTriangle(badTarget,z,one,p,o),"reject zero-sized target");
    p[0].rhw=0; check(!rasterizeSceneTriangle(fb,z,one,p,o),"reject zero RHW"); p=triangle(.5);
    p[0].u=std::numeric_limits<double>::infinity(); check(!rasterizeSceneTriangle(fb,z,one,p,o),"reject non-finite UV"); p=triangle(.5);
    o.sampleOffsetX=1; check(!rasterizeSceneTriangle(fb,z,one,p,o),"reject invalid sample convention");
    o=options(); p[1].screenX=p[0].screenX; p[2].screenX=p[0].screenX;
    p[1].screenY=p[0].screenY; p[2].screenY=p[0].screenY;
    fb.rgba={1,2,3,4}; z[0]=9; draw(fb,z,one,p,o);
    check(fb.rgba==std::vector<std::uint8_t>({1,2,3,4}) && z[0]==9,"degenerate triangle is harmless no-op");
  }
  {
    auto o=options(); o.depthFunction=SceneDepthFunction::Less; o.depthWrite=true;
    SceneRgbaFramebuffer fb{1,1,{7,8,9,10}}; std::vector<double> z{1};
    const auto one=tex({90,80,70,255},1); const std::vector<std::uint8_t> mask{1};
    std::vector<SceneRasterWriteEvent> events;
    SceneRasterObserver obs{&mask,0x59644,0,[&](const auto& e){events.push_back(e);},{}};
    auto r=rasterizeSceneTriangle(fb,z,one,triangle(.5),o,&obs);
    check(bool(r)&&events.empty(),"depth rejection creates no accepted-write event");
    o.depthFunction=SceneDepthFunction::Always; o.alphaFunction=SceneAlphaFunction::StrictGreater; o.alphaReference=255;
    r=rasterizeSceneTriangle(fb,z,one,triangle(.5),o,&obs);
    check(bool(r)&&events.empty(),"alpha rejection creates no accepted-write event");
    o.alphaFunction=SceneAlphaFunction::Disabled; o.depthWrite=false;
    r=rasterizeSceneTriangle(fb,z,one,triangle(.5),o,&obs);
    check(bool(r)&&events.size()==1&&!events.back().depthAssigned&&events.back().depthBefore==1&&events.back().depthAfter==1,
          "accepted color assignment is logged with depthWrite=false");
    const auto unchanged=fb.rgba;
    r=rasterizeSceneTriangle(fb,z,one,triangle(.5),o,&obs);
    check(bool(r)&&events.size()==2&&events[0].pixel==events[1].pixel&&fb.rgba==unchanged,
          "repeated unchanged assignment counts twice but has one unique pixel");
    o.depthWrite=true;
    r=rasterizeSceneTriangle(fb,z,one,triangle(.5),o,&obs);
    check(bool(r)&&events.size()==3&&events.back().depthAssigned&&events.back().depthAfter==2,
          "depthWrite=true logs actual depth assignment");
    const std::vector<std::uint8_t> outside{0}; obs.mask=&outside;
    r=rasterizeSceneTriangle(fb,z,one,triangle(.5),o,&obs);
    check(bool(r)&&events.size()==3,"outside-mask accepted writes are not collected");
  }
  std::cout << "SceneRaster explicit-state synthetic checks passed\n";
}
