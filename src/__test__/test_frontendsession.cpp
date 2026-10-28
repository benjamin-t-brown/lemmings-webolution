#include "app/FrontendSession.h"

#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "sim/RetailLevel.h"
#include "shell/MenuRender.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

namespace {
int checks = 0, failures = 0;
void check(bool ok, std::string_view what) {
  ++checks;
  if (!ok) { ++failures; std::cerr << "FAIL: " << what << '\n'; }
}
std::array<unsigned char, 4> pixel(const Image& img, int x, int y) {
  const auto p = (static_cast<std::size_t>(y) * img.width + x) * 4;
  return {img.rgba[p], img.rgba[p + 1], img.rgba[p + 2], img.rgba[p + 3]};
}
}

int main() {
  const char* configured = std::getenv("WEBOLUTION_GAME_DIR");
  if (!configured || !*configured) {
    std::cout << "FrontendSession: SKIPPED (set WEBOLUTION_GAME_DIR to retail assets)\n";
    return 0;
  }
  const fs::path game(configured);
  auto library = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!library) { std::cerr << library.error() << '\n'; return 1; }
  auto make = [&]() -> std::optional<app::FrontendSession> {
    auto s = app::FrontendSession::create(*library);
    if (!s) { check(false, s.error()); return std::nullopt; }
    return std::move(*s);
  };
  using shell::Screen;

  auto s = make();
  if (!s) return 1;
  check(s->screen() == Screen::Menu && s->constructionCounter() == 3,
        "retail-backed session starts at menu construction 3");
  check(s->pointerX() == 320 && s->pointerY() == 240 &&
        s->cursorState() == app::CursorState::NoLemming,
        "frontend cursor starts centered in normal state");
  auto logo = loadTexture(*library, "TEXTURES\\MAIN_LOGO.BMP");
  auto font = loadTexture(*library, "TEXTURES\\^FONT.BMP");
  auto uk = loadStrings(*library, "UK");
  check(logo && font && uk, "menu oracle assets load");
  if (logo && font && uk) {
    std::vector<std::string> labels;
    for (auto i : shell::kLabelIndices) labels.emplace_back((*uk)[i]);
    const auto oracle = shell::renderMainMenu(*logo, *font, labels, {}, shell::fontCells());
    check(s->image().rgba == oracle.rgba, "menu image equals existing renderMainMenu on retail assets");
    auto cursorArt = loadTexture(*library, "TEXTURES\\_curNoLem.BMP");
    auto pressedArt = loadTexture(*library, "TEXTURES\\_curMseDwn.BMP");
    if (cursorArt) {
      const auto composed = s->composeFrame();
      bool texelMatch = true, backgroundMatch = true;
      for (int cy = 0; cy < 32; ++cy) for (int cx = 0; cx < 32; ++cx) {
        const auto src = pixel(*cursorArt, cx, cy);
        const auto actual = pixel(composed, 304 + cx, 224 + cy);
        if (src[0] || src[1] || src[2]) texelMatch &= actual == src;
        else backgroundMatch &= actual == pixel(s->image(), 304 + cx, 224 + cy);
      }
      check(texelMatch, "normal frontend cursor presents every non-key texel independently");
      check(backgroundMatch, "black-key cursor leaves independent scene background texels unchanged");
      check(pixel(composed, 0, 0) == pixel(s->image(), 0, 0), "cursor composition preserves distant background");
    } else check(false, "frontend normal cursor artwork loads independently");
    s->setPointer(0, 0);
    const auto clipped = s->composeFrame();
    check(s->pointerX() == 0 && s->pointerY() == 0 &&
          clipped.width == 640 && clipped.height == 480 && cursorArt &&
          pixel(clipped, 0, 0) == pixel(*cursorArt, 16, 16) &&
          pixel(clipped, 16, 16) == pixel(s->image(), 16, 16),
          "cursor is clipped at inclusive top-left without altering adjacent background");
    s->setPointer(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
    check(s->pointerX() == 0 && s->pointerY() == 480,
          "pointer clamp safely handles signed integer extremes");
    s->setPointer(std::numeric_limits<int>::max(), std::numeric_limits<int>::min());
    check(s->pointerX() == 640 && s->pointerY() == 0,
          "pointer clamp safely handles reversed signed integer extremes");
    s->setPointer(640, 480);
    const auto bottomRight = s->composeFrame();
    bool bottomRightTexel = false;
    if (cursorArt) for (int cy = 0; cy < 16 && !bottomRightTexel; ++cy) {
      for (int cx = 0; cx < 16 && !bottomRightTexel; ++cx) {
        const auto source = pixel(*cursorArt, cx, cy);
        if (source[0] || source[1] || source[2]) {
          bottomRightTexel = pixel(bottomRight, 624 + cx, 464 + cy) == source;
        }
      }
    }
    check(bottomRightTexel && pixel(bottomRight, 623, 463) == pixel(s->image(), 623, 463),
          "cursor clips at inclusive bottom-right with adjacent background unchanged");
    s->setPointer(320, 240);
    s->setButtons(true, false);
    check(s->cursorState() == app::CursorState::MouseDown, "left-held frontend cursor uses MouseDown");
    if (pressedArt) {
      const auto pressed = s->composeFrame();
      bool pressedTexels = true, pressedBackground = true;
      for (int cy = 0; cy < 32; ++cy) for (int cx = 0; cx < 32; ++cx) {
        const auto src = pixel(*pressedArt, cx, cy);
        const auto actual = pixel(pressed, 304 + cx, 224 + cy);
        if (src[0] || src[1] || src[2]) pressedTexels &= actual == src;
        else pressedBackground &= actual == pixel(s->image(), 304 + cx, 224 + cy);
      }
      check(pressedTexels && pressedBackground,
            "held frontend cursor presents independent pressed texels and keyed background");
    } else check(false, "frontend pressed cursor artwork loads independently");
    s->setButtons(false, true);
    check(s->cursorState() == app::CursorState::MouseDown, "right-held frontend cursor is MouseDown, not Grab");
    s->setButtons(true, true);
    check(s->cursorState() == app::CursorState::MouseDown, "both held frontend buttons retain MouseDown");
    s->setButtons(false, true);
    check(s->cursorState() == app::CursorState::MouseDown, "releasing one button retains other held state");
    s->setButtons(false, false);
    check(s->cursorState() == app::CursorState::NoLemming, "releasing both restores normal cursor");
    s->setButtons(true, false);
    s->setFocused(false);
    check(s->cursorState() == app::CursorState::Hidden && !s->leftHeld() && !s->rightHeld(),
          "focus loss hides cursor and clears held buttons");
    check(s->composeFrame().rgba == s->image().rgba,
          "hidden frontend cursor composes exactly the cursor-free base image");
    s->setFocused(true);
    check(s->cursorState() == app::CursorState::NoLemming, "focus regain restores normal cursor");
  }
  check(!s->clickMap(206, 204) && s->screen() == Screen::Menu,
        "map click before map has no effect");
  check(!s->click(0, 0) && s->screen() == Screen::Menu, "unrelated menu click ignored");
  check(s->click(320, 333) && s->screen() == Screen::Confirm && s->constructionCounter() == 4,
        "New Game click opens confirmation at construction 4");
  check(s->image().width == 640 && s->image().height == 480, "confirmation is native canvas size");

  // Independent pixel conditions rather than a second implementation of the renderer:
  // the backdrop must survive outside the known text/panel footprints, the heading
  // uses red font pixels without a surrounding panel, and initial Yes is highlighted.
  auto back = loadTexture(*library, "TEXTURES\\SMALL_BACK.BMP");
  if (back) {
    const auto& img = s->image();
    for (const auto& [x, y] : {std::pair{0, 0}, {639, 479}, {80, 250}, {500, 260}, {100, 400}})
      check(pixel(img, x, y) == pixel(*back, x, y), "confirmation preserves SMALL_BACK at exterior sample");
    bool redHeading = false;
    for (int y = 250; y < 279; ++y) for (int x = 180; x < 460; ++x) {
      const auto p = pixel(img, x, y);
      redHeading |= p[0] > 180 && p[1] < 40 && p[2] < 40;
    }
    check(redHeading, "UK[129] heading contributes red font pixels around y=250");
    check(pixel(img, 260, 250) == pixel(*back, 260, 250) &&
          pixel(img, 380, 250) == pixel(*back, 380, 250), "heading has no panel padding");
    check(pixel(img, 283, 301) == std::array<unsigned char, 4>{64,64,64,255}, "Yes panel border at derived origin");
    check(pixel(img, 285, 305) == std::array<unsigned char, 4>{224,128,0,255}, "initial Yes row is highlighted");
    check(pixel(img, 287, 330) == std::array<unsigned char, 4>{64,64,64,255}, "No panel border at derived origin");
    check(pixel(img, 288, 331) == std::array<unsigned char, 4>{224,10,0,255}, "No row is unselected");
    check(pixel(img, 300, 363) == pixel(*back, 300, 363), "choice panel ends below row bounds");
    check(pixel(img, 300, 329) == pixel(*back, 300, 329), "No panel does not start one row early");
    if (font) {
      const auto c = shell::fontCells()['C' - ' '];
      bool found = false;
      for (int y = 0; y < shell::kLineHeight && !found; ++y) for (int x = 0; x < c.width && !found; ++x) {
        const int alpha = pixel(*font, c.x + x, c.y + y)[3];
        if (alpha == 0) continue;
        auto expected = pixel(*back, 272 + x, 250 + y);
        for (int channel = 0; channel < 3; ++channel)
          expected[channel] = (expected[channel] * (255 - alpha) + (channel == 0 ? 255 : 0) * alpha + 127) / 255;
        check(pixel(img, 272 + x, 250 + y) == expected, "heading C glyph starts at exact derived anchor and red tint");
        found = true;
      }
      check(found, "heading glyph oracle found nontransparent C pixel");
    }
  } else check(false, "SMALL_BACK loads");

  auto cancel = make();
  if (!cancel) return 1;
  cancel->pressReturn();
  check(cancel->click(320, 340) && cancel->screen() == Screen::Menu &&
        cancel->constructionCounter() == 5, "No click routes from Confirm to Menu5");
  check(cancel->pressReturn() == shell::Outcome::Handled && cancel->screen() == Screen::Confirm &&
        cancel->constructionCounter() == 6, "Return from Menu5 returns to Confirm6");
  check(cancel->pressReturn() == shell::Outcome::Handled && cancel->screen() == Screen::Map &&
        cancel->constructionCounter() == 7, "Return from Confirm6 routes to Map7");

  auto route = make();
  if (!route) return 1;
  route->pressReturn();
  check(route->pressReturn() == shell::Outcome::Handled && route->screen() == Screen::Map &&
        route->constructionCounter() == 5, "ordinary Return sequence reaches map at construction 5");
  auto map = app::MapScene::build(*library, {});
  check(map.has_value(), "map base scene loads");
  if (map) {
    bool baseUnchanged = true;
    for (int y = 0; y < 480; ++y) for (int x = 0; x < 640; ++x)
      if (x < 205 || x >= 436 || y < 440 || y >= 469)
        baseUnchanged &= pixel(route->image(), x, y) == pixel(map->image(), x, y);
    check(baseUnchanged, "map base pixels outside footer unchanged");
    bool border = true;
    for (int y = 441; y < 468; ++y) for (int x = 205; x < 436; ++x)
      if (x == 205 || x == 435 || y == 441 || y == 467)
        border &= pixel(route->image(), x, y) == std::array<unsigned char, 4>{64,64,64,255};
    check(border, "all 512 footer border pixels match observed native border");
    check(pixel(route->image(), 206, 442) == std::array<unsigned char, 4>{224,128,0,255},
          "map footer initially selected");
    bool blackText = false;
    for (int y = 440; y < 469; ++y) for (int x = 224; x < 417; ++x)
      blackText |= pixel(route->image(), x, y) == std::array<unsigned char, 4>{0,0,0,255};
    check(blackText, "map footer includes black label glyphs");
  }
  check(!route->click(0, 0) && !route->selectedLevel(), "map background click ignored");
  check(route->openLevelMenu() == shell::Outcome::Ignored &&
        route->closeLevelMenu(false) == shell::Outcome::Ignored && route->constructionCounter() == 5,
        "session rejects menu lifecycle calls while Map owns the frontend root");
  check(route->click(206, 204) && route->selectedLevel() == std::optional<std::string>{"MM_CLIMB_002"},
        "node at 206,204 selects MM_CLIMB_002");
  check(route->screen() == Screen::Level && route->constructionCounter() == 5, "node selection enters level with construction counter unchanged");
  check(route->openLevelMenu() == shell::Outcome::Handled && route->screen() == Screen::Dialog &&
        route->constructionCounter() == 7, "session semantic menu open constructs root and page");
  check(route->openLevelMenu() == shell::Outcome::Ignored && route->constructionCounter() == 7,
        "session cannot construct a second root while menu is owned");
  check(route->closeLevelMenu(false) == shell::Outcome::Handled && route->screen() == Screen::Level &&
        route->constructionCounter() == 7 &&
        route->selectedLevel() == std::optional<std::string>{"MM_CLIMB_002"} &&
        route->completionFlags() == std::array<std::int32_t, 102>{} && !route->outcome(),
        "Resume closes root without count or selected-level/progress/outcome mutation");
  check(route->openLevelMenu() == shell::Outcome::Handled && route->closeLevelMenu(true) == shell::Outcome::Handled &&
        route->constructionCounter() == 9 && route->screen() == Screen::Level,
        "session O-close uses a fresh root and preserves paused distinction in shell state");
  check(!route->click(206, 204) && route->selectedLevel() == std::optional<std::string>{"MM_CLIMB_002"},
        "repeated selection click is ignored after leaving map");
  if (!route->selectedLevel()) return 1;
  auto setup = sim::loadRetailLevel(*library, *route->selectedLevel());
  check(setup.has_value(), "selected retail level loads through sim::loadRetailLevel");
  if (setup) {
    sim::World world(*setup);
    for (int i = 0; i < 20; ++i) world.tick();
    check(world.spawned() >= 1, "selected level World spawns from its loaded setup");
  }

  shell::Frontend quirk;
  quirk.setHighlight(1);
  check(quirk.press(shell::Key::Return) == shell::Outcome::Ignored &&
        quirk.screen() == Screen::Menu && quirk.counter() == 3,
        "highlight-1 Return preserves ignored quirk");
  check(quirk.confirmChoice(true) == shell::Outcome::Ignored && quirk.screen() == Screen::Menu,
         "confirmChoice is a no-op outside confirmation");
  check(quirk.activateMapFooter() == shell::Outcome::Ignored,
        "map footer semantic action ignored outside map");
  auto footer = make();
  if (!footer) return 1;
  footer->pressReturn(); footer->pressReturn();
  for (const auto& [x, y] : {std::pair{204, 455}, {436, 455}, {320, 440}, {320, 468}})
    check(!footer->click(x, y) && footer->screen() == Screen::Map, "outside footer hitbox does not leave map");
  check(footer->click(320,455) && footer->screen() == Screen::Menu &&
        footer->constructionCounter() == 6 && !footer->selectedLevel(),
        "footer activation rebuilds main menu without selecting a level");
  footer->pressReturn(); footer->pressReturn();
  check(footer->screen() == Screen::Map && footer->constructionCounter() == 8,
        "new game remains reachable after map footer");
  check(footer->pressReturn() == shell::Outcome::Handled && footer->screen() == Screen::Menu &&
        footer->constructionCounter() == 9, "SDL Return adapter activates sole focused footer");

  std::cout << checks << " checks, " << failures << " failures\n";
  return failures ? 1 : 0;
}
