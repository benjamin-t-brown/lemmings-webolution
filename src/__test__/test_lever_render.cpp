#include "app/BackdropScene.h"
#include "app/AppEntry.h"
#include "app/MapScene.h"
#include <algorithm>
#include <sstream>
#include <cmath>
#include <map>
#include <iomanip>
#include "app/TubeScene.h"
#include "app/Raster.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#include "save/SaveStore.h"
#include "lemmings/compiled_level.hpp"
#include <bit>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
using namespace webo;
namespace fs=std::filesystem;
int measure(int argc,char**argv){
  if(argc<2||!std::getenv("WEBOLUTION_GAME_DIR"))return 2;
  fs::path out=argv[1];fs::create_directories(out);
  auto lib=AssetLibrary::open(fs::path(std::getenv("WEBOLUTION_GAME_DIR"))/"lemmings.box",fs::path(std::getenv("WEBOLUTION_GAME_DIR"))/"patch.box");if(!lib)return 2;
  std::ofstream log(out/"measure.txt");
  for(auto id:{"MM_W_012","E16_KF_1","MM_ALL_8_001","MM_CLIMB_002"}){
    auto r=sim::resolveRetailLevel(*lib,id);if(!r)return 2;auto b=lib->read(r->file);if(!b)return 2;
    auto l=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(b->data()),b->size()});
    for(const auto&t:l.gadgets.tagged_entities){
      if(t.type!=1)continue;
      log<<id<<" lever "<<t.id<<" face="<<int(t.values[0])<<" a0="<<t.flag_a0<<" b0="<<t.flag_b0<<" model="<<(t.model?t.model->name:"none")<<'\n';
      for(std::size_t face=0;face<2;++face){
        log<<"  face="<<(face?1:-1)<<" messages=";
        for(const auto& message:t.record_arrays[face]){
          log<<'[';for(auto value:message)log<<value<<',';log<<']';
        }
        log<<'\n';
      }
      if(!t.model||!t.model->object)continue;
      auto n=readSceneNode(*b,t.model->object->source_extent.typed_root_begin);if(!n)return 2;
      const auto dump=[&](auto&self,const SceneNode&n,int depth)->void{log<<std::string(depth,' ')<<n.name<<" type="<<n.type<<" offset="<<n.sourceOffset<<" matrix=";for(auto x:n.matrix)log<<x<<',';log<<'\n';for(auto&c:n.children)self(self,c,depth+1);};dump(dump,n->root,0);
      for(const auto&a:t.model->object->auxiliary_records){log<<"aux "<<a.name<<" bytes="<<a.serialized_bytes.size()<<" counts="<<a.first_count<<','<<a.second_count<<','<<a.third_count<<'\n';auto anim=app::readHatchAnimation(a.serialized_bytes);if(anim){log<<"duration="<<anim->duration<<'\n';for(auto&tr:anim->tracks){log<<"track "<<tr.node<<'\n';for(auto&k:tr.keys){log<<"end="<<k.end<<" p=";for(auto x:k.position)log<<x<<',';log<<" q=";for(auto x:k.quaternion)log<<x<<',';log<<'\n';}}}else log<<anim.error()<<'\n';}
    }
    for(std::size_t i=0;i<l.gadgets.entities_42acac.size();++i){auto&g=l.gadgets.entities_42acac[i];
      std::vector<std::byte> pattern;auto append=[&](std::uint64_t x,int n){for(int j=0;j<n;++j)pattern.push_back(std::byte((x>>(8*j))&255));};
      for(auto x:g.values_18c_128_168)append(x,4);
      for(auto x:g.triple_170)append(x,8);
      append(g.value_16c,4);
      for(auto x:g.doubles_98_1a8_a0)append(x,8);
      std::size_t start=0;for(auto model:{g.model?&*g.model:nullptr,&g.model_15c,g.model_160?&*g.model_160:nullptr,g.model_164?&*g.model_164:nullptr})if(model&&model->object)start=std::max(start,model->object->source_extent.holder_end);
      auto match=std::search(b->begin()+start,b->end(),pattern.begin(),pattern.end());if(match==b->end())return 2;auto offset=std::size_t(match-b->begin());
      log<<"raw gate "<<i<<" req@0x"<<std::hex<<offset<<" kind@0x"<<offset+4<<" high@0x"<<offset+40<<" storedH@0x"<<offset+48<<" low@0x"<<offset+56<<std::dec<<'\n';
log<<"gate "<<i<<" id="<<g.id<<" req/kind/168=";for(auto x:g.values_18c_128_168)log<<x<<',';log<<" hi/stored/lo=";for(auto x:g.doubles_98_1a8_a0)log<<std::bit_cast<double>(x)<<',';log<<" bounds=";for(auto x:g.double_bits_48_60)log<<std::bit_cast<double>(x)<<',';log<<'\n';}
  }
  return 0;
}

