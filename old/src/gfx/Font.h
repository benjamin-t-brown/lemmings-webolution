// Baked 8x8 bitmap font for the Phase 5 shell HUD and menus.
//
// No SDL_ttf (plain SDL2 only). Glyphs cover printable ASCII plus a few
// Latin-1 letters that show up in retail .LTX (German umlauts, etc.).
// The atlas is an RGBA Image; Renderer uploads it once.

#pragma once

#include "assets/Bitmap.h"

#include <cstdint>
#include <string_view>

namespace webo::gfx {

struct Font {
  static constexpr int kGlyph = 8;
  static constexpr int kCols = 16;
  static constexpr int kRows = 16;  // 256 cells; we fill ASCII + Latin-1

  Image atlas;
  int atlasW = 0;
  int atlasH = 0;

  // Builds the atlas. Safe to call once at startup.
  static Font make();

  // Advances for one UTF-8 code point; returns false if nothing to draw
  // (combining / unknown → advance a space-width gap and skip).
  bool glyphRect(std::uint32_t cp, int& sx, int& sy) const;

  int measure(std::string_view utf8, float scale = 2.0f) const;
};

}  // namespace webo::gfx
