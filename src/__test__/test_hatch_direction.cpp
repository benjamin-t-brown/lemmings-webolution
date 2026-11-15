// Installed data -> production World release and source-built application images.
#include "app/AppEntry.h"
#include "app/FrameConsumption.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#include "lemmings/compiled_level.hpp"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <bit>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>

using namespace webo;
namespace fs = std::filesystem;
static_assert(sizeof(sim::Entrance)==16 && offsetof(sim::Entrance,sheetType)==12);
static_assert(offsetof(HatchRecord,boxMin)==32);
namespace {
#ifdef HATCH_DIRECTION_BEFORE
constexpr bool before = true;
#else
constexpr bool before = false;
#endif
int failures = 0, checks = 0;
void check(bool value, const std::string& message) {
  ++checks;
  if (!value && ++failures < 40) std::cerr << "FAIL: " << message << '\n';
}
std::string bytes(const fs::path& p) {
  std::ifstream in(p, std::ios::binary); return {std::istreambuf_iterator<char>(in), {}};
}
struct SaveSnapshot {
  fs::path path; bool exists; std::string data;
  explicit SaveSnapshot(fs::path p) : path(std::move(p)), exists(fs::exists(path)), data(bytes(path)) {}
  bool unchanged() const { return exists == fs::exists(path) && data == bytes(path); }
};
void quit() { SDL_Event e{}; e.type = SDL_QUIT; check(SDL_PushEvent(&e) == 1, "queue clone quit"); }

int census(AssetLibrary& lib, const fs::path& out) {
  std::ofstream table(out/"hatches.csv");
  table << "file,hatch,count,x,y,direction,alternate,vertical_flip\n";
  int levels = 0, hatches = 0;
  std::map<int,int> fixed, alternating;
  for (const auto& file : levelFileNames(lib)) {
    const auto data = lib.read(file); check(bool(data), "read " + file); if (!data) continue;
    const auto level = lemmings::read_compiled_level({reinterpret_cast<const uint8_t*>(data->data()),data->size()},false);
    const auto setup = sim::loadRetailLevel(lib,file); check(bool(setup), "load " + file); if (!setup) continue;
    const auto projected = readHatchRecords(*data);
    const auto located = findHatchRecords(*data);
    check(projected.size() == level.gadgets.entities_425e50.size(), "structured hatch count");
    check(located.size() == projected.size(), "installed locator count");
    std::vector<int> directions;
    for (std::size_t i=0;i<projected.size();++i) {
      const auto& raw = level.gadgets.entities_425e50[i];
      const int direction = std::bit_cast<int16_t>(raw.short_b4);
      directions.push_back(direction);
      table << file << ',' << i << ',' << raw.shorts_98_9a_9c[0] << ',' << raw.shorts_98_9a_9c[1]
        << ',' << raw.shorts_98_9a_9c[2] << ',' << direction << ',' << raw.flag_b6 << ',' << raw.flag_ad << '\n';
      if (raw.flag_b6) ++alternating[direction]; else ++fixed[direction];
      if (!before) {
        check(projected[i].direction == direction && projected[i].alternateDirection == raw.flag_b6, "structured projection");
        check(located[i].direction == direction && located[i].alternateDirection == raw.flag_b6, "locator projection");
        check(setup->entrances[i].direction == direction && setup->entrances[i].alternateDirection == raw.flag_b6, "production loader direction");
      }
    }
    sim::World world(*setup);
    int seen = 0;
    const int target = std::min(6,setup->totalLemmings);
    for (int tick=0;tick<10000 && seen<target;++tick) {
      world.tick();
      while (seen < world.spawned() && seen < target) {
        const auto hatch = std::size_t(seen) % directions.size();
        const int expected = before ? 1 : directions[hatch] == -1 ? -1 : 1;
        check(int(world.lemmings()[seen].facing) == expected, file + " real-data birth " + std::to_string(seen));
        if (level.gadgets.entities_425e50[hatch].flag_b6)
          directions[hatch] = directions[hatch] == 1 ? -1 : 1;
        ++seen;
      }
    }
    check(seen == target, file + " releases up to six without modifying timing");
    ++levels; hatches += int(projected.size());
  }
  check(levels==102 && hatches==144, "complete effective installed corpus");
  std::ofstream summary(out/"census.txt");
  summary << "levels=" << levels << "\nhatches=" << hatches << '\n';
  for (const auto& [dir,count] : fixed) summary << "fixed_" << dir << '=' << count << '\n';
  for (const auto& [dir,count] : alternating) summary << "alternating_" << dir << '=' << count << '\n';
  return failures ? 1 : 0;
}

void boundaries() {
  // Supplement real-data cases with noncanonical direction shorts and independent
  // alternation phases on multiple hatches; do not change scheduling or rates.
  for (int direction : {-1,0,1,2}) for (bool alternate : {false,true}) {
    sim::LevelSetup setup;
    setup.terrain = sim::Terrain(100,100);
    setup.totalLemmings=6; setup.releaseIntervalTicks=1;
    sim::Entrance e; e.x=40; e.y=10; e.direction=direction; e.alternateDirection=alternate;
    setup.entrances.push_back(e);
    sim::World world(setup);
    for (int i=0;i<6;++i) {
      world.tick();
      check(world.spawned()==i+1,"one integer-model release per tick");
      check(int(world.lemmings()[i].facing)==(before ? 1 : direction==-1 ? -1 : 1),"exact short comparison and post-birth toggle");
      if (alternate) direction=direction==1 ? -1 : 1;
    }
  }
  sim::LevelSetup setup; setup.terrain=sim::Terrain(100,100);
  setup.totalLemmings=6; setup.releaseIntervalTicks=1;
  sim::Entrance a,b; a.x=20;a.y=10;a.direction=-1;a.alternateDirection=true;
  b.x=70;b.y=10;b.direction=1;b.alternateDirection=true; setup.entrances={a,b};
  sim::World world(setup), fresh(setup);
  const int expected[]{-1,1,1,-1,-1,1};
  for(int i=0;i<6;++i) { world.tick();fresh.tick();
    check(int(world.lemmings()[i].facing)==(before ? 1 : expected[i]),"independent hatch alternation");
    check(world.lemmings()[i].facing==fresh.lemmings()[i].facing,"new World resets hatch phases");
  }
}

int run(const fs::path& game, AssetLibrary& lib, const std::string& id, const fs::path& out) {
  fs::create_directories(out);
  const auto setup=sim::loadRetailLevel(lib,id); check(bool(setup),"demo level loads"); if(!setup)return 1;
  const auto file=sim::resolveRetailLevel(lib,id); if(!file)return 2;
  const auto data=lib.read(file->file); if(!data)return 2;
  const auto level=lemmings::read_compiled_level({reinterpret_cast<const uint8_t*>(data->data()),data->size()},false);
  std::vector<int> directions; for(const auto& e:level.gadgets.entities_425e50) directions.push_back(std::bit_cast<int16_t>(e.short_b4));
  int seen=0;
  const auto camera=out/"camera.json";
  std::ofstream(camera) << "{\"viewport\":[640,480],\"yaw_units\":" << setup->entrances[0].x
    << ",\"level_eye_height\":" << level.gadgets.entities_425e50[0].shorts_98_9a_9c[2]+8 << "}";
  std::ofstream trace(out/"births.csv");
  trace << "birth,level_clock_ms,tick,update,hatch,x,y,facing,state,sheet_type\n";
  auto observer=[&](const app::LevelFrameView& frame) {
    if (frame.world.spawned()<=seen) return;
    const auto& f=app::inspectLevelFrame(frame);
    while(seen<f.world.spawned() && seen<6) {
      const auto& l=f.world.lemmings()[seen]; const auto hatch=std::size_t(seen)%directions.size();
      const int expected=before ? 1 : directions[hatch]==-1 ? -1 : 1;
      check(int(l.facing)==expected,id+" application birth "+std::to_string(seen+1));
      // Literal acceptance routes independently pin the installed patterns.
      const int routeFace=id=="MM_W_012" ? (seen%2 ? -1:1) : id=="MM_BASH_002" ? -1:1;
      check(int(l.facing)==(before ? 1:routeFace),id+" pinned birth pattern");
      trace << seen+1 << ',' << f.world.levelClockMs() << ',' << f.world.tickCount() << ','
        << f.world.explicitUpdateCount() << ',' << hatch << ',' << l.x << ',' << l.y << ',' << int(l.facing)
        << ',' << sim::stateName(l.state) << ',' << l.sheetType << '\n';
      check(writePng(out/("birth-"+std::to_string(seen+1)+".png"),f.image).has_value(),"retain actual production birth image");
      if(level.gadgets.entities_425e50[hatch].flag_b6)directions[hatch]=directions[hatch]==1 ? -1 : 1;
      ++seen;
    }
    if(seen==6)quit();
  };
  std::vector<std::string> args{"hatch-direction","--level",id,"--sim","--native","--frames","4000","--game-dir",game.string(),"--camera",camera.string()};
  std::vector<char*> raw;for(auto& s:args)raw.push_back(s.data());
  app::ApplicationRunOptions options;options.frameSeconds=0.02;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  app::ScopedFrameConsumption consumption;
  check(app::runApplication(int(raw.size()),raw.data(),{},observer,options)==0,"production application returns");
  check(seen==6,id+" six actual observed births");
  return failures ? 1:0;
}

int profileFrame(const fs::path& game, const fs::path& out) {
  // Reproduce the existing performance demonstration with identical CLI inputs.
  // Both builds share presentation; only the authenticated release/loader units
  // differ. This independently explains the changed MM_W_012 regression image.
  std::vector<std::string> args{"hatch-direction-profile","--direct-level",
    "--game-dir",game.string(),"--level","MM_W_012","--native",
    "--sim-ticks","50","--frames","150","--no-audio",
    "--window-shot",(out/"frame.png").string()};
  std::vector<char*> raw; for(auto& s:args)raw.push_back(s.data());
  app::ApplicationRunOptions options;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  check(app::runApplication(int(raw.size()),raw.data(),{},{},options)==0,"production profile-frame application returns");
  check(fs::exists(out/"frame.png"),"profile frame retained");
  return failures ? 1:0;
}
}
int main(int argc,char** argv) {
  if(argc<3 || !std::getenv("WEBOLUTION_GAME_DIR"))return 2;
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR")),out(argv[2]);fs::create_directories(out);
  const auto local=save::localAppDataDirectory();if(!local)return 2;
  SaveSnapshot installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;
  if(std::string(argv[1])=="assets") { census(*lib,out);boundaries(); }
  else if(std::string(argv[1])=="profile")profileFrame(game,out);
  else if(argc==4)run(game,*lib,argv[3],out);
  else return 2;
  check(installed.unchanged() && normal.unchanged(),"installed and normal user saves unchanged");
  std::ofstream(out/"checks.txt") << "checks=" << checks << "\nfailures=" << failures << '\n';
  std::cout << checks << " checks, " << failures << " failures\n";
  return failures ? 1:0;
}
