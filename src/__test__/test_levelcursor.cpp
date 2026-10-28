#include "app/AppEntry.h"
#include "app/Pick.h"
#include "assets/Box.h"
#include "assets/Bitmap.h"
#include "sim/World.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#include <cstdlib>
#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {
#if WEBOLUTION_HAVE_SDL2
using namespace webo;

bool pushKey(SDL_Scancode scan, SDL_Keycode key) {
  SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.type = SDL_KEYDOWN;
  e.key.state = SDL_PRESSED; e.key.keysym.scancode = scan; e.key.keysym.sym = key;
  return SDL_PushEvent(&e) == 1;
}
bool pushMotion(int x, int y) {
  SDL_Event e{}; e.type = SDL_MOUSEMOTION; e.motion.type = SDL_MOUSEMOTION;
  e.motion.x = x; e.motion.y = y; e.motion.xrel = 0; e.motion.yrel = 0;
  return SDL_PushEvent(&e) == 1;
}
bool pushButton(Uint32 type, Uint8 button, int x, int y) {
  SDL_Event e{}; e.type = type; e.button.type = type; e.button.button = button;
  e.button.state = type == SDL_MOUSEBUTTONDOWN ? SDL_PRESSED : SDL_RELEASED;
  e.button.x = x; e.button.y = y; return SDL_PushEvent(&e) == 1;
}
bool pushFocusLost() {
  SDL_Event e{}; e.type = SDL_WINDOWEVENT; e.window.type = SDL_WINDOWEVENT;
  e.window.event = SDL_WINDOWEVENT_FOCUS_LOST; return SDL_PushEvent(&e) == 1;
}
bool pushQuit() { SDL_Event e{}; e.type = SDL_QUIT; return SDL_PushEvent(&e) == 1; }

bool cursorPatchDiffers(const Image& a, const Image& b, int x, int y) {
  if (a.width != b.width || a.height != b.height || a.rgba.size() != b.rgba.size()) return false;
  for (int py = y - 16; py < y + 16; ++py) for (int px = x - 16; px < x + 16; ++px) {
    if (px < 0 || py < 0 || px >= a.width || py >= a.height) continue;
    const auto at = (static_cast<std::size_t>(py) * a.width + px) * 4;
    for (int c = 0; c < 4; ++c) if (a.rgba[at + c] != b.rgba[at + c]) return true;
  }
  return false;
}
// Independent decoded-art oracle: every opaque texel must be present, not just
// an arbitrary patch difference that could come from a moving actor beneath it.
bool opaqueArtworkMatches(const Image& image, const Image& art, int x, int y) {
  int checked = 0;
  for (int sy = 0; sy < art.height; ++sy) for (int sx = 0; sx < art.width; ++sx) {
    const auto src = (static_cast<std::size_t>(sy) * art.width + sx) * 4;
    if (art.rgba[src + 3] != 255) continue;
    const int px = x - 16 + sx, py = y - 16 + sy;
    if (px < 0 || py < 0 || px >= image.width || py >= image.height) continue;
    const auto dst = (static_cast<std::size_t>(py) * image.width + px) * 4;
    for (int c = 0; c < 3; ++c) if (image.rgba[dst + c] != art.rgba[src + c]) return false;
    ++checked;
  }
  return checked > 0;
}
bool hasActor(const app::LevelFrameView& f, std::size_t n) {
  for (const auto& a : f.actors) if (a.index == n) return true;
  return false;
}
#endif
}

