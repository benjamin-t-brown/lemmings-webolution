// R8: installed-data regression and source-built production before/after images.
#include "app/AppEntry.h"
#include "app/BackdropScene.h"
#include "app/TubeScene.h"
#include "app/LemmingAnim.h"
#include "app/LemmingMotion.h"
#include "app/MapScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <set>

using namespace webo;
namespace fs=std::filesystem;
namespace {
int failures=0;
void check(bool ok,const std::string& why){if(!ok){if(failures<30)std::cerr<<"FAIL: "<<why<<'\n';++failures;}}
std::string bytes(const fs::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
struct SavedFile{fs::path p;bool exists;std::string data;explicit SavedFile(fs::path path):p(std::move(path)),exists(fs::exists(p)),data(bytes(p)){}bool unchanged()const{return exists==fs::exists(p)&&data==bytes(p);}};
void key(SDL_Scancode k){SDL_Event e{};e.type=SDL_KEYDOWN;e.key.keysym.scancode=k;check(SDL_PushEvent(&e)==1,"enqueue key");}
void point(int x,int y){SDL_Event e{};e.type=SDL_MOUSEMOTION;e.motion.x=x;e.motion.y=y;check(SDL_PushEvent(&e)==1,"enqueue pointer");}
void png(const fs::path& p,const Image& im){check(writePng(p,im).has_value(),"write clone image "+p.string());}
// Nearest-neighbour review aid from the actual application framebuffer. The
// full frame is retained beside it; this does not synthesize actor pixels.
void detail(const fs::path& p,const Image& im,const app::ActorSpritePlacement& box){
  const int left=int(box.originX)-36,top=int(box.originY)-36;Image crop;crop.width=432;crop.height=432;crop.rgba.resize(432*432*4);
  for(int y=0;y<432;++y)for(int x=0;x<432;++x){const int sx=std::clamp(left+x/6,0,im.width-1),sy=std::clamp(top+y/6,0,im.height-1);std::copy_n(im.rgba.begin()+(sy*im.width+sx)*4,4,crop.rgba.begin()+(y*432+x)*4);}
  png(p,crop);
}
std::uint64_t hash(const Image& im){std::uint64_t h=14695981039346656037ull;for(auto c:im.rgba){h^=c;h*=1099511628211ull;}return h;}
std::string state(const sim::World& w){
  std::ostringstream s;s<<std::setprecision(17)<<w.levelClockMs()<<','<<w.tickCount()<<','<<w.explicitUpdateCount()<<','<<w.spawned()<<','<<w.saved()<<','<<w.dead()<<','<<w.terrain().version();
  for(int n:w.setup().skills)s<<','<<n;
  for(const auto& l:w.lemmings()){
    s<<';'<<l.x<<','<<l.y<<','<<int(l.state)<<','<<int(l.facing)<<','<<l.verticalDir<<','<<l.frame<<','<<l.fallDistance<<','<<l.bricksLeft<<','<<l.bashCycles<<','<<l.fuseMs<<','<<l.fasterMs<<','<<l.speedPadLatched<<','<<l.canClimb<<','<<l.canFloat<<','<<l.sheetType<<','<<l.stepBudgetSeconds;
    for(auto p:l.teleportPadsLatched)s<<':'<<p;
  }return s.str();
}
void census(const AssetLibrary& lib,const fs::path& game,const fs::path& out){
  std::array<std::int32_t,102> flags{};
  auto local=save::localAppDataDirectory();check(bool(local),"resolve clone save directory");
  if(local){save::SaveStore store(std::make_unique<save::FileSaveSink>(save::cloneSavePathUnder(*local),game));auto saved=store.load();check(bool(saved),"read current progress without changing it");if(saved)flags=saved->nodeFlags(app::MapScene::nodeMapping());}
  auto map=app::MapScene::build(lib,flags);check(bool(map),"load installed frontend map");if(!map)return;
  std::ofstream f(out/"candidates.txt");
  for(int node:{11,30,33,34,46,49,55,56,59,61,63,65,67,73,80,83,94,96}){
    auto id=map->levelForNode(node);check(bool(id),"candidate has map id");if(!id)continue;
    auto setup=sim::loadRetailLevel(lib,*id);check(bool(setup),"candidate loads");if(!setup)continue;
    f<<node<<' '<<*id<<" title="<<map->titleForNode(node).value_or("?")<<" cells="<<setup->terrain.antiGravCells()<<" current_map_state="<<map->states()[node]<<" reachable="<<(map->states()[node]==2||map->states()[node]==3)<<'\n';
    if(node==11){for(int y=0;y<setup->terrain.height();++y)for(int x=0;x<setup->terrain.width();++x)if(setup->terrain.antiGrav(x,y))f<<"pad "<<x<<','<<y<<'\n';for(const auto& h:setup->entrances)f<<"hatch "<<h.x<<','<<h.y<<'\n';}
  }
}
std::string controlLevel(const AssetLibrary& lib,const fs::path& out){
  // Ordinary scenery seeds flame sprites from wall time. Pick a second real
  // control with no such sprites so the comparison covers EVERY frame pixel.
  auto map=app::MapScene::build(lib,{});if(!map)return {};
  std::ofstream audit(out/"control-selection.txt");std::string selected;
  for(int node=1;node<=10;++node){
    auto id=map->levelForNode(node);if(!id)continue;
    auto setup=sim::loadRetailLevel(lib,*id);auto resolved=sim::resolveRetailLevel(lib,*id);if(!setup||!resolved)continue;
    auto blob=lib.read(resolved->file);if(!blob)continue;
    auto backdrop=app::BackdropScene::build(lib,*blob,*id);if(!backdrop)continue;
    audit<<node<<' '<<*id<<" antigrav="<<setup->terrain.antiGravCells()<<" random_sprites="<<backdrop->spriteCount()<<'\n';
    if(selected.empty()&&setup->terrain.antiGravCells()==0&&backdrop->spriteCount()==0)selected=*id;
  }
  check(!selected.empty(),"second full-frame deterministic installed control exists");audit<<"selected="<<selected<<'\n';return selected;
}
#ifndef ANTIGRAV_BEFORE
// Supplemental placed-actor fixture on UNMODIFIED installed terrain, not a
// natural full-level route. The natural hatch/first flip is tested below.
void returnFlipper(const AssetLibrary& lib,const fs::path& out){
  fs::create_directories(out);
  auto setup=sim::loadRetailLevel(lib,"m06_kf_1");auto resolved=sim::resolveRetailLevel(lib,"m06_kf_1");
  check(bool(setup)&&bool(resolved),"load real return-flipper fixture");if(!setup||!resolved)return;
  auto blob=lib.read(resolved->file);if(!blob){check(false,"read real level bytes");return;}
  auto mesh=findCylinderMesh(*blob);auto index=indexLevel(*blob);if(!mesh||!index){check(false,"real cylinder/index");return;}
  auto tube=app::TubeScene::build(lib,*mesh,*blob,index->grid0);check(bool(tube),"build real return-flipper renderer");if(!tube)return;
  sim::World w(*setup);sim::Lemming l;l.x=24;l.y=8;l.state=sim::State::Walker;l.verticalDir=-1;
  check(w.terrain().antiGrav(l.x,l.y-1),"placed inverted walker touches installed second flipper");
  check(!w.terrain().solid(l.x,l.y),"placed actor is in actual empty space");w.place(l);
  app::LemmingMotionTracker motion;app::LemmingAnimTracker anim;motion.observe(w);
  std::ofstream trace(out/"transition.csv");trace<<"frame,ms,x,y,gravity,state,draw_y,lane,top,bottom,origin_y\n";
  int boxes=0;
  for(int frame=0;frame<8;++frame){
    if(frame)w.advanceElapsedMs(20,false,[&](double){motion.observe(w);return true;});
    anim.advance(w,*tube->lemmingSprites(),frame?0.02:0);
    app::SceneOptions o;o.drawCellRoute=true;o.drawWall=false;o.drawOuter=false;o.drawInner=false;
    o.injectedWalkers=app::injectedWalkersFromWorld(w,w.terrain().height(),&anim,&motion);
    check(o.injectedWalkers[0].verticalDir==(frame?1:-1),"production adapter carries second flip then upright gravity");
    const auto drawn=motion.positionOf(0,w);
    if(frame==1)check(drawn.x==w.lemmings()[0].x&&drawn.y==w.lemmings()[0].y,"gravity change snaps buffered motion");
    app::CameraParams p;p.levelEyeHeight=w.terrain().height();auto camera=app::buildCamera(p);
    app::Raster raster(640,480);raster.clear(19,23,31);tube->render(raster,camera,o,&lib);
    const auto placements=tube->injectedWalkerScreenBoxes(camera,o,lib,raster);
    if(!placements.empty()&&placements[0]){++boxes;const auto& a=*placements[0];const auto& cur=w.lemmings()[0];
      check((a.top+a.bottom-2*a.originY)*(frame?1:-1)<0,"real flipper changes drawn anchor side immediately");
      trace<<frame<<','<<w.levelClockMs()<<','<<cur.x<<','<<cur.y<<','<<cur.verticalDir<<','<<sim::stateName(cur.state)<<','<<drawn.y<<','<<drawn.z<<','<<a.top<<','<<a.bottom<<','<<a.originY<<'\n';
    }
    Image im;im.width=640;im.height=480;im.rgba.assign(640*480*4,255);const auto& rgb=raster.framebuffer().rgb;
    for(std::size_t i=0;i<rgb.size()/3;++i)std::copy_n(rgb.begin()+i*3,3,im.rgba.begin()+i*4);
    png(out/("frame-"+std::to_string(frame)+".png"),im);
  }
  check(boxes==8,"return flip image sequence contains visible actor every frame");
}
void rasterUnits(){
  SpriteFrame f;f.width=4;f.height=4;f.left=0;f.top=0;f.right=4;f.bottom=4;f.uLeft=0;f.uTop=0;f.uRight=1;f.uBottom=1;
  Image sheet;sheet.width=4;sheet.height=4;sheet.rgba.resize(64);
  for(int y=0;y<4;++y)for(int x=0;x<4;++x){auto at=(y*4+x)*4;sheet.rgba[at]=x*40;sheet.rgba[at+1]=y*50;sheet.rgba[at+2]=20;sheet.rgba[at+3]=255;}
  // Independent expected texel coordinates, including horizontal mirror and
  // every supported draw scale. Alpha rejection must follow the flipped texel.
  sheet.rgba[3]=0;
  for(bool horizontal:{false,true})for(int extent:{2,4,8}){
    app::ActorSpritePlacement p{10,10,1,10,10,10.0+extent,10.0+extent};
    auto upright=app::actorSpriteGravityPlacement(p,1);auto inverted=app::actorSpriteGravityPlacement(p,-1);
    check(upright.top==p.top&&upright.bottom==p.bottom&&upright.depth==p.depth,"upright placement unchanged");
    check(inverted.top==10-extent&&inverted.bottom==10&&inverted.left==p.left&&inverted.depth==p.depth,"inversion reflects support anchor and preserves X/depth");
    Image im;im.width=32;im.height=32;im.rgba.assign(32*32*4,0);
    check(bool(app::drawActorSprite(im,f,sheet,p,horizontal,{255,255,255},true,true)),"draw inverted sprite");
    for(int y=0;y<extent;++y)for(int x=0;x<extent;++x){int sx=std::clamp(int(std::floor((horizontal?1.0-double(x)/extent:double(x)/extent)*4)),0,3);int sy=std::clamp(int(std::floor((1.0-double(y)/extent)*4)),0,3);auto src=(sy*4+sx)*4,dst=((10+y)*32+10+x)*4;for(int c=0;c<4;++c)check(im.rgba[dst+c]==(sheet.rgba[src+3]?sheet.rgba[src+c]:0),"independent flipped UV/alpha texel");}
  }
}
#endif
void application(const std::string& id,int frameLimit,const fs::path& game,const fs::path& out){
  fs::create_directories(out);const bool antigrav=id=="m06_kf_1";
  app::ApplicationRunOptions opts;opts.frameSeconds=0.1;
  opts.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  SavedFile privateSave(out/"SavedGame.dat");
  std::ofstream trace(out/"positions.csv"),simTrace(out/"simulation.txt"),pixels(out/"pixels.txt");
  trace<<std::setprecision(17)<<"frame,ms,index,state,x,y,gravity,support,pad,draw_x,draw_y,lane,origin_x,origin_y,top,bottom,clip,temporal,hover,paused\n";
  std::ifstream expectedSim,expectedPixels;
#ifdef ANTIGRAV_BASELINE_AVAILABLE
  const auto before=out.parent_path().parent_path()/"before"/id;
  expectedSim.open(before/"simulation.txt");check(expectedSim.good(),"authentic task-entry simulation exists");
  expectedPixels.open(before/"pixels.txt");check(expectedPixels.good(),"authentic task-entry pixels exist");
#endif
  std::vector<std::string> args{"antigrav","--level",id,"--sim","--frames",std::to_string(frameLimit),"--native","--no-audio","--game-dir",game.string()};std::vector<char*> raw;for(auto& a:args)raw.push_back(a.data());
  int frames=0,inverted=0,ceiling=0,flips=0,backUpright=0,pauseFrames=0,hovered=0,sequence=0,aboveWorld=0;bool pausedOnce=false;int previousGravity=1,previousX=-1,previousY=-1;std::set<int> temporal;
  std::optional<app::LevelActorView> frozen;std::uint64_t frozenMs=0;
  const auto observer=[&](const app::LevelFrameView& f){
    if(antigrav&&f.frame==0)key(SDL_SCANCODE_4); // installed Blocker stock is 30; Climber stock is zero
    ++frames;const auto serial=state(f.world);simTrace<<serial<<'\n';pixels<<hash(f.image)<<'\n';
    if(f.frame==frameLimit-1)png(out/"final.png",f.image);
#ifdef ANTIGRAV_BASELINE_AVAILABLE
    std::string wanted;check(bool(std::getline(expectedSim,wanted))&&serial==wanted,"per-frame simulation equals authentic task entry");
    check(bool(std::getline(expectedPixels,wanted)),"baseline has corresponding pixel frame");if(!antigrav)check(wanted==std::to_string(hash(f.image)),"non-antigravity production route pixel-identical");
#endif
    if(!antigrav||f.world.lemmings().empty())return;
    const auto& l=f.world.lemmings()[0];const int v=l.verticalDir;
    aboveWorld+=l.y<0&&l.state==sim::State::Faller;
    auto actor=std::find_if(f.actors.begin(),f.actors.end(),[](const auto& a){return a.index==0;});
    const bool transition=v!=previousGravity;
    if(transition){++flips;if(v>0)++backUpright;png(out/(v<0?"flip-inverted.png":"flip-upright.png"),f.image);}
    if(actor!=f.actors.end()){
      const auto& a=*actor;const bool supported=f.world.terrain().solid(l.x,l.y+v);
      trace<<f.frame<<','<<f.world.levelClockMs()<<",0,"<<sim::stateName(l.state)<<','<<l.x<<','<<l.y<<','<<v<<','<<supported<<','<<f.world.terrain().antiGrav(l.x,l.y+v)<<','<<a.drawnSample[0]<<','<<a.drawnSample[1]<<','<<a.drawnSample[2]<<','<<a.placement.originX<<','<<a.placement.originY<<','<<a.placement.top<<','<<a.placement.bottom<<','<<a.clip<<','<<a.temporalIndex<<','<<(f.hoveredActor?int(*f.hoveredActor):-1)<<','<<f.paused<<'\n';
      if(v<0){++inverted;temporal.insert(a.temporalIndex);if(l.state==sim::State::Walker&&supported){++ceiling;if(ceiling==1)png(out/"ceiling-first.png",f.image);if(ceiling%5==1&&sequence<12)png(out/("ceiling-"+std::to_string(sequence++)+".png"),f.image);}
        if(!pausedOnce&&f.world.levelClockMs()==9000){pausedOnce=true;key(SDL_SCANCODE_P);}
      }
      if(f.world.levelClockMs()==9000&&!f.paused){png(out/"ceiling-steady.png",f.image);detail(out/"ceiling-steady-detail.png",f.image,a.placement);}
      if(f.paused){++pauseFrames;if(!frozen){frozen=a;frozenMs=f.world.levelClockMs();}
        check(f.world.levelClockMs()==frozenMs&&a.drawnSample==frozen->drawnSample&&a.temporalIndex==frozen->temporalIndex&&a.placement.top==frozen->placement.top,"pause freezes inverted motion, lane, animation and anchor");
        point(int((a.placement.left+a.placement.right)/2),int((a.placement.top+a.placement.bottom)/2));if(pauseFrames>=2){check(f.hoveredActor==0,"hover selects inverted displayed sprite");check(f.markedActor==0,"inverted highlight uses same flipped pixels");++hovered;}if(pauseFrames==4){png(out/"hover.png",f.image);key(SDL_SCANCODE_P);point(620,460);}}
#ifndef ANTIGRAV_BEFORE
      if(v<0)check(a.placement.top+a.placement.bottom>2*a.placement.originY,"inverted box extends below feet");
      else check(a.placement.top+a.placement.bottom<2*a.placement.originY,"upright box extends above feet");
#endif
      if(transition||l.x!=previousX||l.y!=previousY)if(v<0&&sequence<2)png(out/"inverted-fall.png",f.image);
    }
    previousGravity=v;previousX=l.x;previousY=l.y;
  };
  check(app::runApplication(int(raw.size()),raw.data(),{},observer,opts)==0,"production application exits");
  check(privateSave.unchanged(),"direct route does not change even the private save");
  if(antigrav){check(flips>0&&inverted>0&&ceiling>5,"natural installed route flips, rises and walks on ceiling");check(temporal.size()>2,"inverted actor animates");check(hovered>=2,"inverted production hover exercised");}
#ifdef ANTIGRAV_BASELINE_AVAILABLE
  std::string extra;check(!std::getline(expectedSim,extra)&&!std::getline(expectedPixels,extra),"identical baseline frame counts");
#endif
  std::ofstream(out/"summary.txt")<<"frames="<<frames<<"\ninverted_visible_frames="<<inverted<<"\nceiling_walking_frames="<<ceiling<<"\nflips="<<flips<<"\nreturned_upright="<<backUpright<<"\nanimation_entries="<<temporal.size()<<"\nhovered_frames="<<hovered<<"\nabove_world_faller_frames="<<aboveWorld<<"\nfailures="<<failures<<'\n';
}
}
int main(int argc,char** argv){
  if(argc!=2||!std::getenv("WEBOLUTION_GAME_DIR"))return 2;
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR")),out(argv[1]);fs::create_directories(out);
  auto local=save::localAppDataDirectory();if(!local)return 2;SavedFile installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;census(*lib,game,out);
#ifndef ANTIGRAV_BEFORE
  rasterUnits();returnFlipper(*lib,out/"placed-return-flipper");
#endif
  application("m06_kf_1",220,game,out/"m06_kf_1");
  application("MM_CLIMB_002",160,game,out/"MM_CLIMB_002");
  const auto control=controlLevel(*lib,out);if(!control.empty())application(control,160,game,out/control);
  check(installed.unchanged()&&normal.unchanged(),"installed and normal saves untouched");
  std::ofstream(out/"summary.txt")<<"installed_save_unchanged="<<installed.unchanged()<<"\nnormal_save_unchanged="<<normal.unchanged()<<"\nfailures="<<failures<<'\n';
  std::cout<<"anti-gravity evidence: "<<out<<" failures="<<failures<<'\n';return failures?1:0;
}
