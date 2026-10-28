#include "app/EntryStartCue.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string_view>
#include <vector>

using namespace webo::app;

namespace {
int failures = 0;
void check(bool condition, std::string_view message) {
  if (!condition) {
    ++failures;
    std::cerr << "  FAIL  " << message << '\n';
  }
}

webo::Image image(int width, int height, std::uint8_t r = 0, std::uint8_t g = 0, std::uint8_t b = 0,
                  std::uint8_t a = 255) {
  webo::Image result;
  result.width = width;
  result.height = height;
  result.rgba.resize(static_cast<std::size_t>(width) * height * 4);
  for (std::size_t i = 0; i < result.rgba.size(); i += 4) {
    result.rgba[i] = r;
    result.rgba[i + 1] = g;
    result.rgba[i + 2] = b;
    result.rgba[i + 3] = a;
  }
  return result;
}

void clockAndGeometry() {
  EntryStartCue mapEntry;
  check(mapEntry.seconds() == 3.5 && mapEntry.active() && !mapEntry.frame(), "map clock starts at 3.5 and is gated");
  mapEntry.advanceBeforeFrame(2.0, true);
  check(mapEntry.seconds() == 1.5 && mapEntry.frame() && mapEntry.frame()->rect == std::array<int, 4>{-2880, -580, 3520, 1020},
        "post-decrement sample at 1.5 has the native large rectangle");

  EntryStartCue retry(false);
  check(retry.seconds() == 1.5 && retry.frame() && retry.frame()->rect == std::array<int, 4>{-2880, -580, 3520, 1020},
        "no-intro retry starts immediately at 1.5");
  retry.advanceBeforeFrame(std::numeric_limits<double>::quiet_NaN(), true);
  retry.advanceBeforeFrame(-1.0, true);
  check(retry.seconds() == 1.5, "invalid frame deltas do not corrupt clock");
  retry.advanceBeforeFrame(0.5, false);
  check(retry.seconds() == 1.5 && retry.frame().has_value(), "pause freezes but does not suppress cue drawing");
  retry.advanceBeforeFrame(0.5, true);
  check(retry.seconds() == 1.0 && retry.frame() && retry.frame()->rect == std::array<int, 4>{-600, -10, 1240, 450},
        "playing resumes; sampled clock is already decremented");
  retry.advanceBeforeFrame(0.25, true);
  check(retry.frame() && retry.frame()->rect == std::array<int, 4>{-30, 133, 670, 307}, "fractional squared growth truncates toward zero");
  retry.advanceBeforeFrame(0.25, true);
  check(retry.frame() && retry.frame()->rect == std::array<int, 4>{160, 180, 480, 260}, "base rectangle at 0.5");
  retry.advanceBeforeFrame(0.5, true);
  check(retry.seconds() == 0.0 && retry.frame() && !retry.frame()->additive && retry.frame()->vertexAlpha == 255.0,
        "zero is still source-over with full requested vertex alpha");
  retry.advanceBeforeFrame(0.5, true);
  check(retry.seconds() == -0.5 && retry.frame() && retry.frame()->additive && retry.frame()->vertexAlpha == 127.5,
        "negative phase requests additive and countdown vertex alpha");
  retry.advanceBeforeFrame(0.5, true);
  check(retry.active() && retry.seconds() == -1.0 && retry.frame() && retry.frame()->vertexAlpha == 0.0,
        "strict expiry keeps the cue active at exactly -1");
  retry.advanceBeforeFrame(0.0001, true);
  check(!retry.active() && !retry.frame(), "cue retires only below -1");

  EntryStartCue gate(false);
  check(!gate.frame(false).has_value(), "missing level scene gates sampling");
  gate.advanceBeforeFrame(0.0, true);
  check(gate.frame().has_value(), "playing at boundary still has a frame");
}

void softwareRenderer() {
  const std::array<webo::shell::GlyphCell, 2> cells{{{0, 0, 1}, {1, 0, 2}}};
  webo::Image font = image(3, webo::shell::kLineHeight, 200, 80, 40, 128);
  webo::Image target = image(4, 1, 0, 0, 0, 255);
  EntryCueFrame cue{{0, 0, 4, 1}, false, 0.0};
  const auto rects = entryCueGlyphRects(" !", cue, cells);
  check(rects == std::vector<std::array<int, 4>>{{0, 0, 1, 1}, {1, 0, 4, 1}},
        "glyph rectangles use cumulative proportional edges, not summed rounded widths");
  drawEntryStartCue(target, font, " !", cue, cells);
  check(target.rgba[0] == 100 && target.rgba[1] == 40 && target.rgba[2] == 20,
        "normal cue source-over blends texture RGB using texture alpha");
  check(target.rgba[12] == 100, "the final fitted glyph edge reaches the destination right edge");

  webo::Image additive = image(4, 1, 250, 250, 250, 255);
  cue.additive = true;
  cue.vertexAlpha = 0.0;
  drawEntryStartCue(additive, font, " !", cue, cells);
  check(additive.rgba[0] == 255 && additive.rgba[1] == 255 && additive.rgba[2] == 255,
        "additive mode saturates and ignores requested vertex alpha under texture-alpha selection");
  auto clearFont = font;
  for (std::size_t i = 3; i < clearFont.rgba.size(); i += 4) clearFont.rgba[i] = 0;
  auto transparent = image(4, 1, 17, 19, 23);
  const auto unchanged = transparent.rgba;
  drawEntryStartCue(transparent, clearFont, " !", cue, cells);
  check(transparent.rgba == unchanged, "transparent glyph texels draw no filled panel");

  // The first glyph's one-pixel source cell is clipped against this huge
  // destination; work is bounded by the target's 2x2 visible extent.
  webo::Image clipped = image(2, 2);
  cue.rect = {-2880, -580, 3520, 1020};
  drawEntryStartCue(clipped, font, " ", cue, cells);
  check(clipped.rgba[0] != 0, "huge native geometry clips safely to target pixels");

  webo::Image emptyTarget;
  drawEntryStartCue(emptyTarget, font, " ", cue, cells);
  const auto beforeInvalid = clipped.rgba;
  drawEntryStartCue(clipped, font, "", cue, cells);
  webo::Image malformedFont;
  malformedFont.width = 100;
  malformedFont.height = 100;
  malformedFont.rgba = {255};
  drawEntryStartCue(clipped, malformedFont, " ", cue, cells);
  check(clipped.rgba == beforeInvalid && emptyTarget.rgba.empty(),
        "empty text, invalid texture storage, and zero-sized target are safe no-ops");
}
}  // namespace

int main() {
  clockAndGeometry();
  softwareRenderer();
  if (failures != 0) {
    std::cerr << failures << " check(s) failed\n";
    return 1;
  }
  std::cout << "all entry start cue checks passed\n";
  return 0;
}
