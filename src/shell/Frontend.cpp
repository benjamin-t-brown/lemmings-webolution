#include "shell/Frontend.h"

namespace webo::shell {

Frontend::Frontend(Policy policy, int startupConstructions)
    : policy_(policy), counter_(startupConstructions) {
  construct(Screen::Menu);
}

void Frontend::construct(Screen s) {
  // The list is replaced, not stacked: one construction, one live screen.
  screen_ = s;
  ++counter_;
}

Outcome Frontend::press(Key key) {
  switch (screen_) {
    case Screen::Menu:
      // Preserved quirk: RETURN acts only when the highlight is exactly 0.
      if (key == Key::Return && highlight_ == 0) {
        construct(Screen::Confirm);
        return Outcome::Handled;
      }
      return Outcome::Ignored;
    case Screen::Confirm:
      // Observed to advance; the confirm highlight was not read.
      if (key == Key::Return) {
        construct(Screen::Map);
        return Outcome::Handled;
      }
      return Outcome::Ignored;
    case Screen::Map:
      if (key == Key::MapNodeClick) {
        // A level is not a frontend screen: the counter stays at 5 (observed).
        screen_ = Screen::Level;
        paused_ = false;
        return Outcome::Handled;
      }
      return Outcome::Ignored;
    case Screen::Level:
      if (key == Key::P) {
        paused_ = !paused_;
        return Outcome::Handled;
      }
      if (key == Key::EscShort && paused_) {
        if (policy_.shortEsc == ShortEscPolicy::LeavePause) {
          paused_ = false;
          return Outcome::Handled;
        }
        return Outcome::Ignored;
      }
      if (key == Key::EscLong && paused_) return Outcome::Unobserved;
      return Outcome::Ignored;
  }
  return Outcome::Ignored;
}

Outcome Frontend::leaveLevel() {
  if (screen_ != Screen::Level) return Outcome::Ignored;
  if (policy_.levelExit == LevelExitPolicy::Unobserved) return Outcome::Unobserved;
  paused_ = false;
  construct(Screen::Map);
  return Outcome::Handled;
}

int Frontend::uiState() const {
  if (screen_ != Screen::Level) return kUiFrontend;
  return paused_ ? kUiPaused : kUiLevel;
}

std::uint32_t Frontend::nodeVtable() const {
  switch (screen_) {
    case Screen::Menu: return kVtableMenu;
    case Screen::Confirm: return kVtableConfirm;
    case Screen::Map: return kVtableMap;
    case Screen::Level: return 0;
  }
  return 0;
}

}  // namespace webo::shell
