// Small legacy checks for presentation lifetime, live-prefix indexing and
// independent exit clocks. Synthetic contacts, not a gameplay/replay route.
#include "app/LemmingAnim.h"
#include "app/LemmingMotion.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
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
  std::cout << (failures ? "FAIL" : "PASS") << ": exit jump lifetime, indexing, sheets, gravity and accounting\n";
  return failures ? 1 : 0;
}
