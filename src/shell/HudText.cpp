#include "HudText.h"

#include <cstddef>
#include <cstdio>

namespace webo::shell {

std::string counterText(int value) { return std::to_string(value); }

std::string savedText(int saved) { return counterText(static_cast<std::int16_t>(saved)); }

std::string timerText(int secondsRemaining, int secondsPerMinute) {
  const int s = secondsRemaining < 0 ? 0 : secondsRemaining;
  const int minutes = s / secondsPerMinute, seconds = s % secondsPerMinute;
  char buf[32];
  std::snprintf(buf, sizeof buf, seconds < 10 ? "%02d:0%d" : "%02d:%02d", minutes, seconds);
  return buf;
}

Rgb savedColour(int saved, int target, const HudParams& p) {
  return static_cast<std::int16_t>(saved) < static_cast<std::int16_t>(target) ? p.white : p.atTarget;
}

std::vector<HudLine> layoutHud(const HudValues& v, const HudParams& p, std::span<const GlyphCell> cells) {
  std::vector<HudLine> lines;
  HudLine out{counterText(v.out), 0, p.outY, p.white};
  out.x = p.counterRightEnd - textWidth(out.text, cells);
  lines.push_back(out);
  HudLine saved{savedText(v.saved), 0, p.savedY, savedColour(v.saved, v.saveTarget, p)};
  saved.x = p.counterRightEnd - textWidth(saved.text, cells);
  lines.push_back(saved);
  lines.push_back({timerText(v.secondsRemaining, p.secondsPerMinute), p.timerX, p.timerY, p.white});
  return lines;
}

HudBox lineBox(const HudLine& line, std::span<const GlyphCell> cells) {
  return {line.x, line.y, line.x + textWidth(line.text, cells), line.y + kLineHeight};
}

void drawHud(webo::Image& target, const webo::Image& sheet, std::span<const HudLine> lines,
             std::span<const GlyphCell> cells) {
  for (const HudLine& line : lines) {
    int x = line.x;
    for (const char c : line.text) {
      const unsigned idx = (static_cast<unsigned char>(c) - kFirstChar) & 0xff;
      if (idx >= cells.size())
        continue;
      const GlyphCell& g = cells[idx];
      for (int dy = 0; dy < kLineHeight; ++dy) {
        for (int dx = 0; dx < g.width; ++dx) {
          const int sx = g.x + dx, sy = g.y + dy;
          if (sx >= sheet.width || sy >= sheet.height)
            continue;
          const int a = sheet.rgba[(static_cast<std::size_t>(sy) * sheet.width + sx) * 4 + 3];
          const int ox = x + dx, oy = line.y + dy;
          if (a == 0 || ox < 0 || oy < 0 || ox >= target.width || oy >= target.height)
            continue;
          std::uint8_t* px = target.rgba.data() + (static_cast<std::size_t>(oy) * target.width + ox) * 4;
          for (int k = 0; k < 3; ++k)
            px[k] = static_cast<std::uint8_t>((px[k] * (255 - a) + line.colour[k] * a + 127) / 255);
        }
      }
      x += g.width;
    }
  }
}

}  // namespace webo::shell
