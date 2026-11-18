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
  struct Hit {
    int row;
    int direction; // +/-1 for Music/SFX, zero for Accept
  };
  std::optional<Hit> hit(int x, int y, int baseY) const;
  Image render(const Image& backdrop, const Image& font, int baseY, int focus,
               save::SoundSettings levels) const;

 private:
  std::array<std::string, 3> labels_;
  std::array<Image, 2> arrows_;
};

}  // namespace webo::app
