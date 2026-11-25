#pragma once

#include "app/CompositorShaders.h"
#include "app/FrameProfile.h"
#include "app/NativeGlApi.h"
#include "app/PreparedLevelLayers.h"
#include "app/PresentationBackend.h"
#include "app/PresentationShaders.h"

#include <bit>
#include <cmath>
#include <format>
#include <limits>
#include <map>

namespace webo::app {

class GlCompositor {
 public:
  explicit GlCompositor(const GlFunctions& gl) : gl_(gl) {}
  ~GlCompositor() {
    if (!context_.ownsCurrent()) return;
    releaseAssets();
    for (const auto& [screen, page] : pages_) gl_.DeleteTextures(1, &page.texture);
    if (target_) gl_.DeleteFramebuffers(1, &target_);
    if (vao_) gl_.DeleteVertexArrays(1, &vao_);
    if (program_) gl_.DeleteProgram(program_);
#ifndef __EMSCRIPTEN__
    if (hardwareProgram_) gl_.DeleteProgram(hardwareProgram_);
#endif
    if (colour_) gl_.DeleteTextures(1, &colour_);
    if (scratch_) gl_.DeleteTextures(1, &scratch_);
    if (iconTaps_) gl_.DeleteTextures(1, &iconTaps_);
    if (hover_) gl_.DeleteTextures(1, &hover_);
    if (highlightOwners_) gl_.DeleteTextures(1, &highlightOwners_);
#ifdef __EMSCRIPTEN__
    gl_.DeleteTextures(1, &emptyColour_);
    gl_.DeleteTextures(1, &emptyInteger_);
#endif
  }

  std::expected<void, std::string> start() {
    location_ = WEBO_GL_ENTRY(GetUniformLocation, decltype(location_));
    one_ = WEBO_GL_ENTRY(Uniform1i, decltype(one_));
    four_ = WEBO_GL_ENTRY(Uniform4i, decltype(four_));
    depthWords_ = WEBO_GL_ENTRY(Uniform2ui, decltype(depthWords_));
    copy_ = WEBO_GL_ENTRY(CopyTexSubImage2D, decltype(copy_));
    blit_ = WEBO_GL_ENTRY(BlitFramebuffer, decltype(blit_));
#ifndef __EMSCRIPTEN__
    enable_ = WEBO_GL_ENTRY(Enable, decltype(enable_));
    blend_ = WEBO_GL_ENTRY(BlendFuncSeparate, decltype(blend_));
    equation_ = WEBO_GL_ENTRY(BlendEquationSeparate, decltype(equation_));
    if (!enable_ || !blend_ || !equation_)
      return std::unexpected("OpenGL compositor blending is unavailable");
#endif
    if (!location_ || !one_ || !four_ || !depthWords_ || !copy_ || !blit_)
      return std::unexpected("OpenGL compositor entry point is unavailable");
#ifdef __EMSCRIPTEN__
    if (auto linked = selectProgram(false); !linked) return linked;
#endif
    gl_.GenFramebuffers(1, &target_);
    gl_.GenVertexArrays(1, &vao_);
    for (auto* handle : {&colour_, &scratch_, &iconTaps_, &hover_, &highlightOwners_}) *handle = newTexture();
#ifdef __EMSCRIPTEN__
    emptyColour_ = newTexture();
    gl_.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    emptyInteger_ = newTexture();
    gl_.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, 1, 1, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, nullptr);
    bind(3, iconTaps_);
    gl_.TexImage2D(GL_TEXTURE_2D, 0, GL_RG32I, 1, 1, 0, GL_RG_INTEGER, GL_INT, nullptr);
    bind(5, highlightOwners_);
    gl_.TexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 1, 1, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    gl_.ActiveTexture(GL_TEXTURE0);
#endif
    return status("initialize GL compositor");
  }

