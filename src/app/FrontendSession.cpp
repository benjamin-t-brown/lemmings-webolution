#include "app/FrontendSession.h"

#include "assets/GameData.h"
#include "shell/HudText.h"
#include "shell/MenuRender.h"

#include <string_view>
#include <cctype>
#include <algorithm>
#include <cstdio>
#include <vector>

namespace webo::app {

std::expected<FrontendSession, std::string> FrontendSession::create(
    const AssetLibrary& library, std::shared_ptr<save::SaveStore> store) {
  // Restart half of the persistence contract. A missing file is the zero state; a
  // wrong-size or unreadable file leaves the session empty with a visible
  // diagnostic, and it is only ever replaced by a later successful save.
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
  const bool anyLoaded = std::any_of(loadedFlags.begin(), loadedFlags.end(),
                                     [](std::int32_t flag) { return flag != 0; });

  auto logo = loadTexture(library, "TEXTURES\\MAIN_LOGO.BMP");
  auto background = loadTexture(library, "TEXTURES\\SMALL_BACK.BMP");
  auto font = loadTexture(library, "TEXTURES\\^FONT.BMP");
  auto strings = loadStrings(library, "UK");
  // The zero-flag map stays the New Game reset target even when progress loads.
  auto zeroMap = MapScene::build(library, {});
  auto map = (anyLoaded && zeroMap) ? MapScene::build(library, loadedFlags) : zeroMap;
  auto cursor = RuntimeCursor::load(library);
  if (!zeroMap) return std::unexpected(zeroMap.error());
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
  // READ 0x49BBD0..0x49BC0A: independent heading at (320,250), string129,
  // RGB(255,0,0), flags0x840 (horizontal centre, no vertical offset). No panel.
  // Reuse the font alpha blitter only; no HUD-specific layout is applied.
  const std::string question((*strings)[129]);
  result.confirmQuestion_ = question;
  const std::array<shell::HudLine, 1> heading{{
      {question, shell::labelLeft(question, cells, 640), 250, {255, 0, 0}}}};
  shell::drawHud(result.confirm_, *font, heading, cells);

  // READ 0x49AC6B/0x49AC90: shared row builder, layout base340 +100,
  // table0x4C44E0 = UK127. Native screenshot corroborates [205,441,436,468].
  shell::MenuParams footer;
  footer.baseY = 440;
  const std::array<std::string, 1> footerLabels{std::string((*strings)[127])};
  result.mapImage_ = shell::renderMainMenu(map->image(), *font, footerLabels, footer, cells);
  const Image initialMapImage = anyLoaded
      ? shell::renderMainMenu(zeroMap->image(), *font, footerLabels, footer, cells)
      : result.mapImage_;
  const int footerLeft = shell::labelLeft(footerLabels[0], cells, footer.xWord);
  const int footerTop = footer.baseY + footer.panelTop;
  result.mapFooterBox_ = {footerLeft - footer.panelPadX, footerTop,
      footerLeft + shell::textWidth(footerLabels[0], cells) + footer.panelPadX,
      footerTop + footer.panelHeight};
  result.map_ = std::move(*map);
  result.cursor_ = std::move(*cursor);
  result.font_ = std::move(*font);
  result.mapFooterLabel_ = footerLabels[0];
  result.initialMap_ = *zeroMap;
  result.initialMapImage_ = initialMapImage;
  result.completionFlags_ = loadedFlags;
  result.store_ = std::move(store);
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

Image FrontendSession::renderMapImage(const MapScene& map) const {
  shell::MenuParams footer;
  footer.baseY = 440;
  const std::array<std::string, 1> labels{mapFooterLabel_};
  return shell::renderMainMenu(map.image(), font_, labels, footer, shell::fontCells());
}

shell::Outcome FrontendSession::confirmChoice(bool yes) {
  const auto result = state_.confirmChoice(yes);
  if (result == shell::Outcome::Handled && yes) {
    // New Game resets the session and writes the zero image to the clone's own
    // store (0x49BCF4 reset 0x45F883, then 0x49BCFE write 0x45F94D; the reset
    // zeroes the flag and every A/B/C). Never touch the installed save.
    completionFlags_.fill(0);
    map_ = initialMap_;
    mapImage_ = initialMapImage_;
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
    case shell::Screen::SoundOptions:
      soundImage_ = renderSoundOptions(menuBackdrop_);
      return soundImage_;
    case shell::Screen::Results: return results_->image(menuFocus());
    default: return mapImage_; // Level/Dialog composition belongs to the level renderer.
  }
}

Image FrontendSession::composeFrame() const {
  Image composed = image();
  if (screen() == shell::Screen::Map && focused_) {
    // READ 49B2F6: indexed-mask hover, state2/3 only. Each frame starts from
    // the clean map, so leaving a node removes all three lines and the clock.
    // Provisional focus policy retained: hide the block while unfocused.
    if (const auto node = map_.pickNode(pointerX_, pointerY_)) {
      map_.drawHover(composed,font_,*node);
    }
  }
  cursor_.draw(composed, cursorState(), pointerX_, pointerY_);
  return composed;
}

shell::Outcome FrontendSession::pressReturn() {
  if (screen() == shell::Screen::SoundOptions) {
    if (menuFocus() == 2) return closeSoundOptions(true);
    adjustSound(-1);
    return shell::Outcome::Handled;
  }
  // The native shared builder focuses the sole footer row. SDL Return invokes
  // its recovered semantic action; physical Return binding was not remeasured.
  if (screen() == shell::Screen::Map) return state_.activateMapFooter();
  // READ49BC59 dispatches the focused row; observed keyboard No returns Menu5.
  if (screen() == shell::Screen::Confirm) return confirmChoice(menuFocus() == 0);
  return state_.press(shell::Key::Return);
}

bool FrontendSession::moveMenuFocus(int direction) {
  if (direction != -1 && direction != 1) return false;
  if (screen() == shell::Screen::SoundOptions) {
    state_.setHighlight((menuFocus() + direction + 3) % 3);
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
  acceptedSound_ = editingSound_ = levels;
  soundFile_ = std::move(file);
  previewSound_ = std::move(preview);
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
  acceptedSound_ = editingSound_;
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
  if (screen() != shell::Screen::SoundOptions || menuFocus() >= 2 ||
      (direction != -1 && direction != 1)) return false;
  const bool sfx = menuFocus() == 1;
  int& percent = sfx ? editingSound_.sfx : editingSound_.music;
  // Provisional SDL increment: one displayed segment per click or key press.
  const int step = std::clamp((percent * 16 + 50) / 100 + direction, 0, 16);
  percent = (step * 100 + 8) / 16;
  if (previewSound_) previewSound_(editingSound_, sfx);
  return true;
}

void FrontendSession::hoverSound(int x, int y) {
  if (screen() == shell::Screen::SoundOptions)
    if (auto hit = soundMenu_.hit(x, y, soundBaseY())) state_.setHighlight(hit->row);
}

Image FrontendSession::renderSoundOptions(const Image& backdrop) const {
  return soundMenu_.render(backdrop, font_, soundBaseY(), menuFocus(), editingSound_);
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
    if (row < 0 || row == 2) return false; // Row 2 can take focus; its page is unsupported.
    state_.setHighlight(row);
    return pressReturn() == shell::Outcome::Handled;
  }
  if (screen() == shell::Screen::SoundOptions) {
    const auto hit = soundMenu_.hit(x, y, soundBaseY());
    if (!hit) return false;
    state_.setHighlight(hit->row);
    if (hit->row == 2) return closeSoundOptions(true) == shell::Outcome::Handled;
    return adjustSound(hit->direction);
  }
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
        // Clone launch ownership, not a claimed null write to native5D59F0:
        // native retains a path but requires a separate reload request.
        selectedLevel_.reset(); selectedNode_.reset(); outcome_.reset(); results_.reset();
        return true;
      case ResultAction::None: return false;
    }
  }
  return clickMap(x, y);
}

