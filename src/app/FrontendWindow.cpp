#include "app/FrontendWindow.h"

#include "assets/Png.h"
#include <cstdio>
#include <memory>
#ifdef __EMSCRIPTEN__
#include "web/Browser.h"
#endif

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "app/SdlWindow.hpp"
#endif

namespace webo::app {

#if WEBOLUTION_HAVE_SDL2
namespace {
struct CursorVisibility {
  // SDL hides the OS cursor for the software sprite. The alternate native backend owns monochrome handles.
  int previous = SDL_ENABLE;
  CursorVisibility() : previous(SDL_ShowCursor(SDL_QUERY)) { SDL_ShowCursor(SDL_DISABLE); }
  ~CursorVisibility() { if (previous >= 0) SDL_ShowCursor(previous); }
};

std::string sdlError(const char* action) {
  return std::string(action) + ": " + SDL_GetError();
}

#ifdef __EMSCRIPTEN__
bool supportsPortrait(shell::Screen screen) {
  switch (screen) {
    case shell::Screen::Menu:
    case shell::Screen::Confirm:
    case shell::Screen::Language:
    case shell::Screen::SoundOptions:
    case shell::Screen::KeyboardShortcuts: return true;
    default: return false;
  }
}

struct RestorePortraitViewport {
  SdlWindow& presentation;
  const LevelLayout& viewport;
  ~RestorePortraitViewport() {
    if (viewport.portrait()) presentation.setWebViewport(webCanvasLayout(false));
  }
};
#endif
}  // namespace

void updateFrontendCursorInput(FrontendSession& session, const SDL_Event& event) {
  if (event.type == SDL_WINDOWEVENT) {
    if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) session.setFocused(false);
    else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) session.setFocused(true);
  } else if (session.focused() && (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)) {
    const bool down = event.type == SDL_MOUSEBUTTONDOWN;
    if (event.button.button == SDL_BUTTON_LEFT) session.setButtons(down, session.rightHeld());
    else if (event.button.button == SDL_BUTTON_RIGHT) session.setButtons(session.leftHeld(), down);
  }
}

void handleSoundOptionsInput(FrontendSession& session, const SDL_Event& event,
                              const SdlWindow& presentation, LevelLayout viewport) {
  session.setWindowScaleMenu(presentation.windowScaleMenu());
  if (event.type == SDL_MOUSEMOTION) {
    session.hoverSound(event.motion.x, event.motion.y, viewport);
  } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
    session.click(event.button.x, event.button.y, viewport);
  } else if (event.type == SDL_KEYDOWN) {
    const auto key = event.key.keysym.sym;
    if (key == SDLK_LEFT || key == SDLK_RIGHT) session.adjustSound(key == SDLK_LEFT ? -1 : 1, viewport);
    else if (!event.key.repeat) {
      if (key == SDLK_UP || key == SDLK_DOWN) session.moveMenuFocus(key == SDLK_UP ? -1 : 1, viewport);
      else if (key == SDLK_RETURN) session.pressReturn(viewport);
      else if (key == SDLK_ESCAPE) session.pressEscape();
    }
  }
}

std::expected<std::optional<std::string>, std::string> runFrontendWindow(
    FrontendSession& session, const FrontendWindowOptions& options,
    FrontendWindowObserver observer, SdlWindow* sharedWindow) {
  if (options.maxFrames < 0) return std::unexpected("maxFrames cannot be negative");
  if (!options.finalFramePath.empty() && options.maxFrames == 0)
    return std::unexpected("frontend screenshot requires a positive frame limit");
  if (session.selectedLevel()) return session.selectedLevel();
  if (session.quitRequested()) return std::optional<std::string>{};

  SdlWindow localWindow(session.windowScale(),
      windowScaleOverride(0, options.maxFrames > 0 || bool(observer)));
  auto& presentation = sharedWindow ? *sharedWindow : localWindow;
  presentation.updateWindowScale(session.windowScale());
  if (auto ready = presentation.ensure(); !ready) return std::unexpected(ready.error());
  auto* renderer = presentation.renderer();
  using Texture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;
  Texture texture(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
                                    kGameWidth, kGameHeight), SDL_DestroyTexture);
  if (!texture) return std::unexpected(sdlError("SDL_CreateTexture"));
  if (SDL_SetTextureScaleMode(texture.get(), SDL_ScaleModeNearest) != 0)
    return std::unexpected(sdlError("SDL_SetTextureScaleMode"));
  Texture portraitTexture(nullptr, SDL_DestroyTexture);
  LevelLayout viewport;

  CursorVisibility cursorVisibility;

  // Each frontend leg starts a new input lifetime on the shared window.
  // Preserve logical XY without reading or moving the desktop pointer.
  session.setFocused(true);
  session.setButtons(false, false);
  bool running = true;
  int frames = 0;
  TouchInput touch;
  std::optional<Image> lastPresented;
