#include "shell/Frontend.h"

namespace webo::shell {

Frontend::Frontend(Policy policy, int initialCounter)
    : policy_(policy), counter_(initialCounter) {
  (void)beginFrontend(Screen::Menu);
}

Outcome Frontend::beginFrontend(Screen page) {
  if (rootOwned_ || page == Screen::Level)
    return Outcome::Ignored;
  // At 486F60, 48BC00 creates the root before the page factory. Each consumes one serial.
  ++counter_; // root's base constructor
  rootOwned_ = true;
  ++counter_; // selected page's base constructor
  screen_ = page;
  // Full initialization sets the dispatch mode as at 45DF33/45E0B7/45D743. Page replacement preserves it.
  switch (page) {
    case Screen::Menu:
    case Screen::Confirm:
    case Screen::Language:
    case Screen::SoundOptions:
    case Screen::KeyboardShortcuts:
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
        if (highlight_ == 2) {
          // 48EDDB..48EDFC selects page 0x0C. Its builder focuses the first row.
          const auto outcome = replacePage(Screen::Language);
          if (outcome == Outcome::Handled) highlight_ = 0;
          return outcome;
        }
        if (highlight_ == 3) return openSoundOptions();
        if (highlight_ == 4) {
          // At 48EE41..48EE4C, activating [menu+50] sets the application exit flag byte4E3581.
          quitRequested_ = true;
          return Outcome::Handled;
        }
        return Outcome::Unobserved;
      }
      return Outcome::Unobserved;
    case Screen::Confirm:
      if (key == Key::Return) return confirmChoice(highlight_ == 0);
      return Outcome::Unobserved;
    case Screen::Language:
      if (key == Key::Return || key == Key::Esc) {
        // 48E326 replaces the page with selector 7, whose builder focuses row 0.
        const auto outcome = replacePage(Screen::Menu);
        if (outcome == Outcome::Handled) highlight_ = 0;
        return outcome;
      }
      return Outcome::Unobserved;
    case Screen::Map:
      if (key == Key::MapNodeClick) return enterLevel();
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
    case Screen::KeyboardShortcuts:
      if (key == Key::Esc || key == Key::Return) return replacePage(Screen::SoundOptions);
      return Outcome::Unobserved;
    case Screen::Results:
      // Results start with a hidden row. Native keyboard focus repair is unresolved.
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

Outcome Frontend::enterLevel() {
  if ((screen_ != Screen::Menu && screen_ != Screen::Map) ||
      endFrontend() != Outcome::Handled) return Outcome::Ignored;
  screen_ = Screen::Level;
  paused_ = false;
  dispatchMode_ = kUiLevel;
  return Outcome::Handled;
}

int Frontend::uiState() const {
  return dispatchMode_;
}

std::uint32_t Frontend::nodeVtable() const {
  switch (screen_) {
    case Screen::Menu: return kVtableMenu;
    case Screen::Confirm: return kVtableConfirm;
    case Screen::Language: return kVtableLanguage;
    case Screen::Map: return kVtableMap;
    case Screen::Level: return 0;
    case Screen::Dialog: return kVtableDialog;
    case Screen::SoundOptions: return 0x4B24A8;
    case Screen::KeyboardShortcuts: return 0; // Clone page has no original vtable.
    case Screen::Results: return resultsWon_ ? kVtableWinResults : kVtableLossResults;
  }
  return 0;
}

Outcome Frontend::showResults(bool success) {
  if (screen_ != Screen::Level || rootOwned_) return Outcome::Ignored;
  // At 45DF25/45E0A9, results construct a new root and page.
  paused_ = false;
  resultsWon_ = success;
  highlight_ = -1; // The shared builder's initial row is hidden on both results pages.
  return beginFrontend(Screen::Results);
}

Outcome Frontend::activateResultsContinue() {
  if (screen_ != Screen::Results) return Outcome::Ignored;
  // At 4886E4 -> 48726F, retain the root and replace the result page.
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
  // At 45D79E/45D7EC, tear down the frontend before creating the map root and page.
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
  // 48EE6A / 48F617 pass selector 7 / 1F as page+24. Page replacement preserves dispatch mode.
  return replacePage(dispatchMode_ == kUiDialog ? Screen::Dialog : Screen::Menu);
}

Outcome Frontend::openKeyboardShortcuts() {
  if (screen_ != Screen::SoundOptions) return Outcome::Ignored;
  return replacePage(Screen::KeyboardShortcuts);
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
