#include "Renderer.h"

#include "Gl.h"

#include <cmath>
#include <format>
#include <utility>
#include <vector>

namespace webo::gfx {
namespace {

const char* kVertexBody = R"(
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec4 aColour;

uniform vec4 uCamera;     // originX, originY, worldWidth, worldHeight
uniform vec2 uViewport;   // pixels

out vec2 vUv;
out vec4 vColour;

void main() {
  vec2 rel = (aPos - uCamera.xy) / uCamera.zw;   // 0..1 across the view
  vec2 ndc = vec2(rel.x * 2.0 - 1.0, 1.0 - rel.y * 2.0);
  gl_Position = vec4(ndc, 0.0, 1.0);
  vUv = aUv;
  vColour = aColour;
}
)";

const char* kFragmentBody = R"(
in vec2 vUv;
in vec4 vColour;
uniform sampler2D uTexture;
out vec4 oColour;

void main() {
  vec4 t = texture(uTexture, vUv);
  oColour = t * vColour;
  if (oColour.a < 0.01) discard;
}
)";

std::expected<unsigned, std::string> compile(unsigned type, const std::string& source) {
  const unsigned id = WEBO_GL(CreateShader)(type);
  const char* src = source.c_str();
  WEBO_GL(ShaderSource)(id, 1, &src, nullptr);
  WEBO_GL(CompileShader)(id);

  int ok = 0;
  WEBO_GL(GetShaderiv)(id, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    char log[1024] = {};
    WEBO_GL(GetShaderInfoLog)(id, sizeof log, nullptr, log);
    WEBO_GL(DeleteShader)(id);
    return std::unexpected(std::format("shader failed to compile: {}", log));
  }
  return id;
}

unsigned makeSolidTexture(std::uint32_t rgba) {
  unsigned tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &rgba);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  return tex;
}

Colour materialColour(sim::Material m) {
  switch (m) {
    case sim::Material::Empty: return {0, 0, 0, 0};
    case sim::Material::Earth: return {0.78f, 0.67f, 0.44f, 1};
    case sim::Material::Rock: return {0.52f, 0.52f, 0.56f, 1};
    case sim::Material::Steel: return {0.75f, 0.78f, 0.84f, 1};
    case sim::Material::Built: return {0.90f, 0.45f, 0.25f, 1};
  }
  return {1, 0, 1, 1};
}

}  // namespace

Renderer::~Renderer() {
  if (program_)
    WEBO_GL(DeleteProgram)(program_);
  if (vbo_)
    WEBO_GL(DeleteBuffers)(1, &vbo_);
  if (vao_)
    WEBO_GL(DeleteVertexArrays)(1, &vao_);
  if (whiteTex_)
    glDeleteTextures(1, &whiteTex_);
  if (terrainTex_)
    glDeleteTextures(1, &terrainTex_);
  for (unsigned tex : ownedTextures_)
    glDeleteTextures(1, &tex);
}

