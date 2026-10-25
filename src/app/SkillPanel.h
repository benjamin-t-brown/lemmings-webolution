// C1b: compositing primitive for the ten skill-panel button icons.
//
// This round (c1b-panel-icon-render) located the three sprite-sheet names
// LESSONS.md already names as unchased ("Useful strings the engine keeps"):
// GreyIcons, BlackIcons, AdditiveIcons. Reading their real archive .SPR files
// (SPRITES\GREYICONS.SPR, SPRITES\BLACKICONS.SPR, SPRITES\ADDITIVEICONS.SPR)
// with the already-tested production Sprite.h reader shows each resolves to
// exactly ONE named sprite -- "bomb" (18x25, one frame), "questionmark"
// (~20x16, 16 near-duplicate frames) and "armageddon" (<=15x18, 13 tiny
// frames) -- not ten 72x32 panel-index-ordered icons. test_skillpanel.cpp's
// `skillPanelIconSheetsAreNotTheTenIcons` check reproduces this against the
// real archive. See build/tmp/c1b-panel-icon-render/round-note.md for the
// REA addresses (string refs, the shared registrar FUN_0045e51f at
// 0x45e51f-0x45ed80) and the full archive/PE-resource search this rules out.
//
// C1t adds an opt-in headless fixture path (`--skill-panel`) in main.cpp. It
// uses the measured C1s draw points, C1l frame records and C1r sheet pixels.
// It does not draw in windowed mode or claim all-level fidelity; unread blend
// and tint behavior remain explicit in the code and round note.

#pragma once

#include "assets/Bitmap.h"
#include "assets/Sprite.h"

#include <array>
#include <cstddef>
#include <expected>
#include <string>
#include <string_view>

namespace webo { class AssetLibrary; }

namespace webo::app {

// Where in `source` the icon lives, and where in `dest` to place it.
// Coordinates are plain pixel offsets, top-down, in each image's own space --
// not normalised, not perspective-projected (this is a 2D screen-space HUD
// overlay, drawn after the 3D tube frame, not a 3D quad).
struct SkillIconPlacement {
  int destX = 0;
  int destY = 0;
  int srcX = 0;
  int srcY = 0;
  int width = 0;
  int height = 0;
};

// Alpha-composites the `placement` sub-rectangle of `source` onto `dest` at
// (destX, destY), in dest's own pixel space. Both images are the RGBA8,
// top-down `webo::Image` (assets/Bitmap.h) that `webo::loadTexture` already
// produces, with alpha already resolved from the archive's own sigil
// (AlphaMode: colour-key, .ALF plane, embedded, or opaque) -- this function
// does not decide transparency, it only honours whatever alpha the source
// image's pixels already carry:
//
//   alpha == 0    source pixel is fully transparent: dest is left unchanged
//   alpha == 255  source pixel fully replaces dest
//   1..254        linearly blended
//
// Read, not assumed: Bitmap.cpp's own `applyColourKey` (KeyBlack '_' /
// KeyGrey '$') only ever writes alpha 0 or leaves the opaque default (255),
// so those two modes never produce an intermediate value. `applyAlfAlpha`
// (AlfFile '^') copies the sibling .ALF's bytes verbatim into alpha and can
// legitimately carry any 0..255 value, so the blended case is real, not
// hypothetical, for that one AlphaMode.
//
// Both `placement`'s source rectangle and its destination placement are
// clipped to each image's own bounds; nothing is read or written out of
// range in either image. Returns the number of pixels actually written
// (after clipping), 0 if the placement is entirely out of bounds or either
// image is empty.
std::size_t compositeIconRegion(Image& dest, const Image& source, const SkillIconPlacement& placement);

struct SkillPanelRect { double left = 0, top = 0, right = 0, bottom = 0; };
// READ from 0x47A75D: mirror=0 uses x-dx*scale; y-dy-height*scale,
// where scale=1 (mode S=0) or 0.5 (mode S=-1); C1h inputs and 0x47AA12/59.
SkillPanelRect skillPanelRect(const webo::SpriteFrame& frame, double x, double y, int scaleDelta);
// READ frame geometry/UV + MEASURED x/y (C1s); tex alpha resolution follows
// the project AlphaMode convention in Bitmap.cpp. Blend state/tint stay UNREAD.
void drawSkillPanelIcon(webo::Image& dest, const webo::Image& sheet,
                        const webo::SpriteFrame& frame, double x, double y, int scaleDelta);
// READ C1n/C1q: fixed two ASCII digits, glyphs 6x14, white vertex colour.
void drawSkillPanelCounts(webo::Image& dest, const webo::Image& font,
                          const std::array<int, 9>& counts);
// HYPOTHESIS: conditionally models C1bc successful normal-layout ^tab upload
// with p53-nearest arithmetic; not an observed or selected original FPU state.
// Endpoint bilinear 72x32 -> 128x32, full texture UV.
webo::Image resizeSkillPlatePatch(const webo::Image& source);
// HYPOTHESIS: linear sample of s = x * 128 / 72 (C1bi). Request 2 is the
// DERIVED LINEAR name. The nearest index is not the sample position.
void drawSkillPanelPlates(webo::Image& dest, const webo::Image& plate);
// Opt-in, headless C1 fixture renderer. All game data is read from the supplied
// archive and level trailer; the caller only wires this into --shot.
std::expected<void, std::string> drawRetailSkillPanel(webo::Image& dest,
                                                      const webo::AssetLibrary& library,
                                                      std::string_view levelId);

}  // namespace webo::app