  std::expected<void, std::string> allocateTarget(int width, int height) {
    if (width == width_ && height == height_) return {};
    gl_.BindFramebuffer(GL_FRAMEBUFFER, target_);
    for (const auto texture : {colour_, scratch_}) {
      gl_.BindTexture(GL_TEXTURE_2D, texture);
      gl_.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    gl_.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colour_, 0);
#ifndef __EMSCRIPTEN__
    targetColour_ = colour_;
#endif
    if (gl_.CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      return std::unexpected("GL compositor logical RGBA8 framebuffer incomplete");
    width_ = width;
    height_ = height;
    return {};
  }

  void releaseAssets() {
    for (const auto& [image, texture] : assets_) gl_.DeleteTextures(1, &texture);
    assets_.clear();
#ifndef __EMSCRIPTEN__
    if (targetColour_ && targetColour_ != colour_) {
      gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, target_);
      gl_.FramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colour_, 0);
      gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
      targetColour_ = colour_;
    }
#endif
  }

  std::expected<void, std::string> compose(const GlSceneImage& scene, const PreparedPage& page,
                                           const PreparedLevelLayers& layers, bool hardware = false) {
    if (scene.width <= 0 || scene.height <= 0 || (!scene.colour && !page.image))
      return std::unexpected("OpenGL composition requires a viewport and a scene or page");
    if (page.image && (page.image->width != scene.width || page.image->height != scene.height ||
                       page.image->rgba.size() != page.image->pixelCount() * 4))
      return std::unexpected("OpenGL page must match the logical viewport in packed RGBA8");
    if (auto linked = selectProgram(hardware); !linked) return linked;
    gl_.ActiveTexture(GL_TEXTURE0);
    if (auto allocated = allocateTarget(scene.width, scene.height); !allocated) return allocated;
    gl_.BindFramebuffer(GL_FRAMEBUFFER, target_);
    bool drawBase = true;
#ifndef __EMSCRIPTEN__
    // The native level redraws this colour attachment before its next use.
    // A borrowed diagnostic texture and a cached frontend page stay intact.
    const GLuint output = !page.image && scene.consumableColour ? scene.colour : colour_;
    if (targetColour_ != output) {
      gl_.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output, 0);
      targetColour_ = output;
    }
    drawBase = output == colour_;
    dirty_.clear();
    snapshotReady_ = false;
    previousBlend_ = -1;
#endif
    gl_.Disable(GL_DEPTH_TEST);
    gl_.Disable(GL_BLEND);
    gl_.Disable(GL_DITHER);
#ifndef __EMSCRIPTEN__
    gl_.Disable(GL_FRAMEBUFFER_SRGB);
#endif
    gl_.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    gl_.ReadBuffer(GL_COLOR_ATTACHMENT0);
    gl_.BindVertexArray(vao_);
#ifdef __EMSCRIPTEN__
    bind(1, scratch_);
    bind(2, scene.depth ? scene.depth : emptyInteger_);
    bind(4, scene.highlights ? scene.highlights : emptyInteger_);
#else
    if (!hardware_) bind(1, textureBarrier_ ? targetColour_ : scratch_);
    bind(2, scene.depth);
    bind(4, scene.highlights);
#endif
    bind(3, iconTaps_);
    bind(5, highlightOwners_);
    highlights_ = scene.highlights != 0;
    depth_ = scene.depth != 0;
    depth64_ = scene.depth64;
    depthMinimum_ = scene.depthMinimum;
    depthScale_ = scene.depthScale;
    const Rect full{0, 0, width_, height_};
    alpha_ = page.image ? page.image->alpha : AlphaMode::Opaque;
    if (page.image) {
      auto& cache = pages_[page.screen];
      if (!cache.texture || cache.image != page.image || cache.revision != page.revision) {
        gl_.ActiveTexture(GL_TEXTURE0);
        if (!cache.texture) cache.texture = newTexture();
        upload(cache.texture, *page.image);
        cache.image = page.image;
        cache.revision = page.revision;
      }
      rect(full, Replace, Point, cache.texture, full);
    } else if (scene.colour && drawBase) {
      rect(full, Replace, Scene, scene.colour);
    }
    for (const auto& layer : layers.draws) std::visit([this](const auto& item) { draw(item); }, layer);
    gl_.ActiveTexture(GL_TEXTURE0);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, 0);
    gl_.Disable(GL_BLEND);
