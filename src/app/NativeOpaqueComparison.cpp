#define SDL_MAIN_HANDLED
#include "app/ActorGlShaders.h"
#include "app/ActorOcclusion.h"
#include "app/NativeGlApi.h"
#include "app/OpaqueMeshShaders.h"
#include "app/OpaqueSceneComparison.h"
#include "app/PresentationShaders.h"

#include <algorithm>
#include <bit>
#include <chrono>
#include <cmath>
#include <format>
#include <map>
#include <numbers>
#include <tuple>

namespace webo::app {
namespace {

#define OPAQUE_GL_FUNCTIONS(X) \
  X(Enable, decltype(&::glEnable)) \
  X(DepthFunc, decltype(&::glDepthFunc)) \
  X(DepthMask, decltype(&::glDepthMask)) \
  X(BlitFramebuffer, PFNGLBLITFRAMEBUFFERPROC) \
  X(ClearBufferfv, PFNGLCLEARBUFFERFVPROC) \
  X(ClearBufferuiv, PFNGLCLEARBUFFERUIVPROC) \
  X(DrawBuffers, PFNGLDRAWBUFFERSPROC) \
  X(GenBuffers, PFNGLGENBUFFERSPROC) \
  X(DeleteBuffers, PFNGLDELETEBUFFERSPROC) \
  X(BindBuffer, PFNGLBINDBUFFERPROC) \
  X(BufferData, PFNGLBUFFERDATAPROC) \
  X(BufferSubData, PFNGLBUFFERSUBDATAPROC) \
  X(DrawElementsInstanced, PFNGLDRAWELEMENTSINSTANCEDPROC) \
  X(GetUniformLocation, PFNGLGETUNIFORMLOCATIONPROC) \
  X(Uniform1i, PFNGLUNIFORM1IPROC) \
  X(Uniform1ui, PFNGLUNIFORM1UIPROC) \
  X(Uniform2i, PFNGLUNIFORM2IPROC) \
  X(Uniform2f, PFNGLUNIFORM2FPROC) \
  X(Uniform3f, PFNGLUNIFORM3FPROC) \
  X(Uniform4i, PFNGLUNIFORM4IPROC) \
  X(Uniform4ui, PFNGLUNIFORM4UIPROC) \
  X(Uniform4uiv, PFNGLUNIFORM4UIVPROC)

#ifdef __EMSCRIPTEN__
inline constexpr GLenum kOperandTarget = GL_TEXTURE_2D;
inline constexpr GLenum kSceneHighlightAttachment = GL_COLOR_ATTACHMENT1;
inline constexpr GLenum kActorDepthAttachment = GL_COLOR_ATTACHMENT2;
inline constexpr GLenum kActorHighlightAttachment = GL_COLOR_ATTACHMENT1;
#define DESKTOP_MESH_GL_FUNCTIONS(X)
#else
inline constexpr GLenum kOperandTarget = GL_TEXTURE_BUFFER;
inline constexpr GLenum kSceneHighlightAttachment = GL_COLOR_ATTACHMENT4;
inline constexpr GLenum kActorDepthAttachment = GL_COLOR_ATTACHMENT1;
inline constexpr GLenum kActorHighlightAttachment = GL_COLOR_ATTACHMENT2;
#define DESKTOP_MESH_GL_FUNCTIONS(X) \
  X(ColorMaski, PFNGLCOLORMASKIPROC) \
  X(GenQueries, PFNGLGENQUERIESPROC) \
  X(DeleteQueries, PFNGLDELETEQUERIESPROC) \
  X(BeginQuery, PFNGLBEGINQUERYPROC) \
  X(EndQuery, PFNGLENDQUERYPROC) \
  X(GetQueryObjectuiv, PFNGLGETQUERYOBJECTUIVPROC) \
  X(BeginConditionalRender, PFNGLBEGINCONDITIONALRENDERPROC) \
  X(EndConditionalRender, PFNGLENDCONDITIONALRENDERPROC) \
  X(TexBuffer, PFNGLTEXBUFFERPROC)
#endif

struct MeshGlFunctions : GlFunctions {
#define DECLARE_GL(name, type) type name = nullptr;
  OPAQUE_GL_FUNCTIONS(DECLARE_GL)
  DESKTOP_MESH_GL_FUNCTIONS(DECLARE_GL)
#undef DECLARE_GL

  std::expected<void, std::string> load() {
    if (auto shared = GlFunctions::load(); !shared) return shared;
#define LOAD_GL(name, type) \
    name = WEBO_GL_ENTRY(name, type); \
    if (!name) return std::unexpected("OpenGL entry point missing: gl" #name);
    OPAQUE_GL_FUNCTIONS(LOAD_GL)
    DESKTOP_MESH_GL_FUNCTIONS(LOAD_GL)
#undef LOAD_GL
    return {};
  }
};
#undef OPAQUE_GL_FUNCTIONS
#undef DESKTOP_MESH_GL_FUNCTIONS

using Words = std::array<std::uint32_t, 2>;
using PixelWords = std::array<std::uint32_t, 4>;
using MeshVertex = std::array<Words, 10>;
#ifdef __EMSCRIPTEN__
using MeshIndices = std::array<std::uint32_t, 8>;
#else
using MeshIndices = std::array<std::uint32_t, 4>;
#endif
constexpr std::size_t kTrianglesPerBuffer = 4096;  // 15 texels per triangle, below the 3.3 minimum.
constexpr GLuint kParticleOwnerBit = 0x80000000u;

GLuint encodeOwner(std::size_t owner) {
  return owner < ActorDepth::firstParticleOwner ? static_cast<GLuint>(owner) + 1 :
      kParticleOwnerBit | (static_cast<GLuint>(owner - ActorDepth::firstParticleOwner) + 1);
}

template<class T>
struct BufferTexture {
  GLuint buffer = 0, texture = 0;
  std::size_t capacity = 0;
  std::vector<T> contents;
};

struct MeshBuffer {
  BufferTexture<MeshVertex> vertices;
  BufferTexture<MeshIndices> indices;
};

bool sameState(const OpaqueDraw& a, const OpaqueDraw& b) {
  const auto& x = a.material;
  const auto& y = b.material;
  return bool(a.tube) == bool(b.tube) && a.backdrop == b.backdrop &&
      bool(a.sprite) == bool(b.sprite) &&
      (!a.tube || a.tube->texture == b.tube->texture) &&
      x.filter == y.filter && x.address == y.address && x.alphaFunction == y.alphaFunction &&
      x.alphaReference == y.alphaReference && x.blendEnabled == y.blendEnabled &&
      x.sourceBlend == y.sourceBlend && x.destinationBlend == y.destinationBlend &&
      x.depthFunction == y.depthFunction && x.depthWrite == y.depthWrite && x.shade == y.shade &&
      x.alphaOperation == y.alphaOperation && x.sampleOffsetX == y.sampleOffsetX &&
      x.sampleOffsetY == y.sampleOffsetY && x.highlight == y.highlight &&
      x.highlightTextureRgb == y.highlightTextureRgb;
}

template<class T>
void flipRows(std::vector<T>& values, int stride, int height) {
  for (int y = 0; y < height / 2; ++y) {
    auto first = values.begin() + std::size_t(y) * stride;
    auto other = values.begin() + std::size_t(height - 1 - y) * stride;
    std::swap_ranges(first, first + stride, other);
  }
}

class SceneGpu final : public OpaqueGpu {
 public:
  ~SceneGpu() override {
    if (!context_.ownsCurrent()) return;
    end();
    for (auto& [key, mesh] : meshes_) {
      release(mesh.vertices);
      release(mesh.indices);
    }
    for (const auto& [key, texture] : surfaces_) gl_.DeleteTextures(1, &texture);
    for (const auto& [key, texture] : atlases_) gl_.DeleteTextures(1, &texture);
    if (streamAtlas_) gl_.DeleteTextures(1, &streamAtlas_);
    release(actorRings_);
    release(hatchMesh_.vertices);
    release(hatchMesh_.indices);
    release(brickMesh_.vertices);
    release(brickMesh_.indices);
    release(particleMesh_.vertices);
    release(particleMesh_.indices);
#ifdef __EMSCRIPTEN__
    if (probeTarget_) gl_.DeleteFramebuffers(1, &probeTarget_);
    if (emptyOperands_) gl_.DeleteTextures(1, &emptyOperands_);
#else
    if (drowningVisible_) gl_.DeleteQueries(1, &drowningVisible_);
#endif
    if (highlightTexture_) gl_.DeleteTextures(1, &highlightTexture_);
    if (collision_) gl_.DeleteTextures(1, &collision_);
    if (actorTarget_) gl_.DeleteFramebuffers(1, &actorTarget_);
    if (brickTarget_) gl_.DeleteFramebuffers(1, &brickTarget_);
    if (actorData_) gl_.DeleteTextures(1, &actorData_);
    if (brickTextures_[0]) gl_.DeleteTextures(3, brickTextures_.data());
    for (const auto program : {actorProgram_, hatchProgram_, reduceProgram_})
      if (program) gl_.DeleteProgram(program);
    if (program_) gl_.DeleteProgram(program_);
    if (skyProgram_) gl_.DeleteProgram(skyProgram_);
    if (vao_) gl_.DeleteVertexArrays(1, &vao_);
    if (indices_) gl_.DeleteBuffers(1, &indices_);
    if (target_) gl_.DeleteFramebuffers(1, &target_);
    if (attachments_[0]) gl_.DeleteTextures(static_cast<GLsizei>(attachments_.size()), attachments_.data());
    for (const auto& copy : {beforeFluid_, afterFluid_, colourScratch_, depthScratch_, highlightScratch_,
                            hatchMask_, hatchRows_, hatchResult_}) {
      if (copy.framebuffer) gl_.DeleteFramebuffers(1, &copy.framebuffer);
      if (copy.texture) gl_.DeleteTextures(1, &copy.texture);
    }
  }

