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

Outcome Frontend::press(Key key, int holdFrames) {
  switch (screen_) {
    case Screen::Menu:
      // Preserved quirk: RETURN acts only when the highlight is exactly 0.
      if (key == Key::Return) {
        if (highlight_ != 0) return Outcome::Ignored;
        construct(Screen::Confirm);
        return Outcome::Handled;
      }
      return Outcome::Unobserved;
    case Screen::Confirm:
      // Observed to advance; the confirm highlight was not read.
      if (key == Key::Return) {
        construct(Screen::Map);
        return Outcome::Handled;
      }
      return Outcome::Unobserved;
    case Screen::Map:
      if (key == Key::MapNodeClick) {
        // A level is not a frontend screen: the counter stays at 5 (observed).
        screen_ = Screen::Level;
        paused_ = false;
        return Outcome::Handled;
      }
      return Outcome::Unobserved;
    case Screen::Level:
      if (key == Key::P) {
        paused_ = !paused_;
        return Outcome::Handled;
      }
      if (key == Key::Esc && paused_) {
        // Observed at holds 2, 5, 10, 25, 40; the range between is policy.
        if (holdFrames < kEscHoldMin || holdFrames > kEscHoldMax) return Outcome::Unobserved;
        paused_ = false;
        return Outcome::Handled;
      }
      if (key == Key::Esc) {
        // Observed once, at hold 20.
        if (holdFrames != kEscHoldLevel) return Outcome::Unobserved;
        screen_ = Screen::Dialog;
        counter_ += kLevelEscConstructions;
        return Outcome::Handled;
      }
      return Outcome::Unobserved;
    case Screen::Dialog:
      // Only O was observed here. ESC, P and RETURN at the dialog were not sent.
      if (key == Key::O) {
        screen_ = Screen::Level;
        paused_ = true;
        return Outcome::Handled;
      }
      return Outcome::Unobserved;
  }
  return Outcome::Unobserved;
}

Outcome Frontend::leaveLevel() {
  if (screen_ == Screen::Dialog) return Outcome::Unobserved;
  if (screen_ != Screen::Level) return Outcome::Ignored;
  if (policy_.levelExit == LevelExitPolicy::Unobserved) return Outcome::Unobserved;
  paused_ = false;
  construct(Screen::Map);
  return Outcome::Handled;
}

int Frontend::uiState() const {
  if (screen_ == Screen::Dialog) return kUiDialog;
  if (screen_ != Screen::Level) return kUiFrontend;
  return paused_ ? kUiPaused : kUiLevel;
}

std::uint32_t Frontend::nodeVtable() const {
  switch (screen_) {
    case Screen::Menu: return kVtableMenu;
    case Screen::Confirm: return kVtableConfirm;
    case Screen::Map: return kVtableMap;
    case Screen::Level: return 0;
    case Screen::Dialog: return kVtableDialog;
  }
  return 0;
}

}  // namespace webo::shell