std::expected<void, std::string> FrontendSession::completeLevel(
    const AssetLibrary& library, const LevelOutcome& outcome) {
  // Playtest 6: the map stores 20 launch ids in mixed case (MM_Block_Bomb_A)
  // while the level leg reports the resolved archive id (MM_BLOCK_BOMB_A).
  // Clone policy: ids are compared without ASCII case, as the level resolver
  // does, and the session keeps the map's own spelling for retry.
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
  // READ445498: success is trigger-time latched, counts are final-call values.
  // A timeout loss can legitimately display saved>=required after late rescues.
  // Prepare all fallible assets before changing screen or progress. READ485E55
  // writes1 to the winning selected slot; a loss never clears earlier progress.
  auto results = ResultsScene::build(library, outcome);
  if (!results) return std::unexpected(results.error());
  auto flags = completionFlags_;
  if (outcome.success) flags[*selectedNode_] = 1;
  auto map = MapScene::build(library, flags);
  if (!map) return std::unexpected(map.error());
  auto mapImage = renderMapImage(*map);
  if (state_.showResults(outcome.success) != shell::Outcome::Handled)
    return std::unexpected("cannot present result outside a level");
  completionFlags_ = flags;
  map_ = std::move(*map);
  mapImage_ = std::move(mapImage);
  results_ = std::move(*results);
  outcome_ = outcome;
  outcome_->level = *selectedLevel_;
  selectedLevel_.reset();
  if (outcome.success) {
    // READ485E55 (0x485E2B..0x485E65): mark C of the winning (group, slot), then
    // write the file. Loss, retry, abandon and exit never reach this. A failed
    // write is reported by persist() and does not discard the results above.
    const auto& entry = MapScene::nodeMapping()[*selectedNode_];
    savedGame_.markComplete(entry[0], entry[1]);
    persist();
  }
  return {};
}

bool FrontendSession::retryLevel() {
  if ((screen() != shell::Screen::Level && screen() != shell::Screen::Dialog) ||
      !selectedLevel_ || !selectedNode_) return false;
  if (screen() == shell::Screen::Dialog && state_.retryLevelMenu() != shell::Outcome::Handled)
    return false;
  levelIntroEnabled_ = false; // READ48F5ED, selected launch remains owned.
  outcome_.reset(); results_.reset();
  return true;
}

bool FrontendSession::abandonLevelToMap() {
  if (state_.abandonLevelToMap() != shell::Outcome::Handled) return false;
  selectedLevel_.reset(); selectedNode_.reset(); outcome_.reset(); results_.reset();
  // Unlike loss-results Quit,48F601 does NOT change the intro preference.
  // Preserve the prepared current-progress map; no fabricated LevelOutcome.
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
