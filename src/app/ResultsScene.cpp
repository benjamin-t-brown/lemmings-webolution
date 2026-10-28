#include "app/ResultsScene.h"

#include "assets/GameData.h"
#include "shell/HudText.h"
#include "shell/MenuRender.h"

#include <vector>

namespace webo::app {

std::expected<ResultsScene, std::string> ResultsScene::build(
    const AssetLibrary& library, const LevelOutcome& outcome) {
  auto background = loadTexture(library, "TEXTURES\\SMALL_BACK.BMP");
  auto font = loadTexture(library, "TEXTURES\\^FONT.BMP");
  auto strings = loadStrings(library, "UK");
  if (!background) return std::unexpected(background.error());
  if (!font) return std::unexpected(font.error());
  if (!strings) return std::unexpected(strings.error());
  if (background->width != 640 || background->height != 480 || strings->size() <= 122)
    return std::unexpected("results require native-size SMALL_BACK and UK strings through122");

  ResultsScene result;
  const auto& cells = shell::fontCells();
  // Native row0 is an empty widget whose draw bit is cleared. Start at row1
  // rather than painting a blank panel, and don't guess repaired focus.
  shell::MenuParams buttons;
  buttons.baseY = 349;
  buttons.highlight = -1;
  std::vector<std::string> labels;
  if (outcome.success) {
    labels.emplace_back((*strings)[47]);
    result.actions_[0] = ResultAction::Continue;
  } else {
    labels = {std::string((*strings)[48]), std::string((*strings)[47]), std::string((*strings)[43])};
    result.actions_ = {ResultAction::Retry, ResultAction::Continue, ResultAction::Quit};
  }
  result.image_ = shell::renderMainMenu(*background, *font, labels, buttons, cells);
  for (std::size_t i = 0; i < labels.size(); ++i) {
    const int left = shell::labelLeft(labels[i], cells, 640);
    const int top = buttons.baseY + static_cast<int>(i) * buttons.rowStep + buttons.panelTop;
    result.boxes_[i] = {left - buttons.panelPadX, top,
        left + shell::textWidth(labels[i], cells) + buttons.panelPadX, top + buttons.panelHeight};
  }
  std::vector<shell::HudLine> lines;
  const auto addLine = [&](const std::string& text, int y) {
    lines.push_back({text, shell::labelLeft(text, cells, 640), y, {255, 0, 0}});
  };
  const std::string title((*strings)[outcome.success ? 50 : 49]);
  std::size_t start = 0;
  int y = 220;
  do {
    const auto end = title.find('^', start);
    addLine(title.substr(start, end == std::string::npos ? end : end - start), y);
    if (end == std::string::npos) break;
    start = end + 1;
    y += shell::kLineHeight;
  } while (start <= title.size());
  addLine(std::string((*strings)[121]) + " " + std::to_string(outcome.required), 280);
  addLine(std::string((*strings)[122]) + " " + std::to_string(outcome.saved), 310);
  shell::drawHud(result.image_, *font, lines, cells);
  return result;
}

ResultAction ResultsScene::actionAt(int x, int y) const {
  for (std::size_t i = 0; i < boxes_.size(); ++i) {
    const auto& b = boxes_[i];
    if (x >= b[0] && y >= b[1] && x < b[2] && y < b[3]) return actions_[i];
  }
  return ResultAction::None;
}

} // namespace webo::app
