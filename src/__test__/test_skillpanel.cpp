// C1b: the compositing primitive, offline, plus the negative archive finding
// this round reached (see build/tmp/c1b-panel-icon-render/round-note.md).
//
// Two independent halves:
//   1. compositeIconRegion's own blit/clip/alpha-skip math -- fully offline,
//      synthetic fixtures, no archive and no WEBOLUTION_GAME_DIR needed.
//   2. A real-archive check (gated the same way test_frontend.cpp's
//      testFrontendAssetsLoad is) that GreyIcons/BlackIcons/AdditiveIcons --
//      the three sprite-sheet names LESSONS.md names as unchased -- resolve,
//      through the already-tested production webo::loadSpriteSheet reader,
//      to exactly one sprite each ("bomb", "questionmark", "armageddon")
//      rather than ten 72x32 panel-index-ordered icons. This reproduces,
//      with the production reader rather than a scratch parser, the finding
//      this round used to decide not to wire a guessed render.

#include "app/SkillPanel.h"
#include "assets/Box.h"
#include "assets/Sprite.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
using webo::Image;
using webo::app::compositeIconRegion;
using webo::app::SkillIconPlacement;

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string_view what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

Image makeSolid(int width, int height, std::uint8_t r, std::uint8_t g, std::uint8_t b,
                 std::uint8_t a) {
  Image img;
  img.width = width;
  img.height = height;
  img.rgba.resize(static_cast<std::size_t>(width) * height * 4);
  for (std::size_t i = 0; i < img.pixelCount(); ++i) {
    img.rgba[i * 4] = r;
    img.rgba[i * 4 + 1] = g;
    img.rgba[i * 4 + 2] = b;
    img.rgba[i * 4 + 3] = a;
  }
  return img;
}

void testOpaqueCopy() {
  std::cout << "opaque copy\n";
  Image dest = makeSolid(10, 10, 10, 20, 30, 255);
  Image source = makeSolid(4, 4, 200, 150, 100, 255);
  SkillIconPlacement placement{/*destX=*/3, /*destY=*/2, /*srcX=*/0, /*srcY=*/0,
                               /*width=*/4, /*height=*/4};
  const std::size_t written = compositeIconRegion(dest, source, placement);
  check(written == 16, "opaque: writes the full 4x4 region");
  // Inside the placed rectangle: replaced.
  check(dest.rgba[(2 * 10 + 3) * 4] == 200, "opaque: top-left corner pixel replaced (R)");
  check(dest.rgba[(2 * 10 + 3) * 4 + 1] == 150, "opaque: top-left corner pixel replaced (G)");
  check(dest.rgba[(5 * 10 + 6) * 4] == 200, "opaque: bottom-right corner pixel replaced");
  // Outside the placed rectangle: untouched.
  check(dest.rgba[(0 * 10 + 0) * 4] == 10, "opaque: pixel outside the rectangle is untouched");
  check(dest.rgba[(6 * 10 + 3) * 4] == 10, "opaque: row just past the rectangle is untouched");
}

void testFullyTransparentSkipsWrite() {
  std::cout << "fully transparent source pixel\n";
  Image dest = makeSolid(4, 4, 1, 2, 3, 255);
  Image source = makeSolid(4, 4, 200, 150, 100, 0);
  const std::size_t written = compositeIconRegion(dest, source, {0, 0, 0, 0, 4, 4});
  check(written == 16, "transparent: still reports every clipped pixel as visited");
  check(dest.rgba[0] == 1 && dest.rgba[1] == 2 && dest.rgba[2] == 3,
        "transparent: alpha-0 source pixel leaves dest colour unchanged");
}

void testPartialAlphaBlend() {
  std::cout << "partial alpha blend\n";
  Image dest = makeSolid(2, 2, 0, 0, 0, 255);
  Image source = makeSolid(2, 2, 255, 255, 255, 128);  // ~50% alpha
  compositeIconRegion(dest, source, {0, 0, 0, 0, 2, 2});
  // (255*128 + 0*127) / 255 = 128 (integer division), not a guess at a
  // specific blend formula elsewhere -- this is the formula this file's own
  // header documents.
  check(dest.rgba[0] == 128, std::format("partial blend: R channel is 128, was {}", dest.rgba[0]));
}

void testClipsDestinationEdge() {
  std::cout << "clipping: placement overhangs the destination\n";
  Image dest = makeSolid(8, 8, 9, 9, 9, 255);
  Image source = makeSolid(5, 5, 50, 60, 70, 255);
  // destX=6/destY=6 with a 5x5 placement overhangs both the right and
  // bottom edges of an 8x8 dest: only dest x in [6,8), y in [6,8) exists.
  const std::size_t written =
      compositeIconRegion(dest, source, {/*destX=*/6, /*destY=*/6, /*srcX=*/0, /*srcY=*/0,
                                         /*width=*/5, /*height=*/5});
  check(written == 4,
        std::format("dest-edge clip: only the 2x2 overlap is written, got {}", written));
  check(dest.rgba[(6 * 8 + 6) * 4] == 50, "dest-edge clip: the surviving overlap pixel is replaced");
  check(dest.rgba[(5 * 8 + 5) * 4] == 9, "dest-edge clip: a pixel outside the overlap is untouched");
}