  std::expected<void, std::string> start() {
    if (!current()) return std::unexpected("GL scene requires a current context");
    if (auto loaded = gl_.load(); !loaded) return loaded;
#ifdef __EMSCRIPTEN__
    constexpr bool precise = false; // ES 3.0 has no shader binary64.
#else
    fp64_ = SDL_GL_ExtensionSupported("GL_ARB_gpu_shader_fp64");
    const bool precise = SDL_GL_ExtensionSupported("GL_ARB_gpu_shader5");
#endif
    precision_ = fp64_ ? (precise ? "binary64 precise" : "binary64") : "binary32";
    const auto shaderSource = [&](std::string_view body, bool mesh = true) {
      std::string source = presentationShader(kSceneShaderDialect, "");
      if (fp64_) source += "#extension GL_ARB_gpu_shader_fp64 : require\n#define MESH_FP64\n";
      if (precise) source += "#extension GL_ARB_gpu_shader5 : require\n#define MESH_PRECISE precise\n";
      else source += "#define MESH_PRECISE\n";
      source += kGlReal;
      if (mesh) source += kOpaqueMeshOperands;
      source += body;
      return source;
    };
    const auto program = linkGlProgram(gl_, shaderSource(kOpaqueMeshVertex), shaderSource(kOpaqueMeshFragment));
    if (!program) return std::unexpected(program.error());
    program_ = *program;
    const auto locate = [&](const char* name) { return gl_.GetUniformLocation(program_, name); };
    uniforms_ = {locate("meshVertices"), locate("meshIndices"), locate("surfaceImage"), locate("destinationImage"),
                 locate("viewportSize"), locate("firstTriangle"),
                 locate("firstOwner"), locate("depthInterval"), locate("sampling"), locate("alphaState"),
                 locate("logicalSampleOffset"), locate("blendFactors"), locate("effectSceneDepth"),
                 locate("previousActor"), locate("effectMode"), locate("particleOwner"),
                 locate("previousHighlight"), locate("highlightState")};
    auto skyProgram = linkGlProgram(gl_, presentationShader(kSceneShaderDialect, kPresentationVertex),
                                    presentationShader(kSceneShaderDialect, kBlueSkyFragment));
    if (!skyProgram) return std::unexpected(skyProgram.error());
    skyProgram_ = *skyProgram;
    skyUniforms_ = {gl_.GetUniformLocation(skyProgram_, "sceneColour"),
                    gl_.GetUniformLocation(skyProgram_, "sceneDepth"),
                    gl_.GetUniformLocation(skyProgram_, "fillColour")};
    const auto screenVertex = presentationShader(kSceneShaderDialect, kPresentationVertex);
    auto actorProgram = linkGlProgram(gl_, screenVertex, shaderSource(kActorFragment, false));
    if (!actorProgram) return std::unexpected(actorProgram.error());
    actorProgram_ = *actorProgram;
    const auto actorUniform = [&](const char* name) { return gl_.GetUniformLocation(actorProgram_, name); };
    actorUniforms_ = {actorUniform("viewport"), actorUniform("ringCount"), actorUniform("cameraRay[0]"),
        actorUniform("plane[0]"), actorUniform("rectangle[0]"), actorUniform("atlasUv[0]"),
        actorUniform("frameBox"), actorUniform("drawState"), actorUniform("actorTint"),
        actorUniform("effectOperands"), actorUniform("patchFluidDepth"), actorUniform("trackHighlights")};
#ifdef __EMSCRIPTEN__
    actorUniforms_.visibility = actorUniform("visibility");
    actorUniforms_.requireVisibility = actorUniform("requireVisibility");
    actorUniforms_.footprintCentre = actorUniform("footprintCentre");
    actorUniforms_.footprintOrigin = actorUniform("footprintOrigin");
    actorUniforms_.footprintDx = actorUniform("footprintDx");
    actorUniforms_.footprintDy = actorUniform("footprintDy");
#endif
    auto hatchProgram = linkGlProgram(gl_, shaderSource(kOpaqueMeshVertex), shaderSource(kHatchMaskFragment));
    if (!hatchProgram) return std::unexpected(hatchProgram.error());
    hatchProgram_ = *hatchProgram;
    auto reducer = linkGlProgram(gl_, screenVertex,
        presentationShader(kSceneShaderDialect, kHatchReduceFragment));
    if (!reducer) return std::unexpected(reducer.error());
    reduceProgram_ = *reducer;
    reduceUniforms_ = {gl_.GetUniformLocation(reduceProgram_, "viewport"),
        gl_.GetUniformLocation(reduceProgram_, "range"), gl_.GetUniformLocation(reduceProgram_, "reduceRows")};
    const auto driverString = [&](GLenum key) {
      const auto* value = gl_.GetString(key);
      return value ? reinterpret_cast<const char*>(value) : "unknown";
    };
    driver_ = std::format("{} / {} / {}", driverString(GL_VENDOR), driverString(GL_RENDERER),
                          driverString(GL_VERSION));

    GLint priorVao = 0;
    gl_.GetIntegerv(GL_VERTEX_ARRAY_BINDING, &priorVao);
    gl_.GenVertexArrays(1, &vao_);
    gl_.BindVertexArray(vao_);
    gl_.GenBuffers(1, &indices_);
    gl_.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_);
    constexpr std::array<GLubyte, 6> corners{0, 1, 2, 1, 3, 2};
    gl_.BufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(corners), corners.data(), GL_STATIC_DRAW);
    gl_.BindVertexArray(priorVao);
    gl_.GenFramebuffers(1, &target_);
    gl_.GenTextures(static_cast<GLsizei>(attachments_.size()), attachments_.data());
    gl_.GenFramebuffers(1, &actorTarget_);
    gl_.GenFramebuffers(1, &brickTarget_);
    gl_.GenTextures(1, &actorData_);
    gl_.GenTextures(1, &highlightTexture_);
    gl_.GenTextures(1, &streamAtlas_);
    gl_.GenTextures(1, &collision_);
    gl_.GenTextures(3, brickTextures_.data());
#ifdef __EMSCRIPTEN__
    gl_.GenFramebuffers(1, &probeTarget_);
    gl_.GenTextures(1, &emptyOperands_);
    textureImage(emptyOperands_, 1, 1, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
#else
    gl_.GenQueries(1, &drowningVisible_);
#endif
    for (auto* copy : {&beforeFluid_, &afterFluid_, &colourScratch_, &depthScratch_, &highlightScratch_,
                       &hatchMask_, &hatchRows_, &hatchResult_}) {
      gl_.GenFramebuffers(1, &copy->framebuffer);
      gl_.GenTextures(1, &copy->texture);
    }
    return status("allocate opaque comparison objects");
  }

  std::string_view driver() const override { return driver_; }
  std::string_view precision() const override { return precision_; }

  GlSceneImage sceneImage() const override {
    return {attachments_[0], attachments_[2], width_, height_, fp64_, trackHighlights_ ? highlightTexture_ : 0};
  }

  std::expected<void, std::string> begin(int width, int height,
                                       double minimum, double maximum, bool trackHighlights) override {
    if (!current()) return std::unexpected("opaque comparison lost its current GL context");
    saveBindings();
    trackHighlights_ = trackHighlights;
    gl_.BindFramebuffer(GL_FRAMEBUFFER, target_);
    gl_.ActiveTexture(GL_TEXTURE0);
    if (width != width_ || height != height_) {
      constexpr std::array specs{
          Attachment{GL_COLOR_ATTACHMENT0, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE},
#ifdef __EMSCRIPTEN__
          Attachment{GL_NONE, 0, GL_NONE, GL_NONE}, // Owner travels in the depth record.
#else
          Attachment{GL_COLOR_ATTACHMENT1, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT},
#endif
          Attachment{GL_COLOR_ATTACHMENT2, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT},
          Attachment{GL_COLOR_ATTACHMENT3, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT},
          Attachment{GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT}};
      for (std::size_t i = 0; i < specs.size(); ++i) {
        const auto [attachment, internal, layout, type] = specs[i];
        if (attachment == GL_NONE) continue;
        gl_.BindTexture(GL_TEXTURE_2D, attachments_[i]);
        gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        gl_.TexImage2D(GL_TEXTURE_2D, 0, internal, width, height, 0, layout, type, nullptr);
        gl_.FramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, attachments_[i], 0);
      }
      textureImage(highlightTexture_, width, height, GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT);
      gl_.FramebufferTexture2D(GL_FRAMEBUFFER, kSceneHighlightAttachment, GL_TEXTURE_2D, highlightTexture_, 0);
      selectDesktopSceneBuffers();
      enableSceneOutputs(true, true, true);
      if (gl_.CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return std::unexpected("opaque comparison framebuffer is incomplete");
      if (!allocateCopy(beforeFluid_, width, height, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT) ||
          !allocateCopy(afterFluid_, width, height, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT) ||
          !allocateCopy(colourScratch_, width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE) ||
          !allocateCopy(highlightScratch_, width, height, GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT))
        return std::unexpected("scene comparison snapshot framebuffer is incomplete");
      if (!allocateActorTargets(width, height))
        return std::unexpected("actor or hatch comparison framebuffer is incomplete");
      gl_.BindFramebuffer(GL_FRAMEBUFFER, target_);
      width_ = width;
      height_ = height;
    }
    gl_.Viewport(0, 0, width, height);
    gl_.Disable(GL_BLEND);
    gl_.Disable(GL_DITHER);
#ifndef __EMSCRIPTEN__
    gl_.Disable(GL_MULTISAMPLE);
    gl_.Disable(GL_FRAMEBUFFER_SRGB);
#endif
    gl_.Enable(GL_DEPTH_TEST);
    gl_.DepthMask(GL_TRUE);
    enableSceneOutputs(true, true, true);
#ifndef __EMSCRIPTEN__
    gl_.ColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
    constexpr GLfloat black[]{0, 0, 0, 1}, clearDepth = 1;
    constexpr GLuint clearOwner[4]{};
    const auto depthBits = clearDepthWords();
    gl_.ClearBufferfv(GL_COLOR, 0, black);
#ifndef __EMSCRIPTEN__
    gl_.ClearBufferuiv(GL_COLOR, 1, clearOwner);
#endif
    gl_.ClearBufferuiv(GL_COLOR, 2, depthBits.data());
    gl_.ClearBufferuiv(GL_COLOR, 3, depthBits.data());
    if (trackHighlights_) gl_.ClearBufferuiv(GL_COLOR, kSceneHighlightAttachment - GL_COLOR_ATTACHMENT0, clearOwner);
    gl_.ClearBufferfv(GL_DEPTH, 0, &clearDepth);
#ifndef __EMSCRIPTEN__
    gl_.ColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE); // The CPU target has RGB only.
#else
    bindTextureUnit(4, emptyOperands_);
    bindTextureUnit(5, emptyOperands_);
#endif
    gl_.UseProgram(program_);
    gl_.BindVertexArray(vao_);
    gl_.Uniform1i(uniforms_.vertices, 1);
    gl_.Uniform1i(uniforms_.indices, 2);
    gl_.Uniform1i(uniforms_.surface, 0);
    gl_.ActiveTexture(GL_TEXTURE3);
    gl_.BindTexture(GL_TEXTURE_2D, colourScratch_.texture);
    gl_.Uniform1i(uniforms_.destination, 3);
    gl_.Uniform1i(uniforms_.effectDepth, 4);
    gl_.Uniform1i(uniforms_.effectActor, 5);
    gl_.Uniform1i(uniforms_.effectMode, 0);
    gl_.Uniform1i(uniforms_.priorHighlight, 11);
    bindTextureUnit(11, highlightScratch_.texture);
    gl_.ActiveTexture(GL_TEXTURE0);
    gl_.Uniform2f(uniforms_.viewport, float(width), float(height));
    sceneMinimum_ = minimum;
    sceneExtent_ = maximum - minimum;
    setMeshDepthInterval(sceneMinimum_, sceneExtent_);
    nextOwner_ = 1;
    return status("begin opaque comparison");
  }

