#pragma once

#include "app/MapScene.h"
#include "app/LevelOutcome.h"
#include "app/ResultsScene.h"
#include "app/RuntimeCursor.h"
#include "shell/Frontend.h"

#include <algorithm>
#include <array>
#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace webo::app {

// Retail assets plus the retained B2 screen sequence. SDL is only an input
// adapter: the model does not claim that an SDL click proves native DirectInput
// binding. New-game progress starts empty in memory; no installed save is read
// or reset. Results Continue/retry/Quit preserve this owned progress; New Game Yes
// clears it. Other menu actions remain unimplemented; main-menu UP/DOWN is an
// SDL adapter, not a claim about physical DirectInput bindings.
class FrontendSession {
 public:
  static std::expected<FrontendSession, std::string> create(
      const AssetLibrary& library);

  const Image& image() const;
  // Base scene stays cursor-free for renderer tests; production paths present
  // this independent copy with the frontend cursor composited last.
  Image composeFrame() const;
  // SOURCE absolute SDL adapter; native453C3A..453D23 uses these inclusive
  // extents after adding relative device deltas. No pointer warp is performed.
  void setPointer(int x, int y) {
    pointerX_ = std::clamp(x, 0, 640);
    pointerY_ = std::clamp(y, 0, 480);
  }
  void setButtons(bool left, bool right) { leftHeld_ = left; rightHeld_ = right; }
  void setFocused(bool focused) {
    focused_ = focused;
    if (!focused) leftHeld_ = rightHeld_ = false;
  }
  int pointerX() const { return pointerX_; }
  int pointerY() const { return pointerY_; }
  bool leftHeld() const { return leftHeld_; }
  bool rightHeld() const { return rightHeld_; }
  bool focused() const { return focused_; }
  CursorState cursorState() const {
    return frontendCursorState(focused_, leftHeld_ || rightHeld_);
  }
  shell::Screen screen() const { return state_.screen(); }
  int constructionCounter() const { return state_.counter(); }
  shell::Outcome pressReturn();
  bool moveMenuFocus(int direction);
  int menuFocus() const { return state_.highlight(); }
  shell::Outcome openLevelMenu();
  shell::Outcome closeLevelMenu(bool paused);
  bool click(int x, int y);
  bool clickMap(int x, int y);
  const std::optional<std::string>& selectedLevel() const { return selectedLevel_; }
  bool levelIntroEnabled() const { return levelIntroEnabled_; }
  bool retryLevel();
  bool abandonLevelToMap();
  const std::array<std::int32_t, 102>& completionFlags() const { return completionFlags_; }
  const std::array<std::int32_t, 102>& mapStates() const { return map_.states(); }
  const std::optional<LevelOutcome>& outcome() const { return outcome_; }
  std::expected<void, std::string> completeLevel(const AssetLibrary& library,
                                               const LevelOutcome& outcome);

 private:
  FrontendSession() = default;
  shell::Outcome confirmChoice(bool yes);
  Image renderMapImage(const MapScene& map) const;
  shell::Frontend state_;
  // A render cache, not a second focus owner: page replacement can reset the
  // model's highlight without passing through directional input.
  mutable Image menu_;
  mutable int menuImageHighlight_ = 0;
  Image menuBackdrop_, confirm_, mapImage_;
  std::vector<std::string> menuLabels_;
  Image font_, initialMapImage_;
  std::string mapFooterLabel_;
  int firstMenuLeft_ = 0, firstMenuRight_ = 0;
  std::array<std::array<int, 4>, 2> confirmBoxes_{};
  std::array<int, 4> mapFooterBox_{};
  MapScene map_;
  RuntimeCursor cursor_;
  int pointerX_ = 320, pointerY_ = 240;
  bool leftHeld_ = false, rightHeld_ = false, focused_ = true;
  MapScene initialMap_;
  std::array<std::int32_t, 102> completionFlags_{};
  std::optional<std::size_t> selectedNode_;
  std::optional<std::string> selectedLevel_;
  std::optional<LevelOutcome> outcome_;
  std::optional<ResultsScene> results_;
  // READ4E3584: next-load intro preference, not a routing or reload flag.
  bool levelIntroEnabled_ = true;
};

}  // namespace webo::app
