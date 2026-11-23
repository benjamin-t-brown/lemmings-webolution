#include "app/LevelMenu.h"

#include "app/WindowScale.h"
#include "assets/GameData.h"
#include "shell/MenuRender.h"

#include <utility>

namespace webo::app {
namespace {

constexpr std::array kActions{
    LevelMenuAction::Resume, LevelMenuAction::Retry,
    LevelMenuAction::MapQuit, LevelMenuAction::SoundOptions, LevelMenuAction::Examine};
constexpr std::array<std::size_t, 4> kLabelIndices{44, 48, 43, 3};
constexpr int kMenuBaseY = 100;
constexpr shell::MenuParams kTouchButtonParams{.xWord = kGameWidth, .baseY = 8, .highlight = -1};
const std::array<std::string, 1> kTouchButtonLabel{"Menu"};

bool containsStrict(int x, int y, std::size_t row, std::string_view label) {
  const auto& cells = shell::fontCells();
  const int width = shell::textWidth(label, cells);
  const int left = shell::labelLeft(label, cells, kGameWidth);
  const int top = kMenuBaseY + static_cast<int>(row) * shell::kLineHeight;
  return left < x && x < left + width && top < y && y < top + shell::kLineHeight;
}

} // namespace

std::expected<LevelMenu, std::string> LevelMenu::load(const AssetLibrary& library) {
  auto font = loadTexture(library, "TEXTURES\\^FONT.BMP");
  if (!font) return std::unexpected(font.error());
  auto strings = loadStrings(library, "UK");
  if (!strings) return std::unexpected(strings.error());
  if (font->width != 256 || font->height != 240 || strings->size() <= 48)
    return std::unexpected("level menu requires native ^FONT and UK strings through label 48");

  LevelMenu menu;
  menu.font_ = std::move(*font);
  for (std::size_t i = 0; i < kLabelIndices.size(); ++i)
    menu.labels_[i] = std::string((*strings)[kLabelIndices[i]]);
  menu.labels_.back() = "Examine";
  return menu;
}

void LevelMenu::open() {
  open_ = true;
  focusedIndex_ = 0;
}

void LevelMenu::close() { open_ = false; }

void LevelMenu::moveFocus(int direction) {
  if (!open_ || (direction != -1 && direction != 1)) return;
  constexpr int rows = static_cast<int>(kActions.size());
  focusedIndex_ = (focusedIndex_ + rows + direction) % rows;
}

void LevelMenu::hover(int x, int y) {
  if (!open_) return;
  for (std::size_t i = 0; i < kActions.size(); ++i) {
    if (containsStrict(x, y, i, labels_[i])) {
      focusedIndex_ = static_cast<int>(i);
      return;
    }
  }
  // Pointer misses preserve keyboard/programmatic focus.
}

LevelMenuAction LevelMenu::actionAt(int x, int y) const {
  if (!open_) return LevelMenuAction::None;
  for (std::size_t i = 0; i < kActions.size(); ++i)
    if (containsStrict(x, y, i, labels_[i])) return kActions[i];
  return LevelMenuAction::None;
}

LevelMenuAction LevelMenu::focusedAction() const {
  if (!open_ || focusedIndex_ < 0 || focusedIndex_ >= static_cast<int>(kActions.size()))
    return LevelMenuAction::None;
  return kActions[static_cast<std::size_t>(focusedIndex_)];
}

void LevelMenu::draw(Image& frame) const {
  if (!open_) return;
  shell::MenuParams params;
  params.baseY = kMenuBaseY;
  params.highlight = focusedIndex_;
  shell::drawMenuRows(frame, font_, labels_, params, shell::fontCells());
}

bool LevelMenu::touchButtonAt(int x, int y) const {
  const auto& cells = shell::fontCells();
  const auto& label = kTouchButtonLabel.front();
  const auto bounds = shell::menuPlateBounds(shell::labelLeft(label, cells, kTouchButtonParams.xWord),
      kTouchButtonParams.baseY, shell::textWidth(label, cells), kTouchButtonParams);
  return shell::menuPanelAt(std::span(&bounds, 1), x, y) == 0;
}

void LevelMenu::drawTouchButton(Image& frame) const {
  shell::drawMenuRows(frame, font_, kTouchButtonLabel, kTouchButtonParams, shell::fontCells());
}

} // namespace webo::app
