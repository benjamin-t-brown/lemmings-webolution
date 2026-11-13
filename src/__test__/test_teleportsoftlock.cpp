// teleport-softlock-001: source-built production routes on installed assets.
// No original execution. --observe retains the unchanged baseline without claiming a pass.
#include "app/AppEntry.h"
#include "assets/Box.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"
#include "save/SaveStore.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;
using namespace webo::sim;
namespace {
bool observe = false;
int failures = 0;
int checks = 0;
void check(bool b, const std::string& s) {
  ++checks;
  if (!b) { std::cerr << (observe ? "BASELINE defect: " : "FAIL: ") << s << '\n'; if (!observe) ++failures; }
}
const std::vector<std::string> levels{"H06_MT_1", "H08_MT_1", "H10_MT_1", "M12_MT_1", "M16_MG_1",
  "M17_MT_1", "M18_MT_1", "M19_MT_1", "M21_KF_1", "MM_TELE_003", "SHOW_KF3", "SHOW_MT3",
  "VH08_MT", "VH10_MT", "VH12_KF1", "VH13_MT", "VH18_JC", "VH21_JC"};
bool inside(const World& w, const Teleport& p, const Lemming& l) {
  int dx = w.terrain().wrapX(l.x - p.x + w.terrain().width()/2) - w.terrain().width()/2;
  return dx >= 0 && dx <= p.width && l.y >= p.y && l.y <= p.y+p.height;
}
std::string bytes(const fs::path& p) {
  std::ifstream f(p, std::ios::binary); return {std::istreambuf_iterator<char>(f), {}};
}
void summary(std::ostream& out, const std::string& level, const World& w) {
  std::map<std::string,int> states;
  for (const auto& l : w.lemmings()) ++states[std::string(stateName(l.state))];
  out << level << ',' << w.levelClockMs() << ',' << w.spawned() << ',' << w.saved() << ',' << w.dead();
  for (const auto& [s,n] : states) out << ',' << s << ':' << n;
  out << '\n';
}
// Same idle horizon as level-sweep-001: 1 s passes, five seconds after last release,
// or time limit +2 s / level end. No injected actors or skills in this measurement.
void sweep(const AssetLibrary& lib, const fs::path& out) {
  std::ofstream table(out/"idle.csv");
  table << "level,ms,spawned,saved,dead,states\n";
  std::ofstream visits(out/"visits.csv");
  visits << "level,entries,arrivals,departures,max_transit_ms,immediate_reentries\n";
  std::ofstream events(out/"events.csv");
  events << "level,ms,actor,event,x,y,state\n";
  std::ofstream directed(out/"directed.csv");
  directed << "level,source,destination,facing,arrival_x,arrival_y,left,bounced\n";
  for (const auto& name : levels) {
    auto setup = loadRetailLevel(lib, name);
    check(setup.has_value(), name+" loads"); if (!setup) continue;
    World w(*setup);
    struct Visit { State previous = State::Faller; long entered = -1; int destination = -1; };
    std::vector<Visit> watched;
    int entries = 0, arrivals = 0, departures = 0, reentries = 0;
    long maxTransit = 0;
    long released = -1;
    const long cap = setup->timeLimitTicks > 0 ? setup->timeLimitTicks*200L+2000 : 2000000;
    while (w.levelClockMs() < static_cast<unsigned long>(cap) && !w.finished()) {
      w.advanceElapsedMs(1000, false, [&](double) {
        watched.resize(w.lemmings().size());
        for (std::size_t i = 0; i < watched.size(); ++i) {
          auto& v = watched[i];
          const auto& a = w.lemmings()[i];
          const long now = static_cast<long>(w.levelClockMs());
          const auto event = [&](const char* what) {
            events << name << ',' << now << ',' << i << ',' << what << ',' << a.x << ',' << a.y << ',' << stateName(a.state) << '\n';
          };
          if (v.destination >= 0 && !inside(w, setup->teleports[v.destination], a)) {
            ++departures; v.destination = -1; event("leave-destination");
          }
          if (a.state == State::Teleporting && v.previous != State::Teleporting) {
            if (v.destination >= 0) ++reentries;
            ++entries; v.entered = now; event("enter");
          }
          if (v.entered >= 0) {
            maxTransit = std::max(maxTransit, now - v.entered);
            if (v.previous == State::Teleporting && a.state == State::Walker) {
              ++arrivals; event("arrive");
              for (std::size_t j = 0; j < setup->teleports.size(); ++j)
                if (inside(w, setup->teleports[j], a)) v.destination = static_cast<int>(j);
              v.entered = -1;
            } else if (a.state != State::Teleporting) v.entered = -1;
          }
          v.previous = a.state;
        }
        return true;
      });
      if (released < 0 && w.spawned() == setup->totalLemmings) released = static_cast<long>(w.levelClockMs());
      if (released >= 0 && w.levelClockMs() >= static_cast<unsigned long>(released+5000)) break;
    }
    summary(table, name, w);
    visits << name << ',' << entries << ',' << arrivals << ',' << departures << ',' << maxTransit << ',' << reentries << '\n';
    check(reentries == 0 && maxTransit <= 800, name + " natural visits have bounded transit and no immediate re-entry");
    // Natural idle is not a win test. No teleport state should be stuck for longer
    // than the existing four-tick transition; test every actual visit separately below.
    for (std::size_t i=0; i<setup->teleports.size(); ++i) for (Facing face : {Facing::Left, Facing::Right}) {
      LevelSetup route = *setup; route.totalLemmings=0; route.saveTarget=0;
      const auto& pad=route.teleports[i];
      Lemming l; l.x=route.terrain.wrapX(pad.x+pad.width/2); l.y=pad.y+pad.height;
      l.state=State::Walker; l.facing=face;
      World trial(std::move(route)); trial.place(l);
      trial.advanceElapsedMs(200);
      check(trial.lemmings()[0].state==State::Teleporting, name+" enters pad "+std::to_string(i));
      // Observe the actual transition, without imposing a timing retune: the
      // hybrid implementation also increments frame in World's common tail.
      for (int t=0;t<6 && trial.lemmings()[0].state==State::Teleporting;++t) trial.advanceElapsedMs(200);
      const Lemming arrival=trial.lemmings()[0];
      check(arrival.state==State::Walker && arrival.x==trial.terrain().wrapX(pad.destX) && arrival.y==pad.destY && arrival.facing==face,
            name+" reaches serialized partner and preserves facing");
      int destination=-1, containing=0;
      for (std::size_t j=0;j<setup->teleports.size();++j) if (inside(trial,setup->teleports[j],arrival)) {destination=static_cast<int>(j);++containing;}
      check(containing==1, name+" destination has exactly one containing pad");
      bool left=false, bounced=false;
      for (int t=0;t<30 && !left;++t) {
        trial.advanceElapsedMs(200);
        const auto& a=trial.lemmings()[0];
        bounced |= a.state==State::Teleporting;
        left=destination>=0 && !inside(trial,setup->teleports[destination],a);
      }
      check(left && !bounced, name+" leaves destination without re-entry, pad "+std::to_string(i)+" facing "+std::to_string(static_cast<int>(face)));
      directed << name << ',' << i << ',' << destination << ',' << static_cast<int>(face) << ',' << arrival.x << ',' << arrival.y << ',' << left << ',' << bounced << '\n';
      if (left && !bounced) {
        // A departed actor can use the destination again. Carry its production
        // state through one outside update, then seed a deliberate return visit
        // on the same retail pad. A second actor there must be independent.
        trial.advanceElapsedMs(200);
        Lemming returning = trial.lemmings()[0];
        returning.x = arrival.x; returning.y = arrival.y;
        returning.state = State::Walker; returning.frame = 0;
        LevelSetup again = *setup; again.totalLemmings = 0; again.saveTarget = 0;
        World revisit(std::move(again)); revisit.place(returning);
        Lemming newcomer; newcomer.x = arrival.x; newcomer.y = arrival.y;
        newcomer.state = State::Walker; newcomer.facing = face;
        revisit.place(newcomer); revisit.advanceElapsedMs(200);
        check(revisit.lemmings()[0].state == State::Teleporting && revisit.lemmings()[1].state == State::Teleporting,
              name + " rearms a departed actor and keeps visits independent");
      }
    }
  }
}
void trace(const AssetLibrary& lib,const fs::path& out) {
  auto setup=loadRetailLevel(lib,"MM_TELE_003"); check(setup.has_value(),"trace level loads"); if(!setup)return;
  World w(*setup); std::ofstream csv(out/"trace.csv");
  csv << "tick,ms,x,y,state,frame,facing\n";
  int entered=-1, arrivals=0; bool left=false, reentered=false; int dest=-1;
  State previous=State::Faller;
  for(int t=0;t<2500;++t) {
    w.advanceElapsedMs(200);
    if(w.lemmings().empty()) continue;
    const auto& a=w.lemmings()[0];
    csv << w.tickCount()<<','<<w.levelClockMs()<<','<<a.x<<','<<a.y<<','<<stateName(a.state)<<','<<a.frame<<','<<static_cast<int>(a.facing)<<'\n';
    if(entered<0 && a.state==State::Teleporting) entered=t;
    if(previous==State::Teleporting && a.state==State::Walker) {
      ++arrivals;
      if(dest<0) for(std::size_t j=0;j<setup->teleports.size();++j) if(inside(w,setup->teleports[j],a)) dest=static_cast<int>(j);
    }
    if(arrivals==1 && !left && a.state==State::Teleporting) reentered=true;
    if(arrivals==1 && dest>=0 && a.state!=State::Teleporting && !inside(w,setup->teleports[dest],a)) left=true;
    previous=a.state;
    if(entered>=0 && t>=entered+60) break;
  }
  std::ofstream result(out/"trace-result.txt"); result<<"entry="<<entered<<" arrivals="<<arrivals<<" left="<<left<<" reentered="<<reentered<<'\n';
  check(entered>=0 && arrivals>=1 && left && !reentered,"natural first lemming arrives and leaves MM_TELE_003");
}
void appIdle(const fs::path& game,const std::string& level,const fs::path& out) {
  std::vector<std::string> args{"teleportsoftlock","--game-dir",game.string(),"--level",level,"--sim","--native","--frames","2500","--no-audio"};
  std::vector<char*> argv;for(auto& s:args)argv.push_back(s.data());
  app::ApplicationRunOptions options; options.frameSeconds=1.0;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"private-save"/"SavedGame.dat",game));
  long released=-1;bool quit=false, seenEntry=false,seenArrival=false,seenDeparture=false;
  State previous=State::Faller; int dest=-1;
  std::ofstream table(out/(level+"-app.csv"));
  int status=app::runApplication(static_cast<int>(argv.size()),argv.data(),{},[&](const app::LevelFrameView& f){
    const auto& w=f.world;
    if(!w.lemmings().empty()) {
      const auto& a=w.lemmings()[0];
      if(!seenEntry && a.state==State::Teleporting) {check(writePng(out/(level+"-entry.png"),f.image).has_value(),"entry image");seenEntry=true;}
      if(!seenArrival && previous==State::Teleporting && a.state==State::Walker) {
        check(writePng(out/(level+"-arrival.png"),f.image).has_value(),"arrival image");seenArrival=true;
        for(std::size_t j=0;j<w.setup().teleports.size();++j)if(inside(w,w.setup().teleports[j],a))dest=static_cast<int>(j);
      }
      if(seenArrival && !seenDeparture && dest>=0 && a.state!=State::Teleporting && !inside(w,w.setup().teleports[dest],a)) {
        check(writePng(out/(level+"-departure.png"),f.image).has_value(),"departure image"); seenDeparture=true;
      }
      previous=a.state;
    }
    if(released<0 && w.spawned()==w.setup().totalLemmings)released=static_cast<long>(w.levelClockMs());
    const bool stop=(released>=0 && w.levelClockMs()>=static_cast<unsigned long>(released+5000)) ||
      (w.setup().timeLimitTicks>0 && w.levelClockMs()>=static_cast<unsigned long>(w.setup().timeLimitTicks*200L+2000)) || f.endPhase!=app::LevelEndPhase::Running;
    if(stop && !quit) {
      summary(table,level,w);check(writePng(out/(level+"-final.png"),f.image).has_value(),"final image");
      SDL_Event e{};e.type=SDL_QUIT;check(SDL_PushEvent(&e)==1,"quit event");quit=true;
    }
  },options);
  check(status==0 && quit,"bounded application idle "+level);
  check(seenEntry && seenArrival && seenDeparture, "application images include a pad visit and departure " + level);
}
}
int main(int argc,char** argv) {
  if(argc<3)return 2;
  const std::string mode=argv[1];const fs::path out=argv[2];fs::create_directories(out);
  observe=argc>3 && std::string(argv[argc-1])=="--observe";
  const char* g=std::getenv("WEBOLUTION_GAME_DIR");if(!g)return 2;const fs::path game=g;
  const fs::path normal=std::getenv("LOCALAPPDATA")?fs::path(std::getenv("LOCALAPPDATA"))/"LemmingsWebolution"/"SavedGame.dat":fs::path();
  const bool normalExisted = fs::exists(normal), installedExisted = fs::exists(game/"SavedGame.dat");
  const auto normalBefore=bytes(normal), installedBefore=bytes(game/"SavedGame.dat");
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;
  if(mode=="trace")trace(*lib,out);
  else if(mode=="sweep")sweep(*lib,out);
  else if(mode=="app" && argc>3)appIdle(game,argv[3],out);
  else return 2;
  const bool normalSame = fs::exists(normal) == normalExisted && bytes(normal) == normalBefore;
  const bool installedSame = fs::exists(game/"SavedGame.dat") == installedExisted && bytes(game/"SavedGame.dat") == installedBefore;
  check(normalSame && installedSame,"normal and installed save unchanged");
  std::ofstream proof(out/"save-proof.csv");
  proof << "save,existed,bytes,unchanged\nnormal," << normalExisted << ',' << normalBefore.size() << ',' << normalSame
        << "\ninstalled," << installedExisted << ',' << installedBefore.size() << ',' << installedSame << '\n';
  std::cout<<mode<<": "<<checks<<" checks, "<<failures<<" failures"<<(observe?" (baseline observation)":"")<<'\n';
  return failures?1:0;
}
