#include "app/LevelMenu.h"

#include "assets/GameData.h"
#include "shell/MenuRender.h"

#include <utility>
#include <vector>

namespace webo::app {
namespace {

constexpr std::array<LevelMenuAction, 4> kActions{
    LevelMenuAction::Resume, LevelMenuAction::Retry,
    LevelMenuAction::MapQuit, LevelMenuAction::UnsupportedOptions};
constexpr std::array<std::size_t, 4> kLabelIndices{44, 48, 43, 3};

bool containsStrict(int x, int y, std::size_t row,
                    const std::array<std::string, 4>& labels) {
  const auto& cells = shell::fontCells();
  const int width = shell::textWidth(labels[row], cells);
  const int left = shell::labelLeft(labels[row], cells, 640);
  const int top = 100 + static_cast<int>(row) * shell::kLineHeight;
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
  return menu;
}

void LevelMenu::open() {
  open_ = true;
  focusedIndex_ = 0;
}

void LevelMenu::close() { open_ = false; }

void LevelMenu::hover(int x, int y) {
  if (!open_) return;
  for (std::size_t i = 0; i < kActions.size(); ++i) {
    if (containsStrict(x, y, i, labels_)) {
      focusedIndex_ = static_cast<int>(i);
      return;
    }
  }
  // Pointer misses preserve keyboard/programmatic focus.
}

LevelMenuAction LevelMenu::actionAt(int x, int y) const {
  if (!open_) return LevelMenuAction::None;
  for (std::size_t i = 0; i < kActions.size(); ++i)
    if (containsStrict(x, y, i, labels_)) return kActions[i];
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
  params.baseY = 100;
  params.highlight = focusedIndex_;
  const std::vector<std::string> labels(labels_.begin(), labels_.end());
  frame = shell::renderMainMenu(frame, font_, labels, params, shell::fontCells());
}

} // namespace webo::app
