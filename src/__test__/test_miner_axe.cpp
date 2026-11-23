// Check Miner axe events, lifetime and depth with synthetic actors and installed art.
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>

using namespace webo;
using namespace webo::app;

static bool directionalRefusal() {
  constexpr int x = 49, y = 63;
  for (int blockedFacing : {-1, 1}) {
    CollisionGrid grid;
    grid.height = 96;
    grid.cells.resize(grid.width * grid.height);
    const int cutRow = grid.height - 1 - (y + 1);
    std::fill_n(grid.cells.begin(), (cutRow + 1) * grid.width, kCellSolid);
    const auto mask = blockedFacing < 0 ? kCellNoBashLeft : kCellNoBashRight;
    // A refusal in the second column must also preserve the first column.
    for (int dir : {-1, 1})
      grid.cells[cutRow * grid.width + x + 2 * dir] |= mask;

    for (int dir : {-1, 1}) {
      sim::LevelSetup setup;
      setup.terrain = sim::Terrain::fromCollisionGrid(grid);
      setup.totalLemmings = 0;
      sim::World world(setup);
      sim::Lemming walker;
      walker.x = x;
      walker.y = y;
      walker.state = sim::State::Walker;
      walker.facing = dir < 0 ? sim::Facing::Left : sim::Facing::Right;
      world.place(walker);
      if (!world.assign(0, sim::Skill::Miner))
        return false;
      (void)world.takeSoundEvents();
      const auto version = world.terrain().version();
      for (int i = 0; i < 8 && world.lemmings()[0].state == sim::State::Miner &&
           world.terrain().version() == version; ++i)
        world.tick();

      const bool refused = dir == blockedFacing;
      const auto& actor = world.lemmings()[0];
      const auto cues = world.takeSoundEvents();
      const auto expected = refused ? sim::Material::Earth : sim::Material::Empty;
      if (actor.state != (refused ? sim::State::Walker : sim::State::Miner) ||
          int(actor.facing) != (refused ? -dir : dir) ||
          actor.x != (refused ? x : x + dir) || actor.y != (refused ? y : y + 1) ||
          world.terrain().at(x + dir, y + 1) != expected ||
          world.terrain().at(x + 2 * dir, y + 1) != expected ||
          cues.size() != 1 ||
          (refused ? cues[0].sample != "MineMetal" : cues[0].cue != sim::SoundCue::Work)) {
        std::cerr << "Unexpected Miner stroke facing " << dir << " against mask " << mask << '\n';
        return false;
      }
    }
  }
  return true;
}

