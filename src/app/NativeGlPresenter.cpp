#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>

#include "app/PresentationBackend.h"
#include "app/FrameProfile.h"
#include "app/PresentationShaders.h"
#include "app/WindowScale.h"

#include <algorithm>
#include <cstdio>
#include <format>

namespace webo::app {
namespace {

// Load every entry point from the SDL context, including the Windows GL 1.1
// exports. Types from SDL's GL headers retain the platform calling convention.
#define PRESENTATION_GL_FUNCTIONS(X) \
  X(GetString, decltype(&::glGetString)) \
  X(GetIntegerv, decltype(&::glGetIntegerv)) \
  X(GetError, decltype(&::glGetError)) \
  X(Disable, decltype(&::glDisable)) \
  X(Viewport, decltype(&::glViewport)) \
  X(DrawArrays, decltype(&::glDrawArrays)) \
  X(PixelStorei, decltype(&::glPixelStorei)) \
  X(ReadPixels, decltype(&::glReadPixels)) \
  X(ReadBuffer, decltype(&::glReadBuffer)) \
  X(GenTextures, decltype(&::glGenTextures)) \
  X(DeleteTextures, decltype(&::glDeleteTextures)) \
  X(BindTexture, decltype(&::glBindTexture)) \
  X(TexParameteri, decltype(&::glTexParameteri)) \
  X(TexImage2D, decltype(&::glTexImage2D)) \
  X(TexSubImage2D, decltype(&::glTexSubImage2D)) \
  X(ActiveTexture, PFNGLACTIVETEXTUREPROC) \
  X(CreateShader, PFNGLCREATESHADERPROC) \
  X(DeleteShader, PFNGLDELETESHADERPROC) \
  X(ShaderSource, PFNGLSHADERSOURCEPROC) \
  X(CompileShader, PFNGLCOMPILESHADERPROC) \
  X(GetShaderiv, PFNGLGETSHADERIVPROC) \
  X(GetShaderInfoLog, PFNGLGETSHADERINFOLOGPROC) \
  X(CreateProgram, PFNGLCREATEPROGRAMPROC) \
  X(DeleteProgram, PFNGLDELETEPROGRAMPROC) \
  X(AttachShader, PFNGLATTACHSHADERPROC) \
  X(LinkProgram, PFNGLLINKPROGRAMPROC) \
  X(GetProgramiv, PFNGLGETPROGRAMIVPROC) \
  X(GetProgramInfoLog, PFNGLGETPROGRAMINFOLOGPROC) \
  X(UseProgram, PFNGLUSEPROGRAMPROC) \
  X(GenVertexArrays, PFNGLGENVERTEXARRAYSPROC) \
  X(DeleteVertexArrays, PFNGLDELETEVERTEXARRAYSPROC) \
  X(BindVertexArray, PFNGLBINDVERTEXARRAYPROC) \
  X(GenFramebuffers, PFNGLGENFRAMEBUFFERSPROC) \
  X(DeleteFramebuffers, PFNGLDELETEFRAMEBUFFERSPROC) \
  X(BindFramebuffer, PFNGLBINDFRAMEBUFFERPROC) \
  X(FramebufferTexture2D, PFNGLFRAMEBUFFERTEXTURE2DPROC) \
  X(CheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC) \
  X(GetFramebufferAttachmentParameteriv, PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)

struct GlFunctions {
#define DECLARE_GL(name, type) type name = nullptr;
  PRESENTATION_GL_FUNCTIONS(DECLARE_GL)
#undef DECLARE_GL

