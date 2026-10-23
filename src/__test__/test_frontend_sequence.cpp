// Frontend screen sequence replay against the b2e live capture.

#include "shell/Frontend.h"

#include <array>
#include <iostream>
#include <string_view>

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

// Source: build/tmp/b2e-live-capture/report.json, steps[].counter / ui_state / nodes /
// vtables (OBSERVED, one run). Vtable 0 and nodes 0 mean an empty list head in a level.
struct Row {
  const char* step;
  int counter;
  int uiState;
  int nodes;
  std::uint32_t vtable;
};
constexpr std::array<Row, 7> kB2e = {{
    {"baseline", 3, 3, 1, 0x4B2528},
    {"menu-highlight-0", 3, 3, 1, 0x4B2528},
    {"menu-RETURN", 4, 3, 1, 0x4B3F38},
    {"confirm-RETURN", 5, 3, 1, 0x4B3EB8},
    {"map-node-click", 5, 0, 0, 0},
    {"pause-P", 5, 1, 0, 0},
    {"pause-short-ESC", 5, 0, 0, 0},
}};

void checkRow(const Frontend& f, const Row& r) {
  check(f.counter() == r.counter, std::string(r.step) + ": counter");
  check(f.uiState() == r.uiState, std::string(r.step) + ": ui_state");
  check(f.nodeCount() == r.nodes, std::string(r.step) + ": node count");
  check(f.nodeVtable() == r.vtable, std::string(r.step) + ": node vtable");
}

}  // namespace

int main() {
  std::cout << "frontend sequence: b2e replay\n";
  {
    // LeavePause is the policy that matches the one observed short-ESC sample.
    Frontend f({ShortEscPolicy::LeavePause, LevelExitPolicy::Unobserved});
    checkRow(f, kB2e[0]);
    f.setHighlight(0);
    checkRow(f, kB2e[1]);
    check(f.press(Key::Return) == Outcome::Handled, "menu RETURN handled");
    checkRow(f, kB2e[2]);
    check(f.press(Key::Return) == Outcome::Handled, "confirm RETURN handled");
    checkRow(f, kB2e[3]);
    check(f.press(Key::MapNodeClick) == Outcome::Handled, "map click handled");
    checkRow(f, kB2e[4]);
    check(f.press(Key::P) == Outcome::Handled, "P handled");
    checkRow(f, kB2e[5]);
    check(f.press(Key::EscShort) == Outcome::Handled, "short ESC handled (LeavePause)");
    checkRow(f, kB2e[6]);
  }

  std::cout << "frontend sequence: policy and quirks\n";
  {
    // Default follows the preserved quirk; this row is a policy, not a measurement.
    Frontend f;
    f.press(Key::Return);
    f.press(Key::Return);
    f.press(Key::MapNodeClick);
    f.press(Key::P);
    check(f.press(Key::EscShort) == Outcome::Ignored, "default: short ESC refused");
    check(f.paused() && f.uiState() == kUiPaused, "default: still paused");
    check(f.press(Key::EscLong) == Outcome::Unobserved, "long ESC unobserved");
    check(f.paused() && f.counter() == 5, "long ESC leaves state unchanged");
    check(f.leaveLevel() == Outcome::Unobserved && f.screen() == Screen::Level,
          "default: level exit is a reported no-op");
  }
  {
    Frontend f({ShortEscPolicy::Refuse, LevelExitPolicy::ToMap});
    f.press(Key::Return);
    f.press(Key::Return);
    f.press(Key::MapNodeClick);
    check(f.leaveLevel() == Outcome::Handled && f.screen() == Screen::Map, "ToMap: back on map");
    check(f.counter() == 6, "ToMap: new map construction (hypothesis)");
  }
  {
    // Preserved quirk: RETURN acts only at highlight exactly 0.
    Frontend f;
    f.setHighlight(1);
    check(f.press(Key::Return) == Outcome::Ignored, "RETURN at highlight 1 ignored");
    check(f.screen() == Screen::Menu && f.counter() == 3, "menu unchanged");
    f.setHighlight(0);
    check(f.press(Key::Return) == Outcome::Handled && f.screen() == Screen::Confirm,
          "RETURN at highlight 0 acts");
  }

  std::cout << checks << " checks, " << fails << " failures\n";
  return fails == 0 ? 0 : 1;
}
