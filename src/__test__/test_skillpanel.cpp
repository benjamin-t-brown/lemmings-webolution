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
#include "assets/Bitmap.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <format>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
using webo::Image;
using webo::app::compositeIconRegion;
using webo::app::drawSkillPanelCounts;
using webo::app::resizeSkillPlatePatch;
using webo::app::drawSkillPanelPlates;
using webo::app::drawRetailSkillPanel;
using webo::app::skillPanelRect;

namespace webo::app {
void drawSkillPanelSelectedPlate(webo::Image& dest, const webo::Image& plate);
}
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

void testNativePanelPlacementAndDigit() {
  // READ/C1l geometry with the measured placement and byte-read scale branch.
  webo::SpriteFrame f;
  f.unknown0 = 8; f.unknown1 = 7; f.width = 23; f.height = 30;
  const auto half = skillPanelRect(f, 39, 40, -1);
  check(half.left == 35 && half.top == 18 && half.right == 46.5 && half.bottom == 33,
        "generic icon placement: dx and extent halve while dy stays full");
  f.unknown0 = 9; f.unknown1 = 0; f.width = 18; f.height = 25;
  const auto native = skillPanelRect(f, 38, 121, 0);
  check(native.left == 29 && native.top == 96 && native.right == 47 && native.bottom == 121,
        "special icon placement: native extents and offsets");

  Image dest = makeSolid(90, 400, 0, 0, 0, 255);
  Image font = makeSolid(64, 16, 0, 0, 0, 0);
  // A distinct ink pixel only in the '0' cell makes reverting either ASCII
  // digit fail without depending on the archived font's full pixel pattern.
  font.rgba[0] = 255;
  font.rgba[3] = 255;
  std::array<int, 9> counts{}; counts[0] = 20;
  drawSkillPanelCounts(dest, font, counts);
  check(dest.rgba[(20 * 90 + 66) * 4] == 0, "count text: twenty renders the tens digit '2'");
  check(dest.rgba[(20 * 90 + 72) * 4] == 255, "count text: twenty renders the ones digit '0'");
}

void testZeroCountSkipsTextAtDestination() {
  std::cout << "zero-count text draw guard\n";
  Image font = makeSolid(60, 14, 255, 255, 255, 255);
  Image zeroDest = makeSolid(90, 400, 17, 29, 41, 255);
  std::array<int, 9> counts{};
  counts[0] = 0;
  drawSkillPanelCounts(zeroDest, font, counts);
  bool unchanged = true;
  for (int y = 20; y < 34; ++y)
    for (int x = 66; x < 78; ++x) {
      const auto p = (static_cast<std::size_t>(y) * zeroDest.width + x) * 4;
      unchanged &= zeroDest.rgba[p] == 17 && zeroDest.rgba[p + 1] == 29 &&
                   zeroDest.rgba[p + 2] == 41;
    }
  check(unchanged, "count zero leaves ink-capable destination pixels unchanged");

  for (const int count : {1, 10}) {
    Image dest = makeSolid(90, 400, 17, 29, 41, 255);
    counts.fill(0);
    counts[0] = count;
    drawSkillPanelCounts(dest, font, counts);
    const auto p = (20u * dest.width + 66u) * 4;
    const auto ones = (20u * dest.width + 72u) * 4;
    check(dest.rgba[p] == 255, std::format("count {} draws tens-cell ink", count));
    check(dest.rgba[ones] == 255, std::format("count {} draws ones-cell ink", count));
  }
}

void testPatchPlateResizeRowAsymmetricAlpha() {
  Image src = makeSolid(72, 32, 0, 0, 0, 0);
  src.rgba[(0 * 72 + 0) * 4] = 11; src.rgba[(0 * 72 + 0) * 4 + 3] = 3;
  src.rgba[(31 * 72 + 0) * 4] = 77; src.rgba[(31 * 72 + 0) * 4 + 3] = 201;
  const auto out = resizeSkillPlatePatch(src);
  check(out.width == 128 && out.height == 32, "plate patch grows to 128x32");
  check(out.rgba[3] == 3, "top row alpha remains top row");
  check(out.rgba[(31u * 128u) * 4 + 3] == 201, "bottom row alpha remains bottom row");
  check(out.rgba[(31u * 128u) * 4] == 77, "bottom row RGB remains bottom row");
  Image dest = makeSolid(640,480,0,0,0,255); Image patch = makeSolid(128,32,200,0,0,128);
  drawSkillPanelPlates(dest,patch);
  check(dest.rgba[(12u*640u+18u)*4] == 100, "plate alpha composites into destination pixel");
  check(dest.rgba[(11u*640u+18u)*4] == 0, "plate compositor leaves pixel outside box untouched");

  // Source-derived C1bc discriminator: selected asset row 24 has ALF x6=170,
  // x7=43; at output x11 C1bc p53-nearest is 151 (p24-nearest is 150).
  Image discriminator = makeSolid(72,32,255,255,255,0);
  discriminator.rgba[(24u*72u+6u)*4+3] = 170;
  discriminator.rgba[(24u*72u+7u)*4+3] = 43;
  const auto interp = resizeSkillPlatePatch(discriminator);
  const auto got = interp.rgba[(24u*128u+11u)*4+3];
  check(got == 151, std::format("source-derived p53 interpolation x11,y24 is 151 (p24 is 150), got {}",got));
}

