#pragma once

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include <emscripten/html5_webgl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#include <expected>
#include <string>
#ifndef __EMSCRIPTEN__
#include <vector>
#endif

namespace webo::app {

// Share the context-owned renderer. Emscripten supplies the ES entry points
// directly, while desktop entry points come from the current SDL context.
#ifdef __EMSCRIPTEN__
#define WEBO_GL_ENTRY(name, type) (&::gl##name)
#else
#define WEBO_GL_ENTRY(name, type) reinterpret_cast<type>(SDL_GL_GetProcAddress("gl" #name))
#endif

class CurrentGlContext {
 public:
  bool ownsCurrent() const {
#ifdef __EMSCRIPTEN__
    return context_ > 0 && emscripten_webgl_get_current_context() == context_;
#else
    return context_ && SDL_GL_GetCurrentContext() == context_ && SDL_GL_GetCurrentWindow() == window_;
#endif
  }

  bool current() const {
#ifdef __EMSCRIPTEN__
    return ownsCurrent() && !emscripten_is_webgl_context_lost(context_);
#else
    return ownsCurrent();
#endif
  }

 private:
#ifdef __EMSCRIPTEN__
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_ = emscripten_webgl_get_current_context();
#else
  SDL_GLContext context_ = SDL_GL_GetCurrentContext();
  SDL_Window* window_ = SDL_GL_GetCurrentWindow();
#endif
};

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
    name = WEBO_GL_ENTRY(name, type); \
    if (!name) return std::unexpected("OpenGL entry point missing: gl" #name);
    PRESENTATION_GL_FUNCTIONS(LOAD_GL)
#undef LOAD_GL
    return {};
  }
};
#undef PRESENTATION_GL_FUNCTIONS

inline std::expected<GLuint, std::string> linkGlProgram(
    const GlFunctions& gl, const std::string& vertexSource, const std::string& fragmentSource) {
  const auto compile = [&](GLenum type, const char* name, const std::string& source)
      -> std::expected<GLuint, std::string> {
    const GLuint shader = gl.CreateShader(type);
    if (!shader) return std::unexpected(std::string("OpenGL could not create the ") + name + " shader");
    const auto* code = source.c_str();
    gl.ShaderSource(shader, 1, &code, nullptr);
    gl.CompileShader(shader);
    GLint compiled = GL_FALSE;
    gl.GetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_TRUE) return shader;
    char log[4096]{};
    gl.GetShaderInfoLog(shader, sizeof(log), nullptr, log);
    gl.DeleteShader(shader);
    return std::unexpected(std::string("OpenGL ") + name + " shader: " + log);
  };

  const auto vertex = compile(GL_VERTEX_SHADER, "vertex", vertexSource);
  if (!vertex) return std::unexpected(vertex.error());
  const auto fragment = compile(GL_FRAGMENT_SHADER, "fragment", fragmentSource);
  if (!fragment) {
    gl.DeleteShader(*vertex);
    return std::unexpected(fragment.error());
  }
  const auto program = gl.CreateProgram();
  if (program) {
    gl.AttachShader(program, *vertex);
    gl.AttachShader(program, *fragment);
    gl.LinkProgram(program);
  }
  gl.DeleteShader(*vertex);
  gl.DeleteShader(*fragment);
  if (!program) return std::unexpected("OpenGL could not create a program");
  GLint linked = GL_FALSE;
  gl.GetProgramiv(program, GL_LINK_STATUS, &linked);
  if (linked == GL_TRUE) return program;
  char log[4096]{};
  gl.GetProgramInfoLog(program, sizeof(log), nullptr, log);
  gl.DeleteProgram(program);
  return std::unexpected(std::string("OpenGL program: ") + log);
}

#ifndef __EMSCRIPTEN__
// Link each immutable shader once in the window's context. Scene legs retain
// their own framebuffer, textures and geometry, and set uniforms before use.
class GlProgramCache {
 public:
  explicit GlProgramCache(const GlFunctions& gl) : gl_(gl) {}
  ~GlProgramCache() {
    if (context_.ownsCurrent())
      for (const auto& entry : entries_) gl_.DeleteProgram(entry.program);
  }
  GlProgramCache(const GlProgramCache&) = delete;
  GlProgramCache& operator=(const GlProgramCache&) = delete;

  std::expected<GLuint, std::string> link(std::string vertex, std::string fragment) {
    for (const auto& entry : entries_)
      if (entry.vertex == vertex && entry.fragment == fragment) return entry.program;
    auto program = linkGlProgram(gl_, vertex, fragment);
    if (program) entries_.push_back({std::move(vertex), std::move(fragment), *program});
    return program;
  }

 private:
  struct Entry { std::string vertex, fragment; GLuint program; };
  const GlFunctions& gl_;
  CurrentGlContext context_;
  std::vector<Entry> entries_;
};
#endif

}  // namespace webo::app
