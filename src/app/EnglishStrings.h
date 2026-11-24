#pragma once

#include "assets/GameData.h"

namespace webo::app {

inline constexpr std::string_view kEnglishLanguageCode = "UK";

inline std::expected<StringTable, std::string> loadEnglishStrings(const AssetLibrary& library) {
  return loadStrings(library, kEnglishLanguageCode);
}

}  // namespace webo::app
