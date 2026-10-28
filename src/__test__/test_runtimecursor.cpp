#include "app/RuntimeCursor.h"
#include "assets/Box.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

namespace {

void check(bool condition, std::string_view label) {
  if (!condition) {
    std::cerr << "runtime cursor FAIL: " << label << '\n';
    std::exit(1);
  }
}

webo::Image blankTarget() {
  webo::Image image;
  image.width = image.height = 64;
  image.rgba.resize(64u * 64u * 4u);
  for (std::size_t i = 0; i < 64u * 64u; ++i) {
    image.rgba[i * 4 + 0] = 17;
    image.rgba[i * 4 + 1] = 31;
    image.rgba[i * 4 + 2] = 63;
    image.rgba[i * 4 + 3] = 255;
  }
  return image;
}

webo::Image expectedComposite(const webo::Image& source, int x, int y) {
  auto expected = blankTarget();
  const int left = x - 16;
  const int top = y - 16;
  for (int sy = 0; sy < 32; ++sy) {
    for (int sx = 0; sx < 32; ++sx) {
      const int dx = left + sx;
      const int dy = top + sy;
      if (dx < 0 || dx >= 64 || dy < 0 || dy >= 64) continue;
      const std::size_t sourceOffset = (static_cast<std::size_t>(sy) * 32 + sx) * 4;
      const std::size_t targetOffset = (static_cast<std::size_t>(dy) * 64 + dx) * 4;
      const unsigned alpha = source.rgba[sourceOffset + 3];
      for (int channel = 0; channel < 3; ++channel) {
        const unsigned src = source.rgba[sourceOffset + channel];
        const unsigned dst = expected.rgba[targetOffset + channel];
        expected.rgba[targetOffset + channel] = static_cast<std::uint8_t>(
            (src * alpha + dst * (255 - alpha)) / 255);
      }
      expected.rgba[targetOffset + 3] = 255;
    }
  }
  return expected;
}

void testStateSelection() {
  using webo::app::CursorState;
  using webo::app::runtimeCursorState;
  check(runtimeCursorState(false, false, true, false, 2, 1) == CursorState::Hidden,
        "focus loss hides cursor even with actor hit");
  check(runtimeCursorState(true, true, false, true, 2, 1) == CursorState::Grab,
        "grab overrides populated panel");
  check(runtimeCursorState(true, false, true, false, -1, 0) == CursorState::Lemming,
        "actor hit selects lemming cursor");
  check(runtimeCursorState(true, false, false, false, -1, 0) == CursorState::NoLemming,
        "empty world hit selects no-lemming cursor");
  check(runtimeCursorState(true, false, false, true, 3, 1) == CursorState::Lemming,
        "stock panel item selects lemming cursor");
   check(runtimeCursorState(true, false, false, true, 3, 0) == CursorState::NoLemming,
         "empty panel item with app-suppressed actor hit remains no-lemming");
   check(runtimeCursorState(true, false, true, true, 3, 0) == CursorState::Lemming,
         "ordered native policy retains earlier hit for unavailable panel in display mode0");
}

void testFrontendStateSelection() {
  using webo::app::CursorState;
  using webo::app::frontendCursorState;
  check(frontendCursorState(true, false) == CursorState::NoLemming,
        "focused frontend without held buttons selects normal cursor");
  check(frontendCursorState(true, true) == CursorState::MouseDown,
        "either held frontend button selects MouseDown");
  check(frontendCursorState(false, false) == CursorState::Hidden,
        "unfocused frontend cursor is hidden");
  check(frontendCursorState(false, true) == CursorState::Hidden,
        "focus loss hides frontend cursor even while button held");
}

} // namespace

int main() {
  using namespace webo;
  using namespace webo::app;
  testStateSelection();
  testFrontendStateSelection();

  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir || !std::filesystem::exists(std::filesystem::path(dir) / "lemmings.box")) {
    std::cout << "runtime cursor retail pixels: SKIPPED (WEBOLUTION_GAME_DIR unset or archive absent)\n";
    std::cout << "runtime cursor state: passed\n";
    return 0;
  }

  auto library = AssetLibrary::open(std::filesystem::path(dir) / "lemmings.box",
                                    std::filesystem::path(dir) / "patch.box");
  check(bool(library), "open retail asset library");
  if (!library) return 1;

  constexpr std::array<std::string_view, 4> resources = {
      "TEXTURES\\_curNoLem.BMP", "TEXTURES\\_curLem.BMP",
      "TEXTURES\\^curGrab.BMP", "TEXTURES\\_curMseDwn.BMP"};
  constexpr std::array<CursorState, 4> states = {
      CursorState::NoLemming, CursorState::Lemming,
      CursorState::Grab, CursorState::MouseDown};
  std::array<Image, 4> sourceImages;
  for (std::size_t i = 0; i < resources.size(); ++i) {
    auto image = loadTexture(*library, resources[i]);
    check(bool(image), "load retail cursor image");
    if (!image) return 1;
    check(image->width == 32 && image->height == 32, "retail cursor dimensions are 32x32");
    sourceImages[i] = std::move(*image);
  }
  auto cursor = RuntimeCursor::load(*library);
  check(bool(cursor), "load RuntimeCursor");
  if (!cursor) return 1;

  constexpr std::array<std::array<int, 2>, 5> positions = {{{32, 32}, {0, 0}, {63, 63}, {-40, -40}, {100, 100}}};
  for (std::size_t stateIndex = 0; stateIndex < states.size(); ++stateIndex) {
    for (const auto& position : positions) {
      auto actual = blankTarget();
      const auto expected = expectedComposite(sourceImages[stateIndex], position[0], position[1]);
      cursor->draw(actual, states[stateIndex], position[0], position[1]);
      check(actual.rgba == expected.rgba, "retail cursor pixels match independent clipped integer blend");
    }
  }

  auto unchanged = blankTarget();
  const auto original = unchanged.rgba;
  cursor->draw(unchanged, CursorState::Hidden, 32, 32);
  check(unchanged.rgba == original, "hidden cursor leaves target unchanged");

  // Grab uses the archived per-pixel alpha plane; ensure actual partially
  // transparent texels participate in fractional blending, not binary masking.
  auto renderedGrab = blankTarget();
  cursor->draw(renderedGrab, CursorState::Grab, 32, 32);
  std::size_t partialAlpha = 0;
  bool partialPixelBlended = false;
  for (std::size_t p = 0; p < sourceImages[2].pixelCount(); ++p) {
    const auto offset = p * 4;
    const unsigned alpha = sourceImages[2].rgba[offset + 3];
    if (alpha == 0 || alpha == 255) continue;
    ++partialAlpha;
    const std::size_t x = p % 32 + 16;
    const std::size_t y = p / 32 + 16;
    const auto renderedOffset = (y * 64 + x) * 4;
    bool differsFromBackground = false;
    bool nonwhite = false;
    for (int c = 0; c < 3; ++c) {
      const auto color = renderedGrab.rgba[renderedOffset + c];
      differsFromBackground |= color != (c == 0 ? 17 : c == 1 ? 31 : 63);
      nonwhite |= color != 255;
    }
    partialPixelBlended |= differsFromBackground && nonwhite;
  }
  check(partialAlpha > 0 && partialPixelBlended,
        "grab contains represented partial-alpha pixels (not only a binary mask)");
  std::cout << "runtime cursor state and retail pixel composition passed\n";
}
