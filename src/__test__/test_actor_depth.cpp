// Check actor coverage and depth with synthetic poses.
#include "app/ActorDepth.h"
#include "app/ActorOcclusion.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "sim/World.h"

#include <cassert>
#include <filesystem>
#include <iostream>

using namespace webo;
using namespace webo::app;

int main(int argc, char** argv) {
  ActorDepth layer;
  layer.reset(3);
  assert(layer.submit(0, 20, 2));
  assert(!layer.submit(0, 21, 0));
  assert(layer.submit(0, 19, 4));
  assert(!layer.submit(0, 19, 5));
  assert(layer.submit(0, 19, 1) && layer.owns(0,1) && !layer.owns(0,4));
  assert(!layer.submit(1, std::numeric_limits<double>::infinity(), 0));
  assert(!layer.submit(1, std::numeric_limits<double>::quiet_NaN(), 0));
  layer.reset(3);
  assert(layer.submit(0, 30, 99));

  // Check the inverse ray against independent f32 points on the upright actor plane
  // under yaw and pitch.
  for (int turn=0; turn<32; ++turn) for (double pitch : {-.03,.04}) {
    CameraParams cp; cp.yawUnits=turn*8; cp.fixedPitchTurns=pitch;
    const auto camera=buildCamera(cp);
    const auto& m=camera.worldToView;
    const std::array<double,3> origin{40,35,20};
    const ActorOcclusionPlane plane(camera,origin);
    const std::array<double,3> point{origin[0]-m[8]*2,origin[1]+5,origin[2]+m[2]*2};
    std::array<double,3> view{};
    for(int k=0;k<3;++k) view[k]=m[k]*point[0]+m[k+3]*point[1]+m[k+6]*point[2]+m[k+9];
    const auto recovered=plane.worldAt(camera,camera.cx+camera.focalX*view[0]/view[2],
        camera.cy-camera.focalY*view[1]/view[2]);
    for(int k=0;k<3;++k) assert(std::abs(recovered[k]-point[k])<1e-4);
  }

  CylinderMesh cylinder;
  cylinder.angularSteps = 32; cylinder.ringCount = 7;
  cylinder.faceCount = 112; cylinder.thickness = 10;
  for (int ring=0; ring<7; ++ring) for (int step=0; step<32; ++step)
    for (double radius : {50.,60.}) {
      const double angle = step*2*std::numbers::pi/32;
      cylinder.vertices.push_back({-radius*std::sin(angle),ring*16.,radius*std::cos(angle)});
    }
  CollisionGrid grid;
  grid.width=256; grid.height=96; grid.cells.resize(256*96);
  const ActorPoseLevel pose{112,{}, {}, &cylinder};
  int inversions=0;
  // Independent cell membership: one solid column, adjacent cells empty.
  // Vary every column, shell layer and ring; includes the 255/0 seam.
  for (int x=0; x<256; ++x) for (int y : {1,17,35,65,94}) {
    grid.cells[y*256+x] = kCellSolid;
    for (double z : {.25,.75}) for (double offset : {-.5,.5,1.5}) {
      const auto p=actorPoseFromCylinderSample(pose,{x+offset,y+.5,z},1).modelPosition;
      assert(actorTexelInsideTerrain(cylinder,grid,p) == (offset==.5));
      ++inversions;
    }
    // Flags without geometry must never hide the figure.
    grid.cells[y*256+x] = kCellZoneType0 | kCellBlocker;
    assert(!actorTexelInsideTerrain(cylinder,grid,
        actorPoseFromCylinderSample(pose,{x+.5,y+.5,.75},1).modelPosition));
    grid.cells[y*256+x] = 0;
  }
  std::cout << inversions << " polygonal footprint inversions, flags and depth ties: PASS\n";
  if (argc<2 || std::string_view(argv[1]).empty()) return 0;
  const std::filesystem::path game(argv[1]);
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box"); assert(lib);
  std::string name;
  for (const auto& n:levelFileNames(*lib)) if(n.find("MM_E1_3_SKILLS.")!=std::string::npos) name=n;
  auto bytes=lib->read(name); assert(bytes);
  auto mesh=findCylinderMesh(*bytes); auto index=indexLevel(*bytes); assert(mesh&&index);
  auto scene=TubeScene::build(*lib,*mesh,*bytes,index->grid0); assert(scene);
  const ActorPoseLevel installedPose{int(mesh->faceCount),index->grid0,mesh->surface,&*mesh};
  int overlaps=0, views=0;
  // Every cylinder sector, front and oblique cameras. Two lanes at the same
  // column plus a third overlapping actor with distinct installed artwork.
  for (int sector=0; sector<32; ++sector) for (double offset : {-56.,0.,56.}) {
    CameraParams cp; cp.yawUnits=sector*8+offset;
    const auto camera=buildCamera(cp);
    SceneOptions options;
    options.injectedWalkers={{sector*8.+.2,70,.25,1},
                            {sector*8.+.2,70,.75,1},
                            {sector*8.+.6,70.5,.5,1}};
    options.injectedWalkers[1].sheetType=1;
    options.injectedWalkers[2].sheetType=2;
    Raster empty(640,480); empty.clear(19,23,31);
    auto combined=empty;
    scene->drawInjectedWalkers(combined,camera,options,*lib);
    std::array<Raster,3> single{empty,empty,empty};
    std::array<double,3> distance;
    for (int i=0;i<3;++i) {
      auto one=options; one.injectedWalkers={options.injectedWalkers[i]};
      scene->drawInjectedWalkers(single[i],camera,one,*lib);
      const auto& w=options.injectedWalkers[i];
      const auto p=actorPoseFromCylinderSample(installedPose,{w.plus8X,w.plus8Y,w.plus8Z},1).modelPosition;
      distance[i]=camera.worldToView[2]*p[0]+camera.worldToView[8]*p[2];
    }
    for (std::size_t p=0;p<640*480;++p) {
      int winner=-1, count=0;
      for (int i=0;i<3;++i) if(!std::equal(single[i].framebuffer().rgb.begin()+p*3,
          single[i].framebuffer().rgb.begin()+p*3+3,empty.framebuffer().rgb.begin()+p*3)) {
        ++count;
        if(winner<0 || distance[i]<distance[winner]) winner=i;
      }
      if(count>1) ++overlaps;
      const auto& expected=winner<0 ? empty.framebuffer().rgb : single[winner].framebuffer().rgb;
      assert(std::equal(expected.begin()+p*3,expected.begin()+p*3+3,combined.framebuffer().rgb.begin()+p*3));
    }
    auto reversed=empty;
    std::reverse(options.injectedWalkers.begin(),options.injectedWalkers.end());
    scene->drawInjectedWalkers(reversed,camera,options,*lib);
    assert(reversed.framebuffer().rgb==combined.framebuffer().rgb);
    assert(combined.depthBuffer()==empty.depthBuffer());
    ++views;
  }
  assert(overlaps>1000);
  std::cout << views << " views / " << overlaps << " overlapping pixels: nearest body wins, list reversal invariant, scene Z preserved: PASS\n";

  for (double offset : {-56.,0.,56.}) for (double phase : {.2,.7}) {
    CameraParams cp; cp.yawUnits=90+offset;
    const auto camera=buildCamera(cp);
    SceneOptions options;
    options.injectedWalkers={{90,70,.25,1},{90,73,.75,1}};
    auto& builder=options.injectedWalkers[0];
    builder.clip="build"; builder.builderX=90; builder.builderY=70;
    builder.builderPhase=phase; builder.walkTemporalIndex=int(phase*16);
    options.injectedWalkers[1].sheetType=1;
    Raster a(640,480); a.clear(19,23,31); auto b=a;
    scene->drawInjectedWalkers(a,camera,options,*lib);
    std::reverse(options.injectedWalkers.begin(),options.injectedWalkers.end());
    scene->drawInjectedWalkers(b,camera,options,*lib);
    assert(a.framebuffer().rgb==b.framebuffer().rgb);
  }
  // Check BITS and colored-chip rejection against an occupied actor layer.
  sim::Lemming lemming;
  lemming.x=90; lemming.y=25;
  const std::array<std::array<int,2>,1> removed{{{90,25}}};
  scene->emitDeathParticles(lemming,true,removed,1000,96,*lib);
  scene->emitTerrainDebris(lemming,removed,1000,96);
  scene->advanceDeathParticles(1100);
  CameraParams cp; cp.yawUnits=90;
  const auto camera=buildCamera(cp);
  Raster empty(640,480); empty.clear(19,23,31); auto visible=empty, hidden=empty;
  scene->drawDeathParticles(visible,camera,*lib);
  assert(visible.framebuffer().rgb!=empty.framebuffer().rgb);
  hidden.mutableActorDepth().reset(640*480);
  for (std::size_t p=0;p<640*480;++p) assert(hidden.mutableActorDepth().submit(p,1,0));
  scene->drawDeathParticles(hidden,camera,*lib);
  assert(hidden.framebuffer().rgb==empty.framebuffer().rgb);
  assert(hidden.depthBuffer()==empty.depthBuffer());
  hidden.clear(19,23,31);
  scene->drawDeathParticles(hidden,camera,*lib);
  assert(hidden.framebuffer().rgb==visible.framebuffer().rgb);
  std::cout << "carried mesh list reversal; body pieces, death chips and terrain debris share actor coverage; frame clear resets it: PASS\n";
}
