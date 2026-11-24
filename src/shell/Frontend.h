// Models screen, root/page ownership, focus, pause and key or semantic actions.
// The construction counter at 0x004C416C starts at 1 and increments for each root and page.
// A frontend list holds one page node. Opening the in-level menu inserts vtable 0x4B26B0.
// P and ESC share the pause store at 0x45D508. Provisional key policy accepts ESC holds of
// 2 to 40 frames while paused and 20 in an unpaused level. Unsupported keys preserve state.

#pragma once

#include <cstdint>

namespace webo::shell {

// Dialog is the in-level menu (48F4E0, selector1F).
enum class Screen : std::uint8_t { Menu, Confirm, Map, Level, Dialog, Results, SoundOptions, KeyboardShortcuts };

enum class Key : std::uint8_t { Return, Esc, P, O, MapNodeClick };

enum class Outcome : std::uint8_t {
  Handled,     // action applied
  Ignored,     // action rejected by a guard
  Unobserved,  // input outside the modeled domain
};

// Provisional direct exit policy. Menu Quit takes menu7 -> Map9 via root and page initialization.
enum class LevelExitPolicy : std::uint8_t { Unobserved, ToMap };

struct Policy {
  LevelExitPolicy levelExit = LevelExitPolicy::Unobserved;
};

// Original vtables for the single page node. An empty list returns 0.
inline constexpr std::uint32_t kVtableMenu = 0x4B2528;
inline constexpr std::uint32_t kVtableConfirm = 0x4B3F38;
inline constexpr std::uint32_t kVtableMap = 0x4B3EB8;
inline constexpr std::uint32_t kVtableDialog = 0x4B26B0;
// Result constructors 485DF0/4862F0 use selectors21/20 respectively.
inline constexpr std::uint32_t kVtableWinResults = 0x4B1C30;
inline constexpr std::uint32_t kVtableLossResults = 0x4B1CB0;

// Original ui_state values.
inline constexpr int kUiFrontend = 3;
inline constexpr int kUiLevel = 0;
inline constexpr int kUiPaused = 1;
inline constexpr int kUiDialog = 2;

// Accepted Esc hold lengths in frames.
inline constexpr int kEscHoldMin = 2;
inline constexpr int kEscHoldMax = 40;
inline constexpr int kEscHoldLevel = 20;

class Frontend {
 public:
  // The initial root and Menu page consume two serials.
  explicit Frontend(Policy policy = {}, int initialCounter = 1);

  // holdFrames matters only for Esc.
  Outcome press(Key key, int holdFrames = kEscHoldLevel);
  // At 0x49BC59, Yes opens selector 0x23 (map) and No opens selector 7 (menu).
  Outcome confirmChoice(bool yes);
  // At 0x49B1E4, event0 from the map footer constructs selector7. The app binds the input.
  Outcome activateMapFooter();
  Outcome enterLevel();
  // Changes result pages at semantic actions. Original input timing and end delay are unmodeled.
  Outcome showResults(bool success);
  Outcome activateResultsContinue();
  Outcome activateResultsRetry();
  Outcome activateResultsQuit();
  Outcome abandonLevelToMap();
  // Opening creates a root and menu page. Dismissing removes both without reclaiming serials.
  Outcome openLevelMenu();
  Outcome closeLevelMenu(bool paused);
  Outcome retryLevelMenu();
  Outcome openSoundOptions();
  Outcome returnFromSoundOptions();
  Outcome openKeyboardShortcuts();
  // Shared menu/Confirm focus; page construction/return starts at row0.
  void setHighlight(int highlight) { highlight_ = highlight; }
  // Leave the level for the map without assuming a physical input binding.
  Outcome leaveLevel();

  Screen screen() const { return screen_; }
  int counter() const { return counter_; }
  int highlight() const { return highlight_; }
  bool paused() const { return paused_; }
  bool quitRequested() const { return quitRequested_; }
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
  bool quitRequested_ = false;
  // Results Continue replaces the page while retaining dispatch mode 4 (win) or 5 (loss).
  int dispatchMode_ = kUiFrontend;
};

}  // namespace webo::shell
