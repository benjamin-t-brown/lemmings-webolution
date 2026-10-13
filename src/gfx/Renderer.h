// Draws the simulation.
//
// Everything is textured quads in a single batch with one shader, which is all
// the 2D view needs and maps straight onto WebGL2. The cylinder comes later;
// this is the flat, unwrapped view the plan calls for first, and it is what
// makes the simulation visible while its rules are still being pinned down.

#pragma once

#include "assets/Bitmap.h"
#include "sim/World.h"

#include <cstdint>
#include <expected>
#include <string>
#include <vector>

namespace webo::gfx {

struct Colour {
  float r = 1, g = 1, b = 1, a = 1;
};

class Renderer {
public:
  Renderer() = default;
  ~Renderer();
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  std::expected<void, std::string> init();

  void beginFrame(int viewportWidth, int viewportHeight, Colour clear);
  // Camera, in world pixels: what the top-left of the window shows, and how
  // many world pixels wide the window is.
  void setCamera(float originX, float originY, float worldWidth);

  // Uploads a terrain mask as a texture. Call when the terrain changes.
  void uploadTerrain(const sim::Terrain& terrain);
  // Draws the terrain once per horizontal repeat needed to cover the view,
  // which is what makes the cylinder's seam invisible.
  void drawTerrain();

  void drawQuad(float x, float y, float w, float h, Colour c);

  // Uploads an RGBA image and returns a texture name for drawSprite. The
  // renderer owns it and deletes it on destruction.
  unsigned uploadImage(const Image& image);

  // Draws a sub-rectangle of a sheet, the rectangle given in texels. Mirrors
  // horizontally when asked, which is how one facing serves for both -- the
  // sheets only store lemmings walking one way.
  void drawSprite(unsigned texture, float x, float y, float w, float h, int sheetWidth,
                  int sheetHeight, int sx, int sy, int sw, int sh, bool mirror = false,
                  Colour tint = {});

  void endFrame();

  int drawCalls() const { return drawCalls_; }

private:
  struct Vertex {
    float x, y, u, v, r, g, b, a;
  };

  void flush();
  void push(float x, float y, float w, float h, float u0, float v0, float u1, float v1, Colour c);

  unsigned program_ = 0;
  unsigned vao_ = 0;
  unsigned vbo_ = 0;
  unsigned whiteTex_ = 0;
  unsigned terrainTex_ = 0;
  std::vector<unsigned> ownedTextures_;
  int uCamera_ = -1;
  int uViewport_ = -1;
  int uTexture_ = -1;

  int terrainW_ = 0;
  int terrainH_ = 0;

  float camX_ = 0, camY_ = 0, camWidth_ = 320;
  int viewW_ = 1, viewH_ = 1;

  unsigned boundTex_ = 0;
  std::vector<Vertex> batch_;
  int drawCalls_ = 0;
};

}  // namespace webo::gfx
