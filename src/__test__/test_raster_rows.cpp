#include "app/Raster.h"
#include "app/RasterRows.h"
#include "app/SceneRaster.h"

#include <cassert>
#include <limits>
#include <utility>

using namespace webo::app;

namespace {
template<class T>
concept TriangleTexture = requires(Raster& raster,
    const std::array<std::array<double, 4>, 3>& p,
    const std::array<std::array<double, 2>, 3>& uv, T&& texture) {
  raster.triangle(p, uv, std::forward<T>(texture), {1, 1, 1});
};

// Queued triangle draws must never borrow a temporary Texture, including the
// prvalue produced by `tile ? tile->texture : Texture{}` in the surface path.
static_assert(TriangleTexture<Texture&> && TriangleTexture<const Texture&>);
static_assert(!TriangleTexture<Texture> && !TriangleTexture<const Texture>);
}  // namespace

int main() {
  constexpr int w = 320, h = 240;
  SceneRgbaImage texture{3, 2, {1,2,3,255, 200,33,4,200, 250,128,5,0,
                               6,77,88,255, 99,100,111,127, 123,234,45,255}};
  SceneRgbaFramebuffer serial{w,h,std::vector<std::uint8_t>(w*h*4,43)}, parallel=serial;
  std::vector<double> zs(w*h,999), zp=zs, stores(w*h,999), observed=stores;
  std::vector<std::uint8_t> mask(w*h,1);
  SceneRasterObserver observer;
  observer.mask=&mask;
  observer.accepted=[&](const SceneRasterWriteEvent& event) {
    if(event.depthAssigned) observed[event.pixel]=event.depthAfter;
  };
  const auto draw=[&](auto& image,auto& depth,bool batch) {
    detail::RasterCommandBatch commands(h,batch);
    for(int i=0;i<24;++i) {
      const double z=40-i;
      std::array<SceneRasterVertex,3> p{{
        {-12.,-8.,z,1./z,{255,127,42,200},-2.2,0.3},
        {333.,40.,z+2,1./(z+2),{127,255,88,255},3.1,-0.4},
        {60.,250.,z+4,1./(z+4),{60,90,255,150},0.7,2.9}}};
      SceneRasterOptions o{SceneFilter::Linear,SceneAddress::Wrap,
        SceneAlphaFunction::StrictGreater,10,true,SceneBlendFactor::SrcAlpha,
        SceneBlendFactor::InvSrcAlpha,SceneDepthFunction::Less,true,
        SceneShade::Gouraud,SceneRgbOperation::Modulate,SceneAlphaOperation::Modulate,0.5,0.5};
      o.filter=i%2?SceneFilter::Point:SceneFilter::Linear;
      o.address=i%4<2?SceneAddress::Clamp:SceneAddress::Wrap;
      o.alphaFunction=i%3?SceneAlphaFunction::StrictGreater:SceneAlphaFunction::Disabled;
      o.blendEnabled=i%3!=0;
      o.sourceBlend=static_cast<SceneBlendFactor>(i%3);
      o.destinationBlend=static_cast<SceneBlendFactor>((i+1)%3);
      o.depthFunction=static_cast<SceneDepthFunction>(i%8);
      o.depthWrite=i%4!=0;
      o.shade=i%2?SceneShade::Flat:SceneShade::Gouraud;
      o.alphaOperation=i%2?SceneAlphaOperation::SelectTexture:SceneAlphaOperation::Modulate;
      if(batch)o.depthOutput=stores;
      auto result=rasterizeSceneTriangle(image,depth,texture,p,o,batch?nullptr:&observer);
      assert(result);
    }
    const auto errors = commands.flush();
    assert(errors.empty());
  };
  draw(serial,zs,false);draw(parallel,zp,true);
  std::vector<std::uint8_t> rgb(w*h*3,43);
  SceneRgbaFramebuffer directRgb{w,h,{},rgb};
  std::vector<double> zr(w*h,999);
  draw(directRgb,zr,true);
  assert(zr==zs);
  for(std::size_t i=0;i<zr.size();++i)for(int c=0;c<3;++c)
    assert(rgb[i*3+c]==serial.rgba[i*4+c]);
  assert(serial.rgba==parallel.rgba && zs==zp && observed==stores);

  Texture tile{1,1,{64,128,255}};
  using P=std::array<std::array<double,4>,3>;
  const std::array<std::array<double,2>,3> uv{{{0,0},{1,0},{0,1}}};
  const auto triangles=[&](Raster& r) {
    for(int i=0;i<12;++i) {
      const double z=100-i;
      const P p{{{0,0,z,1/z},{320,0,z,1/z},{0,240,z,1/z}}};
      r.triangle(p,uv,tile,{1,0.8,0.7});
    }
  };
  Raster a(w,h),b(w,h),layer(w,h);
  a.clear(3,4,5);b.clear(3,4,5);layer.clear(0,0,0);
  for(std::size_t i=0;i<a.depthBuffer().size();++i) {
    // Occluded, equal and exposed fragments against pre-existing scenery.
    a.mutableDepthBuffer()[i]=b.mutableDepthBuffer()[i]=i%3==0?50:i%3==1?89:999;
  }
  triangles(a);
  {
    detail::RasterCommandBatch commands(h);
    triangles(layer);
    const auto errors = commands.flush();
    assert(errors.empty());
  }
  b.compositeOpaque(layer);
  assert(a.framebuffer().rgb==b.framebuffer().rgb && a.depthBuffer()==b.depthBuffer());
  assert(layer.drawnTriangles()==12 && layer.drawnPixels()>0);
}