bool sameRgb(const Image& a, const Image& b, int x, int y) {
  const auto pa = (static_cast<std::size_t>(y) * a.width + x) * 4;
  const auto pb = (static_cast<std::size_t>(y) * b.width + x) * 4;
  return a.rgba[pa] == b.rgba[pb] && a.rgba[pa + 1] == b.rgba[pb + 1] && a.rgba[pa + 2] == b.rgba[pb + 2];
}

bool isBackground(const Image& img, int x, int y, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
  const auto p = (static_cast<std::size_t>(y) * img.width + x) * 4;
  return img.rgba[p] == r && img.rgba[p + 1] == g && img.rgba[p + 2] == b;
}

void testSelectedQuadDestination() {
  std::cout << "selected plate quad\n";
  constexpr std::uint8_t br = 17, bg = 29, bb = 41;
  Image omitted = makeSolid(640, 480, br, bg, bb, 255);
  Image drawn = omitted;
  Image patch = makeSolid(128, 32, 200, 10, 30, 128);
  webo::app::drawSkillPanelSelectedPlate(drawn, patch);

  bool insideChanged = false;
  bool omittedUnchanged = true;
  for (int y = 12; y < 44; ++y)
    for (int x = 18; x < 90; ++x) {
      insideChanged = insideChanged || !isBackground(drawn, x, y, br, bg, bb);
      omittedUnchanged = omittedUnchanged && isBackground(omitted, x, y, br, bg, bb);
    }
  check(insideChanged, "selected quad changes pixels inside 18,12,90,44");
  check(omittedUnchanged, "omitting the selected quad leaves 18,12,90,44 unchanged");

  bool outsideUnchanged = isBackground(drawn, 17, 12, br, bg, bb) &&
                          isBackground(drawn, 90, 12, br, bg, bb) &&
                          isBackground(drawn, 18, 11, br, bg, bb) &&
                          isBackground(drawn, 18, 44, br, bg, bb) &&
                          isBackground(drawn, 18, 52, br, bg, bb);
  check(outsideUnchanged, "selected quad leaves pixels outside 18,12,90,44 unchanged");

  Image plates = omitted;
  drawSkillPanelPlates(plates, patch);
  bool index0Matches = true;
  bool index1Differs = false;
  for (int y = 12; y < 44; ++y)
    for (int x = 18; x < 90; ++x)
      index0Matches = index0Matches && sameRgb(drawn, plates, x, y);
  for (int y = 52; y < 84; ++y)
    for (int x = 18; x < 90; ++x)
      index1Differs = index1Differs || !sameRgb(drawn, plates, x, y);
  check(index0Matches, "selected quad matches the base-plate sampler at index 0");
  check(index1Differs, "selected quad does not draw the other nine plates");

  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir) {
    std::cout << "selected retail destination: SKIPPED (WEBOLUTION_GAME_DIR unset)\n";
    return;
  }
  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box", fs::path(dir) / "patch.box");
  check(bool(lib), "open archive for selected-plate destination");
  if (!lib) return;
  auto baseSrc = webo::loadTexture(*lib, "TEXTURES\\^TAB.BMP");
  auto selSrc = webo::loadTexture(*lib, "TEXTURES\\^TAB_SELECTED.BMP");
  check(bool(baseSrc) && bool(selSrc), "load ^tab and ^tab_selected");
  if (!baseSrc || !selSrc) return;
  const Image base = resizeSkillPlatePatch(*baseSrc);
  const Image selected = resizeSkillPlatePatch(*selSrc);
  check(!base.empty() && !selected.empty(), "resize both plate sources");
  if (base.empty() || selected.empty()) return;

  Image platesOnly = makeSolid(640, 480, 0, 0, 0, 255);
  drawSkillPanelPlates(platesOnly, base);
  Image withSelected = platesOnly;
  webo::app::drawSkillPanelSelectedPlate(withSelected, selected);
  int differ = 0;
  for (int y = 12; y < 44; ++y)
    for (int x = 18; x < 90; ++x)
      differ += !sameRgb(platesOnly, withSelected, x, y) ? 1 : 0;
  check(differ > 0, "archived selected quad changes pixels inside 18,12,90,44");

  Image retail = makeSolid(640, 480, 0, 0, 0, 255);
  auto panel = drawRetailSkillPanel(retail, *lib, "MM_CLIMB_002");
  if (!panel) {
    check(false, panel.error());
    return;
  }
  check(true, "retail panel draw");
  int matched = 0;
  for (int y = 12; y < 44; ++y)
    for (int x = 18; x < 90; ++x)
      if (!sameRgb(platesOnly, withSelected, x, y) && sameRgb(retail, withSelected, x, y))
        ++matched;
  std::cout << std::format("    selected-vs-base pixels {}, retail matches {}\n", differ, matched);
  check(matched > 0, "retail panel includes the selected quad inside 18,12,90,44");
}

