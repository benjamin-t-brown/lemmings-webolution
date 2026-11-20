#include "app/AppEntry.h"
#include "app/LemmingMotion.h"
#include "app/RuntimeHud.h"
#include "assets/Png.h"
#include "sim/World.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool ok, const std::string& message) {
  if (!ok) { ++failures; if (failures < 30) std::cerr << "FAIL: " << message << '\n'; }
}
bool near(double a, double b) { return std::abs(a-b) < 1e-8; }
void push(SDL_Event e) { check(SDL_PushEvent(&e) == 1, "enqueue public SDL input"); }
void key(SDL_Scancode k) { SDL_Event e{}; e.type=SDL_KEYDOWN; e.key.keysym.scancode=k; push(e); }
void motion(int x, int y) { SDL_Event e{}; e.type=SDL_MOUSEMOTION; e.motion.x=x; e.motion.y=y; push(e); }
void click(int x, int y) {
  SDL_Event e{}; e.type=SDL_MOUSEBUTTONDOWN; e.button.button=SDL_BUTTON_LEFT;
  e.button.x=x; e.button.y=y; push(e); e.type=SDL_MOUSEBUTTONUP; push(e);
}
void quit() { SDL_Event e{}; e.type=SDL_QUIT; push(e); }
std::string bytes(const fs::path& p) { std::ifstream in(p,std::ios::binary); return {std::istreambuf_iterator<char>(in),{}}; }
struct SaveSnapshot {
  fs::path path; bool exists; std::string data;
  explicit SaveSnapshot(fs::path p):path(std::move(p)),exists(fs::exists(path)),data(bytes(path)){}
  bool unchanged() const { return exists==fs::exists(path) && data==bytes(path); }
};
void image(const fs::path& p,const Image& im) { check(writePng(p,im).has_value(),"write clone PNG "+p.string()); }

// All public per-lemming simulation fields, counters, stock, ticks and outcome.
// Presentation and image data are deliberately absent from this identity trace.
std::string state(const sim::World& w) {
  std::ostringstream s; s << std::setprecision(17) << w.levelClockMs() << ',' << w.tickCount() << ','
    << w.explicitUpdateCount() << ',' << w.spawned() << ',' << w.saved() << ',' << w.dead() << ','
    << w.finished() << ',' << w.won() << ',' << w.terrain().version();
  for (int stock:w.setup().skills) s << ',' << stock;
  for (const auto& l:w.lemmings()) {
    s << ';' << l.x << ',' << l.y << ',' << int(l.state) << ',' << int(l.facing) << ','
      << l.fallDistance << ',' << l.frame << ',' << l.bricksLeft << ',' << l.bashCycles << ','
      << l.fuseMs << ',' << l.fasterMs << ',' << l.speedPadLatched << ',' << l.canClimb << ','
      << l.canFloat << ',' << l.verticalDir << ',' << l.sheetType << ',' << l.stepBudgetSeconds;
    for (auto pad:l.teleportPadsLatched) s << ':' << pad;
  }
  return s.str();
}

