#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <emscripten.h>

#include "app/FrameProfile.h"
#include "app/NativeCompositor.h"
#include "app/PresentationBackend.h"
#include "app/WindowScale.h"

#include <cstdio>
#include <format>

namespace webo::app {
namespace {

// SDL keeps the input and software surface on #canvas. A DOM canvas cannot
// switch context type, so the GL canvas covers it without taking its input.
EM_JS(bool, addWebGlCanvas, (int width, int height), {
  const input = Module['canvas'];
  if (!input?.parentElement) return false;
  const canvas = document.createElement('canvas');
  canvas.id = 'webgl-canvas';
  canvas.width = width;
  canvas.height = height;
  canvas.hidden = true;
  canvas.setAttribute('aria-hidden', 'true');
  input.after(canvas);
  return true;
});

EM_JS(void, showWebGlCanvas, (), { document.getElementById('webgl-canvas').hidden = false; });
EM_JS(void, removeWebGlCanvas, (), { document.getElementById('webgl-canvas')?.remove(); });

std::unexpected<std::string> sdlError(const char* operation) {
  return std::unexpected(std::string(operation) + ": " + SDL_GetError());
}

class WebGlPresenter final : public GlPresenter {
 public:
  ~WebGlPresenter() override {
    releaseContext();
    if (hasCanvas_) removeWebGlCanvas();
    if (softwareRenderer_) SDL_DestroyRenderer(softwareRenderer_);  // Owns the texture.
    if (window_) SDL_DestroyWindow(window_);
  }

  std::expected<void, std::string> start(const char* title, int width, int height) {
    hasCanvas_ = addWebGlCanvas(width, height);
    if (!hasCanvas_) return std::unexpected("WebGL2 requires a browser display canvas");
    EmscriptenWebGLContextAttributes attributes;
    emscripten_webgl_init_context_attributes(&attributes);
    attributes.majorVersion = 2;
    attributes.minorVersion = 0;
    attributes.alpha = attributes.depth = attributes.stencil = attributes.antialias = false;
    attributes.enableExtensionsByDefault = false;
    attributes.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_DISALLOW;
    const auto created = emscripten_webgl_create_context("#webgl-canvas", &attributes);
    if (created <= 0) return std::unexpected("WebGL2 context creation failed");
    context_ = created;
    if (emscripten_webgl_make_context_current(context_) != EMSCRIPTEN_RESULT_SUCCESS)
      return std::unexpected("Could not make the WebGL2 context current");

    const auto* rawVersion = glGetString(GL_VERSION);
    const std::string_view version = rawVersion ? reinterpret_cast<const char*>(rawVersion) : "";
    if (!version.starts_with("OpenGL ES 3.0 (WebGL 2.0"))
      return std::unexpected(std::format("WebGL2 / OpenGL ES 3.0 required (got '{}')", version));
    int drawableWidth = 0, drawableHeight = 0;
    if (emscripten_webgl_get_drawing_buffer_size(context_, &drawableWidth, &drawableHeight) !=
            EMSCRIPTEN_RESULT_SUCCESS || drawableWidth != width || drawableHeight != height)
      return std::unexpected("WebGL2 canvas has the wrong drawing buffer size");
    GLint red = 0, green = 0, blue = 0;
    glGetIntegerv(GL_RED_BITS, &red);
    glGetIntegerv(GL_GREEN_BITS, &green);
    glGetIntegerv(GL_BLUE_BITS, &blue);
    if (red < 8 || green < 8 || blue < 8 || glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      return std::unexpected("WebGL2 canvas needs a complete RGB8 framebuffer");

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if (auto loaded = gl_.load(); !loaded) return loaded;
    compositor_.emplace(gl_);
    if (auto ready = compositor_->start(); !ready) return ready;
    if (auto ready = compositor_->allocateTarget(kGameWidth, kGameHeight); !ready) return ready;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width, height, SDL_WINDOW_SHOWN);
    if (!window_) return sdlError("SDL_CreateWindow (WebGL2 input)");
    description_ = std::format("{}; canvas framebuffer complete (RGB {}/{}/{}); "
                               "logical framebuffer complete ({}x{} RGBA8)",
                               version, red, green, blue, kGameWidth, kGameHeight);
    return {};
  }

  SDL_Window* window() const override { return window_; }
  const std::string& description() const override { return description_; }
  bool gpuActive() const override { return context_ != 0; }
  bool contextLost() const override { return context_ && emscripten_is_webgl_context_lost(context_); }
  bool gpuComposition() const override { return gpuActive(); }

