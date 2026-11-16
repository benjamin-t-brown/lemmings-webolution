// Short invariants for stored limits, table selection and hold cancellation.
#include "app/RuntimeSkillPanel.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "sim/RetailLevel.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <set>

using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool ok, const char* message) {
  if (!ok) { ++failures; std::cerr << "FAIL " << message << '\n'; }
}
void units(const fs::path& game) {
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");check(bool(lib),"installed archive opens");if(!lib)return;
  auto rates=loadReleaseRateTable(*lib);check(bool(rates),"installed interval table opens");if(!rates)return;
  int levels=0,hatches=0;std::set<std::array<int,3>> values;
  for(const auto& file:levelFileNames(*lib)) {
    auto blob=lib->read(file);check(bool(blob),"installed LVL readable");if(!blob)continue;
    auto records=readHatchRecords(*blob);check(!records.empty(),"structured installed hatch records");
    const auto start=file.find_last_of("\\/")+1;
    auto id=file.substr(start,file.size()-start-4);
    for(const auto& h:records) {
      check(h.rateMin>=1&&h.rateMax<=99&&h.rate>=h.rateMin&&h.rate<=h.rateMax,"stored initial and limits valid");++hatches;
    }
    auto control=app::RuntimeReleaseRateControl::create(*lib,id);check(bool(control),"production control loads installed primary hatch");
    if(control&&!records.empty()) {const auto& h=records.front();
      check(control->current()==h.rate&&control->minimum()==h.rateMin&&control->maximum()==h.rateMax,"control uses stored initial/minimum/maximum");
      values.insert({h.rate,h.rateMin,h.rateMax});
    }
    ++levels;
  }
  check(levels==102&&hatches==144&&values.size()>=3,"entire installed corpus measured, varying rate limits");
  auto setup=sim::loadRetailLevel(*lib,"MM_CLIMB_002");check(bool(setup),"installed test route loads");if(!setup)return;
  sim::World a(*setup),b(*setup);auto control=app::RuntimeReleaseRateControl::create(*lib,"MM_CLIMB_002");
  check(bool(control),"installed control loads");if(!control)return;
  check(a.setup().releaseIntervalSeconds==rates->secondsPerLemming(20),"accepted initial mapping preserved");
  check(!control->press(17,440,3,a)&&!control->press(90,440,3,a)&&!control->press(54,468,3,a),"box hit boundaries exclude outside");
  check(!control->press(54,440,2,a),"middle button is inert");
  a.advanceElapsedMs(1000);b.advanceElapsedMs(1000);
  control->press(54,440,3,a);control->release(3);
  check(control->current()==21&&a.setup().releaseIntervalSeconds==rates->secondsPerLemming(21),"right click increments exactly one table row");
  for(int ms=0;ms<1500 && b.spawned()==0;++ms) {
    a.advanceElapsedMs(1);b.advanceElapsedMs(1);
    check(a.spawned()==b.spawned(),"rate edit preserves first release countdown");
  }
  check(b.spawned()==1,"bounded first release reached");
  control->press(54,440,1,a);control->release(1);check(control->current()==20,"left click decrements exactly one row");
  control->press(54,440,3,a);control->advance(0.399,54,440,a);check(control->current()==21,"repeat delay before 400ms");
  control->advance(0.001,54,440,a);check(control->current()==22,"hold repeats after 400ms");
  control->advance(0.1,54,440,a);check(control->current()==23,"100ms subsequent repeat");
  control->reset();control->advance(5,54,440,a);check(control->current()==23,"focus/menu cancellation stops repeat");
  control->press(54,440,3,a);control->advance(1,90,440,a);control->advance(5,54,440,a);
  check(control->current()==24,"leaving box cancels held gesture");
  control->press(54,440,3,a);control->advance(10,54,440,a);control->release(3);
  check(control->current()==50&&a.setup().releaseIntervalSeconds==rates->secondsPerLemming(50),"hold clamps at stored maximum");
  control->press(54,440,1,a);control->advance(10,54,440,a);control->release(1);
  check(control->current()==1&&a.setup().releaseIntervalSeconds==rates->secondsPerLemming(1),"hold clamps at stored minimum");
  auto fresh=app::RuntimeReleaseRateControl::create(*lib,"MM_CLIMB_002");check(fresh&&fresh->current()==20,"fresh level resets initial rate");
  sim::World idle(*setup),baseline(*setup);
  if(fresh) for(int i=0;i<2000;++i) {
    fresh->advance(0.01,54,440,idle);
    idle.advanceElapsedMs(10);baseline.advanceElapsedMs(10);
    check(idle.spawned()==baseline.spawned() &&
          idle.levelClockMs()==baseline.levelClockMs() &&
          idle.setup().releaseIntervalSeconds==baseline.setup().releaseIntervalSeconds,
          "idle control preserves release schedule and clock");
  }
  auto fixed=app::RuntimeReleaseRateControl::create(*lib,"E10_KF_1");
  check(fixed&&fixed->current()==1&&fixed->minimum()==1&&fixed->maximum()==1,
        "fixed-rate level retains authored range");
  if(fixed) {
    const double interval=idle.setup().releaseIntervalSeconds;
    for(int button:{1,3}) {
      fixed->press(54,440,button,idle);fixed->advance(10,54,440,idle);fixed->release(button);
    }
    check(fixed->current()==1&&idle.setup().releaseIntervalSeconds==interval,
          "fixed range consumes input without changing the interval");
  }
}
}
int main() {
  const auto game = std::getenv("WEBOLUTION_GAME_DIR");
  if (!game) return 2;
  units(fs::path(game));
  return failures ? 1 : 0;
}
