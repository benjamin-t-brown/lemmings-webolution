// T5(a,c): real installed assets and source-built clone evidence. No original execution.
#include "app/AppEntry.h"
#include "app/BackdropScene.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/Png.h"
#include "assets/LevelIndex.h"
#include "sim/RetailLevel.h"
#include "lemmings/compiled_level.hpp"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
using namespace webo;
namespace fs=std::filesystem;
namespace {
int failures=0;
void check(bool ok,const std::string& why){if(!ok){if(failures<30)std::cerr<<"FAIL: "<<why<<'\n';++failures;}}
std::string bytes(const fs::path&p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
struct Preserved{fs::path p;bool exists;std::string data;explicit Preserved(fs::path path):p(std::move(path)),exists(fs::exists(p)),data(bytes(p)){}bool unchanged()const{return exists==fs::exists(p)&&data==bytes(p);}};
void png(const fs::path&p,const Image&im){check(writePng(p,im).has_value(),"retain clone image "+p.string());}
Image image(const app::Raster&r){Image im;im.width=r.framebuffer().width;im.height=r.framebuffer().height;const auto&rgb=r.framebuffer().rgb;im.rgba.reserve(rgb.size()/3*4);for(std::size_t p=0;p<rgb.size();p+=3)im.rgba.insert(im.rgba.end(),{rgb[p],rgb[p+1],rgb[p+2],255});return im;}
std::uint64_t hash(const Image&im){std::uint64_t h=14695981039346656037ull;for(auto c:im.rgba){h^=c;h*=1099511628211ull;}return h;}
std::string state(const sim::World&w){std::ostringstream f;f<<std::setprecision(17)<<w.levelClockMs()<<','<<w.spawned()<<','<<w.saved()<<','<<w.dead()<<','<<w.activeCount();for(const auto&l:w.lemmings())f<<';'<<l.x<<','<<l.y<<','<<l.verticalDir<<','<<int(l.state)<<','<<l.frame<<','<<l.fallDistance<<','<<l.stepBudgetSeconds<<','<<l.sheetType;return f.str();}
void assets(const AssetLibrary&library,const fs::path&out){const auto*lib=&library;
 auto resolved=sim::resolveRetailLevel(*lib,"m06_kf_1");if(!resolved){check(false,"resolve gravity asset");return;}auto blob=lib->read(resolved->file);if(!blob){check(false,"read gravity asset");return;}
 auto level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(blob->data()),blob->size()});
 std::ofstream f(out/"assets.txt");
 const auto dump=[&](const auto& entities,const char* name){
  for(std::size_t i=0;i<entities.size();++i){const auto&e=entities[i];f<<name<<'['<<i<<"] id="<<e.id;if constexpr(requires{e.type;})f<<" tag="<<e.type;for(auto d:e.double_bits)f<<' '<<std::bit_cast<double>(d);
   if(e.model){f<<" model="<<e.model->name<<" type="<<e.model->type;if(e.model->object){f<<" root="<<e.model->object->source_extent.typed_root_begin;for(const auto&a:e.model->object->auxiliary_records){std::ofstream raw(out/(a.name+"-"+std::to_string(i)+".bin"),std::ios::binary);raw.write(reinterpret_cast<const char*>(a.serialized_bytes.data()),a.serialized_bytes.size());f<<" aux="<<a.name<<":"<<a.serialized_bytes.size()<<" duration="<<std::bit_cast<double>(a.leading_double_bits)<<" transforms="<<a.first_count<<" uv_tracks="<<a.second_count<<" events="<<a.third_count;}}}f<<'\n';}
 };
 const auto&g=level.gadgets;dump(g.basic_entities,"basic_entities");dump(g.linked_entities,"linked_entities");dump(g.spatial_entities,"spatial_entities");dump(g.basic_4342bf,"basic_4342bf");dump(g.spatial_41e038,"spatial_41e038");dump(g.tagged_entities,"tagged_entities");dump(g.named_entities,"named_entities");
 }
