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

bool hasArrows(int row) {
  return row == SoundMenu::Music || row == SoundMenu::Sfx || row == SoundMenu::WindowSize;
}

std::string dimensions(int width, int height) {
  return std::to_string(width) + "x" + std::to_string(height);
}

std::string fixedSize(int scale) {
  return std::to_string(scale) + "x " + dimensions(kGameWidth * scale, kGameHeight * scale);
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
  // Clone additions in the existing Options layout.
  menu.labels_[false] = {std::string(strings[11]), std::string(strings[12]),
                         "Unlock all levels (Off)", {}, std::string(strings[24])};
  menu.labels_[true] = menu.labels_[false];
  menu.labels_[true][UnlockAll] = "Unlock all levels (On)";
  menu.arrows_ = {std::move(*left), std::move(*right)};
  return menu;
}

std::array<std::string, SoundMenu::RowCount> SoundMenu::labels(
    save::SoundSettings settings, const WindowScaleMenu& window) const {
  auto rows = labels_[settings.unlockAllLevels];
  auto& label = rows[WindowSize];
  label = "Window size - ";
  const int choice = settings.windowScale;
#ifdef __EMSCRIPTEN__
  label += choice ? fixedSize(choice) : "Fill";
  if (!choice && window.fillWidth > 0 && window.fillHeight > 0)
    label += " " + dimensions(window.fillWidth, window.fillHeight);
  const int applied = window.runOverride;
#else
  const int fit = desktopWindowScale(choice, window.largestDesktopFit);
  label += choice ? fixedSize(choice) : "Auto " + fixedSize(fit);
  if (choice && fit < choice)
    label += " (Fits " + std::to_string(fit) + "x)";
  const int applied = desktopWindowScale(window.runOverride, window.largestDesktopFit);
#endif
  if (window.runOverride)
    label += " (Run " + std::to_string(applied) + "x)";
  return rows;
}

std::optional<SoundMenu::Hit> SoundMenu::hit(int x, int y, int baseY,
                                          save::SoundSettings settings,
                                          const WindowScaleMenu& window) const {
  const auto rows = labels(settings, window);
  for (int i = 0; i < RowCount; ++i) {
    const auto row = layout(rows[i], i, baseY);
    if (hasArrows(i)) {
      for (int side = 0; side < 2; ++side) {
        const int start = row.arrowX(side);
        if (start <= x && x < start + arrows_[side].width &&
            row.top <= y && y < row.top + arrows_[side].height)
          return Hit{i, side == 0 ? -1 : 1};
      }
    }
    if (row.left <= x && x < row.right && row.top <= y && y < row.top + shell::kLineHeight)
      return Hit{i, i < UnlockAll ? -1 : 0}; // volume label activation decreases a bus
  }
  return std::nullopt;
}

Image SoundMenu::render(const Image& backdrop, const Image& font, int baseY, int focus,
                        save::SoundSettings settings, const WindowScaleMenu& window) const {
  shell::MenuParams params;
  params.baseY = baseY;
  params.rowStep = rowSpacing;
  params.highlight = focus;
  const auto rows = labels(settings, window);
  Image frame = shell::renderMainMenu(backdrop, font, rows, params, shell::fontCells());
  for (int i : {Music, Sfx, WindowSize}) {
    const auto row = layout(rows[i], i, baseY);
    for (int side = 0; side < 2; ++side) {
      const auto& art = arrows_[side];
      compositeIconRegion(frame, art, {row.arrowX(side), row.top, 0, 0, art.width, art.height});
    }
    if (i != WindowSize)
      shell::drawVolumeMeter(frame, row.top + shell::kLineHeight + 8,
                            i == Music ? settings.music : settings.sfx);
  }
  return frame;
}

}  // namespace webo::app