  std::expected<std::vector<OpaqueUpload>, std::string> draw(
      LevelScenePass pass, std::span<const OpaqueDrawRange> ranges) override {
    std::vector<OpaqueUpload> uploads;
    if (ranges.empty() && pass == LevelScenePass::TeleportEffects) return uploads;
    if (pass == LevelScenePass::TeleportEffects) {
      gl_.BindFramebuffer(GL_FRAMEBUFFER, target_);
      gl_.Viewport(0, 0, width_, height_);
      gl_.UseProgram(program_);
      gl_.Enable(GL_DEPTH_TEST);
      bindTextureUnit(3, colourScratch_.texture);
#ifdef __EMSCRIPTEN__
      bindTextureUnit(4, emptyOperands_);
      bindTextureUnit(5, emptyOperands_);
#endif
      setMeshDepthInterval(sceneMinimum_, sceneExtent_);
    }
    if (pass == LevelScenePass::BlueSky) {
      drawSky();
      if (auto drawn = status("draw late sky fill"); !drawn) return std::unexpected(drawn.error());
      return uploads;
    }
    if (pass == LevelScenePass::Fluid) copySceneDepth(beforeFluid_);
    const bool marks = pass == LevelScenePass::ExplosionMarks && !ranges.empty();
    if (marks) {
      copySceneDepth(depthScratch_);
      bindTextureUnit(4, depthScratch_.texture);
      gl_.Uniform1i(uniforms_.effectMode, 1);
      gl_.Disable(GL_DEPTH_TEST);
    }
    std::map<std::tuple<std::uint64_t, int, int>, std::size_t> occurrences;
    for (const auto& range : ranges) {
      const auto occurrence = occurrences[{range.source, range.band, range.bank}]++;
      for (std::size_t offset = 0; offset < range.draws.size(); offset += kTrianglesPerBuffer) {
        const auto draws = range.draws.subspan(offset,
            std::min(kTrianglesPerBuffer, range.draws.size() - offset));
        auto& mesh = meshes_[{pass, range.source, range.band, range.bank, occurrence, offset}];
        packMesh(draws);
        gl_.ActiveTexture(GL_TEXTURE1);
        const auto vertexBytes = sync(mesh.vertices, vertices_);
        gl_.ActiveTexture(GL_TEXTURE2);
        const auto indexBytes = sync(mesh.indices, triangleIndices_);
        if (vertexBytes != 0 || indexBytes != 0)
          uploads.push_back({range.source, range.band, range.bank, draws.size(), vertexBytes, indexBytes});
        gl_.Uniform1ui(uniforms_.owner, nextOwner_);
        gl_.ActiveTexture(GL_TEXTURE0);
        for (std::size_t first = 0; first < draws.size();) {
          std::size_t end = first + 1;
          while (end < draws.size() && sameState(draws[first], draws[end])) ++end;
          bindMaterial(draws[first], pass == LevelScenePass::Pipes ? MeshOutput::Pipes : MeshOutput::Scene);
          if (draws[first].material.blendEnabled) {
            for (std::size_t i = first; i < end; ++i)
              if (copyBlendDestination(draws[i])) drawTriangles(i, 1);
          } else {
            drawTriangles(first, end - first);
          }
          first = end;
        }
        nextOwner_ += static_cast<std::uint32_t>(draws.size());
      }
    }
    if (marks) {
      gl_.Uniform1i(uniforms_.effectMode, 0);
      gl_.Enable(GL_DEPTH_TEST);
    }
    if (pass == LevelScenePass::Fluid) copySceneDepth(afterFluid_);
    if (auto drawn = status("draw scene comparison"); !drawn) return std::unexpected(drawn.error());
    return uploads;
  }

  std::expected<OpaqueGpuReadback, std::string> read() override {
    const auto start = std::chrono::steady_clock::now();
    OpaqueGpuReadback frame;
    frame.colour.width = width_;
    frame.colour.height = height_;
    const auto pixels = frame.colour.pixelCount();
    frame.colour.rgba.resize(pixels * 4);
    frame.owners.resize(pixels);
    gl_.PixelStorei(GL_PACK_ALIGNMENT, 1);
    gl_.ReadBuffer(GL_COLOR_ATTACHMENT0);
    gl_.ReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, frame.colour.rgba.data());
#ifdef __EMSCRIPTEN__
    const auto words = readWords(GL_COLOR_ATTACHMENT2);
    std::vector<std::uint32_t> encodedDepth(words.size());
    for (std::size_t i = 0; i < words.size(); ++i) {
      frame.owners[i] = words[i][3];
      encodedDepth[i] = words[i][0];
    }
#else
    gl_.ReadBuffer(GL_COLOR_ATTACHMENT1);
    gl_.ReadPixels(0, 0, width_, height_, GL_RED_INTEGER, GL_UNSIGNED_INT, frame.owners.data());
    auto encodedDepth = readEncodedDepth(GL_COLOR_ATTACHMENT2);
#endif
    if (auto sampled = status("read opaque comparison"); !sampled) return std::unexpected(sampled.error());
    frame.milliseconds = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - start).count();
    flipRows(frame.colour.rgba, width_ * 4, height_);
    flipRows(frame.owners, width_, height_);
    frame.depth = decodeDepthRows(encodedDepth);
    return frame;
  }

  std::expected<SceneDepthSnapshots, std::string> readDepthSnapshots() override {
    SceneDepthSnapshots depths;
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, target_);
    auto pipes = readEncodedDepth(GL_COLOR_ATTACHMENT3);
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, beforeFluid_.framebuffer);
    auto beforeFluid = readEncodedDepth(GL_COLOR_ATTACHMENT0);
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, afterFluid_.framebuffer);
    auto afterFluid = readEncodedDepth(GL_COLOR_ATTACHMENT0);
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, target_);
    if (auto read = status("read scene depth snapshots"); !read) return std::unexpected(read.error());
    depths.pipes = decodeDepthRows(pipes);
    depths.beforeFluid = decodeDepthRows(beforeFluid);
    depths.afterFluid = decodeDepthRows(afterFluid);
    return depths;
  }

  std::expected<void, std::string> drawActors(const CameraBasis& camera,
      const TubeScene::PreparedActors& actors, bool patchFluidDepth) override {
    if (!actors.physical && !actors.draws.empty())
      return std::unexpected("actor comparison requires the physical actor pass");
    gl_.BindFramebuffer(GL_FRAMEBUFFER, actorTarget_);
    constexpr PixelWords empty{};
    enableActorOutputs(true, true, trackHighlights_);
    gl_.ClearBufferuiv(GL_COLOR, kActorDepthAttachment - GL_COLOR_ATTACHMENT0, empty.data());
    if (actors.draws.empty()) return {};
    configureActorInputs(camera, actors.footprint, patchFluidDepth);
    for (const auto& actor : actors.draws) {
      const auto& body = actor.body.placement;
      const auto box = spriteBox(actor.bounds);
      if (body.right <= body.left || body.bottom <= body.top || boxEmpty(box)) continue;
      const bool hasBrick = !actor.attachment.empty();
      if (hasBrick) drawBrick(actor.attachment);
      if (actor.drowning) drawDrowning(camera, actor, box, hasBrick);
      else drawSprite(camera, actor.body, box, actor.owner, SpriteMode::Body,
                      actor.body.worldPosition, actor.body.tint, hasBrick);
    }
    gl_.Viewport(0, 0, width_, height_);
    return status("draw actor bodies, bricks and drowning effects");
  }

  std::expected<void, std::string> drawParticles(const CameraBasis& camera,
      const TubeScene::PreparedParticles& particles) override {
    if (particles.draws.empty()) return {};
    configureActorInputs(camera, particles.footprint, false);
    std::vector<OpaqueDraw> triangles;
    const auto draws = std::span(particles.draws);
    for (std::size_t next = 0; next < draws.size();) {
      if (const auto* sprite = std::get_if<TubeScene::PreparedParticles::Sprite>(&draws[next])) {
        using Depth = TubeScene::PreparedParticles::Sprite::Depth;
        const auto mode = sprite->depth == Depth::Origin ? SpriteMode::OriginParticle :
                          sprite->depth == Depth::AxePlane ? SpriteMode::Axe : SpriteMode::Flight;
        drawSprite(camera, sprite->sprite, spriteBox(sprite->sprite.placement), sprite->owner, mode,
                   sprite->sprite.worldPosition, sprite->sprite.tint);
        ++next;
        continue;
      }
      triangles.clear();
      while (next < draws.size() && triangles.size() < kTrianglesPerBuffer) {
        const auto* triangle = std::get_if<PreparedRasterTriangle>(&draws[next]);
        if (!triangle) break;
        triangles.push_back(opaqueRasterDraw(*triangle, true));
        ++next;
      }
      drawParticleTriangles(triangles);
    }
    gl_.Viewport(0, 0, width_, height_);
    return status("draw ordered particles");
  }

  std::expected<GlActorReadback, std::string> readActors(bool readDrowningDepth) override {
    const auto start = std::chrono::steady_clock::now();
    GlActorReadback result;
    result.colour.width = width_;
    result.colour.height = height_;
    result.colour.rgba.resize(result.colour.pixelCount() * 4);
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, actorTarget_);
    gl_.ReadBuffer(GL_COLOR_ATTACHMENT0);
    gl_.ReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, result.colour.rgba.data());
    auto actor = readWords(kActorDepthAttachment);
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, target_);
    auto scene = readWords(GL_COLOR_ATTACHMENT2);
    auto pipes = readWords(GL_COLOR_ATTACHMENT3);
    std::vector<PixelWords> before, after;
    if (readDrowningDepth) {
      gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, beforeFluid_.framebuffer);
      before = readWords(GL_COLOR_ATTACHMENT0);
      gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, afterFluid_.framebuffer);
      after = readWords(GL_COLOR_ATTACHMENT0);
      gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, target_);
    }
    result.milliseconds = elapsedMs(start);
    if (auto read = status("read actor comparison"); !read) return std::unexpected(read.error());
    flipRows(result.colour.rgba, width_ * 4, height_);
    flipRows(actor, width_, height_);
    flipRows(scene, width_, height_);
    flipRows(pipes, width_, height_);
    if (readDrowningDepth) {
      flipRows(before, width_, height_);
      flipRows(after, width_, height_);
      result.drowningDepth.reserve(scene.size());
    }
    result.actor.reserve(actor.size());
    result.sceneDepth.reserve(actor.size());
    result.pipeDepth.reserve(actor.size());
    for (std::size_t i = 0; i < actor.size(); ++i) {
      result.actor.push_back(decodeActor(actor[i]));
      const auto z = decode({scene[i][1], scene[i][2]});
      result.sceneDepth.push_back(z);
      result.pipeDepth.push_back(decode({pipes[i][1], pipes[i][2]}));
      if (readDrowningDepth) {
        const auto postFluid = decode({after[i][1], after[i][2]});
        result.drowningDepth.push_back(z == postFluid ? decode({before[i][1], before[i][2]}) : z);
      }
    }
    return result;
  }

  std::expected<GlHatchQueries, std::string> readHatches(
      std::span<const PreparedHatchQuery> queries) override {
    GlHatchQueries result;
    if (queries.empty()) return result;
    if (queries.size() > static_cast<std::size_t>(width_))
      return std::unexpected("too many hatch queries for the reduction target");
    const ScopedQuery queryBindings(*this);
    gl_.Disable(GL_DEPTH_TEST);
#ifndef __EMSCRIPTEN__
    gl_.ColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
    gl_.Viewport(0, 0, width_, height_);
    for (std::size_t i = 0; i < queries.size(); ++i) {
      const auto box = drawHatchMask(queries[i]);
      if (box.left >= box.right || box.top >= box.bottom) {
        // A 1x1 draw into the result slot with an empty range emits zeros.
        reduceHatch(hatchRows_, hatchResult_, {static_cast<int>(i), 0, static_cast<int>(i) + 1, 1},
                    0, 0, true);
      } else {
        const int bottom = height_ - box.bottom, top = height_ - box.top;
        reduceHatch(hatchMask_, hatchRows_, {0, bottom, 1, top}, box.left, box.right, false);
        reduceHatch(hatchRows_, hatchResult_, {static_cast<int>(i), 0, static_cast<int>(i) + 1, 1},
                    bottom, top, true);
      }
    }
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, hatchResult_.framebuffer);
    gl_.ReadBuffer(GL_COLOR_ATTACHMENT0);
    std::vector<PixelWords> extents(queries.size());
    const auto start = std::chrono::steady_clock::now();
    gl_.ReadPixels(0, 0, static_cast<GLsizei>(queries.size()), 1, GL_RGBA_INTEGER, GL_UNSIGNED_INT, extents.data());
    result.readbackMilliseconds = elapsedMs(start);
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, target_);
    gl_.Viewport(0, 0, width_, height_);
    if (auto read = status("reduce visible hatch bounds"); !read) return std::unexpected(read.error());
    for (std::size_t i = 0; i < queries.size(); ++i) {
      const auto& e = extents[i];
      if (e[0] != 0)
        result.targets.push_back({queries[i].index, width_ - int(e[0]), height_ - int(e[1]), int(e[2]), int(e[3])});
    }
    return result;
  }

  std::expected<GlPointQuery, std::string> readPoint(int x, int y) override {
    if (!current()) return std::unexpected("point query lost the displayed frame's context");
    if (x < 0 || x >= width_ || y < 0 || y >= height_)
      return std::unexpected("point query outside the displayed frame");
    GLint prior = 0;
    gl_.GetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &prior);
    const auto start = std::chrono::steady_clock::now();
    PixelWords words{};
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, actorTarget_);
    gl_.ReadBuffer(kActorDepthAttachment);
    gl_.ReadPixels(x, height_ - 1 - y, 1, 1, GL_RGBA_INTEGER, GL_UNSIGNED_INT, words.data());
    auto pixel = decodeActor(words);
    if (pixel.owner == ActorDepth::noOwner) {
      gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, target_);
      gl_.ReadBuffer(GL_COLOR_ATTACHMENT2);
      gl_.ReadPixels(x, height_ - 1 - y, 1, 1, GL_RGBA_INTEGER, GL_UNSIGNED_INT, words.data());
      pixel.depth = decode({words[1], words[2]});
    }
    const double duration = elapsedMs(start);
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, prior);
    if (auto read = status("read displayed point"); !read) return std::unexpected(read.error());
    return GlPointQuery{pixel, duration};
  }

  std::expected<SceneHighlightId, std::string> readHighlight(int x, int y) override {
    if (!trackHighlights_ || x < 0 || x >= width_ || y < 0 || y >= height_) return SceneHighlightId{};
    const ScopedQuery queryBindings(*this);
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, target_);
    gl_.ReadBuffer(kSceneHighlightAttachment);
