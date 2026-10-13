// A very small OpenGL shim.
//
// The project deliberately has no GL loader dependency, and only needs a
// couple of dozen entry points. On the web, GLES3 is linked directly and
// nothing has to be resolved; on the desktop the GL 2.0+ entry points are
// fetched through SDL_GL_GetProcAddress once at startup.
//
// Shaders are written against the common subset of GLSL 3.30 core and GLSL ES
// 3.00, which is why the version line is injected rather than hard-coded.

#pragma once

#if defined(__EMSCRIPTEN__)
#include <GLES3/gl3.h>
#else
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#endif

#include <string>

namespace webo::gfx {

#if !defined(__EMSCRIPTEN__)
// Resolved at startup. Named without the gl prefix so the call sites read
// normally once the macros below are in scope.
struct GlFunctions {
  PFNGLCREATESHADERPROC CreateShader = nullptr;
  PFNGLSHADERSOURCEPROC ShaderSource = nullptr;
  PFNGLCOMPILESHADERPROC CompileShader = nullptr;
  PFNGLGETSHADERIVPROC GetShaderiv = nullptr;
  PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog = nullptr;
  PFNGLDELETESHADERPROC DeleteShader = nullptr;
  PFNGLCREATEPROGRAMPROC CreateProgram = nullptr;
  PFNGLATTACHSHADERPROC AttachShader = nullptr;
  PFNGLLINKPROGRAMPROC LinkProgram = nullptr;
  PFNGLGETPROGRAMIVPROC GetProgramiv = nullptr;
  PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog = nullptr;
  PFNGLUSEPROGRAMPROC UseProgram = nullptr;
  PFNGLDELETEPROGRAMPROC DeleteProgram = nullptr;
  PFNGLGETUNIFORMLOCATIONPROC GetUniformLocation = nullptr;
  PFNGLUNIFORM1IPROC Uniform1i = nullptr;
  PFNGLUNIFORM2FPROC Uniform2f = nullptr;
  PFNGLUNIFORM4FPROC Uniform4f = nullptr;
  PFNGLUNIFORMMATRIX4FVPROC UniformMatrix4fv = nullptr;
  PFNGLGENVERTEXARRAYSPROC GenVertexArrays = nullptr;
  PFNGLBINDVERTEXARRAYPROC BindVertexArray = nullptr;
  PFNGLDELETEVERTEXARRAYSPROC DeleteVertexArrays = nullptr;
  PFNGLGENBUFFERSPROC GenBuffers = nullptr;
  PFNGLBINDBUFFERPROC BindBuffer = nullptr;
  PFNGLBUFFERDATAPROC BufferData = nullptr;
  PFNGLDELETEBUFFERSPROC DeleteBuffers = nullptr;
  PFNGLVERTEXATTRIBPOINTERPROC VertexAttribPointer = nullptr;
  PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray = nullptr;
  PFNGLACTIVETEXTUREPROC ActiveTexture = nullptr;
};

const GlFunctions& gl();
// Resolves everything. Returns a message naming the first missing entry point.
std::string loadGlFunctions();

#define WEBO_GL(name) ::webo::gfx::gl().name
#else
std::string loadGlFunctions();
#define WEBO_GL(name) gl##name
#endif

// The right #version line for whichever GL this build targets.
const char* glslVersionLine();

}  // namespace webo::gfx
