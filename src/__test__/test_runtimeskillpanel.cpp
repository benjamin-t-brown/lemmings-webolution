#include "app/RuntimeSkillPanel.h"
#include "assets/Box.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace {
void check(bool ok, const char* what) {
  if (!ok) { std::fprintf(stderr, "FAIL: %s\n", what); std::exit(1); }
}
}

int main() {
  using namespace webo;
  using namespace webo::app;
  using namespace webo::sim;

  RuntimeSkillSelection selection;
  constexpr Skill mapping[] = {Skill::Climber, Skill::Floater, Skill::Bomber,
      Skill::Blocker, Skill::Builder, Skill::Basher, Skill::Miner, Skill::Digger};
  check(selection.selectedIndex() == 0 && selection.selectedSkill() == Skill::Climber, "default selection");
  for (int i = 0; i < 8; ++i) {
    check(selection.selectIndex(i), "valid index selected");
    check(selection.selectedIndex() == i && selection.selectedSkill() == mapping[i], "explicit skill mapping");
  }
  check(!selection.selectIndex(-1) && selection.selectedIndex() == 7, "negative rejected without mutation");
  check(!selection.selectIndex(8) && selection.selectedIndex() == 7, "index eight rejected without mutation");
  selection.selectIndex(0);
  constexpr std::array<int, 9> available = {1, 1, 1, 1, 1, 1, 1, 1, 0};
  constexpr std::array<int, 9> exhausted = {1, 0, 1, 1, 1, 1, 1, 1, 0};
  check(!selection.selectAvailableIndex(1, exhausted) && selection.selectedIndex() == 0,
        "zero-stock keyboard-style selection rejected without mutation");
  auto negativeStock = available;
  negativeStock[1] = -1;
  check(!selection.selectAvailableIndex(1, negativeStock) && selection.selectedIndex() == 0,
        "negative-stock keyboard-style selection rejected without mutation");
  check(selection.selectAvailableIndex(1, available) && selection.selectedIndex() == 1,
        "positive-stock keyboard-style selection accepted");
  selection.selectIndex(0);
  check(!selection.consumePanelClick(17, 12, available), "left outside");
  check(!selection.consumePanelClick(90, 12, available), "right edge outside");
  check(!selection.consumePanelClick(18, 11, available), "top outside");
  check(selection.consumePanelClick(18, 12, available) && selection.selectedIndex() == 0, "plate zero inclusive origin");
  check(selection.consumePanelClick(89, 43, available), "plate zero last pixel");
  check(!selection.consumePanelClick(40, 44, available), "vertical gap is outside");
  selection.selectIndex(0);
  check(selection.consumePanelClick(50, 52, exhausted) && selection.selectedIndex() == 0,
        "exhausted skill plate consumes click without changing selection");
  for (int i = 0; i < 8; ++i) {
    check(selection.consumePanelClick(50, 12 + 40 * i, available), "each skill plate consumes click");
    check(selection.selectedIndex() == i, "each skill plate selects matching index");
  }
  selection.selectIndex(5);
  check(selection.consumePanelClick(50, 12 + 40 * 8, available) && selection.selectedIndex() == 5, "fast-forward plate consumed, selection unchanged");
  check(selection.consumePanelClick(50, 12 + 40 * 9, available) && selection.selectedIndex() == 5, "Armageddon plate consumed, selection unchanged");
  check(!selection.consumePanelClick(50, 404, available), "after final plate outside");

  LevelSetup setup;
  setup.terrain = Terrain(256, 96);
  setup.terrain.fillBox(0, 90, 256, 6, Material::Earth);
  setup.entrances.push_back({100, 10});
  setup.totalLemmings = 1;
  setup.releaseIntervalTicks = 1;
  setup.skills = {1, 2, 3, 4, 5, 6, 7, 8};
  World world(setup);
  world.tick();
  auto before = runtimeSkillPanelCounts(world);
  check(before[0] == 1 && before[7] == 8 && before[8] == 0, "runtime counts mirror skills; ninth is zero");
  check(world.assign(0, Skill::Floater), "successful assignment");
  auto after = runtimeSkillPanelCounts(world);
  check(after[1] == before[1] - 1, "successful assignment decrements rendered count");
  check(!world.assign(99, Skill::Climber), "invalid actor assignment fails");
  check(runtimeSkillPanelCounts(world) == after, "failed assignment leaves rendered counts unchanged");
  selection.selectIndex(1);
  World depletedWorld(setup);
  depletedWorld.tick();
  check(depletedWorld.assign(0, Skill::Climber), "consume final climber stock");
  const auto depletedCounts = runtimeSkillPanelCounts(depletedWorld);
  check(depletedCounts[0] == 0, "assignment reduces skill stock to zero");
  check(!selection.selectAvailableIndex(0, depletedCounts) && selection.selectedIndex() == 1,
        "assignment-depleted skill cannot be reselected");

  LevelSetup extremes;
  extremes.skills = {-1, -4, 0, 1, 98, 99, 100, 500};
  World countWorld(extremes);
  const auto capped = runtimeSkillPanelCounts(countWorld);
  check((capped == std::array<int, 9>{0, 0, 0, 1, 98, 99, 99, 99, 0}), "counts clamp nonpositive and cap at 99");

  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (dir && *dir && std::filesystem::exists(std::filesystem::path(dir) / "lemmings.box")) {
    auto library = AssetLibrary::open(std::filesystem::path(dir) / "lemmings.box", std::filesystem::path(dir) / "patch.box");
    check(bool(library), "open retail archive");
    if (library) {
      auto panel = RetailSkillPanel::create(*library, "MM_CLIMB_002");
      check(bool(panel), "create cached retail panel");
      if (panel) {
        auto blank = [] { Image image; image.width = 640; image.height = 480; image.rgba.assign(640u * 480u * 4u, 0); return image; };
        Image a = blank(), b = blank(), repeat = blank();
        selection.selectIndex(0);
        drawRuntimeSkillPanel(a, *panel, world, selection);
        drawRuntimeSkillPanel(repeat, *panel, world, selection);
        check(a.rgba == repeat.rgba, "repeated rendering is stable and does not advance simulation");
        selection.selectIndex(1);
        drawRuntimeSkillPanel(b, *panel, world, selection);
        check(a.rgba != b.rgba, "cached retail panel pixels reflect runtime selection");
        selection.selectIndex(0);
        check(world.assign(0, Skill::Climber), "consume final climber stock");
        Image depleted = blank();
        drawRuntimeSkillPanel(depleted, *panel, world, selection);
        bool changed = false;
        for (int y = 0; y < 480; ++y) for (int x = 0; x < 640; ++x) {
          const auto offset = static_cast<std::size_t>(y * 640 + x) * 4;
          for (int channel = 0; channel < 3; ++channel) {
            const bool differs = a.rgba[offset + channel] != depleted.rgba[offset + channel];
            if (x >= 66 && x < 78 && y >= 20 && y < 34) changed |= differs;
            else check(!differs, "stock depletion only changes its own count glyphs");
          }
        }
        check(changed, "runtime draw reflects assignment and removes zero-count glyphs");
        check(!world.assign(0, Skill::Climber), "repeated assignment fails");
        Image failed = blank();
        drawRuntimeSkillPanel(failed, *panel, world, selection);
        check(failed.rgba == depleted.rgba, "failed assignment leaves panel pixels unchanged");
      }
    }
  } else {
    std::cout << "runtime skill panel retail pixels: SKIPPED (WEBOLUTION_GAME_DIR unset)\n";
  }
  std::cout << "runtime skill panel: passed\n";
}
