#include "app/SkillPanel.h"

#include "assets/Box.h"
#include "assets/LevelObjects.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <optional>

namespace webo::app {

std::size_t compositeIconRegion(Image& dest, const Image& source, const SkillIconPlacement& placement) {
  if (dest.empty() || source.empty() || placement.width <= 0 || placement.height <= 0)
    return 0;

  // Clip the source rectangle to the source image, and the destination
  // placement to the destination image, keeping both offsets in step so a
  // partially off-screen or partially off-sheet placement still copies the
  // overlapping region correctly rather than reading/writing out of bounds.
  int srcX0 = placement.srcX;
  int srcY0 = placement.srcY;
  int destX0 = placement.destX;
  int destY0 = placement.destY;
  int width = placement.width;
  int height = placement.height;

  if (srcX0 < 0) {
    width += srcX0;
    destX0 -= srcX0;
    srcX0 = 0;
  }
  if (srcY0 < 0) {
    height += srcY0;
    destY0 -= srcY0;
    srcY0 = 0;
  }
  if (destX0 < 0) {
    width += destX0;
    srcX0 -= destX0;
    destX0 = 0;
  }
  if (destY0 < 0) {
    height += destY0;
    srcY0 -= destY0;
    destY0 = 0;
  }
  width = std::min({width, source.width - srcX0, dest.width - destX0});
  height = std::min({height, source.height - srcY0, dest.height - destY0});
  if (width <= 0 || height <= 0)
    return 0;

  std::size_t written = 0;
  for (int row = 0; row < height; ++row) {
    const std::size_t srcRowBase = (static_cast<std::size_t>(srcY0 + row) * source.width + srcX0) * 4;
    const std::size_t destRowBase = (static_cast<std::size_t>(destY0 + row) * dest.width + destX0) * 4;
    for (int col = 0; col < width; ++col) {
      const std::uint8_t* s = source.rgba.data() + srcRowBase + static_cast<std::size_t>(col) * 4;
      std::uint8_t* d = dest.rgba.data() + destRowBase + static_cast<std::size_t>(col) * 4;
      const std::uint8_t a = s[3];
      if (a == 0) {
        // Fully transparent: dest keeps whatever it already had.
      } else if (a == 255) {
        d[0] = s[0];
        d[1] = s[1];
        d[2] = s[2];
        d[3] = 255;
      } else {
        // Linear blend, straight (non-premultiplied) alpha: the convention
        // decodeBmp/applyAlfAlpha already use (colour channels are the
        // decoded BMP's own values regardless of alpha).
        for (int c = 0; c < 3; ++c) {
          const int blended = (static_cast<int>(s[c]) * a + static_cast<int>(d[c]) * (255 - a)) / 255;
          d[c] = static_cast<std::uint8_t>(blended);
        }
        d[3] = static_cast<std::uint8_t>(std::max<int>(d[3], a));
      }
      ++written;
    }
  }
  return written;
}

SkillPanelRect skillPanelRect(const webo::SpriteFrame& f, double x, double y, int scaleDelta) {
  const double scale = scaleDelta == 0 ? 1.0 : scaleDelta < 0 ? 0.5 : 2.0;
  const double left = x - f.unknown0 * scale;
  // READ: unmirrored code preserves full dy subtraction; only the source
  // frame's vertical extent is scaled (0x47AA59..AA86).
  const double top = y - f.unknown1 - f.height * scale;
  return {left, top, left + f.width * scale, top + f.height * scale};
}

void drawSkillPanelIcon(Image& dest, const Image& sheet, const webo::SpriteFrame& f,
                        double x, double y, int scaleDelta) {
  if (dest.empty() || sheet.empty() || f.width == 0 || f.height == 0) return;
  const auto r = skillPanelRect(f, x, y, scaleDelta);
  const int x0 = std::max(0, static_cast<int>(std::ceil(r.left)));
  const int x1 = std::min(dest.width, static_cast<int>(std::ceil(r.right)));
  const int y0 = std::max(0, static_cast<int>(std::ceil(r.top)));
  const int y1 = std::min(dest.height, static_cast<int>(std::ceil(r.bottom)));
  for (int py = y0; py < y1; ++py) {
    const double v = (py - r.top) / (r.bottom - r.top);
    const int sy = std::clamp(static_cast<int>(std::floor((f.uTop + v * (f.uBottom - f.uTop)) * sheet.height)),
                              static_cast<int>(f.top), static_cast<int>(f.bottom) - 1);
    for (int px = x0; px < x1; ++px) {
      const double u = (px - r.left) / (r.right - r.left);
      const int sx = std::clamp(static_cast<int>(std::floor((f.uLeft + u * (f.uRight - f.uLeft)) * sheet.width)),
                                static_cast<int>(f.left), static_cast<int>(f.right) - 1);
      const auto* src = sheet.rgba.data() + (static_cast<std::size_t>(sy) * sheet.width + sx) * 4;
      auto* dst = dest.rgba.data() + (static_cast<std::size_t>(py) * dest.width + px) * 4;
      const int a = src[3];
      // This is project AlphaMode compositing, not a claim about unread D3D blending.
      for (int c = 0; c < 3; ++c)
        dst[c] = static_cast<std::uint8_t>((dst[c] * (255 - a) + src[c] * a + 127) / 255);
    }
  }
}

void drawSkillPanelCounts(Image& dest, const Image& font, const std::array<int, 9>& counts) {
  for (int i = 0; i < 9; ++i) {
    const int count = counts[static_cast<std::size_t>(i)];
    const char digits[2] = {static_cast<char>('0' + count / 10), static_cast<char>('0' + count % 10)};
    for (int d = 0; d < 2; ++d) for (int y = 0; y < 14; ++y) for (int x = 0; x < 6; ++x) {
      const int sx = (digits[d] - '0') * 6 + x, dx = 66 + d * 6 + x, dy = 20 + 40 * i + y;
      if (sx < 0 || sx >= font.width || y >= font.height || dx < 0 || dx >= dest.width || dy < 0 || dy >= dest.height) continue;
      const auto* src = font.rgba.data() + (static_cast<std::size_t>(y) * font.width + sx) * 4;
      auto* dst = dest.rgba.data() + (static_cast<std::size_t>(dy) * dest.width + dx) * 4;
      const int a = src[3];
      for (int c = 0; c < 3; ++c) dst[c] = static_cast<std::uint8_t>((dst[c] * (255 - a) + 255 * a + 127) / 255);
    }
  }
}

Image resizeSkillPlatePatch(const Image& source) {
  if (source.width != 72 || source.height != 32 || source.rgba.size() != 72u * 32u * 4u) return {};
  Image out; out.width = 128; out.height = 32; out.alpha = source.alpha;
  out.rgba.resize(128u * 32u * 4u);
  for (int y = 0; y < 32; ++y) for (int x = 0; x < 128; ++x) {
    // HYPOTHESIS, conditional C1bc p53-nearest successful stream: x87 p53
    // arithmetic, then binary32 store. Original entry FPU state is unobserved.
    const float fx = static_cast<float>(static_cast<double>(x) / 127.0 * 71.0);
    const int x0 = static_cast<int>(fx), x1 = std::min(x0 + 1, 71);
    const float tx = static_cast<float>(static_cast<double>(fx) - x0);
    const float gx = static_cast<float>(1.0 - static_cast<double>(tx));
    for (int c = 0; c < 4; ++c) {
      const auto at = [&](int xx) { return source.rgba[(static_cast<std::size_t>(y) * 72 + xx) * 4 + c]; };
      const double value = static_cast<double>(at(x0)) * gx + static_cast<double>(at(x1)) * tx;
      out.rgba[(static_cast<std::size_t>(y) * 128 + x) * 4 + c] = static_cast<std::uint8_t>(value);
    }
  }
  return out;
}

void drawSkillPanelPlates(Image& dest, const Image& plate) {
  for (int i = 0; i < 10; ++i) {
    const int py = 12 + 40 * i;
    for (int y = 0; y < 32 && py + y < dest.height; ++y) for (int x = 0; x < 72 && 18 + x < dest.width; ++x) {
      // HYPOTHESIS: destination pixel-center nearest across complete [0,1] UV.
      const int sx = std::min(127, (x * 128 + 36) / 72);
      const auto* s = plate.rgba.data() + (static_cast<std::size_t>(y) * 128 + sx) * 4;
      auto* d = dest.rgba.data() + (static_cast<std::size_t>(py + y) * dest.width + 18 + x) * 4;
      const unsigned a = s[3];
      for (int c = 0; c < 3; ++c) d[c] = static_cast<std::uint8_t>((s[c] * a + d[c] * (255 - a)) / 255);
    }
  }
}

std::expected<void, std::string> drawRetailSkillPanel(Image& dest, const AssetLibrary& library,
                                                       std::string_view levelId) {
  auto lvl = library.read("COMPILED\\" + std::string(levelId) + ".LVL");
  auto spriteBytes = library.read("SPRITES\\LEMMINGSHI.SPR");
  auto font = loadTexture(library, "TEXTURES\\_FONTSMALL.BMP");
  auto plateSource = loadTexture(library, "TEXTURES\\^TAB.BMP");
  if (!lvl || !spriteBytes || !font || !plateSource)
    return std::unexpected(!lvl ? lvl.error() : (!spriteBytes ? spriteBytes.error() : (!font ? font.error() : plateSource.error())));
  auto skills = findLevelSkills(*lvl);
  auto spriteSheet = parseSpriteSheet(*spriteBytes);
  if (!skills || !spriteSheet)
    return std::unexpected(!skills ? skills.error() : spriteSheet.error());

  // MEASURED, C1s: the ten 0x47A75D x/y arguments from four stable frames.
  constexpr int drawX[10] = {39,37,38,39,40,39,41,37,38,38};
  constexpr int drawY[10] = {40,92,121,158,197,238,279,317,364,410};
  // READ, C1l: animation sub-entry (+1CC) and selected frame (+1B8).
  constexpr int sub[10] = {4,0,0,0,15,0,21,1,0,6};
  constexpr int frameNo[10] = {2,4,0,4,2,2,2,4,0,0};
  // READ, C1g/C1l: skill action names and special sheet registrations.
  constexpr const char* names[10] = {"climb","float","bomb","block","build","bash","mine","dig","questionmark","armageddon"};
  constexpr const char* specialSpr[3] = {"SPRITES\\GREYICONS.SPR","SPRITES\\BLACKICONS.SPR","SPRITES\\ADDITIVEICONS.SPR"};

  std::array<int, 9> counts{};
  for (std::size_t i = 0; i < skills->counts.size(); ++i)
    counts[i] = static_cast<int>(skills->counts[i]); // READ: level .LVL trailer, findLevelSkills.
  counts[8] = 0; // UNREAD on MM_DIG_000; C1l MEASURED zero for this slot on MM_CLIMB_002.
  // HYPOTHESIS: ten unselected ^tab textures. Selection is not established;
  // C1bc stream is conditional, and no score-based state selection is made.
  const Image plate = resizeSkillPlatePatch(*plateSource);
  if (plate.empty()) return std::unexpected("skill-panel plate must be 72x32");
  drawSkillPanelPlates(dest, plate);
  drawSkillPanelCounts(dest, *font, counts); // READ, C1g: count quads precede icon quads.

  for (int i = 0; i < 10; ++i) {
    const Sprite* sprite = nullptr;
    const SpriteSheet* selectedSheet = &*spriteSheet;
    std::optional<SpriteSheet> special;
    if (i == 2 || i == 8 || i == 9) {
      auto raw = library.read(specialSpr[i == 2 ? 0 : i == 8 ? 1 : 2]);
      if (!raw) return std::unexpected(raw.error());
      auto parsed = parseSpriteSheet(*raw);
      if (!parsed) return std::unexpected(parsed.error());
      special.emplace(std::move(*parsed));
      selectedSheet = &*special;
    }
    sprite = selectedSheet->find(names[i]);
    if (!sprite || static_cast<std::size_t>(sub[i]) >= sprite->animations.size())
      return std::unexpected(std::format("skill-panel sprite/animation missing {}/{}", names[i], sub[i]));
    const auto& frames = sprite->animations[static_cast<std::size_t>(sub[i])].frames;
    const auto fi = static_cast<std::size_t>(frameNo[i]);
    if (fi >= frames.size())
      return std::unexpected(std::format("skill-panel frame missing {}/{}", names[i], fi));
    const auto& frame = frames[fi];
    // READ: existing production texture loader plus Sprite.h sheet-name mapping.
    auto texture = loadTexture(library, webo::sheetAssetName(frame.sheet));
    if (!texture)
      return std::unexpected(std::format("{}: {}", frame.sheet, texture.error()));
    // DERIVED from byte reads: seven generic icons use mode S=-1; indices 2/8/9 use S=0.
    drawSkillPanelIcon(dest, *texture, frame, drawX[i], drawY[i], (i == 2 || i == 8 || i == 9) ? 0 : -1);
  }
  return {};
}

}  // namespace webo::app
