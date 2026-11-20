// Exercise Nuke on installed levels with SDL pointer input and private saves.
#include "app/AppEntry.h"
#include "app/RuntimeSkillPanel.h"
#include "app/RuntimeHud.h"
#include "assets/Png.h"
#include "assets/Box.h"
#include "sim/RetailLevel.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <map>

using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool ok, const char* what) { if (!ok) { ++failures; std::cerr << "FAIL " << what << '\n'; } }
void key(SDL_Scancode code) { SDL_Event e{}; e.type=SDL_KEYDOWN; e.key.state=SDL_PRESSED;
  e.key.keysym.scancode=code; e.key.keysym.sym=SDL_GetKeyFromScancode(code); SDL_PushEvent(&e); }
void button(int x, int y, bool down, unsigned timestamp=0) {
  SDL_Event e{}; e.type=down?SDL_MOUSEBUTTONDOWN:SDL_MOUSEBUTTONUP;
  e.button.button=SDL_BUTTON_LEFT; e.button.state=down?SDL_PRESSED:SDL_RELEASED;
  e.button.x=x; e.button.y=y; e.button.timestamp=timestamp; SDL_PushEvent(&e);
}
void click(int x,int y) { button(x,y,true); button(x,y,false); }
void quit() { SDL_Event e{}; e.type=SDL_QUIT; SDL_PushEvent(&e); }
std::vector<char> bytes(const fs::path& p) { std::ifstream f(p,std::ios::binary); return {std::istreambuf_iterator<char>(f),{}}; }
struct Preserved {
  fs::path path; bool existed; std::vector<char> data;
  explicit Preserved(fs::path p):path(std::move(p)),existed(fs::exists(path)),data(bytes(path)){}
  void verify() const { check(fs::exists(path)==existed && bytes(path)==data,"preserved save bytes/existence"); }
};
void shot(const fs::path& p,const Image& img) { check(writePng(p,img).has_value(),"write clone PNG"); }