  std::expected<void, std::string> load() {
#define LOAD_GL(name, type) \
    name = reinterpret_cast<type>(SDL_GL_GetProcAddress("gl" #name)); \
    if (!name) return std::unexpected("OpenGL entry point missing: gl" #name);
    PRESENTATION_GL_FUNCTIONS(LOAD_GL)
#undef LOAD_GL
    return {};
  }
};
#undef PRESENTATION_GL_FUNCTIONS

std::unexpected<std::string> sdlError(const char* action) {
  return std::unexpected(std::string(action) + ": " + SDL_GetError());
}

class NativeGlPresenter final : public GlPresenter {
 public:
  ~NativeGlPresenter() override {
    if (context_) {
      if (SDL_GL_GetCurrentContext() == context_) {
        if (target_) gl_.DeleteFramebuffers(1, &target_);
        if (targetColour_) gl_.DeleteTextures(1, &targetColour_);
        if (source_) gl_.DeleteTextures(1, &source_);
        if (vao_) gl_.DeleteVertexArrays(1, &vao_);
        if (program_) gl_.DeleteProgram(program_);
      }
      SDL_GL_DeleteContext(context_);
    }
    if (window_) SDL_DestroyWindow(window_);
  }

  std::expected<void, std::string> start(const char* title, int width, int height, bool hidden) {
    struct ResetAttributes {
      ~ResetAttributes() { SDL_GL_ResetAttributes(); }
    } resetAttributes;
    SDL_GL_ResetAttributes();
    constexpr std::pair<SDL_GLattr, int> attributes[] = {
        {SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE},
        {SDL_GL_CONTEXT_MAJOR_VERSION, 3}, {SDL_GL_CONTEXT_MINOR_VERSION, 3},
        {SDL_GL_RED_SIZE, 8}, {SDL_GL_GREEN_SIZE, 8}, {SDL_GL_BLUE_SIZE, 8},
        {SDL_GL_DOUBLEBUFFER, 1}, {SDL_GL_DEPTH_SIZE, 0},
        {SDL_GL_MULTISAMPLEBUFFERS, 0}, {SDL_GL_MULTISAMPLESAMPLES, 0},
    };
    for (const auto& [attribute, value] : attributes)
      if (SDL_GL_SetAttribute(attribute, value) != 0) return sdlError("SDL_GL_SetAttribute");

    window_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width, height, SDL_WINDOW_OPENGL |
                                   (hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN));
    if (!window_) return sdlError("SDL_CreateWindow (OpenGL)");
    context_ = SDL_GL_CreateContext(window_);
    if (!context_) return sdlError("SDL_GL_CreateContext");
    if (!current()) return std::unexpected("SDL did not make the OpenGL context current");
    if (auto loaded = gl_.load(); !loaded) return loaded;

    GLint major = 0, minor = 0, profile = 0;
    gl_.GetIntegerv(GL_MAJOR_VERSION, &major);
    gl_.GetIntegerv(GL_MINOR_VERSION, &minor);
    gl_.GetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
    if (major < 3 || (major == 3 && minor < 3) || !(profile & GL_CONTEXT_CORE_PROFILE_BIT))
      return std::unexpected(std::format("OpenGL 3.3 core required (got {}.{}, profile 0x{:x})",
                                         major, minor, profile));

    int drawableWidth = 0, drawableHeight = 0;
    SDL_GL_GetDrawableSize(window_, &drawableWidth, &drawableHeight);
    if (drawableWidth <= 0 || drawableHeight <= 0)
      return std::unexpected("OpenGL window has no drawable");
    if (auto status = framebufferStatus("window"); !status) return status;
    GLint doubleBuffered = 0, red = 0, green = 0, blue = 0;
    gl_.GetIntegerv(GL_DOUBLEBUFFER, &doubleBuffered);
    gl_.GetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT,
                                            GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &red);
    gl_.GetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT,
                                            GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &green);
    gl_.GetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT,
                                            GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &blue);
    if (!doubleBuffered || red < 8 || green < 8 || blue < 8)
      return std::unexpected("OpenGL window needs a double-buffered RGB8 drawable");

