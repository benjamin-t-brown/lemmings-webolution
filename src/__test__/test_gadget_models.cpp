// Short legacy invariants: authored clips, presentation notifications and samples.
// No scripted level route, replay fixture or save store.
#include "app/BackdropScene.h"
#include "assets/Box.h"
#include "assets/LevelObjects.h"
#include "audio/CuePlayback.h"
#include "sim/RetailLevel.h"
#include "lemmings/compiled_level.hpp"
#include <filesystem>
#include <iostream>
#include <cmath>

using namespace webo;
namespace {
int failures=0;
void check(bool ok,const std::string& why){if(!ok){++failures;std::cerr<<"FAIL: "<<why<<'\n';}}
void contacts(){
 sim::LevelSetup setup;setup.terrain=sim::Terrain(64,32);
 for(int x=0;x<64;++x)setup.terrain.set(x,25,sim::Material::Earth);
 setup.totalLemmings=0;setup.teleports={{5,16,12,8,40,24},{34,16,12,8,11,24}};
 setup.speedPads={{47,18,2,6,0,true}};
 sim::World world(setup);std::vector<sim::World::GadgetUse> uses;
 world.setGadgetObserver([&](auto kind,std::size_t,const auto&){uses.push_back(kind);});
 sim::Lemming actor;actor.x=11;actor.y=24;actor.state=sim::State::Walker;world.place(actor);
 world.tick();check(uses.size()==1&&uses[0]==sim::World::GadgetUse::TeleportDeparture,"entry notifies once");
 for(int i=0;i<6&&uses.size()<2;++i)world.tick();
 check(uses.size()==2&&uses[1]==sim::World::GadgetUse::TeleportArrival,"relocation notifies destination once");
 check(world.lemmings()[0].x==40&&world.lemmings()[0].y==24,"existing teleport destination preserved");
 actor.x=48;world.place(actor);world.tick();
 check(uses.size()==3&&uses.back()==sim::World::GadgetUse::SpeedPad,"speed contact notifies");
 check(world.lemmings()[1].fasterMs>0,"speed contact retains existing effect");
}
void assets(const AssetLibrary& lib){
 audio::CuePlayback audio;std::vector<std::string> heard;
 audio.setObserver([&](std::string_view s){heard.emplace_back(s);});
 audio.start(lib,false);audio.playGadgetSample("");
 check(heard.empty(),"empty serialized sample stays silent");
 audio.playGadgetSample("teleport");audio.playGadgetSample("teleport2");
 check(heard==std::vector<std::string>{"teleport","teleport2"},"both serialized samples reach the cue adapter");
 audio.disable();audio.playGadgetSample("teleport");
 check(heard.size()==2,"no-audio suppresses gadget samples");
 int levels=0,teleLevels=0,speedLevels=0,clips=0,sampleLevels=0;
 for(const auto& name:lib.names())if(normaliseAssetName(name).ends_with(".LVL")){
   auto bytes=lib.read(name);if(!bytes){check(false,bytes.error());continue;}
   const auto level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes->data()),bytes->size()});
   int speed=0;for(const auto& e:level.gadgets.tagged_entities)if(e.type==4)++speed;
   if(level.gadgets.entities_4347ec.empty()&&!speed)continue;
   ++levels;teleLevels+=!level.gadgets.entities_4347ec.empty();speedLevels+=speed>0;
   auto scene=app::BackdropScene::build(lib,*bytes,name);if(!scene){check(false,scene.error());continue;}
   auto presentation=scene->gadgetPresentation(*bytes);if(!presentation){check(false,presentation.error());continue;}
   auto setup=sim::loadRetailLevel(lib,name);if(!setup){check(false,setup.error());continue;}
   check(presentation->bindings.size()==level.gadgets.entities_4347ec.size()*3+speed,name+" admits every model owner");
   bool samples=false;
   for(const auto& b:presentation->bindings){
     ++clips;check(b.simIndex<(b.teleport?setup->teleports.size():setup->speedPads.size()),name+" simulation binding index");
     if(b.teleport){
       check(b.clip.spins.size()==(b.fxSide<0?4u:8u),name+" authored rotation tracks");
       samples|=!b.samples[0].empty()||!b.samples[1].empty();
     }else check(b.clip.uv.size()==2&&!b.clip.uv[0].corners.empty(),name+" authored speed UV targets");
     if(b.fxSide<0){
       WorldPose pose;pose.position={b.original.matrix[9],b.original.matrix[10],b.original.matrix[11]};
       if(b.teleport){
         const auto box=teleportTriggerFromPose(pose,setup->terrain.width(),setup->terrain.height());
         const auto& sim=setup->teleports.at(b.simIndex);
         check(box.x==sim.x&&box.y==sim.y,name+" base model maps to its simulation pad");
       }else{
         const auto box=speedPadTriggerFromPose(pose,setup->terrain.width(),setup->terrain.height());
         const auto& sim=setup->speedPads.at(b.simIndex);
         check(box.x==sim.x&&box.y==sim.y,name+" speed model maps to its simulation pad");
       }
     }
   }
   sampleLevels+=samples;
   scene->animateGadgets(*presentation,0);
   // A render-state resample is absolute: no accumulating UV or rotation.
   scene->animateGadgets(*presentation,.1);
   for(const auto& b:presentation->bindings)if(b.teleport&&b.fxSide<0){
     scene->visitRoots([&](const auto&,const auto& root){
       if(root.sourceOffset!=b.original.sourceOffset)return;
       std::vector<const SceneNode*> nodes;
       const auto collect=[&](auto& self,const SceneNode& n)->void{nodes.push_back(&n);for(const auto& c:n.children)self(self,c);};collect(collect,root);
       for(const auto& spin:b.clip.spins){
         // Native selector 0 means Y, selector 1 means X (476EE0).
         const auto& m=nodes.at(spin.node)->matrix;
         check(std::abs(m[spin.axis==0?4:spin.axis==1?0:8]-1)<1e-12,name+" native rotation axis stays fixed");
       }
     });
   }
   std::vector<std::array<double,12>> matrices;
   scene->visitRoots([&](const auto&,const auto& n){for(const auto& c:n.children)matrices.push_back(c.matrix);});
   scene->animateGadgets(*presentation,.1);
   std::size_t index=0;
   scene->visitRoots([&](const auto&,const auto& n){for(const auto& c:n.children)check(c.matrix==matrices.at(index++),name+" repeated draw holds rotation");});
   for(const auto& b:presentation->bindings)if(b.fxSide<0){
     auto sample=scene->triggerGadget(*presentation,b.teleport,b.simIndex,false,1,1);
     check(sample==(b.teleport?b.samples[0]:std::string{}),name+" departure sample selection");
     if(b.teleport){
       sample=scene->triggerGadget(*presentation,true,b.simIndex,true,1,1.5);
       check(sample==b.samples[1],name+" arrival sample selection");
     }
   }
   scene->animateGadgets(*presentation,1.6);
   scene->animateGadgets(*presentation,20);
   std::cout<<name<<" clips="<<presentation->bindings.size()<<" samples="<<samples<<'\n';
 }
 check(levels==22&&teleLevels==18&&speedLevels==9,"installed level reach 22 / 18 / 9");
 check(clips==178&&sampleLevels==1,"178 authored clips; nonempty samples only on SHOW_KF3");
}
}
int main(int argc,char** argv){
 if(argc!=2)return 2;
 const std::filesystem::path dir=argv[1];
 auto lib=AssetLibrary::open(dir/"lemmings.box",dir/"patch.box");
 if(!lib){std::cerr<<lib.error()<<'\n';return 2;}
 contacts();assets(*lib);return failures?1:0;
}