std::expected<void, std::string> Renderer::init() {
  if (auto err = loadGlFunctions(); !err.empty())
    return std::unexpected(err);

  const std::string version = glslVersionLine();
  auto vs = compile(GL_VERTEX_SHADER, version + kVertexBody);
  if (!vs)
    return std::unexpected(vs.error());
  auto fs = compile(GL_FRAGMENT_SHADER, version + kFragmentBody);
  if (!fs)
    return std::unexpected(fs.error());

  program_ = WEBO_GL(CreateProgram)();
  WEBO_GL(AttachShader)(program_, *vs);
  WEBO_GL(AttachShader)(program_, *fs);
  WEBO_GL(LinkProgram)(program_);

  int ok = 0;
  WEBO_GL(GetProgramiv)(program_, GL_LINK_STATUS, &ok);
  if (!ok) {
    char log[1024] = {};
    WEBO_GL(GetProgramInfoLog)(program_, sizeof log, nullptr, log);
    return std::unexpected(std::format("program failed to link: {}", log));
  }
  WEBO_GL(DeleteShader)(*vs);
  WEBO_GL(DeleteShader)(*fs);

  uCamera_ = WEBO_GL(GetUniformLocation)(program_, "uCamera");
  uViewport_ = WEBO_GL(GetUniformLocation)(program_, "uViewport");
  uTexture_ = WEBO_GL(GetUniformLocation)(program_, "uTexture");

  WEBO_GL(GenVertexArrays)(1, &vao_);
  WEBO_GL(BindVertexArray)(vao_);
  WEBO_GL(GenBuffers)(1, &vbo_);
  WEBO_GL(BindBuffer)(GL_ARRAY_BUFFER, vbo_);

  const auto stride = static_cast<int>(sizeof(Vertex));
  WEBO_GL(VertexAttribPointer)(0, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
  WEBO_GL(EnableVertexAttribArray)(0);
  WEBO_GL(VertexAttribPointer)(1, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(8));
  WEBO_GL(EnableVertexAttribArray)(1);
  WEBO_GL(VertexAttribPointer)(2, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(16));
  WEBO_GL(EnableVertexAttribArray)(2);

  whiteTex_ = makeSolidTexture(0xFFFFFFFFu);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  return {};
}

void Renderer::uploadTerrain(const sim::Terrain& terrain) {
  terrainW_ = terrain.width();
  terrainH_ = terrain.height();
  if (terrainW_ <= 0 || terrainH_ <= 0)
    return;

  std::vector<std::uint8_t> rgba(static_cast<std::size_t>(terrainW_) * terrainH_ * 4);
  for (int y = 0; y < terrainH_; ++y)
    for (int x = 0; x < terrainW_; ++x) {
      const Colour c = materialColour(terrain.at(x, y));
      std::uint8_t* p = rgba.data() + (static_cast<std::size_t>(y) * terrainW_ + x) * 4;
      p[0] = static_cast<std::uint8_t>(c.r * 255);
      p[1] = static_cast<std::uint8_t>(c.g * 255);
      p[2] = static_cast<std::uint8_t>(c.b * 255);
      p[3] = static_cast<std::uint8_t>(c.a * 255);
    }

  if (!terrainTex_) {
    glGenTextures(1, &terrainTex_);
    glBindTexture(GL_TEXTURE_2D, terrainTex_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // X repeats because the level is a cylinder; Y clamps because it is not.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  glBindTexture(GL_TEXTURE_2D, terrainTex_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, terrainW_, terrainH_, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               rgba.data());
  boundTex_ = terrainTex_;
}

void Renderer::beginFrame(int viewportWidth, int viewportHeight, Colour clear) {
  viewW_ = viewportWidth > 0 ? viewportWidth : 1;
  viewH_ = viewportHeight > 0 ? viewportHeight : 1;
  glViewport(0, 0, viewW_, viewH_);
  glClearColor(clear.r, clear.g, clear.b, clear.a);
  glClear(GL_COLOR_BUFFER_BIT);
  drawCalls_ = 0;
  batch_.clear();
  boundTex_ = 0;
}

void Renderer::setCamera(float originX, float originY, float worldWidth) {
  camX_ = originX;
  camY_ = originY;
  camWidth_ = worldWidth > 1 ? worldWidth : 1;
}

void Renderer::push(float x, float y, float w, float h, float u0, float v0, float u1, float v1,
                    Colour c) {
  const Vertex a{x, y, u0, v0, c.r, c.g, c.b, c.a};
  const Vertex b{x + w, y, u1, v0, c.r, c.g, c.b, c.a};
  const Vertex d{x + w, y + h, u1, v1, c.r, c.g, c.b, c.a};
  const Vertex e{x, y + h, u0, v1, c.r, c.g, c.b, c.a};
  batch_.insert(batch_.end(), {a, b, d, a, d, e});
}

void Renderer::flush() {
  if (batch_.empty())
    return;
  const float worldHeight = camWidth_ * static_cast<float>(viewH_) / static_cast<float>(viewW_);

  WEBO_GL(UseProgram)(program_);
  WEBO_GL(Uniform4f)(uCamera_, camX_, camY_, camWidth_, worldHeight);
  WEBO_GL(Uniform2f)(uViewport_, static_cast<float>(viewW_), static_cast<float>(viewH_));
  WEBO_GL(Uniform1i)(uTexture_, 0);
  WEBO_GL(ActiveTexture)(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, boundTex_ ? boundTex_ : whiteTex_);

  WEBO_GL(BindVertexArray)(vao_);
  WEBO_GL(BindBuffer)(GL_ARRAY_BUFFER, vbo_);
  WEBO_GL(BufferData)(GL_ARRAY_BUFFER,
                      static_cast<long>(batch_.size() * sizeof(Vertex)), batch_.data(),
                      GL_STREAM_DRAW);
  glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(batch_.size()));
  ++drawCalls_;
  batch_.clear();
}

void Renderer::drawTerrain() {
  if (!terrainTex_ || terrainW_ <= 0)
    return;
  if (boundTex_ != terrainTex_) {
    flush();
    boundTex_ = terrainTex_;
  }

  // The level wraps, so draw it as many times as the view needs. GL_REPEAT on
  // S means one quad with extended U would do, which is exactly what this is:
  // the U range follows the camera.
  const float u0 = camX_ / static_cast<float>(terrainW_);
  const float u1 = (camX_ + camWidth_) / static_cast<float>(terrainW_);
  push(camX_, 0, camWidth_, static_cast<float>(terrainH_), u0, 0, u1, 1, {1, 1, 1, 1});
}

unsigned Renderer::uploadImage(const Image& image) {
  if (image.empty() || image.width <= 0 || image.height <= 0)
    return 0;
  unsigned tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               image.rgba.data());
  glBindTexture(GL_TEXTURE_2D, boundTex_);
  ownedTextures_.push_back(tex);
  return tex;
}

void Renderer::drawSprite(unsigned texture, float x, float y, float w, float h, int sheetWidth,
                          int sheetHeight, int sx, int sy, int sw, int sh, bool mirror,
                          Colour tint) {
  if (!texture || sheetWidth <= 0 || sheetHeight <= 0)
    return;
  if (boundTex_ != texture) {
    flush();
    boundTex_ = texture;
  }
  float u0 = static_cast<float>(sx) / static_cast<float>(sheetWidth);
  float u1 = static_cast<float>(sx + sw) / static_cast<float>(sheetWidth);
  const float v0 = static_cast<float>(sy) / static_cast<float>(sheetHeight);
  const float v1 = static_cast<float>(sy + sh) / static_cast<float>(sheetHeight);
  if (mirror)
    std::swap(u0, u1);
  push(x, y, w, h, u0, v0, u1, v1, tint);
}

void Renderer::drawQuad(float x, float y, float w, float h, Colour c) {
  if (boundTex_ != whiteTex_) {
    flush();
    boundTex_ = whiteTex_;
  }
  push(x, y, w, h, 0, 0, 1, 1, c);
}

void Renderer::endFrame() { flush(); }

}  // namespace webo::gfx
