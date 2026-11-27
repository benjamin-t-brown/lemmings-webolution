// Provisional clone ownership shares one window across connected play.
// Standalone callers own their window. Textures are local to each leg.
// Native HWND and device continuity across legs are unresolved.
#pragma once

#if defined(WEBOLUTION_HAVE_SDL2) && WEBOLUTION_HAVE_SDL2
#include <SDL.h>
#include "app/SdlShutdown.hpp"
#include "app/LevelLayout.h"
#include "app/TouchInput.h"
#include "app/WindowScale.h"
#ifdef __EMSCRIPTEN__
#include "web/Browser.h"
#endif
#include <cmath>
#include <expected>
#include <optional>
#include <string>
#include <string_view>

namespace webo::app {

inline bool isFingerEvent(Uint32 type) {
  return type == SDL_FINGERDOWN || type == SDL_FINGERMOTION || type == SDL_FINGERUP;
}

inline bool isTouchMouseEvent(const SDL_Event& event) {
  switch (event.type) {
    case SDL_MOUSEMOTION: return event.motion.which == SDL_TOUCH_MOUSEID;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: return event.button.which == SDL_TOUCH_MOUSEID;
    case SDL_MOUSEWHEEL: return event.wheel.which == SDL_TOUCH_MOUSEID;
    default: return false;
  }
}

inline int touchPixel(double position) { return static_cast<int>(std::floor(position)); }

#ifdef __EMSCRIPTEN__
inline LevelLayout webCanvasLayout(bool allowPortrait) {
  const int height = web::layoutPortraitCanvas(allowPortrait, kPortraitWidth);
  return {height ? std::max(kMinPortraitHeight, height) : 0};
}
#endif

// A recognized tap enters the left-click handlers directly in presentation pixels.
inline SDL_Event touchButton(Uint32 type, TouchPoint point, Uint32 timestamp) {
  SDL_Event event{};
  event.type = type;
  event.button.timestamp = timestamp;
  event.button.button = SDL_BUTTON_LEFT;
  event.button.x = touchPixel(point.x);
  event.button.y = touchPixel(point.y);
  return event;
}

class SdlWindow {
 public:
  explicit SdlWindow(int choice = 1, int runOverride = 0,
                     void (*setIcon)(SDL_Window*) = nullptr)
      : choice_(choice), runOverride_(runOverride), setIcon_(setIcon) {
#ifndef __EMSCRIPTEN__
    scale_ = desktopWindowScale(windowScaleChoice(choice, runOverride), 0);
#endif
  }
  SdlWindow(const SdlWindow&) = delete;
  SdlWindow& operator=(const SdlWindow&) = delete;
  ~SdlWindow() {
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    if (initialized_) quitWindowSubsystems();
  }
  std::expected<void, std::string> ensure(
      const char* title = "Lemmings Webolution " WEBOLUTION_VERSION_STRING) {
    if (renderer_) return {};
    if (choice_ < 0 || choice_ > kMaxFixedWindowScale || runOverride_ < 0 ||
        runOverride_ > kMaxFixedWindowScale)
      return std::unexpected("window scale must be 0, 1, 2 or 3");
    SDL_SetMainReady();
    if (!initialized_) {
      SDL_SetHintWithPriority(SDL_HINT_TOUCH_MOUSE_EVENTS, "0", SDL_HINT_OVERRIDE);
      SDL_SetHintWithPriority(SDL_HINT_MOUSE_TOUCH_EVENTS, "0", SDL_HINT_OVERRIDE);
      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) return error("SDL_Init");
      initialized_ = true;
#ifdef __EMSCRIPTEN__
      // The level loop enables wheel zoom during examine pause.
      SDL_EventState(SDL_MOUSEWHEEL, SDL_IGNORE);
#endif
    }
    if (!window_) {
#ifdef __EMSCRIPTEN__
      constexpr bool desktop = false;
      const int initialScale = 1;
      const auto flags = SDL_WINDOW_SHOWN;
#else
      const bool desktop = hasDesktop();
      const int initialScale = desktop ? 1 : scale_;
      const auto flags = desktop ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN;
#endif
      window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
          kGameWidth * initialScale, kGameHeight * initialScale, flags);
      if (!window_) return error("SDL_CreateWindow");
      if (desktop && setIcon_) setIcon_(window_);
      applyWindowScale();
      if (desktop) SDL_ShowWindow(window_);
    }
    // Scale the final texture on the GPU when available. Headless
    // drivers use the software fallback. The renderer choice is scale-independent.
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_) return error("SDL_CreateRenderer");
    return {};
  }
  SDL_Renderer* renderer() const { return renderer_; }
  int scale() const { return scale_; }
#ifdef __EMSCRIPTEN__
  void setWebViewport(LevelLayout layout) {
    SDL_SetWindowSize(window_, layout.width(), layout.height());
  }
