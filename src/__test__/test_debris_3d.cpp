// Fast geometry/depth invariants, no gameplay route or original comparison.
#include "app/DeathParticles.h"
#include "app/Raster.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

using namespace webo::app;

int main() {
  DeathParticle chunk;
  chunk.size = 0.8;
  chunk.bornMs = 1000;
  chunk.rotation = {0.3, 0.7};
  chunk.angularVelocity = {3.0, -4.0};
  const auto initial = chunk.chunkOffsets(1000), spun = chunk.chunkOffsets(1350);
  const auto distance = [](const auto& a, const auto& b) {
    double d = 0;
    for (int i = 0; i < 3; ++i) d += (a[i]-b[i])*(a[i]-b[i]);
    return d;
  };
  assert(distance(initial[0],spun[0]) > 0.01);
  for (int i = 0; i < 8; ++i)
    for (int j = 0; j < 8; ++j)
      assert(std::abs(distance(initial[i],initial[j])-distance(spun[i],spun[j])) < 1e-12);
  // Identical gameplay clock (pause) gives identical rigid pose.
  assert(spun == chunk.chunkOffsets(1350));

  Raster raster(16,16);
  raster.clear(0,0,0);
  raster.mutableActorDepth().reset(256);
  std::fill(raster.mutableDepthBuffer().begin(),raster.mutableDepthBuffer().end(),10.0);
  const auto sceneDepth = raster.depthBuffer();
  const Texture terrain{2,2,{255,0,0, 0,255,0, 0,0,255, 255,255,255}};
  const Texture white{1,1,{255,255,255}};
  const std::array<std::array<double,2>,3> uv{{{0,0},{1,0},{0,1}}};
  const auto triangle = [](double z) {
    return std::array<std::array<double,4>,3>{{{1,1,z,1/z},{15,1,z,1/z},{1,15,z,1/z}}};
  };
  // A mesh's rear face cannot cover its nearer face; terrain UVs vary colour.
  raster.triangle(triangle(8),uv,terrain,{1,1,1},100);
  const auto textured = raster.framebuffer().rgb;
  raster.triangle(triangle(9),uv,white,{1,1,1},100);
  assert(textured == raster.framebuffer().rgb);
  const auto a = (3*16+3)*3, b = (3*16+8)*3;
  assert(textured[a] != textured[b] || textured[a+1] != textured[b+1]);
  // A nearer lemming masks mesh pixels. Scenery masks both.
  assert(raster.mutableActorDepth().submit(3*16+3,7,0));
  raster.triangle(triangle(8),uv,white,{1,1,1},100);
  assert(raster.framebuffer().rgb[a] == textured[a]);
  const auto actors = raster.framebuffer().rgb;
  raster.triangle(triangle(11),uv,white,{1,0,0},101);
  assert(actors == raster.framebuffer().rgb);
  assert(sceneDepth == raster.depthBuffer());
  // A nearer mesh covers an actor, with its own depth and without scene writes.
  raster.triangle(triangle(6),uv,white,{0,1,0},101);
  assert(raster.framebuffer().rgb[a] == 0 && raster.framebuffer().rgb[a+1] == 255);
  assert(raster.actorDepth().owns(3*16+3,101));
  assert(sceneDepth == raster.depthBuffer());
  // Ordinary scenery triangles retain their existing depth-write contract.
  Raster scenery(16,16);
  scenery.triangle(triangle(8),uv,white,{1,0,0});
  const auto red = scenery.framebuffer().rgb;
  assert(std::abs(scenery.depthBuffer()[3*16+3] - 8) < 1e-12);
  scenery.triangle(triangle(9),uv,white,{0,1,0});
  assert(red == scenery.framebuffer().rgb);
  scenery.triangle(triangle(7),uv,white,{0,1,0});
  assert(scenery.framebuffer().rgb[a+1] == 255);
  std::cout << "PASS: rigid tumble, terrain UVs, face/actor/scenery depth isolation\n";
}
