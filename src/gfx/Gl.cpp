#include "Gl.h"

#include <SDL2/SDL_video.h>

#include <format>

namespace webo::gfx {

const char* glslVersionLine() {
#if defined(__EMSCRIPTEN__)
  return "#version 300 es\nprecision mediump float;\n";
#else
  return "#version 330 core\n";
#endif
}

#if defined(__EMSCRIPTEN__)

std::string loadGlFunctions() { return {}; }

#else

namespace {
GlFunctions g_gl;
}

const GlFunctions& gl() { return g_gl; }

std::string loadGlFunctions() {
  std::string missing;

  const auto load = [&](auto& slot, const char* name) {
    slot = reinterpret_cast<std::remove_reference_t<decltype(slot)>>(SDL_GL_GetProcAddress(name));
    if (!slot && missing.empty())
      missing = name;
  };

  load(g_gl.CreateShader, "glCreateShader");
  load(g_gl.ShaderSource, "glShaderSource");
  load(g_gl.CompileShader, "glCompileShader");
  load(g_gl.GetShaderiv, "glGetShaderiv");
  load(g_gl.GetShaderInfoLog, "glGetShaderInfoLog");
  load(g_gl.DeleteShader, "glDeleteShader");
  load(g_gl.CreateProgram, "glCreateProgram");
  load(g_gl.AttachShader, "glAttachShader");
  load(g_gl.LinkProgram, "glLinkProgram");
  load(g_gl.GetProgramiv, "glGetProgramiv");
  load(g_gl.GetProgramInfoLog, "glGetProgramInfoLog");
  load(g_gl.UseProgram, "glUseProgram");
  load(g_gl.DeleteProgram, "glDeleteProgram");
  load(g_gl.GetUniformLocation, "glGetUniformLocation");
  load(g_gl.Uniform1i, "glUniform1i");
  load(g_gl.Uniform2f, "glUniform2f");
  load(g_gl.Uniform4f, "glUniform4f");
  load(g_gl.GenVertexArrays, "glGenVertexArrays");
  load(g_gl.BindVertexArray, "glBindVertexArray");
  load(g_gl.DeleteVertexArrays, "glDeleteVertexArrays");
  load(g_gl.GenBuffers, "glGenBuffers");
  load(g_gl.BindBuffer, "glBindBuffer");
  load(g_gl.BufferData, "glBufferData");
  load(g_gl.DeleteBuffers, "glDeleteBuffers");
  load(g_gl.VertexAttribPointer, "glVertexAttribPointer");
  load(g_gl.EnableVertexAttribArray, "glEnableVertexAttribArray");
  load(g_gl.ActiveTexture, "glActiveTexture");

  if (!missing.empty())
    return std::format("OpenGL entry point not available: {}", missing);
  return {};
}

#endif

}  // namespace webo::gfx
