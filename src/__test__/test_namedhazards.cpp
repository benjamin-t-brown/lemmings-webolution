#include "sim/RetailLevel.h"
#include "app/BackdropScene.h"
#include "app/MapScene.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>

using namespace webo;
using namespace webo::sim;
namespace {
int failures = 0;
void check(bool ok, const std::string& why) {
  if (!ok) { ++failures; std::cerr << "FAIL: " << why << '\n'; }
}
bool near(double a, double b) { return std::abs(a-b)<1e-8; }
NamedHazard patrol(bool loop = false) {
  NamedHazard h;
  h.moving = true; h.position = h.initialPosition = {20,20,0.5};
  h.boxOrigin = {19,18,0}; h.cutOffsets = {-1,-2,3,3};
  h.paths[0].loop = loop;
  h.paths[0].segments = {{2,1,{20,20},{5,0}}, {0,0,{30,20},{0,0}}};
  if (loop) h.paths[0].segments = {{2,1,{20,20},{5,0}}, {2,0,{30,20},{-5,0}}};
  return h;
}
Lemming actor(int x, int originalY) {
  Lemming l; l.x=x; l.y=63-originalY; l.state=State::Blocker; return l;
}
void units() {
  Terrain t(256,64);
  auto h=patrol(); h.advancePatrol(1,t);
  check(near(h.position[0],25)&&!h.reverse,"path uses stored velocity and time");
  h.advancePatrol(2,t);
  check(near(h.position[0],25)&&h.reverse,"open path reflects past final endpoint");
  h.advancePatrol(2,t);
  check(near(h.position[0],25)&&!h.reverse,"open path reflects past first endpoint");
  h=patrol(true); h.advancePatrol(4.5,t);
  check(near(h.position[0],22.5)&&!h.reverse,"loop wraps without reflecting");
  h=patrol(); h.reverse=true; h.travel=0.5; h.advancePatrol(1,t);
  check(near(h.position[0],22.5)&&!h.reverse,"negative starting travel normalizes");
  h=patrol(); h.speed=40; h.advancePatrol(0.2,t);
  check(near(h.position[0],20)&&h.reverse,"large speed crosses both boundaries safely");
  h=patrol(); t.set(23,63-18,Material::Earth); t.set(23,63-19,Material::Steel);
  t.set(23,63-20,Material::Earth); h.travel=1; h.advancePatrol(0.1,t);
  check(t.at(23,63-18)==Material::Empty&&t.at(23,63-20)==Material::Earth&&h.reverse,
        "leading edge cuts in order and reverses at steel without cutting beyond it");
  h=patrol(); h.enabled=false; const auto before=h.position; h.advancePatrol(1,t);
  check(h.position==before,"disabled patrol does not advance");
  t=Terrain(256,64);
  h=patrol(true); h.paths[0].junctions={1}; h.paths[1].loop=false;
  h.paths[1].segments={{2,3,{30,20},{0,5}},{0,0,{30,30},{0,0}}};
  h.paths[1].junctions={0}; h.requestedPath=1; h.travel=1.9; h.advancePatrol(0.2,t);
  check(h.path==1&&near(h.position[0],30)&&near(h.position[1],20.5),
        "requested path switches at paired junction preserving leftover travel");
  h.paths[1].segments[0].duration=std::numeric_limits<double>::infinity();
  h.paths[1].segments[0].velocity={0,0}; h.travel=1;
  h.advancePatrol(100,t);
  check(near(h.position[0],30)&&near(h.position[1],20),"authored infinite endpoint remains stationary");
  h.paths[1].loop=true;h.paths[1].segments.back().duration=std::numeric_limits<double>::infinity();
  h.reverse=true;h.travel=0;h.segment=0;h.advancePatrol(0.1,t);
  check(std::isfinite(h.position[0])&&std::isfinite(h.position[1]),"reverse wrap into infinite wait cannot produce NaN coordinates");

  LevelSetup s; s.terrain=Terrain(256,64); s.totalLemmings=0;
  NamedHazard saw; saw.kind=NamedHazard::Kind::Buzzsaw; saw.boxOrigin={40,10,0};
  saw.volumes.push_back({{0,0,0},{4,6,1}}); saw.restDuration=1.2; saw.particles=false;
  s.namedHazards.push_back(saw);
  World w(s); int particles=0;
  w.setDeathObserver([&](const auto&,bool,auto){++particles;});
  w.place(actor(40,10)); w.place(actor(44,16)); w.place(actor(45,10));
  w.advanceElapsedMs(0);
  check(w.dead()==2&&particles==0&&w.lemmings()[2].active(),
        "buzzsaw removes all closed-boundary contacts without a particle state");
  w.advanceElapsedMs(0); check(w.dead()==2,"removed victims are not counted twice");
  s.namedHazards[0].particles=true;
  World roller(s); roller.setDeathObserver([&](const auto&,bool,auto){++particles;});
  roller.place(actor(40,10));roller.place(actor(44,16));roller.advanceElapsedMs(0);
  check(roller.dead()==2&&particles==2,"roller death selectors emit one particle notification per victim");
  s.namedHazards[0].allContacts=false; s.namedHazards[0].particles=true;
  s.namedHazards[0].restDuration=0.6; s.namedHazards[0].attackDuration=1;
  s.terrain.fillBox(0,52,256,1,Material::Earth);
  World bird(s); bird.place(actor(42,12)); bird.place(actor(42,12));
  bird.advanceElapsedMs(0); check(bird.dead()==0,"single-victim contact waits for clip completion");
  bird.advanceElapsedMs(600);
  check(bird.dead()==1&&bird.setup().namedHazards[0].attacking,"first pending contact dies at rest completion");
  bird.advanceElapsedMs(500); check(bird.dead()==1,"attack latch prevents another death mid-clip");
  bird.advanceElapsedMs(501); check(bird.dead()==2,"attack completion rearms the next single contact");
  World pending(s);auto walker=actor(44,12);walker.state=State::Walker;
  pending.place(walker);pending.advanceElapsedMs(0);pending.advanceElapsedMs(601);
  check(pending.dead()==1&&pending.lemmings()[0].x>44,"pending victim is retained after leaving the contact box");
  s.namedHazards[0]=patrol(true);s.namedHazards[0].paths[1]=s.namedHazards[0].paths[0];
  s.objectIds[7]={ObjectClass::NamedHazard,0,"named_entities"};
  Lever lever;lever.linked=true;lever.facePlus={{7,0,1,0}};
  activateLeverFace(s,lever,1);
  check(s.namedHazards[0].requestedPath==1&&s.namedHazards[0].path==0,
        "lever path request preserves current path until a junction");
  lever.facePlus={{7,5,0,0}};activateLeverFace(s,lever,1);
  check(!s.namedHazards[0].enabled,"disable message reaches named hazard");
  lever.facePlus={{7,4,0,0}};activateLeverFace(s,lever,1);
  check(s.namedHazards[0].enabled,"enable message reaches named hazard");
}
void installed(const AssetLibrary& lib) {
  const auto table=loadLevelTable(lib); check(bool(table),"installed map loads"); if(!table)return;
  const auto& mapping=app::MapScene::nodeMapping();
  int count=0, moving=0;
  for(int node:{20,24,30,34,39,46,49,60,68,72,83,85,93,101,37,58,62,78,63,81}) {
    const auto& id=table->groups.at(mapping[node][0]).at(mapping[node][1]).id;
    auto report=loadRetailLevelReport(lib,id);
    check(bool(report),id+" production load" );
    if(!report){std::cerr<<report.error()<<'\n';continue;}
    auto& hazards=report->setup.namedHazards;
    check(!hazards.empty(),id+" named hazard bound");
    count+=int(hazards.size()); for(const auto& h:hazards)moving+=h.moving;
    auto resolved=resolveRetailLevel(lib,id); auto bytes=lib.read(resolved->file);
    auto scene=app::BackdropScene::build(lib,*bytes,id);
    check(bool(scene),id+" scene builds"); if(!scene){std::cerr<<scene.error()<<'\n';continue;}
    auto presentation=scene->namedHazardPresentation(*bytes);
    check(bool(presentation),id+" animation bindings"); if(!presentation)continue;
    check(presentation->bindings.size()==hazards.size(),id+" all models rendered");
    // Data/logic check only: sample every installed route, including secondary
    // paths and zero-duration terminal keys, without commanding any lemmings.
    for(auto h:hazards) {
      if(h.moving) {
        h.advancePatrol(0,report->setup.terrain);
        check(std::isfinite(h.position[0])&&std::isfinite(h.position[1]),id+" initial path sample");
      }
      h.animationSeconds=0.2;
      scene->setNamedHazardStates(*presentation,std::span(&h,1));
      std::cout<<"node="<<node<<" level="<<id<<" entity="<<h.entityIndex
          <<" moving="<<h.moving<<" position="<<h.position[0]<<','<<h.position[1]
          <<" rest="<<h.restDuration<<" attack="<<h.attackDuration<<'\n';
    }
    if(node==20) {
      auto h=hazards[0]; h.advancePatrol(0,report->setup.terrain);
      check(near(h.position[0],157)&&near(h.position[1],49),"node 20 stored path phase samples (157,49)");
      std::vector<std::array<double,12>> before,after;
      const auto matrices=[&](auto& out){scene->visitRoots([&](const auto& source,const SceneNode& root){
        if(source.owner!="named-hazard")return;
        out.push_back(root.matrix);for(const auto& child:root.children)out.push_back(child.matrix);
      });};
      scene->setNamedHazardStates(*presentation,hazards);matrices(before);
      h.advancePatrol(0.1,report->setup.terrain);h.animationSeconds=0.1;
      scene->setNamedHazardStates(*presentation,std::span(&h,1));matrices(after);
      check(before!=after,"node 20 production matrices move and spin");
    }
  }
  check(count==29&&moving==18,"22 rollers, 5 robins, one scorpion, one saw; 18 moving rollers");
}
}
int main(){
  units();
  if(const char* dir=std::getenv("WEBOLUTION_GAME_DIR")) {
    const std::filesystem::path root(dir);
    auto lib=AssetLibrary::open(root/"lemmings.box",root/"patch.box");
    check(bool(lib),"open installed assets");if(lib)installed(*lib);
  }
  std::cout<<"namedhazards failures="<<failures<<'\n';return failures?1:0;
}