#ifdef __EMSCRIPTEN__
    PixelWords words{};
    gl_.ReadPixels(x, height_ - 1 - y, 1, 1, GL_RGBA_INTEGER, GL_UNSIGNED_INT, words.data());
#else
    Words words{};
    gl_.ReadPixels(x, height_ - 1 - y, 1, 1, GL_RG_INTEGER, GL_UNSIGNED_INT, words.data());
#endif
    if (auto read = status("read switch hover owner"); !read) return std::unexpected(read.error());
    return unpackSceneHighlight(words[0]);
  }

  std::expected<void, std::string> clipSensor(std::span<const SensorFragment> candidates,
                                             std::vector<HoverFragment>& fragments) override {
    if (candidates.empty()) return {};
    PickBox box{width_, height_, 0, 0};
    for (const auto& value : candidates) {
      const int x = int(value.fragment.pixel % width_), y = int(value.fragment.pixel / width_);
      box.left = std::min(box.left, x); box.top = std::min(box.top, y);
      box.right = std::max(box.right, x + 1); box.bottom = std::max(box.bottom, y + 1);
    }
    const int w = box.right - box.left, h = box.bottom - box.top;
    const ScopedQuery queryBindings(*this);
    std::vector<PixelWords> actor(std::size_t(w) * h), scene(actor.size());
    const auto readRegion = [&](GLuint framebuffer, GLenum attachment, auto& words) {
      gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
      gl_.ReadBuffer(attachment);
      gl_.ReadPixels(box.left, height_ - box.bottom, w, h, GL_RGBA_INTEGER, GL_UNSIGNED_INT, words.data());
    };
    readRegion(actorTarget_, kActorDepthAttachment, actor);
    readRegion(target_, GL_COLOR_ATTACHMENT2, scene);
    if (auto read = status("read sensor occluders"); !read) return read;
    for (const auto& value : candidates) {
      const int x = int(value.fragment.pixel % width_), y = int(value.fragment.pixel / width_);
      const auto p = std::size_t(box.bottom - 1 - y) * w + (x - box.left);
      if (value.depth < decode({scene[p][1], scene[p][2]}) && value.depth < decodeActor(actor[p]).depth)
        fragments.push_back(value.fragment);
    }
    return {};
  }

  std::expected<bool, std::string> markActor(const CameraBasis& camera, const TubeScene::PreparedActors& actors,
      std::size_t walker, bool patchFluidDepth, bool needCoverage) override {
    if (!actors.physical) return false;
    const auto actor = std::ranges::find(actors.draws, walker, &TubeScene::PreparedActors::Actor::owner);
    if (actor == actors.draws.end()) return false;
    const auto box = spriteBox(actor->body.placement);
    if (boxEmpty(box)) return false;
    const ScopedQuery queryBindings(*this);
    configureActorInputs(camera, actors.footprint, patchFluidDepth);
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, actorTarget_);
    GLuint visible = GL_TRUE;
#ifdef __EMSCRIPTEN__
    if (needCoverage) {
      probeSprite(camera, actor->body, box, walker,
                  actor->drowning ? SpriteMode::MarkDrowningProbe : SpriteMode::MarkProbe,
                  actor->body.worldPosition, {255, 255, 255}, false);
      gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, hatchResult_.framebuffer);
      gl_.ReadBuffer(GL_COLOR_ATTACHMENT0);
      PixelWords words{};
      gl_.ReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_UNSIGNED_INT, words.data());
      visible = words[0];
      gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, actorTarget_);
    }
#else
    if (needCoverage) gl_.BeginQuery(GL_ANY_SAMPLES_PASSED, drowningVisible_);
#endif
    enableActorOutputs(true, false, false);
    drawSprite(camera, actor->body, box, walker, actor->drowning ? SpriteMode::MarkDrowning : SpriteMode::Mark,
               actor->body.worldPosition, {255, 255, 255});
#ifndef __EMSCRIPTEN__
    if (needCoverage) {
      gl_.EndQuery(GL_ANY_SAMPLES_PASSED);
      gl_.GetQueryObjectuiv(drowningVisible_, GL_QUERY_RESULT, &visible);
    }
