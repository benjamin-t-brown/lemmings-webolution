#pragma once

#include <string>
#include <string_view>

namespace webo::app {

enum class ShaderDialect { DesktopCore, Embedded };

#ifdef __EMSCRIPTEN__
inline constexpr auto kSceneShaderDialect = ShaderDialect::Embedded;
#else
inline constexpr auto kSceneShaderDialect = ShaderDialect::DesktopCore;
#endif

inline std::string presentationShader(ShaderDialect dialect, std::string_view body) {
  std::string source = dialect == ShaderDialect::DesktopCore
      ? "#version 330 core\n"
      : "#version 300 es\n#define WEBGL2\nprecision highp float;\nprecision highp int;\n"
        "precision highp sampler2D;\nprecision highp usampler2D;\nprecision highp isampler2D;\n";
  source.append(body);
  return source;
}

inline constexpr std::string_view kPresentationVertex = R"glsl(
out vec2 uv;
void main() {
  vec2 corner = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
  gl_Position = vec4(corner * 2.0 - 1.0, 0.0, 1.0);
  // The first uploaded row is the top of the CPU image.
  uv = vec2(corner.x, 1.0 - corner.y);
}
)glsl";

inline constexpr std::string_view kPresentationFragment = R"glsl(
in vec2 uv;
uniform sampler2D sourceImage;
out vec4 colour;
void main() {
  colour = texture(sourceImage, uv);
}
)glsl";

}  // namespace webo::app
