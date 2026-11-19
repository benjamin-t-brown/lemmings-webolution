#include "app/SoundMenu.h"

#include "app/SkillPanel.h"
#include "shell/MenuRender.h"

namespace webo::app {
namespace {

constexpr int rowSpacing = 2 * shell::kLineHeight; // 48E5B4 uses rows 1, 3, 5
struct Row {
  int left, right, top;
  int arrowX(int side) const { return side == 0 ? left - 36 : right + 20; } // 48A99B
};

Row layout(const std::string& label, int index, int baseY) {
  const auto& cells = shell::fontCells();
  const int left = shell::labelLeft(label, cells, 640);
  return {left, left + shell::textWidth(label, cells), baseY + index * rowSpacing};
}

}  // namespace

std::expected<SoundMenu, std::string> SoundMenu::load(const AssetLibrary& library,
                                                       const StringTable& strings) {
  if (strings.size() <= 24) return std::unexpected("sound settings require UK strings through 24");
  auto left = loadTexture(library, "TEXTURES\\_BTN_LEFT.BMP");
  auto right = loadTexture(library, "TEXTURES\\_BTN_RIGHT.BMP");
  if (!left) return std::unexpected(left.error());
  if (!right) return std::unexpected(right.error());
  SoundMenu menu;
  // Clone-only row in the original sound page layout.
  menu.labels_[false] = {std::string(strings[11]), std::string(strings[12]),
                         "Unlock all levels (Off)", std::string(strings[24])};
  menu.labels_[true] = menu.labels_[false];
  menu.labels_[true][UnlockAll] = "Unlock all levels (On)";
  menu.arrows_ = {std::move(*left), std::move(*right)};
  return menu;
}

std::optional<SoundMenu::Hit> SoundMenu::hit(int x, int y, int baseY, bool unlockAllLevels) const {
  const auto& labels = labels_[unlockAllLevels];
  for (int i = 0; i < RowCount; ++i) {
    const auto row = layout(labels[i], i, baseY);
    if (i < UnlockAll) {
      for (int side = 0; side < 2; ++side) {
        const int start = row.arrowX(side);
        if (start <= x && x < start + arrows_[side].width &&
            row.top <= y && y < row.top + arrows_[side].height)
          return Hit{i, side == 0 ? -1 : 1};
      }
    }
    if (row.left <= x && x < row.right && row.top <= y && y < row.top + shell::kLineHeight)
      return Hit{i, i < UnlockAll ? -1 : 0}; // native volume label activation decreases a bus
  }
  return std::nullopt;
}

Image SoundMenu::render(const Image& backdrop, const Image& font, int baseY, int focus,
                        save::SoundSettings settings) const {
  shell::MenuParams params;
  params.baseY = baseY;
  params.rowStep = rowSpacing;
  params.highlight = focus;
  const auto& labels = labels_[settings.unlockAllLevels];
  Image frame = shell::renderMainMenu(backdrop, font, labels, params, shell::fontCells());
  for (int i = 0; i < UnlockAll; ++i) {
    const auto row = layout(labels[i], i, baseY);
    for (int side = 0; side < 2; ++side) {
      const auto& art = arrows_[side];
      compositeIconRegion(frame, art, {row.arrowX(side), row.top, 0, 0, art.width, art.height});
    }
    shell::drawVolumeMeter(frame, row.top + shell::kLineHeight + 8,
                          i == Music ? settings.music : settings.sfx);
  }
  return frame;
}

}  // namespace webo::app
