#include "app/GameDataLocation.h"

#include "assets/Box.h"
#include "assets/DiscGameData.h"
#include "save/SaveStore.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string_view>
#include <vector>

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif
#if defined(_WIN32) && !WEBOLUTION_HAVE_SDL2
#include <windows.h>
#elif defined(__APPLE__) && !WEBOLUTION_HAVE_SDL2
#include <mach-o/dyld.h>
#endif

namespace webo::app {
namespace {
namespace fs = std::filesystem;

#ifndef __EMSCRIPTEN__
fs::path executableDirectory() {
#if WEBOLUTION_HAVE_SDL2
  const std::unique_ptr<char, decltype(&SDL_free)> base(SDL_GetBasePath(), SDL_free);
  if (!base) return {};
  const std::string_view utf8(base.get());
  return fs::path(std::u8string(utf8.begin(), utf8.end()));
#elif defined(_WIN32)
  std::wstring path(32768, L'\0');
  const auto length = GetModuleFileNameW(nullptr, path.data(), path.size());
  if (!length || length >= path.size()) return {};
  path.resize(length);
  return fs::path(path).parent_path();
#elif defined(__linux__)
  std::error_code error;
  return fs::read_symlink("/proc/self/exe", error).parent_path();
#elif defined(__APPLE__)
  std::string path(4096, '\0');
  std::uint32_t length = path.size();
  if (_NSGetExecutablePath(path.data(), &length) != 0) {
    path.resize(length);
    if (_NSGetExecutablePath(path.data(), &length) != 0) return {};
  }
  path.resize(std::char_traits<char>::length(path.c_str()));
  return fs::path(path).parent_path();
#else
  return {};
#endif
}

bool hasBox(const fs::path& path) {
  std::error_code error;
  return fs::is_regular_file(resolveGameDataFile(path), error);
}

std::vector<fs::path> cdImages(const fs::path& directory) {
  std::vector<fs::path> images;
  if (directory.empty()) return images;
  std::error_code error;
  for (fs::directory_iterator it(directory, error), end; !error && it != end; it.increment(error)) {
    auto extension = it->path().extension().u8string();
    for (auto& c : extension)
      if (c >= u8'A' && c <= u8'Z') c += u8'a' - u8'A';
    std::error_code fileError;
    if ((extension == u8".iso" || extension == u8".bin") && it->is_regular_file(fileError))
      images.push_back(it->path());
  }
  std::ranges::sort(images);
  return images;
}

std::string pathText(const fs::path& path) {
  const auto utf8 = path.u8string();
  return std::string(utf8.begin(), utf8.end());
}
#endif
}  // namespace

GameDataLocation locateGameData(const fs::path& requested) {
  if (!requested.empty()) return {requested, false, {}, {}};
  if (const char* env = std::getenv("WEBOLUTION_GAME_DIR"); env && *env)
    return {fs::path(env), false, {}, {}};

  GameDataLocation fallback{"Lemmings Revolution", false, {}, {}};
#ifndef __EMSCRIPTEN__
  // Clone distribution policy for a desktop package.
  const auto exeDir = executableDirectory();
  if (!exeDir.empty() && hasBox(exeDir / "lemmings.box")) return {exeDir, false, {}, {}};
  if (!exeDir.empty()) fallback.executableSearch = pathText(exeDir);

  auto local = save::localAppDataDirectory();
  const auto cache = local ? save::cloneSavePathUnder(*local).parent_path() / "GameData" : fs::path{};
  if (local && hasBox(cache / "lemmings.box") && hasBox(cache / "patch.box"))
    return {cache, true, {}, {}};

  for (const auto& image : cdImages(exeDir)) {
    const auto shown = pathText(image);
    if (!local) {
      fallback.discError = "Could not prepare game data from " + shown + ". " + local.error();
      break;
    }
    if (save::pathIsInside(cache, fallback.directory) || save::pathIsInside(cache, exeDir)) {
      fallback.discError = "The per-user game data folder must be outside the game installation.";
      break;
    }
    std::fprintf(stderr, "Preparing game data from %s\n", shown.c_str());
    std::fflush(stderr);
    const auto imported = installDiscGameData(image, cache);
    if (imported) return {cache, true, {}, {}};
    if (!fallback.discError.empty()) fallback.discError += '\n';
    fallback.discError += shown + ": " + imported.error();
  }
#endif
  return fallback;
}

void reportGameDataError(std::string error, [[maybe_unused]] bool showDialog) {
#ifndef __EMSCRIPTEN__
  error += "\n\nCopy lemmings.box and patch.box from your Lemmings Revolution installation "
           "folder or the CD root (LEMMINGS.BOX and PATCH.BOX) next to the Webolution executable.\n"
           "You can also place a Lemmings Revolution CD image (.iso or raw .bin) next to "
           "the executable, or use --game-dir PATH to point to a folder containing both boxes.\n"
           "Browser version: https://benjamin-t-brown.github.io/lemmings-webolution/";
#endif
  std::fprintf(stderr, "%s\n", error.c_str());
#if WEBOLUTION_HAVE_SDL2 && !defined(__EMSCRIPTEN__)
  const char* driver = SDL_GetCurrentVideoDriver();
  if (!driver) driver = SDL_GetHint(SDL_HINT_VIDEODRIVER);
  if (showDialog && (!driver || std::string_view(driver) != "dummy"))
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Lemmings Webolution game data",
                             error.c_str(), nullptr);
#endif
}

}  // namespace webo::app
