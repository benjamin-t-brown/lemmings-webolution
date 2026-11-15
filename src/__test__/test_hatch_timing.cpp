// Owned-process clone observations through the ordinary frontend. No original run.
#include "app/AppEntry.h"
#include "app/RuntimeHud.h"
#include "app/LevelStartSequence.h"
#include "app/RuntimePick.h"
#include "app/HatchAnimation.h"
#include "app/BackdropScene.h"
#include "app/Raster.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#include "lemmings/compiled_level.hpp"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
using namespace webo;
namespace fs = std::filesystem;
namespace {
#ifdef HATCH_TIMING_BEFORE
constexpr bool before = true;
#else
constexpr bool before = false;
#endif
#ifdef EMPTY_RECEIVER_BASELINE
constexpr bool receiverBaseline = true;
#else
constexpr bool receiverBaseline = false;
#endif
int failures = 0;
void check(bool ok, const std::string& message) {
  if (!ok && ++failures < 25) std::cerr << "FAIL: " << message << '\n';
}
std::string bytes(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}
struct Snapshot {
  fs::path path;
  bool exists;
  std::string content;
  explicit Snapshot(fs::path p) : path(std::move(p)), exists(fs::exists(path)), content(bytes(path)) {}
  bool unchanged() const { return exists == fs::exists(path) && content == bytes(path); }
};
void key(SDL_Scancode code) {
  SDL_Event e{}; e.type = SDL_KEYDOWN;
  e.key.keysym.scancode = code; e.key.keysym.sym = SDL_GetKeyFromScancode(code);
  check(SDL_PushEvent(&e) == 1, "queue key");
}
void click(int x, int y) {
  SDL_Event e{}; e.type = SDL_MOUSEBUTTONDOWN; e.button.button = SDL_BUTTON_LEFT;
  e.button.x = x; e.button.y = y;
  check(SDL_PushEvent(&e) == 1, "queue click");
  e.type = SDL_MOUSEBUTTONUP; check(SDL_PushEvent(&e) == 1, "queue release");
}
void quit() { SDL_Event e{}; e.type = SDL_QUIT; check(SDL_PushEvent(&e) == 1, "queue quit"); }
void png(const fs::path& p, const Image& im) { check(writePng(p, im).has_value(), "write image"); }
AssetLibrary library(const fs::path& game) {
  auto lib = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!lib) throw std::runtime_error(lib.error());
  return std::move(*lib);
}
void seed(const fs::path& path) {
  // An explicit private fixture save, never imported from either real save.
  save::SavedGame s;
  for (auto& row : s.records) row.c = 1;
  const auto data = s.serialize();
  std::ofstream out(path, std::ios::binary);
  out.write(reinterpret_cast<const char*>(data.data()), data.size());
}
int entry(const fs::path& out, bool fast) {
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  const auto local = save::localAppDataDirectory();
  if (!local) return 2;
  Snapshot installed(game / "SavedGame.dat"), normal(save::cloneSavePathUnder(*local));
  auto lib = library(game);
  std::array<int32_t,102> flags{}; flags.fill(1);
  auto map = app::MapScene::build(lib, flags);
  if (!map) return 2;
  for (std::size_t node : {0u, 2u}) {
    const auto id = map->levelForNode(node);
    check(bool(id), "installed map node exists");
    if (!id) continue;
    const fs::path dir = out / *id;
    fs::create_directories(dir);
    seed(dir / "SavedGame.dat");
    Snapshot privateSave(dir / "SavedGame.dat");
    int px = -1, py = -1;
    for (int y = 0; y < 480 && px < 0; ++y)
      for (int x = 0; x < 640; ++x)
        if (map->pickNode(x,y) == node) { px=x; py=y; break; }
    check(px >= 0, "public map target found");
    double elapsed = 0, first = -1, cueEnd = -1, doorStart = -1, doorEnd = -1;
    double priorOpening = -1;
    int spawned = 0, paused = 0, frameCount = 0;
    bool fastObserved = false;
    std::vector<unsigned long long> releases;
    std::ofstream trace(dir / "frames.csv");
    trace << "frame,presentation_seconds,level_ms,cue_seconds,cue_visible,opening,spawned,paused\n";
    trace << std::setprecision(17);
    auto front = [&](const app::FrontendSession& s, int) {
      if (s.screen() == shell::Screen::Menu) { key(SDL_SCANCODE_DOWN); key(SDL_SCANCODE_RETURN); }
      else if (s.screen() == shell::Screen::Map) click(px,py);
      else { check(false, "entry stays on expected frontend route"); quit(); }
    };
    auto frame = [&](const app::LevelFrameView& f) {
      ++frameCount;
      if (!f.paused) elapsed += 0.1;
      check(f.levelName == *id, "selected installed level matches");
      fastObserved |= f.world.levelClockMs() > elapsed*1000.0 + 500.0;
      // The frozen task-entry main samples this exact expression. Current main
      // publishes the value it actually passed to BackdropScene.
      const double opening = before ? std::min(1.0, f.world.levelClockMs()/1000.0/
          f.world.setup().releaseArmSeconds) : f.hatchOpeningProgress;
      trace << f.frame << ',' << elapsed << ',' << f.world.levelClockMs() << ','
            << f.entryCueSeconds << ',' << bool(f.entryCueFrame) << ',' << opening
            << ',' << f.world.spawned() << ',' << f.paused << '\n';
      bool shot = f.frame == 0 || (f.frame % 5 == 0 && elapsed < 7.0);
      if (doorStart < 0 && opening > 0) { doorStart=elapsed; shot=true; }
      if (doorEnd < 0 && opening >= 1.0) { doorEnd=elapsed; shot=true; }
      if (cueEnd < 0 && f.entryCueSeconds < -1.0) { cueEnd=elapsed; shot=true; }
      if (!before) {
        if (f.entryCueSeconds >= -1.0 || opening < 1.0)
          check(f.world.spawned() == 0, "no birth until cue retired AND doors open");
        check(opening + 1e-9 >= priorOpening, "door sample monotonic");
        if (f.paused) check(opening == priorOpening, "pause freezes doors");
      }
      priorOpening = opening;
      if (f.world.spawned() > spawned) {
        check(f.world.spawned() == spawned + 1, "one release per observed frame");
        releases.push_back(f.world.levelClockMs()); spawned=f.world.spawned();
        if (first < 0) { first=elapsed; png(dir / "first-release.png", f.image); }
        if (!before) check(cueEnd >= 0 && doorEnd >= 0 && first > doorEnd,
                           "first release after cue and fully open doors");
        shot = true;
      }
      if (shot) png(dir / ("frame-" + std::to_string(f.frame) + ".png"), f.image);
      if (f.frame == 8) key(SDL_SCANCODE_P);
      if (f.paused && ++paused == 4) key(SDL_SCANCODE_P);
      if (fast && (f.frame == 1 || f.frame == 35)) click(50,348); // public FF plate
      if ((!fast && spawned == f.world.setup().totalLemmings) || (fast && elapsed > 8.0)) quit();
    };
    std::vector<std::string> args{"hatch-timing", "--frontend", "--native", "--frames", "900", "--game-dir", game.string()};
    std::vector<char*> argv; for (auto& a : args) argv.push_back(a.data());
    app::ApplicationRunOptions options; options.frameSeconds=0.1;
    options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(dir / "SavedGame.dat",game));
    check(app::runApplication(int(argv.size()),argv.data(),front,frame,options) == 0, "ordinary entry exits");
    check(first > 0 && cueEnd > 0 && doorEnd > 0 && paused == 4, "entry measurement completed");
    if (fast) check(fastObserved, "public fast-forward gesture exercised accelerated simulation");
    if (!before) check(std::abs(doorEnd-2.8) < 0.11, "full door duration is 2.8 s, within frame resolution");
    if (!fast) {
      check(spawned > 2, "recurring release measured");
      // Compare every recurring gap exactly at this matched owned partition.
      // A clean checkout still asserts production order/count/cadence; paired
      // historical replay is available when its authentic frozen source exists.
      const auto baselineReleases=out.parent_path()/"before"/ *id / "releases.txt";
      if (!before && fs::exists(baselineReleases)) {
        std::ifstream in(baselineReleases);
        std::vector<unsigned long long> old; unsigned long long n;
        while(in >> n) old.push_back(n);
        check(old.size() == releases.size(), "release count preserved");
        for(std::size_t i=1;i<std::min(old.size(),releases.size());++i)
          check(std::llabs(static_cast<long long>(releases[i]-releases[i-1])-
                          static_cast<long long>(old[i]-old[i-1])) == 0,
                "every recurring release interval exactly unchanged");
      }
    }
    std::ofstream releaseOut(dir / "releases.txt"); for(auto n:releases) releaseOut<<n<<'\n';
    check(privateSave.unchanged(), "entry does not write its private save");
    std::ofstream summary(dir / "summary.txt");
    summary << std::setprecision(10) << "node=" << node << "\nlevel=" << *id << "\ndoor_start_first_sample="
      << doorStart << "\ndoor_end=" << doorEnd << "\ncue_end=" << cueEnd << "\nfirst_release=" << first
      << "\nrelease_count=" << spawned << "\npaused_frames=" << paused << "\nframes=" << frameCount
      << "\nprivate_save_unchanged=" << privateSave.unchanged() << "\nfailures=" << failures << '\n';
  }
  check(installed.unchanged() && normal.unchanged(), "installed and normal saves untouched");
  std::ofstream(out / "saves.txt") << "installed=" << installed.unchanged() << "\nnormal=" << normal.unchanged() << '\n';
  return failures ? 1 : 0;
}
int assets(const fs::path& out) {
  auto lib=library(std::getenv("WEBOLUTION_GAME_DIR"));
  std::ofstream census(out/"door-keys.csv");
  census << std::setprecision(17) << "level,clip,track,keys,duration,max_40ms_key_error\n";
  int levels=0,hatches=0;
  for(const auto& name:levelFileNames(lib)) {
    auto blob=lib.read(name); if(!blob) return 2;
    const auto level=lemmings::read_compiled_level({reinterpret_cast<const uint8_t*>(blob->data()),blob->size()});
    ++levels;
    for(const auto& e:level.gadgets.entities_425e50) {
      check(e.model && e.model->object,"installed hatch has inline model");
      if(!e.model || !e.model->object) continue;
      bool found=false;
      for(const auto& aux:e.model->object->auxiliary_records) {
        if(aux.name!="entrance" && aux.name!="entrance_acid") continue;
        auto a=app::readHatchAnimation(aux.serialized_bytes);
        check(bool(a),"installed animation decodes"); if(!a) continue;
        found=true; ++hatches;
        check(a->tracks.size()==2 && std::abs(a->duration-2.8)<1e-6,"2.8 s installed door pair");
        for(const auto& t:a->tracks) {
          double error=0;
          float accumulated=0.0f;
          for(std::size_t i=0;i<t.keys.size();++i) {
            error=std::max(error,std::abs(t.keys[i].end-0.04*(i+1)));
            // Stored doubles retain accumulated float 40 ms times, with the
            // last key snapped to the float clip duration. Check those exact
            // values rather than imposing a false uniform-double grid.
            accumulated+=0.04f;
            const double expected=i+1==t.keys.size() ? a->duration : double(accumulated);
            check(t.keys[i].end==expected,"serialized 40 ms float accumulation and final endpoint");
          }
          check(t.keys.size()==70,"70 door keys");
          census<<name<<','<<aux.name<<','<<t.node<<','<<t.keys.size()<<','<<a->duration<<','<<error<<'\n';
        }
      }
      check(found,"installed hatch clip found");
    }
  }
  check(levels==102 && hatches==144,"complete installed hatch census");
  // Supplemental model-only views expose the door poses which the ordinary
  // camera intro/title can obscure. These do not replace application frames.
  auto resolved=sim::resolveRetailLevel(lib,"MM_CLIMB_002"); if(!resolved) return 2;
  auto blob=lib.read(resolved->file);
  if(!blob) return 2;
  auto scene=app::BackdropScene::build(lib,*blob,"MM_CLIMB_002");
  if(!scene) return 2;
  auto presentation=scene->entranceExitPresentation(*blob); if(!presentation) return 2;
  for(double seconds:{0.0,0.4,0.8,1.4,2.0,2.8}) {
    app::LevelStartSequence sample(2.799999952316284); sample.advance(seconds,true);
    scene->setEntranceExitPresentation(*presentation,sample.openingProgress(),0);
    app::Raster raster(640,480); raster.clear(0,0,0); app::BackdropStats stats;
    scene->renderStage(raster,app::buildCamera({}),lib,stats,app::BackdropScene::Stage::LegacyUnattributed);
    Image im; im.width=640;im.height=480;
    const auto& rgb=raster.framebuffer().rgb;
    for(std::size_t i=0;i<rgb.size();i+=3) im.rgba.insert(im.rgba.end(),{rgb[i],rgb[i+1],rgb[i+2],255});
    png(out/("model-seconds-"+std::to_string(seconds)+".png"),im);
  }
  std::ofstream(out/"summary.txt")<<"levels="<<levels<<"\nhatches="<<hatches<<"\nfailures="<<failures<<'\n';
  return failures ? 1:0;
}
int win(const fs::path& out) {
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  const auto local=save::localAppDataDirectory(); if(!local) return 2;
  Snapshot installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  bool won=false; int pending=0,frames=0,births=0,seconds=-1;
  std::ofstream trace(out/"route.csv"); trace<<"frame,ms,spawned,saved,dead,end_phase\n";
  auto front=[&](const app::FrontendSession& s,int) {
    if(s.screen()==shell::Screen::Menu || s.screen()==shell::Screen::Confirm) key(SDL_SCANCODE_RETURN);
    else if(s.screen()==shell::Screen::Map) click(206,204);
    else if(s.screen()==shell::Screen::Results) {
      check(pending>10 && s.outcome() && s.outcome()->success && s.outcome()->saved==10,
            "same natural Climber 10/10 win reaches Results");
      won=s.outcome() && s.outcome()->success;
      if(s.outcome()) seconds=s.outcome()->secondsRemaining;
      png(out/"results.png",s.composeFrame()); quit();
    }
  };
  auto frame=[&](const app::LevelFrameView& f) {
    ++frames; births=f.world.spawned();
    trace<<f.frame<<','<<f.world.levelClockMs()<<','<<births<<','<<f.world.saved()<<','<<f.world.dead()<<','<<int(f.endPhase)<<'\n';
    if(f.endPhase==app::LevelEndPhase::Pending && ++pending==1) png(out/"pending.png",f.image);
    for(const auto& a:f.actors) {
      const auto& l=f.world.lemmings()[a.index];
      if(!l.active() || l.canClimb || !app::firstLevelClimberSupport(l,sim::Skill::Climber,f.world.terrain()).value_or(false)) continue;
      const int x=int((a.placement.left+a.placement.right)/2),y=int((a.placement.top+a.placement.bottom)/2);
      if(x>=100 && x<640 && y>=0 && y<480) { click(x,y); break; }
    }
  };
  std::vector<std::string> args{"hatch-timing-win","--frontend","--native","--frames","1500","--game-dir",game.string()};
  std::vector<char*> argv;for(auto& a:args)argv.push_back(a.data());
  app::ApplicationRunOptions options;options.frameSeconds=0.2;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  check(app::runApplication(int(argv.size()),argv.data(),front,frame,options)==0,"natural win route exits");
  check(won && births==10,"route outcome and release count unchanged");
  check(installed.unchanged() && normal.unchanged(),"win preserves both real saves");
  std::ofstream(out/"summary.txt")<<"won="<<won<<"\nspawned="<<births<<"\nframes="<<frames
    <<"\nseconds_remaining="<<seconds<<"\ninstalled="<<installed.unchanged()<<"\nnormal="<<normal.unchanged()<<"\nfailures="<<failures<<'\n';
  return failures ? 1:0;
}
int retry(const fs::path& out) {
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  const auto local=save::localAppDataDirectory();if(!local)return 2;
  Snapshot installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  int stage=0; double cueEnd=-1,doorEnd=-1,first=-1;
  std::ofstream trace(out/"retry.csv");trace<<"frame,ms,cue,opening,spawned\n";
  auto front=[&](const app::FrontendSession& s,int) {
    if(s.screen()==shell::Screen::Menu || s.screen()==shell::Screen::Confirm) key(SDL_SCANCODE_RETURN);
    else if(s.screen()==shell::Screen::Map)click(206,204);
    else {check(false,"Retry stays in level");quit();}
  };
  auto frame=[&](const app::LevelFrameView& f) {
    if(stage==0) {key(SDL_SCANCODE_ESCAPE);stage=1;return;}
    if(stage==1) {check(f.levelMenuOpen && f.paused,"public menu opened");click(320,140);stage=2;return;}
    if(stage==2) {
      check(f.frame==0 && !f.cameraIntroActive && std::abs(f.entryCueSeconds-1.4)<1e-8,"fresh Retry has short cue and no camera intro");
      stage=3;
    }
    const double t=f.world.levelClockMs()/1000.0;
    trace<<f.frame<<','<<f.world.levelClockMs()<<','<<f.entryCueSeconds<<','<<f.hatchOpeningProgress<<','<<f.world.spawned()<<'\n';
    if(cueEnd<0 && f.entryCueSeconds < -1) {cueEnd=t;png(out/"cue-retired.png",f.image);}
    if(doorEnd<0 && f.hatchOpeningProgress>=1) {doorEnd=t;png(out/"doors-open.png",f.image);}
    if(f.entryCueSeconds>=-1 || f.hatchOpeningProgress<1) check(f.world.spawned()==0,"Retry holds release until both gates finish");
    if(f.world.spawned()>0) {first=t;png(out/"first-release.png",f.image);quit();}
  };
  std::vector<std::string> args{"hatch-timing-retry","--frontend","--native","--frames","100","--game-dir",game.string()};
  std::vector<char*> argv;for(auto& a:args)argv.push_back(a.data());
  app::ApplicationRunOptions options;options.frameSeconds=0.1;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  check(app::runApplication(int(argv.size()),argv.data(),front,frame,options)==0,"public Retry exits");
  check(first>doorEnd && doorEnd>cueEnd && cueEnd>0,"Retry waits for full doors after shorter cue");
  check(installed.unchanged() && normal.unchanged(),"Retry preserves real saves");
  std::ofstream(out/"summary.txt")<<"cue_end="<<cueEnd<<"\ndoor_end="<<doorEnd<<"\nfirst_release="<<first<<"\nfailures="<<failures<<'\n';
  return failures ? 1:0;
}
// Matched replay for the review's expansion WIN -> LOSS finding. The historical
// wall-clock driver did NOT make the same assignments: its winning Retry left
// blockers a few cells away from the losses. Reproduce both exact sets of
// placement choices with owned 200 ms frames and public input, then compare every
// post-birth actor/stock/gadget sample to the authentic pre-task application.
// The level clock still runs during the added wait, so compare only up to timeout
// and assert the final saved/count/outcome separately (no invented extra time).
std::string routeState(const app::LevelFrameView& f) {
  const auto& w=f.world;
  std::ostringstream s; s<<std::setprecision(17)<<w.spawned()<<','<<w.saved()<<','<<w.dead();
  for(int stock:w.setup().skills) s<<','<<stock;
  for(const auto& l:w.lemmings()) {
    s<<';'<<l.x<<','<<l.y<<','<<int(l.state)<<','<<int(l.facing)<<','
     <<l.fallDistance<<','<<l.frame<<','<<l.bricksLeft<<','<<l.bashCycles<<','
     <<l.fuseMs<<','<<l.fasterMs<<','<<l.speedPadLatched<<','<<l.canClimb<<','
     <<l.canFloat<<','<<l.verticalDir<<','<<l.sheetType<<','<<l.stepBudgetSeconds;
    for(auto p:l.teleportPadsLatched) s<<':'<<p;
  }
  for(const auto& l:w.setup().levers) s<<";lever,"<<l.state<<','<<l.cooldownRemaining;
  for(const auto& g:w.setup().gates) s<<";gate,"<<g.height<<','<<g.request<<','<<g.done<<','<<g.armed;
  return s.str();
}
int routes(const fs::path& out) {
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  const auto local=save::localAppDataDirectory(); if(!local) return 2;
  Snapshot installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  auto lib=library(game);
  // Only node 0 is a fixture prerequisite. The blocker win must earn node 8;
  // neither its flag nor the gadget's selectable state is injected.
  const auto saveFile=out/"SavedGame.dat";
  save::SavedGame fixture; fixture.records[0].c=1;
  const auto data=fixture.serialize();
  {std::ofstream s(saveFile,std::ios::binary);s.write(reinterpret_cast<const char*>(data.data()),data.size());}
  auto store=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(saveFile,game));
  struct Site { std::size_t actor; int x,y; sim::Facing face; };
  // Use actors-at-end, not state-blocker events: the historical observer resets
  // its previous-state vector at each birth and can miss valid transitions.
  const std::vector<Site> winning{{0,177,24,sim::Facing::Right},{1,82,45,sim::Facing::Left},
                               {2,176,65,sim::Facing::Right},{3,155,65,sim::Facing::Left}};
  const std::vector<Site> losing{{0,175,24,sim::Facing::Right},{1,82,45,sim::Facing::Left},
                               {2,175,65,sim::Facing::Right},
                               {3,153,65,sim::Facing::Left}};
  // Actual final placements in the pre-task expansion gadget route. Its third
  // click intended actor 11 but picked actor 12. Replay that resulting input
  // choice explicitly, rather than relying on a wall-clock picking race.
  const std::vector<Site> gadgetSites{{2,214,41,sim::Facing::Right},{3,163,41,sim::Facing::Left},
                                     {12,185,26,sim::Facing::Right}};
  for(int pass=0;pass<3;++pass) {
    const bool gadget=pass==1, miss=pass==2;
    const std::string label=gadget?"earned-gadget":miss?"losing-placements":"winning-placements";
    const std::size_t node=gadget?8:3;
    const std::string level=gadget?"MM_W_012":"MM_BLOCK_001_A";
    const auto dir=out/label; fs::create_directories(dir);
    const auto baselineDir=out.parent_path()/"routes-before"/label;
    const auto& sites=gadget?gadgetSites:miss?losing:winning;
    std::ofstream trace(dir/"frames.csv"),actions(dir/"actions.csv"),states(dir/"gameplay.txt"),losses(dir/"dry-receiver-losses.csv");
    losses << "age_frame,actor,x,y,fall_distance,receiver_id,height,request\n";
    std::set<std::size_t> drySplats, exitedActors;
    trace<<"frame,ms,age_frame,spawned,saved,dead,phase,stock\n";
    actions<<"age_frame,actor,x,y,click_x,click_y,stock\n";
    std::ifstream expected;
    const bool paired=!before && fs::exists(baselineDir/"gameplay.txt");
    if(paired) expected.open(baselineDir/"gameplay.txt");
    int leg=0,firstFrame=-1,age=-1,spawned=0,saved=0,clicks=0,confirmed=0,pending=0;
    int stage=0,ffAge=-1; bool results=false,won=false,mapEarned=false;
    unsigned long long firstMs=0; std::vector<unsigned long long> births;
    int remaining=-1; std::size_t next=0; int pendingActor=-1;
    auto front=[&](const app::FrontendSession& s,int) {
      if(s.screen()==shell::Screen::Menu) {key(SDL_SCANCODE_DOWN);key(SDL_SCANCODE_RETURN);}
      else if(s.screen()==shell::Screen::Map) {
        mapEarned |= s.completionFlags()[3]==1 && (s.mapStates()[8]==2 || s.mapStates()[8]==3);
        if(results) {png(dir/"earned-map.png",s.composeFrame());quit();return;}
        auto map=app::MapScene::build(lib,s.completionFlags());
        check(bool(map),"matched map builds");if(!map){quit();return;}
        check(s.mapStates()[node]==2 || s.mapStates()[node]==3,"matched target genuinely selectable");
        int px=-1,py=-1;
        for(int y=0;y<480 && px<0;++y)for(int x=0;x<640;++x)
          if(map->pickNode(x,y)==node){px=x;py=y;break;}
        check(px>=0,"matched public map pixel");if(px>=0)click(px,py);else quit();
      } else if(s.screen()==shell::Screen::Results) {
        results=true;
        check(s.outcome().has_value(),"matched results snapshot exists");
        if(s.outcome()) {
          won=s.outcome()->success; saved=s.outcome()->saved; spawned=s.outcome()->total;
          remaining=s.outcome()->secondsRemaining;
        }
        png(dir/"results.png",s.composeFrame());
        if(pass==0)click(320,360);else quit();
      } else {check(false,"matched expected frontend screen");quit();}
    };
    auto frame=[&](const app::LevelFrameView& f) {
      if(f.frame==0)++leg;
      check(f.levelName==level,"matched installed level");
      // Public Retry gives the same short-cue route as historical attempt 2.
      if(!gadget && stage==0){key(SDL_SCANCODE_ESCAPE);stage=1;return;}
      if(!gadget && stage==1){check(f.levelMenuOpen,"matched menu opened");click(320,140);stage=2;return;}
      if(stage<3){stage=3;key(SDL_SCANCODE_4);}
      if(f.world.spawned()>0 && firstFrame<0){firstFrame=f.frame;firstMs=f.world.levelClockMs();png(dir/"first-release.png",f.image);}
      age=firstFrame<0?-1:f.frame-firstFrame;
      if(f.world.spawned()>int(births.size())) {
        check(f.world.spawned()==int(births.size())+1,"matched births individually observed");
        births.push_back(f.world.levelClockMs());
      }
      trace<<f.frame<<','<<f.world.levelClockMs()<<','<<age<<','<<f.world.spawned()<<','
           <<f.world.saved()<<','<<f.world.dead()<<','<<int(f.endPhase)<<','<<f.world.setup().skills[3]<<'\n';
      if(gadget) {
        for(std::size_t i=0;i<f.world.lemmings().size();++i) {
          const auto& l=f.world.lemmings()[i];
          if(l.state==sim::State::Exited) exitedActors.insert(i);
          if(l.state!=sim::State::Splatter || drySplats.contains(i)) continue;
          for(const auto& g:f.world.setup().gates) {
            if(g.height==0 && l.x>=g.minX && l.x<g.maxX &&
               f.world.terrain().height()-1-l.y==g.minY &&
               f.world.terrain().solid(l.x,l.y+1)) {
              drySplats.insert(i);
              losses<<age<<','<<i<<','<<l.x<<','<<l.y<<','<<l.fallDistance<<','
                    <<g.id<<','<<g.height<<','<<g.request<<'\n';
              check(l.x==222 && l.y==91 && l.fallDistance==50,
                    "gadget's former survivors fall fifty cells to the actual dry receiver floor");
              png(dir/("dry-floor-splat-"+std::to_string(i)+".png"),f.image);
              break;
            }
          }
        }
      }
      if(firstFrame>=0 && !f.world.timedOut()) {
        const auto state=routeState(f);states<<state<<'\n';
        if(paired){std::string old;check(bool(std::getline(expected,old)) && old==state,
                                      label+" all post-birth gameplay state matches");}
      }
      if(pendingActor>=0) {
        const auto& l=f.world.lemmings()[pendingActor];
        check(l.state==sim::State::Blocker,"matched click leaves intended lasting Blocker");
        ++confirmed;png(dir/("blocker-"+std::to_string(pendingActor)+".png"),f.image);pendingActor=-1;
      }
      if(f.endPhase!=app::LevelEndPhase::Running){if(++pending==1)png(dir/"pending.png",f.image);return;}
      if(next<sites.size()) {
        const auto site=sites[next];
        if(site.actor<f.world.lemmings().size()) {
          const auto& l=f.world.lemmings()[site.actor];
          if(l.state==sim::State::Walker && l.x==site.x && l.y==site.y && l.facing==site.face) {
            auto a=std::find_if(f.actors.begin(),f.actors.end(),[&](const auto& v){return v.index==site.actor;});
            check(a!=f.actors.end(),"matched target is visibly projected");
            if(a!=f.actors.end()) {
              const int x=int((a->placement.left+a->placement.right)/2),y=int((a->placement.top+a->placement.bottom)/2);
              check(x>=100 && x<640 && y>=0 && y<400,"matched click in gameplay viewport");
              actions<<age<<','<<site.actor<<','<<l.x<<','<<l.y<<','<<x<<','<<y<<','<<f.world.setup().skills[3]<<'\n';
              actions.flush();
              png(dir/("click-"+std::to_string(site.actor)+".png"),f.image);
              click(x,y);pendingActor=int(site.actor);++clicks;++next;
            }
          }
        }
      } else if(ffAge<0 && f.world.spawned()==(gadget?20:30)) {click(54,348);ffAge=age;}
      if(age>700 && next<sites.size()) {check(false,"matched placement was not reached");quit();}
    };
    std::vector<std::string> args{"hatch-matched-routes","--frontend","--native","--frames","2000","--game-dir",game.string()};
    std::vector<char*> argv;for(auto& a:args)argv.push_back(a.data());
    app::ApplicationRunOptions options;options.frameSeconds=0.2;options.saveStore=store;
    check(app::runApplication(int(argv.size()),argv.data(),front,frame,options)==0,"matched route exits");
    check(results && firstFrame>=0 && pending>0,"matched route reaches natural Results");
    check(next==sites.size() && confirmed==int(sites.size()),"all matched placements confirmed");
    if(pass==0)check(won && saved==23 && mapEarned,"historical placements win 23/30 and genuinely unlock gadget");
    if(miss)check(!won && saved==0,"historical losing placements reproduce trapped zero-save loss");
    if(gadget) {
      // U3: both former survivors used the baked dry-fluid lid. Keep an
      // authenticated entry-simulation replay alongside the corrected route,
      // rather than accepting an arbitrary changed saved count.
      check(mapEarned && spawned==20 && !won && saved==(receiverBaseline?2:0),
            "earned gadget retains its loss, with the U3 dry-pit correction to the saved count");
      check(receiverBaseline ? (drySplats.empty() && exitedActors==std::set<std::size_t>{0,1}) :
                               (drySplats==std::set<std::size_t>{0,1} && exitedActors.empty()),
            "exactly actors 0 and 1 change from saved across the dry lid to dry-floor splats");
    }
    check(births.size()==std::size_t(gadget?20:30),"matched full release count");
    if(paired) {
      std::ifstream oldBirths(baselineDir/"births.txt");unsigned long long ms;std::size_t i=0;
      while(oldBirths>>ms){check(i<births.size() && births[i]-firstMs==ms,"matched every release offset");++i;}
      check(i==births.size(),"matched same number of releases");
      std::ifstream oldOutcome(baselineDir/"outcome.txt");std::string oldResult;std::getline(oldOutcome,oldResult);
      check(oldResult==std::to_string(won)+","+std::to_string(saved)+","+std::to_string(spawned),
            "matched exact saved/count/win outcome");
    }
    {std::ofstream b(dir/"births.txt");for(auto ms:births)b<<ms-firstMs<<'\n';}
    std::ofstream(dir/"outcome.txt")<<won<<','<<saved<<','<<spawned<<'\n';
    std::ofstream(dir/"summary.txt")<<"level="<<level<<"\nwon="<<won<<"\nsaved="<<saved<<"\nspawned="<<spawned
      <<"\nfirst_ms="<<firstMs<<"\nretry_legs="<<leg<<"\nclicks="<<clicks<<"\nconfirmed="<<confirmed
      <<"\nfast_forward_age_frame="<<ffAge<<"\nseconds_remaining="<<remaining<<"\nmap_earned="<<mapEarned
      <<"\ndry_receiver_splats="<<drySplats.size()<<"\nexited_actors="<<exitedActors.size()
      <<"\npaired="<<paired<<"\nfailures="<<failures<<'\n';
  }
  check(installed.unchanged() && normal.unchanged(),"matched routes preserve real saves");
  std::ofstream(out/"summary.txt")<<"installed="<<installed.unchanged()<<"\nnormal="<<normal.unchanged()<<"\nfailures="<<failures<<'\n';
  return failures?1:0;
}
int units(const fs::path& game) {
  auto lib=library(game); auto setup=sim::loadRetailLevel(lib,"MM_CLIMB_002");
  if(!setup) return 2;
  sim::World w(*setup), reference(*setup);
  app::RuntimeSimState sim;
  for(int i=0;i<60;++i) sim.advanceElapsed(w,0.1,false,{},0,false);
  check(w.levelClockMs()==6000 && w.spawned()==0, "hold only release; level clock advances");
  // Default arm/countdown/cadence are untouched by the new gate itself.
  for(int i=0;i<350;++i) {
    sim.advanceElapsed(w,0.1,false,{},0,true);
    reference.advanceElapsedMs(100);
    check(w.spawned()==reference.spawned(), "release stream identical after held offset");
  }
  app::LevelStartSequence start(2.8);
  start.advance(2.8,true);
  check(start.openingProgress()==1 && !start.releaseEnabled(true) && start.releaseEnabled(false),
        "full clip and cue retirement both required");
  app::LevelStartSequence retry(2.8); retry.advance(2.5,true);
  check(!retry.releaseEnabled(false), "short Retry cue still waits for doors");
  retry.advance(20,false); check(retry.openingProgress()<1,"paused door clock");
  return failures ? 1 : 0;
}
}
int main(int argc,char** argv) {
  if(argc!=3 || !std::getenv("WEBOLUTION_GAME_DIR")) return 2;
  fs::path out(argv[2]); fs::create_directories(out);
  if(std::string(argv[1])=="units") return units(std::getenv("WEBOLUTION_GAME_DIR"));
  if(std::string(argv[1])=="assets") return assets(out);
  if(!std::getenv("SDL_VIDEODRIVER") || std::string(std::getenv("SDL_VIDEODRIVER"))!="dummy") return 2;
  if(std::string(argv[1])=="win") return win(out);
  if(std::string(argv[1])=="retry") return retry(out);
  if(std::string(argv[1])=="routes") return routes(out);
  return entry(out,std::string(argv[1])=="fast");
}
