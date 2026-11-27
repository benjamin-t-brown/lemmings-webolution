#pragma once

#include "shell/MenuRender.h"

namespace webo::app {

inline constexpr int kPortraitWidth = 390;
inline constexpr int kPortraitHeight = 844;
inline constexpr int kPortraitHorizonOffset = -38;

struct LevelButton {
  int x, y, width, height;

  constexpr bool contains(int px, int py) const {
    return px >= x && px < x + width && py >= y && py < y + height;
  }
};

constexpr LevelButton skillPlate(int index, bool portrait = false) {
  if (!portrait) return {18, 12 + 40 * index, 72, 32};
  if (index < 8) return {14 + 92 * (index % 4), 709 + 58 * (index / 4), 86, 47};
  return {14 + 186 * (index - 8), 68, 176, 50};
}

inline constexpr LevelButton kPortraitMenu{14, 14, 80, 42};
inline constexpr LevelButton kPortraitZoom{330, 646, 46, 46};

// Fit the original red menu plate around a phone-sized touch target.
inline shell::MenuParams portraitButtonParams(LevelButton button, std::string_view label) {
  const int width = shell::textWidth(label, shell::fontCells());
  return {.xWord = 2 * button.x + button.width, .baseY = button.y + (button.height - shell::kLineHeight) / 2,
          .panelPadX = (button.width - width) / 2, .panelTop = -(button.height - shell::kLineHeight) / 2,
          .panelHeight = button.height, .highlight = -1};
}

}  // namespace webo::app
