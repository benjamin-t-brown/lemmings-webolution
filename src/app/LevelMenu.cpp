#include "app/LevelMenu.h"

#include "app/EnglishStrings.h"
#include "app/LevelLayout.h"
#include "app/WindowScale.h"

#include <utility>

namespace webo::app {
namespace {

constexpr int kMenuBaseY = 100;
constexpr int kTitleTop = 60;
constexpr int kTitleShadow = 2;
constexpr int kTitleLineStep = shell::kLineHeight + kTitleShadow;
constexpr int kPortraitMenuBaseY = 40;
constexpr int kPortraitRowStep = 40, kPortraitRowHeight = 38;
constexpr shell::MenuParams kTouchButtonParams{.xWord = kGameWidth, .baseY = 8, .highlight = -1};
const std::array<std::string, 1> kTouchButtonLabel{"Menu"};

LevelButton portraitMenuRow(int baseY, int row, std::string_view label) {
  return portraitTextButton(label, baseY + row * kPortraitRowStep, kPortraitRowHeight);
}

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
  const auto title = level && !level->name.empty() ? std::string_view(level->name) : levelId;
  menu.landscapeTitle_ = layoutTitle(title, false);
  menu.portraitTitle_ = layoutTitle(title, true);
  return menu;
}

LevelMenu::TitleLayout LevelMenu::layoutTitle(std::string_view title, bool portrait) {
  const int width = portrait ? kPortraitWidth : kGameWidth;
  const int maxWidth = width - 2 * (16 + kTitleShadow);
  const int titleTop = portrait ? 0 : kTitleTop;
  const int rowBaseY = portrait ? kPortraitMenuBaseY : kMenuBaseY;
  constexpr int landscapeMaxLines = 1 + (kGameHeight - kMenuBaseY -
      static_cast<int>(kRows.size()) * shell::kLineHeight) / kTitleLineStep;
  constexpr int portraitMaxLines = 3;
  const int maxLines = portrait ? portraitMaxLines : landscapeMaxLines;
  TitleLayout result{.lines = {}, .rowBaseY = rowBaseY};
  // Clone heading: centre the map hover's black-over-red text above the rows.
  // Wrap on spaces when possible, or at a glyph boundary in a long word.
  for (int line = 0; line < maxLines && !title.empty(); ++line) {
    auto [count, textWidth] = fitTitleLine(title, maxWidth);
    const bool ellipsize = line + 1 == maxLines && count < title.size();
    constexpr std::string_view ellipsis = "...";
    if (ellipsize) {
      const int ellipsisWidth = shell::textWidth(ellipsis, shell::fontCells());
      const auto fitted = fitTitleLine(title, maxWidth - ellipsisWidth);
      count = fitted.first;
      textWidth = fitted.second + ellipsisWidth;
    }
    std::string text(title.substr(0, count));
    if (ellipsize) text += ellipsis;
    const int x = (width - textWidth) / 2;
    const int y = titleTop + line * kTitleLineStep;
    const shell::Rgb shadow = portrait ? shell::Rgb{0, 0, 0} : shell::Rgb{220, 50, 50};
    const shell::Rgb colour = portrait ? shell::Rgb{255, 216, 120} : shell::Rgb{0, 0, 0};
    result.lines.push_back({text, x + kTitleShadow, y + kTitleShadow, shadow});
    result.lines.push_back({std::move(text), x, y, colour});
    result.rowBaseY = rowBaseY + line * kTitleLineStep;
    const auto next = title.find_first_not_of(' ', count);
    if (next == std::string_view::npos) break;
    title.remove_prefix(next);
  }
  return result;
}

int LevelMenu::portraitMenuTop(LevelLayout layout) const {
  constexpr int rowsHeight = (static_cast<int>(kRows.size()) - 1) * kPortraitRowStep + kPortraitRowHeight;
  return (layout.height() - portraitTitle_.rowBaseY - rowsHeight) / 2;
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

int LevelMenu::rowAt(int x, int y, LevelLayout layout) const {
  if (!open_) return -1;
  const int baseY = layout.portrait() ? portraitMenuTop(layout) + portraitTitle_.rowBaseY
                                      : landscapeTitle_.rowBaseY;
  for (int i = 0; i < static_cast<int>(kRows.size()); ++i)
    if (layout.portrait() ? portraitMenuRow(baseY, i, labels_[i]).contains(x, y)
                          : containsStrict(x, y, baseY, i, labels_[i]))
      return i;
  return -1;
}

void LevelMenu::hover(int x, int y, LevelLayout layout) {
  // Pointer misses preserve keyboard/programmatic focus.
  if (const int row = rowAt(x, y, layout); row >= 0) focusedIndex_ = row;
}

LevelMenuAction LevelMenu::actionAt(int x, int y, LevelLayout layout) const {
  const int row = rowAt(x, y, layout);
  return row >= 0 ? kRows[row].action : LevelMenuAction::None;
}

LevelMenuAction LevelMenu::focusedAction() const {
  if (!open_ || focusedIndex_ < 0 || focusedIndex_ >= static_cast<int>(kRows.size()))
    return LevelMenuAction::None;
  return kRows[static_cast<std::size_t>(focusedIndex_)].action;
}

void LevelMenu::draw(Image& frame, LevelLayout layout) const {
  if (!open_) return;
  const auto& cells = shell::fontCells();
  if (layout.portrait()) {
    const int top = portraitMenuTop(layout);
    auto lines = portraitTitle_.lines;
    for (auto& line : lines) line.y += top;
    shell::drawHud(frame, font_, lines, cells);
    for (int i = 0; i < static_cast<int>(kRows.size()); ++i) {
      const auto row = portraitMenuRow(top + portraitTitle_.rowBaseY, i, labels_[i]);
      auto params = portraitButtonParams(row, labels_[i]);
      params.highlight = i == focusedIndex_ ? 0 : -1;
      shell::drawMenuRows(frame, font_, std::span(&labels_[i], 1), params, cells);
    }
    return;
  }
  shell::drawHud(frame, font_, landscapeTitle_.lines, cells);
  shell::MenuParams params;
  params.baseY = landscapeTitle_.rowBaseY;
  params.highlight = focusedIndex_;
  shell::drawMenuRows(frame, font_, labels_, params, cells);
}

bool LevelMenu::touchButtonAt(int x, int y, bool portrait) const {
  if (portrait) return kPortraitMenu.contains(x, y);
  const auto& cells = shell::fontCells();
  const auto& label = kTouchButtonLabel.front();
  const auto bounds = shell::menuPlateBounds(shell::labelLeft(label, cells, kTouchButtonParams.xWord),
      kTouchButtonParams.baseY, shell::textWidth(label, cells), kTouchButtonParams);
  return shell::menuPanelAt(std::span(&bounds, 1), x, y) == 0;
}

void LevelMenu::drawTouchButton(Image& frame, bool portrait) const {
  const auto params = portrait ? portraitButtonParams(kPortraitMenu, kTouchButtonLabel.front())
                               : kTouchButtonParams;
  shell::drawMenuRows(frame, font_, kTouchButtonLabel, params, shell::fontCells());
}

} // namespace webo::app
