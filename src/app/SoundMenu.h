#pragma once

#include "assets/GameData.h"
#include "assets/Bitmap.h"
#include "app/WindowScale.h"
#include "app/SkillPanel.h"
#include "save/SoundSettings.h"
#include "shell/MenuRender.h"

#include <array>
#include <optional>
#include <vector>

namespace webo::app {

class SoundMenu {
 public:
  static std::expected<SoundMenu, std::string> load(const AssetLibrary& library,
                                                     const StringTable& strings);
  enum Row { Music, Sfx, UnlockAll, WindowSize, KeyboardShortcuts, Accept, RowCount };
  struct Hit {
    int row;
    int direction; // +/-1 for arrows and volume labels, zero for the other actions
  };
  std::optional<Hit> hit(int x, int y, int baseY, save::SoundSettings settings,
                         const WindowScaleMenu& window) const;
  Image render(const Image& backdrop, const Image& font, int baseY, int focus,
               save::SoundSettings settings, const WindowScaleMenu& window) const;
  struct Prepared {
    struct Row {
      shell::PreparedMenuRows label;
      std::vector<PreparedIconRegion> arrows;
      std::optional<std::array<shell::ScreenRect, 16>> meter;
    };
    std::vector<Row> rows;
    void draw(Image&) const;
  };
  Prepared prepare(int viewportWidth, const Image& font, int baseY, int focus,
                   save::SoundSettings settings, const WindowScaleMenu& window) const;
  static Image renderShortcuts(const Image& backdrop, const Image& font);
  static bool shortcutsBackAt(int x, int y);

 private:
  std::array<std::string, RowCount> labels(save::SoundSettings settings,
                                           const WindowScaleMenu& window) const;
  std::array<std::array<std::string, RowCount>, 2> labels_;
  std::array<Image, 2> arrows_;
};

}  // namespace webo::app
