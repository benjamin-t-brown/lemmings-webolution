#include "app/SoundMenu.h"

#include "app/SkillPanel.h"
#include "shell/HudText.h"
#include "shell/MenuRender.h"

namespace webo::app {
namespace {

struct Row {
  int left, right, top;
  int arrowX(int side) const { return side == 0 ? left - 36 : right + 20; } // 48A99B
};

bool hasMeter(int row) {
  return row == SoundMenu::Music || row == SoundMenu::Sfx;
}

bool hasArrows(int row) {
  return hasMeter(row) || row == SoundMenu::WindowSize;
}

struct ShortcutLine {
  std::string_view keys, action;
  // The legacy test checks these names against the gameplay key handler.
  std::string_view scancodes;
};

constexpr ShortcutLine kShortcuts[]{
    {"1 to 8", "Select a stocked skill",
     "SDL_SCANCODE_1 SDL_SCANCODE_2 SDL_SCANCODE_3 SDL_SCANCODE_4 "
     "SDL_SCANCODE_5 SDL_SCANCODE_6 SDL_SCANCODE_7 SDL_SCANCODE_8"},
    {"P", "Pause / resume", "SDL_SCANCODE_P"},
    {"Enter / Pause/Break", "Examine Level", "SDL_SCANCODE_RETURN SDL_SCANCODE_PAUSE"},
    {"Keypad Enter", "Examine Level", "SDL_SCANCODE_KP_ENTER"},
    {"Escape", "Open level menu", "SDL_SCANCODE_ESCAPE"},
    {"Left / Right", "Rotate camera (Shift: faster)", "SDL_SCANCODE_LEFT SDL_SCANCODE_RIGHT"},
};

constexpr std::string_view kShortcutsLabel = "Keyboard Shortcuts", kBackLabel = "Back";
constexpr int kBackY = 447;

// The accept/exit row keeps a warmer idle plate and turns red on hover/focus.
constexpr shell::Rgb kAcceptOrange{0xE0, 0x80, 0};
constexpr shell::Rgb kAcceptHoverRed{255, 0, 0};

std::array<Row, SoundMenu::RowCount> layout(
    const std::array<std::string, SoundMenu::RowCount>& labels, int baseY) {
  const auto& cells = shell::fontCells();
  std::array<Row, SoundMenu::RowCount> rows{};
  int top = baseY;
  for (int i = 0; i < SoundMenu::RowCount; ++i) {
    const int left = shell::labelLeft(labels[i], cells, 640);
    rows[i] = {left, left + shell::textWidth(labels[i], cells), top};
    // 48E5B4 reserves a line under each volume label for its meter.
    top += (hasMeter(i) ? 2 : 1) * shell::kLineHeight;
  }
  return rows;
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
  menu.labels_ = {std::string(strings[11]), std::string(strings[12]), {}, {}, {},
                  std::string(kShortcutsLabel), std::string(strings[24])};
  menu.arrows_ = {std::move(*left), std::move(*right)};
  return menu;
}

std::array<std::string, SoundMenu::RowCount> SoundMenu::labels(
    save::SoundSettings settings, const WindowScaleMenu& window) const {
  auto rows = labels_;
  rows[UnlockAll] = settings.unlockAllLevels ? "Unlock all levels (On)" : "Unlock all levels (Off)";
  rows[ReverseDrag] = settings.reverseDrag ? "Reverse camera drag (On)" : "Reverse camera drag (Off)";
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
  const auto rows = layout(labels(settings, window), baseY);
  for (int i = 0; i < RowCount; ++i) {
    const auto& row = rows[i];
    if (hasArrows(i)) {
      for (int side = 0; side < 2; ++side) {
        const int start = row.arrowX(side);
        if (start <= x && x < start + arrows_[side].width &&
            row.top <= y && y < row.top + arrows_[side].height)
          return Hit{i, side == 0 ? -1 : 1};
      }
    }
    if (i == KeyboardShortcuts) {
      const auto box = shell::menuPlateBounds(row.left, row.top, row.right - row.left, {});
      if (shell::menuPanelAt(std::span(&box, 1), x, y) == 0) return Hit{i, 0};
    } else if (row.left <= x && x < row.right && row.top <= y && y < row.top + shell::kLineHeight) {
      return Hit{i, i < UnlockAll ? -1 : 0}; // volume label activation decreases a bus
    }
  }
  return std::nullopt;
}

Image SoundMenu::render(const Image& backdrop, const Image& font, int baseY, int focus,
                        save::SoundSettings settings, const WindowScaleMenu& window) const {
  shell::MenuParams params;
  const auto text = labels(settings, window);
  const auto rows = layout(text, baseY);
  Image frame = backdrop;
  for (int i = 0; i < RowCount; ++i) {
    const auto& row = rows[i];
    shell::MenuParams rowParams = params;
    rowParams.baseY = row.top;
    rowParams.highlight = i == focus ? 0 : -1;
    if (i == Accept) {
      rowParams.panelNormal = kAcceptOrange;
      rowParams.panelHighlight = kAcceptHoverRed;
    }
    shell::drawMenuRows(frame, font, std::span(&text[i], 1), rowParams, shell::fontCells());
    if (hasArrows(i)) {
      for (int side = 0; side < 2; ++side) {
        const auto& art = arrows_[side];
        compositeIconRegion(frame, art, {row.arrowX(side), row.top, 0, 0, art.width, art.height});
      }
    }
    if (hasMeter(i))
      shell::drawVolumeMeter(frame, row.top + shell::kLineHeight + 8,
                            i == Music ? settings.music : settings.sfx);
  }
  return frame;
}

Image SoundMenu::renderShortcuts(const Image& backdrop, const Image& font) {
  Image frame = backdrop;
  for (std::size_t i = 0; i < frame.rgba.size(); i += 4)
    for (int channel = 0; channel < 3; ++channel)
      frame.rgba[i + channel] /= 3;

  const auto& cells = shell::fontCells();
  constexpr shell::Rgb white{255, 255, 255}, gold{255, 216, 120};
  std::vector<shell::HudLine> lines{
      {std::string(kShortcutsLabel), shell::labelLeft(kShortcutsLabel, cells, 640), 5, gold}};
  constexpr int x = 16;
  int y = 40;
  for (const auto& line : kShortcuts) {
    lines.push_back({std::string(line.keys), x, y, gold});
    lines.push_back({std::string(line.action), x + shell::textWidth(line.keys, cells) + 12, y, white});
    y += 21;
  }
  shell::drawHud(frame, font, lines, cells);

  shell::MenuParams params;
  params.baseY = kBackY;
  const std::array labels{std::string(kBackLabel)};
  shell::drawMenuRows(frame, font, labels, params, cells);
  return frame;
}

bool SoundMenu::shortcutsBackAt(int x, int y) {
  const auto& cells = shell::fontCells();
  const auto box = shell::menuPlateBounds(shell::labelLeft(kBackLabel, cells, 640), kBackY,
                                          shell::textWidth(kBackLabel, cells), {});
  return shell::menuPanelAt(std::span(&box, 1), x, y) == 0;
}

}  // namespace webo::app
