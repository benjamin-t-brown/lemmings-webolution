#define SDL_MAIN_HANDLED
#include "app/HardwareLevelScene.h"

#include "app/ActorOcclusion.h"
#include "app/HardwareSceneShaders.h"
#include "app/NativeGlApi.h"
#include "app/OpaqueSceneComparison.h"
#include "app/PresentationShaders.h"

#include <bit>
#include <cstddef>
#include <format>
#include <map>

namespace webo::app {
namespace {

#define HARDWARE_GL_FUNCTIONS(X) \
  X(Enable, decltype(&::glEnable)) \
  X(Disablei, PFNGLDISABLEIPROC) \
  X(BlendFuncSeparate, PFNGLBLENDFUNCSEPARATEPROC) \
  X(DepthMask, decltype(&::glDepthMask)) \
  X(DepthFunc, decltype(&::glDepthFunc)) \
  X(Scissor, decltype(&::glScissor)) \
  X(DrawBuffers, PFNGLDRAWBUFFERSPROC) \
  X(ClearBufferfv, PFNGLCLEARBUFFERFVPROC) \
  X(BlitFramebuffer, PFNGLBLITFRAMEBUFFERPROC) \
  X(GenBuffers, PFNGLGENBUFFERSPROC) \
  X(DeleteBuffers, PFNGLDELETEBUFFERSPROC) \
  X(BindBuffer, PFNGLBINDBUFFERPROC) \
  X(BufferData, PFNGLBUFFERDATAPROC) \
  X(EnableVertexAttribArray, PFNGLENABLEVERTEXATTRIBARRAYPROC) \
  X(VertexAttribPointer, PFNGLVERTEXATTRIBPOINTERPROC) \
  X(GenSamplers, PFNGLGENSAMPLERSPROC) \
  X(DeleteSamplers, PFNGLDELETESAMPLERSPROC) \
  X(SamplerParameteri, PFNGLSAMPLERPARAMETERIPROC) \
  X(BindSampler, PFNGLBINDSAMPLERPROC) \
  X(GetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC) \
  X(Uniform1i, PFNGLUNIFORM1IPROC) \
  X(Uniform1f, PFNGLUNIFORM1FPROC) \
  X(Uniform2f, PFNGLUNIFORM2FPROC) \
  X(Uniform3f, PFNGLUNIFORM3FPROC) \
  X(Uniform4f, PFNGLUNIFORM4FPROC) \
  X(Uniform4i, PFNGLUNIFORM4IPROC)

struct HardwareGl : GlFunctions {
#define DECLARE(name, type) type name = nullptr;
  HARDWARE_GL_FUNCTIONS(DECLARE)
#undef DECLARE

  std::expected<void, std::string> load() {
    if (auto loaded = GlFunctions::load(); !loaded) return loaded;
#define LOAD(name, type) \
    name = WEBO_GL_ENTRY(name, type); \
    if (!name) return std::unexpected("OpenGL entry point missing: gl" #name);
    HARDWARE_GL_FUNCTIONS(LOAD)
#undef LOAD
    return {};
  }
};
#undef HARDWARE_GL_FUNCTIONS

using Pass = LevelScenePass;
constexpr std::array kScenePasses{Pass::Primary, Pass::LegacyUnattributed, Pass::Tube,
    Pass::GadgetEffects, Pass::Pipes, Pass::Fluid, Pass::Secondary, Pass::CounterGates,
    Pass::ExplosionMarks, Pass::TeleportEffects};

struct Vertex {
  std::array<float, 4> position;
  std::array<float, 2> uv;
  std::array<float, 4> tint;
};

struct Material {
  GLuint texture;
  unsigned sampler;
  SceneDepthFunction depthFunction;
  bool depthWrite, pipeWrite, selectTextureAlpha;
  int alphaReference;
  GLenum sourceBlend, destinationBlend;
  bool blend;
  bool operator==(const Material&) const = default;
};

struct Batch {
  Material material;
  GLint first;
  GLsizei count;
};

struct Geometry {
  GLuint buffer = 0, vao = 0;
  std::vector<Batch> batches;
  std::shared_ptr<const void> key;
  float minimum = std::numeric_limits<float>::infinity();
  float maximum = -minimum;

  void include(float z) {
    minimum = std::min(minimum, z);
    maximum = std::max(maximum, z);
  }
};

class HardwareScene final : public HardwareLevelScene {
 public:
  explicit HardwareScene(GlProgramCache& programs) : programs_(programs) {}

  ~HardwareScene() override {
    if (!context_.ownsCurrent()) return;
    for (const auto& mesh : passes_) release(mesh);
    release(effect_);
    for (const auto& [key, name] : textures_) gl_.DeleteTextures(1, &name);
    gl_.DeleteTextures(images_.size(), images_.data());
    gl_.DeleteSamplers(samplers_.size(), samplers_.data());
    gl_.DeleteFramebuffers(targets_.size(), targets_.data());
    if (emptyVao_) gl_.DeleteVertexArrays(1, &emptyVao_);
  }