#endif
    enableActorOutputs(true, true, trackHighlights_);
    if (auto marked = status("draw hovered actor"); !marked) return std::unexpected(marked.error());
    return visible != 0;
  }

  void end() override {
    if (!active_ || !current()) return;
#ifndef __EMSCRIPTEN__
    gl_.ColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    gl_.ColorMaski(2, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    gl_.ColorMaski(3, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    gl_.ColorMaski(4, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
    gl_.DepthMask(GL_TRUE);
    gl_.Disable(GL_DEPTH_TEST);
    gl_.Disable(GL_BLEND);
    for (auto unit = GL_TEXTURE1; unit <= GL_TEXTURE12; ++unit) {
      gl_.ActiveTexture(unit);
#ifndef __EMSCRIPTEN__
      gl_.BindTexture(GL_TEXTURE_BUFFER, 0);
#endif
      gl_.BindTexture(GL_TEXTURE_2D, 0);
    }
    gl_.ActiveTexture(GL_TEXTURE0);
    gl_.BindTexture(GL_TEXTURE_2D, 0);
#ifndef __EMSCRIPTEN__
    gl_.BindBuffer(GL_TEXTURE_BUFFER, 0);
#endif
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, priorDrawFramebuffer_);
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, priorReadFramebuffer_);
    gl_.BindVertexArray(priorVao_);
    gl_.UseProgram(priorProgram_);
    active_ = false;
  }

 private:
  struct CopyTarget { GLuint framebuffer = 0, texture = 0; };

  void saveBindings() {
    gl_.GetIntegerv(GL_CURRENT_PROGRAM, &priorProgram_);
    gl_.GetIntegerv(GL_VERTEX_ARRAY_BINDING, &priorVao_);
    gl_.GetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &priorDrawFramebuffer_);
    gl_.GetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &priorReadFramebuffer_);
    active_ = true;
  }

  struct ScopedQuery {
    SceneGpu& gpu;
    bool restore;
    explicit ScopedQuery(SceneGpu& context) : gpu(context), restore(!gpu.active_) {
      if (restore) gpu.saveBindings();
      gpu.gl_.BindVertexArray(gpu.vao_);
    }
    ~ScopedQuery() { if (restore) gpu.end(); }
  };

  static double elapsedMs(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
  }

  static int pixelBound(double value, int limit) { return int(std::clamp(value, 0.0, double(limit))); }

  PixelWords clearDepthWords() const {
    const auto depth = encode(std::numeric_limits<double>::infinity());
    return {std::bit_cast<std::uint32_t>(1.0f), depth[0], depth[1], 0};
  }

  void setMeshDepthInterval(double minimum, double extent) {
#ifdef __EMSCRIPTEN__
    meshDepthMinimum_ = minimum;
    meshDepthScale_ = 0.5 / extent;
#endif
    const auto lower = encode(minimum), span = encode(extent);
    gl_.Uniform4ui(uniforms_.interval, lower[0], lower[1], span[0], span[1]);
  }

  void textureImage(GLuint texture, int width, int height, GLint internal, GLenum layout,
                    GLenum type, const void* data = nullptr) {
    gl_.BindTexture(GL_TEXTURE_2D, texture);
    gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl_.TexImage2D(GL_TEXTURE_2D, 0, internal, width, height, 0, layout, type, data);
  }

  bool allocateActorTargets(int width, int height) {
    textureImage(actorData_, width, height, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, actorTarget_);
    gl_.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, attachments_[0], 0);
    gl_.FramebufferTexture2D(GL_FRAMEBUFFER, kActorDepthAttachment, GL_TEXTURE_2D, actorData_, 0);
    gl_.FramebufferTexture2D(GL_FRAMEBUFFER, kActorHighlightAttachment, GL_TEXTURE_2D, highlightTexture_, 0);
    selectDesktopActorBuffers();
    enableActorOutputs(true, true, trackHighlights_);
    if (gl_.CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;

    gl_.BindFramebuffer(GL_FRAMEBUFFER, brickTarget_);
    textureImage(brickTextures_[0], width, height, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
    textureImage(brickTextures_[1], width, height, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
    textureImage(brickTextures_[2], width, height, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
    gl_.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brickTextures_[0], 0);
    gl_.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, brickTextures_[1], 0);
    gl_.FramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, brickTextures_[2], 0);
    constexpr GLenum brickOutputs[]{GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT2};
    gl_.DrawBuffers(3, brickOutputs);
    if (gl_.CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;
    if (!allocateCopy(depthScratch_, width, height, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT) ||
        !allocateCopy(hatchMask_, width, height, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE) ||
        !allocateCopy(hatchRows_, 1, height, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT) ||
        !allocateCopy(hatchResult_, width, 1, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT)) return false;
#ifdef __EMSCRIPTEN__
    gl_.BindFramebuffer(GL_FRAMEBUFFER, probeTarget_);
    gl_.FramebufferTexture2D(GL_FRAMEBUFFER, kActorDepthAttachment, GL_TEXTURE_2D, hatchMask_.texture, 0);
    enableActorOutputs(false, true, false);
    gl_.ReadBuffer(GL_NONE);
    if (gl_.CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return false;
#endif
    return true;
  }

  void bindTextureUnit(int unit, GLuint texture, GLenum target = GL_TEXTURE_2D) {
    gl_.ActiveTexture(GL_TEXTURE0 + unit);
    gl_.BindTexture(target, texture);
  }

  template<std::size_t N>
  void uploadOperands(GLint location, const std::array<double, N>& values) {
    static_assert(N % 2 == 0);
    std::array<std::uint32_t, N * 2> words;
    for (std::size_t i = 0; i < N; ++i) {
      const auto [lo, hi] = encode(values[i]);
      words[2 * i] = lo;
      words[2 * i + 1] = hi;
    }
    gl_.Uniform4uiv(location, N / 2, words.data());
  }

  enum class SpriteMode {
    Body, Drowning, Bubble, OriginParticle, Axe, Flight, DrowningProbe, Mark, MarkDrowning, MarkProbe, MarkDrowningProbe
  };

  PickBox spriteBox(const ActorSpritePlacement& p) const {
    return {pixelBound(std::ceil(p.left), width_), pixelBound(std::ceil(p.top), height_),
            pixelBound(std::ceil(p.right), width_), pixelBound(std::ceil(p.bottom), height_)};
  }

  static bool boxEmpty(const PickBox& box) {
    return box.left >= box.right || box.top >= box.bottom;
  }

  void copyActorRegion(GLenum attachment, const CopyTarget& copy, const PickBox& box) {
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, actorTarget_);
    gl_.ReadBuffer(attachment);
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, copy.framebuffer);
    const int y0 = height_ - box.bottom, y1 = height_ - box.top;
    gl_.BlitFramebuffer(box.left, y0, box.right, y1, box.left, y0, box.right, y1,
                        GL_COLOR_BUFFER_BIT, GL_NEAREST);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, actorTarget_);
  }

  void configureActorInputs(const CameraBasis& camera,
      const std::shared_ptr<const ActorFootprint>& footprint, bool patchFluidDepth) {
    gl_.UseProgram(actorProgram_);
    // Use distinct units for the three sampler types on the actor program.
    constexpr std::array names{"atlas", "sceneDepth", "pipeDepth", "previousActor",
        "brickDepth", "brickColour", "rings", "collision", "beforeFluid", "afterFluid",
        "destinationImage", "previousHighlight"};
    for (std::size_t i = 0; i < names.size(); ++i)
      gl_.Uniform1i(gl_.GetUniformLocation(actorProgram_, names[i]), static_cast<GLint>(i));
    gl_.Uniform2i(actorUniforms_.viewport, width_, height_);
    gl_.Uniform1i(actorUniforms_.patchFluidDepth, patchFluidDepth);
    gl_.Uniform1i(actorUniforms_.trackHighlights, trackHighlights_);
#ifndef __EMSCRIPTEN__
    const auto& m = camera.worldToView;
    const auto& eye = camera.eyeWorld;
    uploadOperands(actorUniforms_.camera, std::array{camera.cx, camera.cy, camera.focalX, camera.focalY,
        eye[0], eye[1], eye[2], m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]});
#else
    (void)camera;
#endif
    const bool valid = footprint && footprint->lattice.angularSteps == 32 &&
        footprint->lattice.ringCount >= 2 && footprint->collision.width == 256 &&
        footprint->lattice.vertices.size() >= footprint->lattice.ringCount * 64 &&
        footprint->collision.height > 0 &&
        footprint->collision.cells.size() == std::size_t(footprint->collision.width) * footprint->collision.height;
    gl_.Uniform1i(actorUniforms_.ringCount, valid ? int(footprint->lattice.ringCount) : 0);
    if (valid && footprint != uploadedFootprint_) {
      std::vector<PixelWords> corners;
      const auto vertices = std::span(footprint->lattice.vertices).first(footprint->lattice.ringCount * 64);
      corners.reserve(vertices.size() * 2 + 32);
      for (const auto& v : vertices) {
        const auto x = encode(v.x), y = encode(v.y), z = encode(v.z);
        corners.push_back({x[0], x[1], y[0], y[1]});
        corners.push_back({z[0], z[1], 0, 0});
      }
      for (int step = 0; step < 32; ++step) {
        const double angle = step * (2 * std::numbers::pi / 32);
        const auto x = encode(std::cos(angle)), y = encode(std::sin(angle));
        corners.push_back({x[0], x[1], y[0], y[1]});
      }
      gl_.ActiveTexture(GL_TEXTURE6);
      sync(actorRings_, corners);
      std::vector<PixelWords> cells;
      cells.reserve(footprint->collision.cells.size());
      for (std::size_t i = 0; i < footprint->collision.cells.size(); ++i) {
        const auto thickness = encode(i < footprint->gateThickness.size() ? footprint->gateThickness[i] : 1.0);
        cells.push_back({footprint->collision.cells[i], thickness[0], thickness[1], 0});
      }
      gl_.ActiveTexture(GL_TEXTURE7);
      textureImage(collision_, int(footprint->collision.width), int(footprint->collision.height),
                   GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, cells.data());
      uploadedFootprint_ = footprint;
    }
    bindTextureUnit(6, actorRings_.texture
#ifdef __EMSCRIPTEN__
        ? actorRings_.texture : emptyOperands_
#endif
        , kOperandTarget);
    bindTextureUnit(7, collision_);
    bindTextureUnit(8, beforeFluid_.texture);
    bindTextureUnit(9, afterFluid_.texture);
    bindTextureUnit(10, colourScratch_.texture);
    bindTextureUnit(11, highlightScratch_.texture);
#ifdef __EMSCRIPTEN__
    gl_.Uniform1i(actorUniforms_.visibility, 12);
    gl_.Uniform1i(actorUniforms_.requireVisibility, 0);
    bindTextureUnit(12, hatchResult_.texture);
#endif
  }

  void drawSprite(const CameraBasis& camera, const PreparedActorSprite& body, const PickBox& box,
      std::size_t owner, SpriteMode mode, const std::array<double, 3>& worldPosition,
      std::array<std::uint8_t, 3> tint, bool hasBrick = false, double strength = 1.0, GLuint target = 0) {
    const auto& p = body.placement;
    const auto& f = body.frame;
    if (boxEmpty(box) || f.width == 0 || f.height == 0) return;
    if (mode != SpriteMode::DrowningProbe)
      copyActorRegion(kActorDepthAttachment, depthScratch_, box);
    const bool translucent = mode == SpriteMode::Drowning || mode == SpriteMode::Bubble;
    if (translucent || mode == SpriteMode::Mark || mode == SpriteMode::MarkDrowning)
      copyActorRegion(GL_COLOR_ATTACHMENT0, colourScratch_, box);
    if (trackHighlights_ && translucent)
      copyActorRegion(kActorHighlightAttachment, highlightScratch_, box);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, target ? target : actorTarget_);
    gl_.Disable(GL_DEPTH_TEST);
    gl_.UseProgram(actorProgram_);
    gl_.Viewport(box.left, height_ - box.bottom, box.right - box.left, box.bottom - box.top);
    bindTextureUnit(1, attachments_[2]);
    bindTextureUnit(2, attachments_[3]);
    bindTextureUnit(3, depthScratch_.texture);
    bindTextureUnit(4, brickTextures_[1]);
    bindTextureUnit(5, brickTextures_[0]);
    gl_.ActiveTexture(GL_TEXTURE0);
    const auto& image = body.sheet();
    if (body.texture) {
      auto [atlas, created] = atlases_.try_emplace(body.texture, 0);
      if (created) {
        gl_.GenTextures(1, &atlas->second);
        textureImage(atlas->second, image.width, image.height, GL_RGBA8UI, GL_RGBA_INTEGER,
                     GL_UNSIGNED_BYTE, image.rgba.data());
      } else {
        gl_.BindTexture(GL_TEXTURE_2D, atlas->second);
      }
    } else {
      textureImage(streamAtlas_, image.width, image.height, GL_RGBA8UI, GL_RGBA_INTEGER,
                   GL_UNSIGNED_BYTE, image.rgba.data());
    }
    const ActorOcclusionPlane plane(camera, worldPosition);
    const auto coefficients = plane.coefficients();
    uploadOperands(actorUniforms_.plane, coefficients);