void natural(const AssetLibrary&lib,const fs::path&out){
 auto setup=sim::loadRetailLevel(lib,"m06_kf_1");check(bool(setup),"load actual gravity level");if(!setup)return;
 sim::World w(*setup);std::ofstream trace(out/"natural.csv");trace<<"ms,index,x,y,gravity,state,fall,dead,active\n";
 bool flip=false,ceiling=false,escaped=false,splat=false,dead=false;int firstLoss=-1;
 for(int ms=0;ms<=24000;ms+=20){
  if(ms)w.advanceElapsedMs(20);
  check(w.spawned()==w.activeCount()+w.saved()+w.dead(),"natural route counts every released lemming exactly once");
  for(std::size_t i=0;i<w.lemmings().size();++i){const auto&l=w.lemmings()[i];trace<<ms<<','<<i<<','<<l.x<<','<<l.y<<','<<l.verticalDir<<','<<sim::stateName(l.state)<<','<<l.fallDistance<<','<<w.dead()<<','<<w.activeCount()<<'\n';
#ifndef GRAVITY_GADGETS_BEFORE
   check(!l.active()||l.verticalDir>=0||(l.y>=0&&l.y<w.terrain().height()),"no inverted actor remains active outside the installed world");
#endif
   if(i==0){flip|=l.verticalDir<0;ceiling|=l.verticalDir<0&&l.state==sim::State::Walker&&w.terrain().solid(l.x,l.y-1);escaped|=l.y<0;splat|=l.state==sim::State::Splatter;dead|=l.state==sim::State::Dead;if(firstLoss<0&&(splat||dead))firstLoss=ms;}
  }
 }
 check(flip&&ceiling,"natural hatch route flips then lands on a real ceiling");
#ifdef GRAVITY_GADGETS_BEFORE
 check(escaped&&!dead,"authentic entry reproduces the roof-escape soft lock");
#else
 check(!escaped&&splat&&dead&&firstLoss>=18000&&firstLoss<=20000,"natural upward escape becomes a counted loss and terminates");
#endif
 std::ofstream(out/"natural-summary.txt")<<"flipped="<<flip<<"\nceiling_landed="<<ceiling<<"\nescaped="<<escaped<<"\nsplat="<<splat<<"\ndead="<<dead<<"\nfirst_loss_ms="<<firstLoss<<"\nworld_dead="<<w.dead()<<'\n';
}
#ifndef GRAVITY_GADGETS_BEFORE
void falls(const AssetLibrary&lib,const fs::path&out){
 auto loaded=sim::loadRetailLevel(lib,"m06_kf_1");if(!loaded){check(false,"load ceiling fixture");return;}auto setup=*loaded;setup.totalLemmings=0;setup.entrances.clear();setup.exits.clear();
 int x=-1,roof=-1;
 for(int c=0;c<setup.terrain.width()&&x<0;++c)for(int y=0;y+33<setup.terrain.height();++y){if(!setup.terrain.solid(c,y)||setup.terrain.trampoline(c,y)||setup.terrain.waterHazard(c,y))continue;bool clear=true;for(int d=1;d<=33;++d)clear&=!setup.terrain.solid(c,y+d)&&!setup.terrain.waterHazard(c,y+d);if(clear){x=c;roof=y;break;}}
 check(x>=0,"real level supplies a 31-cell ceiling drop");if(x<0)return;
 std::ofstream trace(out/"placed-falls.csv");trace<<"distance,float,ms,x,y,state,fall,dead\n";
 for(int distance:{0,30,31})for(bool floating:{false,true}){
  sim::World w(setup);sim::Lemming l;l.x=x;l.y=roof+1+distance;l.verticalDir=-1;l.canFloat=floating;l.state=sim::State::Faller;w.place(l);
  int ms=0;while(ms<5000&&(w.lemmings()[0].state==sim::State::Faller||w.lemmings()[0].state==sim::State::Floater)){w.advanceElapsedMs(20);ms+=20;const auto&a=w.lemmings()[0];trace<<distance<<','<<floating<<','<<ms<<','<<a.x<<','<<a.y<<','<<sim::stateName(a.state)<<','<<a.fallDistance<<','<<w.dead()<<'\n';}
  const auto&a=w.lemmings()[0];check(a.y==roof+1,"inverted fall stops adjacent to the actual ceiling, including immediate contact");
  const bool lethal=distance>30&&!floating;check(a.state==(lethal?sim::State::Splatter:sim::State::Walker),"same strict >30 splat rule and floater exemption on a ceiling");check(w.dead()==int(lethal),"ceiling loss counted exactly once");
  if(lethal){w.advanceElapsedMs(4000);check(w.lemmings()[0].state==sim::State::Dead&&w.dead()==1&&w.finished(),"ceiling splat ends and cannot strand level completion");}
 }
 // The real roof escape column, with the same production World entry point.
 sim::World w(setup);sim::Lemming l;l.x=104;l.y=0;l.verticalDir=-1;l.state=sim::State::Faller;check(!w.terrain().solid(l.x,0),"natural roof escape column is open");w.place(l);w.advanceElapsedMs(200);check(w.dead()==1&&w.lemmings()[0].state==sim::State::Splatter,"open roof escape is a loss independent of fall distance");w.advanceElapsedMs(4000);check(w.finished()&&w.dead()==1,"roof loss counted once and terminates");
}
#endif
void scenery(const AssetLibrary&lib,const fs::path&out){
 auto resolved=sim::resolveRetailLevel(lib,"m06_kf_1");if(!resolved)return;auto blob=lib.read(resolved->file);if(!blob)return;
 auto mesh=findCylinderMesh(*blob);auto idx=indexLevel(*blob);if(!mesh||!idx){check(false,"real cylinder/index");return;}
 auto tube=app::TubeScene::build(lib,*mesh,*blob,idx->grid0);auto back=app::BackdropScene::build(lib,*blob,"m06_kf_1");if(!tube||!back){check(false,"build production scene");return;}
 auto level=lemmings::read_compiled_level({reinterpret_cast<const std::uint8_t*>(blob->data()),blob->size()});
 std::ofstream audit(out/"render.csv");audit<<"pad,sample,root_count,stores,visible_pixels,min_x,min_y,max_x,max_y,image_hash\n";
 int gravityRoots=0;back->visitRoots([&](const auto&s,const SceneNode&n){if(s.owner=="tagged-gravity"){++gravityRoots;audit<<"root,"<<s.entityIndex<<','<<n.sourceOffset<<','<<n.matrix[9]<<','<<n.matrix[10]<<','<<n.matrix[11]<<'\n';}});
#ifdef GRAVITY_GADGETS_BEFORE
 check(gravityRoots==0,"authentic production scene omits both gravity models");
#else
 check(gravityRoots==2,"both installed gravity gadgets admitted");
 auto animation=back->gravityPresentation(*blob);check(bool(animation),animation?"UV animation parsed":animation.error());if(!animation)return;
 check(animation->bindings.size()==2,"both installed flippers have animation bindings");
 for(const auto& b:animation->bindings){check(b.tracks.size()==2&&std::abs(b.duration-3.3333332538604736)<1e-10,"installed UV clip layout/duration");for(const auto&t:b.tracks)check(t.keys.size()==100&&!t.corners.empty(),"100 UV keys bind real mesh corners");}
#endif
 for(int pad=0;pad<2;++pad){
  const auto&e=level.gadgets.tagged_entities[pad];const int cx=int(std::bit_cast<double>(e.double_bits[0]));const int nativeY=int(std::bit_cast<double>(e.double_bits[1]));
  auto setup=sim::loadRetailLevel(lib,"m06_kf_1");check(bool(setup),"real terrain for gadget placement");if(!setup)return;
  check(setup->terrain.antiGrav(cx,setup->terrain.height()-nativeY)||setup->terrain.antiGrav(cx,setup->terrain.height()-1-nativeY),"placed gadget neighbours the actual gravity-change cell");
  app::CameraParams p;p.yawUnits=cx;p.levelEyeHeight=96;auto cam=app::buildCamera(p);
 std::uint64_t lastHash=0;
  for(int sample=0;sample<3;++sample){
#ifndef GRAVITY_GADGETS_BEFORE
   back->advanceScenery(sample==0?0:100,true,sample==2);back->animateGravity(*animation);
   if(pad==0&&sample==1){
    // Independent value from the installed first 3 UV intervals: at 100 ms
    // V has advanced 0.03028485, with no change to U or placed geometry.
    const auto& binding=animation->bindings[0];const auto& c=binding.tracks[0].corners[0];
    std::size_t root=0;back->visitRoots([&](const auto&,const SceneNode& n){
     if(root++!=binding.root)return;
     std::vector<const SceneNode*> nodes;
     const auto collect=[&](auto& self,const SceneNode& node)->void{nodes.push_back(&node);for(const auto& child:node.children)self(self,child);};collect(collect,n);
     const auto& uv=nodes[c.node]->meshes[c.mesh].records[c.record].corners[c.corner];
     check(std::abs(uv.u-c.u)<1e-12&&std::abs(uv.v-c.v-0.03028485)<1e-7,"installed UV intervals sampled at 100 ms");
    });
   }
#endif
   app::Raster r(640,480);r.clear(0,0,0);app::BackdropStats stats;using Stage=app::BackdropScene::Stage;
   std::vector<std::uint8_t> mask(640*480,1);app::SceneRasterObserver observer;observer.mask=&mask;
   std::map<std::size_t,std::array<std::uint8_t,4>> writes;int stores=0;
   const auto&extent=e.model->object->source_extent;
   observer.accepted=[&](const app::SceneRasterWriteEvent&v){if(v.source>=extent.typed_root_begin&&v.source<extent.tree_end){++stores;writes[v.pixel]=v.colorAfter;check(v.depthAssigned&&v.depthAfter<v.depthBefore,"gadget uses scene depth test/write");}};
   back->renderStage(r,cam,lib,stats,Stage::Primary);back->renderStage(r,cam,lib,stats,Stage::LegacyUnattributed,&observer);
   app::SceneOptions opts;opts.drawCellRoute=true;opts.drawWall=false;opts.drawOuter=false;opts.drawInner=false;tube->render(r,cam,opts,&lib);
   back->renderStage(r,cam,lib,stats,Stage::FluidGate);back->renderStage(r,cam,lib,stats,Stage::Secondary);
   int visible=0,x0=640,y0=480,x1=0,y1=0;const auto&rgb=r.framebuffer().rgb;
   for(const auto&[pixel,c]:writes)if(std::equal(c.begin(),c.begin()+3,rgb.begin()+3*pixel)){++visible;int x=pixel%640,y=pixel/640;x0=std::min(x0,x);x1=std::max(x1,x);y0=std::min(y0,y);y1=std::max(y1,y);}
   const auto im=image(r);const auto h=hash(im);audit<<pad<<','<<sample<<','<<gravityRoots<<','<<stores<<','<<visible<<','<<x0<<','<<y0<<','<<x1<<','<<y1<<','<<h<<'\n';png(out/("pad-"+std::to_string(pad)+"-"+std::to_string(sample)+".png"),im);
#ifdef GRAVITY_GADGETS_BEFORE
   check(stores==0&&visible==0,"no flipper pixels in authentic before scene");
#else
   check(stores>20&&visible>10,"flipper pixels survive the completed real-level scene at each placement");
   if(sample==1)check(h!=lastHash,"stored UV animation changes the real flipper pixels");
   if(sample==2)check(h==lastHash,"pause freezes the gravity animation pixels");
   app::Raster blocked(640,480);blocked.clear(0,0,0);std::fill(blocked.mutableDepthBuffer().begin(),blocked.mutableDepthBuffer().end(),0.0);app::BackdropStats blockedStats;int oldStores=stores;back->renderStage(blocked,cam,lib,blockedStats,Stage::LegacyUnattributed,&observer);check(stores==oldStores,"near scene depth completely occludes gadget pixels");
#endif
   lastHash=h;
  }
 }
}
void application(const AssetLibrary&,const fs::path&game,const fs::path&base,const std::string&id,int count){
 const auto out=base/id;fs::create_directories(out);app::ApplicationRunOptions opts;opts.frameSeconds=.1;opts.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));Preserved privateSave(out/"SavedGame.dat");
 std::vector<std::string> args{"gravity-gadgets","--level",id,"--sim","--native","--no-audio","--frames",std::to_string(count),"--game-dir",game.string()};std::vector<char*>argv;for(auto&a:args)argv.push_back(a.data());
 std::ofstream states(out/"states.txt"),pixels(out/"pixels.txt"),trace(out/"positions.csv");trace<<"frame,ms,x,y,gravity,state,fall,dead,active\n";int frames=0,births=0;bool escaped=false,terminated=false;