namespace {
int failures=0;
void check(bool ok,const std::string& message){if(!ok){++failures;std::cerr<<"FAIL: "<<message<<'\n';}}
std::string file(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
struct Saved {fs::path p;bool exists;std::string bytes;explicit Saved(fs::path path):p(path),exists(fs::exists(p)),bytes(file(p)){}bool unchanged()const{return fs::exists(p)==exists&&bytes==file(p);}};
void png(const fs::path& p,const Image& image){check(writePng(p,image).has_value(),"write "+p.string());}
[[maybe_unused]] std::size_t different(const Image&a,const Image&b){check(a.rgba.size()==b.rgba.size(),"matched image sizes");std::size_t n=0;for(std::size_t i=0;i<a.rgba.size();i+=4)if(!std::equal(a.rgba.begin()+i,a.rgba.begin()+i+3,b.rgba.begin()+i))++n;return n;}
struct DepthCounts {std::size_t visible=0,occluded=0;};
Image shoot(app::BackdropScene& back,const app::TubeScene& tube,const AssetLibrary& lib,const sim::World& world,int yaw,std::ostream* depthLog=nullptr,DepthCounts* counts=nullptr){
  std::vector<app::BackdropScene::FluidGateState> fluid;for(auto&g:world.setup().gates)fluid.push_back({g.lane,g.height,g.wavePhase,g.animated,(world.levelClockMs()%4000)/1000.0});back.setFluidState(std::move(fluid));
  app::CameraParams p;p.yawUnits=yaw;app::Raster raster(640,480);raster.clear(0,0,0);auto cam=app::buildCamera(p);app::BackdropStats stats;
  using S=app::BackdropScene::Stage;back.renderStage(raster,cam,lib,stats,S::Primary);
  std::vector<lemmings::ObjectSourceExtent> extents;back.visitRoots([&](const auto&src,const auto&){if(src.owner=="tagged-lever")extents.push_back(src.extent);});
  std::vector<std::uint8_t> mask(640*480,1);app::SceneRasterObserver observer;observer.mask=&mask;
  std::map<std::size_t,double> leverDepth;
  observer.accepted=[&](const app::SceneRasterWriteEvent&e){for(auto&extent:extents)if(e.source>=extent.typed_root_begin&&e.source<extent.tree_end){check(e.depthAssigned&&e.depthAfter<e.depthBefore,"lever uses scene Less/write-on depth");leverDepth[e.pixel]=e.depthAfter;}};
  back.renderStage(raster,cam,lib,stats,S::LegacyUnattributed,&observer);
  app::SceneOptions options;options.drawCellRoute=true;options.drawWall=false;options.drawOuter=false;options.drawInner=false;tube.render(raster,cam,options,&lib);
  back.renderStage(raster,cam,lib,stats,S::FluidGate);back.renderStage(raster,cam,lib,stats,S::Secondary);
  DepthCounts depthCounts;
  for(auto [pixel,z]:leverDepth){depthCounts.visible+=raster.depthBuffer()[pixel]==z;depthCounts.occluded+=raster.depthBuffer()[pixel]<z;}
  if(counts)*counts=depthCounts;
  if(depthLog)*depthLog<<"depth yaw="<<yaw<<" lever_stores="<<leverDepth.size()<<" survive="<<depthCounts.visible<<" occluded="<<depthCounts.occluded<<'\n';
  check(stats.rejected==stats.projectionRejected&&stats.rasterRejected==0&&stats.missingTextures==0,"scene has no texture/raster failures (rear projections may be culled)");
  Image image;image.width=640;image.height=480;auto&rgb=raster.framebuffer().rgb;for(std::size_t i=0;i<rgb.size();i+=3)image.rgba.insert(image.rgba.end(),{rgb[i],rgb[i+1],rgb[i+2],255});return image;
}
void census(const AssetLibrary&lib,const fs::path&out){
  std::ofstream log(out/"admission.txt");std::size_t levels=0,levers=0;
  for(auto&name:levelFileNames(lib)){auto bytes=lib.read(name);if(!bytes){check(false,"census read");continue;}auto level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes->data()),bytes->size()});std::size_t count=0;
    for(auto&t:level.gadgets.tagged_entities)if(t.type==1){++count;check(t.model&&t.model->object,"installed lever has embedded object");if(!t.model||!t.model->object)continue;bool found=false;for(auto&a:t.model->object->auxiliary_records)if(a.name=="lever"){found=true;auto animation=app::readHatchAnimation(a.serialized_bytes);check(animation.has_value(),"installed lever transform track supported");if(animation)for(auto&tr:animation->tracks)check(tr.node!=0,"lever animation preserves placed root");}check(found,"installed lever clip present");}
#ifdef LEVER_RENDER_after
    if(count){auto back=app::BackdropScene::build(lib,*bytes,name);check(back.has_value(),"all installed lever scenes build");if(back){auto bindings=back->leverPresentation(*bytes);check(bindings&&bindings->bindings.size()==count,"all installed lever bindings admitted");}}
#endif
    log<<name<<" levers="<<count<<'\n';++levels;levers+=count;
  }log<<"levels="<<levels<<" levers="<<levers<<'\n';check(levels==102&&levers==130,"retained installed census 102 levels / 130 levers");
}
void fixtures(const AssetLibrary&lib,const fs::path&out){
  std::ofstream trace(out/"fixtures.txt");
  for(auto id:{"MM_W_012","E16_KF_1","MM_ALL_8_001","MM_CLIMB_002","MM_FLOAT"}){
    fs::path dir=out/id;fs::create_directories(dir);auto r=sim::resolveRetailLevel(lib,id);if(!r){check(false,"resolve");continue;}auto bytes=lib.read(r->file);if(!bytes){check(false,"read");continue;}
    auto back=app::BackdropScene::build(lib,*bytes,id);auto setup=sim::loadRetailLevel(lib,id);auto mesh=findCylinderMesh(*bytes);auto index=indexLevel(*bytes);if(!back||!setup||!mesh||!index){check(false,"load scene");continue;}auto tube=app::TubeScene::build(lib,*mesh,*bytes,index->grid0,true);if(!tube){check(false,"tube");continue;}
    // Decode MM_W_012's initial receivers independently and check the loader
    // after stored-face fan-out and its first update.
    if(std::string_view(id)=="MM_W_012"){
      auto decoded=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(bytes->data()),bytes->size()});
      check(setup->gates.size()==19&&setup->levers.size()==4,"MM_W_012 receiver/lever count");
      check(setup->leverStats.delivered==0,"all four stored initial faces send empty lists");
      for(std::size_t i=0;i<setup->gates.size();++i){
        const auto& gate=setup->gates[i];const auto& raw=decoded.gadgets.entities_42acac[i];
        const bool raised=i>=8&&i<=10;
        check(raw.values_18c_128_168[0]==(raised?0u:1u)&&raw.values_18c_128_168[1]==0,"installed initial request and water kind");
        check(gate.height==(raised?22.5:0.0),"runtime initial receiver height includes load fan-out");
        trace<<"initial gate="<<i<<" id="<<gate.id<<" kind="<<gate.kind<<" request="<<gate.request<<" height="<<gate.height<<'\n';
      }
      for(const auto& lever:setup->levers){
        check(lever.state==lever.initialFace,"stored initial lever face retained");
        check((lever.state==1?lever.facePlus:lever.faceMinus).empty(),"initial face list independently empty");
      }
    }
    sim::World world(std::move(*setup));world.advanceElapsedMs(20);
    std::size_t admitted=0;back->visitRoots([&](auto&source,auto&){admitted+=source.owner=="tagged-lever";});