int main(int argc, char** argv) {
  assert(argc > 1);
  if (!directionalRefusal()) return 1;
  for (int facing : {-1,1}) {
    sim::LevelSetup setup; setup.terrain = sim::Terrain(256,96); setup.totalLemmings = 0;
    setup.terrain.fillBox(0,64,256,32,sim::Material::Earth);
    setup.terrain.set(49+facing,63,sim::Material::Steel);
    sim::World world(setup), plain(setup);
    sim::Lemming l; l.x=49; l.y=63; l.state=sim::State::Miner;
    l.facing = facing < 0 ? sim::Facing::Left : sim::Facing::Right;
    world.place(l); plain.place(l);
    int callbacks=0;
    world.setMinerRefusalObserver([&](const sim::Lemming& refused) {
      ++callbacks;
      assert(refused.state == sim::State::Miner && int(refused.facing) == facing);
    });
    for(int i=0;i<12;++i) { world.tick(); plain.tick(); }
    assert(callbacks == 1);
    const auto& a=world.lemmings()[0]; const auto& b=plain.lemmings()[0];
    assert(a.x==b.x && a.y==b.y && a.state==b.state && a.facing==b.facing && a.frame==b.frame);
    assert(world.terrain().version()==plain.terrain().version());
    const auto cues=world.takeSoundEvents(), plainCues=plain.takeSoundEvents();
    assert(cues.size()==plainCues.size());
    // Successful mining has no thrown tool, even though its SPR has a pickaxe.
    setup.terrain.set(49+facing,63,sim::Material::Earth);
    sim::World success(setup); success.place(l);
    success.setMinerRefusalObserver([&](const sim::Lemming&) { assert(false); });
    for(int i=0;i<12;++i) success.tick();
  }
  const std::filesystem::path game(argv[1]);
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box"); assert(lib);
  auto bytes=lib->read("COMPILED\\MM_MINE.LVL"); assert(bytes);
  auto mesh=findCylinderMesh(*bytes); auto index=indexLevel(*bytes); assert(mesh && index);
  auto axes=loadSpriteSheet(*lib,"SPRITES\\AXE.SPR"); assert(axes);
  const auto* axe=axes->find("axe"); assert(axe && axe->animations.size()==8 && axe->rate==1);
  int pixelUvs=0, normalizedUvs=0;
  for(const auto& entry:axe->animations) {
    assert(entry.frames.size()==5);
    for(const auto& frame:entry.frames) {
      pixelUvs += frame.uLeft==frame.left && frame.uRight==frame.right &&
                  frame.uTop==frame.top && frame.uBottom==frame.bottom;
      normalizedUvs += frame.uRight<=1 && frame.uBottom<=1;
    }
  }
  std::cout << "Axe UV records: " << pixelUvs << " pixel-valued, " << normalizedUvs << " normalized\n";
  int cases=0;
  for(int gravity:{-1,1}) for(int facing:{-1,1}) for(int zoom:{0,1}) {
    auto scene=TubeScene::build(*lib,*mesh,*bytes,index->grid0); assert(scene);
    // Empty collision is a synthetic render fixture, not a cleared gameplay route.
    scene->syncTerrain(sim::Terrain(256,96));
    sim::Lemming l; l.x=49; l.y=63; l.state=sim::State::Miner; l.verticalDir=gravity;
    l.facing=facing<0?sim::Facing::Left:sim::Facing::Right;
    scene->emitMinerAxe(l,0,96); assert(scene->deathParticleCount()==1);
    std::array<int,5> visiblePhases{};
    for(int age=0; age<480; age+=60) {
      scene->advanceDeathParticles(age);
      int view=0;
      for(int yaw:{-64,-32,0,32,64}) {
        CameraParams params; params.yawUnits=49+yaw; params.zoomShift=zoom;
        params.eyeDistance=800; // keep the rotated synthetic tool inside both zoom viewports
        params.fixedPitchTurns=0;
        params.levelEyeHeight=32-params.heightBias;
        const auto camera=buildCamera(params);
        Raster empty(640,480); empty.clear(19,23,31);
        auto raw=empty; scene->drawDeathParticles(raw,camera,*lib);
        // Require an exposed tool during the animation at every tested angle.
        visiblePhases[view++] += raw.framebuffer().rgb!=empty.framebuffer().rgb;
        assert(raw.depthBuffer()==empty.depthBuffer());
        auto covered=empty;
        std::fill(covered.mutableDepthBuffer().begin(),covered.mutableDepthBuffer().end(),0.0);
        const auto depth=covered.depthBuffer();
        scene->drawDeathParticles(covered,camera,*lib);
        assert(covered.framebuffer().rgb==empty.framebuffer().rgb && covered.depthBuffer()==depth);
        // Actor coverage also hides the effect; it does not draw on top of figures.
        auto actors=empty; actors.mutableActorDepth().reset(640*480);
        for(std::size_t p=0;p<640*480;++p) assert(actors.mutableActorDepth().submit(p,0,0));
        scene->drawDeathParticles(actors,camera,*lib);
        assert(actors.framebuffer().rgb==empty.framebuffer().rgb);
        if(argc>2 && yaw==0 && age==0) {
          Image image; image.width=640; image.height=480; image.rgba.assign(640*480*4,255);
          for(std::size_t p=0;p<image.pixelCount();++p)
            std::copy_n(raw.framebuffer().rgb.begin()+p*3,3,image.rgba.begin()+p*4);
          assert(writePng(std::filesystem::path(argv[2])/("refusal-"+std::to_string(gravity)+"-"+
              std::to_string(facing)+"-"+std::to_string(zoom)+".png"),image));
        }
        ++cases;
      }
    }
    for(int visible:visiblePhases) assert(visible>0);
    scene->advanceDeathParticles(4499); assert(scene->deathParticleCount()==1);
    scene->advanceDeathParticles(4500); assert(scene->deathParticleCount()==0);
  }
  std::cout<<"one pre-turn refusal event, unchanged gameplay, success silent, installed Axe, "
           <<cases<<" yaw/facing/gravity/zoom/depth cases, expiry: PASS\n";
}