  std::expected<void, std::string> start() {
    if (!context_.current()) return std::unexpected("hardware scene requires a current context");
    if (auto loaded = gl_.load(); !loaded) return loaded;
    auto mesh = link(kHardwareMeshVertex, kHardwareMeshFragment);
    if (!mesh) return std::unexpected(mesh.error());
    meshProgram_ = *mesh;
    gl_.UseProgram(meshProgram_);
    const auto m = [&](const char* name) { return location(meshProgram_, name); };
    mesh_ = {m("viewportScale"), m("depthMap"), m("checkOcclusion"), m("selectTextureAlpha"),
             m("alphaReference")};
    gl_.Uniform1i(m("surfaceImage"), 0);
    gl_.Uniform1i(m("occlusionDepth"), 1);

    gl_.GenTextures(images_.size(), images_.data());
    gl_.GenFramebuffers(targets_.size(), targets_.data());
    gl_.GenVertexArrays(1, &emptyVao_);
    gl_.GenSamplers(samplers_.size(), samplers_.data());
    for (unsigned i = 0; i < samplers_.size(); ++i) {
      const auto filter = i & 1 ? GL_LINEAR : GL_NEAREST;
      const auto address = i & 2 ? GL_REPEAT : GL_CLAMP_TO_EDGE;
      gl_.SamplerParameteri(samplers_[i], GL_TEXTURE_MIN_FILTER, filter);
      gl_.SamplerParameteri(samplers_[i], GL_TEXTURE_MAG_FILTER, filter);
      gl_.SamplerParameteri(samplers_[i], GL_TEXTURE_WRAP_S, address);
      gl_.SamplerParameteri(samplers_[i], GL_TEXTURE_WRAP_T, address);
    }
    gl_.ActiveTexture(GL_TEXTURE0);
    constexpr std::array<std::uint8_t, 4> white{255, 255, 255, 255};
    textureImage(images_[White], 1, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, white.data());
    return status("initialize hardware scene");
  }

  GlSceneImage image() const override {
    return {images_[colour_], images_[SceneDepth], width_, height_, false, 0, true,
            minimum_, scale_};
  }