#ifndef __EMSCRIPTEN__
    if (hardware_) equation_(GL_FUNC_ADD, GL_FUNC_ADD);
    // Ordinary play stops asking for GL errors after a warm-up: the query
    // synchronizes with a threaded driver. The exact path keeps its check.
    if (hardware_ && hardwareComposed_++ >= 16) return {};
#endif
    return status("compose OpenGL frame");
  }

  std::expected<void, std::string> read(Image& image) const {
    image = {width_, height_, alpha_, {}};
    image.rgba.resize(image.pixelCount() * 4);
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, target_);
    gl_.ReadBuffer(GL_COLOR_ATTACHMENT0);
    gl_.PixelStorei(GL_PACK_ALIGNMENT, 1);
    gl_.ReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, image.rgba.data());
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    const auto rowBytes = std::size_t(width_) * 4;
    for (int y = 0; y < height_ / 2; ++y) {
      auto row = image.rgba.begin() + y * rowBytes;
      std::swap_ranges(row, row + rowBytes, image.rgba.begin() + (height_ - 1 - y) * rowBytes);
    }
    return status("read OpenGL frame");
  }

  std::expected<void, std::string> copyToWindow(int width, int height) const {
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, target_);
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    blit_(0, 0, width_, height_, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    return status("scale OpenGL frame");
  }

 private:
  struct Rect { int left, top, width, height; };
  enum Blend { Replace, SolidRgb, IconAlpha, TruncateRgb, RoundRgb, GlyphAlpha, AddRgb, AddAlpha, Caption };
  enum Sample { Constant, Point, IconTaps, Scene, Plate, SceneHighlights };

  std::expected<void, std::string> selectProgram(bool hardware) {
    GLuint* program = &program_;
    Uniforms* uniforms = &exactUniforms_;
#ifndef __EMSCRIPTEN__
    if (hardware) {
      program = &hardwareProgram_;
      uniforms = &hardwareUniforms_;
    }
#endif
    hardware_ = hardware;
    if (!*program) {
      auto fragment = presentationShader(kSceneShaderDialect,
          hardware ? "#define HARDWARE_COMPOSITION\n" : "");
      fragment += kCompositorFragment;
      const auto linked = linkGlProgram(gl_, presentationShader(kSceneShaderDialect, kPresentationVertex), fragment);
      if (!linked) return std::unexpected(linked.error());
      *program = *linked;
      gl_.UseProgram(*program);
      const auto locate = [&](const char* name) { return location_(*program, name); };
      one_(locate("sourceImage"), 0);
      one_(locate("destinationImage"), 1);
      one_(locate("sceneDepth"), 2);
      one_(locate("iconTaps"), 3);
      one_(locate("sceneHighlights"), 4);
      one_(locate("highlightOwners"), 5);
      *uniforms = {locate("destinationRect"), locate("sourceRect"), locate("state"),
                   locate("ink"), locate("captionScale"), locate("labelDepth")};
#ifndef __EMSCRIPTEN__
      if (!hardware && SDL_GL_ExtensionSupported("GL_ARB_texture_barrier"))
        textureBarrier_ = WEBO_GL_ENTRY(TextureBarrier, PFNGLTEXTUREBARRIERPROC);
#endif
    } else {
      gl_.UseProgram(*program);
    }
    uniforms_ = *uniforms;
    return {};
  }

  std::expected<void, std::string> status(const char* operation) const {
    const auto error = gl_.GetError();
    if (error != GL_NO_ERROR) return std::unexpected(std::format("{}: GL error 0x{:x}", operation, error));
    return {};
  }

  GLuint newTexture() const {
    GLuint texture = 0;
    gl_.GenTextures(1, &texture);
    gl_.BindTexture(GL_TEXTURE_2D, texture);
    gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return texture;
  }

  void bind(int unit, GLuint texture) const {
    gl_.ActiveTexture(GL_TEXTURE0 + unit);
    gl_.BindTexture(GL_TEXTURE_2D, texture);
  }

  void upload(GLuint texture, const Image& image) const {
    gl_.BindTexture(GL_TEXTURE_2D, texture);
    gl_.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width, image.height, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, image.rgba.data());
    profile::glUpload(image.rgba.size());
  }

  GLuint texture(const Image& image) {
    gl_.ActiveTexture(GL_TEXTURE0);
    auto [it, inserted] = assets_.try_emplace(&image, 0);
    if (inserted) {
      it->second = newTexture();
      upload(it->second, image);
    }
    return it->second;
  }

  static Rect bounds(const std::array<int, 4>& box) {
    return {box[0], box[1], box[2] - box[0], box[3] - box[1]};
  }

  void rect(Rect dest, Blend blend, Sample sample = Constant, GLuint source = 0, Rect from = {0, 0, 0, 0},
            shell::Rgb rgb = {}, int alpha = 255, int scale = 255, std::optional<double> depth = {}) {
    const int left = std::max(dest.left, 0), top = std::max(dest.top, 0);
    const int right = std::min(dest.left + dest.width, width_);
    const int bottom = std::min(dest.top + dest.height, height_);
    if (left >= right || top >= bottom) return;
    if (blend != Replace && !hardware_) {
#ifndef __EMSCRIPTEN__
      // Disjoint operations can share a destination snapshot. Overlapping
      // operations still see all preceding byte-rounded writes.
      if (!snapshotReady_ || std::ranges::any_of(dirty_, [&](const Rect& written) {
            return left < written.left + written.width && right > written.left &&
                   top < written.top + written.height && bottom > written.top;
          })) {
        if (textureBarrier_) {
          textureBarrier_();
        } else {
          bind(1, scratch_);
          copy_(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width_, height_);
        }
        dirty_.clear();
        snapshotReady_ = true;
      }
#else
      // Snapshot only the rectangle this draw can read. The source and destination
      // of a draw are distinct textures even when several glyphs overlap.
      bind(1, scratch_);
      copy_(GL_TEXTURE_2D, 0, left, height_ - bottom, left, height_ - bottom, right - left, bottom - top);
#endif
    }
#ifndef __EMSCRIPTEN__
    if (hardware_ && blend != previousBlend_) {
      previousBlend_ = blend;
      if (blend == Replace) {
        gl_.Disable(GL_BLEND);
      } else {
        enable_(GL_BLEND);
        equation_(GL_FUNC_ADD, blend == IconAlpha ? GL_MAX : GL_FUNC_ADD);
        if (blend == SolidRgb) blend_(GL_ONE, GL_ZERO, GL_ZERO, GL_ONE);
        else if (blend == AddRgb || blend == AddAlpha) blend_(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
        else blend_(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
      }
    } else if (!hardware_) {
      dirty_.push_back({left, top, right - left, bottom - top});
    }
#endif
#ifdef __EMSCRIPTEN__
    if (!source) source = emptyColour_;
#endif
    bind(0, source);
    gl_.Viewport(left, height_ - bottom, right - left, bottom - top);
    four_(uniforms_.dest, dest.left, dest.top, dest.width, dest.height);
    four_(uniforms_.source, from.left, from.top, from.width, from.height);
    const int depthMode = depth_ && depth ? (depth64_ ? 2 : 1) : 0;
    four_(uniforms_.state, blend, sample, height_, depthMode);
    four_(uniforms_.ink, rgb[0], rgb[1], rgb[2], alpha);
    one_(uniforms_.caption, scale);
    if (depthMode) {
      const double value = hardware_ ? (*depth - depthMinimum_) * depthScale_ : *depth;
      auto stored = static_cast<float>(value);
      if (stored > value) stored = std::nextafter(stored, -std::numeric_limits<float>::infinity());
      const auto bits = depth64_ ? std::bit_cast<std::uint64_t>(*depth) : std::bit_cast<std::uint32_t>(stored);
      depthWords_(uniforms_.depth, GLuint(bits), GLuint(bits >> 32));
    }
    gl_.DrawArrays(GL_TRIANGLES, 0, 3);
    profile::glDraw();
  }

  void draw(const shell::ScreenRect& draw) {
    rect(bounds(draw.bounds), draw.opaqueAlpha ? Replace : SolidRgb, Constant, 0, {}, draw.colour);
  }

  void draw(const PreparedIconRegion& draw) {
    const auto& p = draw.placement;
    rect({p.destX, p.destY, p.width, p.height}, IconAlpha, Point, texture(*draw.texture),
         {p.srcX, p.srcY, p.width, p.height});
  }

  void glyphs(const Image& font, std::span<const shell::HudGlyph> glyphs) {
    if (glyphs.empty()) return;
    const auto sheet = texture(font);
    for (const auto& g : glyphs)
      rect({g.x, g.y, g.source.width, g.height}, GlyphAlpha, Point, sheet,
           {g.source.x, g.source.y, g.source.width, g.height}, g.colour, 255, 255, g.comparisonDepth);
  }

  void draw(const PreparedLevelLayers::Text& draw) { glyphs(*draw.font, draw.glyphs); }

  void draw(const PreparedSkillIcon& draw) {
    const auto& f = draw.frame;
    if (!f.width || !f.height) return;
    const auto& r = draw.rect;
    const auto& sheet = *draw.texture;
    const int left = int(std::ceil(r.left)), top = int(std::ceil(r.top));
    const int width = int(std::ceil(r.right)) - left, height = int(std::ceil(r.bottom)) - top;
    if (width <= 0 || height <= 0) return;
    // The SPR UV endpoints are binary64. Preserve the CPU floor at texel
    // boundaries with independent X/Y taps, leaving sampling and blend on GL.
    taps_.assign(std::max(width, height), {});
    for (int x = 0; x < width; ++x) {
      const double u = (left + x - r.left) / (r.right - r.left);
      taps_[x][0] = std::clamp(int(std::floor((f.uLeft + u * (f.uRight - f.uLeft)) * sheet.width)),
                              int(f.left), int(f.right) - 1);
    }
    for (int y = 0; y < height; ++y) {
      const double v = (top + y - r.top) / (r.bottom - r.top);
      taps_[y][1] = std::clamp(int(std::floor((f.uTop + v * (f.uBottom - f.uTop)) * sheet.height)),
                              int(f.top), int(f.bottom) - 1);
    }
    bind(3, iconTaps_);
    gl_.TexImage2D(GL_TEXTURE_2D, 0, GL_RG32I, int(taps_.size()), 1, 0, GL_RG_INTEGER, GL_INT, taps_.data());
    profile::glUpload(taps_.size() * sizeof(taps_[0]));
    rect({left, top, width, height}, sheet.alpha == AlphaMode::Additive ? AddRgb : RoundRgb,
         IconTaps, texture(sheet));
  }

  void draw(const PreparedSkillPanel& panel) {
    for (const auto& p : panel.plates)
      rect({p.left, p.top, p.width, p.height}, TruncateRgb, Plate, texture(*p.texture),
           {0, 0, p.texture->width, p.texture->height});
    glyphs(*panel.font, panel.digits);
    for (const auto& icon : panel.icons) draw(icon);
  }

  void draw(const PreparedEntryCue& cue) {
    if (cue.glyphs.empty()) return;
    const auto font = texture(*cue.font);
    for (const auto& g : cue.glyphs)
      rect(bounds(g.destination), cue.additive ? AddAlpha : RoundRgb, Point, font,
           {g.source.x, g.source.y, g.source.width, shell::kLineHeight});
  }

  void draw(const shell::PreparedMenuRows& menu) {
    for (const auto& row : menu.rows) {
      draw(row.border);
      draw(row.fill);
      glyphs(*menu.font, row.glyphs);
    }
  }

  void draw(const SoundMenu::Prepared& menu) {
    for (const auto& row : menu.rows) {
      draw(row.label);
      for (const auto& arrow : row.arrows) draw(arrow);
      if (row.meter) for (const auto& bar : *row.meter) draw(bar);
    }
  }

  void draw(const shell::CaptionParams& panel) {
    const auto& r = panel.rect;
    const int scale = std::max(1, panel.alphaScale);
    rect({r.x0, r.y0, r.x1 - r.x0, r.y1 - r.y0}, Caption, Constant, 0, {}, panel.fill,
         std::clamp(panel.alpha, 0, scale), scale);
  }

  void draw(const Image* image) {
    const Rect full{0, 0, image->width, image->height};
    rect(full, Replace, Point, texture(*image), full);
    alpha_ = image->alpha;
  }

  void draw(const std::vector<HoverFragment>& fragments) {
    if (fragments.empty()) return;
    int left = width_, top = height_, right = 0, bottom = 0;
    for (const auto& fragment : fragments) {
      const int x = int(fragment.pixel % width_), y = int(fragment.pixel / width_);
      left = std::min(left, x); top = std::min(top, y);
      right = std::max(right, x + 1); bottom = std::max(bottom, y + 1);
    }
    const int width = right - left, height = bottom - top;
    hoverBytes_.assign(std::size_t(width) * height * 4, 0);
    for (const auto& fragment : fragments) {
      const int x = int(fragment.pixel % width_) - left, y = int(fragment.pixel / width_) - top;
      addHoverRgb(hoverBytes_.data() + (std::size_t(y) * width + x) * 4, fragment.rgb.data());
    }
    bind(0, hover_);
    gl_.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, hoverBytes_.data());
    profile::glUpload(hoverBytes_.size());
    rect({left, top, width, height}, AddRgb, Point, hover_, {0, 0, width, height});
  }

  void draw(const std::vector<SceneHighlightId>& owners) {
    if (!highlights_ || owners.empty()) return;
    highlightIds_.clear();
    for (const auto owner : owners) highlightIds_.push_back(packSceneHighlight(owner));
    bind(5, highlightOwners_);
    gl_.TexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, int(highlightIds_.size()), 1, 0,
                   GL_RED_INTEGER, GL_UNSIGNED_INT, highlightIds_.data());
    rect({0, 0, width_, height_}, AddRgb, SceneHighlights);
  }

  const GlFunctions& gl_;
  CurrentGlContext context_;
  GLuint target_ = 0, vao_ = 0, program_ = 0, colour_ = 0, scratch_ = 0, iconTaps_ = 0, hover_ = 0;
  GLuint highlightOwners_ = 0;
