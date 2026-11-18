// Short presentation invariants, using installed Just Bomb art. Synthetic
// explosions are supplemental; no new gameplay route or fidelity assertion.
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>

using namespace webo;
using namespace webo::app;

int main(int argc, char** argv) {
  assert(argc > 1);
  const std::filesystem::path game(argv[1]);
  auto lib = AssetLibrary::open(game/"lemmings.box", game/"patch.box"); assert(lib);
  auto bytes = lib->read("COMPILED\\MM_BOMB.LVL"); assert(bytes);
  auto mesh = findCylinderMesh(*bytes); auto index = indexLevel(*bytes); assert(mesh && index);
  auto scene = TubeScene::build(*lib,*mesh,*bytes,index->grid0); assert(scene);
  sim::Lemming l; l.x=49; l.y=63; l.state=sim::State::Dead;
  scene->emitDeathParticles(l,false,{},1000,96,*lib);
  assert(scene->explosionDecalCount()==0); // splats do not scorch
  scene->emitDeathParticles(l,true,{},1000,96,*lib);
  assert(scene->explosionDecalCount()==1); // also when the blast removed no earth
  CameraParams params; params.yawUnits=49; params.fixedPitchTurns=0;
  params.levelEyeHeight=32-params.heightBias;
  const auto camera=buildCamera(params);
  Raster empty(640,480); empty.clear(19,23,31);
  auto fresh=empty; scene->drawExplosionDecals(fresh,camera);
  assert(fresh.framebuffer().rgb!=empty.framebuffer().rgb);
  assert(fresh.depthBuffer()==empty.depthBuffer());
  auto hidden=empty; std::fill(hidden.mutableDepthBuffer().begin(),hidden.mutableDepthBuffer().end(),0);
  scene->drawExplosionDecals(hidden,camera);
  assert(hidden.framebuffer().rgb==empty.framebuffer().rgb);
  scene->advanceDeathParticles(11000); // particles already expired; decal clock must still advance
  assert(scene->deathParticleCount()==0 && scene->explosionDecalCount()==1);
  auto faded=empty; scene->drawExplosionDecals(faded,camera);
  std::uint64_t before=0,after=0;
  for(std::size_t i=0;i<empty.framebuffer().rgb.size();++i) {
    const int a=fresh.framebuffer().rgb[i]-empty.framebuffer().rgb[i];
    const int b=faded.framebuffer().rgb[i]-empty.framebuffer().rgb[i];
    assert(a>=b && b>=0); before+=a; after+=b;
  }
  assert(after>0 && after<before);
  auto paused=empty; scene->advanceDeathParticles(11000); scene->drawExplosionDecals(paused,camera);
  assert(paused.framebuffer().rgb==faded.framebuffer().rgb);
  scene->advanceDeathParticles(20999); assert(scene->explosionDecalCount()==1);
  scene->advanceDeathParticles(21000); assert(scene->explosionDecalCount()==0);
  auto expired=empty; scene->drawExplosionDecals(expired,camera);
  assert(expired.framebuffer().rgb==empty.framebuffer().rgb);
  // Installed terrain supplies real depth: carve one synthetic crater, then
  // draw the decal at its inner surface. This is not a played Bomber route.
  auto setup=sim::loadRetailLevel(*lib,"MM_BOMB"); assert(setup);
  setup->terrain.clearExplosion(l.x,l.y);
  scene->syncTerrain(setup->terrain);
  scene->emitDeathParticles(l,true,{},22000,96,*lib);
  SceneOptions options; options.drawWall=false; options.drawCellRoute=true;
  auto crater=empty; scene->render(crater,camera,options);
  auto marked=crater; scene->drawExplosionDecals(marked,camera);
  assert(marked.framebuffer().rgb!=crater.framebuffer().rgb);
  assert(marked.depthBuffer()==crater.depthBuffer());
  if(argc>2) {
    for(const auto& [name,raster] : {std::pair{"fresh",&fresh},std::pair{"half-life",&faded},
                                   std::pair{"installed-crater",&marked}}) {
      Image image; image.width=640; image.height=480; image.rgba.assign(640*480*4,255);
      for(std::size_t p=0;p<image.pixelCount();++p)
        std::copy_n(raster->framebuffer().rgb.begin()+p*3,3,image.rgba.begin()+p*4);
      assert(writePng(std::filesystem::path(argv[2])/(std::string(name)+".png"),image));
    }
  }
  scene->advanceDeathParticles(42000);
  // An existing Bomber mark keeps its own lifetime when Nuke starts.
  scene->emitDeathParticles(l, true, {}, 43000, 96, *lib);
  const auto emitNukeMark = [&](std::uint64_t clockMs) {
    scene->emitDeathParticles(l, true, {}, clockMs, 96, *lib, /*nukeActive=*/true);
  };
  for (int i = 0; i < 100; ++i) {
    l.x = i * 2;
    emitNukeMark(43001);
  }
  assert(scene->explosionDecalCount() == 101);
  auto nuke = empty;
  const auto start=std::chrono::steady_clock::now();
  scene->drawExplosionDecals(nuke, camera);
  std::cout << "101-mark draw ms=" << std::chrono::duration<double,std::milli>(
      std::chrono::steady_clock::now()-start).count() << '\n';
  scene->advanceDeathParticles(46000);
  assert(scene->explosionDecalCount() == 101);
  scene->advanceDeathParticles(46001);
  assert(scene->explosionDecalCount() == 1);
  for (int i = 0; i < 260; ++i) emitNukeMark(46001);
  assert(scene->explosionDecalCount() == 256);
  scene->advanceDeathParticles(49001);
  assert(scene->explosionDecalCount() == 0);
  auto retry=TubeScene::build(*lib,*mesh,*bytes,index->grid0); assert(retry && retry->explosionDecalCount()==0);
  std::cout << "installed explosion texture, fading, occlusion, pause, expiry and bounded storage: passed\n";
}
