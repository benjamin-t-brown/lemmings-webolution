#include "app/ResultsScene.h"

#include "assets/GameData.h"
#include "shell/HudText.h"
#include "shell/MenuRender.h"

#include <utility>

namespace webo::app {
namespace {
constexpr int firstVisibleRowY = 349;
}

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
  buttons.baseY = firstVisibleRowY;
  if (outcome.success) {
    result.labels_.emplace_back((*strings)[47]);
    result.actions_[0] = ResultAction::Continue;
  } else {
    result.labels_ = {std::string((*strings)[48]), std::string((*strings)[47]), std::string((*strings)[43])};
    result.actions_ = {ResultAction::Retry, ResultAction::Continue, ResultAction::Quit};
  }
  for (std::size_t i = 0; i < result.labels_.size(); ++i) {
    const auto& label = result.labels_[i];
    const int left = shell::labelLeft(label, cells, 640);
    const int top = buttons.baseY + static_cast<int>(i) * buttons.rowStep + buttons.panelTop;
    result.boxes_[i] = {left - buttons.panelPadX, top,
        left + shell::textWidth(label, cells) + buttons.panelPadX, top + buttons.panelHeight};
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
  result.backdrop_ = std::move(*background);
  result.font_ = std::move(*font);
  shell::drawHud(result.backdrop_, result.font_, lines, cells);
  return result;
}

const Image& ResultsScene::image(int highlight) const {
  if (image_.rgba.empty() || highlight != imageHighlight_) {
    shell::MenuParams buttons;
    buttons.baseY = firstVisibleRowY;
    buttons.highlight = highlight;
    image_ = shell::renderMainMenu(backdrop_, font_, labels_, buttons, shell::fontCells());
    imageHighlight_ = highlight;
  }
  return image_;
}

int ResultsScene::rowAt(int x, int y) const {
  return shell::menuPanelAt(boxes_, x, y);
}

ResultAction ResultsScene::actionAt(int x, int y) const {
  const int row = rowAt(x, y);
  return row < 0 ? ResultAction::None : actions_[row];
}

} // namespace webo::app
