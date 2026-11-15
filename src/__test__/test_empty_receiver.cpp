// Installed receiver regression and production application images. No original execution.
#include "app/AppEntry.h"
#include "FrameConsumptionAudit.h"
#include "app/BackdropScene.h"
#include "app/TubeScene.h"
#include "app/LemmingAnim.h"
#include "assets/Box.h"
#include "assets/CollisionGrid.h"
#include "assets/Png.h"
#include "assets/GameData.h"
#include "assets/LevelIndex.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures=0;
void check(bool ok,const char* why){if(!ok){++failures;std::cerr<<"FAIL: "<<why<<'\n';}}
std::string bytes(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
struct Preserved{fs::path p;bool exists;std::string data;Preserved(fs::path path):p(path),exists(fs::exists(p)),data(bytes(p)){} bool unchanged()const{return exists==fs::exists(p)&&data==bytes(p);}};
// Placed-actor fixture images use the production scene/actor composition and
// state clip at temporal entry zero. The natural route below uses runApplication.
void snapshot(const AssetLibrary& lib,const sim::World& world,const fs::path& path){
 auto level=sim::resolveRetailLevel(lib,"MM_W_012");if(!level){check(false,"snapshot resolve");return;}
 auto data=lib.read(level->file);if(!data){check(false,"snapshot read");return;}
 auto back=app::BackdropScene::build(lib,*data,"MM_W_012");auto mesh=findCylinderMesh(*data);auto index=indexLevel(*data);
 if(!back||!mesh||!index){check(false,"snapshot scene data");return;}
 auto tube=app::TubeScene::build(lib,*mesh,*data,index->grid0,true);if(!tube){check(false,"snapshot tube");return;}
 tube->syncTerrain(world.terrain());
 std::vector<app::BackdropScene::FluidGateState> fluids;for(const auto&g:world.setup().gates)fluids.push_back({g.lane,g.height,g.wavePhase,g.animated,0.0});back->setFluidState(std::move(fluids));
 app::CameraParams p;p.yawUnits=160;p.levelEyeHeight=40;auto camera=app::buildCamera(p);app::Raster raster(640,480);raster.clear(0,0,0);app::BackdropStats stats;using Stage=app::BackdropScene::Stage;
 back->renderStage(raster,camera,lib,stats,Stage::Primary);back->renderStage(raster,camera,lib,stats,Stage::LegacyUnattributed);
 app::SceneOptions opts;opts.drawCellRoute=true;opts.drawWall=false;opts.drawOuter=false;opts.drawInner=false;
 app::LemmingAnimTracker anim;if(tube->lemmingSprites())anim.advance(world,*tube->lemmingSprites(),0.0);
 opts.injectedWalkers=app::injectedWalkersFromWorld(world,world.terrain().height(),&anim);
 auto actors=tube->injectedWalkerScreenBoxes(camera,opts,lib);check(!actors.empty()&&actors[0].has_value(),"fixture actor projects into scene");
 tube->render(raster,camera,opts,&lib);back->renderStage(raster,camera,lib,stats,Stage::FluidGate);back->renderStage(raster,camera,lib,stats,Stage::Secondary);
 Image image;image.width=640;image.height=480;const auto&rgb=raster.framebuffer().rgb;for(std::size_t i=0;i<rgb.size();i+=3)image.rgba.insert(image.rgba.end(),{rgb[i],rgb[i+1],rgb[i+2],255});check(writePng(path,image).has_value(),"retain fixture pixels");
}
void trace(const AssetLibrary& lib,const fs::path& out){
 auto s=sim::loadRetailLevel(lib,"MM_W_012");check(bool(s),"load installed water level");if(!s)return;
 auto grid=loadCollisionGrid(lib,"COMPILED\\MM_W_012.LVL");check(bool(grid),"load raw flags");if(!grid)return;
 std::ofstream cells(out/"cells.csv");cells<<"x,row,raw_flags,loaded_solid,water_zone\n";
 for(int x=152;x<224;++x)if(x<168||x>=216)for(int row=2;row<=30;++row)cells<<x<<','<<row<<','<<grid->at(x,row)<<','<<s->terrain.solid(x,95-row)<<','<<s->terrain.zoneWater(x,95-row)<<'\n';
 sim::World w(std::move(*s));std::ofstream f(out/"natural.csv");f<<"ms,index,x,y,state,fall,probe_row,raw_flags,material_solid,live_zone,saved,dead\n";
 bool floorSplat=false;
 for(int n=0;n<6000&&!w.finished();++n){w.advanceElapsedMs(20);for(std::size_t i=0;i<w.lemmings().size();++i){const auto& l=w.lemmings()[i];f<<w.levelClockMs()<<','<<i<<','<<l.x<<','<<l.y<<','<<sim::stateName(l.state)<<','<<l.fallDistance<<','<<94-l.y<<','<<grid->at(l.x,94-l.y)<<','<<w.terrain().solid(l.x,l.y+1)<<','<<w.terrain().zoneWater(l.x,l.y+1)<<','<<w.saved()<<','<<w.dead()<<'\n';floorSplat|=i==0&&l.x==222&&l.y==91&&l.state==sim::State::Splatter&&l.fallDistance==50;}}
 std::ofstream(out/"natural-summary.txt")<<"saved="<<w.saved()<<"\ndead="<<w.dead()<<"\n";
#ifdef EMPTY_RECEIVER_AFTER
 check(w.saved()==0&&w.dead()==20,"unassisted route no longer wins across dry receivers");
 check(floorSplat,"natural first actor falls fifty cells to the actual receiver floor and splats");
 check(std::all_of(w.setup().gates.begin(),w.setup().gates.end(),[](const auto&g){return g.minY!=4||g.height==0;}),"unassisted route leaves lower receivers empty");
#else
 check(w.saved()==20&&w.dead()==0,"authentic baseline unassisted route wins across dry receivers");
 check(!floorSplat,"baseline phantom lid prevents the natural floor splat");
#endif
}
void fixtures(const AssetLibrary& lib,const fs::path& out){
 std::ofstream f(out/"fixtures.csv");f<<"phase,sheet,ms,x,y,state,fall,height,saved,dead\n";
 for(int phase=0;phase<2;++phase)for(int sheet=0;sheet<=1;++sheet){
  auto s=sim::loadRetailLevel(lib,"MM_W_012");if(!s){check(false,"fixture load");return;}s->totalLemmings=0;
  // Preserve real terrain, receiver geometry and all other gadgets. Start a
  // walker on the actual ledge immediately before the lower [152,168) basin.
  if(phase>0)sim::dispatchLeverMessages(*s,{{5,4,0,0},{6,4,0,0}});
  sim::World w(std::move(*s));if(phase>0)w.advanceElapsedMs(14000);
  sim::Lemming l;l.x=151;l.y=66;l.state=sim::State::Walker;l.sheetType=sheet;w.place(l);
  bool fell=false,drowned=false,crossed=false,splat=false;int maxY=0;
  for(int n=0;n<1200;++n){w.advanceElapsedMs(20);const auto&a=w.lemmings()[0];fell|=a.state==sim::State::Faller;drowned|=a.state==sim::State::Drown;crossed|=a.x>=168&&a.y<=67&&a.state==sim::State::Walker;splat|=a.state==sim::State::Splatter;maxY=std::max(maxY,a.y);f<<phase<<','<<sheet<<','<<w.levelClockMs()<<','<<a.x<<','<<a.y<<','<<sim::stateName(a.state)<<','<<a.fallDistance<<','<<w.setup().gates[12].height<<','<<w.saved()<<','<<w.dead()<<'\n';if(n==50)snapshot(lib,w,out/("fixture-phase-"+std::to_string(phase)+"-sheet-"+std::to_string(sheet)+".png"));}
#ifdef EMPTY_RECEIVER_AFTER
  if(phase==0)check(fell&&maxY>=89&&!crossed,"empty receiver falls to its floor");
  if(phase==1&&sheet==1)check(crossed&&!drowned,"water lemming crosses filled receiver");
  if(phase==1&&sheet==0)check(drowned&&w.dead()==1,"ordinary lemming drowns in filled receiver");
#else
  if(phase==0)check(crossed&&!fell,"baseline walks over empty receiver");
#endif
  (void)splat;
 }
}
#ifdef EMPTY_RECEIVER_AFTER
void transitions(const AssetLibrary& lib,const fs::path& out){
 std::ofstream log(out/"height-transitions.csv");log<<"drain,ms,x,y,state,height\n";
 for(bool drain:{false,true}){
  auto s=sim::loadRetailLevel(lib,"MM_W_012");check(bool(s),"transition load");if(!s)return;
  s->totalLemmings=0;sim::dispatchLeverMessages(*s,{{5,4,0,0},{6,4,0,0}});
  sim::World primed(std::move(*s));primed.advanceElapsedMs(drain?14000:6000);
  auto setup=primed.setup();if(drain)sim::dispatchLeverMessages(setup,{{5,5,0,0},{6,5,0,0}});
  sim::World world(std::move(setup));sim::Lemming l;l.x=162;l.y=95-int(std::floor(world.setup().gates[12].minY+world.setup().gates[12].height));l.state=sim::State::Walker;l.sheetType=1;world.place(l);
  const int initialY=l.y;bool moved=false,fell=false;
  for(int n=0;n<50;++n){world.advanceElapsedMs(20);const auto&a=world.lemmings()[0];const auto&g=world.setup().gates[12];moved|=drain?a.y>initialY:a.y<initialY;fell|=a.state==sim::State::Faller;check(a.state!=sim::State::Drown,"matching water species survives moving surface");log<<drain<<','<<world.levelClockMs()<<','<<a.x<<','<<a.y<<','<<sim::stateName(a.state)<<','<<g.height<<'\n';}
  check(moved,"support follows real rising/draining height");
  const auto& a=world.lemmings()[0];const auto& g=world.setup().gates[12];
  check(std::abs(a.y-(95-int(std::floor(g.minY+g.height))))<=1,"surface rider follows live mean within one cell");
  (void)fell; // A slowly draining surface can also use the ordinary step-down arm.
 }
 // Preserve a real floor/bridge even when it lies inside a receiver. This
 // supplements the installed ledge crossing; the bridge itself is synthetic.
 auto s=sim::loadRetailLevel(lib,"MM_W_012");if(!s){check(false,"bridge load");return;}s->totalLemmings=0;
 s->terrain.fillBox(152,68,16,1,sim::Material::Earth);sim::World world(std::move(*s));sim::Lemming l;l.x=151;l.y=67;l.state=sim::State::Walker;l.sheetType=1;world.place(l);world.advanceElapsedMs(3000);
 check(world.lemmings()[0].x>=166&&world.lemmings()[0].y==67,"dry receiver preserves constructed bridge support");
}
#endif
void application(const fs::path& game,const fs::path& out){
 auto camera=out/"camera.json";std::ofstream(camera)<<"{\"viewport\":[640,480],\"yaw_units\":224,\"level_eye_height\":40}\n";
 std::vector<std::string> args{"empty-receiver","--game-dir",game.string(),"--level","MM_W_012","--sim","--native","--camera",camera.string(),"--frames","300","--no-audio"};std::vector<char*> raw;for(auto&a:args)raw.push_back(a.data());
 app::ApplicationRunOptions opt;opt.frameSeconds=0.1;opt.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
 test::FrameConsumptionAudit audit(out);
 bool captured=false,deep=false,floor=false;std::ofstream f(out/"app.csv");f<<"frame,ms,index,x,y,state,saved,dead\n";
 auto observe=[&](const app::LevelFrameView& v){audit.frame(v);for(std::size_t i=0;i<v.world.lemmings().size();++i){const auto&l=v.world.lemmings()[i];f<<v.frame<<','<<v.world.levelClockMs()<<','<<i<<','<<l.x<<','<<l.y<<','<<sim::stateName(l.state)<<','<<v.world.saved()<<','<<v.world.dead()<<'\n';if(!captured&&l.x>=216&&l.x<224&&l.y>=66){check(writePng(out/"first-lower-pit.png",test::inspectedImage(v)).has_value(),"retain production pit image");captured=true;}
 if(!deep&&l.x>=216&&l.x<224&&l.y>=78&&l.state==sim::State::Faller){check(writePng(out/"inside-empty-pit.png",test::inspectedImage(v)).has_value(),"retain deep fall image");deep=true;}
 if(!floor&&l.x>=216&&l.x<224&&l.state==sim::State::Splatter){check(writePng(out/"pit-floor-splat.png",test::inspectedImage(v)).has_value(),"retain floor splat image");floor=true;}
 }if(v.frame==232||v.frame==238||v.frame==243||v.frame==280)check(writePng(out/("frame-"+std::to_string(v.frame)+".png"),test::inspectedImage(v)).has_value(),"retain matched frame");};
 check(app::runApplication(int(raw.size()),raw.data(),{},observe,opt)==0,"run production application");check(captured,"natural actor reaches lower receiver");
#ifdef EMPTY_RECEIVER_AFTER
 check(deep&&floor,"production app shows ordinary fall and splat in dry receiver");
#endif
}
}
int main(int argc,char**argv){if(argc!=2||!std::getenv("WEBOLUTION_GAME_DIR"))return 2;fs::path out=argv[1],game=std::getenv("WEBOLUTION_GAME_DIR");fs::create_directories(out);auto local=save::localAppDataDirectory();if(!local)return 2;Preserved installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;trace(*lib,out);fixtures(*lib,out);
#ifdef EMPTY_RECEIVER_AFTER
 transitions(*lib,out);
#endif
 application(game,out);check(installed.unchanged()&&normal.unchanged(),"installed and normal saves preserved");std::ofstream(out/"summary.txt")<<"failures="<<failures<<"\ninstalled_save_unchanged="<<installed.unchanged()<<"\nnormal_save_unchanged="<<normal.unchanged()<<'\n';return failures?1:0;}