#ifdef __EMSCRIPTEN__
    // Use the body placement as the anchor for both the body and its hover
    // mark. The assembly bounds may also include a carried brick.
    uploadFootprintProjection(camera, plane, coefficients, spriteBox(p));
#endif
    uploadOperands(actorUniforms_.placement, std::array{p.left, p.top, p.right, p.bottom});
    uploadOperands(actorUniforms_.uv, std::array<double, 4>{f.uLeft, f.uTop, f.uRight, f.uBottom});
    uploadOperands(actorUniforms_.effect, std::array{actorPhysicalDepth(p, width_), strength});
    gl_.Uniform4i(actorUniforms_.frame, f.left, f.top, f.right, f.bottom);
    gl_.Uniform4i(actorUniforms_.state, body.mirrored, body.inverted, hasBrick, static_cast<int>(mode));
    gl_.Uniform4ui(actorUniforms_.tint, tint[0], tint[1], tint[2], encodeOwner(owner));
    gl_.DrawArrays(GL_TRIANGLES, 0, 3);
  }

#ifdef __EMSCRIPTEN__
  void uploadFootprintProjection(const CameraBasis& camera, const ActorOcclusionPlane& plane,
                                 const std::array<double, 4>& coefficients, const PickBox& box) {
    const double x = (box.left + box.right) * 0.5;
    const double y = (box.top + box.bottom) * 0.5;
    const double denominator = coefficients[1] + coefficients[2] * x + coefficients[3] * y;
    const auto origin = plane.worldAt(camera, x, y);
    const auto& basis = camera.worldToView;
    std::array<double, 3> dx, dy;
    for (int axis = 0; axis < 3; ++axis) {
      const double eyeOffset = camera.eyeWorld[axis] - origin[axis];
      dx[axis] = eyeOffset * coefficients[2] + coefficients[0] * basis[3 * axis] / camera.focalX;
      dy[axis] = eyeOffset * coefficients[3] - coefficients[0] * basis[3 * axis + 1] / camera.focalY;
    }
    const auto uploadVec3 = [&](GLint location, const std::array<double, 3>& v) {
      gl_.Uniform3f(location, static_cast<float>(v[0]), static_cast<float>(v[1]), static_cast<float>(v[2]));
    };
    uploadVec3(actorUniforms_.footprintCentre, {x, y, denominator});
    uploadVec3(actorUniforms_.footprintOrigin, origin);
    uploadVec3(actorUniforms_.footprintDx, dx);
    uploadVec3(actorUniforms_.footprintDy, dy);
  }

  void probeSprite(const CameraBasis& camera, const PreparedActorSprite& sprite, const PickBox& box,
                   std::size_t owner, SpriteMode mode, const std::array<double, 3>& worldPosition,
                   std::array<std::uint8_t, 3> tint, bool hasBrick) {
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, probeTarget_);
    constexpr PixelWords zero{};
    gl_.ClearBufferuiv(GL_COLOR, kActorDepthAttachment - GL_COLOR_ATTACHMENT0, zero.data());
    drawSprite(camera, sprite, box, owner, mode, worldPosition, tint, hasBrick, 1.0, probeTarget_);
    const int bottom = height_ - box.bottom, top = height_ - box.top;
    reduceHatch(hatchMask_, hatchRows_, {0, bottom, 1, top}, box.left, box.right, false);
    reduceHatch(hatchRows_, hatchResult_, {0, 0, 1, 1}, bottom, top, true);
    gl_.UseProgram(actorProgram_);
  }
