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
        return confirmChoice(true);
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
    case Screen::Results:
      // Initial native focus is the subsequently hidden row0; automatic focus
      // repair is unresolved. Do not invent any-key/Return-to-Continue.
      return Outcome::Unobserved;
  }
  return Outcome::Unobserved;
}

Outcome Frontend::confirmChoice(bool yes) {
  if (screen_ != Screen::Confirm) return Outcome::Ignored;
  construct(yes ? Screen::Map : Screen::Menu);
  if (!yes) highlight_ = 0;  // rebuilt menu's initial SOURCE selection
  return Outcome::Handled;
}

Outcome Frontend::leaveLevel() {
  if (screen_ == Screen::Dialog) return Outcome::Unobserved;
  if (screen_ != Screen::Level) return Outcome::Ignored;
  if (policy_.levelExit == LevelExitPolicy::Unobserved) return Outcome::Unobserved;
  paused_ = false;
  construct(Screen::Map);
  return Outcome::Handled;
}

Outcome Frontend::activateMapFooter() {
  if (screen_ != Screen::Map) return Outcome::Ignored;
  construct(Screen::Menu);
  highlight_ = 0;
  return Outcome::Handled;
}

int Frontend::uiState() const {
  if (screen_ == Screen::Results) return resultsWon_ ? 4 : 5;
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
    case Screen::Results: return resultsWon_ ? kVtableWinResults : kVtableLossResults;
  }
  return 0;
}

Outcome Frontend::showResults(bool success) {
  if (screen_ != Screen::Level) return Outcome::Ignored;
  paused_ = false;
  resultsWon_ = success;
  construct(Screen::Results);
  return Outcome::Handled;
}

Outcome Frontend::activateResultsContinue() {
  if (screen_ != Screen::Results) return Outcome::Ignored;
  // READ 486090 (win),4865AC (loss): selector23, not a new-game reset.
  construct(Screen::Map);
  return Outcome::Handled;
}

Outcome Frontend::activateResultsRetry() {
  if (screen_ != Screen::Results || resultsWon_) return Outcome::Ignored;
  // READ 486568 -> main reload via4605D0(1). SOURCE immediate reload omits fades.
  screen_ = Screen::Level;
  paused_ = false;
  return Outcome::Handled;
}

Outcome Frontend::activateResultsQuit() {
  if (screen_ != Screen::Results || resultsWon_) return Outcome::Ignored;
  // READ486583 ->45D72F..45D7EC: UI3/map23, not Menu7 or application exit.
  // Semantic destination only; native consumes a request on the next dispatch.
  construct(Screen::Map);
  return Outcome::Handled;
}

Outcome Frontend::abandonLevelToMap() {
  if (screen_ != Screen::Level) return Outcome::Ignored;
  // READ48F601 ->45D606..45D69B: no result or completion write.
  paused_ = false;
  construct(Screen::Map);
  return Outcome::Handled;
}

}  // namespace webo::shell