  std::expected<void, std::string> composeGl(const GlSceneImage& scene, const PreparedPage& page,
                                                const PreparedLevelLayers& layers) override {
    if (!current()) return std::unexpected("WebGL2 composition lost its current context");
    return compositor_->compose(scene, page, layers);
  }

  std::expected<void, std::string> readGl(Image& image) override {
    if (!current()) return std::unexpected("WebGL2 readback lost its current context");
    return compositor_->read(image);
  }

  std::expected<void, std::string> showGl() override {
    if (!current()) return std::unexpected("WebGL2 presentation lost its current context");
    int width = 0, height = 0;
    if (emscripten_webgl_get_drawing_buffer_size(context_, &width, &height) != EMSCRIPTEN_RESULT_SUCCESS)
      return std::unexpected("WebGL2 canvas has no drawable");
    {
      profile::Scope scope(profile::Copy);
      if (auto copied = compositor_->copyToWindow(width, height); !copied) return copied;
    }
    // The browser composites when the existing animation-frame loop yields.
    if (!shown_) {
      showWebGlCanvas();
      shown_ = true;
    }
    return {};
  }

  void releaseLayerAssets() override { if (current()) compositor_->releaseAssets(); }

  std::expected<bool, std::string> fallbackToSoftware(std::string_view reason) override {
    if (!gpuActive()) return false;
    const bool lost = contextLost();
    if (reason.empty() && !lost) return false;
    releaseContext();
    if (auto ready = startSoftware(); !ready) return std::unexpected(ready.error());
    removeWebGlCanvas();
    hasCanvas_ = false;
    if (lost) reason = "WebGL2 context lost";
    std::fprintf(stderr, "renderer fallback: gl -> software; %.*s\n",
                 static_cast<int>(reason.size()), reason.data());
    return true;
  }

  std::expected<void, std::string> present(const Image& image, Image* readback) override {
    if (!softwareTexture_) return std::unexpected("WebGL2 requires a prepared frame");
    if (image.width != kGameWidth || image.height != kGameHeight ||
        image.rgba.size() != std::size_t(kGameWidth) * kGameHeight * 4)
      return std::unexpected("software presentation requires a tightly packed 640x480 RGBA image");
    if (SDL_UpdateTexture(softwareTexture_, nullptr, image.rgba.data(), kGameWidth * 4) != 0 ||
        SDL_RenderClear(softwareRenderer_) != 0 ||
        SDL_RenderCopy(softwareRenderer_, softwareTexture_, nullptr, nullptr) != 0)
      return sdlError("SDL rendering (WebGL2 fallback)");
    SDL_RenderPresent(softwareRenderer_);
    if (readback) *readback = image;
    return {};
  }

 private:
  bool current() const {
    return context_ && emscripten_webgl_get_current_context() == context_ &&
           !emscripten_is_webgl_context_lost(context_);
  }

  void releaseContext() {
    if (!context_) return;
    compositor_.reset();
    if (emscripten_webgl_get_current_context() == context_) emscripten_webgl_make_context_current(0);
    emscripten_webgl_destroy_context(context_);
    context_ = 0;
  }

  std::expected<void, std::string> startSoftware() {
    softwareRenderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!softwareRenderer_) return sdlError("SDL_CreateRenderer (WebGL2 fallback)");
    softwareTexture_ = SDL_CreateTexture(softwareRenderer_, SDL_PIXELFORMAT_RGBX32,
                                         SDL_TEXTUREACCESS_STREAMING, kGameWidth, kGameHeight);
    if (!softwareTexture_) return sdlError("SDL_CreateTexture (WebGL2 fallback)");
    if (SDL_SetTextureScaleMode(softwareTexture_, SDL_ScaleModeNearest) != 0)
      return sdlError("SDL_SetTextureScaleMode (WebGL2 fallback)");
    return {};
  }

  SDL_Window* window_ = nullptr;
  SDL_Renderer* softwareRenderer_ = nullptr;
  SDL_Texture* softwareTexture_ = nullptr;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_ = 0;
  GlFunctions gl_;
  std::optional<GlCompositor> compositor_;
  bool hasCanvas_ = false, shown_ = false;
  std::string description_;
};

}  // namespace

std::expected<std::unique_ptr<GlPresenter>, std::string> createWebGlPresenter(
    const char* title, int width, int height, bool /*hidden*/) {
  auto presenter = std::make_unique<WebGlPresenter>();
  if (auto started = presenter->start(title, width, height); !started)
    return std::unexpected(started.error());
  return presenter;
}

}  // namespace webo::app