#ifdef __EMSCRIPTEN__
  const RestorePortraitViewport restoreViewport{presentation, viewport};
  bool portraitUnavailable = false;
  int portraitTextureHeight = 0;
  const auto updateViewport = [&] {
    auto wanted = webCanvasLayout(supportsPortrait(session.screen()) && !portraitUnavailable);
    if (wanted.portraitHeight == viewport.portraitHeight) return;
    if (wanted.portrait() && portraitTextureHeight != wanted.height()) {
      portraitTexture.reset(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
                                              wanted.width(), wanted.height()));
      if (!portraitTexture || SDL_SetTextureScaleMode(portraitTexture.get(), SDL_ScaleModeNearest) != 0) {
        std::fprintf(stderr, "SDL portrait frontend texture: %s\n", SDL_GetError());
        portraitUnavailable = true;
        wanted = webCanvasLayout(false);
        if (!viewport.portrait()) return;
      } else {
        portraitTextureHeight = wanted.height();
      }
    }
    viewport = wanted;
    presentation.setWebViewport(viewport);
    touch.setViewport(viewport.width(), viewport.height());
    session.setPointer(viewport.width() / 2, viewport.height() / 2, viewport);
  };
  updateViewport();
#endif
  const auto routeEvent = [&](const SDL_Event& event) {
    updateFrontendCursorInput(session, event);
    if (!session.focused()) return;
    if (event.type == SDL_MOUSEMOTION) session.setPointer(event.motion.x, event.motion.y, viewport);
    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
      session.setPointer(event.button.x, event.button.y, viewport);
    if (session.screen() == shell::Screen::SoundOptions ||
        session.screen() == shell::Screen::KeyboardShortcuts) {
      handleSoundOptionsInput(session, event, presentation, viewport);
    } else if (event.type == SDL_MOUSEMOTION) {
      session.hoverMenu(session.pointerX(), session.pointerY(), viewport);
    } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
      session.click(session.pointerX(), session.pointerY(), viewport);
    } else if (event.type == SDL_KEYDOWN && !event.key.repeat) {
      const auto key = event.key.keysym.sym;
      if (key == SDLK_RETURN) session.pressReturn();
      else if (key == SDLK_ESCAPE && session.screen() == shell::Screen::Language)
        session.pressEscape();
      else if (key == SDLK_UP || key == SDLK_DOWN) session.moveMenuFocus(key == SDLK_UP ? -1 : 1);
    }
  };
  while (running && (options.maxFrames == 0 || frames < options.maxFrames)) {
    bool displayChanged = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (isTouchMouseEvent(event)) continue;
      if (event.type == SDL_QUIT) { running = false; break; }
      displayChanged |= event.type == SDL_WINDOWEVENT &&
                        event.window.event == SDL_WINDOWEVENT_DISPLAY_CHANGED;
      const auto screen = session.screen();
      if (isFingerEvent(event.type)) {
        const auto point = presentation.touchCoordinates(event.tfinger);
        if (!point || !session.focused() || session.leftHeld() || session.rightHeld()) continue;
        const auto action = presentation.feedTouch(touch, event.tfinger, *point);
        if (action.kind == TouchActionKind::Tap) {
          routeEvent(touchButton(SDL_MOUSEBUTTONDOWN, action.point, event.tfinger.timestamp));
          routeEvent(touchButton(SDL_MOUSEBUTTONUP, action.point, event.tfinger.timestamp));
        }
      } else {
        if (event.type == SDL_MOUSEBUTTONDOWN) touch.reset();
        presentation.gameCoordinates(event);
        routeEvent(event);
      }
      if (!session.focused() || screen != session.screen()) touch.reset();
      if (session.selectedLevel()) return session.selectedLevel();
      if (session.quitRequested()) return std::optional<std::string>{};
    }
    if (!running) break;

    // All queued mouse events above belong to the old client size.
    presentation.updateWindowScale(session.windowScale(), displayChanged);
#ifdef __EMSCRIPTEN__
    updateViewport();
#endif
    if (session.screen() == shell::Screen::SoundOptions)
      session.setWindowScaleMenu(presentation.windowScaleMenu());
    const Image image = session.composeFrame(viewport);
    if (image.width != viewport.width() || image.height != viewport.height() ||
        image.rgba.size() != image.pixelCount() * 4)
      return std::unexpected("frontend image must be tightly packed RGBA at the viewport size");
    auto* output = viewport.portrait() ? portraitTexture.get() : texture.get();
    if (SDL_UpdateTexture(output, nullptr, image.rgba.data(), image.width * 4) != 0)
      return std::unexpected(sdlError("SDL_UpdateTexture"));
    if (SDL_RenderClear(renderer) != 0 ||
        SDL_RenderCopy(renderer, output, nullptr, nullptr) != 0)
      return std::unexpected(sdlError("SDL_RenderCopy"));
    SDL_RenderPresent(renderer);
    if (!options.finalFramePath.empty()) lastPresented = image;
    const int frame = frames++;
    if (observer) observer(session, frame);
    if (options.maxFrames > 0 && frames >= options.maxFrames) break;
#ifdef __EMSCRIPTEN__
    web::nextFrame();
#else
    SDL_Delay(1);
#endif
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
    FrontendSession&, const FrontendWindowOptions&, FrontendWindowObserver, SdlWindow*) {
  return std::unexpected("frontend window requires SDL2");
}
#endif

}  // namespace webo::app
