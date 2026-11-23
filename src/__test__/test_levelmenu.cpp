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
  auto menuResult = app::LevelMenu::load(*library, "mm_climb_002");
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
  // UK content rectangles are (L,T,R,B) and exclude their edges.
  const std::array<std::array<int, 4>, 5> rects{{
      {{281, 100, 360, 129}}, {{271, 129, 369, 158}},
      {{300, 158, 341, 187}}, {{282, 187, 359, 216}},
      {{279, 216, 362, 245}},
  }};
  const std::array<LevelMenuAction, 5> actions{{
      LevelMenuAction::Resume, LevelMenuAction::Retry,
      LevelMenuAction::MapQuit, LevelMenuAction::SoundOptions, LevelMenuAction::Examine,
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
  check(menu.actionAt(320, 245) == LevelMenuAction::None &&
        menu.actionAt(320, 259) == LevelMenuAction::None,
        "native empty widget has no additional hit area or action");
  for (std::size_t i = 0; i < actions.size(); ++i) {
    check(menu.focusedIndex() == static_cast<int>(i) && menu.focusedAction() == actions[i],
          "keyboard focus visits only action rows");
    menu.moveFocus(1);
  }
  menu.moveFocus(-1);
  check(menu.focusedAction() == LevelMenuAction::Examine, "up from Resume wraps to Examine");
  menu.moveFocus(1);
  check(menu.focusedAction() == LevelMenuAction::Resume, "down from Examine wraps to Resume");

  menu.hover(320, 140);
  check(menu.focusedIndex() == 1 && menu.focusedAction() == LevelMenuAction::Retry,
        "hovering Retry updates focus");
  menu.hover(320, 75);
  check(menu.focusedIndex() == 1 && menu.actionAt(320, 75) == LevelMenuAction::None &&
        !menu.touchButtonAt(320, 75), "heading is outside row and touch plate input");
  check(menu.touchButtonAt(320, 20), "touch Menu plate retains its own hit area");
  menu.hover(252, 141); // panel edge, but outside the text/content hit rectangle
  check(menu.focusedIndex() == 1 && menu.focusedAction() == LevelMenuAction::Retry,
        "miss preserves prior focus");
  menu.hover(320, 200);
  check(menu.focusedIndex() == 3 && menu.focusedAction() == LevelMenuAction::SoundOptions,
        "focused Options requests sound settings");
  menu.hover(320, 230);
  check(menu.focusedIndex() == 4 && menu.focusedAction() == LevelMenuAction::Examine,
        "hovering Examine updates focus");

  Image frame = before;
  menu.draw(frame);
  const std::vector<std::string> labels{"Resume", "Try Again", "Quit", "Options", "Examine"};
  shell::MenuParams params;
  params.baseY = 100;
  params.highlight = 4;
  auto font = loadTexture(*library, "TEXTURES\\^FONT.BMP");
  check(static_cast<bool>(font), "load font for independent render comparison");
  if (!font) return 1;
  const auto& cells = shell::fontCells();
  const auto drawHeading = [&](Image& target, std::string title, int top) {
    const int left = (640 - shell::textWidth(title, cells)) / 2;
    const std::array heading{shell::HudLine{title, left + 2, top + 2, {220, 50, 50}},
                             shell::HudLine{title, left, top, {0, 0, 0}}};
    shell::drawHud(target, *font, heading, cells);
  };
  auto expected = shell::renderMainMenu(before, *font, labels, params, cells);
  drawHeading(expected, "Just Climb", 60);
  check(frame.rgba == expected.rgba,
        "centred installed heading uses the map colours above the existing menu rows");
  // Sample the scene outside all panel and text bounds.
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

  const std::string first(20, 'W'), second(20, 'M');
  auto wrapped = app::LevelMenu::load(*library, first + " " + second);
  check(wrapped.has_value(), "load menu for an id with no installed title");
  if (!wrapped) return 1;
  wrapped->open();
  wrapped->hover(320, 171);
  check(wrapped->focusedIndex() == 1, "hover follows the shifted Retry row");
  wrapped->hover(320, 105);
  check(wrapped->focusedIndex() == 1 && wrapped->actionAt(320, 105) == LevelMenuAction::None &&
        !wrapped->touchButtonAt(320, 105), "second heading line is also outside input");
  for (std::size_t i = 0; i < rects.size(); ++i) {
    const auto& r = rects[i];
    check(wrapped->actionAt(r[0] + 1, r[1] + 32) == actions[i],
          "wrapped title shifts each row hit down by one line");
  }
  params.baseY = 131;
  params.highlight = 1;
  expected = shell::renderMainMenu(before, *font, labels, params, cells);
  drawHeading(expected, first, 60);
  drawHeading(expected, second, 91);
  frame = before;
  wrapped->draw(frame);
  check(frame.rgba == expected.rgba,
        "the id fallback wraps at a space and moves the drawn rows with their hit areas");

  std::cout << checks << " checks, " << failures << " failures\n";
  return failures == 0 ? 0 : 1;
}
