#include "app/FrontendSession.h"

#include "assets/GameData.h"
#include "shell/HudText.h"
#include "shell/MenuRender.h"

#include <vector>

namespace webo::app {

std::expected<FrontendSession, std::string> FrontendSession::create(
    const AssetLibrary& library) {
  auto logo = loadTexture(library, "TEXTURES\\MAIN_LOGO.BMP");
  auto background = loadTexture(library, "TEXTURES\\SMALL_BACK.BMP");
  auto font = loadTexture(library, "TEXTURES\\^FONT.BMP");
  auto strings = loadStrings(library, "UK");
  auto map = MapScene::build(library, {});
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
  const auto& cells = shell::fontCells();
  for (const auto index : shell::kLabelIndices)
    result.menuLabels_.emplace_back((*strings)[index]);
  result.menuBackdrop_ = *logo;
  shell::MenuParams menuParams;
  menuParams.highlight = result.menuFocus();
  result.menu_ = shell::renderMainMenu(*logo, *font, result.menuLabels_, menuParams, cells);
  result.firstMenuLeft_ = shell::labelLeft(result.menuLabels_[0], cells, 640) - menuParams.panelPadX;
  result.firstMenuRight_ = result.firstMenuLeft_ + shell::textWidth(result.menuLabels_[0], cells)
                           + 2 * menuParams.panelPadX;

  // READ 0x49BB6B: base200 + shared row offset100, table 0x4C4514 = {21,22}.
  // Factory selector0x24 binds SMALL_BACK at 0x4879AD (pointer 0x4C08F0).
  shell::MenuParams choices;
  choices.baseY = 300;
  const std::array<std::string, 2> labels{std::string((*strings)[21]), std::string((*strings)[22])};
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
  const std::array<shell::HudLine, 1> heading{{
      {question, shell::labelLeft(question, cells, 640), 250, {255, 0, 0}}}};
  shell::drawHud(result.confirm_, *font, heading, cells);

  // READ 0x49AC6B/0x49AC90: shared row builder, layout base340 +100,
  // table0x4C44E0 = UK127. Native screenshot corroborates [205,441,436,468].
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
  result.initialMap_ = result.map_;
  result.initialMapImage_ = result.mapImage_;
  return result;
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
    // New Game resets this session only. Never touch the installed save.
    completionFlags_.fill(0);
    map_ = initialMap_;
    mapImage_ = initialMapImage_;
    selectedNode_.reset(); selectedLevel_.reset(); outcome_.reset();
    results_.reset();
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
    case shell::Screen::Confirm: return confirm_;
    case shell::Screen::Results: return results_->image();
    default: return mapImage_; // Level/Dialog composition belongs to the level renderer.
  }
}

Image FrontendSession::composeFrame() const {
  Image composed = image();
  cursor_.draw(composed, cursorState(), pointerX_, pointerY_);
  return composed;
}

shell::Outcome FrontendSession::pressReturn() {
  // The native shared builder focuses the sole footer row. SDL Return invokes
  // its recovered semantic action; physical Return binding was not remeasured.
  if (screen() == shell::Screen::Map) return state_.activateMapFooter();
  if (screen() == shell::Screen::Confirm) return confirmChoice(true);
  return state_.press(shell::Key::Return);
}

bool FrontendSession::moveMenuFocus(int direction) {
  if (screen() != shell::Screen::Menu || (direction != -1 && direction != 1)) return false;
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
  // mouse predicate. Only New Game is implemented on the main menu.
  if (screen() == shell::Screen::Menu) {
    if (x < firstMenuLeft_ || x >= firstMenuRight_ || y < 321 || y >= 348) return false;
    // Preserve the existing explicit New Game click route independent of the
    // keyboard-selected row; hover and other row clicks remain unsupported.
    state_.setHighlight(0);
    return pressReturn() == shell::Outcome::Handled;
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