#endif

  void drawDrowning(const CameraBasis& camera, const TubeScene::PreparedActors::Actor& actor,
                    const PickBox& bounds, bool hasBrick) {
    const auto& body = actor.body;
    const bool probe = !actor.bubbles.empty() || hasBrick || body.worldPosition != actor.drowningPosition;
    if (probe) {
#ifdef __EMSCRIPTEN__
      // WebGL queries complete on a later browser turn. Reduce the visibility
      // mask on the GPU so the body and bubbles can use it in this frame.
      probeSprite(camera, body, bounds, actor.owner, SpriteMode::DrowningProbe, body.worldPosition,
                  body.tint, hasBrick);
      gl_.Uniform1i(actorUniforms_.requireVisibility, 1);
#else
      enableActorOutputs(false, false, false);
      gl_.BeginQuery(GL_ANY_SAMPLES_PASSED, drowningVisible_);
      drawSprite(camera, body, bounds, actor.owner, SpriteMode::DrowningProbe, body.worldPosition,
                 body.tint, hasBrick);
      gl_.EndQuery(GL_ANY_SAMPLES_PASSED);
      enableActorOutputs(true, true, trackHighlights_);
      // The query gates body/bubble draws on the context thread without a CPU readback.
      gl_.BeginConditionalRender(drowningVisible_, GL_QUERY_WAIT);
#endif
    }
    const auto grey = actor.drowningGrey;
    drawSprite(camera, body, spriteBox(body.placement), actor.owner, SpriteMode::Drowning,
               actor.drowningPosition, {grey, grey, grey});
    for (const auto& bubble : actor.bubbles)
      drawSprite(camera, bubble.sprite, spriteBox(bubble.sprite.placement), actor.owner,
                 SpriteMode::Bubble, {}, {255, 255, 255}, false, bubble.strength);
    if (probe) {
#ifdef __EMSCRIPTEN__
      gl_.Uniform1i(actorUniforms_.requireVisibility, 0);
#else
      gl_.EndConditionalRender();
#endif
    }
  }

  void bindPackedMesh(MeshBuffer& mesh, std::span<const OpaqueDraw> draws) {
    packMesh(draws);
    gl_.ActiveTexture(GL_TEXTURE1);
    sync(mesh.vertices, vertices_);
    gl_.ActiveTexture(GL_TEXTURE2);
    sync(mesh.indices, triangleIndices_);
  }

  void drawParticleTriangles(std::span<const OpaqueDraw> draws) {
    gl_.BindFramebuffer(GL_FRAMEBUFFER, actorTarget_);
    // Reuse the mesh shader's integer depth output for the actor layer.
#ifdef __EMSCRIPTEN__
    enableActorOutputs(true, true, trackHighlights_);
#else
    constexpr GLenum particleOutputs[]{GL_COLOR_ATTACHMENT0, GL_NONE, kActorDepthAttachment,
                                       GL_NONE, kActorHighlightAttachment};
    gl_.DrawBuffers(5, particleOutputs);
#endif
    gl_.Viewport(0, 0, width_, height_);
    gl_.Disable(GL_DEPTH_TEST);
    gl_.UseProgram(program_);
    gl_.Uniform1i(uniforms_.effectMode, 2);
    bindPackedMesh(particleMesh_, draws);
#ifdef __EMSCRIPTEN__
    bindTextureUnit(3, colourScratch_.texture);
#endif
    bindTextureUnit(4, attachments_[2]);
    bindTextureUnit(5, depthScratch_.texture);
    for (std::size_t i = 0; i < draws.size(); ++i) {
      const auto& draw = draws[i];
      const auto& [a, b, c] = draw.vertices;
      const PickBox box{
          pixelBound(std::floor(std::min({a.screenX, b.screenX, c.screenX})), width_),
          pixelBound(std::floor(std::min({a.screenY, b.screenY, c.screenY})), height_),
          pixelBound(std::ceil(std::max({a.screenX, b.screenX, c.screenX})), width_),
          pixelBound(std::ceil(std::max({a.screenY, b.screenY, c.screenY})), height_)};
      if (boxEmpty(box)) continue;
      copyActorRegion(kActorDepthAttachment, depthScratch_, box);
      gl_.ActiveTexture(GL_TEXTURE0);
      bindMaterial(draw, MeshOutput::Particle);
#ifndef __EMSCRIPTEN__
      gl_.ColorMaski(2, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
      gl_.Uniform1ui(uniforms_.particleOwner, encodeOwner(draw.tube->actorOwner));
      drawTriangles(i, 1);
    }
    gl_.Uniform1i(uniforms_.effectMode, 0);
    selectDesktopActorBuffers();
    enableActorOutputs(true, true, trackHighlights_);
  }

  void drawBrick(std::span<const PreparedRasterTriangle> triangles) {
    gl_.BindFramebuffer(GL_FRAMEBUFFER, brickTarget_);
    gl_.Viewport(0, 0, width_, height_);
    gl_.Enable(GL_DEPTH_TEST);
    gl_.DepthMask(GL_TRUE);
#ifndef __EMSCRIPTEN__
    gl_.ColorMaski(2, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
    constexpr GLfloat clear = 1.0f;
    const auto words = clearDepthWords();
    gl_.ClearBufferuiv(GL_COLOR, 2, words.data());
    gl_.ClearBufferfv(GL_DEPTH, 0, &clear);
    gl_.UseProgram(program_);
    // The depth-only parts of this target are private to this one carried brick.
    double minimum = std::numeric_limits<double>::infinity(), maximum = -minimum;
    std::vector<OpaqueDraw> draws;
    draws.reserve(triangles.size());
    for (const auto& triangle : triangles) {
      const auto& draw = draws.emplace_back(opaqueRasterDraw(triangle, true));
      for (const auto& v : draw.vertices) {
        minimum = std::min(minimum, v.screenZ);
        maximum = std::max(maximum, v.screenZ);
      }
    }
    const double margin = std::max(1e-4, (maximum - minimum) * 0.01);
    setMeshDepthInterval(minimum - margin, maximum - minimum + 2 * margin);
    bindPackedMesh(brickMesh_, draws);
    gl_.ActiveTexture(GL_TEXTURE0);
    // All six prepared cuboid faces have the same material sampling state.
    bindMaterial(draws.front(), MeshOutput::Brick);
    bindTextureUnit(3, colourScratch_.texture);
#ifdef __EMSCRIPTEN__
    bindTextureUnit(4, emptyOperands_);
    bindTextureUnit(5, emptyOperands_);
#else
    bindTextureUnit(4, 0);
    bindTextureUnit(5, 0);
#endif
    drawTriangles(0, draws.size());
    setMeshDepthInterval(sceneMinimum_, sceneExtent_);
  }

  PickBox drawHatchMask(const PreparedHatchQuery& query) {
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, hatchMask_.framebuffer);
    gl_.Viewport(0, 0, width_, height_);
    constexpr PixelWords zero{};
    gl_.ClearBufferuiv(GL_COLOR, 0, zero.data());
    PickBox box{width_, height_, 0, 0};
    std::vector<OpaqueDraw> draws;
    draws.reserve(query.triangles.size());
    for (const auto& t : query.triangles) {
      auto& draw = draws.emplace_back(OpaqueDraw{});
      for (std::size_t i = 0; i < 3; ++i) {
        const auto& [x, y, z] = t[i];
        draw.vertices[i] = {x, y, z, 1, {}, 0, 0};
        box.left = std::min(box.left, pixelBound(std::floor(x), width_));
        box.top = std::min(box.top, pixelBound(std::floor(y), height_));
        box.right = std::max(box.right, pixelBound(std::ceil(x), width_));
        box.bottom = std::max(box.bottom, pixelBound(std::ceil(y), height_));
      }
    }
    gl_.UseProgram(hatchProgram_);
    const auto uniform = [&](const char* name) { return gl_.GetUniformLocation(hatchProgram_, name); };
    gl_.Uniform1i(uniform("meshVertices"), 1);
    gl_.Uniform1i(uniform("meshIndices"), 2);
    gl_.Uniform1i(uniform("sceneDepth"), 0);
    gl_.Uniform1i(uniform("firstTriangle"), 0);
    gl_.Uniform2f(uniform("viewportSize"), float(width_), float(height_));
    bindTextureUnit(0, attachments_[2]);
    for (std::size_t offset = 0; offset < draws.size(); offset += kTrianglesPerBuffer) {
      const auto batch = std::span(draws).subspan(offset, std::min(kTrianglesPerBuffer, draws.size() - offset));
      bindPackedMesh(hatchMesh_, batch);
      gl_.DrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, static_cast<GLsizei>(batch.size()));
    }
    return box;
  }

  void reduceHatch(const CopyTarget& source, const CopyTarget& target, const PickBox& glBox,
                    int begin, int end, bool reduceRows) {
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, target.framebuffer);
    gl_.Viewport(glBox.left, glBox.top, glBox.right - glBox.left, glBox.bottom - glBox.top);
    gl_.UseProgram(reduceProgram_);
    bindTextureUnit(0, source.texture);
    gl_.Uniform2i(reduceUniforms_.viewport, width_, height_);
    gl_.Uniform2i(reduceUniforms_.range, begin, end);
    gl_.Uniform1i(reduceUniforms_.rows, reduceRows);
    gl_.DrawArrays(GL_TRIANGLES, 0, 3);
  }

  std::vector<PixelWords> readWords(GLenum attachment) {
    std::vector<PixelWords> pixels(std::size_t(width_) * height_);
    gl_.ReadBuffer(attachment);
    gl_.ReadPixels(0, 0, width_, height_, GL_RGBA_INTEGER, GL_UNSIGNED_INT, pixels.data());
    return pixels;
  }

  GlActorPixel decodeActor(const PixelWords& words) const {
    if (words[2] == 0) return {};
    const auto code = words[2];
    const std::size_t owner = code & kParticleOwnerBit ?
        ActorDepth::firstParticleOwner + (code & ~kParticleOwnerBit) - 1 : code - 1;
    return {decode({words[0], words[1]}), owner,
        static_cast<GlActorPixel::Kind>(words[3])};
  }

  void selectDesktopSceneBuffers() {
#ifndef __EMSCRIPTEN__
    constexpr std::array<GLenum, 5> outputs{
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4};
    gl_.DrawBuffers(static_cast<GLsizei>(outputs.size()), outputs.data());
#endif
  }

  void selectDesktopActorBuffers() {
#ifndef __EMSCRIPTEN__
    constexpr GLenum outputs[]{GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    gl_.DrawBuffers(3, outputs);
#endif
  }

  void enableSceneOutputs(bool depth, bool pipe, bool highlight) {
#ifdef __EMSCRIPTEN__
    // In ES each enabled output must name the matching attachment index.
    const GLenum outputs[]{GL_COLOR_ATTACHMENT0, highlight ? kSceneHighlightAttachment : GL_NONE,
                           depth ? GLenum(GL_COLOR_ATTACHMENT2) : GL_NONE,
                           pipe ? GLenum(GL_COLOR_ATTACHMENT3) : GL_NONE};
    gl_.DrawBuffers(4, outputs);
#else
    gl_.ColorMaski(2, depth, depth, depth, depth);
    gl_.ColorMaski(3, pipe, pipe, pipe, pipe);
    gl_.ColorMaski(4, highlight, highlight, highlight, highlight);
#endif
  }

  void enableActorOutputs(bool colour, bool actor, bool highlight) {
#ifdef __EMSCRIPTEN__
    const GLenum outputs[]{colour ? GLenum(GL_COLOR_ATTACHMENT0) : GL_NONE,
                           highlight ? kActorHighlightAttachment : GL_NONE,
                           actor ? kActorDepthAttachment : GL_NONE};
    gl_.DrawBuffers(3, outputs);
#else
    gl_.ColorMaski(0, colour, colour, colour, GL_FALSE);
    gl_.ColorMaski(1, actor, actor, actor, actor);
    gl_.ColorMaski(2, highlight, highlight, highlight, highlight);
#endif
  }

  bool allocateCopy(const CopyTarget& copy, int width, int height, GLint internal,
                    GLenum layout, GLenum type) {
    textureImage(copy.texture, width, height, internal, layout, type);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, copy.framebuffer);
    gl_.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, copy.texture, 0);
    return gl_.CheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
  }

  void blitColour() {
    gl_.BlitFramebuffer(0, 0, width_, height_, 0, 0, width_, height_, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  }

  void copySceneDepth(const CopyTarget& copy) {
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, target_);
    gl_.ReadBuffer(GL_COLOR_ATTACHMENT2);
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, copy.framebuffer);
    blitColour();
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, target_);
  }

  bool copyBlendDestination(const OpaqueDraw& draw) {
    const auto& [a, b, c] = draw.vertices;
    const auto left = std::min({a.screenX, b.screenX, c.screenX}) - draw.material.sampleOffsetX;
    const auto right = std::max({a.screenX, b.screenX, c.screenX}) - draw.material.sampleOffsetX;
    const auto top = std::min({a.screenY, b.screenY, c.screenY}) - draw.material.sampleOffsetY;
    const auto bottom = std::max({a.screenY, b.screenY, c.screenY}) - draw.material.sampleOffsetY;
    const auto bound = [](double value, int limit) {
      return static_cast<GLint>(std::clamp(value, 0.0, double(limit)));
    };
    const GLint x0 = bound(std::floor(left) - 1, width_);
    const GLint x1 = bound(std::ceil(right) + 2, width_);
    const GLint y0 = height_ - bound(std::ceil(bottom) + 2, height_);
    const GLint y1 = height_ - bound(std::floor(top) - 1, height_);
    if (x0 == x1 || y0 == y1) return false;
    gl_.ReadBuffer(GL_COLOR_ATTACHMENT0);
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, colourScratch_.framebuffer);
    gl_.BlitFramebuffer(x0, y0, x1, y1, x0, y0, x1, y1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    if (trackHighlights_) {
      gl_.ReadBuffer(kSceneHighlightAttachment);
      gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, highlightScratch_.framebuffer);
      gl_.BlitFramebuffer(x0, y0, x1, y1, x0, y0, x1, y1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
      gl_.ReadBuffer(GL_COLOR_ATTACHMENT0);
    }
    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, target_);
    return true;
  }

  void drawTriangles(std::size_t first, std::size_t count) {
    gl_.Uniform1i(uniforms_.first, static_cast<GLint>(first));
    gl_.DrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, static_cast<GLsizei>(count));
  }

  void drawSky() {
    // The shader reads completed colour and clear-depth codes from the scene
    // target. Its draw framebuffer attaches only the separate sky result.
    gl_.ActiveTexture(GL_TEXTURE3);
    gl_.BindTexture(GL_TEXTURE_2D, 0);
    gl_.BindFramebuffer(GL_FRAMEBUFFER, colourScratch_.framebuffer);
    gl_.Disable(GL_DEPTH_TEST);
    gl_.Disable(GL_BLEND);
#ifndef __EMSCRIPTEN__
    gl_.ColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
    gl_.UseProgram(skyProgram_);
    gl_.ActiveTexture(GL_TEXTURE0);
    gl_.BindTexture(GL_TEXTURE_2D, attachments_[0]);
    gl_.ActiveTexture(GL_TEXTURE1);
    gl_.BindTexture(GL_TEXTURE_2D, attachments_[2]);
    gl_.Uniform1i(skyUniforms_.colour, 0);
    gl_.Uniform1i(skyUniforms_.depth, 1);
    gl_.Uniform4i(skyUniforms_.fill, kBlueSkyRgb[0], kBlueSkyRgb[1], kBlueSkyRgb[2], 255);
    gl_.DrawArrays(GL_TRIANGLES, 0, 3);

    gl_.BindFramebuffer(GL_DRAW_FRAMEBUFFER, target_);
    constexpr GLenum colourOnly = GL_COLOR_ATTACHMENT0;
    gl_.DrawBuffers(1, &colourOnly);
    blitColour();
    gl_.BindFramebuffer(GL_READ_FRAMEBUFFER, target_);
    selectDesktopSceneBuffers();
    enableSceneOutputs(true, true, trackHighlights_);
    gl_.Enable(GL_DEPTH_TEST);
#ifndef __EMSCRIPTEN__
    gl_.ColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
#endif
    gl_.UseProgram(program_);
    gl_.BindTexture(GL_TEXTURE_2D, 0);
    gl_.ActiveTexture(GL_TEXTURE3);
    gl_.BindTexture(GL_TEXTURE_2D, colourScratch_.texture);
    gl_.ActiveTexture(GL_TEXTURE0);
  }

  std::vector<std::uint32_t> readEncodedDepth(GLenum attachment) {
    std::vector<std::uint32_t> encoded(std::size_t(width_) * height_);
#ifdef __EMSCRIPTEN__
    const auto words = readWords(attachment);
    std::ranges::transform(words, encoded.begin(), [](const PixelWords& pixel) { return pixel[0]; });
#else
    gl_.ReadBuffer(attachment);
    gl_.ReadPixels(0, 0, width_, height_, GL_RED_INTEGER, GL_UNSIGNED_INT, encoded.data());
#endif
    return encoded;
  }

  std::vector<float> decodeDepthRows(std::vector<std::uint32_t>& encoded) {
    flipRows(encoded, width_, height_);
    std::vector<float> depth;
    depth.reserve(encoded.size());
    for (const auto bits : encoded) depth.push_back(std::bit_cast<float>(bits));
    return depth;
  }

  bool current() const {
    return context_.current();
  }

  std::expected<void, std::string> status(const char* operation) const {
    const auto error = gl_.GetError();
    if (error != GL_NO_ERROR) return std::unexpected(std::format("{}: GL error 0x{:x}", operation, error));
    return {};
  }

  Words encode(double value) const {
    if (!fp64_) return {std::bit_cast<std::uint32_t>(static_cast<float>(value)), 0};
    const auto bits = std::bit_cast<std::uint64_t>(value);
    return {static_cast<std::uint32_t>(bits), static_cast<std::uint32_t>(bits >> 32)};
  }

  double decode(Words words) const {
    if (!fp64_) return std::bit_cast<float>(words[0]);
    return std::bit_cast<double>(std::uint64_t(words[0]) | (std::uint64_t(words[1]) << 32));
  }

  void packMesh(std::span<const OpaqueDraw> draws) {
    // Only identical complete corner records share an index. UV seams,
    // flat-face colour and scene comparison bias can split a position.
    std::map<MeshVertex, std::uint32_t> corners;
    triangleIndices_.clear();
    vertices_.clear();
    for (const auto& draw : draws) {
      auto& indices = triangleIndices_.emplace_back(MeshIndices{});
      std::size_t corner = 0;
      for (const auto& v : draw.vertices) {
        MeshVertex words;
        auto out = words.begin();
        for (double value : {v.screenX, v.screenY, v.screenZ, v.rhw, v.u, v.v,
                             v.diffuse[0], v.diffuse[1], v.diffuse[2], v.diffuse[3]})
          *out++ = encode(value);
        const auto [index, added] = corners.try_emplace(words, static_cast<std::uint32_t>(vertices_.size()));
        if (added) vertices_.push_back(words);
        indices[corner++] = index->second;
      }
#ifdef __EMSCRIPTEN__
      const auto& [a, b, c] = draw.vertices;
      const double bx = b.screenX - a.screenX, by = b.screenY - a.screenY, bz = b.screenZ - a.screenZ;
      const double cx = c.screenX - a.screenX, cy = c.screenY - a.screenY, cz = c.screenZ - a.screenZ;
      const double area = bx * cy - by * cx;
      if (std::abs(area) >= 1e-12) {
        const double scale = meshDepthScale_ / area;
        indices[4] = encode((a.screenZ - meshDepthMinimum_) * meshDepthScale_)[0];
        indices[5] = encode((bz * cy - cz * by) * scale)[0];
        indices[6] = encode((bx * cz - cx * bz) * scale)[0];
        indices[7] = 1;
      }
#endif
    }
  }

  template<class T>
  std::size_t sync(BufferTexture<T>& resident, std::vector<T>& contents) {
    // At a fixed pose, upload changed terrain blocks and moving nodes. ES
    // index records also carry the depth-plane coefficients for the pose.
    if (!resident.texture) {
#ifndef __EMSCRIPTEN__
      gl_.GenBuffers(1, &resident.buffer);
#endif
      gl_.GenTextures(1, &resident.texture);
    }
    gl_.BindTexture(kOperandTarget, resident.texture);
    if (resident.contents == contents) return 0;
    const auto bytes = contents.size() * sizeof(T);
#ifdef __EMSCRIPTEN__
    static_assert(sizeof(T) % sizeof(PixelWords) == 0);
    constexpr int rowWidth = 256; // Must agree with bufferWord in kGlReal.
    const auto texels = bytes / sizeof(PixelWords);
    const int fullRows = static_cast<int>(texels / rowWidth), tail = static_cast<int>(texels % rowWidth);
    const int height = fullRows + (tail != 0);
    if (resident.capacity < static_cast<std::size_t>(height)) {
      resident.capacity = static_cast<std::size_t>(height);
      textureImage(resident.texture, rowWidth, height, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT);
    }
    if (fullRows)
      gl_.TexSubImage2D(kOperandTarget, 0, 0, 0, rowWidth, fullRows, GL_RGBA_INTEGER, GL_UNSIGNED_INT,
                        contents.data());
    if (tail) {
      const auto data = std::as_bytes(std::span(contents));
      gl_.TexSubImage2D(kOperandTarget, 0, 0, fullRows, tail, 1, GL_RGBA_INTEGER, GL_UNSIGNED_INT,
                        data.data() + std::size_t(fullRows) * rowWidth * sizeof(PixelWords));
    }
#else
    gl_.BindBuffer(GL_TEXTURE_BUFFER, resident.buffer);
    if (resident.capacity < bytes) {
      resident.capacity = bytes;
      gl_.BufferData(GL_TEXTURE_BUFFER, bytes, contents.data(), GL_DYNAMIC_DRAW);
      gl_.TexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, resident.buffer);
    } else {
      gl_.BufferSubData(GL_TEXTURE_BUFFER, 0, bytes, contents.data());
    }