sim::LevelSetup flat() {
  sim::LevelSetup s; s.terrain=sim::Terrain(256,160);
  s.terrain.fillBox(0,140,256,20,sim::Material::Earth);
  s.totalLemmings=0; s.continuousStepperClock=true;
  s.skills.fill(-1); return s;
}
void units(const fs::path& out) {
  fs::create_directories(out);
  std::ofstream trace(out/"trace.csv"); trace << "fixture,ms,sim_x,sim_y,draw_x,draw_y\n";
  for (auto st:{sim::State::Walker,sim::State::Faller,sim::State::Floater,
               sim::State::Climber,sim::State::Builder,sim::State::Miner,sim::State::Digger,sim::State::Bounce}) {
    auto setup=flat();
    if(st==sim::State::Climber) setup.terrain.fillBox(31,0,2,140,sim::Material::Earth);
    if(st==sim::State::Miner || st==sim::State::Digger) setup.terrain.fillBox(0,100,256,40,sim::Material::Earth);
    sim::World w(setup), control(setup);
    sim::Lemming l; l.x=30;l.y=139;l.state=st;l.bricksLeft=12;
    if(st==sim::State::Climber) {l.x=31;l.y=120;}
    if(st==sim::State::Faller || st==sim::State::Floater || st==sim::State::Bounce) l.y=60;
    if(st==sim::State::Floater) {l.canFloat=true;l.fallDistance=30;}
    w.place(l); control.place(l);
    app::LemmingMotionTracker tracker; tracker.observe(w);
    auto previous=tracker.positionOf(0,w); auto prior=l;
    int between=0, fractional=0;
    for(int f=0;f<150;++f) {
      w.advanceElapsedMs(20,false,[&](double){tracker.observe(w);return true;});
      control.advanceElapsedMs(20);
      check(state(w)==state(control),"tracker never changes simulation fields");
      auto p=tracker.positionOf(0,w);const auto& cur=w.lemmings()[0];
      if(cur.state==st && prior.state==st && cur.facing==prior.facing) {
        if(cur.x==prior.x && cur.y==prior.y && (!near(p.x,previous.x)||!near(p.y,previous.y))) ++between;
        if(!near(p.x,std::round(p.x)) || !near(p.y,std::round(p.y))) ++fractional;
        if(st==sim::State::Walker) {
          check(p.x>=previous.x-1e-8 && p.x<=cur.x+1e-8,"walker monotonic, no overshoot");
          if(w.levelClockMs()==220) check(near(p.x,30.1),"known previous/current walk segment sampled at 10 percent");
        }
        if(st==sim::State::Climber) check(p.y<=previous.y+1e-8 && p.y>=cur.y-1e-8,"climber monotonic, no overshoot");
        if(st==sim::State::Faller || st==sim::State::Floater) check(p.y>=previous.y-1e-8 && p.y<=cur.y+1e-8,"fall/float monotonic, no overshoot");
      }
      if(cur.facing!=prior.facing || cur.verticalDir!=prior.verticalDir ||
          (cur.state==sim::State::Shrugger && prior.state!=cur.state))
        check(near(p.x,cur.x)&&near(p.y,cur.y),"turn/gravity/stationary discontinuity snaps immediately");
      trace << sim::stateName(st)<<','<<w.levelClockMs()<<','<<cur.x<<','<<cur.y<<','<<p.x<<','<<p.y<<'\n';
      prior=cur;previous=p;
    }
    check(between>0 && fractional>0,"fractional movement between steps: "+std::string(sim::stateName(st)));
    app::RuntimeSimState runtime; runtime.togglePause(); const auto frozen=tracker.positionOf(0,w);
    for(int f=0;f<12;++f) {
      runtime.advanceElapsed(w,0.1,false,[&]{tracker.observe(w);});tracker.observe(w);
      auto p=tracker.positionOf(0,w);check(near(p.x,frozen.x)&&near(p.y,frozen.y),"pause freezes presentation exactly");
    }
  }
  // Real wall turn while an earlier segment is still buffered.
  {
    auto setup=flat();setup.terrain.fillBox(33,0,1,140,sim::Material::Steel);
    sim::World w(setup);sim::Lemming l;l.x=31;l.y=139;l.state=sim::State::Walker;w.place(l);
    app::LemmingMotionTracker t;t.observe(w);bool turned=false;
    for(int f=0;f<50;++f) {
      auto old=w.lemmings()[0];w.advanceElapsedMs(20,false,[&](double){t.observe(w);return true;});
      if(old.facing!=w.lemmings()[0].facing) {auto p=t.positionOf(0,w);check(near(p.x,w.lemmings()[0].x),"wall turn clears buffered approach");turned=true;}
    }
    check(turned,"wall-turn fixture actually turns");
  }
  // Seam wraps continuously, instead of interpolating through the cylinder.
  {
    sim::World w(flat());sim::Lemming l;l.x=255;l.y=139;l.state=sim::State::Walker;w.place(l);
    app::LemmingMotionTracker t;t.observe(w);
    w.advanceElapsedMs(200,false,[&](double){t.observe(w);return true;});
    w.advanceElapsedMs(100,false,[&](double){t.observe(w);return true;});
    check(near(t.positionOf(0,w).x,255.5),"wrapped seam uses shortest local segment");
  }
  // Existing teleport state transition must snap even if the destination is nearby.
  for(int dest:{31,180}) {
    sim::World w(flat());sim::Lemming l;l.x=30;l.y=139;l.state=sim::State::Teleporting;
    l.frame=w.rules().teleportDelayTicks-1;l.bricksLeft=dest;l.fallDistance=139;w.place(l);
    app::LemmingMotionTracker t;t.observe(w);w.advanceElapsedMs(200,false,[&](double){t.observe(w);return true;});
    auto p=t.positionOf(0,w);check(w.lemmings()[0].x==dest && near(p.x,dest),"teleport relocation snaps");
  }
  std::ofstream(out/"summary.txt") << "failures="<<failures<<'\n';
}

