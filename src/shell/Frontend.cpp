#include "shell/Frontend.h"

namespace webo::shell {

Frontend::Frontend(Policy policy, int initialCounter)
    : policy_(policy), counter_(initialCounter) {
  (void)beginFrontend(Screen::Menu);
}

Outcome Frontend::beginFrontend(Screen page) {
  if (rootOwned_ || page == Screen::Level)
    return Outcome::Ignored;
  // READ 486F60 -> 48BC00 root, then page factory; each consumes one serial.
  ++counter_; // root's base constructor
  rootOwned_ = true;
  ++counter_; // selected page's base constructor
  screen_ = page;
  // Model the full-initialization caller's UI write (45DF33/45E0B7/45D743),
  // not a side effect of the native page constructor. In contrast,
  // replacePage deliberately changes only page identity and preserves the mode.
  switch (page) {
    case Screen::Menu:
    case Screen::Confirm:
    case Screen::SoundOptions:
    case Screen::Map:
      dispatchMode_ = kUiFrontend;
      break;
    case Screen::Dialog:
      dispatchMode_ = kUiDialog;
      break;
    case Screen::Results:
      dispatchMode_ = resultsWon_ ? 4 : 5;
      break;
    case Screen::Level:
      break; // rejected above
  }
  return Outcome::Handled;
}

Outcome Frontend::replacePage(Screen page) {
  if (!rootOwned_ || page == Screen::Level)
    return Outcome::Ignored;
  ++counter_;
  screen_ = page;
  return Outcome::Handled;
}

Outcome Frontend::endFrontend() {
  if (!rootOwned_) return Outcome::Ignored;
  rootOwned_ = false;
  // Destruction does not decrement the global serial allocator.
  return Outcome::Handled;
}

Outcome Frontend::press(Key key, int holdFrames) {
  switch (screen_) {
    case Screen::Menu:
      if (key == Key::Return) {
        if (highlight_ == 0) return replacePage(Screen::Confirm);
        if (highlight_ == 1) return replacePage(Screen::Map);
        if (highlight_ == 3) return openSoundOptions();
        if (highlight_ == 4) {
          // READ48EE41..48EE4C: activating row pointer [menu+50] sets
          // application exit byte4E3581. Results/level-menu Quit are separate.
          quitRequested_ = true;
          return Outcome::Handled;
        }
        return Outcome::Unobserved;
      }
      return Outcome::Unobserved;
    case Screen::Confirm:
      if (key == Key::Return) return confirmChoice(highlight_ == 0);
      return Outcome::Unobserved;
    case Screen::Map:
      if (key == Key::MapNodeClick) {
        if (endFrontend() != Outcome::Handled) return Outcome::Ignored;
        screen_ = Screen::Level;
        paused_ = false;
        dispatchMode_ = kUiLevel;
        return Outcome::Handled;
      }
      return Outcome::Unobserved;
    case Screen::Level:
      if (key == Key::P) {
        paused_ = !paused_;
        dispatchMode_ = paused_ ? kUiPaused : kUiLevel;
        return Outcome::Handled;
      }
      if (key == Key::Esc && paused_) {
        if (holdFrames < kEscHoldMin || holdFrames > kEscHoldMax) return Outcome::Unobserved;
        paused_ = false;
        dispatchMode_ = kUiLevel;
        return Outcome::Handled;
      }
      if (key == Key::Esc) {
        if (holdFrames != kEscHoldLevel) return Outcome::Unobserved;
        // Legacy key replay is intentionally kept distinct from production semantic open.
        return beginFrontend(Screen::Dialog);
      }
      return Outcome::Unobserved;
    case Screen::Dialog:
      if (key == Key::O) return closeLevelMenu(true);
      return Outcome::Unobserved;
    case Screen::SoundOptions:
      if (key == Key::Esc) return returnFromSoundOptions();
      if (key == Key::O && dispatchMode_ == kUiDialog) return closeLevelMenu(true);
      return Outcome::Unobserved;
    case Screen::Results:
      // Native initial hidden-row focus repair remains unresolved. No invented
      // Return-to-Continue; the semantic activation below is separate.
      return Outcome::Unobserved;
  }
  return Outcome::Unobserved;
}

Outcome Frontend::confirmChoice(bool yes) {
  if (screen_ != Screen::Confirm || !rootOwned_) return Outcome::Ignored;
  if (replacePage(yes ? Screen::Map : Screen::Menu) != Outcome::Handled)
    return Outcome::Ignored;
  if (!yes) highlight_ = 0;
  return Outcome::Handled;
}

Outcome Frontend::leaveLevel() {
  if (screen_ == Screen::Dialog) return Outcome::Unobserved;
  if (screen_ != Screen::Level) return Outcome::Ignored;
  if (policy_.levelExit == LevelExitPolicy::Unobserved) return Outcome::Unobserved;
  paused_ = false;
  return abandonLevelToMap();
}

Outcome Frontend::activateMapFooter() {
  if (screen_ != Screen::Map) return Outcome::Ignored;
  if (replacePage(Screen::Menu) != Outcome::Handled) return Outcome::Ignored;
  highlight_ = 0;
  return Outcome::Handled;
}

int Frontend::uiState() const {
  return dispatchMode_;
}

std::uint32_t Frontend::nodeVtable() const {
  switch (screen_) {
    case Screen::Menu: return kVtableMenu;
    case Screen::Confirm: return kVtableConfirm;
    case Screen::Map: return kVtableMap;
    case Screen::Level: return 0;
    case Screen::Dialog: return kVtableDialog;
    case Screen::SoundOptions: return 0x4B24A8;
    case Screen::Results: return resultsWon_ ? kVtableWinResults : kVtableLossResults;
  }
  return 0;
}

Outcome Frontend::showResults(bool success) {
  if (screen_ != Screen::Level || rootOwned_) return Outcome::Ignored;
  // READ45DF25/45E0A9: full initialization, unlike Continue's page replacement.
  paused_ = false;
  resultsWon_ = success;
  highlight_ = -1; // The shared builder's initial row is hidden on both results pages.
  return beginFrontend(Screen::Results);
}

Outcome Frontend::activateResultsContinue() {
  if (screen_ != Screen::Results) return Outcome::Ignored;
  // READ4886E4 ->48726F: preserve the root, retire/replace the result page.
  return replacePage(Screen::Map);
}

Outcome Frontend::activateResultsRetry() {
  if (screen_ != Screen::Results || resultsWon_) return Outcome::Ignored;
  if (endFrontend() != Outcome::Handled) return Outcome::Ignored;
  screen_ = Screen::Level;
  paused_ = false;
  dispatchMode_ = kUiLevel;
  return Outcome::Handled;
}

Outcome Frontend::activateResultsQuit() {
  if (screen_ != Screen::Results || resultsWon_) return Outcome::Ignored;
  // READ45D79E/45D7EC: teardown, then full initialization (not Continue).
  if (endFrontend() != Outcome::Handled) return Outcome::Ignored;
  return beginFrontend(Screen::Map);
}

Outcome Frontend::abandonLevelToMap() {
  if (screen_ != Screen::Level && screen_ != Screen::Dialog) return Outcome::Ignored;
  if (rootOwned_ && endFrontend() != Outcome::Handled) return Outcome::Ignored;
  paused_ = false;
  return beginFrontend(Screen::Map);
}

Outcome Frontend::openLevelMenu() {
  if (screen_ != Screen::Level || rootOwned_) return Outcome::Ignored;
  return beginFrontend(Screen::Dialog);
}

Outcome Frontend::closeLevelMenu(bool paused) {
  if ((screen_ != Screen::Dialog && !(screen_ == Screen::SoundOptions && dispatchMode_ == kUiDialog))
      || !rootOwned_) return Outcome::Ignored;
  if (endFrontend() != Outcome::Handled) return Outcome::Ignored;
  screen_ = Screen::Level;
  paused_ = paused;
  dispatchMode_ = paused ? kUiPaused : kUiLevel;
  return Outcome::Handled;
}

Outcome Frontend::openSoundOptions() {
  if (screen_ != Screen::Menu && screen_ != Screen::Dialog) return Outcome::Ignored;
  highlight_ = 0; // 48E810..48E821 focuses Music.
  return replacePage(Screen::SoundOptions);
}

Outcome Frontend::returnFromSoundOptions() {
  if (screen_ != Screen::SoundOptions) return Outcome::Ignored;
  highlight_ = 0;
  // 48EE6A / 48F617 pass selector 7 / 1F as page+24. Dispatch mode
  // survives page replacement (including a main menu reached from results).
  return replacePage(dispatchMode_ == kUiDialog ? Screen::Dialog : Screen::Menu);
}

Outcome Frontend::retryLevelMenu() {
  if (screen_ != Screen::Dialog || !rootOwned_) return Outcome::Ignored;
  if (endFrontend() != Outcome::Handled) return Outcome::Ignored;
  screen_ = Screen::Level;
  paused_ = false;
  dispatchMode_ = kUiLevel;
  return Outcome::Handled;
}

}  // namespace webo::shell
