#include "app/FrontendWindow.h"

#include "assets/Png.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

namespace webo::app {

#if WEBOLUTION_HAVE_SDL2
namespace {
struct SdlResources {
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_Texture* texture = nullptr;
  ~SdlResources() {
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
  }
};

struct CursorVisibility {
  // This frontend path uses the software sprite. Native's alternate backend
  // owns separate monochrome OS handles; SDL hiding here is not that backend.
  int previous = SDL_ENABLE;
  CursorVisibility() : previous(SDL_ShowCursor(SDL_QUERY)) { SDL_ShowCursor(SDL_DISABLE); }
  ~CursorVisibility() { if (previous >= 0) SDL_ShowCursor(previous); }
};

std::string sdlError(const char* action) {
  return std::string(action) + ": " + SDL_GetError();
}
}  // namespace

std::expected<std::optional<std::string>, std::string> runFrontendWindow(
    FrontendSession& session, const FrontendWindowOptions& options,
    FrontendWindowObserver observer) {
  if (options.maxFrames < 0) return std::unexpected("maxFrames cannot be negative");
  if (!options.finalFramePath.empty() && options.maxFrames == 0)
    return std::unexpected("frontend screenshot requires a positive frame limit");
  if (session.selectedLevel()) return session.selectedLevel();

  SDL_SetMainReady();
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
    return std::unexpected(sdlError("SDL_Init"));
  SdlResources sdl;
  sdl.window = SDL_CreateWindow("Lemmings Webolution", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
  if (!sdl.window) return std::unexpected(sdlError("SDL_CreateWindow"));
  sdl.renderer = SDL_CreateRenderer(sdl.window, -1, SDL_RENDERER_SOFTWARE);
  if (!sdl.renderer) return std::unexpected(sdlError("SDL_CreateRenderer"));
  sdl.texture = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_RGBA32,
                                  SDL_TEXTUREACCESS_STREAMING, 640, 480);
  if (!sdl.texture) return std::unexpected(sdlError("SDL_CreateTexture"));

  CursorVisibility cursorVisibility;

  // A fresh SDL window cannot own holds from a previous window/level. Retain
  // logical XY, but start a new input lifetime without querying/warping desktop
  // input. This is the SOURCE adapter boundary, not native focus sampling.
  session.setFocused(true);
  session.setButtons(false, false);
  bool running = true;
  int frames = 0;
  std::optional<Image> lastPresented;
  while (running && (options.maxFrames == 0 || frames < options.maxFrames)) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) { running = false; break; }
      if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
          session.setFocused(false);
        } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
          session.setFocused(true);
        }
      } else if (session.focused() && event.type == SDL_MOUSEMOTION) {
        session.setPointer(event.motion.x, event.motion.y);
      } else if (session.focused() && event.type == SDL_KEYDOWN && !event.key.repeat &&
                 event.key.keysym.sym == SDLK_RETURN) {
        session.pressReturn();
      } else if (session.focused() && event.type == SDL_KEYDOWN && !event.key.repeat &&
                 (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN)) {
        session.moveMenuFocus(event.key.keysym.sym == SDLK_UP ? -1 : 1);
      } else if (session.focused() && (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)) {
        session.setPointer(event.button.x, event.button.y);
        const bool down = event.type == SDL_MOUSEBUTTONDOWN;
        if (event.button.button == SDL_BUTTON_LEFT) session.setButtons(down, session.rightHeld());
        else if (event.button.button == SDL_BUTTON_RIGHT) session.setButtons(session.leftHeld(), down);
        if (down && event.button.button == SDL_BUTTON_LEFT)
          session.click(session.pointerX(), session.pointerY());
      }
      if (session.selectedLevel()) return session.selectedLevel();
    }
    if (!running) break;

    const Image image = session.composeFrame();
    if (image.width != 640 || image.height != 480 || image.rgba.size() != 640u * 480u * 4u)
      return std::unexpected("frontend image must be tightly packed 640x480 RGBA");
    if (SDL_UpdateTexture(sdl.texture, nullptr, image.rgba.data(), 640 * 4) != 0)
      return std::unexpected(sdlError("SDL_UpdateTexture"));
    if (SDL_RenderClear(sdl.renderer) != 0 ||
        SDL_RenderCopy(sdl.renderer, sdl.texture, nullptr, nullptr) != 0)
      return std::unexpected(sdlError("SDL_RenderCopy"));
    SDL_RenderPresent(sdl.renderer);
    if (!options.finalFramePath.empty()) lastPresented = image;
    const int frame = frames++;
    if (observer) observer(session, frame);
    if (options.maxFrames > 0 && frames >= options.maxFrames) break;
    SDL_Delay(1);
  }

  if (!running && !options.finalFramePath.empty())
    return std::unexpected("window closed before requested final frame was captured");
  if (options.maxFrames > 0 && frames >= options.maxFrames && !options.finalFramePath.empty()) {
    if (!lastPresented) return std::unexpected("no frontend frame was presented");
    if (auto written = writePng(options.finalFramePath, *lastPresented); !written)
      return std::unexpected(written.error());
  }
  return std::optional<std::string>{};
}
#else
std::expected<std::optional<std::string>, std::string> runFrontendWindow(
    FrontendSession&, const FrontendWindowOptions&, FrontendWindowObserver) {
  return std::unexpected("frontend window requires SDL2");
}
#endif

}  // namespace webo::app