#ifdef __EMSCRIPTEN__
  GLuint emptyColour_ = 0, emptyInteger_ = 0;
#endif
  int width_ = 0, height_ = 0;
  bool depth_ = false, depth64_ = false, highlights_ = false, hardware_ = false;
  float depthMinimum_ = 0, depthScale_ = 0;
  AlphaMode alpha_ = AlphaMode::Opaque;
  struct Page { const Image* image = nullptr; std::uint64_t revision = 0; GLuint texture = 0; };
  std::map<shell::Screen, Page> pages_;
  std::map<const Image*, GLuint> assets_;
  std::vector<std::array<int, 2>> taps_;
  std::vector<std::uint8_t> hoverBytes_;
  std::vector<std::uint32_t> highlightIds_;
  struct Uniforms { GLint dest, source, state, ink, caption, depth; };
  Uniforms uniforms_{}, exactUniforms_{};
  PFNGLGETUNIFORMLOCATIONPROC location_ = nullptr;
  PFNGLUNIFORM1IPROC one_ = nullptr;
  PFNGLUNIFORM4IPROC four_ = nullptr;
  PFNGLUNIFORM2UIPROC depthWords_ = nullptr;
  decltype(&::glCopyTexSubImage2D) copy_ = nullptr;
  PFNGLBLITFRAMEBUFFERPROC blit_ = nullptr;
#ifndef __EMSCRIPTEN__
  GLuint hardwareProgram_ = 0;
  unsigned hardwareComposed_ = 0;
  Uniforms hardwareUniforms_{};
  decltype(&::glEnable) enable_ = nullptr;
  PFNGLBLENDFUNCSEPARATEPROC blend_ = nullptr;
  PFNGLBLENDEQUATIONSEPARATEPROC equation_ = nullptr;
  int previousBlend_ = -1;
  GLuint targetColour_ = 0;
  bool snapshotReady_ = false;
  std::vector<Rect> dirty_;
  PFNGLTEXTUREBARRIERPROC textureBarrier_ = nullptr;
#endif
};

}  // namespace webo::app
