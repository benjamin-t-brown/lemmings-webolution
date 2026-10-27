#pragma once
#include "assets/Box.h"
#include "assets/Bitmap.h"
#include <array>
#include <expected>
#include <filesystem>
#include <string>
namespace webo::app {
std::expected<std::array<std::int32_t,102>,std::string> readMapFlags(const std::filesystem::path& path);
class MapScene {
public:
 static std::expected<MapScene,std::string> build(const AssetLibrary& lib,const std::array<std::int32_t,102>& flags);
 const Image& image() const { return image_; }
 const std::array<std::int32_t,102>& states() const { return states_; }
private:
 Image image_;
 std::array<std::int32_t,102> states_{};
};
}
