#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>

#include "app/PresentationBackend.h"
#include "app/NativeGlApi.h"
#include "app/NativeCompositor.h"
#include "app/FrameProfile.h"
#include "app/PresentationShaders.h"
#include "app/WindowScale.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <format>

namespace webo::app {
namespace {

std::unexpected<std::string> sdlError(const char* action) {
  return std::unexpected(std::string(action) + ": " + SDL_GetError());
}

class NativeGlPresenter final : public GlPresenter {
 public:
  ~NativeGlPresenter() override {
    if (context_) {
      if (SDL_GL_GetCurrentContext() == context_) {
        releaseLevelGpuProfile();
        compositor_.reset();
        programs_.reset();
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
    const bool profiling = std::getenv("WEBOLUTION_FRAME_PROFILE") != nullptr;
    const auto start = profile::Clock::now();
    const auto logStartup = [&](const char* stage) {
      if (!profiling) return;
      const auto ms = std::chrono::duration<double, std::milli>(profile::Clock::now() - start);
      std::fprintf(stderr, "profile: native GL presenter %s %.3f ms\n", stage, ms.count());
    };
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
    logStartup("SDL window");
    context_ = SDL_GL_CreateContext(window_);
    if (!context_) return sdlError("SDL_GL_CreateContext");
    if (!current()) return std::unexpected("SDL did not make the OpenGL context current");
    SDL_GL_SetSwapInterval(0);
    logStartup("SDL window and context");
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

    gl_.ActiveTexture(GL_TEXTURE0);
    gl_.Disable(GL_BLEND);
    gl_.Disable(GL_DEPTH_TEST);
    gl_.Disable(GL_DITHER);
    gl_.Disable(GL_MULTISAMPLE);
    gl_.Disable(GL_FRAMEBUFFER_SRGB);
    gl_.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    gl_.PixelStorei(GL_PACK_ALIGNMENT, 1);

    if (auto status = glStatus("OpenGL presenter initialization"); !status) return status;
    programs_.emplace(gl_);
    compositor_.emplace(gl_);
    if (auto started = compositor_->start(); !started) return started;
    logStartup("compositor ready");
    description_ = std::format(
        "OpenGL {}.{} core [{}; {}]; window framebuffer complete (RGB {}/{}/{}, double-buffered); "
        "swap interval={}",
        major, minor, glString(GL_VERSION), glString(GL_RENDERER),
        red, green, blue, SDL_GL_GetSwapInterval());
    return {};
  }

  SDL_Window* window() const override { return window_; }
  const std::string& description() const override { return description_; }
  bool gpuComposition() const override { return true; }
  void setExactPageComposition(bool exact) override { exactPages_ = exact; }
  GlProgramCache* programCache() override { return &*programs_; }

  void beginLevelGpuProfile(int frame) override {
    if (!profile::active || timerActive_) return;
    profile::Scope scope(profile::GlQuery);
    if (!timerInitialized_) {
      timerInitialized_ = true;
      const auto gen = WEBO_GL_ENTRY(GenQueries, PFNGLGENQUERIESPROC);
      deleteQueries_ = WEBO_GL_ENTRY(DeleteQueries, PFNGLDELETEQUERIESPROC);
      beginQuery_ = WEBO_GL_ENTRY(BeginQuery, PFNGLBEGINQUERYPROC);
      endQuery_ = WEBO_GL_ENTRY(EndQuery, PFNGLENDQUERYPROC);
      queryState_ = WEBO_GL_ENTRY(GetQueryObjectuiv, PFNGLGETQUERYOBJECTUIVPROC);
      queryTime_ = WEBO_GL_ENTRY(GetQueryObjectui64v, PFNGLGETQUERYOBJECTUI64VPROC);
      if (!gen || !deleteQueries_ || !beginQuery_ || !endQuery_ || !queryState_ || !queryTime_) return;
      gen(timerQueries_.size(), timerQueries_.data());
    }
    if (!timerQueries_[0]) return;
    if (pendingTimers_) {
      GLuint ready = GL_FALSE;
      queryState_(timerQueries_[timerHead_], GL_QUERY_RESULT_AVAILABLE, &ready);
      if (ready) {
        GLuint64 elapsed = 0;
        queryTime_(timerQueries_[timerHead_], GL_QUERY_RESULT, &elapsed);
        profile::active->gpuFrame = timerFrames_[timerHead_];
        profile::active->gpuNanoseconds = elapsed;
        timerHead_ = (timerHead_ + 1) % timerQueries_.size();
        --pendingTimers_;
      }
    }
    if (pendingTimers_ == timerQueries_.size()) return;
    const auto tail = (timerHead_ + pendingTimers_) % timerQueries_.size();
    beginQuery_(GL_TIME_ELAPSED, timerQueries_[tail]);
    timerFrames_[tail] = frame;
    ++pendingTimers_;
    timerActive_ = true;
  }

  void endLevelGpuProfile() override {
    if (!timerActive_) return;
    profile::Scope scope(profile::GlQuery);
    endQuery_(GL_TIME_ELAPSED);
    timerActive_ = false;
  }

  std::expected<void, std::string> composeGl(const GlSceneImage& scene, const PreparedPage& page,
                                                const PreparedLevelLayers& layers) override {
    if (!current()) return std::unexpected("OpenGL composition requires its current SDL context");
    profile::Scope scope(profile::GlCompose);
    return compositor_->compose(scene, page, layers, scene.depthScale != 0 || (!scene.colour && !exactPages_));
  }

  std::expected<void, std::string> readGl(Image& image) override {
    if (!current()) return std::unexpected("OpenGL readback requires its current SDL context");
    profile::Scope scope(profile::GlReadback);
    return compositor_->read(image);
  }

  std::expected<void, std::string> showGl() override {
    if (!current()) return std::unexpected("OpenGL presentation requires its current SDL context");
    int width = 0, height = 0;
    SDL_GL_GetDrawableSize(window_, &width, &height);
    if (width > 0 && height > 0) {
      { profile::Scope scope(profile::GlWindowCopy);
        if (auto copied = compositor_->copyToWindow(width, height); !copied) return copied;
      }
      endLevelGpuProfile();
      { profile::Scope scope(profile::GlSwap); SDL_GL_SwapWindow(window_); }
    }
    return {};
  }

  void releaseLayerAssets() override {
    if (!current()) return;
    releaseLevelGpuProfile();
    compositor_->releaseAssets();
  }

  std::expected<void, std::string> present(const Image& image, Image* readback) override {
    if (!current()) return std::unexpected("OpenGL presentation requires its current SDL context");
    if (image.width != kGameWidth || image.height != kGameHeight ||
        image.rgba.size() != std::size_t(kGameWidth) * kGameHeight * 4)
      return std::unexpected("OpenGL presentation requires a tightly packed 640x480 RGBA image");

    if (!program_) {
      const auto linked = linkGlProgram(gl_,
          presentationShader(ShaderDialect::DesktopCore, kPresentationVertex),
          presentationShader(ShaderDialect::DesktopCore, kPresentationFragment));
      if (!linked) return std::unexpected(linked.error());
      program_ = *linked;
      gl_.GenVertexArrays(1, &vao_);
      if (!vao_) return std::unexpected("OpenGL did not allocate the upload presenter VAO");
      // The only program sampler defaults to texture unit zero.
    }
    gl_.BindFramebuffer(GL_FRAMEBUFFER, 0);
    gl_.UseProgram(program_);
    gl_.BindVertexArray(vao_);
    gl_.ActiveTexture(GL_TEXTURE0);

    {
      profile::Scope scope(profile::Upload);
      if (!source_) allocateTexture(source_);
      gl_.BindTexture(GL_TEXTURE_2D, source_);
      // Keep straight alpha, and upload top-down. The vertex shader maps row 0 to the top.
      gl_.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kGameWidth, kGameHeight,
                        GL_RGBA, GL_UNSIGNED_BYTE, image.rgba.data());
      profile::glUpload(image.rgba.size());
    }
    if (readback) {
      profile::Scope scope(profile::GlReadback);
      const auto start = profile::Clock::now();
      if (!target_) {
        if (auto ready = allocateComparison(); !ready) return ready;
      }
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
      { profile::Scope scope(profile::GlWindowCopy); draw(width, height); }
      { profile::Scope scope(profile::GlSwap); SDL_GL_SwapWindow(window_); }
    }
    return glStatus("OpenGL presentation");
  }

 private:
  std::array<GLuint, 16> timerQueries_{};
  std::array<int, 16> timerFrames_{};
  std::size_t timerHead_ = 0, pendingTimers_ = 0;
  bool timerInitialized_ = false, timerActive_ = false;
  PFNGLDELETEQUERIESPROC deleteQueries_ = nullptr;
  PFNGLBEGINQUERYPROC beginQuery_ = nullptr;
  PFNGLENDQUERYPROC endQuery_ = nullptr;
  PFNGLGETQUERYOBJECTUIVPROC queryState_ = nullptr;
  PFNGLGETQUERYOBJECTUI64VPROC queryTime_ = nullptr;

  void releaseLevelGpuProfile() {
    endLevelGpuProfile();
    // Level frame numbers restart on Retry. Discard the previous leg's
    // in-flight queries without asking the GPU for their results.
    if (timerQueries_[0]) {
      deleteQueries_(timerQueries_.size(), timerQueries_.data());
      timerQueries_.fill(0);
    }
    timerHead_ = pendingTimers_ = 0;
    timerInitialized_ = false;
  }

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

  std::expected<void, std::string> allocateComparison() {
    allocateTexture(targetColour_);
    gl_.GenFramebuffers(1, &target_);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, target_);
    gl_.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetColour_, 0);
    auto complete = framebufferStatus("comparison");
    if (complete) gl_.ReadBuffer(GL_COLOR_ATTACHMENT0);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, 0);
    if (!complete) {
      gl_.DeleteFramebuffers(1, &target_);
      gl_.DeleteTextures(1, &targetColour_);
      target_ = targetColour_ = 0;
    }
    // allocateTexture binds the destination. Subsequent draws sample the source.
    gl_.BindTexture(GL_TEXTURE_2D, source_);
    return complete;
  }

  void draw(int width, int height) const {
    gl_.Viewport(0, 0, width, height);
    gl_.DrawArrays(GL_TRIANGLES, 0, 3);
    profile::glDraw();
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
  std::optional<GlProgramCache> programs_;
  std::optional<GlCompositor> compositor_;
  bool exactPages_ = false;
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