int application(bool baseline,const fs::path& out,const fs::path& before) {
  const char* gameEnv=std::getenv("WEBOLUTION_GAME_DIR");
  if(!gameEnv || !std::getenv("SDL_VIDEODRIVER") || std::string(std::getenv("SDL_VIDEODRIVER"))!="dummy") return 2;
  fs::create_directories(out);const fs::path game(gameEnv);
  SaveSnapshot installed(game/"SavedGame.dat");auto local=save::localAppDataDirectory();
  if(!local) return 2;
  SaveSnapshot normal(save::cloneSavePathUnder(*local));
  std::ofstream trace(out/"motion.csv"),simTrace(out/"simulation.txt");
  // The production-motion check supports a single application build.
  const bool compare=!baseline && before!=fs::path("-");
  std::ifstream expected;if(compare) expected.open(before/"simulation.txt");
  check(!compare||expected.good(),"authentic baseline trace available for simulation identity");
  trace << std::setprecision(17)<<"frame,ms,index,state,sim_x,sim_y,draw_x,draw_y,origin_x,origin_y,paused,hover\n";
  app::ApplicationRunOptions options;options.frameSeconds=0.02;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  std::vector<std::string> args{"actor-motion","--level","MM_CLIMB_002","--sim","--frames","1400","--native","--game-dir",game.string()};
  std::vector<char*> raw;for(auto& a:args)raw.push_back(a.data());
  int frames=0,walkFrames=0,drawChanges=0,simChanges=0,between=0,pauseFrames=0,pausedAt=-1;
  bool assigned=false,clicked=false,pausedOnce=false;int sampleSequence=0,landingFrames=0;
  std::optional<app::LevelActorView> prevActor;sim::Lemming previous;std::uint64_t previousMs=0;
  double frozenX=0,frozenY=0;int stock=-1;bool sawClimb=false;
  const auto observer=[&](const app::LevelFrameView& f) {
    ++frames;const auto serial=std::to_string(f.frame)+","+std::to_string(f.paused)+","+state(f.world);
    simTrace<<serial<<'\n';if(compare){std::string wanted;check(bool(std::getline(expected,wanted))&&serial==wanted,"full per-frame simulation equals authentic task-entry route at frame "+std::to_string(f.frame));}
    auto actor=std::find_if(f.actors.begin(),f.actors.end(),[](const auto& a){return a.index==0;});
    if(actor==f.actors.end()) return;
    auto a=*actor;const auto& l=f.world.lemmings()[0];
    if(baseline) a.drawnSample={double(l.x),f.world.terrain().height()-1.0-l.y,0.75};
    const double x=a.drawnSample[0],y=f.world.terrain().height()-1.0-a.drawnSample[1];
    if(prevActor && previous.state==sim::State::Faller && l.state==sim::State::Walker && previous.facing==l.facing) {
      const double priorY=f.world.terrain().height()-1.0-prevActor->drawnSample[1];
      if(!baseline) check(y>=priorY && y-priorY<=1.0,"installed landing keeps the continuous confirmed fall path");
      image(out/"landing.png",f.image);++landingFrames;
    }
    trace<<f.frame<<','<<f.world.levelClockMs()<<",0,"<<sim::stateName(l.state)<<','<<l.x<<','<<l.y<<','<<x<<','<<y<<','
      <<a.placement.originX<<','<<a.placement.originY<<','<<f.paused<<','<<(f.hoveredActor?int(*f.hoveredActor):-1)<<'\n';
    if(prevActor && !f.paused && !assigned && previous.state==sim::State::Walker && l.state==sim::State::Walker && l.facing==previous.facing) {
      ++walkFrames;const bool sc=l.x!=previous.x||l.y!=previous.y;
      const bool dc=!near(a.placement.originX,prevActor->placement.originX)||!near(a.placement.originY,prevActor->placement.originY);
      simChanges+=sc;drawChanges+=dc;between+=dc&&!sc;
      if(baseline) check(dc==sc,"before drawn origin changes only on a simulation cell step");
      else check(x>=prevActor->drawnSample[0]-1e-8 && x<=l.x+1e-8,"installed walker is monotonic and never overshoots");
      if(walkFrames>=35 && sampleSequence<12) {image(out/("walk-"+std::to_string(sampleSequence++)+".png"),f.image);}
    }
    if(!pausedOnce && walkFrames>=75) {pausedOnce=true;key(SDL_SCANCODE_P);}
    if(f.paused) {
      ++pauseFrames;
      if(pausedAt<0) {
        pausedAt=f.frame;frozenX=x;frozenY=y;stock=f.world.setup().skills[0];
        if(!baseline) check(!near(x,std::round(x)),"pick regression pauses at an actual fractional draw position");
        image(out/"pause-start.png",f.image);
      }
      check(near(x,frozenX)&&near(y,frozenY)&&f.world.levelClockMs()==previousMs,"paused actor and clock frozen");
      // Aim using the rectangle the player actually sees, including fractional placement.
      const int px=int((a.placement.left+a.placement.right)/2),py=int((a.placement.top+a.placement.bottom)/2);
      motion(px,py);
      if(pauseFrames==8) {
        check(f.hoveredActor==0,"hover selects the displayed interpolated actor");
        check(px>=100&&px<640&&py>=0&&py<480,"pick is outside skill plates and inside window");
        image(out/"pause-end.png",f.image);click(px,py);clicked=true;
      }
    }
    if(clicked && !f.paused && !assigned) {
      check(l.canClimb && f.world.setup().skills[0]==stock-1,"public click assigns the visible actor and resumes");assigned=true;
      image(out/"assigned.png",f.image);motion(620,460);
    }
    if(l.state==sim::State::Climber) {if(!sawClimb) image(out/"climb.png",f.image);sawClimb=true;}
    if(assigned && f.frame>pausedAt+700) quit();
    previous=l;prevActor=a;previousMs=f.world.levelClockMs();
  };
  check(app::runApplication(int(raw.size()),raw.data(),{},observer,options)==0,"production application exits successfully");
  check(walkFrames>=70 && simChanges>=5,"installed normal-speed measurement has sustained walking");
  check(baseline?between==0:between>walkFrames/2,"between-step presented movement matches expected side");
  check(pausedOnce&&pauseFrames==8&&assigned,"pause/hover/click route completes");
  check(sawClimb,"assigned walker reaches climbing on the actual installed wall");
  check(landingFrames>0,"installed route actually exercises fall-to-walk continuity");
  if(compare){std::string extra;check(!std::getline(expected,extra),"baseline and production have identical frame counts/outcome");}
  check(installed.unchanged()&&normal.unchanged(),"installed and normal user saves unchanged");
  std::ofstream summary(out/"summary.txt");summary<<"frames="<<frames<<"\nwalk_intervals="<<walkFrames<<"\nwalk_seconds="<<walkFrames*0.02
    <<"\nsimulation_position_changes="<<simChanges<<"\ndrawn_origin_changes="<<drawChanges<<"\nbetween_step_draw_changes="<<between
    <<"\npaused_frames="<<pauseFrames<<"\nassigned="<<assigned<<"\nsaw_climb="<<sawClimb<<"\nlanding_transitions="<<landingFrames<<"\npaired_simulation_comparison="<<compare
    <<"\ninstalled_save_unchanged="<<installed.unchanged()<<"\nnormal_save_unchanged="<<normal.unchanged()<<"\nfailures="<<failures<<'\n';
  std::cout<<"evidence: "<<out.string()<<"; walk "<<walkFrames<<" frames, "<<simChanges<<" cell changes, "<<drawChanges<<" drawn origin changes\n";
  return failures?1:0;
}

