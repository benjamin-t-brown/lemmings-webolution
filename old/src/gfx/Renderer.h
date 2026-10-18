// Draws the simulation.
//
// Everything is textured quads in a single batch with one shader, which maps
// straight onto WebGL2.
//
// There are two views, and the same batch feeds both: the flat unwrapped one,
// which is the easier thing to debug a simulation against, and the cylinder
// projection, which is how the game is actually played. The vertex shader
// chooses between them, so nothing above the renderer has to know which is
// active -- a quad at world (x, y) lands in the right place either way.

#pragma once

#include "CylinderView.h"
#include "Font.h"
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

  // Switches to the cylinder projection: world X becomes the angle around a
  // tube of the given radius, world Y becomes height along its axis. Pass a
  // radius of worldWidth / 2pi to keep texels square, which is what makes the
  // wrapped view look like the flat one rolled up rather than stretched.
  //
  // The view is the camera orbiting the tube; see CylinderView.h.
  void setCylinder(const CylinderView& view, float worldWidth, float radius);
  // Back to the flat unwrapped view.
  void setFlat();
  bool cylinderEnabled() const { return cylinder_; }

  // Uploads a terrain mask as a texture. Call when the terrain changes.
  void uploadTerrain(const sim::Terrain& terrain);
  // Draws the terrain once per horizontal repeat needed to cover the view,
  // which is what makes the cylinder's seam invisible.
  void drawTerrain();

  void drawQuad(float x, float y, float w, float h, Colour c);

  // Pixel-space UI: setFlat + camera so one unit is one viewport pixel.
  void beginScreenUi(int viewportWidth, int viewportHeight);

  // UTF-8 text using a Font atlas previously uploaded via uploadImage.
  void drawText(const Font& font, unsigned fontTex, float x, float y, float scale,
                std::string_view utf8, Colour c = {1, 1, 1, 1});

  // Uploads an RGBA image and returns a texture name for drawSprite. The
  // renderer owns it and deletes it on destruction.
  unsigned uploadImage(const Image& image);

  // Draws a sub-rectangle of a sheet, the rectangle given in texels. Mirrors
  // horizontally when asked, which is how one facing serves for both -- the
  // sheets only store lemmings walking one way.
  void drawSprite(unsigned texture, float x, float y, float w, float h, int sheetWidth,
                  int sheetHeight, int sx, int sy, int sw, int sh, bool mirror = false,
                  Colour tint = {});

  // A textured quad subdivided horizontally, so it can follow the tube's
  // curve. In the flat view the subdivision is harmless: the pieces abut.
  void drawCylinderPatch(unsigned texture, float x, float y, float w, float h, int segments);

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
  int uViewProj_ = -1;
  int uCyl_ = -1;

  bool cylinder_ = false;
  float cylRadius_ = 0;
  float cylWorldWidth_ = 0;
  Mat4 viewProj_ = identity();

  int terrainW_ = 0;
  int terrainH_ = 0;

  float camX_ = 0, camY_ = 0, camWidth_ = 320;
  int viewW_ = 1, viewH_ = 1;

  unsigned boundTex_ = 0;
  std::vector<Vertex> batch_;
  int drawCalls_ = 0;
};

}  // namespace webo::gfx
