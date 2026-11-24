#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5_webgl.h>

#include "app/FrameProfile.h"
#include "app/PresentationBackend.h"
#include "app/PresentationShaders.h"
#include "app/WindowScale.h"

#include <algorithm>
#include <cstdio>
#include <format>

namespace webo::app {
namespace {

// SDL's events and software surface always use #canvas. WebGL gets a separate
// canvas because a DOM canvas cannot change its context type once initialized.
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

std::expected<void, std::string> glStatus(const char* operation) {
  const auto error = glGetError();
  if (error == GL_NO_ERROR) return {};
  return std::unexpected(std::format("{}: WebGL error 0x{:x}", operation, error));
}

std::expected<GLuint, std::string> compile(GLenum type, std::string_view body) {
  const auto shader = glCreateShader(type);
  if (!shader) return std::unexpected("WebGL2 could not allocate a presentation shader");
  const auto source = presentationShader(ShaderDialect::Embedded, body);
  const char* text = source.c_str();
  glShaderSource(shader, 1, &text, nullptr);
  glCompileShader(shader);
  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_TRUE) return shader;
  char log[2048]{};
  glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
  glDeleteShader(shader);
  return std::unexpected(std::format("WebGL2 presentation shader: {}", log));
}

GLuint allocateTexture() {
  GLuint texture = 0;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kGameWidth, kGameHeight, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  return texture;
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
    attributes.majorVersion = 2;  // WebGL 2.0 implements the GLES 3.0 subset.
    attributes.minorVersion = 0;
    attributes.alpha = attributes.depth = attributes.stencil = attributes.antialias = false;
    attributes.enableExtensionsByDefault = false;
    attributes.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_DISALLOW;
    context_ = emscripten_webgl_create_context("#webgl-canvas", &attributes);
    if (!context_) return std::unexpected("WebGL2 context creation failed");
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

    if (auto linked = createProgram(); !linked) return linked;
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glUseProgram(program_);
    glActiveTexture(GL_TEXTURE0);  // The sampler defaults to unit zero.
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DITHER);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    source_ = allocateTexture();
    targetColour_ = allocateTexture();
    glGenFramebuffers(1, &target_);
    glBindFramebuffer(GL_FRAMEBUFFER, target_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetColour_, 0);
    if (!vao_ || !source_ || !targetColour_ || !target_ ||
        glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      return std::unexpected("WebGL2 could not allocate the logical RGBA8 framebuffer");
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (auto status = glStatus("WebGL2 presenter initialization"); !status) return status;

    window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width, height, SDL_WINDOW_SHOWN);
    if (!window_) return sdlError("SDL_CreateWindow (WebGL2 input)");
    description_ = std::format("{}; canvas framebuffer complete (RGB {}/{}/{}); "
                               "comparison framebuffer complete ({}x{} RGBA8)",
                               version, red, green, blue, kGameWidth, kGameHeight);
    return {};
  }

  SDL_Window* window() const override { return window_; }
  const std::string& description() const override { return description_; }
  bool gpuActive() const override { return context_ != 0; }

  std::expected<void, std::string> present(const Image& image, Image* readback) override {
    if (image.width != kGameWidth || image.height != kGameHeight ||
        image.rgba.size() != std::size_t(kGameWidth) * kGameHeight * 4)
      return std::unexpected("WebGL2 presentation requires a tightly packed 640x480 RGBA image");

    if (context_) {
      auto shown = presentGpu(image, readback);
      if (shown) return {};
      const char* reason = emscripten_is_webgl_context_lost(context_) ? "WebGL2 context lost" : shown.error().c_str();
      releaseContext();
      if (auto ready = startSoftware(); !ready) return ready;
      if (auto fallback = presentSoftware(image, readback); !fallback) return fallback;
      removeWebGlCanvas();
      hasCanvas_ = false;
      std::fprintf(stderr, "renderer fallback: gl -> software; %s\n", reason);
      return {};
    }
    return presentSoftware(image, readback);
  }

 private:
  std::expected<void, std::string> createProgram() {
    program_ = glCreateProgram();
    if (!program_) return std::unexpected("WebGL2 could not create the presentation program");
    const auto vertex = compile(GL_VERTEX_SHADER, kPresentationVertex);
    if (!vertex) return std::unexpected(vertex.error());
    const auto fragment = compile(GL_FRAGMENT_SHADER, kPresentationFragment);
    if (!fragment) {
      glDeleteShader(*vertex);
      return std::unexpected(fragment.error());
    }
    glAttachShader(program_, *vertex);
    glAttachShader(program_, *fragment);
    glLinkProgram(program_);
    glDeleteShader(*vertex);
    glDeleteShader(*fragment);
    GLint linked = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &linked);
    if (linked == GL_TRUE) return {};
    char log[2048]{};
    glGetProgramInfoLog(program_, sizeof(log), nullptr, log);
    return std::unexpected(std::format("WebGL2 presentation program: {}", log));
  }

  std::expected<void, std::string> presentGpu(const Image& image, Image* readback) {
    if (emscripten_is_webgl_context_lost(context_)) return std::unexpected("WebGL2 context lost");
    if (emscripten_webgl_get_current_context() != context_)
      return std::unexpected("WebGL2 presentation requires its current context");

    {
      profile::Scope scope(profile::Upload);
      glBindTexture(GL_TEXTURE_2D, source_);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kGameWidth, kGameHeight,
                      GL_RGBA, GL_UNSIGNED_BYTE, image.rgba.data());
    }
    glViewport(0, 0, kGameWidth, kGameHeight);
    if (readback) {
      glBindFramebuffer(GL_FRAMEBUFFER, target_);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      readback->width = kGameWidth;
      readback->height = kGameHeight;
      readback->alpha = image.alpha;
      readback->rgba.resize(image.rgba.size());
      glReadPixels(0, 0, kGameWidth, kGameHeight, GL_RGBA, GL_UNSIGNED_BYTE, readback->rgba.data());
      constexpr int rowBytes = kGameWidth * 4;
      for (int y = 0; y < kGameHeight / 2; ++y) {
        auto row = readback->rgba.begin() + y * rowBytes;
        auto other = readback->rgba.begin() + (kGameHeight - 1 - y) * rowBytes;
        std::swap_ranges(row, row + rowBytes, other);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    { profile::Scope scope(profile::Copy); glDrawArrays(GL_TRIANGLES, 0, 3); }
    if (auto status = glStatus("WebGL2 presentation"); !status) return status;
    // The browser composites when the existing animation-frame loop yields.
    if (!shown_) {
      showWebGlCanvas();
      shown_ = true;
    }
    return {};
  }

  void releaseContext() {
    if (!context_) return;
    if (emscripten_webgl_get_current_context() == context_) {
      if (!emscripten_is_webgl_context_lost(context_)) {
        glDeleteFramebuffers(1, &target_);
        glDeleteTextures(1, &targetColour_);
        glDeleteTextures(1, &source_);
        glDeleteVertexArrays(1, &vao_);
        glDeleteProgram(program_);
      }
      emscripten_webgl_make_context_current(0);
    }
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

  std::expected<void, std::string> presentSoftware(const Image& image, Image* readback) {
    if (SDL_UpdateTexture(softwareTexture_, nullptr, image.rgba.data(), kGameWidth * 4) != 0 ||
        SDL_RenderClear(softwareRenderer_) != 0 ||
        SDL_RenderCopy(softwareRenderer_, softwareTexture_, nullptr, nullptr) != 0)
      return sdlError("SDL rendering (WebGL2 fallback)");
    SDL_RenderPresent(softwareRenderer_);
    if (readback) *readback = image;
    return {};
  }

  SDL_Window* window_ = nullptr;
  SDL_Renderer* softwareRenderer_ = nullptr;
  SDL_Texture* softwareTexture_ = nullptr;
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_ = 0;
  GLuint program_ = 0, vao_ = 0, source_ = 0, targetColour_ = 0, target_ = 0;
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