// Use the ordinary wall clock without --frames or a frameSeconds override to count
// presented frames and position changes per second.
int wallMeasurement(bool baseline,const fs::path& out) {
  if(!std::getenv("WEBOLUTION_GAME_DIR") || !std::getenv("SDL_VIDEODRIVER") ||
      std::string(std::getenv("SDL_VIDEODRIVER"))!="dummy") return 2;
  fs::create_directories(out);const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  auto local=save::localAppDataDirectory();if(!local)return 2;
  SaveSnapshot installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  app::ApplicationRunOptions options;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  std::vector<std::string> args{"actor-motion-wall","--level","MM_CLIMB_002","--sim","--native","--game-dir",game.string()};
  std::vector<char*> raw;for(auto& a:args)raw.push_back(a.data());
  std::ofstream trace(out/"motion.csv");trace<<std::setprecision(17)<<"frame,wall_seconds,level_ms,sim_x,sim_y,origin_x,origin_y\n";
  auto started=std::chrono::steady_clock::now(),previousWall=started;
  std::optional<app::LevelActorView> prev;sim::Lemming previous;
  int walking=0,sc=0,dc=0,between=0,frames=0,images=0;double seconds=0;
  const auto observer=[&](const app::LevelFrameView& f) {
    const auto now=std::chrono::steady_clock::now();++frames;
    auto actor=std::find_if(f.actors.begin(),f.actors.end(),[](const auto& a){return a.index==0;});
    if(actor!=f.actors.end()) {
      const auto& l=f.world.lemmings()[0];
      trace<<f.frame<<','<<std::chrono::duration<double>(now-started).count()<<','<<f.world.levelClockMs()<<','
        <<l.x<<','<<l.y<<','<<actor->placement.originX<<','<<actor->placement.originY<<'\n';
      if(prev && l.state==sim::State::Walker && previous.state==l.state && l.facing==previous.facing) {
        ++walking;seconds+=std::chrono::duration<double>(now-previousWall).count();
        const bool simChanged=l.x!=previous.x||l.y!=previous.y;
        const bool drawChanged=!near(actor->placement.originX,prev->placement.originX)||!near(actor->placement.originY,prev->placement.originY);
        sc+=simChanged;dc+=drawChanged;between+=drawChanged&&!simChanged;
        if(baseline) check(drawChanged==simChanged,"ordinary wall-clock before placement only changes on a cell step");
        if(walking>=12 && images<12) image(out/("walk-"+std::to_string(images++)+".png"),f.image);
      }
      prev=*actor;previous=l;
    }
    previousWall=now;
    if(f.world.levelClockMs()>=6000 || std::chrono::duration<double>(now-started).count()>15.0) quit();
  };
  check(app::runApplication(int(raw.size()),raw.data(),{},observer,options)==0,"ordinary clock application exits");
  check(walking>=15 && seconds>1 && sc>=5,"ordinary normal-speed installed walking measurement is substantial");
  check(baseline?between==0:between>0,"ordinary wall-clock drawing gains between-step movement");
  check(installed.unchanged()&&normal.unchanged(),"wall measurement preserves both real saves");
  std::ofstream summary(out/"summary.txt");summary<<"frames="<<frames<<"\nwalking_intervals="<<walking<<"\nwalking_wall_seconds="<<seconds
    <<"\npresented_frames_per_second="<<walking/seconds<<"\nsimulation_position_changes_per_second="<<sc/seconds
    <<"\ndrawn_origin_changes_per_second="<<dc/seconds<<"\nbetween_step_draw_changes="<<between
    <<"\ninstalled_save_unchanged="<<installed.unchanged()<<"\nnormal_save_unchanged="<<normal.unchanged()<<"\nfailures="<<failures<<'\n';
  std::cout<<"ordinary clock evidence: "<<out.string()<<"; "<<walking/seconds<<" FPS, "<<sc/seconds<<" sim changes/s, "<<dc/seconds<<" drawn changes/s\n";
  return failures?1:0;
}
}
int main(int argc,char** argv) {
  if(argc<3)return 2;
  if(std::string(argv[1])=="units") {units(argv[2]);return failures?1:0;}
  if(std::string(argv[1])=="wall-baseline" || std::string(argv[1])=="wall-production")
    return wallMeasurement(std::string(argv[1])=="wall-baseline",argv[2]);
  if(argc!=4)return 2;
  return application(std::string(argv[1])=="baseline",argv[2],argv[3]);
}