#endif
  void updateWindowScale(std::optional<int> choice, bool displayChanged = false) {
    const bool changed = choice && *choice != choice_;
    if (changed) choice_ = *choice;
    if (window_ && (displayChanged || (changed && !runOverride_)))
      applyWindowScale(changed && !runOverride_);
  }
  WindowScaleMenu windowScaleMenu() const {
    WindowScaleMenu info;
    info.runOverride = runOverride_;
#ifdef __EMSCRIPTEN__
    web::canvasFillSize(info.fillWidth, info.fillHeight);
#else
    if (const auto area = desktopArea()) info.largestDesktopFit = area->largestScale();
#endif
    return info;
  }
  // Provisional clone coordinates. Map SDL events once to render pixels.
  // Callers divide relative deltas in floating point. Web scale stays 1
  // because SDL already maps the CSS canvas rectangle.
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

  bool receivedTouch() const { return receivedTouch_; }
  std::optional<TouchPoint> touchCoordinates(const SDL_TouchFingerEvent& event) {
    if (event.touchId == SDL_MOUSE_TOUCHID ||
        (event.windowID && event.windowID != SDL_GetWindowID(window_)) ||
        !std::isfinite(event.x) || !std::isfinite(event.y)) return std::nullopt;
    receivedTouch_ = true;
    int width, height;
    SDL_GetWindowSize(window_, &width, &height);
    // SDL reports normalized client coordinates. Keep subpixel drag deltas;
    // map clicks to pixels in the same client-to-presentation scale as mouse input.
    const auto axis = [this](double normalized, int size) {
      const double extent = double(size) / scale_;
      return normalized == 1.0 ? std::nextafter(extent, 0.0) : normalized * extent;
    };
    return TouchPoint{axis(event.x, width), axis(event.y, height)};
  }

  static TouchAction feedTouch(TouchInput& touch, const SDL_TouchFingerEvent& event,
                               TouchPoint point, TouchTarget target = TouchTarget::Tap) {
    const TouchId id{event.touchId, event.fingerId};
    if (event.type == SDL_FINGERDOWN) return touch.down(id, point, target, event.timestamp);
    if (event.type == SDL_FINGERMOTION) return touch.motion(id, point);
    return touch.up(id, point, event.timestamp);
  }

 private:
#ifndef __EMSCRIPTEN__
  struct DesktopArea {
    SDL_Rect usable{};
    int top = 0, left = 0, bottom = 0, right = 0;
    int largestScale() const {
      return largestFittingWindowScale(usable.w - left - right, usable.h - top - bottom);
    }
  };

  static bool hasDesktop() {
    // These drivers supply synthetic bounds. Offscreen output can use any scale.
    const char* name = SDL_GetCurrentVideoDriver();
    return name && std::string_view(name) != "dummy" && std::string_view(name) != "offscreen";
  }

  std::optional<DesktopArea> desktopArea() const {
    if (!window_ || !hasDesktop()) return std::nullopt;
    const int display = SDL_GetWindowDisplayIndex(window_);
    DesktopArea area;
    if (display < 0 || SDL_GetDisplayUsableBounds(display, &area.usable) != 0)
      return std::nullopt;
    if (SDL_GetWindowBordersSize(window_, &area.top, &area.left, &area.bottom, &area.right) != 0)
      area.top = 32; // Provisional title-bar allowance on drivers without border metrics.
    return area;
  }
#endif

  void applyWindowScale([[maybe_unused]] bool recenter = true) {
    const int choice = windowScaleChoice(choice_, runOverride_);
#ifdef __EMSCRIPTEN__
    web::setCanvasScale(choice);
#else
    const auto area = desktopArea();
    scale_ = desktopWindowScale(choice, area ? area->largestScale() : 0);
    const int width = kGameWidth * scale_, height = kGameHeight * scale_;
    int currentWidth, currentHeight;
    SDL_GetWindowSize(window_, &currentWidth, &currentHeight);
    const bool resized = width != currentWidth || height != currentHeight;
    if (resized)
      SDL_SetWindowSize(window_, width, height);
    if (!resized && !recenter) return;
    if (area) {
      // SDL's window position is the client origin. Center the outer frame.
      SDL_SetWindowPosition(window_,
          area->usable.x + (area->usable.w - width + area->left - area->right) / 2,
          area->usable.y + (area->usable.h - height + area->top - area->bottom) / 2);
    } else if (hasDesktop()) {
      const int centered = SDL_WINDOWPOS_CENTERED_DISPLAY(std::max(0, SDL_GetWindowDisplayIndex(window_)));
      SDL_SetWindowPosition(window_, centered, centered);
    }
#endif
  }

  static std::unexpected<std::string> error(const char* action) {
    return std::unexpected(std::string(action) + ": " + SDL_GetError());
  }
  bool initialized_ = false;
  bool receivedTouch_ = false;
  int choice_, runOverride_;
  void (*setIcon_)(SDL_Window*);
  int scale_ = 1;
  SDL_Window* window_ = nullptr;
  SDL_Renderer* renderer_ = nullptr;
};
}  // namespace webo::app
#endif
