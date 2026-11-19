#pragma once

#include "assets/GameData.h"
#include "assets/Bitmap.h"
#include "save/SoundSettings.h"

#include <array>
#include <optional>

namespace webo::app {

class SoundMenu {
 public:
  static std::expected<SoundMenu, std::string> load(const AssetLibrary& library,
                                                     const StringTable& strings);
  enum Row { Music, Sfx, UnlockAll, Accept, RowCount };
  struct Hit {
    int row;
    int direction; // +/-1 for Music/SFX, zero for the other actions
  };
  std::optional<Hit> hit(int x, int y, int baseY, bool unlockAllLevels) const;
  Image render(const Image& backdrop, const Image& font, int baseY, int focus,
               save::SoundSettings settings) const;

 private:
  std::array<std::array<std::string, RowCount>, 2> labels_;
  std::array<Image, 2> arrows_;
};

}  // namespace webo::app