int main() {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "LevelCursor: requires SDL2\n"; return 1;
#else
  const char* driver = std::getenv("SDL_VIDEODRIVER");
  if (!driver || std::string(driver) != "dummy") {
    std::cerr << "LevelCursor: refusing to run without SDL_VIDEODRIVER=dummy\n"; return 1;
  }
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir || !std::filesystem::is_regular_file(std::filesystem::path(gameDir) / "lemmings.box")) {
    std::cout << "LevelCursor: SKIPPED (retail assets unavailable)\n"; return 0;
  }

  auto library = webo::AssetLibrary::open(std::filesystem::path(gameDir) / "lemmings.box",
                                        std::filesystem::path(gameDir) / "patch.box");
  if (!library) { std::cerr << library.error() << '\n'; return 1; }
  auto lemArt = webo::loadTexture(*library, "TEXTURES\\_CURLEM.BMP");
  auto noLemArt = webo::loadTexture(*library, "TEXTURES\\_CURNOLEM.BMP");
  if (!lemArt || !noLemArt) { std::cerr << "cannot decode cursor artwork\n"; return 1; }
  bool ok = true; std::string failure; int calls = 0;
  int actorX = 0, actorY = 0; Image hoverImage; int savedX = 0, savedY = 0;
  const auto fail = [&](const char* why) { if (ok) failure = why; ok = false; };
  const auto observer = [&](const app::LevelFrameView& f) {
    ++calls;
    if (f.frame != calls - 1 || calls > 10) { fail("unexpected frame sequence"); pushQuit(); return; }
    const auto& lemmings = f.world.lemmings();
    if (f.cursorState == webo::app::CursorState::Lemming &&
        !opaqueArtworkMatches(f.image, *lemArt, f.cursorX, f.cursorY)) fail("Lemming artwork pixels missing");
    if (f.cursorState == webo::app::CursorState::NoLemming &&
        !opaqueArtworkMatches(f.image, *noLemArt, f.cursorX, f.cursorY)) fail("NoLemming artwork pixels missing");
    switch (calls - 1) {
      case 0: {
        if (lemmings.empty() || !lemmings[0].active() || !hasActor(f, 0)) { fail("actor 0 is not live and visible"); pushQuit(); return; }
        const auto it = std::find_if(f.actors.begin(), f.actors.end(), [](const auto& a) { return a.index == 0; });
        actorX = static_cast<int>((it->placement.left + it->placement.right) / 2);
        actorY = static_cast<int>((it->placement.top + it->placement.bottom) / 2);
        if (actorX < 100 || actorX >= 640 || actorY < 0 || actorY >= 480) { fail("actor center is outside test region"); pushQuit(); return; }
        if (!pushKey(SDL_SCANCODE_P, SDLK_p) || !pushMotion(actorX, actorY)) fail("could not queue pause and actor motion");
        break;
      }
      case 1:
        if (!f.paused || f.hoveredActor != std::optional<std::size_t>{0} || f.cursorState != app::CursorState::Lemming || f.cursorX != actorX || f.cursorY != actorY)
          fail("zero-relative motion over actor did not report absolute hover and Lemming cursor");
        hoverImage = f.image; savedX = f.cursorX; savedY = f.cursorY;
        if (!pushButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT, actorX, actorY)) fail("could not queue eligible-actor grab");
        break;
      case 2:
        if (!f.paused || f.hoveredActor || f.cursorState != app::CursorState::Grab)
          fail("grab over eligible actor did not suppress hover");
        if (!pushButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, actorX, actorY) ||
            !pushButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT, actorX, actorY)) fail("could not queue grabbed eligible-actor click");
        break;
      case 3:
        if (!f.paused || lemmings[0].canClimb ||
            f.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)] != 10 ||
            f.hoveredActor != std::optional<std::size_t>{0} || f.cursorState != app::CursorState::Lemming)
          fail("grabbed click assigned an eligible actor or release did not restore hover");
        if (!pushButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, actorX, actorY)) fail("could not queue actor click");
        break;
      case 4:
        if (f.paused || lemmings.size() <= 0 || !lemmings[0].canClimb || f.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)] != 9)
          fail("actor click did not assign Climber and resume");
        if (f.hoveredActor || f.cursorState != app::CursorState::NoLemming || f.cursorX != actorX || f.cursorY != actorY)
          fail("assigned actor remained hover-eligible or cursor coordinates changed");
        if (!cursorPatchDiffers(hoverImage, f.image, savedX, savedY)) fail("hover cursor patch did not change after actor became ineligible");
        if (!pushKey(SDL_SCANCODE_P, SDLK_p) || !pushMotion(600, 40)) fail("could not queue pause and empty-space motion");
        break;
      case 5:
        if (!f.paused || f.hoveredActor || f.cursorState != app::CursorState::NoLemming || f.cursorX != 600 || f.cursorY != 40)
          fail("empty-space hover snapshot incorrect");
        if (!pushMotion(54, 28)) fail("could not queue available panel motion");
        break;
      case 6:
        if (!f.paused || f.hoveredActor || f.cursorState != app::CursorState::Lemming || f.cursorX != 54 || f.cursorY != 28 || f.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)] != 9)
          fail("available panel hover did not select Lemming cursor without actor hover");
        if (!pushMotion(54, 68)) fail("could not queue unavailable floater motion");
        break;
      case 7:
        if (!f.paused || f.hoveredActor || f.cursorState != app::CursorState::NoLemming || f.cursorX != 54 || f.cursorY != 68)
          fail("unavailable panel hover did not report NoLemming");
        if (!pushButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT, 54, 68) || !pushMotion(400, 200)) fail("could not queue grab input");
        break;
      case 8:
        if (!f.paused || f.hoveredActor || f.cursorState != app::CursorState::Grab || f.cursorX != 54 || f.cursorY != 68)
          fail("right-button grab did not freeze cursor and suppress actor hover");
        if (!pushButton(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, actorX, actorY) ||
            !pushButton(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT, 54, 68) || !pushFocusLost()) fail("could not queue grabbed click/release/focus loss");
        break;
      case 9:
        if (!f.paused || f.hoveredActor || f.cursorState != app::CursorState::Hidden || f.world.setup().skills[static_cast<std::size_t>(sim::Skill::Climber)] != 9 || !lemmings[0].canClimb)
          fail("focus loss did not hide cursor or grabbed click assigned another skill");
        if (!pushQuit()) fail("could not queue quit");
        break;
    }
  };

  std::vector<std::string> args{"levelcursor-test", "--level", "MM_CLIMB_002", "--native", "--sim-ticks", "20", "--frames", "16", "--game-dir", gameDir};
  std::vector<char*> raw; for (auto& arg : args) raw.push_back(arg.data());
  if (app::runApplication(static_cast<int>(raw.size()), raw.data(), {}, observer) != 0) fail("runApplication returned failure");
  if (calls != 10) fail("did not observe exactly ten frames");
  if (!ok) { std::cerr << "FAIL: LevelCursor: " << failure << '\n'; return 1; }
  std::cout << "PASS: SDL dummy cursor hover, panel, grab and focus-loss regression\n"; return 0;
#endif
}
