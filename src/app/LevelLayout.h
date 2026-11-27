#pragma once

#include "app/WindowScale.h"
#include "shell/MenuRender.h"

namespace webo::app {

inline constexpr int kPortraitWidth = 390;
inline constexpr int kMinPortraitHeight = 320; // Keep the zoom plate below the top controls.
inline constexpr int kPortraitHorizonOffset = -38;

struct LevelLayout {
  int portraitHeight = 0;

  constexpr bool portrait() const { return portraitHeight != 0; }
  constexpr int width() const { return portrait() ? kPortraitWidth : kGameWidth; }
  constexpr int height() const { return portrait() ? portraitHeight : kGameHeight; }
};

struct LevelButton {
  int x, y, width, height;

  constexpr bool contains(int px, int py) const {
    return px >= x && px < x + width && py >= y && py < y + height;
  }
};

inline constexpr LevelButton kPortraitFastForward{14, 68, 152, 50};
inline constexpr LevelButton kPortraitNuke{176, 68, 112, 50};
inline constexpr LevelButton kPortraitRotate{298, 68, 78, 50};

constexpr LevelButton skillPlate(int index, LevelLayout layout = {}) {
  if (!layout.portrait()) return {18, 12 + 40 * index, 72, 32};
  if (index < 8) return {14 + 92 * (index % 4), layout.height() - 135 + 58 * (index / 4), 86, 47};
  return index == 8 ? kPortraitFastForward : kPortraitNuke;
}

inline constexpr LevelButton kPortraitMenu{14, 14, 80, 42};
constexpr LevelButton portraitZoomButton(LevelLayout layout) {
  return {330, layout.height() - 198, 46, 46};
}

inline LevelButton portraitTextButton(std::string_view label, int top, int height) {
  constexpr int padding = shell::MenuParams{}.panelPadX;
  const auto& cells = shell::fontCells();
  const int left = shell::labelLeft(label, cells, kPortraitWidth);
  return {left - padding, top, shell::textWidth(label, cells) + 2 * padding, height};
}

// Fit the original red menu plate around a phone-sized touch target.
inline shell::MenuParams portraitButtonParams(LevelButton button, std::string_view label) {
  const int width = shell::textWidth(label, shell::fontCells());
  return {.xWord = 2 * button.x + button.width, .baseY = button.y + (button.height - shell::kLineHeight) / 2,
          .panelPadX = (button.width - width) / 2, .panelTop = -(button.height - shell::kLineHeight) / 2,
          .panelHeight = button.height, .highlight = -1};
}

}  // namespace webo::app
