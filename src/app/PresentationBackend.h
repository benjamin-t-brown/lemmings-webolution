#pragma once

#include "assets/Bitmap.h"
#include "shell/Frontend.h"

#include <expected>
#include <functional>
#include <memory>
#include <string>

struct SDL_Window;

namespace webo::app {

enum class RendererChoice { Software, Gl, Auto };

// The presenter owns its window and context. The application borrows the
// window for SDL input and sizing on the same thread.
class GlPresenter {
 public:
  virtual ~GlPresenter() = default;
  virtual SDL_Window* window() const = 0;
  virtual const std::string& description() const = 0;
  virtual bool gpuActive() const { return true; }
  virtual std::expected<void, std::string> present(const Image& image, Image* readback) = 0;
};

using CreateGlPresenter = std::expected<std::unique_ptr<GlPresenter>, std::string> (*)(
    const char* title, int width, int height, bool hidden);

std::expected<std::unique_ptr<GlPresenter>, std::string> createNativeGlPresenter(
    const char* title, int width, int height, bool hidden);
std::expected<std::unique_ptr<GlPresenter>, std::string> createWebGlPresenter(
    const char* title, int width, int height, bool hidden);

struct GlFrameComparison {
  std::function<bool(shell::Screen, int frame)> checkpoint;
  // Both images are top-down logical RGBA8, valid for the duration of the call.
  std::function<void(shell::Screen, int frame, const Image& cpu, const Image& gpu)> compare;
};

}  // namespace webo::app