#ifdef GRAVITY_GADGETS_BASELINE_AVAILABLE
 std::ifstream expectedStates(base.parent_path()/"before"/id/"states.txt"),expectedPixels(base.parent_path()/"before"/id/"pixels.txt");check(bool(expectedStates)&&bool(expectedPixels),"authentic entry route evidence exists");
#endif
 const auto observe=[&](const app::LevelFrameView&f){++frames;auto serial=state(f.world);auto h=hash(f.image);states<<serial<<'\n';pixels<<h<<'\n';
#ifdef GRAVITY_GADGETS_BASELINE_AVAILABLE
  std::string s,p;check(bool(std::getline(expectedStates,s))&&bool(std::getline(expectedPixels,p)),"matching entry frame exists");if(id=="MM_CLIMB_002"){check(s==serial,"unchanged right-facing non-gravity simulation/outcomes");check(p==std::to_string(h),"unchanged right-facing non-gravity production pixels");}
  else if(id=="m06_kf_1" && f.world.levelClockMs()<=18900)check(s==serial,"gravity route physics/timing identical until the demonstrated roof escape");
#endif
  // hatch-release-direction-001: this control stores fixed left. The old
  // simulation baseline hard-coded right, so its later motion/pixels change.
  // Keep both traces and pin the corrected natural births instead of comparing
  // the obsolete all-right trajectory. MM_CLIMB_002 remains an exact control.
  if(id=="MM_BASH_002")while(births<f.world.spawned()) {
#ifdef GRAVITY_GADGETS_BEFORE
    check(f.world.lemmings()[births].facing==sim::Facing::Right,"historical all-right hatch control");
#else
    check(f.world.lemmings()[births].facing==sim::Facing::Left,"stored fixed-left hatch control");
#endif
    ++births;
  }
  const auto ms=f.world.levelClockMs();if(f.frame==0||f.frame==count-1||ms==7400||ms==9000||ms==18900||ms==19000||ms==20600)png(out/("frame-"+std::to_string(ms)+".png"),f.image);
  if(!f.world.lemmings().empty()){const auto&l=f.world.lemmings()[0];escaped|=l.y<0;terminated|=l.state==sim::State::Dead;trace<<f.frame<<','<<ms<<','<<l.x<<','<<l.y<<','<<l.verticalDir<<','<<sim::stateName(l.state)<<','<<l.fallDistance<<','<<f.world.dead()<<','<<f.world.activeCount()<<'\n';}
 };
 check(app::runApplication(int(argv.size()),argv.data(),{},observe,opts)==0,"bounded production application route returns success");check(frames==count,"all production frames observed");check(privateSave.unchanged(),"direct demonstration leaves private save untouched");
 if(id=="m06_kf_1"){
#ifdef GRAVITY_GADGETS_BEFORE
  check(escaped&&!terminated,"production application reproduces apparent stuck actor");
#else
  check(!escaped&&terminated,"production application loses the roof faller");
#endif
 }
 std::ofstream(out/"summary.txt")<<"frames="<<frames<<"\nescaped="<<escaped<<"\nterminated="<<terminated<<"\nprivate_save_unchanged="<<privateSave.unchanged()<<'\n';
}
}
int main(int argc,char**argv){
 if(argc!=2||!std::getenv("WEBOLUTION_GAME_DIR"))return 2;
 fs::path out(argv[1]),game(std::getenv("WEBOLUTION_GAME_DIR"));fs::create_directories(out);
 auto local=save::localAppDataDirectory();if(!local)return 2;Preserved installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;
 assets(*lib,out);natural(*lib,out);scenery(*lib,out);
#ifndef GRAVITY_GADGETS_BEFORE
 falls(*lib,out);
#endif
 application(*lib,game,out,"m06_kf_1",240);application(*lib,game,out,"MM_CLIMB_002",160);application(*lib,game,out,"MM_BASH_002",160);
 check(installed.unchanged()&&normal.unchanged(),"installed and normal saves untouched");std::ofstream(out/"summary.txt")<<"failures="<<failures<<"\ninstalled_save_unchanged="<<installed.unchanged()<<"\nnormal_save_unchanged="<<normal.unchanged()<<'\n';std::cout<<"gravity gadget evidence "<<out<<" failures="<<failures<<'\n';return failures?1:0;
}
