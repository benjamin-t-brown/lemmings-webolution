#include "app/AppEntry.h"
#include "app/RuntimeCursor.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "shell/MenuRender.h"
#include "shell/PauseCaption.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <array>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

using namespace webo;

namespace {
bool key(SDL_Keycode code) {
  SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.keysym.sym = code;
  e.key.keysym.scancode = SDL_GetScancodeFromKey(code);
  return SDL_PushEvent(&e) == 1;
}
bool quit() { SDL_Event e{}; e.type = SDL_QUIT; return SDL_PushEvent(&e) == 1; }
bool selectNode() {
  SDL_Event e{}; e.type = SDL_MOUSEBUTTONDOWN; e.button.button = SDL_BUTTON_LEFT;
  e.button.x = 206; e.button.y = 204;
  return SDL_PushEvent(&e) == 1;
}
}

int main() {
  const char* driver = std::getenv("SDL_VIDEODRIVER");
  if (!driver || std::string(driver) != "dummy") {
    std::cerr << "EntryCueWindow requires SDL_VIDEODRIVER=dummy\n"; return 1;
  }
  const char* game = std::getenv("WEBOLUTION_GAME_DIR");
  if (!game || !*game) { std::cout << "EntryCueWindow: SKIPPED (retail assets unavailable)\n"; return 0; }
  auto library = AssetLibrary::open(std::filesystem::path(game) / "lemmings.box",
                                    std::filesystem::path(game) / "patch.box");
  if (!library) return 1;
  auto font = loadTexture(*library, "TEXTURES\\^FONT.BMP");
  auto strings = loadStrings(*library, "UK");
  if (!font || !strings || strings->size() <= 71 || (*strings)[71] != "Let's Go!") return 1;
  auto cursor = app::RuntimeCursor::load(*library);
  if (!cursor) return 1;

  // Independent interior-only mask for the base (160,180)-(480,260) cue.
  // Use fully opaque texels at the SOURCE point-sample locations. This tests
  // app composition/omission, not native half-texel, sampling or raster edges.
  const auto& cells = shell::fontCells();
  const std::string text = "Let's Go!";
  int width = 0;
  for (unsigned char c : text) width += cells[c - 32].width;
  const auto mask = [&](int startX, int startY, int endX, int endY) {
  std::vector<std::pair<int, int>> opaque;
  double left = startX;
  for (unsigned char c : text) {
    const auto cell = cells[c - 32];
    const double right = left + cell.width * static_cast<double>(endX - startX) / width;
    const int x0 = static_cast<int>(left), x1 = static_cast<int>(right);
    for (int y = std::max(0, startY); y < std::min(480, endY); ++y)
    for (int x = std::max(0, x0); x < std::min(640, x1); ++x) {
      const int sx = cell.x + (x - x0) * cell.width / (x1 - x0);
      const int sy = cell.y + (y - startY) * 29 / (endY - startY);
      if (sx <= cell.x || sx >= cell.x + cell.width - 1 || sy <= cell.y || sy >= cell.y + 28) continue;
      bool solid = true;
      const auto i = (sy * font->width + sx) * 4;
      for (int channel = 0; channel < 4; ++channel) solid &= font->rgba[i + channel] == 255;
      if (solid) opaque.emplace_back(x, y);
    }
    left = right;
  }
  return opaque;
  };
  const auto opaque = mask(160, 180, 480, 260);
  const auto largeOpaque = mask(-600, -10, 1240, 450);
  if (opaque.size() < 50 || largeOpaque.size() < 50) {
    int maxAlpha = 0, maxRgb = 0;
    for (std::size_t i = 0; i < font->rgba.size(); i += 4) {
      maxAlpha = std::max(maxAlpha, static_cast<int>(font->rgba[i + 3]));
      maxRgb = std::max(maxRgb, static_cast<int>(font->rgba[i]));
    }
    std::cerr << "insufficient cue interior mask: count=" << opaque.size() << " large=" << largeOpaque.size()
              << " max-alpha=" << maxAlpha << " max-rgb=" << maxRgb << '\n'; return 1;
  }
  Image white; white.width = 640; white.height = 480; white.rgba.assign(640 * 480 * 4, 255);
  bool ok = true;
  int frames = 0;
  Image firstPaused;
  const auto front = [&](const app::FrontendSession& s, int) {
    if (s.screen() == shell::Screen::Map) ok &= selectNode();
    else ok &= key(SDLK_RETURN);
  };
  const std::array clocks{3.0, 2.5, 2.0, 1.5, 1.0, 1.0, 1.0, 0.5, 0.0, -0.5, -1.0, -1.5};
  const auto level = [&](const app::LevelFrameView& f) {
    ++frames;
    if (f.frame >= static_cast<int>(clocks.size())) { ok = false; quit(); return; }
    ok &= f.entryCueSeconds == clocks[f.frame];
    ok &= f.entryCueFrame.has_value() == (f.frame >= 3 && f.frame <= 10);
    ok &= f.paused == (f.frame == 5 || f.frame == 6);
    if (f.entryCueFrame) {
      ok &= f.entryCueFrame->additive == (f.frame == 9 || f.frame == 10);
      if (f.frame >= 7) ok &= f.entryCueFrame->rect == std::array<int, 4>{160, 180, 480, 260};
    }
    if (f.frame >= 4 && f.frame <= 10) {
      Image expected = white;
      cursor->draw(expected, f.cursorState, f.cursorX, f.cursorY);
      if (f.paused) shell::drawCaption(expected, *font, cells);
      for (const auto& [x, y] : (f.frame <= 6 ? largeOpaque : opaque)) for (int channel = 0; channel < 3; ++channel) {
        const auto i = (y * 640 + x) * 4 + channel;
        ok &= f.image.rgba[i] == expected.rgba[i];
      }
    }
    if (f.frame == 4 || f.frame == 6) ok &= key(SDLK_p);
    if (f.frame == 5) firstPaused = f.image;
    if (f.frame == 6) ok &= firstPaused.rgba == f.image.rgba;
    if (f.frame == 11) {
      int changed = 0;
      for (const auto& [x, y] : opaque) {
        const auto i = (y * 640 + x) * 4;
        if (f.image.rgba[i] != 255 || f.image.rgba[i + 1] != 255 || f.image.rgba[i + 2] != 255) ++changed;
      }
      ok &= changed > 20; // positive control: retired cue must no longer cover the scene
      ok &= quit();
    }
    if (!ok) { std::cerr << "entry cue mismatch at frame " << f.frame << '\n'; quit(); }
  };
  std::vector<std::string> args{"entry-cue-test", "--frontend", "--frames", "16", "--game-dir", game};
  std::vector<char*> argv; for (auto& arg : args) argv.push_back(arg.data());
  const int result = app::runApplication(static_cast<int>(argv.size()), argv.data(), front, level,
                                        {.frameSeconds = 0.5});
  if (result != 0 || !ok || frames != 12) return 1;
  std::cout << "PASS: actual-app cue timing, pause, texture alpha, composition and retirement\n";
  return 0;
}
