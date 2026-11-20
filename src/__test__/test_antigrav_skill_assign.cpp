// Assign a skill on the installed ceiling route through SDL panel selection and hover.
#include "app/AppEntry.h"
#include "app/MapScene.h"
#include "app/RuntimePick.h"
#include "app/RuntimeHud.h"
#include "assets/Box.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool ok, const std::string& why) {
  if (!ok) { ++failures; if (failures < 30) std::cerr << "FAIL: " << why << '\n'; }
}
std::string bytes(const fs::path& p) { std::ifstream f(p, std::ios::binary); return {std::istreambuf_iterator<char>(f), {}}; }
struct Preserved {
  fs::path p; bool exists; std::string data;
  explicit Preserved(fs::path path): p(std::move(path)), exists(fs::exists(p)), data(bytes(p)) {}
  bool unchanged() const { return exists == fs::exists(p) && data == bytes(p); }
};
void key(SDL_Scancode k) { SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.keysym.scancode = k; check(SDL_PushEvent(&e) == 1, "queue key"); }
void point(int x, int y) { SDL_Event e{}; e.type = SDL_MOUSEMOTION; e.motion.x = x; e.motion.y = y; check(SDL_PushEvent(&e) == 1, "queue pointer"); }
void click(int x, int y) {
  SDL_Event e{}; e.type = SDL_MOUSEBUTTONDOWN; e.button.button = SDL_BUTTON_LEFT; e.button.x = x; e.button.y = y;
  check(SDL_PushEvent(&e) == 1, "queue click"); e.type = SDL_MOUSEBUTTONUP; check(SDL_PushEvent(&e) == 1, "queue release");
}
void png(const fs::path& p, const Image& im) { check(writePng(p, im).has_value(), "write clone image"); }
void detail(const fs::path& p, const Image& im, const app::ActorSpritePlacement& a) {
  Image crop; crop.width = crop.height = 384; crop.rgba.resize(384*384*4);
  for (int y=0;y<384;++y) for (int x=0;x<384;++x) {
    const int sx=std::clamp(int(a.originX)-32+x/6,0,im.width-1), sy=std::clamp(int(a.originY)-32+y/6,0,im.height-1);
    std::copy_n(im.rgba.begin()+(sy*im.width+sx)*4,4,crop.rgba.begin()+(y*384+x)*4);
  }
  png(p,crop);
}
void record(std::ostream& o, const sim::World& w, std::string_view event) {
  for (std::size_t i=0;i<w.lemmings().size();++i) {
    const auto& l=w.lemmings()[i];
    o<<event<<','<<w.levelClockMs()<<','<<i<<','<<l.x<<','<<l.y<<','<<l.verticalDir<<','<<int(l.facing)<<','<<sim::stateName(l.state)<<','<<w.saved()<<','<<w.dead()<<'\n';
  }
}
void application(const fs::path& game, const fs::path& out) {
  app::ApplicationRunOptions options; options.frameSeconds=0.1;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  Preserved privateSave(out/"SavedGame.dat");
  std::vector<std::string> args{"antigrav-skill","--level","m06_kf_1","--sim","--native","--no-audio","--frames","230","--game-dir",game.string()};
  std::vector<char*> argv; for (auto& a:args) argv.push_back(a.data());
  std::ofstream trace(out/"pointer-route.csv"); trace<<"event,ms,index,x,y,gravity,facing,state,saved,dead\n";
  int stage=0,stock=0,target=-1; bool flipped=false,assigned=false,turned=false; int turns=0;
  std::map<std::size_t,sim::Lemming> previous;
  const auto observe=[&](const app::LevelFrameView& f) {
    if (f.frame==0) click(50,148); // Blocker plate 3; selection uses stock gate.
    record(trace,f.world,"frame");
    if (f.world.levelClockMs()>=13300 && f.world.levelClockMs()<=14200) {
      png(out/("turn-sequence-"+std::to_string(f.world.levelClockMs())+".png"),f.image);
    }
    for (const auto& l:f.world.lemmings()) flipped |= l.verticalDir<0;
    if (stage==0 && f.world.levelClockMs()>=9000) {
      for (const auto& a:f.actors) {
        const auto& l=f.world.lemmings()[a.index];
        if (l.verticalDir>=0 || l.state!=sim::State::Walker || l.x<71 || l.x>77) continue;
        target=int(a.index); stock=f.world.setup().skills[3];
        key(SDL_SCANCODE_P); point(int((a.placement.left+a.placement.right)/2),int((a.placement.top+a.placement.bottom)/2)); stage=1; break;
      }
    } else if (stage==1 && f.paused) {
      check(f.selectedSkillIndex==3,"panel click selects installed Blocker");
      check(f.hoveredActor.has_value(),"production hover targets inverted sprite");
      if (f.hoveredActor) target=int(*f.hoveredActor);
      const auto& l=f.world.lemmings()[target]; check(l.verticalDir<0 && l.state==sim::State::Walker,"picked actor walks inverted after natural flipper");
      check(f.world.terrain().solid(l.x,l.y-1) && !f.world.terrain().solid(l.x,l.y+1),"ceiling support and empty upright support");
      png(out/"before-click.png",f.image);
      for (const auto& a:f.actors) if (int(a.index)==target) detail(out/"before-click-detail.png",f.image,a.placement);
      trace<<"hover,"<<f.world.levelClockMs()<<','<<target<<','<<f.cursorX<<','<<f.cursorY<<",-1,0,hover,0,0\n";
      click(f.cursorX,f.cursorY); stage=2;
    } else if (stage==2) {
      const auto& l=f.world.lemmings()[target]; assigned=l.state==sim::State::Blocker;
#ifdef ANTIGRAV_SKILL_BEFORE
      check(!assigned && f.world.setup().skills[3]==stock && f.paused,"task-entry same production click refused with stock/pause unchanged");
      key(SDL_SCANCODE_P);
#else
      check(assigned && f.world.setup().skills[3]==stock-1 && !f.paused,"production click enters ceiling Blocker, spends one, resumes");
#endif
      record(trace,f.world,"after-click"); png(out/"after-click.png",f.image);
      for (const auto& a:f.actors) if (int(a.index)==target) detail(out/"after-click-detail.png",f.image,a.placement);
      stage=3;
    }
    if (assigned && stage==3) {
      const auto& blocker=f.world.lemmings()[target];
      check(blocker.state==sim::State::Blocker && blocker.verticalDir==-1,"ceiling Blocker remains supported");
      for (std::size_t i=0;i<f.world.lemmings().size();++i) {
        const auto& l=f.world.lemmings()[i]; auto p=previous.find(i);
        if (int(i)==target || p==previous.end() || l.verticalDir!=-1 || l.state!=sim::State::Walker || p->second.state!=sim::State::Walker) continue;
        if (p->second.facing!=l.facing && std::abs(p->second.x-blocker.x)<=2) {
          turned=true; ++turns; record(trace,f.world,"blocker-turn"); png(out/("turn-"+std::to_string(turns)+".png"),f.image);
          for (const auto& a:f.actors) if (int(a.index)==target) detail(out/("turn-"+std::to_string(turns)+"-detail.png"),f.image,a.placement);
        }
      }
    }
    previous.clear(); for (std::size_t i=0;i<f.world.lemmings().size();++i) previous.emplace(i,f.world.lemmings()[i]);
    if(f.frame==229) {png(out/"final.png",f.image);record(trace,f.world,"final");}
  };
  check(app::runApplication(int(argv.size()),argv.data(),{},observe,options)==0,"production route returns");
  check(flipped && stage==3,"natural flipper, ceiling hover and click all exercised");
#ifndef ANTIGRAV_SKILL_BEFORE
  check(turned,"ceiling Blocker turns another naturally inverted walker");
#endif
  check(privateSave.unchanged(),"private save unchanged by direct level route");
  std::ofstream(out/"pointer-summary.txt")<<"assigned="<<assigned<<"\nturns="<<turns<<"\nprivate_save_unchanged="<<privateSave.unchanged()<<'\n';
}
void audit(const AssetLibrary& lib,const fs::path& out) {
  auto loaded=sim::loadRetailLevel(lib,"m06_kf_1"); check(bool(loaded),"load named installed level"); if(!loaded)return;
  std::array<std::int32_t,102> flags{}; auto map=app::MapScene::build(lib,flags); check(bool(map),"parse installed map");
  std::ofstream data(out/"installed.txt");
  if(map) { data<<"node11="<<map->levelForNode(11).value_or("?")<<" title="<<map->titleForNode(11).value_or("?")<<'\n'; check(map->levelForNode(11)=="m06_kf_1","named map id"); }
  data<<"skills"; for(int n:loaded->skills)data<<','<<n;data<<"\nrequired="<<loaded->saveTarget<<"\n";
  for(const auto&e:loaded->exits)data<<"exit,"<<e.x<<','<<e.y<<','<<e.width<<','<<e.height<<'\n';
  for(int y=0;y<loaded->terrain.height();++y){data<<y<<' ';for(int x=0;x<loaded->terrain.width();++x)data<<(loaded->terrain.antiGrav(x,y)?'G':loaded->terrain.solid(x,y)?'#':'.');data<<'\n';}
  if (map) {
    std::ofstream census(out/"gravity-level-skills.csv");census<<"node,id,title,climber,floater,bomber,blocker,builder,basher,miner,digger\n";
    for(int n=0;n<102;++n)if(auto id=map->levelForNode(n)) {
      auto setup=sim::loadRetailLevel(lib,*id);if(!setup || !setup->terrain.antiGravCells())continue;
      census<<n<<','<<*id<<','<<map->titleForNode(n).value_or("?");for(int stock:setup->skills)census<<','<<stock;census<<'\n';
    }
  }
  std::ofstream skills(out/"skills.csv"); skills<<"skill,installed_stock,inverted_assigned,state,ability,fuse\n";
  // Identical real-data natural walk, fresh World for every available job.
  for(int s=0;s<8;++s) {
    auto setup=*loaded;
    const bool offered=setup.skills[s]>0;
    if(!offered) setup.skills[s]=1; // supplemental stock only; explicitly recorded.
    sim::World w(setup); std::size_t index=0; bool reached=false;
    for(int ms=0;ms<20000 && !reached;ms+=20){w.advanceElapsedMs(20);for(std::size_t i=0;i<w.lemmings().size();++i){const auto&l=w.lemmings()[i];if(l.verticalDir==-1 && l.state==sim::State::Walker && l.x>=71 && l.x<=77){index=i;reached=true;break;}}}
    check(reached,"natural ceiling actor for skill audit"); if(!reached)continue;
    check(app::passesKnownPickExclusions(w.lemmings()[index],sim::Skill(s)),"runtime candidate accepts naturally inverted Walker");
    const bool ok=w.assign(index,sim::Skill(s));const auto&l=w.lemmings()[index];
    skills<<sim::skillName(sim::Skill(s))<<','<<loaded->skills[s]<<','<<ok<<','<<sim::stateName(l.state)<<','<<(l.canClimb||l.canFloat)<<','<<l.fuseMs<<'\n';
#ifdef ANTIGRAV_SKILL_BEFORE
    check(ok==(s!=3),"entry accepts every other inverted skill but refuses Blocker");
#else
    check(ok,"all eight inverted skills enter with stock");
#endif
  }
}
void normalTrace(const AssetLibrary& lib,const fs::path& out) {
  auto loaded=sim::loadRetailLevel(lib,"MM_CLIMB_002");check(bool(loaded),"load unchanged upright control level");if(!loaded)return;
  std::ofstream trace(out/"normal-gravity.txt");
  for(int s=0;s<8;++s) {
    auto setup=*loaded;setup.skills[s]=4;sim::World w(setup);
    for(int n=0;n<1000;++n) {w.advanceElapsedMs(20);if(!w.lemmings().empty()&&w.lemmings()[0].state==sim::State::Walker)break;}
    check(!w.lemmings().empty() && w.lemmings()[0].verticalDir==1,"natural upright control actor");
    trace<<"assign,"<<s<<','<<w.assign(0,sim::Skill(s))<<'\n';
    for(int n=0;n<100;++n) {
      w.tick();trace<<n<<','<<w.saved()<<','<<w.dead()<<','<<w.terrain().version()<<','<<w.terrain().blockerFieldCells();
      for(int stock:w.setup().skills)trace<<','<<stock;
      for(const auto&l:w.lemmings())trace<<';'<<l.x<<','<<l.y<<','<<l.verticalDir<<','<<int(l.facing)<<','<<int(l.state)<<','<<l.frame<<','<<l.fallDistance<<','<<l.fuseMs<<','<<l.canClimb<<','<<l.canFloat<<','<<l.bricksLeft<<','<<l.bashCycles;
      // Compare actual edited geometry as well as state/stock, across every skill.
      std::uint64_t h=14695981039346656037ull;
      for(int y=0;y<w.terrain().height();++y)for(int x=0;x<w.terrain().width();++x){h^=unsigned(w.terrain().at(x,y));h*=1099511628211ull;}
      trace<<'|'<<h<<'\n';
    }
  }
}
#ifndef ANTIGRAV_SKILL_BEFORE
void blockerRegressions() {
  for (int v:{1,-1}) {
    sim::LevelSetup setup;setup.terrain=sim::Terrain(60,60);setup.totalLemmings=0;setup.skills.fill(4);
    setup.terrain.fillBox(0,v==1?31:29,60,1,sim::Material::Earth);
    sim::Lemming l;l.x=30;l.y=30;l.verticalDir=v;l.state=sim::State::Walker;
    sim::World w(setup);w.place(l);
    check(w.assign(0,sim::Skill::Blocker),"supported Blocker enters for either gravity");
    check(w.terrain().blockerFieldCells()==std::size_t(w.rules().blockerFieldHeight),"exact four-cell field");
    for(int k=0;k<w.rules().blockerFieldHeight;++k) {
      check(w.terrain().blockerField(30,30-k*v),"field extends away from support");
      if(k)check(!w.terrain().blockerField(30,30+k*v),"field does not point into support");
    }
    w.tick();check(w.lemmings()[0].state==sim::State::Blocker,"blocker remains supported after update");
    check(w.assign(0,sim::Skill::Builder),"existing supported reassignment behavior preserved");
    check(w.terrain().blockerFieldCells()==0,"leaving inverted or upright blocker erases entire field");
    check(w.terrain().solid(30,30+v),"field cleanup preserves support material");
    // After a Digger removes a Blocker's support, the Blocker's next step clears its field and falls.
    sim::World lost(setup);lost.place(l);check(lost.assign(0,sim::Skill::Blocker),"second blocker enters");
    auto digger=l;digger.x=31;lost.place(digger);check(lost.assign(1,sim::Skill::Digger),"neighbour digger enters");
    for(int i=0;i<40 && lost.lemmings()[0].state==sim::State::Blocker;++i)lost.tick();
    check(lost.lemmings()[0].state!=sim::State::Blocker && lost.terrain().blockerFieldCells()==0,"loss of ceiling/floor support clears field");
    setup.terrain=sim::Terrain(60,60);sim::World air(setup);air.place(l);
    check(!air.assign(0,sim::Skill::Blocker) && air.setup().skills[3]==4,"airborne Blocker refusal preserves stock");
  }
}
void winningRoute(const AssetLibrary& lib,const fs::path& out) {
  auto setup=sim::loadRetailLevel(lib,"m06_kf_1");if(!setup)return;
  sim::World w(*setup);app::RuntimeSimState runtime;
  std::ofstream trace(out/"winning-route.csv");trace<<"event,ms,index,x,y,gravity,facing,state,saved,dead\n";
  std::array<bool,7> done{};
  std::uint64_t targetMs=0;
  // Use connected play's ordinary completion adapter, including the timeout
  // latch. Reaching World::won after the deadline cannot establish a win.
  for(int ms=0;ms<600000 && !runtime.endTransition().snapshot();ms+=20) {
    runtime.advanceElapsed(w,0.02,true);
    if(w.won() && !targetMs) {targetMs=w.levelClockMs();check(!w.timedOut(),"save target reached before deadline");record(trace,w,"target-reached");}
    for(std::size_t i=0;i<w.lemmings().size();++i) {
      const auto& l=w.lemmings()[i];if(l.state!=sim::State::Walker)continue;
      int job=-1;
      if(!done[0] && l.x==91 && l.y==63 && l.verticalDir==-1)job=0;
      if(!done[1] && l.x==96 && l.y==27 && l.verticalDir==1)job=1;
      if(!done[2] && l.x==65 && l.y==39 && l.verticalDir==1)job=2;
      if(!done[3] && l.x==64 && l.y==46 && l.verticalDir==-1)job=3;
      if(!done[4] && l.x==91 && l.y==33 && l.verticalDir==-1)job=4;
      if(!done[5] && l.x==91 && l.y==56 && l.verticalDir==1)job=5;
      if(!done[6] && l.x==64 && l.y==70 && l.verticalDir==1)job=6;
      if(job<0)continue;
      done[job]=w.assign(i,sim::Skill::Blocker);check(done[job],"scripted real-level Blocker accepted");record(trace,w,"assign-"+std::to_string(job));
    }
    if(ms%1000==0)record(trace,w,"sample");
  }
  record(trace,w,"final");
  const auto& result=runtime.endTransition().snapshot();
  std::ofstream summary(out/"winning-summary.txt");summary<<"success_snapshot="<<(result && result->success)<<"\ntarget_ms="<<targetMs<<"\nresult_ms="<<w.levelClockMs()<<"\nspawned="<<w.spawned()<<"\nsaved="<<w.saved()<<"\nrequired="<<w.setup().saveTarget<<"\ndead="<<w.dead()<<"\ntotal="<<w.setup().totalLemmings<<"\nblockers=";for(bool b:done)summary<<b;summary<<'\n';
  check(std::all_of(done.begin(),done.end(),[](bool b){return b;}),"all seven real route control points reached");
  check(result && result->success && targetMs>0,"installed level publishes a successful completion with inverted Blockers and unmodified stock/terrain/release");
}
#endif
}
int main(int argc,char** argv) {
  if(argc!=2 || !std::getenv("WEBOLUTION_GAME_DIR"))return 2;
  fs::path game(std::getenv("WEBOLUTION_GAME_DIR")),out(argv[1]);fs::create_directories(out);
  auto local=save::localAppDataDirectory();if(!local)return 2;
  Preserved installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;
  audit(*lib,out);normalTrace(*lib,out);application(game,out);
#ifndef ANTIGRAV_SKILL_BEFORE
  blockerRegressions();winningRoute(*lib,out);
#endif
  check(installed.unchanged()&&normal.unchanged(),"installed and normal saves untouched");
  std::ofstream(out/"summary.txt")<<"failures="<<failures<<"\ninstalled_save_unchanged="<<installed.unchanged()<<"\nnormal_save_unchanged="<<normal.unchanged()<<'\n';
  std::cout<<"antigrav-skill evidence "<<out<<" failures="<<failures<<'\n';return failures?1:0;
}