  std::expected<void, std::string> draw(const PreparedLevelScene& frame) override {
    if (!context_.current()) return std::unexpected("hardware scene lost its OpenGL context");
    if (!frame.tube || std::ranges::find(frame.passes, Pass::Tube) == frame.passes.end())
      return std::unexpected("hardware scene requires the ordinary tube pass");
    struct Restore {
      HardwareScene& scene;
      ~Restore() { scene.end(); }
    } restore{*this};
    gl_.ActiveTexture(GL_TEXTURE0);
    gl_.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    if (frame.camera.width != width_ || frame.camera.height != height_) {
      if (auto allocated = resize(frame.camera.width, frame.camera.height); !allocated) return allocated;
    }
    colour_ = SceneColour;
    prepare(frame);
    const bool hasActors = !frame.actors.draws.empty() &&
        std::ranges::find(frame.passes, Pass::Actors) != frame.passes.end();
    const bool hasParticles = !frame.particles.draws.empty();
    if (hasActors || hasParticles) {
      if (!spriteProgram_)
        if (auto linked = startSprites(); !linked) return linked;
      if (auto effects = prepareEffects(frame.actors); !effects) return effects;
    }
    gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Scene]);
    attachColour();
    viewport();
    gl_.Enable(GL_DEPTH_TEST);
    gl_.Disable(GL_BLEND);
    gl_.DepthMask(GL_TRUE);
    gl_.DrawBuffers(2, kColourAndPipe.data());
    gl_.ClearBufferfv(GL_COLOR, 0, kBlack.data());
    gl_.ClearBufferfv(GL_COLOR, 1, kClear.data());
    gl_.ClearBufferfv(GL_DEPTH, 0, kClear.data());
    const bool fluid = frame.hasFluidDrowning && !frame.fluid.draws.empty();
    for (const auto pass : frame.passes) {
      switch (pass) {
        case Pass::BlueSky:
          if (auto sky = fillSky(); !sky) return sky;
          break;
        case Pass::Actors:
          if (hasActors) drawActors(frame.camera, frame.actors, fluid);
          break;
        case Pass::Particles:
          if (hasParticles) drawParticles(frame.camera, frame.particles);
          break;
        case Pass::TeleportEffects:
          if (!geometry(pass).batches.empty()) {
            gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Scene]);
            attachColour();
            drawGeometry(geometry(pass), actorDepthReady_ ? images_[ActorDepthImage] : 0);
          }
          break;
        case Pass::Fluid:
          if (fluid) {
            if (auto ready = prepareFluidDepth(); !ready) return ready;
            snapshotDepth(BeforeFluid);
          }
          drawGeometry(geometry(pass));
          if (fluid) snapshotDepth(AfterFluid);
          break;
        case Pass::CompleteDrowningDepth:
          // The sprite samples pre-fluid depth only where the scene's last
          // depth write still belongs to the fluid pass.
          break;
        default:
          drawGeometry(geometry(pass), 0, pass == Pass::Pipes);
          break;
      }
    }
    return status("draw hardware scene");
  }

 private:
  enum ImageSlot {
    SceneColour, SceneDepth, Pipe, ActorDepthImage, CarriedColour, CarriedDepth, Before, After,
    SkyColour, White, Rings, Collision, StreamAtlas, ProbeCoverage, ProbeVisible, ImageCount
  };
  enum Target { Scene, Actors, Brick, BeforeFluid, AfterFluid, Sky, Probe, Visibility, TargetCount };
  enum class SpriteMode { Body, Drowning, Bubble, Origin, Axe, DrowningProbe };
  static constexpr std::array<GLenum, 2> kColourAndPipe{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  static constexpr GLenum kColourOnly = GL_COLOR_ATTACHMENT0;
  static constexpr std::array<float, 4> kBlack{0, 0, 0, 1};
  static constexpr std::array<float, 4> kClear{1, 1, 1, 1};

  std::expected<GLuint, std::string> link(std::string_view vs, std::string_view fs) {
    return programs_.link(presentationShader(kSceneShaderDialect, vs),
                          presentationShader(kSceneShaderDialect, fs));
  }

  GLint location(GLuint program, const char* name) const { return gl_.GetUniformLocation(program, name); }

  // glGetError synchronizes with a threaded driver. Ordinary play checks it
  // while resources are created and for a short warm-up, then stops.
  std::expected<void, std::string> status(const char* message) const {
    if (checked_ >= kCheckedFrames) return {};
    ++checked_;
    const GLenum error = gl_.GetError();
    if (error != GL_NO_ERROR) return std::unexpected(std::format("{} (GL 0x{:x})", message, error));
    return {};
  }

  void viewport() const { gl_.Viewport(0, 0, width_, height_); }

  void bind(int unit, GLuint image) const {
    gl_.ActiveTexture(GL_TEXTURE0 + unit);
    gl_.BindTexture(GL_TEXTURE_2D, image);
  }

  void textureImage(GLuint name, int width, int height, GLint internal, GLenum layout,
                    GLenum type, const void* data = nullptr) const {
    gl_.BindTexture(GL_TEXTURE_2D, name);
    gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl_.TexImage2D(GL_TEXTURE_2D, 0, internal, width, height, 0, layout, type, data);
  }

  GLuint texture(const void* key, int width, int height, bool rgba, const std::uint8_t* bytes) {
    auto [entry, created] = textures_.try_emplace(key, 0);
    if (created) {
      gl_.GenTextures(1, &entry->second);
      textureImage(entry->second, width, height, rgba ? GL_RGBA8 : GL_RGB8,
                   rgba ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, bytes);
      profile::glUpload(std::size_t(width) * height * (rgba ? 4 : 3));
    }
    return entry->second;
  }

  void attach(GLenum attachment, ImageSlot image) const {
    gl_.FramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, images_[image], 0);
  }

  void attachColour() const { attach(GL_COLOR_ATTACHMENT0, colour_); }

  bool complete() const { return gl_.CheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE; }

  void allocate(ImageSlot image, GLint internal, GLenum layout, GLenum type) const {
    textureImage(images_[image], width_, height_, internal, layout, type);
  }

  void allocateDepth(ImageSlot image) const {
    allocate(image, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
  }

  std::expected<void, std::string> resize(int width, int height) {
    width_ = width;
    height_ = height;
    fluidDepthReady_ = skyReady_ = brickReady_ = actorsReady_ = probeReady_ = false;
    allocate(SceneColour, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    allocateDepth(SceneDepth);
    allocate(Pipe, GL_R32F, GL_RED, GL_FLOAT);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Scene]);
    attach(GL_COLOR_ATTACHMENT0, SceneColour);
    attach(GL_COLOR_ATTACHMENT1, Pipe);
    attach(GL_DEPTH_ATTACHMENT, SceneDepth);
    if (!complete()) return std::unexpected("hardware scenery framebuffer is incomplete");
    return {};
  }

  std::expected<void, std::string> prepareFluidDepth() {
    if (fluidDepthReady_) return {};
    gl_.ActiveTexture(GL_TEXTURE0);
    for (const auto& [target, image] : {std::pair{BeforeFluid, Before}, std::pair{AfterFluid, After}}) {
      allocateDepth(image);
      gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[target]);
      attach(GL_DEPTH_ATTACHMENT, image);
      gl_.DrawBuffers(0, nullptr);
      gl_.ReadBuffer(GL_NONE);
      if (!complete()) return std::unexpected("hardware fluid depth framebuffer is incomplete");
    }
    fluidDepthReady_ = true;
    gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Scene]);
    return {};
  }

  void snapshotDepth(Target target) const {
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, targets_[target]);
    gl_.BlitFramebuffer(0, 0, width_, height_, 0, 0, width_, height_, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, targets_[Scene]);
  }

  Geometry& geometry(Pass pass) { return passes_[std::size_t(pass)]; }

  void append(Geometry& geometry, const OpaqueDraw& draw, bool pipe) {
    const auto& state = draw.material;
    const auto& v = draw.vertices;
    if (std::ranges::any_of(v, [](const SceneRasterVertex& corner) { return !(corner.rhw > 0.0); }))
      return;
    GLuint image = images_[White];
    if (draw.tube) {
      const auto* tile = draw.tube->texture;
      image = texture(tile, tile->width, tile->height, false, tile->rgb.data());
    } else if (const auto* surface = draw.backdrop) {
      image = texture(surface, surface->width, surface->height, true, surface->rgba.data());
    }
    constexpr std::array<GLenum, 3> factors{GL_ONE, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA};
    const Material material{image,
        unsigned(state.filter == SceneFilter::Linear) + 2u * unsigned(state.address == SceneAddress::Wrap),
        state.depthFunction, state.depthWrite, pipe && state.depthWrite && !draw.sprite,
        state.alphaOperation == SceneAlphaOperation::SelectTexture,
        state.alphaFunction == SceneAlphaFunction::StrictGreater ? int(state.alphaReference) : -1,
        factors[std::size_t(state.sourceBlend)], factors[std::size_t(state.destinationBlend)], state.blendEnabled};
    if (geometry.batches.empty() || geometry.batches.back().material != material)
      geometry.batches.push_back({material, GLint(vertices_.size()), 0});
    geometry.batches.back().count += 3;
    for (const auto& p : v) {
      const auto& tint = state.shade == SceneShade::Flat ? v.front().diffuse : p.diffuse;
      const double divisor = draw.tube ? 1.0 : 255.0;
      geometry.include(float(p.screenZ));
      vertices_.push_back({
          {float(p.screenX + (0.5 - state.sampleOffsetX)), float(p.screenY + (0.5 - state.sampleOffsetY)),
           float(p.screenZ), float(p.rhw)},
          {float(p.u), float(p.v)},
          {float(tint[0] / divisor), float(tint[1] / divisor),
           float(tint[2] / divisor), float(tint[3] / divisor)}});
    }
  }

  void append(Geometry& geometry, const PreparedSceneTriangle& t, bool pipe = false, bool sprite = false) {
    // Pipes publish only accepted mesh depth. Sprite submissions share the
    // same draw stream but do not write that auxiliary depth.
    append(geometry, {t.vertices, t.material, t.texture, nullptr, 0, 0, {}, {}}, pipe && !sprite);
  }

  void reset(Geometry& geometry) {
    geometry.batches.clear();
    geometry.minimum = std::numeric_limits<float>::infinity();
    geometry.maximum = -geometry.minimum;
    vertices_.clear();
  }

  void upload(Geometry& geometry, GLenum usage) {
    if (vertices_.empty()) return;
    if (!geometry.buffer) {
      gl_.GenBuffers(1, &geometry.buffer);
      gl_.GenVertexArrays(1, &geometry.vao);
      gl_.BindVertexArray(geometry.vao);
      gl_.BindBuffer(GL_ARRAY_BUFFER, geometry.buffer);
      gl_.VertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<const void*>(offsetof(Vertex, position)));
      gl_.VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<const void*>(offsetof(Vertex, uv)));
      gl_.VertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<const void*>(offsetof(Vertex, tint)));
      for (GLuint i = 0; i < 3; ++i) gl_.EnableVertexAttribArray(i);
    } else {
      gl_.BindBuffer(GL_ARRAY_BUFFER, geometry.buffer);
    }
    const auto bytes = vertices_.size() * sizeof(Vertex);
    gl_.BufferData(GL_ARRAY_BUFFER, bytes, vertices_.data(), usage);
    profile::glUpload(bytes);
  }

  void release(const Geometry& geometry) {
    if (geometry.vao) gl_.DeleteVertexArrays(1, &geometry.vao);
    if (geometry.buffer) gl_.DeleteBuffers(1, &geometry.buffer);
  }

  void prepare(const PreparedLevelScene& frame) {
    const auto stage = [&](Pass pass, const BackdropScene::PreparedStage& stage,
                           std::shared_ptr<const void> key = {}) {
      auto& data = geometry(pass);
      if (key && key == data.key) return;
      reset(data);
      for (const auto& draw : stage.draws)
        append(data, draw.triangle, pass == Pass::Pipes, draw.submission.has_value());
      upload(data, key ? GL_STATIC_DRAW : GL_STREAM_DRAW);
      data.key = std::move(key);
    };
    if (frame.primary) stage(Pass::Primary, *frame.primary, frame.primary);
    else {
      auto& primary = geometry(Pass::Primary);
      reset(primary);
      primary.key.reset();
    }
    auto& tube = geometry(Pass::Tube);
    if (tube.key != frame.tube) {
      reset(tube);
      for (const auto& triangle : frame.tube->triangles)
        append(tube, opaqueRasterDraw(triangle, frame.tube->depthTest), false);
      upload(tube, frame.cachePose ? GL_STATIC_DRAW : GL_STREAM_DRAW);
      tube.key = frame.tube;
    }
    stage(Pass::LegacyUnattributed, frame.legacy);
    stage(Pass::GadgetEffects, frame.gadgetEffects);
    stage(Pass::Pipes, frame.pipes);
    stage(Pass::Fluid, frame.fluid);
    stage(Pass::Secondary, frame.secondary);
    stage(Pass::TeleportEffects, frame.teleportEffects);
    const auto triangles = [&](Pass pass, std::span<const PreparedSceneTriangle> draws) {
      auto& data = geometry(pass);
      reset(data);
      for (const auto& t : draws) append(data, t);
      upload(data, GL_STREAM_DRAW);
    };
    triangles(Pass::CounterGates, frame.counterGates);
    triangles(Pass::ExplosionMarks, frame.explosionMarks);
    float nearest = 0, farthest = 1;
    for (const auto pass : kScenePasses) {
      nearest = std::min(nearest, geometry(pass).minimum);
      farthest = std::max(farthest, geometry(pass).maximum);
    }
    const float margin = std::max(16.0f, (farthest - nearest) * 0.05f);
    minimum_ = nearest - margin;
    scale_ = 0.9f / (farthest + margin - minimum_);
  }

  void blend(bool enabled, GLenum source = GL_ONE, GLenum destination = GL_ONE) const {
    if (enabled) {
      gl_.Enable(GL_BLEND);
      gl_.BlendFuncSeparate(source, destination, GL_ZERO, GL_ONE);
      gl_.Disablei(GL_BLEND, 1);
    } else {
      gl_.Disable(GL_BLEND);
    }
  }

  void drawGeometry(const Geometry& geometry, GLuint occlusion = 0, bool pipe = false) {
    if (geometry.batches.empty()) return;
    constexpr std::array<GLenum, 8> depthFunctions{
        GL_NEVER, GL_LESS, GL_LEQUAL, GL_EQUAL, GL_GEQUAL, GL_GREATER, GL_NOTEQUAL, GL_ALWAYS};
    viewport();
    gl_.UseProgram(meshProgram_);
    gl_.BindVertexArray(geometry.vao);
    gl_.Enable(GL_DEPTH_TEST);
    gl_.Uniform2f(mesh_.viewport, 2.0f / width_, -2.0f / height_);
    gl_.Uniform2f(mesh_.depth, minimum_, scale_);
    gl_.Uniform1i(mesh_.occlusion, occlusion != 0);
    bind(1, occlusion);
    gl_.ActiveTexture(GL_TEXTURE0);
    bool writingPipe = false;
    gl_.DrawBuffers(1, &kColourOnly);
    for (const auto& batch : geometry.batches) {
      const auto& m = batch.material;
      if (pipe && writingPipe != m.pipeWrite) {
        gl_.DrawBuffers(m.pipeWrite ? 2 : 1, kColourAndPipe.data());
        writingPipe = m.pipeWrite;
      }
      gl_.BindTexture(GL_TEXTURE_2D, m.texture);
      gl_.BindSampler(0, samplers_[m.sampler]);
      gl_.DepthFunc(depthFunctions[std::size_t(m.depthFunction)]);
      gl_.DepthMask(m.depthWrite);
      blend(m.blend, m.sourceBlend, m.destinationBlend);
      gl_.Uniform1i(mesh_.textureAlpha, m.selectTextureAlpha);
      gl_.Uniform1f(mesh_.alpha, m.alphaReference / 255.0f);
      gl_.DrawArrays(GL_TRIANGLES, batch.first, batch.count);
      profile::glDraw();
    }
    gl_.BindSampler(0, 0);
  }

  std::expected<void, std::string> fillSky() {
    if (!skyProgram_) {
      auto program = link(kPresentationVertex, kHardwareSkyFragment);
      if (!program) return std::unexpected(program.error());
      skyProgram_ = *program;
      gl_.UseProgram(skyProgram_);
      gl_.Uniform1i(location(skyProgram_, "sceneDepth"), 1);
    }
    gl_.ActiveTexture(GL_TEXTURE0);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Sky]);
    if (!skyReady_) {
      allocate(SkyColour, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
      attach(GL_COLOR_ATTACHMENT0, SkyColour);
      if (!complete()) return std::unexpected("hardware sky framebuffer is incomplete");
      skyReady_ = true;
    }
    gl_.Disable(GL_DEPTH_TEST);
    gl_.Disable(GL_BLEND);
    gl_.UseProgram(skyProgram_);
    gl_.BindVertexArray(emptyVao_);
    bind(0, images_[colour_]);
    bind(1, images_[SceneDepth]);
    gl_.DrawArrays(GL_TRIANGLES, 0, 3);
    profile::glDraw();
    colour_ = SkyColour;
    gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Scene]);
    attachColour();
    return {};
  }

  std::expected<void, std::string> startSprites() {
    auto program = link(kPresentationVertex, kHardwareSpriteFragment);
    if (!program) return std::unexpected(program.error());
    spriteProgram_ = *program;
    gl_.UseProgram(spriteProgram_);
    const auto s = [&](const char* name) { return location(spriteProgram_, name); };
    constexpr std::array names{"atlas", "scenery", "pipes", "beforeFluid", "afterFluid",
                               "brickDepth", "brickColour", "lattice", "collision", "visibility"};
    for (unsigned i = 0; i < names.size(); ++i) gl_.Uniform1i(s(names[i]), i);
    sprite_ = {s("spriteState"), s("frameBox"), s("placement"), s("atlasUv"), s("plane"), s("tintStrength"),
               s("depthMap"), s("originDepth"), s("height"), s("ringCount"), s("patchFluid"), s("requireVisibility"),
               s("eye"), s("rayOrigin"), s("rayDx"), s("rayDy")};
    return {};
  }

  std::expected<void, std::string> prepareEffects(const TubeScene::PreparedActors& actors) {
    gl_.ActiveTexture(GL_TEXTURE0);
    if (!actorsReady_) {
      gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Actors]);
      allocateDepth(ActorDepthImage);
      attach(GL_DEPTH_ATTACHMENT, ActorDepthImage);
      attachColour();
      if (!complete()) return std::unexpected("hardware actor framebuffer is incomplete");
      actorsReady_ = true;
    }
    if (!brickReady_ && std::ranges::any_of(actors.draws, [](const auto& actor) {
          return !actor.attachment.empty();
        })) {
      gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Brick]);
      allocate(CarriedColour, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
      allocateDepth(CarriedDepth);
      attach(GL_COLOR_ATTACHMENT0, CarriedColour);
      attach(GL_DEPTH_ATTACHMENT, CarriedDepth);
      if (!complete()) return std::unexpected("hardware attachment framebuffer is incomplete");
      brickReady_ = true;
    }
    if (!probeReady_ && std::ranges::any_of(actors.draws, needsDrowningProbe)) {
      auto program = link(kPresentationVertex, kHardwareVisibilityFragment);
      if (!program) return std::unexpected(program.error());
      visibilityProgram_ = *program;
      visibilityBounds_ = location(visibilityProgram_, "bounds");
      gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Probe]);
      allocate(ProbeCoverage, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
      attach(GL_COLOR_ATTACHMENT0, ProbeCoverage);
      if (!complete()) return std::unexpected("hardware drowning coverage framebuffer is incomplete");
      gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Visibility]);
      textureImage(images_[ProbeVisible], 1, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
      attach(GL_COLOR_ATTACHMENT0, ProbeVisible);
      if (!complete()) return std::unexpected("hardware drowning visibility framebuffer is incomplete");
      probeReady_ = true;
    }
    return {};
  }

  void startActors() {
    gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Actors]);
    if (actorDepthReady_) return;
    attachColour();
    gl_.DepthMask(GL_TRUE);
    gl_.ClearBufferfv(GL_DEPTH, 0, kClear.data());
    actorDepthReady_ = true;
  }

  void spriteInputs(const CameraBasis& camera, const std::shared_ptr<const ActorFootprint>& footprint,
                    bool patchFluid) {
    gl_.UseProgram(spriteProgram_);
    gl_.Uniform1i(sprite_.height, height_);
    gl_.Uniform1i(sprite_.patchFluid, patchFluid);
    gl_.Uniform1i(sprite_.visibility, 0);
    gl_.Uniform2f(sprite_.depth, minimum_, scale_);
    const auto vector = [&](GLint where, const auto& v) {
      gl_.Uniform3f(where, float(v[0]), float(v[1]), float(v[2]));
    };
    vector(sprite_.eye, camera.eyeWorld);
    vector(sprite_.rayOrigin, camera.worldRay(0, 0));
    const auto& m = camera.worldToView;
    vector(sprite_.rayDx, std::array{m[0] / camera.focalX, m[3] / camera.focalX, m[6] / camera.focalX});
    vector(sprite_.rayDy, std::array{-m[1] / camera.focalY, -m[4] / camera.focalY, -m[7] / camera.focalY});
    gl_.Uniform1i(sprite_.rings, footprint ? int(footprint->lattice.ringCount) : 0);
    if (footprint && footprint != footprint_) {
      const auto& lattice = footprint->lattice;
      std::vector<std::array<float, 4>> positions;
      positions.reserve(lattice.vertices.size());
      for (const auto& v : lattice.vertices)
        positions.push_back({float(v.x), float(v.y), float(v.z), 0.0f});
      gl_.ActiveTexture(GL_TEXTURE0);
      textureImage(images_[Rings], 64, int(lattice.ringCount), GL_RGBA32F, GL_RGBA, GL_FLOAT, positions.data());
      std::vector<std::array<std::uint32_t, 2>> cells;
      cells.reserve(footprint->collision.cells.size());
      for (std::size_t i = 0; i < footprint->collision.cells.size(); ++i) {
        const auto thickness = i < footprint->gateThickness.size() ? footprint->gateThickness[i] : 1.0;
        cells.push_back({footprint->collision.cells[i], std::bit_cast<std::uint32_t>(float(thickness))});
      }
      textureImage(images_[Collision], int(footprint->collision.width), int(footprint->collision.height),
                   GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, cells.data());
      profile::glUpload(positions.size() * sizeof(positions[0]) + cells.size() * sizeof(cells[0]));
      footprint_ = footprint;
    }
    bind(1, images_[SceneDepth]);
    bind(2, images_[Pipe]);
    bind(3, images_[Before]);
    bind(4, images_[After]);
    bind(5, images_[CarriedDepth]);
    bind(6, images_[CarriedColour]);
    bind(7, images_[Rings]);
    bind(8, images_[Collision]);
    bind(9, images_[ProbeVisible]);
  }

  PickBox spriteBox(const ActorSpritePlacement& bounds) const {
    const auto bound = [](double v, int limit) { return int(std::clamp(std::ceil(v), 0.0, double(limit))); };
    return {bound(bounds.left, width_), bound(bounds.top, height_),
            bound(bounds.right, width_), bound(bounds.bottom, height_)};
  }

  void drawSprite(const CameraBasis& camera, const PreparedActorSprite& sprite,
                  const ActorSpritePlacement& bounds, const std::array<double, 3>& position,
                  std::array<std::uint8_t, 3> tint, SpriteMode mode,
                  bool carried = false, double strength = 1.0, GLenum depthFunction = GL_LEQUAL) {
    const auto [left, top, right, bottom] = spriteBox(bounds);
    const auto& rect = sprite.placement;
    const auto& f = sprite.frame;
    if (left >= right || top >= bottom || !f.width || !f.height) return;
    gl_.UseProgram(spriteProgram_);
    gl_.BindVertexArray(emptyVao_);
    if (mode == SpriteMode::DrowningProbe) {
      gl_.Disable(GL_DEPTH_TEST);
    } else {
      gl_.Enable(GL_DEPTH_TEST);
      gl_.DepthMask(GL_TRUE);
      gl_.DepthFunc(depthFunction);
    }
    blend(mode == SpriteMode::Drowning || mode == SpriteMode::Bubble);
    gl_.Viewport(left, height_ - bottom, right - left, bottom - top);
    gl_.ActiveTexture(GL_TEXTURE0);
    const auto& sheet = sprite.sheet();
    if (sprite.texture) {
      gl_.BindTexture(GL_TEXTURE_2D,
          texture(sprite.texture, sheet.width, sheet.height, true, sheet.rgba.data()));
    } else {
      textureImage(images_[StreamAtlas], sheet.width, sheet.height, GL_RGBA8, GL_RGBA,
                   GL_UNSIGNED_BYTE, sheet.rgba.data());
      profile::glUpload(sheet.rgba.size());
    }
    const auto plane = ActorOcclusionPlane(camera, position).coefficients();
    gl_.Uniform4f(sprite_.plane, float(plane[0]), float(plane[1]), float(plane[2]), float(plane[3]));
    gl_.Uniform4f(sprite_.placement, float(rect.left), float(rect.top), float(rect.right), float(rect.bottom));
    gl_.Uniform4f(sprite_.uv, float(f.uLeft), float(f.uTop), float(f.uRight), float(f.uBottom));
    gl_.Uniform4i(sprite_.frame, f.left, f.top, f.right, f.bottom);
    gl_.Uniform4i(sprite_.state, sprite.mirrored, sprite.inverted, carried, int(mode));
    gl_.Uniform4f(sprite_.tintStrength, tint[0], tint[1], tint[2], float(strength));
    gl_.Uniform1f(sprite_.origin, float(actorPhysicalDepth(rect, width_)));
    gl_.DrawArrays(GL_TRIANGLES, 0, 3);
    profile::glDraw();
  }

  void drawBrick(std::span<const PreparedRasterTriangle> triangles) {
    gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Brick]);
    gl_.ActiveTexture(GL_TEXTURE0);
    gl_.DepthMask(GL_TRUE);
    gl_.ClearBufferfv(GL_DEPTH, 0, kClear.data());
    reset(effect_);
    for (const auto& t : triangles) append(effect_, opaqueRasterDraw(t, true), false);
    upload(effect_, GL_STREAM_DRAW);
    drawGeometry(effect_);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Actors]);
    bind(1, images_[SceneDepth]);
  }

  using Actor = TubeScene::PreparedActors::Actor;

  static bool needsDrowningProbe(const Actor& actor) {
    return actor.drowning && (!actor.bubbles.empty() || actor.body.worldPosition != actor.drowningPosition ||
                              !actor.attachment.empty());
  }

  bool probeDrowning(const CameraBasis& camera, const Actor& actor) {
    const auto [left, top, right, bottom] = spriteBox(actor.bounds);
    if (left >= right || top >= bottom) return false;
    if (!actor.attachment.empty()) drawBrick(actor.attachment);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Probe]);
    gl_.Enable(GL_SCISSOR_TEST);
    gl_.Scissor(left, height_ - bottom, right - left, bottom - top);
    constexpr std::array<float, 4> zero{};
    gl_.ClearBufferfv(GL_COLOR, 0, zero.data());
    gl_.Disable(GL_SCISSOR_TEST);
    drawSprite(camera, actor.body, actor.bounds, actor.body.worldPosition, actor.body.tint,
               SpriteMode::DrowningProbe, !actor.attachment.empty());

    gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Visibility]);
    gl_.UseProgram(visibilityProgram_);
    gl_.Viewport(0, 0, 1, 1);
    gl_.Uniform4i(visibilityBounds_, left, height_ - bottom, right, height_ - top);
    bind(0, images_[ProbeCoverage]);
    gl_.DrawArrays(GL_TRIANGLES, 0, 3);
    profile::glDraw();
    gl_.UseProgram(spriteProgram_);
    gl_.Uniform1i(sprite_.visibility, 1);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, targets_[Actors]);
    return true;
  }

  void drawActors(const CameraBasis& camera, const TubeScene::PreparedActors& actors, bool fluid) {
    startActors();
    spriteInputs(camera, actors.footprint, fluid);
    // Preparation puts larger owners first at equal body-plane depth. The
    // later body can overwrite the tie, even when a swimmer shares its plane.
    for (const auto& actor : actors.draws) {
      const auto& body = actor.body;
      if (actor.drowning) {
        const bool probe = needsDrowningProbe(actor);
        if (probe && !probeDrowning(camera, actor)) continue;
        const auto grey = actor.drowningGrey;
        drawSprite(camera, body, body.placement, actor.drowningPosition, {grey, grey, grey}, SpriteMode::Drowning);
        for (const auto& bubble : actor.bubbles)
          drawSprite(camera, bubble.sprite, bubble.sprite.placement, {}, {255, 255, 255},
                     SpriteMode::Bubble, false, bubble.strength);
        if (probe) gl_.Uniform1i(sprite_.visibility, 0);
      } else {
        if (!actor.attachment.empty()) drawBrick(actor.attachment);
        drawSprite(camera, body, actor.bounds, body.worldPosition, body.tint, SpriteMode::Body,
                   !actor.attachment.empty());
      }
    }
  }

  void drawParticles(const CameraBasis& camera, const TubeScene::PreparedParticles& particles) {
    startActors();
    spriteInputs(camera, particles.footprint, false);
    const auto draws = std::span(particles.draws);
    for (std::size_t i = 0; i < draws.size();) {
      if (const auto* sprite = std::get_if<TubeScene::PreparedParticles::Sprite>(&draws[i])) {
        using Depth = TubeScene::PreparedParticles::Sprite::Depth;
        const auto mode = sprite->depth == Depth::Origin ? SpriteMode::Origin :
                          sprite->depth == Depth::AxePlane ? SpriteMode::Axe : SpriteMode::Body;
        const auto& body = sprite->sprite;
        // These have higher owners than the bodies already in the target.
        drawSprite(camera, body, body.placement, body.worldPosition, body.tint, mode, false, 1.0, GL_LESS);
        ++i;
      } else {
        reset(effect_);
        gl_.ActiveTexture(GL_TEXTURE0);
        while (i < draws.size()) {
          const auto* triangle = std::get_if<PreparedRasterTriangle>(&draws[i]);
          if (!triangle) break;
          append(effect_, opaqueRasterDraw(*triangle, true), false);
          ++i;
        }
        upload(effect_, GL_STREAM_DRAW);
        drawGeometry(effect_, images_[SceneDepth]);
      }
    }
  }

  void end() {
    actorDepthReady_ = false;
    gl_.ActiveTexture(GL_TEXTURE0);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, 0);
    gl_.Disable(GL_BLEND);
    gl_.Disable(GL_DEPTH_TEST);
    gl_.DepthMask(GL_TRUE);
  }

  GlProgramCache& programs_;
  CurrentGlContext context_;
  HardwareGl gl_;
  std::array<GLuint, ImageCount> images_{};
  std::array<GLuint, TargetCount> targets_{};
  std::array<GLuint, 4> samplers_{};
  GLuint meshProgram_ = 0, spriteProgram_ = 0, skyProgram_ = 0, visibilityProgram_ = 0, emptyVao_ = 0;
  GLint visibilityBounds_ = -1;
  std::array<Geometry, std::size_t(Pass::TeleportEffects) + 1> passes_{};
  Geometry effect_;
  std::vector<Vertex> vertices_;
  std::map<const void*, GLuint> textures_;
  std::shared_ptr<const ActorFootprint> footprint_;
  ImageSlot colour_ = SceneColour;
  int width_ = 0, height_ = 0;
  float minimum_ = 0, scale_ = 0;
  bool fluidDepthReady_ = false, skyReady_ = false, brickReady_ = false, actorsReady_ = false, probeReady_ = false;
  bool actorDepthReady_ = false;
  static constexpr unsigned kCheckedFrames = 16;
  mutable unsigned checked_ = 0;
  struct {
    GLint viewport, depth, occlusion, textureAlpha, alpha;
  } mesh_{};
  struct {
    GLint state, frame, placement, uv, plane, tintStrength, depth, origin, height, rings, patchFluid, visibility;
    GLint eye, rayOrigin, rayDx, rayDy;
  } sprite_{};
};

}  // namespace

std::expected<std::unique_ptr<HardwareLevelScene>, std::string> createHardwareLevelScene(
    GlProgramCache& programs) {
  auto scene = std::make_unique<HardwareScene>(programs);
  if (auto started = scene->start(); !started) return std::unexpected(started.error());
  return scene;
}

}  // namespace webo::app
