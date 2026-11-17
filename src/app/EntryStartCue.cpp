#include "EntryStartCue.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace webo::app {
namespace {

int truncInt(double value) {
  value = std::trunc(value);
  if (value <= static_cast<double>(std::numeric_limits<int>::min()))
    return std::numeric_limits<int>::min();
  if (value >= static_cast<double>(std::numeric_limits<int>::max()))
    return std::numeric_limits<int>::max();
  return static_cast<int>(value);
}

std::vector<const shell::GlyphCell*> glyphRun(std::string_view text, std::span<const shell::GlyphCell> cells) {
  std::vector<const shell::GlyphCell*> result;
  result.reserve(text.size());
  for (unsigned char ch : text) {
    if (ch < shell::kFirstChar)
      continue;
    const auto index = static_cast<std::size_t>(ch - shell::kFirstChar);
    if (index >= cells.size() || cells[index].width == 0)
      continue;
    result.push_back(&cells[index]);
  }
  return result;
}

std::size_t byteCount(const webo::Image& image) {
  if (image.width <= 0 || image.height <= 0)
    return 0;
  const auto w = static_cast<std::size_t>(image.width);
  const auto h = static_cast<std::size_t>(image.height);
  if (w > std::numeric_limits<std::size_t>::max() / h / 4)
    return 0;
  return w * h * 4;
}

std::uint8_t clampByte(int v) { return static_cast<std::uint8_t>(std::clamp(v, 0, 255)); }

}  // namespace

EntryStartCue::EntryStartCue(bool introEnabled) : seconds_(introEnabled ? 3.5 : 1.5) {}

void EntryStartCue::advanceBeforeFrame(double seconds, bool playing) {
  if (!active_ || !playing || !std::isfinite(seconds) || seconds < 0.0)
    return;
  seconds_ -= seconds;
  if (seconds_ < -1.0)
    active_ = false;
}

std::optional<EntryCueFrame> EntryStartCue::frame(bool levelScenePresent) const {
  if (!active_ || !levelScenePresent || seconds_ > 1.5)
    return std::nullopt;

  int left = 160, top = 180, right = 480, bottom = 260;
  if (seconds_ > 0.5) {
    const double growth = (seconds_ - 0.5) * (seconds_ - 0.5);
    const int dx = truncInt(3040.0 * growth);
    const int dy = truncInt(760.0 * growth);
    left = truncInt(static_cast<double>(left) - dx);
    top = truncInt(static_cast<double>(top) - dy);
    right = truncInt(static_cast<double>(right) + dx);
    bottom = truncInt(static_cast<double>(bottom) + dy);
  }
  const bool additive = seconds_ < 0.0;
  const double alpha = additive ? 255.0 + seconds_ * 255.0 : 255.0;
  return EntryCueFrame{{left, top, right, bottom}, additive, alpha};
}

std::vector<std::array<int, 4>> entryCueGlyphRects(std::string_view text, const EntryCueFrame& cue,
                                                    std::span<const shell::GlyphCell> cells) {
  std::vector<std::array<int, 4>> rects;
  const auto glyphs = glyphRun(text, cells);
  if (glyphs.empty())
    return rects;
  const auto& r = cue.rect;
  const double totalWidth = [&] {
    double sum = 0.0;
    for (const auto* glyph : glyphs)
      sum += glyph->width;
    return sum;
  }();
  const double destinationWidth = static_cast<double>(r[2]) - static_cast<double>(r[0]);
  if (!(totalWidth > 0.0) || !(destinationWidth > 0.0) || r[3] <= r[1])
    return rects;

  rects.reserve(glyphs.size());
  const double scale = destinationWidth / totalWidth;
  double edge = static_cast<double>(r[0]);
  for (const auto* glyph : glyphs) {
    const double next = edge + static_cast<double>(glyph->width) * scale;
    rects.push_back({truncInt(edge), r[1], truncInt(next), r[3]});
    edge = next;
  }
  return rects;
}

void drawEntryStartCue(webo::Image& target, const webo::Image& font, std::string_view text,
                       const EntryCueFrame& cue, std::span<const shell::GlyphCell> cells) {
  const std::size_t targetBytes = byteCount(target), fontBytes = byteCount(font);
  if (targetBytes == 0 || fontBytes == 0 || target.rgba.size() < targetBytes || font.rgba.size() < fontBytes)
    return;

  const auto glyphs = glyphRun(text, cells);
  if (glyphs.empty())
    return;
  const auto rects = entryCueGlyphRects(text, cue, cells);
  if (rects.size() != glyphs.size())
    return;

  for (std::size_t i = 0; i < glyphs.size(); ++i) {
    const auto& glyph = *glyphs[i];
    const auto& r = rects[i];
    const int x0 = std::max(0, r[0]), y0 = std::max(0, r[1]);
    const int x1 = std::min(target.width, r[2]), y1 = std::min(target.height, r[3]);
    const auto glyphWidth = static_cast<std::int64_t>(r[2]) - r[0];
    const auto glyphHeight = static_cast<std::int64_t>(r[3]) - r[1];
    if (x0 >= x1 || y0 >= y1 || glyphWidth <= 0 || glyphHeight <= 0)
      continue;

    for (int y = y0; y < y1; ++y) {
      const int sy = static_cast<int>(((static_cast<std::int64_t>(y) - r[1]) * shell::kLineHeight) / glyphHeight) + glyph.y;
      if (sy < 0 || sy >= font.height)
        continue;
      for (int x = x0; x < x1; ++x) {
        const int sx = static_cast<int>(((static_cast<std::int64_t>(x) - r[0]) * glyph.width) / glyphWidth) + glyph.x;
        if (sx < 0 || sx >= font.width)
          continue;
        const std::size_t source = (static_cast<std::size_t>(sy) * font.width + sx) * 4;
        const int a = font.rgba[source + 3];  // SELECTARG1(TEXTURE): vertexAlpha is intentionally ignored.
        if (a == 0)
          continue;
        const std::size_t dest = (static_cast<std::size_t>(y) * target.width + x) * 4;
        for (int channel = 0; channel < 3; ++channel) {
          const int src = font.rgba[source + channel];
          const int dst = target.rgba[dest + channel];
          const int out = cue.additive ? dst + (src * a + 127) / 255
                                       : (src * a + dst * (255 - a) + 127) / 255;
          target.rgba[dest + channel] = clampByte(out);
        }
        // Like the existing HUD compositor, preserve the target alpha plane.
        // This is an RGB presentation adapter, not an offscreen RGBA blend model.
      }
    }
  }
}

}  // namespace webo::app
