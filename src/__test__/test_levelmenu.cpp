#include "app/LevelMenu.h"

#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "shell/MenuRender.h"

#include <array>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

namespace {
int checks = 0;
int failures = 0;

void check(bool ok, std::string_view what) {
  ++checks;
  if (!ok) {
    ++failures;
    std::cerr << "FAIL: " << what << '\n';
  }
}

Image pattern() {
  Image image;
  image.width = 640;
  image.height = 480;
  image.rgba.resize(static_cast<std::size_t>(image.width) * image.height * 4);
  for (int y = 0; y < image.height; ++y) {
    for (int x = 0; x < image.width; ++x) {
      const auto at = (static_cast<std::size_t>(y) * image.width + x) * 4;
      image.rgba[at] = static_cast<std::uint8_t>((x * 3 + y) & 255);
      image.rgba[at + 1] = static_cast<std::uint8_t>((x + y * 5) & 255);
      image.rgba[at + 2] = static_cast<std::uint8_t>((x * 7 + y * 11) & 255);
      image.rgba[at + 3] = 255;
    }
  }
  return image;
}
} // namespace

int main() {
  const char* configured = std::getenv("WEBOLUTION_GAME_DIR");
  if (!configured || !*configured) {
    std::cout << "LevelMenu: SKIPPED (set WEBOLUTION_GAME_DIR to retail assets)\n";
    return 0;
  }
  const fs::path game(configured);
  auto library = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!library) {
    std::cerr << library.error() << '\n';
    return 1;
  }
  auto menuResult = app::LevelMenu::load(*library);
  check(static_cast<bool>(menuResult), "load UK level-menu font and labels");
  if (!menuResult) return 1;
  auto menu = std::move(*menuResult);

  using app::LevelMenuAction;
  check(!menu.isOpen() && menu.focusedIndex() == 0 &&
        menu.focusedAction() == LevelMenuAction::None,
        "starts closed; inactive focus has no action");
  const Image before = pattern();
  Image closedFrame = before;
  menu.draw(closedFrame);
  check(closedFrame.rgba == before.rgba, "closed draw is a no-op");
  check(menu.actionAt(320, 110) == LevelMenuAction::None,
        "closed menu hit returns None");

  menu.open();
  check(menu.isOpen() && menu.focusedIndex() == 0 &&
        menu.focusedAction() == LevelMenuAction::Resume,
        "open focuses Resume");
  // Verified UK content rectangles: (L,T,R,B), with strict edges.
  const std::array<std::array<int, 4>, 4> rects{{
      {{281, 100, 360, 129}}, {{271, 129, 369, 158}},
      {{300, 158, 341, 187}}, {{282, 187, 359, 216}},
  }};
  const std::array<LevelMenuAction, 4> actions{{
      LevelMenuAction::Resume, LevelMenuAction::Retry,
      LevelMenuAction::MapQuit, LevelMenuAction::UnsupportedOptions,
  }};
  for (std::size_t i = 0; i < rects.size(); ++i) {
    const auto& r = rects[i];
    check(menu.actionAt(r[0] + 1, r[1] + 1) == actions[i], "content interior maps to row action");
    check(menu.actionAt(r[0], r[1] + 1) == LevelMenuAction::None &&
          menu.actionAt(r[2], r[1] + 1) == LevelMenuAction::None &&
          menu.actionAt(r[0] + 1, r[1]) == LevelMenuAction::None &&
          menu.actionAt(r[0] + 1, r[3]) == LevelMenuAction::None,
          "content rectangle edges are strict");
  }
  check(menu.actionAt(320, 216) == LevelMenuAction::None &&
        menu.actionAt(320, 230) == LevelMenuAction::None,
        "empty fifth widget has no invented hit area/action");

  menu.hover(320, 140);
  check(menu.focusedIndex() == 1 && menu.focusedAction() == LevelMenuAction::Retry,
        "hovering Retry updates focus");
  menu.hover(252, 141); // panel edge, but outside the text/content hit rectangle
  check(menu.focusedIndex() == 1 && menu.focusedAction() == LevelMenuAction::Retry,
        "miss preserves prior focus");
  menu.hover(320, 200);
  check(menu.focusedIndex() == 3 && menu.focusedAction() == LevelMenuAction::UnsupportedOptions,
        "Options is focused but explicitly unsupported");

  Image frame = before;
  menu.draw(frame);
  const std::vector<std::string> literalUkLabels{"Resume", "Try Again", "Quit", "Options"};
  shell::MenuParams params;
  params.baseY = 100;
  params.highlight = 3;
  auto font = loadTexture(*library, "TEXTURES\\^FONT.BMP");
  check(static_cast<bool>(font), "load font for independent render comparison");
  if (!font) return 1;
  const auto expected = shell::renderMainMenu(before, *font, literalUkLabels,
                                               params, shell::fontCells());
  check(frame.rgba == expected.rgba,
        "draw matches independent literal-UK renderer assembly (shared primitives, not native oracle)");
  // A known untouched pixel away from all panel/text bounds remains from the scene.
  const auto pixelOffset = (static_cast<std::size_t>(20) * 640 + 20) * 4;
  check(std::equal(frame.rgba.begin() + static_cast<std::ptrdiff_t>(pixelOffset),
                   frame.rgba.begin() + static_cast<std::ptrdiff_t>(pixelOffset + 4),
                   before.rgba.begin() + static_cast<std::ptrdiff_t>(pixelOffset)),
        "draw leaves pixels outside menu panels unchanged");

  menu.close();
  check(!menu.isOpen() && menu.actionAt(320, 140) == LevelMenuAction::None &&
        menu.focusedAction() == LevelMenuAction::None,
        "closed menu actions return None");
  Image afterClose = before;
  menu.draw(afterClose);
  check(afterClose.rgba == before.rgba, "closed menu remains a draw no-op");
  menu.open();
  check(menu.focusedIndex() == 0 && menu.focusedAction() == LevelMenuAction::Resume,
        "reopening resets focus to Resume");

  std::cout << checks << " checks, " << failures << " failures\n";
  return failures == 0 ? 0 : 1;
}
