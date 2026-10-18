// Progress unlock chain and serialize round-trip.

#include "assets/GameData.h"
#include "shell/Progress.h"

#include <cmath>
#include <filesystem>
#include <format>
#include <iostream>

using namespace webo;
using namespace webo::shell;

namespace {

int fails = 0;
int checks = 0;

void check(bool cond, std::string_view msg) {
  ++checks;
  if (!cond) {
    ++fails;
    std::cout << "  FAIL  " << msg << "\n";
  }
}

LevelTable toyTable() {
  LevelTable t;
  t.groups.resize(2);
  t.groups[0] = {{"A1", 1, 0, 0}, {"blank", 1, 0, 0}, {"A3", 1, 0, 0}};
  t.groups[1] = {{"B1", 1, 0, 0}};
  return t;
}

}  // namespace

int main() {
  std::cout << "progress unlock\n";
  {
    const LevelTable t = toyTable();
    check(firstPlayableSlot(t, 0) == 0, "first playable is slot 0");
    int ng = -1, ns = -1;
    check(nextPlayable(t, 0, 0, ng, ns) && ng == 0 && ns == 2, "skips blank to A3");
    check(nextPlayable(t, 0, 2, ng, ns) && ng == 1 && ns == 0, "wraps to next group");

    Progress p;
    p.newGame(t);
    check(p.state(0, 0) == SlotState::Unlocked, "new game unlocks first");
    check(p.state(0, 2) == SlotState::Locked, "later still locked");
    p.complete(t, 0, 0);
    check(p.state(0, 0) == SlotState::Completed, "completed");
    check(p.state(0, 2) == SlotState::Unlocked, "unlocks next non-blank");
    p.complete(t, 0, 2);
    check(p.state(1, 0) == SlotState::Unlocked, "unlocks next group");
  }

  std::cout << "progress serialize\n";
  {
    const LevelTable t = toyTable();
    Progress p;
    p.newGame(t);
    p.language = "UK";
    p.musicVolume = 0.4f;
    p.sfxVolume = 0.8f;
    p.complete(t, 0, 0);
    const auto path = std::filesystem::temp_directory_path() / "webo_progress_test.dat";
    auto saved = saveProgress(p, path);
    check(saved.has_value(), "save ok");
    auto loaded = loadProgress(path);
    check(loaded.has_value(), "load ok");
    if (loaded) {
      check(loaded->language == "UK", "language");
      check(std::abs(loaded->musicVolume - 0.4f) < 0.01f, "music vol");
      check(loaded->state(0, 0) == SlotState::Completed, "slot0");
      check(loaded->state(0, 2) == SlotState::Unlocked, "slot2");
    }
    std::filesystem::remove(path);
  }

  std::cout << std::format("{} checks, {} failures\n", checks, fails);
  return fails == 0 ? 0 : 1;
}
