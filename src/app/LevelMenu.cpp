#include "app/LevelMenu.h"

#include "app/EnglishStrings.h"
#include "app/WindowScale.h"

#include <utility>

namespace webo::app {
namespace {

constexpr int kMenuBaseY = 100;
constexpr int kTitleTop = 60;
constexpr int kTitleShadow = 2;
constexpr int kTitleLineStep = shell::kLineHeight + kTitleShadow;
constexpr int kTitleMaxWidth = kGameWidth - 2 * (16 + kTitleShadow);
constexpr shell::MenuParams kTouchButtonParams{.xWord = kGameWidth, .baseY = 8, .highlight = -1};
const std::array<std::string, 1> kTouchButtonLabel{"Menu"};

bool containsStrict(int x, int y, int baseY, std::size_t row, std::string_view label) {
  const auto& cells = shell::fontCells();
  const int width = shell::textWidth(label, cells);
  const int left = shell::labelLeft(label, cells, kGameWidth);
  const int top = baseY + static_cast<int>(row) * shell::kLineHeight;
  return left < x && x < left + width && top < y && y < top + shell::kLineHeight;
}

// Use the same literal glyph advances as drawHud, including a literal '^'.
std::pair<std::size_t, int> fitTitleLine(std::string_view title, int maxWidth) {
  const auto& cells = shell::fontCells();
  std::size_t count = 0, space = 0;
  int width = 0, widthBeforeSpace = 0;
  for (; count < title.size(); ++count) {
    const auto ch = static_cast<unsigned char>(title[count]);
    if (ch == ' ') {
      space = count;
      widthBeforeSpace = width;
    }
    const unsigned index = ch - shell::kFirstChar;
    const int advance = index < cells.size() ? cells[index].width : 0;
    if (width + advance > maxWidth) break;
    width += advance;
  }
  if (count < title.size() && space > 0) return {space, widthBeforeSpace};
  return {count, width};
}

} // namespace

std::expected<LevelMenu, std::string> LevelMenu::load(const AssetLibrary& library,
                                                      std::string_view levelId) {
  auto font = loadTexture(library, "TEXTURES\\^FONT.BMP");
  if (!font) return std::unexpected(font.error());
  auto strings = loadEnglishStrings(library);
  if (!strings) return std::unexpected(strings.error());
  if (font->width != 256 || font->height != 240 || strings->size() <= 48)
    return std::unexpected("level menu requires native ^FONT and UK strings through label 48");

  LevelMenu menu;
  menu.font_ = std::move(*font);
  for (std::size_t i = 0; i < kRows.size(); ++i) {
    const auto& row = kRows[i];
    menu.labels_[i] = row.labelIndex ? (*strings)[*row.labelIndex] : row.cloneLabel;
  }
  const auto levels = loadLevelMap(library);
  const auto* level = levels ? findLevelMapEntry(*levels, levelId) : nullptr;
  menu.layoutTitle(level && !level->name.empty() ? std::string_view(level->name) : levelId);
  return menu;
}

void LevelMenu::layoutTitle(std::string_view title) {
  constexpr int kTitleMaxLines = 1 + (kGameHeight - kMenuBaseY -
      static_cast<int>(kRows.size()) * shell::kLineHeight) / kTitleLineStep;
  rowBaseY_ = kMenuBaseY;
  // Clone heading: centre the map hover's black-over-red text above the rows.
  // Wrap on spaces when possible, or at a glyph boundary in a long word.
  for (int line = 0; line < kTitleMaxLines && !title.empty(); ++line) {
    auto [count, width] = fitTitleLine(title, kTitleMaxWidth);
    const bool ellipsize = line + 1 == kTitleMaxLines && count < title.size();
    constexpr std::string_view ellipsis = "...";
    if (ellipsize) {
      const int ellipsisWidth = shell::textWidth(ellipsis, shell::fontCells());
      const auto fitted = fitTitleLine(title, kTitleMaxWidth - ellipsisWidth);
      count = fitted.first;
      width = fitted.second + ellipsisWidth;
    }
    std::string text(title.substr(0, count));
    if (ellipsize) text += ellipsis;
    const int x = (kGameWidth - width) / 2;
    const int y = kTitleTop + line * kTitleLineStep;
    titleLines_.push_back({text, x + kTitleShadow, y + kTitleShadow, {220, 50, 50}});
    titleLines_.push_back({std::move(text), x, y, {0, 0, 0}});
    rowBaseY_ = kMenuBaseY + line * kTitleLineStep;
    const auto next = title.find_first_not_of(' ', count);
    if (next == std::string_view::npos) break;
    title.remove_prefix(next);
  }
}

void LevelMenu::open() {
  open_ = true;
  focusedIndex_ = 0;
}

void LevelMenu::close() { open_ = false; }

void LevelMenu::moveFocus(int direction) {
  if (!open_ || (direction != -1 && direction != 1)) return;
  constexpr int rows = static_cast<int>(kRows.size());
  focusedIndex_ = (focusedIndex_ + rows + direction) % rows;
}

void LevelMenu::hover(int x, int y) {
  if (!open_) return;
  for (std::size_t i = 0; i < kRows.size(); ++i) {
    if (containsStrict(x, y, rowBaseY_, i, labels_[i])) {
      focusedIndex_ = static_cast<int>(i);
      return;
    }
  }
  // Pointer misses preserve keyboard/programmatic focus.
}

LevelMenuAction LevelMenu::actionAt(int x, int y) const {
  if (!open_) return LevelMenuAction::None;
  for (std::size_t i = 0; i < kRows.size(); ++i)
    if (containsStrict(x, y, rowBaseY_, i, labels_[i])) return kRows[i].action;
  return LevelMenuAction::None;
}

LevelMenuAction LevelMenu::focusedAction() const {
  if (!open_ || focusedIndex_ < 0 || focusedIndex_ >= static_cast<int>(kRows.size()))
    return LevelMenuAction::None;
  return kRows[static_cast<std::size_t>(focusedIndex_)].action;
}

void LevelMenu::draw(Image& frame) const {
  if (!open_) return;
  shell::drawHud(frame, font_, titleLines_, shell::fontCells());
  shell::drawMenuRows(frame, prepareRows());
}

shell::PreparedMenuRows LevelMenu::prepareRows() const {
  shell::MenuParams params;
  params.baseY = rowBaseY_;
  params.highlight = focusedIndex_;
  return shell::prepareMenuRows(font_, labels_, params, shell::fontCells());
}

bool LevelMenu::touchButtonAt(int x, int y) const {
  const auto& cells = shell::fontCells();
  const auto& label = kTouchButtonLabel.front();
  const auto bounds = shell::menuPlateBounds(shell::labelLeft(label, cells, kTouchButtonParams.xWord),
      kTouchButtonParams.baseY, shell::textWidth(label, cells), kTouchButtonParams);
  return shell::menuPanelAt(std::span(&bounds, 1), x, y) == 0;
}

void LevelMenu::drawTouchButton(Image& frame) const {
  shell::drawMenuRows(frame, prepareTouchButton());
}

shell::PreparedMenuRows LevelMenu::prepareTouchButton() const {
  return shell::prepareMenuRows(font_, kTouchButtonLabel, kTouchButtonParams, shell::fontCells());
}

} // namespace webo::app