sim::LevelSetup flat() {
  sim::LevelSetup s; s.terrain=sim::Terrain(200,100); s.totalLemmings=0;
  for(int y=80;y<100;++y)for(int x=0;x<200;++x)s.terrain.set(x,y,sim::Material::Earth);
  return s;
}
void units(const fs::path& game, const fs::path& out) {
  auto s=flat(); s.totalLemmings=10;s.entrances.push_back({10,79});
  sim::World w(s);app::RuntimeNukeInput input;
  input.press(50,388); input.release(50,388,1000,w);
  check(!w.nukeRequested() && input.armed(),"single click only arms confirmation");
  input.press(50,388); input.release(50,388,1500,w);
  check(!w.nukeRequested(),"500 ms excluded");
  input.press(50,388); input.release(50,388,1999,w);
  check(w.nukeRequested() && w.setup().nukeStock==0 && w.remainingToRelease()==0,"499 ms accepted, stock consumed, hatches cancelled");
  w.nuke(); check(w.nukeCountdownMs()==5000,"repeat cannot restart countdown");
  for(int i=0;i<100;++i)w.tick(true);
  check(w.spawned()==0 && w.finished(),"pre-hatch nuke releases none and finishes");
  auto blocked=flat(); blocked.nukeStock=0;sim::World noStock(blocked);noStock.nuke();
  check(!noStock.nukeRequested(),"zero resource stock rejects command");
  sim::World edges(flat());app::RuntimeNukeInput edge;
  edge.release(50,388,10,edges); check(!edge.armed(),"release without press inert");
  edge.press(50,388);edge.release(50,388,0xfffffff0u,edges);
  edge.press(50,388);edge.release(50,388,20,edges);
  check(edges.nukeRequested(),"release interval handles timestamp wrap");
  sim::World cancelled(flat());app::RuntimeNukeInput cancel;
  cancel.press(50,388);cancel.release(50,388,1000,cancelled);
  cancel.press(5,5);cancel.release(50,388,1100,cancelled);
  check(!cancel.armed()&&!cancelled.nukeRequested(),"off-plate press cancels confirmation");
  cancel.press(50,388);cancel.release(50,388,1200,cancelled);cancel.reset();
  cancel.press(50,388);cancel.release(50,388,1300,cancelled);
  check(!cancelled.nukeRequested(),"focus/menu reset requires new click pair");

  // Supplemental state/cleanup checks, not an installed gameplay route.
  auto terrain=flat();terrain.skills.fill(10);sim::World mixed(terrain);
  for(int x:{20,60,100,140}){sim::Lemming l;l.x=x;l.y=79;l.state=sim::State::Walker;mixed.place(l);}
  check(mixed.assign(0,sim::Skill::Blocker),"place real blocker field through skill entry");
  check(mixed.terrain().blockerFieldCells()>0,"blocker stamped field");
  check(mixed.assign(1,sim::Skill::Bomber),"ordinary Bomber coexists");
  const auto stock=mixed.setup().skills;
  mixed.nuke();app::RuntimeSimState runtime;runtime.togglePause();
  runtime.advanceElapsed(mixed,10,true,{},0);
  check(mixed.nukeCountdownMs()==5000 && mixed.dead()==0,"pause freezes nuke countdown");
  runtime.resume();std::set<int> deathTimes;
  int prior=0;
  for(int t=0;t<100;++t){mixed.tick();if(mixed.dead()>prior)deathTimes.insert(t);prior=mixed.dead();}
  check(mixed.dead()==4 && mixed.saved()==0 && mixed.finished(),"all retained actors die once");
  check(deathTimes.size()>=3,"sequential deaths, including pre-lit Bomber");
  check(mixed.terrain().blockerFieldCells()==0,"primary replacement detaches blocker");
  check(mixed.setup().skills==stock,"nuke consumes no actor skill stock");

  sim::World segmented(flat());segmented.advanceElapsedMs(73);segmented.nuke();
  segmented.advanceElapsedMs(4999);
  check(segmented.nukeCountdownMs()==1,"global countdown spends exact shared segment time");
  segmented.advanceElapsedMs(1);
  check(segmented.nukeCountdownMs()==0,"five-second countdown on a non-tick activation phase");
  sim::World fresh(flat());check(!fresh.nukeRequested()&&fresh.setup().nukeStock==1,"fresh level resets nuke state and stock");

  sim::World splat(flat());sim::Lemming standing;standing.x=20;standing.y=79;standing.state=sim::State::Walker;
  splat.place(standing);splat.nuke();for(int i=0;i<24;++i)splat.tick();
  sim::Lemming falling;falling.x=140;falling.y=76;falling.fallDistance=100;splat.place(falling);
  splat.tick();check(splat.dead()==1&&splat.lemmings()[1].state==sim::State::Splatter,"natural splat during nuke sequence counts loss at entry");
  for(int i=0;i<60;++i)splat.tick();
  check(splat.dead()==2&&splat.finished(),"nuke skips already counted splat, no duplicate death");

  auto rats=flat();rats.timeLimitTicks=1;sim::World longWave(rats);
  for(int i=0;i<40;++i){sim::Lemming rat;rat.x=(i*4)%180;rat.y=79;rat.state=sim::State::Walker;rat.sheetType=3;longWave.place(rat);}
  longWave.nuke();app::RuntimeSimState longRuntime;
  for(int i=0;i<200&&longRuntime.endTransition().phase()!=app::LevelEndPhase::Results;++i){
    longRuntime.advanceElapsed(longWave,0.2,true,{},0);
    if(longRuntime.endTransition().phase()==app::LevelEndPhase::Results)
      check(longWave.dead()==40,"timeout and type-3 population cannot truncate nuke wave");
  }
  check(longWave.dead()==40&&longRuntime.endTransition().snapshot().has_value(),"long wave removes all actors and reaches results");

  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");check(bool(lib),"open installed archive");
  if(lib){auto retail=sim::loadRetailLevel(*lib,"MM_CLIMB_002");check(retail&&retail->nukeStock==1,"installed command stock loaded from ordered tail");
    if(retail)std::ofstream(out/"installed-stock.txt")<<"MM_CLIMB_002 nukeStock="<<retail->nukeStock<<" total="<<retail->totalLemmings<<'\n';}
}

