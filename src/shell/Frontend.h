// Frontend screen sequence for the Phase 5 shell.
//
// Pure state machine (screen, construction counter, highlight, paused) driven by key
// events. It reproduces the b2e live capture (docs/scratch/phase-b2e-live-capture.md,
// build/tmp/b2e-live-capture/report.json) and nothing more. Behaviour the capture did
// not see is a named policy switch or a reported no-op, never a silent choice.
//
// OBSERVED (b2e, one run): construction counter 0x004C416C reads 3 at the menu, 4 after
// the first RETURN (confirm), 5 after the second (map), 5 through a map node click, P and
// short ESC. Each frontend screen was the only list node, replaced rather than stacked.
// HYPOTHESIS: the counter counts screens constructed, and two startup screens consume 1
// and 2 before the menu.

#pragma once

#include <cstdint>

namespace webo::shell {

enum class Screen : std::uint8_t { Menu, Confirm, Map, Level };

enum class Key : std::uint8_t { Return, EscShort, EscLong, P, MapNodeClick };

enum class Outcome : std::uint8_t {
  Handled,     // state advanced as the capture (or a preserved quirk) says
  Ignored,     // the key does nothing on this screen
  Unobserved,  // the capture has no data for this input; state unchanged
};

// Short ESC while paused. OBSERVED once: left pause. Preserved quirk (rules file):
// fails to leave pause. The cause of the disagreement is unknown, so the default
// follows the quirk and LeavePause is the policy that replays the b2e row.
enum class ShortEscPolicy : std::uint8_t { Refuse, LeavePause };

// Level back to the map. NOT observed. Unobserved is a no-op; ToMap constructs a new
// map screen (counter + 1), which is a hypothesis.
enum class LevelExitPolicy : std::uint8_t { Unobserved, ToMap };

struct Policy {
  ShortEscPolicy shortEsc = ShortEscPolicy::Refuse;
  LevelExitPolicy levelExit = LevelExitPolicy::Unobserved;
};

// OBSERVED vtables of the single list node (b2e); 0 inside a level (list head 0).
inline constexpr std::uint32_t kVtableMenu = 0x4B2528;
inline constexpr std::uint32_t kVtableConfirm = 0x4B3F38;
inline constexpr std::uint32_t kVtableMap = 0x4B3EB8;

// OBSERVED ui_state: 3 on a frontend screen, 0 in a level, 1 paused.
inline constexpr int kUiFrontend = 3;
inline constexpr int kUiLevel = 0;
inline constexpr int kUiPaused = 1;

class Frontend {
 public:
  // startupConstructions is the HYPOTHESIS above; the menu is the next construction.
  explicit Frontend(Policy policy = {}, int startupConstructions = 2);

  Outcome press(Key key);
  // Highlight of the menu. Initial value 0 is unread, not observed.
  void setHighlight(int highlight) { highlight_ = highlight; }
  // Level back to the map; the triggering input is unobserved, so it is a method.
  Outcome leaveLevel();

  Screen screen() const { return screen_; }
  int counter() const { return counter_; }
  int highlight() const { return highlight_; }
  bool paused() const { return paused_; }
  int uiState() const;
  // Number of nodes in the screen list: 1 on a frontend screen, 0 in a level.
  int nodeCount() const { return screen_ == Screen::Level ? 0 : 1; }
  std::uint32_t nodeVtable() const;

 private:
  void construct(Screen s);

  Policy policy_;
  Screen screen_ = Screen::Menu;
  int counter_ = 0;
  int highlight_ = 0;
  bool paused_ = false;
};

}  // namespace webo::shell
