#pragma once

#include "assets/GameData.h"
#include "assets/Bitmap.h"
#include "app/WindowScale.h"
#include "save/SoundSettings.h"

#include <array>
#include <optional>

namespace webo::app {

class SoundMenu {
 public:
  static std::expected<SoundMenu, std::string> load(const AssetLibrary& library,
                                                     const StringTable& strings);
  enum Row { Music, Sfx, UnlockAll, ReverseDrag, WindowSize, KeyboardShortcuts, Accept, RowCount };
  struct Hit {
    int row;
    int direction; // +/-1 for arrows and volume labels, zero for the other actions
  };
  std::optional<Hit> hit(int x, int y, int baseY, save::SoundSettings settings,
                         const WindowScaleMenu& window) const;
  Image render(const Image& backdrop, const Image& font, int baseY, int focus,
               save::SoundSettings settings, const WindowScaleMenu& window) const;
  static Image renderShortcuts(const Image& backdrop, const Image& font);
  static bool shortcutsBackAt(int x, int y);

 private:
  std::array<std::string, RowCount> labels(save::SoundSettings settings,
                                           const WindowScaleMenu& window) const;
  std::array<std::string, RowCount> labels_;
  std::array<Image, 2> arrows_;
};

}  // namespace webo::app
