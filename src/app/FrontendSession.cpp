#include "app/FrontendSession.h"

#include "assets/GameData.h"
#include "shell/HudText.h"
#include "shell/MenuRender.h"

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
      loadError = loaded.error();
      std::fprintf(stderr,
                   "save: %s; starting with empty progress (the file is left untouched until a "
                   "later save succeeds)\n", loadError->c_str());
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
    case shell::Screen::Results: return results_->image();
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
  // The native shared builder focuses the sole footer row. SDL Return invokes
  // its recovered semantic action; physical Return binding was not remeasured.
  if (screen() == shell::Screen::Map) return state_.activateMapFooter();
  // READ49BC59 dispatches the focused row; observed keyboard No returns Menu5.
  if (screen() == shell::Screen::Confirm) return confirmChoice(menuFocus() == 0);
  return state_.press(shell::Key::Return);
}

bool FrontendSession::moveMenuFocus(int direction) {
  if ((screen() != shell::Screen::Menu && screen() != shell::Screen::Confirm) ||
      (direction != -1 && direction != 1)) return false;
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
  return state_.closeLevelMenu(paused);
}

bool FrontendSession::click(int x, int y) {
  // SOURCE SDL adapter: hit the rendered panels, not a recovered native menu
  // mouse predicate. Main-menu activation reuses the existing semantic route.
  if (screen() == shell::Screen::Menu) {
    for (std::size_t row = 0; row < menuBoxes_.size(); ++row) {
      if (row != 0 && row != 1 && row != 4) continue; // unsupported actions retain their old input policy
      const auto& box = menuBoxes_[row];
      if (x >= box[0] && x < box[2] && y >= box[1] && y < box[3]) {
        state_.setHighlight(static_cast<int>(row));
        return pressReturn() == shell::Outcome::Handled;
      }
    }
    return false;
  }
  if (screen() == shell::Screen::Confirm) {
    for (int i = 0; i < 2; ++i) {
      const auto& box = confirmBoxes_[i];
      if (x >= box[0] && x < box[2] && y >= box[1] && y < box[3])
        return confirmChoice(i == 0) == shell::Outcome::Handled;
    }
    return false;
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
  if (screen() != shell::Screen::Level || !selectedLevel_ || !selectedNode_ ||
      outcome.level != *selectedLevel_ || map_.levelForNode(*selectedNode_) != selectedLevel_)
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
