// Check walker lane changes in MM_CLIMB_002 through a natural hatch release and wall turn.
#include "app/AppEntry.h"
#include "app/LemmingMotion.h"
#include "app/RuntimeHud.h"
#include "assets/Png.h"
#include "sim/World.h"
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

using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures=0;
void check(bool b,const std::string& m) { if(!b) {if(failures<25)std::cerr<<"FAIL: "<<m<<'\n';++failures;} }
bool near(double a,double b) {return std::abs(a-b)<1e-8;}
void event(SDL_Event e) {check(SDL_PushEvent(&e)==1,"enqueue public SDL input");}
void key(SDL_Scancode k) {SDL_Event e{};e.type=SDL_KEYDOWN;e.key.keysym.scancode=k;event(e);}
void point(int x,int y) {SDL_Event e{};e.type=SDL_MOUSEMOTION;e.motion.x=x;e.motion.y=y;event(e);}
void quit() {SDL_Event e{};e.type=SDL_QUIT;event(e);}
std::string bytes(const fs::path& p) {std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
struct SaveSnapshot {
  fs::path path;bool exists;std::string data;
  explicit SaveSnapshot(fs::path p):path(std::move(p)),exists(fs::exists(path)),data(bytes(path)){}
  bool unchanged() const {return exists==fs::exists(path)&&data==bytes(path);}
};
// Every public lemming field, World counters, stock, terrain version and outcome.
std::string state(const sim::World& w) {
  std::ostringstream s;s<<std::setprecision(17)<<w.levelClockMs()<<','<<w.tickCount()<<','<<w.explicitUpdateCount()
    <<','<<w.spawned()<<','<<w.saved()<<','<<w.dead()<<','<<w.finished()<<','<<w.won()<<','<<w.terrain().version();
  for(int stock:w.setup().skills)s<<','<<stock;
  for(const auto& l:w.lemmings()) {
    s<<';'<<l.x<<','<<l.y<<','<<int(l.state)<<','<<int(l.facing)<<','<<l.fallDistance<<','<<l.frame
      <<','<<l.bricksLeft<<','<<l.bashCycles<<','<<l.fuseMs<<','<<l.fasterMs<<','<<l.speedPadLatched
      <<','<<l.canClimb<<','<<l.canFloat<<','<<l.verticalDir<<','<<l.sheetType<<','<<l.stepBudgetSeconds;
    for(auto pad:l.teleportPadsLatched)s<<':'<<pad;
  }
  return s.str();
}
void png(const fs::path& p,const Image& im) {check(writePng(p,im).has_value(),"write clone PNG");}
#ifndef LANE_BASELINE
void hatchAndBlockerLane(sim::Facing facing, bool landsOnRelease) {
  sim::LevelSetup setup;
  setup.terrain = sim::Terrain(256, 160);
  setup.totalLemmings = 1;
  setup.continuousStepperClock = true;
  setup.terrain.fillBox(0, 80, 256, 1, sim::Material::Earth);
  setup.terrain.fillBox(0, 90, 256, 20, sim::Material::Earth);
  setup.entrances.push_back({.x = 128, .y = landsOnRelease ? 76 : 70,
                             .direction = static_cast<std::int16_t>(facing)});
  sim::World world(setup);

  app::LemmingMotionTracker tracker;
  const auto lane = [&] { return tracker.positionOf(0, world).z; };
  const auto state = [&] { return world.lemmings()[0].state; };
  const auto advance = [&] {
    world.advanceElapsedMs(20, false, [&](double) { tracker.observe(world); return true; });
  };
  tracker.observe(world);
  world.advanceElapsedMs(200);
  check(world.lemmings().size() == 1, "hatch releases one lemming");
  if (world.lemmings().empty()) return;
  check(state() == (landsOnRelease ? sim::State::Walker : sim::State::Faller),
        "first observation covers both a fall and an immediate landing");
  check(near(lane(), 0.5), "unobserved hatchling is centred");
  tracker.observe(world);
  if (landsOnRelease) {
    const int landedX = world.lemmings()[0].x;
    app::RuntimeSimState fast;
    fast.toggleFastForward();
    fast.advanceElapsed(world, 0.02, false, [&] { tracker.observe(world); });
    check(world.lemmings()[0].x != landedX, "fast-forward walks before the first draw");
    check(near(lane(), 0.5), "the first draw stays centred after fast-forward");
  }
  tracker.frameDrawn();
  for (int i = 0; i < 100 && state() == sim::State::Faller; ++i) {
    check(near(lane(), 0.5), "hatchling holds the centre during its fall");
    advance();
  }
  check(state() == sim::State::Walker && near(lane(), 0.5), "first landing is centred");

  const double direction = static_cast<int>(facing);
  const double side = 0.5 + 0.25 * direction;
  advance();
  check(near(lane(), 0.5 + 0.02 * direction), "walking eases out from the centre");
  for (int i = 0; i < 12; ++i) advance();
  check(near(lane(), side), "walker reaches its side before blocking");

  check(world.assign(0, sim::Skill::Blocker), "blocker assignment succeeds");
  tracker.observe(world);
  check(near(lane(), side), "blocker assignment preserves lane at the transition");
  advance();
  check(near(lane(), side - 0.02 * direction), "first blocker update eases inward");
  tracker.observe(world);
  check(near(lane(), side - 0.02 * direction), "same-clock observation holds blocker lane");
  for (int i = 0; i < 20; ++i) advance();
  check(state() == sim::State::Blocker && near(lane(), 0.5), "blocker settles at the centre");

  world.terrain().clearBox(0, 80, 256, 1);
  for (int i = 0; i < 100 && state() != sim::State::Walker; ++i) {
    check(near(lane(), 0.5), "released blocker holds its lane through the fall");
    advance();
  }
  check(state() == sim::State::Walker && near(lane(), 0.5), "released blocker lands at the centre");
  advance();
  check(near(lane(), 0.5 + 0.02 * direction), "released blocker eases back toward its side");
  for (int i = 0; i < 12; ++i) advance();
  check(near(lane(), side), "released blocker reaches its walking lane");
}

void units(const fs::path& out) {
  fs::create_directories(out);
  std::ofstream trace(out/"turns.csv");trace<<"ms,facing,z\n";
  for(auto facing:{sim::Facing::Right,sim::Facing::Left}) {
    sim::LevelSetup setup;setup.terrain=sim::Terrain(256,160);setup.totalLemmings=0;
    setup.continuousStepperClock=true;setup.terrain.fillBox(0,140,256,20,sim::Material::Earth);
    setup.terrain.fillBox(33,0,1,140,sim::Material::Steel);
    setup.terrain.fillBox(20,0,1,140,sim::Material::Steel);
    sim::World w(setup),control(setup);sim::Lemming l;l.x=31;l.y=139;l.state=sim::State::Walker;l.facing=facing;
    w.place(l);control.place(l);app::LemmingMotionTracker t;t.observe(w);
    check(near(t.positionOf(0,w).z,facing==sim::Facing::Left?0.25:0.75),"placed walker starts in its facing lane");
    double prior=t.positionOf(0,w).z;auto oldFacing=facing;bool turned=false,intermediate=false,arrived=false;
    for(int n=0;n<250;++n) {
      w.advanceElapsedMs(20,false,[&](double){t.observe(w);return true;});control.advanceElapsedMs(20);
      check(state(w)==state(control),"radial presentation never writes simulation");
      const auto p=t.positionOf(0,w);const auto& cur=w.lemmings()[0];
      check(p.z>=0.25&&p.z<=0.75&&std::abs(p.z-prior)<=0.02000001,"bounded continuous lane movement, no overshoot");
      if(cur.facing!=oldFacing) {turned=true;check(std::abs(p.z-prior)<0.021,"turn does not jump to opposite lane");}
      if(turned) {intermediate|=p.z>0.25+1e-8&&p.z<0.75-1e-8;arrived|=near(p.z,cur.facing==sim::Facing::Left?0.25:0.75);}
      trace<<w.levelClockMs()<<','<<int(cur.facing)<<','<<p.z<<'\n';
      const auto frozen=p;t.observe(w);check(near(t.positionOf(0,w).z,frozen.z),"repeated observation consumes no lane budget");
      if(n==20) {app::RuntimeSimState runtime;runtime.togglePause();for(int j=0;j<8;++j)runtime.advanceElapsed(w,0.1,false,[&]{t.observe(w);});
        check(near(t.positionOf(0,w).z,frozen.z),"pause freezes an in-flight lane transition");}
      prior=p.z;oldFacing=cur.facing;
    }
    check(turned&&intermediate&&arrived,"natural wall turns cross intermediate depths and settle");
    // Fast-forward advances actor lanes on the shared level clock.
    app::RuntimeSimState fast;fast.toggleFastForward();const auto before=w.levelClockMs();
    fast.advanceElapsed(w,0.02,false,[&]{t.observe(w);});
    check(w.levelClockMs()==before+140,"fast-forward uses unchanged seven-pass level time");
    hatchAndBlockerLane(facing, false);
    hatchAndBlockerLane(facing, true);
  }
  // A narrow lane turns before the crossing completes. Track the confirmed facing on each turn.
  {
    sim::LevelSetup s;s.terrain=sim::Terrain(256,160);s.totalLemmings=0;s.continuousStepperClock=true;
    s.terrain.fillBox(0,140,256,20,sim::Material::Earth);
    s.terrain.fillBox(29,0,1,140,sim::Material::Steel);s.terrain.fillBox(33,0,1,140,sim::Material::Steel);
    sim::World w(s);sim::Lemming l;l.x=31;l.y=139;l.state=sim::State::Walker;w.place(l);
    app::LemmingMotionTracker t;t.observe(w);auto prior=t.positionOf(0,w).z;
    auto facing=l.facing;int turns=0,reversals=0;bool inFlight=false;
    for(int n=0;n<180;++n) {
      w.advanceElapsedMs(20,false,[&](double){t.observe(w);return true;});const auto& cur=w.lemmings()[0];
      const double z=t.positionOf(0,w).z;
      const double target=facing==sim::Facing::Left?0.25:0.75;
      check(near(z,prior+std::clamp(target-prior,-0.02,0.02)),"lane follows previous confirmed facing at one slab unit/s");
      if(cur.facing!=facing) {++turns;reversals+=inFlight;inFlight=z>0.25+1e-8&&z<0.75-1e-8;}
      prior=z;facing=cur.facing;
    }
    check(turns>3&&reversals>0,"narrow corridor retargets repeated turns while still crossing");
  }
  std::ofstream(out/"summary.txt")<<"failures="<<failures<<'\n';
}
#endif

int application(bool baseline,const fs::path& out,const fs::path& before) {
  if(!std::getenv("WEBOLUTION_GAME_DIR")||!std::getenv("SDL_VIDEODRIVER")||std::string(std::getenv("SDL_VIDEODRIVER"))!="dummy")return 2;
  fs::create_directories(out);const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  auto local=save::localAppDataDirectory();if(!local)return 2;
  SaveSnapshot installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  std::ofstream simTrace(out/"simulation.txt"),trace(out/"lane.csv");
  const bool compare=!baseline&&before!=fs::path("-");std::ifstream expected;if(compare)expected.open(before/"simulation.txt");
  check(!compare||expected.good(),"authentic baseline simulation trace exists");
  trace<<std::setprecision(17)<<"frame,ms,index,state,facing,sim_x,sim_y,draw_x,draw_y,z,origin_x,origin_y,hover,paused\n";
  app::ApplicationRunOptions options;options.frameSeconds=0.02;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  std::vector<std::string> args{"walker-lane","--level","MM_CLIMB_002","--sim","--frames","1600","--native","--game-dir",game.string()};
  std::vector<char*> raw;for(auto& a:args)raw.push_back(a.data());
  int frames=0,turnFrame=-1,pausedFrames=0,picks=0,images=0,sliding=0,left=0,right=0;bool pausedOnce=false,resumed=false;
  int lastFacing=1;double prevZ=0.75,frozenZ=0;std::uint64_t frozenMs=0;bool hadActor=false,settled=false;
  bool oppositeLanesTogether=false;
  const auto observer=[&](const app::LevelFrameView& f) {
    ++frames;const auto serial=std::to_string(f.frame)+","+std::to_string(f.paused)+","+state(f.world);simTrace<<serial<<'\n';
    if(compare) {std::string wanted;check(bool(std::getline(expected,wanted))&&serial==wanted,"identical simulation at frame "+std::to_string(f.frame));}
    const auto actor=std::find_if(f.actors.begin(),f.actors.end(),[](const auto& a){return a.index==0;});
    if(f.world.lemmings().empty())return;
    const auto& l=f.world.lemmings()[0];
    if(turnFrame<0&&l.state==sim::State::Walker&&int(l.facing)!=lastFacing)turnFrame=f.frame;
    lastFacing=int(l.facing);
    if(!baseline&&settled) {
      bool innerLeft=false,outerRight=false;
      for(const auto& a:f.actors) {
        const auto& drawn=f.world.lemmings()[a.index];
        if(drawn.state!=sim::State::Walker)continue;
        innerLeft|=drawn.facing==sim::Facing::Left&&near(a.drawnSample[2],0.25);
        outerRight|=drawn.facing==sim::Facing::Right&&near(a.drawnSample[2],0.75);
      }
      oppositeLanesTogether|=innerLeft&&outerRight;
    }
    if(actor!=f.actors.end()) {
      const double z=actor->drawnSample[2];
      trace<<f.frame<<','<<f.world.levelClockMs()<<",0,"<<sim::stateName(l.state)<<','<<int(l.facing)<<','<<l.x<<','<<l.y
        <<','<<actor->drawnSample[0]<<','<<actor->drawnSample[1]<<','<<z<<','<<actor->placement.originX<<','<<actor->placement.originY
        <<','<<(f.hoveredActor?int(*f.hoveredActor):-1)<<','<<f.paused<<'\n';
      if(l.state==sim::State::Walker) {
        left+=l.facing==sim::Facing::Left;right+=l.facing==sim::Facing::Right;
        if(baseline)check(near(z,0.75),"before both facings draw at identical radial depth 0.75");
        else {check(z>=0.25&&z<=0.75,"walk depth lies between lanes");
          if(hadActor)check(std::abs(z-prevZ)<=0.02000001,"installed wall turn slides without depth jump");
          if(turnFrame>=0) {sliding+=z>0.25+1e-8&&z<0.75-1e-8;settled|=near(z,0.25);}}
      }
      if(turnFrame>=0&&f.frame-turnFrame<=40&&images<30) {png(out/("turn-"+std::to_string(images++)+".png"),f.image);}
      if(!pausedOnce&&turnFrame>=0&&f.frame>=turnFrame+8) {pausedOnce=true;key(SDL_SCANCODE_P);}
      if(f.paused) {
        if(pausedFrames++==0) {frozenZ=z;frozenMs=f.world.levelClockMs();png(out/"paused.png",f.image);}
        check(near(z,frozenZ)&&f.world.levelClockMs()==frozenMs,"pause freezes radial sample and simulation clock");
        const int x=int((actor->placement.left+actor->placement.right)/2),y=int((actor->placement.top+actor->placement.bottom)/2);
        point(x,y);
        if(pausedFrames>1) {check(f.hoveredActor==0,"hover picks the drawn lemming during its lane crossing");++picks;}
        if(pausedFrames==5)key(SDL_SCANCODE_P);
      } else if(pausedOnce&&pausedFrames==5)resumed=true;
      prevZ=z;hadActor=true;
    }
    if(turnFrame>=0&&f.frame>turnFrame+55) {png(out/"settled-left.png",f.image);quit();}
    if(f.frame==1600)quit();
  };
  check(app::runApplication(int(raw.size()),raw.data(),{},observer,options)==0,"production app exits normally");
  check(turnFrame>=0&&left>20&&right>20,"installed natural hatch route walks both ways and turns at wall");
  check(pausedFrames==5&&picks==4&&resumed,"paused mid-crossing hover and resume route completes");
  if(!baseline) {
    check(sliding>=15&&settled,"installed walker crosses intermediate depths and settles on inner lane");
    check(oppositeLanesTogether,"installed walkers face both ways simultaneously on opposite sides of the path");
  }
  if(compare) {std::string extra;check(!std::getline(expected,extra),"both traces have identical frame count and outcome");}
  check(installed.unchanged()&&normal.unchanged(),"installed and normal saves unchanged");
  std::ofstream(out/"summary.txt")<<"frames="<<frames<<"\nturn_frame="<<turnFrame<<"\nleft_frames="<<left<<"\nright_frames="<<right
    <<"\nintermediate_depth_frames="<<sliding<<"\nsettled_left="<<settled<<"\npaused_frames="<<pausedFrames<<"\nhover_picks="<<picks
    <<"\nopposite_lanes_together="<<oppositeLanesTogether
    <<"\npaired_simulation_comparison="<<compare<<"\ninstalled_save_unchanged="<<installed.unchanged()<<"\nnormal_save_unchanged="<<normal.unchanged()<<"\nfailures="<<failures<<'\n';
  std::cout<<"walker lane evidence: "<<out<<" frames="<<frames<<" turn="<<turnFrame<<" failures="<<failures<<'\n';
  return failures?1:0;
}
}
int main(int argc,char** argv) {
  if(argc!=4)return 2;
#ifndef LANE_BASELINE
  if(std::string(argv[1])=="units") {units(argv[2]);return failures?1:0;}
#endif
  return application(std::string(argv[1])=="baseline",argv[2],argv[3]);
}
