// Frontend screen sequence replay against the b2e and b2h live captures.

#include "shell/Frontend.h"

#include <array>
#include <iostream>
#include <string>
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
    {"pause-ESC-hold-25", 5, 0, 0, 0},
}};

// Source: build/tmp/b2h-live-esc-sweep/report.json (OBSERVED, one run). ESC in pause at
// each hold left pause: ui_state 0, counter 5, empty list.
constexpr std::array<int, 5> kB2hHolds = {2, 5, 10, 25, 40};
// ESC in the level (hold 20), then O, then P.
constexpr Row kB2hDialog = {"level-ESC-dialog", 7, 2, 1, 0x4B26B0};
constexpr Row kB2hO = {"dialog-O", 7, 1, 0, 0};
constexpr Row kB2hP = {"dialog-O-then-P", 7, 0, 0, 0};

Frontend toPause() {
  Frontend f;
  f.press(Key::Return);
  f.press(Key::Return);
  f.press(Key::MapNodeClick);
  return f;
}

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
    Frontend f;
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
    check(f.press(Key::Esc, 25) == Outcome::Handled, "ESC hold 25 handled by default");
    checkRow(f, kB2e[6]);
  }

  std::cout << "frontend sequence: b2h ESC sweep in pause\n";
  for (int hold : kB2hHolds) {
    Frontend f = toPause();
    f.press(Key::P);
    const std::string tag = "hold " + std::to_string(hold);
    check(f.uiState() == kUiPaused, tag + ": paused before ESC");
    check(f.press(Key::Esc, hold) == Outcome::Handled, tag + ": ESC handled");
    check(!f.paused(), tag + ": left pause");
    checkRow(f, {"pause-ESC", 5, 0, 0, 0});
  }

  std::cout << "frontend sequence: b2h level ESC dialog\n";
  {
    Frontend f = toPause();
    check(f.press(Key::Esc, kEscHoldLevel) == Outcome::Handled, "level ESC handled");
    check(f.screen() == Screen::Dialog, "level ESC opens the dialog");
    checkRow(f, kB2hDialog);
    check(f.counter() == 7, "READ root plus menu-page construction advances counter 5 to 7");
    check(f.press(Key::O) == Outcome::Handled, "O at dialog handled");
    checkRow(f, kB2hO);
    check(f.screen() == Screen::Level && f.paused(), "O returns to pause");
    check(f.press(Key::P) == Outcome::Handled, "P handled after O");
    checkRow(f, kB2hP);
  }

  std::cout << "frontend sequence: explicit root/page lifecycle READ model\n";
  {
    Frontend f;
    check(f.counter() == 3, "initial counter1 plus initial root and Menu page");
    f.press(Key::Return); f.press(Key::Return); f.press(Key::MapNodeClick);
    check(f.counter() == 5 && f.screen() == Screen::Level, "Map activation tears frontend down without count");
    check(f.openLevelMenu() == Outcome::Handled && f.counter() == 7 && f.screen() == Screen::Dialog,
          "semantic in-level open constructs root then menu page");
    check(f.openLevelMenu() == Outcome::Ignored && f.counter() == 7,
          "duplicate open cannot construct a second owned root");
    check(f.closeLevelMenu(false) == Outcome::Handled && f.counter() == 7 &&
          f.screen() == Screen::Level && f.uiState() == kUiLevel,
          "Resume ends root without decrement and resumes playing");
    check(f.closeLevelMenu(true) == Outcome::Ignored && f.counter() == 7,
          "close from Level is idempotent and does not change serial");
    f.openLevelMenu();
    check(f.counter() == 9 && f.closeLevelMenu(true) == Outcome::Handled &&
          f.uiState() == kUiPaused, "O close ends root but retains paused distinction");
    f.openLevelMenu();
    check(f.retryLevelMenu() == Outcome::Handled && f.counter() == 11 && f.screen() == Screen::Level,
          "successful menu retry tears down root without construction");
    check(f.retryLevelMenu() == Outcome::Ignored && f.counter() == 11,
          "retry outside an owned menu is a no-op");
    f.openLevelMenu();
    check(f.abandonLevelToMap() == Outcome::Handled && f.counter() == 15 && f.screen() == Screen::Map,
          "Quit tears down menu root then constructs new Map root and page");
  }

  std::cout << "frontend sequence: unobserved inputs and policy\n";
  {
    // Each of these is a reported no-op: state must not move.
    auto stays = [](Frontend& f, Outcome got, std::string_view what) {
      const Screen s = f.screen();
      const int c = f.counter();
      const bool p = f.paused();
      check(got == Outcome::Unobserved, std::string(what) + ": reported Unobserved");
      check(f.screen() == s && f.counter() == c && f.paused() == p,
            std::string(what) + ": state unchanged");
    };
    {
      Frontend f = toPause();
      f.press(Key::P);
      stays(f, f.press(Key::Esc, 41), "pause ESC hold 41");
      stays(f, f.press(Key::Esc, 1), "pause ESC hold 1");
    }
    {
      Frontend f = toPause();
      stays(f, f.press(Key::Esc, 5), "level ESC hold 5");
      stays(f, f.press(Key::O), "O in level");
    }
    {
      Frontend f = toPause();
      f.press(Key::Esc, kEscHoldLevel);
      stays(f, f.press(Key::Esc, kEscHoldLevel), "ESC at dialog");
      stays(f, f.press(Key::P), "P at dialog");
      stays(f, f.press(Key::Return), "RETURN at dialog");
      stays(f, f.leaveLevel(), "leaveLevel at dialog");
    }
    {
      Frontend f = toPause();
      stays(f, f.leaveLevel(), "default: level exit");
    }
  }
  {
    Frontend f({LevelExitPolicy::ToMap});
    f.press(Key::Return);
    f.press(Key::Return);
    f.press(Key::MapNodeClick);
    check(f.leaveLevel() == Outcome::Handled && f.screen() == Screen::Map, "ToMap: back on map");
    check(f.counter() == 7, "ToMap: teardown then new root and Map page (READ)");
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
