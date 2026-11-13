// One presentation lifetime for connected play; standalone adapters own a local
// instance. Textures remain local to each leg (RGBA frontend, RGB level).
// PROVISIONAL clone ownership policy: native HWND/device continuity across
// menu/level/results has not been observed. A native handle trace would settle
// that comparison; a stable player window is the playable-v1 requirement here.
#pragma once

#if defined(WEBOLUTION_HAVE_SDL2) && WEBOLUTION_HAVE_SDL2
#include <SDL.h>
#include "app/SdlShutdown.hpp"
#include <expected>
#include <string>

namespace webo::app {
class SdlWindow {
 public:
  SdlWindow() = default;
  SdlWindow(const SdlWindow&) = delete;
  SdlWindow& operator=(const SdlWindow&) = delete;
  ~SdlWindow() {
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    if (initialized_) quitWindowSubsystems();
  }
  std::expected<void, std::string> ensure(const char* title = "Lemmings Webolution") {
    if (renderer_) return {};
    SDL_SetMainReady();
    if (!initialized_) {
      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) return error("SDL_Init");
      initialized_ = true;
    }
    if (!window_) window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
    if (!window_) return error("SDL_CreateWindow");
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_) return error("SDL_CreateRenderer");
    return {};
  }
  SDL_Renderer* renderer() const { return renderer_; }

 private:
  static std::unexpected<std::string> error(const char* action) {
    return std::unexpected(std::string(action) + ": " + SDL_GetError());
  }
  bool initialized_ = false;
  SDL_Window* window_ = nullptr;
  SDL_Renderer* renderer_ = nullptr;
};
}  // namespace webo::app
#endif
