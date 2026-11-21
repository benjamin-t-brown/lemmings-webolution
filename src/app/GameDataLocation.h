#pragma once

#include <filesystem>
#include <string>

namespace webo::app {

struct GameDataLocation {
  std::filesystem::path directory;
  bool fromDiscCache = false;
  std::string discError;
  std::string executableSearch;
};

GameDataLocation locateGameData(const std::filesystem::path& requested);
void reportGameDataError(std::string error, bool showDialog);

}  // namespace webo::app
