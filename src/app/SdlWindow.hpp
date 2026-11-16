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
  explicit SdlWindow(int scale = 1) : scale_(scale) {}
  SdlWindow(const SdlWindow&) = delete;
  SdlWindow& operator=(const SdlWindow&) = delete;
  ~SdlWindow() {
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    if (initialized_) quitWindowSubsystems();
  }
  std::expected<void, std::string> ensure(const char* title = "Lemmings Webolution") {
    if (renderer_) return {};
    if (scale_ < 1 || scale_ > 3) return std::unexpected("window scale must be 1, 2 or 3");
    SDL_SetMainReady();
    if (!initialized_) {
      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) return error("SDL_Init");
      initialized_ = true;
    }
    if (!window_) window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 640 * scale_, 480 * scale_, SDL_WINDOW_SHOWN);
    if (!window_) return error("SDL_CreateWindow");
    // Upscale only the final 640x480 texture on the GPU when available. Keeping
    // the enlarged blit off the CPU avoids a scale-dependent frame-time cost;
    // dummy/headless drivers retain the existing software fallback. Use the
    // same backend at every scale; no vsync request or simulation/raster changes.
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_) return error("SDL_CreateRenderer");
    return {};
  }
  SDL_Renderer* renderer() const { return renderer_; }
  int scale() const { return scale_; }
  // PROVISIONAL clone presentation policy (playtest V9), not recovered native
  // window sizing. No logical renderer size/event filter: translate exactly
  // once here, including public SDL_PushEvent automation. A native window/input
  // observation would settle original scaling behavior. Relative camera motion
  // is divided in floating point by the caller so subpixel deltas survive.
  void gameCoordinates(SDL_Event& event) const {
    const auto pixel = [this](int value) {
      const int quotient = value / scale_;
      return quotient - (value < 0 && value % scale_ != 0 ? 1 : 0);
    };
    if (event.type == SDL_MOUSEMOTION) {
      event.motion.x = pixel(event.motion.x); event.motion.y = pixel(event.motion.y);
    } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
      event.button.x = pixel(event.button.x); event.button.y = pixel(event.button.y);
    }
  }

 private:
  static std::unexpected<std::string> error(const char* action) {
    return std::unexpected(std::string(action) + ": " + SDL_GetError());
  }
  bool initialized_ = false;
  int scale_ = 1;
  SDL_Window* window_ = nullptr;
  SDL_Renderer* renderer_ = nullptr;
};
}  // namespace webo::app
#endif
