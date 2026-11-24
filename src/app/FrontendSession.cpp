#include "app/FrontendSession.h"

#include "app/EnglishStrings.h"
#include "shell/HudText.h"
#include "shell/MenuRender.h"

#include <string_view>
#include <cctype>
#include <algorithm>
#include <cstdio>
#include <vector>

namespace webo::app {

std::expected<FrontendSession, std::string> FrontendSession::create(
    const AssetLibrary& library, std::shared_ptr<save::SaveStore> store,
    std::shared_ptr<save::SaveSink> languageFile) {
  // Missing saves start empty. Unreadable or wrong-size saves report a
  // diagnostic and remain on disk until a successful write.
  save::SavedGame loadedImage;
  std::optional<std::string> loadError;
  if (store) {
    if (auto loaded = store->load()) {
      loadedImage = *loaded;
    } else {
#ifdef __EMSCRIPTEN__
      return std::unexpected("cannot load saved progress: " + loaded.error());
#else
      loadError = loaded.error();
      std::fprintf(stderr,
                   "save: %s; starting with empty progress (the file is left untouched until a "
                   "later save succeeds)\n", loadError->c_str());
#endif
    }
  }
  const auto loadedFlags = loadedImage.nodeFlags(MapScene::nodeMapping());

  auto logo = loadTexture(library, "TEXTURES\\MAIN_LOGO.BMP");
  auto background = loadTexture(library, "TEXTURES\\SMALL_BACK.BMP");
  auto font = loadTexture(library, "TEXTURES\\^FONT.BMP");
  auto strings = loadEnglishStrings(library);
  auto map = MapScene::build(library, loadedFlags);
  auto cursor = RuntimeCursor::load(library);
  if (!logo) return std::unexpected(logo.error());
  if (!background) return std::unexpected(background.error());
  if (!font) return std::unexpected(font.error());
  if (!strings) return std::unexpected(strings.error());
  if (!map) return std::unexpected(map.error());
  if (!cursor) return std::unexpected(cursor.error());
  if (logo->width != 640 || logo->height != 480 ||
      background->width != 640 || background->height != 480 || strings->size() <= 129)
    return std::unexpected("frontend requires native-size backgrounds and UK strings through 129");

  FrontendSession result;
  auto sound = SoundMenu::load(library, *strings);
  if (!sound) return std::unexpected(sound.error());
  result.soundMenu_ = std::move(*sound);
  const auto& cells = shell::fontCells();
  for (const auto index : shell::kLabelIndices)
    result.menuLabels_.emplace_back((*strings)[index]);
  result.menuBackdrop_ = *logo;
  shell::MenuParams menuParams;
  menuParams.highlight = result.menuFocus();
  result.menu_ = shell::renderMainMenu(*logo, *font, result.menuLabels_, menuParams, cells);
  for (std::size_t row = 0; row < result.menuBoxes_.size(); ++row) {
    const auto& label = result.menuLabels_[row];
    const int left = shell::labelLeft(label, cells, menuParams.xWord);
    const int top = menuParams.baseY + static_cast<int>(row) * menuParams.rowStep + menuParams.panelTop;
    result.menuBoxes_[row] = {left - menuParams.panelPadX, top,
        left + shell::textWidth(label, cells) + menuParams.panelPadX, top + menuParams.panelHeight};
  }

  // Selector 0x0C uses MAIN_LOGO and the shared row origin (0x48E197).
  // UK[115] is the original first choice and the clone's only current language.
  const std::array english{std::string((*strings)[115])};
  const shell::MenuParams languageParams;
  result.languageImage_ = shell::renderMainMenu(*logo, *font, english, languageParams, cells);
  result.languageBoxes_[0] = shell::menuPlateBounds(
      shell::labelLeft(english[0], cells, languageParams.xWord), languageParams.baseY,
      shell::textWidth(english[0], cells), languageParams);

  // READ 0x49BB6B: base200 + shared row offset100, table 0x4C4514 = {21,22}.
  // Factory selector0x24 binds SMALL_BACK at 0x4879AD (pointer 0x4C08F0).
  shell::MenuParams choices;
  choices.baseY = 300;
  const std::array<std::string, 2> labels{std::string((*strings)[21]), std::string((*strings)[22])};
  result.confirmLabels_ = labels;
  result.confirmBackdrop_ = *background;
  result.confirm_ = shell::renderMainMenu(*background, *font, labels, choices, cells);
  for (int i = 0; i < 2; ++i) {
    const int left = shell::labelLeft(labels[i], cells, 640);
    const int top = choices.baseY + choices.rowStep * i + choices.panelTop;
    result.confirmBoxes_[i] = {left - choices.panelPadX, top,
        left + shell::textWidth(labels[i], cells) + choices.panelPadX, top + choices.panelHeight};
  }
  // 0x49BBD0..0x49BC0A draws standalone string129 at (320,250), RGB(255,0,0).
  // flags0x840 centres it horizontally without a vertical offset.
  const std::string question((*strings)[129]);
  result.confirmQuestion_ = question;
  const std::array<shell::HudLine, 1> heading{{
      {question, shell::labelLeft(question, cells, 640), 250, {255, 0, 0}}}};
  shell::drawHud(result.confirm_, *font, heading, cells);

  // 0x49AC6B/0x49AC90 build the shared row at base340 +100, table0x4C44E0 = UK127.
  shell::MenuParams footer;
  footer.baseY = 440;
  const std::array<std::string, 1> footerLabels{std::string((*strings)[127])};
  result.mapImage_ = shell::renderMainMenu(map->image(), *font, footerLabels, footer, cells);
  const int footerLeft = shell::labelLeft(footerLabels[0], cells, footer.xWord);
  const int footerTop = footer.baseY + footer.panelTop;
  result.mapFooterBox_ = {footerLeft - footer.panelPadX, footerTop,
      footerLeft + shell::textWidth(footerLabels[0], cells) + footer.panelPadX,
      footerTop + footer.panelHeight};
  result.map_ = std::move(*map);
  result.cursor_ = std::move(*cursor);
  result.font_ = std::move(*font);
  result.mapFooterLabel_ = footerLabels[0];
  result.completionFlags_ = loadedFlags;
  result.store_ = std::move(store);
  result.languageFile_ = std::move(languageFile);
  result.savedGame_ = loadedImage;
  result.saveLoadError_ = std::move(loadError);
  return result;
}

void FrontendSession::persist() {
  if (!store_) return;
  ++saveCount_;
  if (auto saved = store_->save(savedGame_)) {
    saveError_.reset();
  } else {
    // The in-memory progress and any results screen stay; the prior file is intact.
    saveError_ = saved.error();
    std::fprintf(stderr, "save: %s (progress kept in memory only)\n", saveError_->c_str());
  }
}

void FrontendSession::refreshMap() {
  map_.setProgress(completionFlags_, acceptedSound_.unlockAllLevels);
  shell::MenuParams footer;
  footer.baseY = 440;
  const std::array<std::string, 1> labels{mapFooterLabel_};
  mapImage_ = shell::renderMainMenu(map_.image(), font_, labels, footer, shell::fontCells());
}

shell::Outcome FrontendSession::confirmChoice(bool yes) {
  const auto result = state_.confirmChoice(yes);
  if (result == shell::Outcome::Handled && yes) {
    // New Game resets the session and writes the zero image to the clone's own
    // store (0x49BCF4 reset 0x45F883, then 0x49BCFE write 0x45F94D; the reset
    // zeroes the flag and every A/B/C). Never touch the installed save.
    completionFlags_.fill(0);
    refreshMap();
    selectedNode_.reset(); selectedLevel_.reset(); outcome_.reset();
    results_.reset();
    savedGame_ = save::SavedGame{};
    persist();
  }
  return result;
}

const Image& FrontendSession::image() const {
  switch (screen()) {
    case shell::Screen::Menu:
      if (menuImageHighlight_ != menuFocus()) {
        shell::MenuParams params;
        params.highlight = menuFocus();
        menu_ = shell::renderMainMenu(menuBackdrop_, font_, menuLabels_, params, shell::fontCells());
        menuImageHighlight_ = menuFocus();
      }
      return menu_;
    case shell::Screen::Confirm:
      if (confirmImageFocus_ != menuFocus()) {
        shell::MenuParams choices;
        choices.baseY = 300;
        choices.highlight = menuFocus();
        // Recompose the choices and independent red heading from owned assets.
        confirm_ = shell::renderMainMenu(confirmBackdrop_, font_, confirmLabels_, choices,
                                         shell::fontCells());
        const auto& question = confirmQuestion_;
        const std::array<shell::HudLine, 1> heading{{
            {question, shell::labelLeft(question, shell::fontCells(), 640), 250, {255, 0, 0}}}};
        shell::drawHud(confirm_, font_, heading, shell::fontCells());
        confirmImageFocus_ = menuFocus();
      }
      return confirm_;
    case shell::Screen::Language: return languageImage_;
    case shell::Screen::SoundOptions:
      soundImage_ = renderSoundOptions(menuBackdrop_);
      return soundImage_;
    case shell::Screen::KeyboardShortcuts:
      if (shortcutsImage_.rgba.empty())
        shortcutsImage_ = SoundMenu::renderShortcuts(menuBackdrop_, font_);
      return shortcutsImage_;
    case shell::Screen::Results: return results_->image(menuFocus());
    default: return mapImage_; // Level/Dialog composition belongs to the level renderer.
  }
}

Image FrontendSession::composeFrame() const {
  Image composed = image();
  if (screen() == shell::Screen::Map && focused_) {
    // 49B2F6 shows the indexed-mask tooltip for state2/3. Compose over the base
    // map each frame. Provisional clone policy hides it while unfocused.
    if (const auto node = map_.pickNode(pointerX_, pointerY_)) {
      map_.drawHover(composed,font_,*node);
    }
  }
  cursor_.draw(composed, cursorState(), pointerX_, pointerY_);
  return composed;
}

shell::Outcome FrontendSession::pressReturn() {
  if (screen() == shell::Screen::Language) {
    const auto outcome = state_.press(shell::Key::Return);
    if (outcome == shell::Outcome::Handled && languageFile_) {
      // This sidecar holds only an explicit choice. Display already uses UK.
      if (auto written = languageFile_->write(std::as_bytes(std::span(kEnglishLanguageCode)));
          !written)
        std::fprintf(stderr, "language settings: %s\n", written.error().c_str());
    }
    return outcome;
  }
  if (screen() == shell::Screen::SoundOptions) {
    if (menuFocus() == SoundMenu::Accept) return closeSoundOptions(true);
    if (menuFocus() == SoundMenu::KeyboardShortcuts) return state_.openKeyboardShortcuts();
    if (menuFocus() == SoundMenu::UnlockAll)
      editingSound_.unlockAllLevels = !editingSound_.unlockAllLevels;
    else
      adjustSound(menuFocus() == SoundMenu::WindowSize ? 1 : -1);
    return shell::Outcome::Handled;
  }
  // SDL Return activates the footer row focused by the native shared builder.
  if (screen() == shell::Screen::Map) return state_.activateMapFooter();
  // 49BC59 dispatches the focused row. No returns to Menu5.
  if (screen() == shell::Screen::Confirm) return confirmChoice(menuFocus() == 0);
  return state_.press(shell::Key::Return);
}

shell::Outcome FrontendSession::pressEscape() {
  if (screen() == shell::Screen::SoundOptions) return closeSoundOptions(false);
  return state_.press(shell::Key::Esc);
}

bool FrontendSession::moveMenuFocus(int direction) {
  if (direction != -1 && direction != 1) return false;
  if (screen() == shell::Screen::SoundOptions) {
    constexpr int rows = SoundMenu::RowCount;
    state_.setHighlight((menuFocus() + direction + rows) % rows);
    return true;
  }
  if (screen() != shell::Screen::Menu && screen() != shell::Screen::Confirm) return false;
  if (screen() == shell::Screen::Confirm) {
    // READ488A92/4891BB: both links of the two-row ring point to the other choice.
    state_.setHighlight((menuFocus() + direction + 2) % 2);
    return true;
  }
  constexpr int rows = static_cast<int>(shell::kLabelIndices.size());
  state_.setHighlight((menuFocus() + direction + rows) % rows);
  return true;
}

shell::Outcome FrontendSession::openLevelMenu() {
  return state_.openLevelMenu();
}

shell::Outcome FrontendSession::closeLevelMenu(bool paused) {
  if (screen() == shell::Screen::SoundOptions && state_.uiState() == shell::kUiDialog)
    finishSoundEdit(true); // O tears down UI2 directly and keeps the live values.
  return state_.closeLevelMenu(paused);
}

void FrontendSession::configureSound(save::SoundSettings levels, std::shared_ptr<save::SaveSink> file,
                                     std::function<void(save::SoundSettings, bool)> preview) {
  const bool unlockChanged = acceptedSound_.unlockAllLevels != levels.unlockAllLevels;
  acceptedSound_ = editingSound_ = levels;
  soundFile_ = std::move(file);
  previewSound_ = std::move(preview);
  if (unlockChanged) refreshMap();
}

int FrontendSession::soundBaseY() const {
  // Keep the existing Music position for each Options caller.
  return state_.uiState() == shell::kUiDialog ? 100 : 204;
}

shell::Outcome FrontendSession::openSoundOptions() {
  return state_.openSoundOptions();
}

void FrontendSession::finishSoundEdit(bool accept) {
  if (!accept) {
    editingSound_ = acceptedSound_;
    if (previewSound_) previewSound_(editingSound_, false);
    return;
  }
  soundWritePending_ |= acceptedSound_ != editingSound_;
  const bool unlockChanged = acceptedSound_.unlockAllLevels != editingSound_.unlockAllLevels;
  acceptedSound_ = editingSound_;
  if (unlockChanged) refreshMap();
  if (soundWritePending_ && soundFile_) {
    if (auto written = acceptedSound_.save(*soundFile_); !written) {
      std::fprintf(stderr, "sound settings: %s (kept in memory)\n", written.error().c_str());
    } else {
      soundWritePending_ = false;
    }
  }
}

shell::Outcome FrontendSession::closeSoundOptions(bool accept) {
  if (screen() != shell::Screen::SoundOptions) return shell::Outcome::Ignored;
  finishSoundEdit(accept);
  return state_.returnFromSoundOptions();
}

bool FrontendSession::adjustSound(int direction) {
  if (screen() != shell::Screen::SoundOptions || menuFocus() >= SoundMenu::KeyboardShortcuts ||
      (direction != -1 && direction != 1)) return false;
  if (menuFocus() == SoundMenu::UnlockAll) {
    editingSound_.unlockAllLevels = direction > 0;
    return true;
  }
  if (menuFocus() == SoundMenu::WindowSize) {
    constexpr int choices = kMaxFixedWindowScale + 1;
    editingSound_.windowScale = (editingSound_.windowScale + direction + choices) % choices;
    return true;
  }
  const bool sfx = menuFocus() == SoundMenu::Sfx;
  int& percent = sfx ? editingSound_.sfx : editingSound_.music;
  // Provisional SDL increment: one displayed segment per click or key press.
  const int step = std::clamp((percent * 16 + 50) / 100 + direction, 0, 16);
  percent = (step * 100 + 8) / 16;
  if (previewSound_) previewSound_(editingSound_, sfx);
  return true;
}

void FrontendSession::hoverSound(int x, int y) {
  if (screen() == shell::Screen::SoundOptions)
    if (auto hit = soundMenu_.hit(x, y, soundBaseY(), editingSound_, windowScaleMenu_))
      state_.setHighlight(hit->row);
}

Image FrontendSession::renderSoundOptions(const Image& backdrop) const {
  Image frame = backdrop;
  prepareSoundOptions(frame.width).draw(frame);
  return frame;
}

SoundMenu::Prepared FrontendSession::prepareSoundOptions(int viewportWidth) const {
  return soundMenu_.prepare(viewportWidth, font_, soundBaseY(), menuFocus(), editingSound_, windowScaleMenu_);
}

void FrontendSession::hoverMenu(int x, int y) {
  int row;
  switch (screen()) {
    case shell::Screen::Menu: row = shell::menuPanelAt(menuBoxes_, x, y); break;
    case shell::Screen::Confirm: row = shell::menuPanelAt(confirmBoxes_, x, y); break;
    case shell::Screen::Results: row = results_->rowAt(x, y); break;
    default: return;
  }
  if (row >= 0) state_.setHighlight(row);
}

bool FrontendSession::click(int x, int y) {
  if (screen() == shell::Screen::Menu) {
    const int row = shell::menuPanelAt(menuBoxes_, x, y);
    if (row < 0) return false;
    state_.setHighlight(row);
    return pressReturn() == shell::Outcome::Handled;
  }
  if (screen() == shell::Screen::Language)
    return shell::menuPanelAt(languageBoxes_, x, y) == 0 && pressReturn() == shell::Outcome::Handled;
  if (screen() == shell::Screen::SoundOptions) {
    const auto hit = soundMenu_.hit(x, y, soundBaseY(), editingSound_, windowScaleMenu_);
    if (!hit) return false;
    state_.setHighlight(hit->row);
    if (hit->direction == 0) return pressReturn() == shell::Outcome::Handled;
    return adjustSound(hit->direction);
  }
  if (screen() == shell::Screen::KeyboardShortcuts)
    return SoundMenu::shortcutsBackAt(x, y) && pressReturn() == shell::Outcome::Handled;
  if (screen() == shell::Screen::Confirm) {
    const int row = shell::menuPanelAt(confirmBoxes_, x, y);
    return row >= 0 && confirmChoice(row == 0) == shell::Outcome::Handled;
  }
  if (screen() == shell::Screen::Map && x >= mapFooterBox_[0] && x < mapFooterBox_[2] &&
      y >= mapFooterBox_[1] && y < mapFooterBox_[3])
    return state_.activateMapFooter() == shell::Outcome::Handled;
  if (screen() == shell::Screen::Results && results_) {
    switch (results_->actionAt(x, y)) {
      case ResultAction::Continue:
        selectedNode_.reset();
        return state_.activateResultsContinue() == shell::Outcome::Handled;
      case ResultAction::Retry:
        if (!outcome_ || state_.activateResultsRetry() != shell::Outcome::Handled) return false;
        selectedLevel_ = outcome_->level;
        levelIntroEnabled_ = false; // READ48656F: retry skips camera introduction.
        outcome_.reset();
        return true;
      case ResultAction::Quit:
        if (state_.activateResultsQuit() != shell::Outcome::Handled) return false;
        levelIntroEnabled_ = true; // READ48658A, independent of map destination.
        // Clear the selected launch. The native path at 5D59F0 requires a reload request.
        selectedLevel_.reset(); selectedNode_.reset(); outcome_.reset(); results_.reset();
        return true;
      case ResultAction::None: return false;
    }
  }
  return clickMap(x, y);
}

std::expected<void, std::string> FrontendSession::completeLevel(
    const AssetLibrary& library, const LevelOutcome& outcome) {
  // Compare launch IDs without ASCII case (MM_Block_Bomb_A / MM_BLOCK_BOMB_A),
  // matching the resolver. Retain the map spelling for retry.
  const auto sameId = [](std::string_view a, std::string_view b) {
    return std::ranges::equal(a, b, [](unsigned char x, unsigned char y) {
      return std::toupper(x) == std::toupper(y); });
  };
  if (screen() != shell::Screen::Level || !selectedLevel_ || !selectedNode_ ||
      !sameId(outcome.level, *selectedLevel_) || map_.levelForNode(*selectedNode_) != selectedLevel_)
    return std::unexpected("result does not belong to the selected level");
  if (outcome.saved < 0 || outcome.total < outcome.saved || outcome.required < 0 ||
      outcome.secondsRemaining < 0)
    return std::unexpected("inconsistent level outcome");
  // 445498 latches success at the trigger and publishes counts at completion.
  // Prepare results assets before committing state. READ485E55 marks only a win.
  auto results = ResultsScene::build(library, outcome);
  if (!results) return std::unexpected(results.error());
  if (state_.showResults(outcome.success) != shell::Outcome::Handled)
    return std::unexpected("cannot present result outside a level");
  results_ = std::move(*results);
  outcome_ = outcome;
  outcome_->level = *selectedLevel_;
  selectedLevel_.reset();
  if (outcome.success) {
    completionFlags_[*selectedNode_] = 1;
    refreshMap();
    // READ485E55 (0x485E2B..0x485E65) marks C of the winning (group, slot) then writes
    // the save. A failed write leaves the prepared results and in-memory progress intact.
    const auto& entry = MapScene::nodeMapping()[*selectedNode_];
    savedGame_.markComplete(entry[0], entry[1]);
    persist();
  }
  return {};
}

bool FrontendSession::startDirectLevel(std::string levelId) {
  if (screen() != shell::Screen::Menu || state_.enterLevel() != shell::Outcome::Handled)
    return false;
  selectedLevel_ = std::move(levelId);
  levelIntroEnabled_ = false;
  return true;
}

bool FrontendSession::retryLevel() {
  if ((screen() != shell::Screen::Level && screen() != shell::Screen::Dialog) ||
      !selectedLevel_) return false;
  if (screen() == shell::Screen::Dialog && state_.retryLevelMenu() != shell::Outcome::Handled)
    return false;
  levelIntroEnabled_ = false; // READ48F5ED, selected launch remains owned.
  outcome_.reset(); results_.reset();
  return true;
}

bool FrontendSession::abandonLevelToMap() {
  if (state_.abandonLevelToMap() != shell::Outcome::Handled) return false;
  selectedLevel_.reset(); selectedNode_.reset(); outcome_.reset(); results_.reset();
  // 48F601 preserves the intro preference. Return to the prepared current-progress map.
  return true;
}

bool FrontendSession::clickMap(int x, int y) {
  if (screen() != shell::Screen::Map) return false;
  const auto node = map_.pickNode(x, y);
  if (!node) return false;
  const auto level = map_.levelForNode(*node);
  if (!level) return false;
  if (state_.press(shell::Key::MapNodeClick) != shell::Outcome::Handled) return false;
  selectedLevel_ = *level;
  selectedNode_ = *node;
  levelIntroEnabled_ = true; // READ49B1CC: every successful map activation.
  outcome_.reset();
  return true;
}

}  // namespace webo::app
