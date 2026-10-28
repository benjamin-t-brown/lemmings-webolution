#include "app/FrontendSession.h"
#include "app/FrontendWindow.h"
#include "assets/Box.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <utility>

namespace fs = std::filesystem;

int main() {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "FrontendWindow test requires SDL2\n";
  return 1;
#else
  const char* configured = std::getenv("WEBOLUTION_GAME_DIR");
  if (!configured || !*configured) {
    std::cout << "FrontendWindow: SKIPPED (set WEBOLUTION_GAME_DIR to retail assets)\n";
    return 0;
  }
  const fs::path game(configured);
  auto library = webo::AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!library) { std::cerr << library.error() << '\n'; return 1; }
  auto createSession = [&]() -> std::optional<webo::app::FrontendSession> {
    auto session = webo::app::FrontendSession::create(*library);
    if (!session) { std::cerr << session.error() << '\n'; return std::nullopt; }
    return std::move(*session);
  };

  auto session = createSession();
  if (!session) return 1;
  bool sequenceOk = true;
  auto push = [](Uint32 type, int x = 0, int y = 0, bool repeat = false) {
    SDL_Event event{};
    if (type == SDL_KEYDOWN) {
      event.type = type; event.key.type = type; event.key.repeat = repeat;
      event.key.keysym.sym = SDLK_RETURN;
    } else if (type == SDL_MOUSEBUTTONDOWN) {
      event.type = type; event.button.type = type; event.button.button = SDL_BUTTON_LEFT;
      event.button.x = x; event.button.y = y;
    } else event.type = type;
    return SDL_PushEvent(&event) == 1;
  };
  auto drive = [&](const webo::app::FrontendSession& current, int frame) {
    using webo::shell::Screen;
    const auto screen = current.screen();
    sequenceOk &= current.constructionCounter() == (frame < 1 ? 3 : frame < 2 ? 4 : 5);
    sequenceOk &= screen == (frame == 0 ? Screen::Menu : frame == 1 ? Screen::Confirm : Screen::Map);
    if (frame == 0) { sequenceOk &= push(SDL_KEYDOWN, 0, 0, true); sequenceOk &= push(SDL_KEYDOWN); }
    else if (frame == 1) sequenceOk &= push(SDL_KEYDOWN);
    else if (frame == 2) sequenceOk &= push(SDL_KEYDOWN, 0, 0, true); // repeats never activate footer
    else if (frame == 3) {
      sequenceOk &= push(SDL_MOUSEBUTTONDOWN, 0, 0);
      sequenceOk &= push(SDL_MOUSEBUTTONDOWN, 206, 204);
    }
  };
  auto selected = webo::app::runFrontendWindow(*session, {.maxFrames = 8, .finalFramePath = {}}, drive);
  if (!selected || *selected != std::optional<std::string>{"MM_CLIMB_002"} || !sequenceOk ||
      session->screen() != webo::shell::Screen::Level || session->constructionCounter() != 5) {
    std::cerr << "SDL frontend event sequence or selected route mismatch\n"; return 1;
  }

  auto closing = createSession();
  if (!closing) return 1;
  bool closeSent = false;
  auto closed = webo::app::runFrontendWindow(*closing, {}, [&](const auto&, int) {
    if (!closeSent) { closeSent = true; push(SDL_QUIT); }
  });
  if (!closed || *closed || !closeSent) { std::cerr << "SDL close route mismatch\n"; return 1; }

  auto capped = createSession();
  if (!capped) return 1;
  auto result = webo::app::runFrontendWindow(*capped, {.maxFrames = 1, .finalFramePath = {}});
  if (!result || *result || capped->constructionCounter() != 3) {
    std::cerr << "SDL capped route mismatch\n"; return 1;
  }
  auto unfocused = createSession();
  if (!unfocused) return 1;
  bool focusOk = true;
  auto focus = [&](Uint8 change) {
    SDL_Event event{};
    event.type = SDL_WINDOWEVENT;
    event.window.event = change;
    focusOk &= SDL_PushEvent(&event) == 1;
  };
  auto focusResult = webo::app::runFrontendWindow(*unfocused,
      {.maxFrames = 4, .finalFramePath = {}}, [&](const auto& current, int frame) {
        if (frame == 0) { focus(SDL_WINDOWEVENT_FOCUS_LOST); focusOk &= push(SDL_KEYDOWN); }
        if (frame == 1) {
          focusOk &= current.screen() == webo::shell::Screen::Menu;
          focus(SDL_WINDOWEVENT_FOCUS_GAINED); focusOk &= push(SDL_KEYDOWN);
        }
        if (frame == 2) {
          focusOk &= current.screen() == webo::shell::Screen::Confirm;
          focusOk &= push(SDL_QUIT);
        }
      });
  if (!focusResult || *focusResult || !focusOk) {
    std::cerr << "SDL focus guard mismatch\n"; return 1;
  }
  auto cancel = createSession();
  if (!cancel) return 1;
  bool cancelOk = true;
  auto cancelled = webo::app::runFrontendWindow(*cancel,
      {.maxFrames = 4, .finalFramePath = {}}, [&](const auto& current, int frame) {
        if (frame == 0) cancelOk &= push(SDL_MOUSEBUTTONDOWN, 320, 333);
        if (frame == 1) {
          cancelOk &= current.screen() == webo::shell::Screen::Confirm && current.constructionCounter() == 4;
          cancelOk &= push(SDL_MOUSEBUTTONDOWN, 320, 340);
        }
        if (frame == 2) {
          cancelOk &= current.screen() == webo::shell::Screen::Menu && current.constructionCounter() == 5;
          cancelOk &= push(SDL_QUIT);
        }
      });
  if (!cancelled || *cancelled || !cancelOk) {
    std::cerr << "SDL New Game/No click route mismatch\n"; return 1;
  }
  auto footer = createSession();
  if (!footer) return 1;
  bool footerOk = true;
  int footerFrames = 0;
  const std::array screens{webo::shell::Screen::Menu, webo::shell::Screen::Confirm,
      webo::shell::Screen::Map, webo::shell::Screen::Menu, webo::shell::Screen::Confirm,
      webo::shell::Screen::Map, webo::shell::Screen::Menu};
  auto footerResult = webo::app::runFrontendWindow(*footer,
      {.maxFrames = 8, .finalFramePath = {}}, [&](const auto& current, int frame) {
        ++footerFrames;
        if (frame >= static_cast<int>(screens.size())) { footerOk = false; push(SDL_QUIT); return; }
        footerOk &= current.screen() == screens[frame] && current.constructionCounter() == frame + 3;
        if (frame == 2) footerOk &= push(SDL_MOUSEBUTTONDOWN, 320, 455);
        else if (frame == 6) footerOk &= push(SDL_QUIT);
        else footerOk &= push(SDL_KEYDOWN);
      });
  if (!footerResult || *footerResult || !footerOk || footerFrames != 7) {
    std::cerr << "SDL map footer click/Return route mismatch\n"; return 1;
  }
  // Known-value result fixture through the real SDL frontend dispatcher. This
  // is not a simulated victory or native input observation (levelreturn covers
  // natural application results separately).
  auto lossQuit = createSession();
  if (!lossQuit) return 1;
  lossQuit->pressReturn(); lossQuit->pressReturn(); lossQuit->clickMap(206, 204);
  if (!lossQuit->completeLevel(*library, {"MM_CLIMB_002", 10, 10, 1, 10, true})) return 1;
  lossQuit->click(320, 360); lossQuit->clickMap(206, 204);
  if (!lossQuit->completeLevel(*library, {"MM_CLIMB_002", 0, 10, 1, 0, false})) return 1;
  const auto earned = lossQuit->completionFlags();
  bool quitOk = true;
  int quitFrames = 0;
  using webo::shell::Screen;
  const std::array quitScreens{Screen::Results, Screen::Map, Screen::Menu,
      Screen::Confirm, Screen::Menu, Screen::Confirm, Screen::Map};
  auto quitResult = webo::app::runFrontendWindow(*lossQuit,
      {.maxFrames = 9, .finalFramePath = {}}, [&](const auto& current, int frame) {
        ++quitFrames;
        if (frame >= static_cast<int>(quitScreens.size())) { quitOk = false; push(SDL_QUIT); return; }
        quitOk &= current.screen() == quitScreens[frame] && !current.selectedLevel();
        if (frame < 6) quitOk &= current.completionFlags() == earned;
        if (frame > 0) quitOk &= !current.outcome();
        if (frame == 0) quitOk &= push(SDL_MOUSEBUTTONDOWN, 320, 420);
        else if (frame == 1) quitOk &= push(SDL_MOUSEBUTTONDOWN, 320, 455);
        else if (frame == 3) quitOk &= push(SDL_MOUSEBUTTONDOWN, 320, 340); // New Game No
        else if (frame == 6) {
          quitOk &= current.completionFlags() == std::array<std::int32_t, 102>{};
          quitOk &= push(SDL_MOUSEBUTTONDOWN, 206, 204);
        } else quitOk &= push(SDL_KEYDOWN);
      });
  if (!quitResult || *quitResult != std::optional<std::string>{"MM_CLIMB_002"} ||
      !quitOk || quitFrames != 7 || !lossQuit->levelIntroEnabled()) {
    std::cerr << "SDL loss Quit/map/footer/New Game progress/launch route mismatch\n"; return 1;
  }
  auto invalid = webo::app::runFrontendWindow(*capped,
      {.maxFrames = 0, .finalFramePath = "must-not-write.png"});
  if (invalid) { std::cerr << "unbounded screenshot request accepted\n"; return 1; }
  std::cout << "PASS: focused SDL frontend Return/map input, close, and frame cap\n";
  return 0;
#endif
}
