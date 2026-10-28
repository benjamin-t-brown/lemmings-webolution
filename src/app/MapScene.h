#pragma once
#include "assets/Box.h"
#include "assets/Bitmap.h"
#include <array>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace webo::app {
std::expected<std::array<std::int32_t,102>,std::string> readMapFlags(const std::filesystem::path& path);

class MapScene {
public:
  static std::expected<MapScene,std::string> build(
      const AssetLibrary& lib, const std::array<std::int32_t,102>& flags);
  const Image& image() const { return image_; }
  const std::array<std::int32_t,102>& states() const { return states_; }

  // SOURCE mask semantics read at 0x0049ABC8; state eligibility at 0x0049B26A.
  std::optional<std::size_t> pickNode(int x, int y) const;
  // The mask index maps to a group/slot tuple at 0x0049AB95.
  std::optional<std::string> levelForNode(std::size_t node) const;

 private:
  Image image_;
  std::array<std::int32_t,102> states_{};
  std::vector<std::uint8_t> mask_;
  std::uint8_t maskBackground_ = 0;
  std::array<std::string,102> levelIds_{};
};
}