#ifdef LEVER_RENDER_after
    check(admitted==world.setup().levers.size(),"all installed lever roots admitted");
    auto presentation=back->leverPresentation(*bytes);if(!presentation){check(false,presentation.error());continue;}
    check(presentation->bindings.size()==admitted,"each lever has a track and serialized ID");
#else
    check(admitted==0,"authentic task-entry renderer omits every lever");
#endif
    trace<<id<<" admitted="<<admitted<<" sim="<<world.setup().levers.size()<<'\n';
    std::size_t visibleLeverPixels=0,occludedLeverPixels=0;
    for(int yaw=0;yaw<256;yaw+=32){
      DepthCounts counts;
      auto shot=shoot(*back,*tube,lib,world,yaw,&trace,&counts);auto name="initial-yaw-"+std::to_string(yaw)+".png";png(dir/name,shot);
      visibleLeverPixels+=counts.visible;occludedLeverPixels+=counts.occluded;
      std::ofstream rawImage(dir/(name+".rgba"),std::ios::binary);rawImage.write(reinterpret_cast<const char*>(shot.rgba.data()),shot.rgba.size());
#if defined(LEVER_RENDER_after) && defined(LEVER_BASELINE_AVAILABLE)
      auto data=file(out.parent_path()/"before"/id/(name+".rgba"));
      check(data.size()==shot.rgba.size(),"baseline RGBA exists");
      if(data.size()==shot.rgba.size()){Image baseline;baseline.rgba.assign(data.begin(),data.end());auto n=different(shot,baseline);trace<<"yaw="<<yaw<<" lever_pixels="<<n<<'\n';if(world.setup().levers.empty())check(n==0,"levels without levers pixel-identical");}
#endif
    }
    trace<<"depth totals visible="<<visibleLeverPixels<<" occluded="<<occludedLeverPixels<<'\n';