    if (auto linked = createProgram(); !linked) return linked;
    gl_.GenVertexArrays(1, &vao_);
    gl_.BindVertexArray(vao_);
    gl_.UseProgram(program_);
    // The only program sampler defaults to texture unit zero.
    gl_.ActiveTexture(GL_TEXTURE0);
    gl_.Disable(GL_BLEND);
    gl_.Disable(GL_DEPTH_TEST);
    gl_.Disable(GL_DITHER);
    gl_.Disable(GL_MULTISAMPLE);
    gl_.Disable(GL_FRAMEBUFFER_SRGB);
    gl_.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    gl_.PixelStorei(GL_PACK_ALIGNMENT, 1);

    allocateTexture(source_);
    allocateTexture(targetColour_);
    gl_.GenFramebuffers(1, &target_);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, target_);
    gl_.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetColour_, 0);
    if (auto status = framebufferStatus("comparison"); !status) return status;
    gl_.ReadBuffer(GL_COLOR_ATTACHMENT0);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, 0);
    if (!vao_ || !source_ || !targetColour_ || !target_)
      return std::unexpected("OpenGL did not allocate the presentation objects");
    if (auto status = glStatus("OpenGL presenter initialization"); !status) return status;
    description_ = std::format(
        "OpenGL {}.{} core [{}; {}]; window framebuffer complete (RGB {}/{}/{}, double-buffered); "
        "comparison framebuffer complete ({}x{} RGBA8); swap interval={}",
        major, minor, glString(GL_VERSION), glString(GL_RENDERER),
        red, green, blue, kGameWidth, kGameHeight, SDL_GL_GetSwapInterval());
    return {};
  }

  SDL_Window* window() const override { return window_; }
  const std::string& description() const override { return description_; }

  std::expected<void, std::string> present(const Image& image, Image* readback) override {
    if (!current()) return std::unexpected("OpenGL presentation requires its current SDL context");
    if (image.width != kGameWidth || image.height != kGameHeight ||
        image.rgba.size() != std::size_t(kGameWidth) * kGameHeight * 4)
      return std::unexpected("OpenGL presentation requires a tightly packed 640x480 RGBA image");

    {
      profile::Scope scope(profile::Upload);
      gl_.BindTexture(GL_TEXTURE_2D, source_);
      // Keep straight alpha, and upload top-down. The vertex shader maps row 0 to the top.
      gl_.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kGameWidth, kGameHeight,
                        GL_RGBA, GL_UNSIGNED_BYTE, image.rgba.data());
    }
    if (readback) {
      const auto start = profile::Clock::now();
      gl_.BindFramebuffer(GL_FRAMEBUFFER, target_);
      draw(kGameWidth, kGameHeight);
      readComparison(*readback, image.alpha);
      gl_.BindFramebuffer(GL_FRAMEBUFFER, 0);
      const auto elapsed = std::chrono::duration<double, std::milli>(profile::Clock::now() - start);
      std::fprintf(stderr, "gl checkpoint: offscreen draw and readback %.3f ms\n", elapsed.count());
    }
    int width = 0, height = 0;
    SDL_GL_GetDrawableSize(window_, &width, &height);
    if (width > 0 && height > 0) {
      { profile::Scope scope(profile::Copy); draw(width, height); }
      { profile::Scope scope(profile::Present); SDL_GL_SwapWindow(window_); }
    }
    return glStatus("OpenGL presentation");
  }

 private:
  bool current() const {
    return SDL_GL_GetCurrentContext() == context_ && SDL_GL_GetCurrentWindow() == window_;
  }

  const char* glString(GLenum key) const {
    const auto* value = gl_.GetString(key);
    return value ? reinterpret_cast<const char*>(value) : "unknown";
  }

  std::expected<void, std::string> glStatus(const char* operation) const {
    const GLenum error = gl_.GetError();
    if (error != GL_NO_ERROR)
      return std::unexpected(std::format("{} failed (GL error 0x{:x})", operation, error));
    return {};
  }

  std::expected<void, std::string> framebufferStatus(const char* name) const {
    const GLenum status = gl_.CheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
      return std::unexpected(std::format("OpenGL {} framebuffer incomplete (0x{:x})", name, status));
    return {};
  }

  std::expected<GLuint, std::string> compile(GLenum type, const char* stage, std::string_view body) {
    const GLuint shader = gl_.CreateShader(type);
    if (!shader) return std::unexpected(std::format("OpenGL could not create the {} shader", stage));
    const auto source = presentationShader(ShaderDialect::DesktopCore, body);
    const auto* text = source.c_str();
    gl_.ShaderSource(shader, 1, &text, nullptr);
    gl_.CompileShader(shader);
    GLint compiled = GL_FALSE;
    gl_.GetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_TRUE) return shader;
    char log[2048]{};
    gl_.GetShaderInfoLog(shader, sizeof(log), nullptr, log);
    gl_.DeleteShader(shader);
    return std::unexpected(std::format("OpenGL {} shader: {}", stage, log));
  }

  std::expected<void, std::string> createProgram() {
    program_ = gl_.CreateProgram();
    if (!program_) return std::unexpected("OpenGL could not create the presentation program");
    const auto vertex = compile(GL_VERTEX_SHADER, "vertex", kPresentationVertex);
    if (!vertex) return std::unexpected(vertex.error());
    const auto fragment = compile(GL_FRAGMENT_SHADER, "fragment", kPresentationFragment);
    if (!fragment) {
      gl_.DeleteShader(*vertex);
      return std::unexpected(fragment.error());
    }
    gl_.AttachShader(program_, *vertex);
    gl_.AttachShader(program_, *fragment);
    gl_.LinkProgram(program_);
    gl_.DeleteShader(*vertex);
    gl_.DeleteShader(*fragment);
    GLint linked = GL_FALSE;
    gl_.GetProgramiv(program_, GL_LINK_STATUS, &linked);
    if (linked == GL_TRUE) return {};
    char log[2048]{};
    gl_.GetProgramInfoLog(program_, sizeof(log), nullptr, log);
    return std::unexpected(std::format("OpenGL presentation program: {}", log));
  }

  void allocateTexture(GLuint& texture) {
    gl_.GenTextures(1, &texture);
    gl_.BindTexture(GL_TEXTURE_2D, texture);
    gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl_.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kGameWidth, kGameHeight, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  }

  void draw(int width, int height) const {
    gl_.Viewport(0, 0, width, height);
    gl_.DrawArrays(GL_TRIANGLES, 0, 3);
  }

  void readComparison(Image& image, AlphaMode alpha) const {
    image.width = kGameWidth;
    image.height = kGameHeight;
    image.alpha = alpha;
    image.rgba.resize(image.pixelCount() * 4);
    gl_.ReadPixels(0, 0, kGameWidth, kGameHeight, GL_RGBA, GL_UNSIGNED_BYTE, image.rgba.data());
    // glReadPixels starts at the lower edge of the FBO.
    constexpr int rowBytes = kGameWidth * 4;
    for (int y = 0; y < kGameHeight / 2; ++y) {
      auto row = image.rgba.begin() + y * rowBytes;
      auto other = image.rgba.begin() + (kGameHeight - 1 - y) * rowBytes;
      std::swap_ranges(row, row + rowBytes, other);
    }
  }

  SDL_Window* window_ = nullptr;
  SDL_GLContext context_ = nullptr;
  GlFunctions gl_;
  GLuint program_ = 0, vao_ = 0, source_ = 0, targetColour_ = 0, target_ = 0;
  std::string description_;
};

}  // namespace

std::expected<std::unique_ptr<GlPresenter>, std::string> createNativeGlPresenter(
    const char* title, int width, int height, bool hidden) {
  auto presenter = std::make_unique<NativeGlPresenter>();
  if (auto started = presenter->start(title, width, height, hidden); !started)
    return std::unexpected(started.error());
  return presenter;
}

}  // namespace webo::app
