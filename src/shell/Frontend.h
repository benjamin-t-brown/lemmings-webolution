// Frontend screen and root/page lifecycle model.
//
// Pure state machine (screen, frontend root/page ownership, serial allocator,
// highlight and paused state) driven by key or semantic lifecycle events. It replays the b2e and b2h captures
// (docs/scratch/phase-b2e-live-capture.md, phase-b2h-live-esc-sweep.md;
// build/tmp/b2e-live-capture/report.json, build/tmp/b2h-live-esc-sweep/report.json) and
// plus the READ Yes/No semantic routes at 0x49BC59. Other statements below are
// tagged OBSERVED, READ or POLICY. The legacy key replay retains its bounded
// sampled domains; semantic lifecycle methods separately implement READ routes.
// Unsupported key events return Outcome::Unobserved without changing state.
//
// OBSERVED (b2e, one run): construction counter 0x004C416C reads 3 at the menu, 4 after
// the first RETURN (confirm), 5 after the second (map), 5 through a map node click and P.
// Each frontend screen was the only list node, replaced rather than stacked.
// OBSERVED (b2e hold 25; b2h holds 2, 5, 10, 25, 40, one sample each): ESC in pause gives
// ui_state 0, counter 5, empty list. The old "short ESC fails to leave pause" quirk is
// retracted (rules file, 2026-10-23).
// OBSERVED (b2h, one run): ESC in an unpaused level (hold 20) gives ui_state 2, counter
// +2 (5 to 7), one head node with vtable 0x4B26B0. READ explains this as root+page;
// O there gives ui_state 1, empty list, counter unchanged; P then gives ui_state 0.
// READ (docs/scratch/frontend-native-return-2026-10-28.md): initial counter is 1;
// full frontend initialization constructs a separate root and selected page.
// Level-menu opening invokes that initialization; it is not a hold-20-specific guess.
// POLICY: ESC hold lengths between the sampled 2 and 40 frames act like the sampled ones
// (b2g read: P and ESC share one pause store, 0x45D508). Holds outside [2, 40] in pause,
// and any hold but 20 in the level, are Unobserved.

#pragma once

#include <cstdint>

namespace webo::shell {

// Dialog is the in-level menu: READ48F4E0, selector1F; one captured page node.
enum class Screen : std::uint8_t { Menu, Confirm, Map, Level, Dialog, Results };

enum class Key : std::uint8_t { Return, Esc, P, O, MapNodeClick };

enum class Outcome : std::uint8_t {
  Handled,     // state advanced as the captures (or a preserved quirk) say
  Ignored,     // preserved quirk: RETURN at a menu highlight other than 0
  Unobserved,  // the captures have no data for this input; state unchanged
};

// Legacy direct level-exit policy; no key for this shortcut was observed.
// Production menu Quit is separate: observed menu7 -> Map9; READ root+page init.
enum class LevelExitPolicy : std::uint8_t { Unobserved, ToMap };

struct Policy {
  LevelExitPolicy levelExit = LevelExitPolicy::Unobserved;
};

// OBSERVED vtables of the single list node (b2e, b2h); 0 when the list is empty.
inline constexpr std::uint32_t kVtableMenu = 0x4B2528;
inline constexpr std::uint32_t kVtableConfirm = 0x4B3F38;
inline constexpr std::uint32_t kVtableMap = 0x4B3EB8;
inline constexpr std::uint32_t kVtableDialog = 0x4B26B0;
// READ result constructors 485DF0/4862F0, selectors21/20 respectively.
inline constexpr std::uint32_t kVtableWinResults = 0x4B1C30;
inline constexpr std::uint32_t kVtableLossResults = 0x4B1CB0;

// OBSERVED ui_state: 3 on a frontend screen, 0 in a level, 1 paused, 2 at the dialog.
inline constexpr int kUiFrontend = 3;
inline constexpr int kUiLevel = 0;
inline constexpr int kUiPaused = 1;
inline constexpr int kUiDialog = 2;

// ESC hold lengths, in frames. Sampled in pause: 2, 5, 10, 25, 40. Sampled in the level: 20.
inline constexpr int kEscHoldMin = 2;
inline constexpr int kEscHoldMax = 40;
inline constexpr int kEscHoldLevel = 20;

class Frontend {
 public:
  // Counter starts at the READ on-disk value (1). Initial root + Menu page -> 3.
  explicit Frontend(Policy policy = {}, int initialCounter = 1);

  // holdFrames matters only for Esc.
  Outcome press(Key key, int holdFrames = kEscHoldLevel);
  // READ semantic activation at 0x49BC59: Yes routes to selector 0x23 (map),
  // No to selector 7 (new menu). Physical RETURN-at-No is not established.
  Outcome confirmChoice(bool yes);
  // READ 0x49B1E4: semantic event0 from map's footer constructs selector7.
  // Mapping a physical key/button to this action is left to the app adapter.
  Outcome activateMapFooter();
  // READ semantic routes, not native end-delay or physical-input timing.
  Outcome showResults(bool success);
  Outcome activateResultsContinue();
  Outcome activateResultsRetry();
  Outcome activateResultsQuit();
  Outcome abandonLevelToMap();
  // Semantic production lifecycle. Opening creates root+menu page; dismissing
  // deletes both without decrement. The hold-20 key replay remains a separate adapter.
  Outcome openLevelMenu();
  Outcome closeLevelMenu(bool paused);
  Outcome retryLevelMenu();
  // Highlight of the menu. Initial value 0 is unread, not observed.
  void setHighlight(int highlight) { highlight_ = highlight; }
  // Level back to the map; the triggering input is unobserved, so it is a method.
  Outcome leaveLevel();

  Screen screen() const { return screen_; }
  int counter() const { return counter_; }
  int highlight() const { return highlight_; }
  bool paused() const { return paused_; }
  int uiState() const;
  // Number of nodes in the screen list: 1 on a frontend screen and at the dialog, 0 in a level.
  int nodeCount() const { return screen_ == Screen::Level ? 0 : 1; }
  std::uint32_t nodeVtable() const;

 private:
  Outcome beginFrontend(Screen page);
  Outcome replacePage(Screen page);
  Outcome endFrontend();

  Policy policy_;
  Screen screen_ = Screen::Menu;
  int counter_ = 0;
  int highlight_ = 0;
  bool paused_ = false;
  bool resultsWon_ = false;
  bool rootOwned_ = false;
  // Native UI dispatch mode is not derived from the currently displayed page:
  // results Continue replaces the page while retaining mode 4 (win) or 5 (loss).
  int dispatchMode_ = kUiFrontend;
};

}  // namespace webo::shell