void testClipsSourceEdge() {
  std::cout << "clipping: source rectangle reads before the source's own origin\n";
  Image dest = makeSolid(8, 8, 9, 9, 9, 255);
  Image source = makeSolid(5, 5, 50, 60, 70, 255);
  // srcX=-2/srcY=-2 asks to read 2 pixels before the source's own left/top
  // edge; the placement shifts destX/destY in lock-step so the destination
  // position for the surviving (in-bounds) source pixels is unaffected.
  const std::size_t written =
      compositeIconRegion(dest, source, {/*destX=*/1, /*destY=*/1, /*srcX=*/-2, /*srcY=*/-2,
                                         /*width=*/5, /*height=*/5});
  // Clipped width/height: 5 - 2 = 3 on each axis, both fitting the 8x8 dest
  // from destX=3,destY=3, so a 3x3 region is written.
  check(written == 9, std::format("source-edge clip: a 3x3 region is written, got {}", written));
  check(dest.rgba[(2 * 8 + 2) * 4] == 9, "source-edge clip: before the shifted origin, untouched");
  check(dest.rgba[(3 * 8 + 3) * 4] == 50, "source-edge clip: the shifted origin pixel is replaced");
  check(dest.rgba[(5 * 8 + 5) * 4] == 50, "source-edge clip: the far corner of the 3x3 is replaced");
  check(dest.rgba[(6 * 8 + 6) * 4] == 9, "source-edge clip: just past the 3x3 region, untouched");
}

void testEmptyInputsAreNoOps() {
  std::cout << "empty inputs\n";
  Image dest;  // default-constructed, empty
  Image source = makeSolid(2, 2, 1, 1, 1, 255);
  check(compositeIconRegion(dest, source, {0, 0, 0, 0, 2, 2}) == 0, "empty dest: no-op");
  Image realDest = makeSolid(2, 2, 1, 1, 1, 255);
  Image emptySource;
  check(compositeIconRegion(realDest, emptySource, {0, 0, 0, 0, 2, 2}) == 0, "empty source: no-op");
}

// The real-archive negative finding. Gated like test_frontend.cpp.
void testSkillPanelIconSheetsAreNotTheTenIcons() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir) {
    std::cout << "skill panel icon sheets: SKIPPED (set WEBOLUTION_GAME_DIR to enable)\n";
    return;
  }
  std::cout << "skill panel icon sheets (real archive)\n";

  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box", fs::path(dir) / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  struct Expectation {
    const char* sheetFile;   // SPRITES\\*.SPR
    const char* spriteName;  // the single sprite name it decodes to
    std::size_t maxFrameW;   // every frame's width/height is at most this --
    std::size_t maxFrameH;   // nowhere near the measured 72x32 button cell.
  };
  const Expectation expectations[] = {
      {"SPRITES\\GREYICONS.SPR", "bomb", 18, 25},
      {"SPRITES\\BLACKICONS.SPR", "questionmark", 20, 19},
      {"SPRITES\\ADDITIVEICONS.SPR", "armageddon", 15, 18},
  };

  for (const auto& expectation : expectations) {
    check(lib->contains(expectation.sheetFile),
          std::format("{} present in the archive", expectation.sheetFile));
    auto sheet = webo::loadSpriteSheet(*lib, expectation.sheetFile);
    if (!sheet) {
      check(false, std::format("{}: {}", expectation.sheetFile, sheet.error()));
      continue;
    }
    std::cout << std::format("    {}: {} sprite(s)\n", expectation.sheetFile, sheet->sprites.size());
    check(sheet->sprites.size() == 1,
          std::format("{}: exactly one sprite (not ten)", expectation.sheetFile));
    const auto* sprite = sheet->find(expectation.spriteName);
    check(sprite != nullptr,
          std::format("{}: holds a sprite named '{}'", expectation.sheetFile, expectation.spriteName));
    if (sprite == nullptr)
      continue;
    std::size_t maxW = 0, maxH = 0;
    for (const auto& anim : sprite->animations)
      for (const auto& frame : anim.frames) {
        maxW = std::max<std::size_t>(maxW, frame.width);
        maxH = std::max<std::size_t>(maxH, frame.height);
      }
    std::cout << std::format("    {}: max frame {}x{} across {} frame(s)\n", expectation.sheetFile,
                             maxW, maxH, sprite->frameCount());
    check(maxW <= expectation.maxFrameW && maxH <= expectation.maxFrameH,
          std::format("{}: every frame stays within {}x{} (not the 72x32 button cell)",
                       expectation.sheetFile, expectation.maxFrameW, expectation.maxFrameH));
    // The falsified hypothesis this round tested: ten frames at the
    // panel_boxes cell size (A7p state.json / PLAN.md A7), in panel-index
    // order. None of the three sheets is that shape.
    check(sprite->frameCount() != 10,
          std::format("{}: frame count is not 10 (it is {})", expectation.sheetFile,
                       sprite->frameCount()));
  }
}

}  // namespace

int main() {
  testOpaqueCopy();
  testFullyTransparentSkipsWrite();
  testPartialAlphaBlend();
  testClipsDestinationEdge();
  testClipsSourceEdge();
  testEmptyInputsAreNoOps();
  testSkillPanelIconSheetsAreNotTheTenIcons();

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
