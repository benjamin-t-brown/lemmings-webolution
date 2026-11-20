// Check presentation lifetime, live-prefix indexing and independent exit clocks with synthetic contacts.
#include "app/LemmingAnim.h"
#include "app/LemmingMotion.h"
#include "app/BackdropScene.h"
#include "app/RuntimeHud.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#include <cmath>
#include <filesystem>
#include <iostream>
#include <string_view>

using namespace webo;
namespace {
int failures = 0;
void check(bool ok, const char* why) {
  if (!ok) { ++failures; std::cerr << "FAIL: " << why << '\n'; }
}
bool near(double a, double b) { return std::abs(a - b) < 1e-9; }
}
int main(int argc, char** argv) {
  if (argc != 2) return 2;
  const std::filesystem::path game = argv[1];
  auto library = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!library) { std::cerr << library.error() << '\n'; return 1; }
  std::array<SpriteSheet, 3> sheets;
  const char* names[]{"SPRITES\\LEMMINGSHI.SPR", "SPRITES\\LEMMINGSWATERHI.SPR",
                      "SPRITES\\LEMMINGSACIDHI.SPR"};
  for (int type = 0; type < 3; ++type) {
    auto sheet = loadSpriteSheet(*library, names[type]);
    if (!sheet) { std::cerr << sheet.error() << '\n'; return 1; }
    sheets[type] = std::move(*sheet);
    std::cout << names[type] << " clips:";
    for (const auto& clip : sheets[type].sprites) std::cout << ' ' << clip.name;
    std::cout << '\n';
    const auto* fall = sheets[type].find("fall");
    check(fall && !fall->animations.empty(), "each installed type has exit fall artwork");
    if (fall) {
      std::cout << names[type] << " fall entries=" << fall->animations.size() << " rate=" << fall->rate << '\n';
      for (std::size_t t = 0; t < fall->animations.size(); ++t) {
        const auto& frames = fall->animations[t].frames;
        check(!frames.empty(), "every exit temporal entry has directional artwork");
        std::cout << "  temporal=" << t << " directions=" << frames.size();
        for (const auto& frame : frames)
          std::cout << " [" << frame.sheet << ' ' << frame.left << ',' << frame.top << ':' << frame.right << ',' << frame.bottom << ']';
        std::cout << '\n';
      }
    }
  }
  const std::array<const SpriteSheet*, 3> sheetRefs{&sheets[0], &sheets[1], &sheets[2]};
  sim::LevelSetup setup;
  setup.terrain = sim::Terrain(256, 112);
  setup.totalLemmings = 0;
  setup.saveTarget = 1;
  setup.exits.push_back({10, 40, 2, 3});
  sim::World world(setup);
  // Exit / live / exit / exit in World order: render tails must not shift the
  // one live actor's screen-box index when early actors become saved.
  sim::Lemming l;
  l.x = 11; l.y = 41; l.state = sim::State::Walker;
  world.place(l);
  l.x = 90; l.state = sim::State::Blocker;
  world.place(l);
  l.x = 11; l.state = sim::State::Walker; l.sheetType = 1; l.verticalDir = -1;
  world.place(l);
  l.sheetType = 2; l.verticalDir = 1;
  world.place(l);
  app::LemmingMotionTracker motion;
  app::LemmingAnimTracker animation;
  motion.observe(world);
  world.tick();
  motion.observe(world);
  animation.advance(world, sheetRefs, 0.0);
  check(world.saved() == 3 && world.activeCount() == 1, "save accounting stays immediate");
  check(world.terrain().blockerFieldCells() == 0, "exit contacts create no blocking field");
  check(world.takeCues().size() == 3, "one cue per immediate save");
  for (std::size_t i : {0u, 2u, 3u}) {
    check(!world.lemmings()[i].alive(), "exiting actors stay inactive");
    check(!world.assign(i, sim::Skill::Climber), "exiting actors refuse skills");
    check(near(motion.exitSeconds(i, world), 0), "close arrivals start individual clocks");
  }
  auto draw = app::injectedWalkersFromWorld(world, 112, &animation, &motion);
  check(draw.size() == 4 && near(draw[0].plus8X, 90), "all live actors precede exit tails");
  for (std::size_t i = 1; i < draw.size(); ++i)
    check(std::string_view(draw[i].clip) == "fall" && near(draw[i].rotationPhase, .25), "exit selects fall and quarter-turn view phase");
  check(draw[2].sheetType == 1 && draw[2].verticalDir == -1 && draw[3].sheetType == 2,
      "water, acid and inverted drawing use existing type/gravity adapters");
  for (int i = 0; i < 5; ++i) {
    world.advanceElapsedMs(50);
    motion.observe(world);
    animation.advance(world, sheetRefs, .05);
  }
  const auto upright = motion.positionOf(0, world), inverted = motion.positionOf(2, world);
  check(upright.z > .75 && upright.z < 1.5 && upright.y < 41,
      "upright actor jumps upward and outward toward basket");
  check(inverted.y > 41 && near(inverted.z, upright.z), "inverted actor jumps downward and outward");
  const auto entry = animation.poseOf(0).temporalIndex;
  check(entry > 0, "saved actor advances its own fall-strip clock");
  motion.observe(world);
  animation.advance(world, sheetRefs, 0);
  check(near(motion.positionOf(0, world).y, upright.y) && entry == animation.poseOf(0).temporalIndex,
      "zero elapsed/pause freezes motion and frames");
  // A later arrival must survive the first group's expiry.
  l.sheetType = 0; l.x = 11; l.y = 41;
  world.place(l);
  motion.observe(world);
  world.tick();
  motion.observe(world);
  animation.advance(world, sheetRefs, .2);
  for (int i = 0; i < 11; ++i) { world.advanceElapsedMs(50); motion.observe(world); }
  draw = app::injectedWalkersFromWorld(world, 112, &animation, &motion);
  check(motion.exitSeconds(0, world) < 0 && motion.exitSeconds(4, world) >= 0 && draw.size() == 2,
      "first group disappears at exactly one second without truncating later arrival");
  world.advanceElapsedMs(450); motion.observe(world);
  draw = app::injectedWalkersFromWorld(world, 112, &animation, &motion);
  check(draw.size() == 1 && world.saved() == 4 && world.takeCues().size() == 1,
      "tail expiry creates no extra saves or cues");
  check(app::injectedWalkersFromWorld(world, 112).size() == 1, "diagnostic without tracker retains live-only behavior");
  // Use installed exit models and synthetic arrivals to check that the
  // exit-tail adapter follows the basket's bob.
  const auto levelNames = levelFileNames(*library);
  for (const char* id : {"MM_CLIMB_002", "MM_W_012", "M06_KF_1"}) {
    std::string name;
    for (const auto& n : levelNames) if (n.find(std::string(id) + ".LVL") != std::string::npos) name = n;
    auto bytes = library->read(name);
    check(bool(bytes), "installed bob level exists");
    if (!bytes) continue;
    auto backdrop = app::BackdropScene::build(*library, *bytes, name);
    check(bool(backdrop), "installed bob scene loads");
    if (!backdrop) continue;
    const auto presentation = backdrop->entranceExitPresentation(*bytes);
    check(presentation && !presentation->exits.empty(), "installed exit is bound");
    if (!presentation || presentation->exits.empty()) continue;
    const auto matrices = [&] {
      std::vector<std::array<double, 12>> out;
      backdrop->visitRoots([&](const auto& source, const auto& node) {
        if (source.owner == "entities_426c6b") out.push_back(node.matrix);
      });
      return out;
    };
    backdrop->setEntranceExitPresentation(*presentation, 1, 0, 0);
    const auto base = matrices();
    for (const auto& [time, offset] : {std::pair{0.5, 0.5}, {1.0, 0.0}, {1.5, -0.5}, {2.0, 0.0}}) {
      backdrop->setEntranceExitPresentation(*presentation, 1, 0, time);
      const auto bobbed = matrices();
      for (std::size_t e = 0; e < base.size(); ++e) {
        check(std::abs(bobbed[e][10] - base[e][10] - offset) < 1e-6,
            "whole basket/model has half-unit bob and two-second period");
        check(std::abs(bobbed[e][9] - base[e][9]) < 1e-6 &&
              std::abs(bobbed[e][11] - base[e][11]) < 1e-6,
            "idle bob preserves horizontal placement");
      }
    }
    auto installed = sim::loadRetailLevel(*library, id);
    check(installed && !installed->exits.empty(), "installed simulation trigger loads");
    if (!installed || installed->exits.empty()) continue;
    installed->totalLemmings = 0;
    const auto trigger = installed->exits.front();
    sim::World arrivals(*installed);
    sim::Lemming a;
    a.x = trigger.x + trigger.width / 2; a.y = trigger.y;
    a.state = sim::State::Walker;
    arrivals.place(a);
    a.verticalDir = -1; arrivals.place(a);
    app::LemmingMotionTracker tails;
    app::RuntimeSimState clock;
    tails.observe(arrivals);
    const auto observe = [&] { tails.observe(arrivals); };
    // Save and cue at exit recognition. Jump and bob are presentation only.
    arrivals.tick(); observe();
    check(arrivals.saved() == 2 && arrivals.takeCues().size() == 2,
        "bobbing does not defer saves or cues until jump completion");
    for (int step = 0; step < 99; ++step) {
      clock.advanceElapsed(arrivals, .01, false, observe);
      const double age = tails.exitSeconds(0, arrivals);
      const double dy = trigger.y - 8.0 - a.y, dz = 1.5 - .75;
      const double dx = trigger.x + trigger.width * .5 - a.x;
      const double lift = std::sqrt(dx*dx + dy*dy + dz*dz)*age - 12.5*age*age;
      backdrop->setEntranceExitPresentation(*presentation, 1, 0, arrivals.levelClockMs()/1000.0);
      const double modelBob = matrices().front()[10] - base.front()[10];
      for (std::size_t actor : {0u, 1u}) {
        const auto p = tails.positionOf(actor, arrivals);
        const int gravity = arrivals.lemmings()[actor].verticalDir;
        check(std::abs(p.y - (a.y - gravity*lift + dy*.01) + modelBob*age) < 1e-6,
            "upright/inverted tails converge on the same displayed basket bob");
      }
    }
    const auto held = tails.positionOf(0, arrivals);
    const auto heldMatrix = matrices();
    clock.togglePause();
    clock.advanceElapsed(arrivals, 5, false, observe);
    backdrop->setEntranceExitPresentation(*presentation, 1, 0, arrivals.levelClockMs()/1000.0);
    check(near(tails.positionOf(0, arrivals).y, held.y) && matrices() == heldMatrix,
        "pause freezes basket and jump on the same clock");
    clock.resume(); clock.toggleFastForward();
    const auto was = arrivals.levelClockMs();
    clock.advanceElapsed(arrivals, .01, false, observe);
    check(arrivals.levelClockMs() == was + 10*app::RuntimeSimState::kFastForwardPasses &&
          tails.exitSeconds(0, arrivals) < 0 && arrivals.saved() == 2 && arrivals.takeCues().empty(),
        "fast-forward advances presentation expiry with no extra save/cue");
    check(arrivals.setup().exits.front().x == trigger.x && arrivals.setup().exits.front().y == trigger.y,
        "all render samples leave the simulation trigger fixed");
    // Departure starts at the sampled bob position.
    backdrop->setEntranceExitPresentation(*presentation, 1, 0, .5);
    const auto beforeDeparture = matrices();
    backdrop->setEntranceExitPresentation(*presentation, 1, 1e-8, .50000001);
    const auto departing = matrices();
    for (std::size_t e = 0; e < departing.size(); ++e)
      for (int c = 9; c < 12; ++c)
        check(std::abs(beforeDeparture[e][c] - departing[e][c]) < 1e-5,
            "departure starts continuously from the displayed bob");
    std::cout << id << ": bob, jump alignment, pause, fast-forward, fixed trigger PASS\n";
  }
  std::cout << (failures ? "FAIL" : "PASS") << ": exit jump lifetime, indexing, sheets, gravity and accounting\n";
  return failures ? 1 : 0;
}
