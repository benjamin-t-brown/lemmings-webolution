#pragma once

#include "assets/Bitmap.h"
#include "shell/Frontend.h"

#include <cstdint>
#include <expected>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

struct SDL_Window;

namespace webo::app {

enum class RendererChoice { Software, Gl, Auto };

struct PreparedLevelLayers;

struct PreparedPage {
  const Image* image = nullptr;
  shell::Screen screen{};
  std::uint64_t revision = 0;
};

// Borrowed, bottom-up textures from the current GL level leg.
struct GlSceneImage {
  unsigned colour = 0, depth = 0;
  int width = 640, height = 480;
  bool depth64 = false;
  unsigned highlights = 0;
};

// The presenter owns its window and context. The application borrows the
// window for SDL input and sizing on the same thread.
class GlPresenter {
 public:
  virtual ~GlPresenter() = default;
  virtual SDL_Window* window() const = 0;
  virtual const std::string& description() const = 0;
  virtual bool gpuActive() const { return true; }
  virtual bool contextLost() const { return false; }
  // An empty reason polls for context loss. A renderer error may also request
  // a fallback. The browser keeps its SDL input window when it switches.
  virtual std::expected<bool, std::string> fallbackToSoftware(std::string_view) { return false; }
  virtual std::expected<void, std::string> present(const Image& image, Image* readback) = 0;
  virtual bool gpuComposition() const { return false; }
  virtual std::expected<void, std::string> composeGl(const GlSceneImage&, const PreparedPage&,
                                                        const PreparedLevelLayers&) {
    return std::unexpected("OpenGL composition is unavailable");
  }
  virtual std::expected<void, std::string> readGl(Image&) {
    return std::unexpected("OpenGL readback is unavailable");
  }
  virtual std::expected<void, std::string> showGl() {
    return std::unexpected("OpenGL presentation is unavailable");
  }
  virtual void releaseLayerAssets() {}
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