int route(const std::string& mode,const fs::path& out,const fs::path& game) {
#ifdef NUKE_BEFORE
  constexpr bool before=true;
#else
  constexpr bool before=false;
#endif
  const bool partial=mode=="partial", empty=mode=="empty";
  const auto local=save::localAppDataDirectory();check(bool(local),"resolve normal save for preservation check");if(!local)return 1;
  Preserved normal(save::cloneSavePathUnder(*local)), installed(game/"SavedGame.dat");
  const auto savePath=out/"private"/(std::to_string(SDL_GetPerformanceCounter())+"-SavedGame.dat");
  app::ApplicationRunOptions options;options.frameSeconds=0.2;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(savePath,game));
  int stage=0, firstClick=-1, activated=-1, spawnAt=-1, savedAt=-1, lastDead=0;
  bool result=false, assigned=false, crater=false, allDead=false, singleInert=false;
  std::vector<std::size_t> liveAt;std::map<std::size_t,std::uint64_t> explosions;
  std::ofstream trace(out/"trace.csv");trace<<"frame,ms,spawned,saved,dead,unreleased,nuke,countdown,end,terrain_changed\n";
  std::ofstream actors(out/"actors.csv");
  const auto front=[&](const app::FrontendSession& s,int){
    if(stage==0){check(s.screen()==shell::Screen::Menu,"fresh menu");key(SDL_SCANCODE_RETURN);stage=1;}
    else if(stage==1){check(s.screen()==shell::Screen::Confirm,"new game confirmation");key(SDL_SCANCODE_RETURN);stage=2;}
    else if(stage==2){check(s.screen()==shell::Screen::Map,"fresh map");click(206,204);stage=3;}
    else if(stage==3){
      check(s.screen()==shell::Screen::Results && s.outcome().has_value(),"production results screen");
      if(s.outcome()){
        check(!before,"baseline must not finish from inert nuke input");
        const bool success=savedAt>=s.outcome()->required;
        check(s.outcome()->saved==savedAt && s.outcome()->total==10 && s.outcome()->success==success,"correct saved/total/pass predicate");
        check(s.completionFlags()[0]==int(success),"completion flag matches actual saved target");
        check(s.saveCount()==1+int(success),"only New Game and a genuine target-reaching result save progress");
        std::ofstream(out/"result.txt")<<"saved="<<s.outcome()->saved<<" total="<<s.outcome()->total
          <<" lost="<<s.outcome()->total-s.outcome()->saved<<" spawned="<<spawnAt<<" success="<<s.outcome()->success<<'\n';
      }
      shot(out/"results.png",s.composeFrame());result=true;quit();
    }
  };
  const auto observe=[&](const app::LevelFrameView& f){
    const auto& w=f.world;
    for(const auto& a:f.actors){const auto& l=w.lemmings()[a.index];
      actors<<f.frame<<','<<w.levelClockMs()<<','<<a.index<<','<<sim::stateName(l.state)<<','<<l.nukeExploding<<','<<l.frame<<','
        <<l.x<<','<<l.y<<','<<a.clip<<','<<a.temporalIndex<<','<<a.placement.left<<','<<a.placement.top<<','<<a.placement.right<<','<<a.placement.bottom<<std::endl;
    }
    trace<<f.frame<<','<<w.levelClockMs()<<','<<w.spawned()<<','<<w.saved()<<','<<w.dead()<<','<<w.remainingToRelease()<<','
      <<w.nukeRequested()<<','<<w.nukeCountdownMs()<<','<<int(f.endPhase)<<','<<f.drawnTerrainCellsChanged<<'\n';
    if(partial && !assigned){
      for(const auto& a:f.actors){const auto& l=w.lemmings()[a.index];if(l.state!=sim::State::Walker||l.canClimb)continue;
        const int x=int((a.placement.left+a.placement.right)/2), y=int((a.placement.top+a.placement.bottom)/2);
        if(x<100||x>=630||y<30||y>=450)continue;
        click(x,y);assigned=true;break;}
    }
    if(firstClick<0 && (empty || (partial ? w.saved()>=1 : w.activeCount()>=5))){
      firstClick=f.frame;spawnAt=w.spawned();savedAt=w.saved();shot(out/"before-click.png",f.image);click(50,388);return;
    }
    if(firstClick>=0 && f.frame==firstClick+1){
      check(!w.nukeRequested(),"one pointer click is inert");singleInert=true;shot(out/"confirmation.png",f.image);
      click(50,388);return;
    }
    if(firstClick>=0 && f.frame==firstClick+2){
      check(w.nukeRequested()!=before,"second pointer release activates only fixed input");
      activated=f.frame;spawnAt=w.spawned();savedAt=w.saved();
      for(std::size_t i=0;i<w.lemmings().size();++i)if(w.lemmings()[i].alive())liveAt.push_back(i);
      shot(out/"activated.png",f.image);
    }
    if(activated>=0){
      if(before){if(f.frame>=activated+18){check(!w.nukeRequested()&&w.setup().nukeStock==1,"authentic old input remains inert");shot(out/"still-inert.png",f.image);quit();}return;}
      check(w.spawned()==spawnAt && w.remainingToRelease()==0,"no release after activation");
      check(w.setup().nukeStock==0,"command stock spent once");
      for(std::size_t i=0;i<w.lemmings().size();++i){const auto& l=w.lemmings()[i];
        if(l.nukeExploding&&l.frame==3&&i<4)shot(out/("exploding-mid-"+std::to_string(i)+".png"),f.image);
        if(l.nukeExploding&&!explosions.contains(i)){
          explosions[i]=w.levelClockMs();
          if(explosions.size()<=4)shot(out/("exploder-"+std::to_string(i)+".png"),f.image);
        }
      }
      if(w.dead()>lastDead){shot(out/("crater-"+std::to_string(w.dead())+".png"),f.image);lastDead=w.dead();}
      crater|=f.drawnTerrainCellsChanged>0;
      if(w.finished()&&!allDead){
        allDead=true;shot(out/"cleared.png",f.image);
        check(w.activeCount()==0 && w.saved()==savedAt && w.dead()==int(liveAt.size()),"every live actor exploded; saved count preserved");
        check(explosions.size()==liveAt.size(),"all activation survivors entered explosion state");
        std::uint64_t previous=0;
        for(auto [id,time]:explosions){check(time>previous,"oldest-first distinct explosion ticks");previous=time;}
      }
    }
    if(f.frame>=550){check(false,"route watchdog");quit();}
  };
  std::vector<std::string> args{"nuke-test","--frontend","--frames","560","--native","--no-audio","--game-dir",game.string()};
  std::vector<char*> raw;for(auto& a:args)raw.push_back(a.data());
  check(app::runApplication(int(raw.size()),raw.data(),front,observe,options)==0,"application returns success");
  check(singleInert && activated>=0,"complete production click pair exercised");
  if(!before){check(result&&allDead,"natural completion reaches results");if(!empty)check(crater,"production terrain mesh reflects craters");if(partial)check(savedAt>0,"partial-rescue route saved a real lemming");}
  normal.verify();installed.verify();
  std::ofstream(out/"summary.txt")<<"before="<<before<<" mode="<<mode<<" spawned_at_nuke="<<spawnAt<<" saved="<<savedAt
    <<" exploders="<<explosions.size()<<" results="<<result<<" craters="<<crater<<" failures="<<failures<<'\n';
  return failures?1:0;
}
}
int main(int argc,char** argv){
  if(argc!=3||!std::getenv("WEBOLUTION_GAME_DIR"))return 2;
  fs::path out(argv[2]),game(std::getenv("WEBOLUTION_GAME_DIR"));fs::create_directories(out);
  if(std::string(argv[1])=="units"){units(game,out);return failures?1:0;}
  if(!std::getenv("SDL_VIDEODRIVER")||std::string(std::getenv("SDL_VIDEODRIVER"))!="dummy")return 2;
  return route(argv[1],out,game);
}