#endif
    resident.contents.swap(contents);
    return bytes;
  }

  template<class T>
  void release(const BufferTexture<T>& resident) {
    if (resident.texture) gl_.DeleteTextures(1, &resident.texture);
    if (resident.buffer) gl_.DeleteBuffers(1, &resident.buffer);
  }

  enum class MeshOutput { Scene, Pipes, Brick, Particle };

  void bindMaterial(const OpaqueDraw& draw, MeshOutput target) {
    const auto key = draw.tube ? static_cast<const void*>(draw.tube->texture) : draw.backdrop;
    auto [texture, created] = surfaces_.try_emplace(key, 0);
    if (created) {
      gl_.GenTextures(1, &texture->second);
      gl_.BindTexture(GL_TEXTURE_2D, texture->second);
      gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      gl_.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      gl_.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
      if (draw.tube) {
        const auto& t = *draw.tube->texture;
        gl_.TexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, t.width, t.height, 0, GL_RGB,
                       GL_UNSIGNED_BYTE, t.rgb.data());
      } else {
        const auto& t = *draw.backdrop;
        gl_.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, t.width, t.height, 0, GL_RGBA,
                       GL_UNSIGNED_BYTE, t.rgba.data());
      }
    } else {
      gl_.BindTexture(GL_TEXTURE_2D, texture->second);
    }
    const auto& state = draw.material;
    constexpr std::array<GLenum, 8> depthFunctions{
        GL_NEVER, GL_LESS, GL_LEQUAL, GL_EQUAL, GL_GEQUAL, GL_GREATER, GL_NOTEQUAL, GL_ALWAYS};
    gl_.DepthFunc(depthFunctions[static_cast<std::size_t>(state.depthFunction)]);
    gl_.DepthMask(state.depthWrite);
    const bool pipeWrite = target == MeshOutput::Pipes && state.depthWrite && !draw.sprite;
    if (target == MeshOutput::Scene || target == MeshOutput::Pipes)
      enableSceneOutputs(state.depthWrite, pipeWrite, trackHighlights_);
    gl_.Uniform2i(uniforms_.blend, state.blendEnabled ? static_cast<int>(state.sourceBlend) : -1,
                  static_cast<int>(state.destinationBlend));
    gl_.Uniform4i(uniforms_.sampling, state.filter == SceneFilter::Point,
                  state.address == SceneAddress::Wrap, bool(draw.tube), state.shade == SceneShade::Gouraud);
    gl_.Uniform2i(uniforms_.alpha, state.alphaOperation == SceneAlphaOperation::SelectTexture,
                  state.alphaFunction == SceneAlphaFunction::StrictGreater ? state.alphaReference : -1);
    gl_.Uniform2f(uniforms_.offset, static_cast<float>(state.sampleOffsetX),
                  static_cast<float>(state.sampleOffsetY));
    gl_.Uniform4ui(uniforms_.highlight, packSceneHighlight(state.highlight), state.highlightTextureRgb,
                    trackHighlights_, 0);
  }

  struct Uniforms {
    GLint vertices, indices, surface, destination, viewport, first, owner, interval, sampling, alpha, offset, blend;
    GLint effectDepth, effectActor, effectMode, particleOwner, priorHighlight, highlight;
  } uniforms_{};
  struct SkyUniforms { GLint colour, depth, fill; } skyUniforms_{};
  struct ActorUniforms {
    GLint viewport, ringCount, camera, plane, placement, uv, frame, state, tint, effect, patchFluidDepth,
          trackHighlights;
#ifdef __EMSCRIPTEN__
    GLint visibility = -1, requireVisibility = -1;
    GLint footprintCentre = -1, footprintOrigin = -1, footprintDx = -1, footprintDy = -1;
#endif
  } actorUniforms_{};
  struct ReduceUniforms { GLint viewport, range, rows; } reduceUniforms_{};
  struct Attachment { GLenum point; GLint internal; GLenum layout, type; };
  CurrentGlContext context_;
  MeshGlFunctions gl_;
  GLuint program_ = 0, vao_ = 0, indices_ = 0, target_ = 0, skyProgram_ = 0;
  std::array<GLuint, 5> attachments_{};
  CopyTarget beforeFluid_, afterFluid_, colourScratch_, highlightScratch_;
  GLuint actorProgram_ = 0, hatchProgram_ = 0, reduceProgram_ = 0;
  GLuint actorTarget_ = 0, actorData_ = 0, brickTarget_ = 0, streamAtlas_ = 0, collision_ = 0;
  GLuint highlightTexture_ = 0;
#ifdef __EMSCRIPTEN__
  GLuint probeTarget_ = 0, emptyOperands_ = 0;
#else
  GLuint drowningVisible_ = 0;
#endif
  std::array<GLuint, 3> brickTextures_{};
  // Copies scenery for the mark pass, then prior actor coverage for sprites and particles.
  CopyTarget depthScratch_, hatchMask_, hatchRows_, hatchResult_;
  MeshBuffer brickMesh_, hatchMesh_, particleMesh_;
  BufferTexture<PixelWords> actorRings_;
  std::map<const Image*, GLuint> atlases_;
  std::shared_ptr<const ActorFootprint> uploadedFootprint_;
  double sceneMinimum_ = 0, sceneExtent_ = 0;
#ifdef __EMSCRIPTEN__
  double meshDepthMinimum_ = 0, meshDepthScale_ = 0;
#endif
  GLint priorProgram_ = 0, priorVao_ = 0, priorDrawFramebuffer_ = 0, priorReadFramebuffer_ = 0;
  int width_ = 0, height_ = 0;
  std::uint32_t nextOwner_ = 1;
  bool active_ = false, fp64_ = false, trackHighlights_ = false;
  std::string driver_, precision_;
  std::map<std::tuple<LevelScenePass, std::uint64_t, int, int, std::size_t, std::size_t>, MeshBuffer> meshes_;
  std::map<const void*, GLuint> surfaces_;
  std::vector<MeshVertex> vertices_;
  std::vector<MeshIndices> triangleIndices_;
};

}  // namespace

std::expected<std::unique_ptr<OpaqueGpu>, std::string> createOpaqueGpu() {
  auto gpu = std::make_unique<SceneGpu>();
  if (auto started = gpu->start(); !started) return std::unexpected(started.error());
  return gpu;
}

}  // namespace webo::app