void testLinearPlateSampleDiffersFromNearest() {
  std::cout << "linear plate sample\n";
  // Two texels on one row. x=0 lands on s=0, so linear equals that texel.
  // x=1 has s=128/72, weights 16 and 56. Truncating blend of 0 and 255 is
  // 198. Nearest min(127,(128+36)/72) is texel 2 and would write 255.
  Image plate = makeSolid(128, 32, 0, 0, 0, 0);
  plate.rgba[0] = 10;
  plate.rgba[1] = 20;
  plate.rgba[2] = 30;
  plate.rgba[3] = 255;
  plate.rgba[4] = 0;
  plate.rgba[5] = 0;
  plate.rgba[6] = 0;
  plate.rgba[7] = 255;
  plate.rgba[8] = 255;
  plate.rgba[9] = 0;
  plate.rgba[10] = 0;
  plate.rgba[11] = 255;

  auto expect = [](const Image& dest, const char* path) {
    const auto p0 = (12u * dest.width + 18u) * 4;
    check(dest.rgba[p0] == 10 && dest.rgba[p0 + 1] == 20 && dest.rgba[p0 + 2] == 30,
          std::format("{} keeps the unrounded coordinate at x=0", path));
    const auto p1 = (12u * dest.width + 19u) * 4;
    check(dest.rgba[p1] == 198 && dest.rgba[p1 + 1] == 0 && dest.rgba[p1 + 2] == 0,
          std::format("{} linear sample at x=1 is 198, not nearest 255 (got {})", path, dest.rgba[p1]));
  };

  Image base = makeSolid(640, 480, 0, 0, 0, 255);
  drawSkillPanelPlates(base, plate);
  expect(base, "base plates");
  Image selected = makeSolid(640, 480, 0, 0, 0, 255);
  webo::app::drawSkillPanelSelectedPlate(selected, plate);
  expect(selected, "selected quad");
}

void testPatchPlateFullWordsAgainstC1bc() {
  if (!std::getenv("WEBOLUTION_C1BC_STREAM_TEST")) {
    std::cout << "C1bc scratch stream comparison: SKIPPED (set WEBOLUTION_C1BC_STREAM_TEST=1 to opt in)\n";
    return;
  }
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir) { std::cout << "C1bc full stream: SKIPPED (WEBOLUTION_GAME_DIR unset)\n"; return; }
  auto lib=webo::AssetLibrary::open(fs::path(dir)/"lemmings.box",fs::path(dir)/"patch.box");
  check(bool(lib), "open archive for C1bc word comparison"); if(!lib)return;
  auto src=webo::loadTexture(*lib,"TEXTURES\\^TAB.BMP");
  check(bool(src), "load original ^tab source"); if(!src)return;
  auto actual=resizeSkillPlatePatch(*src);
  auto expectedPath=fs::current_path()/"../../../build/tmp/c1bc-patch-texels/tab-p53-nearest.bgra";
  if(!fs::exists(expectedPath)) expectedPath=fs::current_path()/"build/tmp/c1bc-patch-texels/tab-p53-nearest.bgra";
  std::ifstream in(expectedPath,std::ios::binary);
  std::vector<std::uint8_t> expected((std::istreambuf_iterator<char>(in)),{});
  check(expected.size()==16384, "read complete C1bc expected stream");
  if(expected.size()!=16384)return;
  std::size_t differences=0;
  for(std::size_t i=0;i<4096;++i) for(int c=0;c<4;++c) {
    const auto streamChannel=expected[i*4+(c==0?2:c==2?0:c)];
    differences += actual.rgba[i*4+c] != streamChannel;
  }
  check(differences==0,std::format("all 16384 channels equal C1bc p53-nearest, mismatches={}",differences));
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
  testNativePanelPlacementAndDigit();
  testZeroCountSkipsTextAtDestination();
  testPatchPlateResizeRowAsymmetricAlpha();
  testSelectedQuadDestination();
  testLinearPlateSampleDiffersFromNearest();
  testPatchPlateFullWordsAgainstC1bc();
  testSkillPanelIconSheetsAreNotTheTenIcons();

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
