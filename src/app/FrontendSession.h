#pragma once

#include "app/MapScene.h"
#include "app/LevelOutcome.h"
#include "app/ResultsScene.h"
#include "app/RuntimeCursor.h"
#include "app/SoundMenu.h"
#include "save/SaveStore.h"
#include "save/SavedGame.h"
#include "shell/Frontend.h"

#include <algorithm>
#include <array>
#include <expected>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace webo::app {

// Owns retail frontend assets and the clone's progress. create() optionally
// loads original-layout SavedGame.dat. Missing or invalid saves start empty,
// with diagnostics for invalid input. A win persists the in-memory commit.
// New Game Yes writes the zero image. Disk I/O requires a store.
class FrontendSession {
 public:
  static std::expected<FrontendSession, std::string> create(
      const AssetLibrary& library, std::shared_ptr<save::SaveStore> store = nullptr,
      std::shared_ptr<save::SaveSink> languageFile = nullptr);
  // Save diagnostics describe a failed load or write. A successful write clears its error.
  const std::optional<std::string>& saveLoadError() const { return saveLoadError_; }
  const std::optional<std::string>& saveError() const { return saveError_; }
  // Session write count, including failed writes.
  int saveCount() const { return saveCount_; }
  const save::SavedGame& savedGame() const { return savedGame_; }

  const Image& image() const;
  // Present the cached base scene with map hover text, then draw the cursor last.
  Image composeFrame() const;
  // SDL supplies absolute coordinates. 453C3A..453D23 clamps relative
  // DirectInput deltas to these same inclusive extents.
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
  shell::Outcome pressEscape();
  void hoverMenu(int x, int y);
  bool moveMenuFocus(int direction);
  int menuFocus() const { return state_.highlight(); }
  bool quitRequested() const { return state_.quitRequested(); }
  shell::Outcome openLevelMenu();
  shell::Outcome closeLevelMenu(bool paused);
  void configureSound(save::SoundSettings levels, std::shared_ptr<save::SaveSink> file,
                      std::function<void(save::SoundSettings, bool previewSfx)> preview);
  int windowScale() const { return acceptedSound_.windowScale; }
  bool reverseDrag() const { return acceptedSound_.reverseDrag; }
  void setWindowScaleMenu(WindowScaleMenu window) { windowScaleMenu_ = window; }
  shell::Outcome openSoundOptions();
  shell::Outcome closeSoundOptions(bool accept);
  bool adjustSound(int direction);
  void hoverSound(int x, int y);
  Image renderSoundOptions(const Image& backdrop) const;
  bool click(int x, int y);
  bool clickMap(int x, int y);
  const std::optional<std::string>& selectedLevel() const { return selectedLevel_; }
  bool levelIntroEnabled() const { return levelIntroEnabled_; }
  bool startDirectLevel(std::string levelId);
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
  void refreshMap();
  shell::Frontend state_;
  SoundMenu soundMenu_;
  save::SoundSettings acceptedSound_, editingSound_;
  WindowScaleMenu windowScaleMenu_;
  std::shared_ptr<save::SaveSink> soundFile_;
  std::function<void(save::SoundSettings, bool)> previewSound_;
  bool soundWritePending_ = false;
  mutable Image soundImage_;
  mutable Image shortcutsImage_;
  int soundBaseY() const;
  void finishSoundEdit(bool accept);
  // Cache the rendered focus. Page replacement can reset the model's highlight.
  mutable Image menu_;
  mutable int menuImageHighlight_ = 0;
  Image menuBackdrop_, confirmBackdrop_;
  mutable Image confirm_;
  Image languageImage_;
  Image mapImage_;
  mutable int confirmImageFocus_ = 0;
  std::vector<std::string> menuLabels_;
  std::array<std::string, 2> confirmLabels_;
  std::string confirmQuestion_;
  Image font_;
  std::string mapFooterLabel_;
  // Half-open hit regions follow the rendered panels. The shell owns focus.
  std::array<std::array<int, 4>, 5> menuBoxes_{};
  std::array<std::array<int, 4>, 2> confirmBoxes_{};
  std::array<std::array<int, 4>, 1> languageBoxes_{};
  std::array<int, 4> mapFooterBox_{};
  MapScene map_;
  RuntimeCursor cursor_;
  int pointerX_ = 320, pointerY_ = 240;
  bool leftHeld_ = false, rightHeld_ = false, focused_ = true;
  std::array<std::int32_t, 102> completionFlags_{};
  // Persistence. savedGame_ is the full original-layout image so unmapped records,
  // A, B and the flag survive load -> complete -> save unchanged.
  std::shared_ptr<save::SaveStore> store_;
  std::shared_ptr<save::SaveSink> languageFile_;
  save::SavedGame savedGame_;
  std::optional<std::string> saveLoadError_, saveError_;
  int saveCount_ = 0;
  void persist();
  std::optional<std::size_t> selectedNode_;
  std::optional<std::string> selectedLevel_;
  std::optional<LevelOutcome> outcome_;
  std::optional<ResultsScene> results_;
  // READ4E3584: next-load intro preference, not a routing or reload flag.
  bool levelIntroEnabled_ = true;
};

}  // namespace webo::app