#ifdef LEVER_RENDER_after
    if(!world.setup().levers.empty()){
      check(visibleLeverPixels>5,"installed scene has visible depth-tested lever pixels");
      check(occludedLeverPixels>5,"installed scenery occludes lever pixels");
    }
#endif
    // Real level/contact fixture: an actor is placed in each actual lever AABB.
    for(std::size_t li=0;li<world.setup().levers.size();++li){
      auto fresh=sim::loadRetailLevel(lib,id);sim::World contact(std::move(*fresh));const auto lever=contact.setup().levers[li];
      sim::Lemming actor;actor.x=lever.x+1;actor.y=lever.y+lever.height;actor.state=sim::State::Walker;actor.facing=static_cast<sim::Facing>(-lever.state);
      for(int y=lever.y;y<=lever.y+lever.height;++y)if(contact.terrain().solid(actor.x,y+1)){actor.y=y;break;}
      contact.place(actor);contact.advanceElapsedMs(16,false,[](double){return true;});
      check(contact.setup().levers[li].state==-lever.state,"actual contact throws lever");
      trace<<"contact id="<<lever.serializedId<<" box="<<lever.x<<','<<lever.y<<" actor="<<actor.x<<','<<actor.y<<" faces="<<lever.state<<','<<contact.setup().levers[li].state<<'\n';
#ifdef LEVER_RENDER_after
      std::vector<std::pair<int,int>> initial,thrown;for(auto&l:world.setup().levers)initial.emplace_back(l.serializedId,l.state);thrown=initial;
      for(auto&f:thrown)if(f.first==lever.serializedId)f.second=contact.setup().levers[li].state;
      std::size_t best=0;int bestYaw=-1;
      for(int yaw=0;yaw<256;yaw+=32){back->setLeverFaces(*presentation,initial);auto a=shoot(*back,*tube,lib,world,yaw);back->setLeverFaces(*presentation,thrown);auto b=shoot(*back,*tube,lib,world,yaw);const auto n=different(a,b);if(n>best){best=n;bestYaw=yaw;png(dir/("lever-"+std::to_string(li)+"-before.png"),a);png(dir/("lever-"+std::to_string(li)+"-thrown.png"),b);}}
      check(best>5,"real lever pixels change with face: "+std::string(id));trace<<"face_pixels="<<best<<" yaw="<<bestYaw<<'\n';
      back->setLeverFaces(*presentation,initial);
#endif
    }
  }
}
void appRoute(const fs::path&game,const fs::path&out){
  fs::create_directories(out);std::ofstream trace(out/"world.csv");trace<<"frame,ms,saved,lost,faces,actors,gate_heights\n";
  auto camera=out/"camera.json";std::ofstream(camera)<<"{\"viewport\":[640,480],\"yaw_units\":160,\"level_eye_height\":96}\n";
  std::vector<std::string>args{"lever-render","--game-dir",game.string(),"--level","MM_W_012","--sim","--native","--camera",camera.string(),"--frames","610","--no-audio"};std::vector<char*> raw;for(auto&a:args)raw.push_back(a.data());
  app::ApplicationRunOptions options;options.frameSeconds=0.1;options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  std::string previousFaces;Image previous;int throws=0,frames=0;
  auto observe=[&](const app::LevelFrameView&f){++frames;std::string faces;for(auto&l:f.world.setup().levers)faces+=std::to_string(l.state)+";";
    trace<<f.frame<<','<<f.world.levelClockMs()<<','<<f.world.saved()<<','<<f.world.dead()<<','<<faces<<',';for(auto&l:f.world.lemmings())trace<<l.x<<':'<<l.y<<':'<<int(l.state)<<';';
    trace<<',';for(const auto& gate:f.world.setup().gates)trace<<gate.id<<':'<<gate.request<<':'<<gate.height<<';';trace<<'\n';
    if(!previousFaces.empty()&&faces!=previousFaces){++throws;png(out/("natural-"+std::to_string(throws)+"-before.png"),previous);png(out/("natural-"+std::to_string(throws)+"-thrown.png"),f.image);}
    if(frames==1){
      png(out/"initial.png",f.image);
      for(const auto& gate:f.world.setup().gates)if(gate.id==5||gate.id==6)
        check(gate.height==0,"bottom receivers start empty in ordinary application");
    }
    if(frames==610){
      png(out/"final.png",f.image);
      for(const auto& gate:f.world.setup().gates)if(gate.id==5||gate.id==6)
        check(gate.height==0,"U3: no-input route loses actors before the lever; bottom receivers stay empty");
      check(f.world.saved()==0&&f.world.dead()>0,"U3: ordinary no-input route cannot cross dry pits to win");
    }
    previous=f.image;previousFaces=faces;
  };
  check(app::runApplication(int(raw.size()),raw.data(),{},observe,options)==0,"ordinary application returned");check(frames>400&&throws==0,"U3: ordinary hatch walkers fall into the first dry pit before reaching a lever");trace.close();
#if defined(LEVER_RENDER_after) && defined(LEVER_BASELINE_AVAILABLE)
  check(file(out/"world.csv")==file(out.parent_path()/"app-before"/"world.csv"),"ordinary scripted route outcomes and every actor state unchanged");
#endif
  std::ofstream(out/"summary.txt")<<"frames="<<frames<<" throws="<<throws<<" failures="<<failures<<'\n';
}
}
int main(int argc,char**argv){
  if(argc<2||!std::getenv("WEBOLUTION_GAME_DIR"))return 2;
  fs::path out=argv[1];fs::create_directories(out);const fs::path game=std::getenv("WEBOLUTION_GAME_DIR");
  auto local=save::localAppDataDirectory();if(!local)return 2;Saved installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  if(argc>2)appRoute(game,out);else{if(measure(argc,argv))return 2;auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;census(*lib,out);fixtures(*lib,out);}
  check(installed.unchanged()&&normal.unchanged(),"installed and normal saves untouched");
  std::ofstream(out/"checks.txt")<<"installed_save_unchanged="<<installed.unchanged()<<" normal_save_unchanged="<<normal.unchanged()<<" failures="<<failures<<'\n';return failures?1:0;
}
