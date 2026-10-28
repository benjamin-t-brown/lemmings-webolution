#include "app/FrontendSession.h"
#include "app/FrontendWindow.h"
#include "assets/Box.h"
#include "assets/Png.h"

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
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

int main() {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "FrontendWindow test requires SDL2\n";
  return 1;
#else
  const char* driver = std::getenv("SDL_VIDEODRIVER");
  if (!driver || std::string_view(driver) != "dummy") {
    std::cerr << "FrontendWindow: refusing to run without SDL_VIDEODRIVER=dummy\n";
    return 1;
  }
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
  auto mouse = [](Uint32 type, Uint8 button, int x, int y) {
    SDL_Event event{};
    event.type = type;
    event.button.type = type;
    event.button.button = button;
    event.button.x = x;
    event.button.y = y;
    return SDL_PushEvent(&event) == 1;
  };
  auto motion = [](int x, int y, int xrel, int yrel) {
    SDL_Event event{};
    event.type = SDL_MOUSEMOTION;
    event.motion.type = SDL_MOUSEMOTION;
    event.motion.x = x; event.motion.y = y;
    event.motion.xrel = xrel; event.motion.yrel = yrel;
    return SDL_PushEvent(&event) == 1;
  };
  auto softwareCursorActive = [] {
    return SDL_ShowCursor(SDL_QUERY) == SDL_DISABLE;
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
  const char* outputConfigured = std::getenv("WEBOLUTION_TEST_OUTPUT_DIR");
  if (!outputConfigured || !*outputConfigured) {
    std::cerr << "frontendwindow test requires CTest-owned WEBOLUTION_TEST_OUTPUT_DIR\n"; return 1;
  }
  const fs::path outputDir(outputConfigured);
  fs::create_directories(outputDir);
  auto screenshot = createSession();
  if (!screenshot) return 1;
  const fs::path actualPng = outputDir / "cursor-window.png";
  const fs::path expectedPng = outputDir / "cursor-expected.png";
  std::optional<webo::Image> observedFrame;
  bool screenshotOk = true;
  auto screenshotResult = webo::app::runFrontendWindow(*screenshot,
      {.maxFrames = 2, .finalFramePath = actualPng.string()}, [&](const auto& current, int frame) {
        screenshotOk &= softwareCursorActive();
        if (frame == 0) screenshotOk &= motion(187, 193, 0, 0);
        if (frame == 1) {
          screenshotOk &= current.pointerX() == 187 && current.pointerY() == 193;
          observedFrame = current.composeFrame();
        }
      });
  if (!screenshotResult || *screenshotResult || !screenshotOk || !observedFrame ||
      !webo::writePng(expectedPng, *observedFrame)) {
    std::cerr << "frontend final-frame cursor capture setup failed\n"; return 1;
  }
  auto bytes = [](const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return std::vector<char>(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
  };
  if (bytes(actualPng) != bytes(expectedPng)) {
    std::cerr << "final frontend PNG differs from observer's moved-cursor composed frame\n"; return 1;
  }
  auto pointer = createSession();
  if (!pointer) return 1;
  bool pointerOk = true;
  auto pointerResult = webo::app::runFrontendWindow(*pointer,
      {.maxFrames = 7, .finalFramePath = {}}, [&](const auto& current, int frame) {
        using webo::app::CursorState;
        if (frame == 0) {
          // Absolute motion with zero relative delta is still the pointer/hit coordinate.
          pointerOk &= motion(206, 204, 0, 0);
          pointerOk &= mouse(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT, 206, 204);
          pointerOk &= current.constructionCounter() == 3 && !current.selectedLevel();
          pointerOk &= current.cursorState() == CursorState::NoLemming;
        } else if (frame == 1) {
          pointerOk &= current.pointerX() == 206 && current.pointerY() == 204;
          pointerOk &= current.cursorState() == CursorState::MouseDown && !current.selectedLevel();
          // Motion must continue while right is held (unlike gameplay Grab).
          pointerOk &= motion(220, 215, 0, 0);
        } else if (frame == 2) {
          pointerOk &= current.pointerX() == 220 && current.pointerY() == 215;
          pointerOk &= current.cursorState() == CursorState::MouseDown;
          pointerOk &= mouse(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 0, 0);
        } else if (frame == 3) {
          pointerOk &= current.cursorState() == CursorState::MouseDown;
          pointerOk &= current.leftHeld() && current.rightHeld();
          pointerOk &= mouse(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, 0, 0);
          pointerOk &= mouse(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 320, 333);
        } else if (frame == 4) {
          pointerOk &= current.screen() == webo::shell::Screen::Confirm;
          pointerOk &= current.cursorState() == CursorState::MouseDown;
          pointerOk &= current.leftHeld() && current.rightHeld();
          pointerOk &= current.pointerX() == 320 && current.pointerY() == 333;
          pointerOk &= mouse(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT, 320, 333);
          pointerOk &= mouse(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 320, 340);
        } else if (frame == 5) {
          pointerOk &= current.screen() == webo::shell::Screen::Menu &&
                       current.constructionCounter() == 5;
          pointerOk &= current.cursorState() == CursorState::MouseDown;
          pointerOk &= current.leftHeld() && !current.rightHeld();
          pointerOk &= current.pointerX() == 320 && current.pointerY() == 340;
          pointerOk &= push(SDL_QUIT);
        }
      });
  if (!pointerResult || *pointerResult || !pointerOk) {
    std::cerr << "SDL frontend cursor motion/button/transition route mismatch\n"; return 1;
  }
  bool recreationOk = false;
  auto recreated = webo::app::runFrontendWindow(*pointer,
      {.maxFrames = 1, .finalFramePath = {}}, [&](const auto& current, int frame) {
        recreationOk = frame == 0 && current.pointerX() == 320 && current.pointerY() == 340 &&
            current.focused() && !current.leftHeld() && !current.rightHeld() &&
            current.cursorState() == webo::app::CursorState::NoLemming && softwareCursorActive();
      });
  if (!recreated || *recreated || !recreationOk) {
    std::cerr << "same-session window recreation did not retain pointer and reset transient input\n"; return 1;
  }
  auto hover = createSession();
  if (!hover) return 1;
  bool hoverOk = true;
  std::array<std::int32_t, 102> hoverFlags{};
  std::array<std::int32_t, 102> hoverMapStates{};
  auto hoverResult = webo::app::runFrontendWindow(*hover,
      {.maxFrames = 6, .finalFramePath = {}}, [&](const auto& current, int frame) {
        using webo::shell::Screen;
        if (frame == 0) hoverOk &= push(SDL_KEYDOWN);
        else if (frame == 1) {
          hoverOk &= current.screen() == Screen::Confirm;
          hoverOk &= push(SDL_KEYDOWN);
        } else if (frame == 2) {
          hoverOk &= current.screen() == Screen::Map;
          hoverOk &= current.constructionCounter() == 5 && !current.selectedLevel();
          hoverFlags = current.completionFlags();
          hoverMapStates = current.mapStates();
          hoverOk &= motion(320, 455, 0, 0); // footer hover is presentation-only
        } else if (frame == 3) {
          hoverOk &= current.screen() == Screen::Map && current.constructionCounter() == 5;
          hoverOk &= !current.selectedLevel() && current.completionFlags() == hoverFlags &&
                     current.mapStates() == hoverMapStates;
          hoverOk &= current.pointerX() == 320 && current.pointerY() == 455;
          hoverOk &= motion(206, 204, 0, 0); // node hover is presentation-only
        } else if (frame == 4) {
          hoverOk &= current.screen() == Screen::Map && current.constructionCounter() == 5 &&
                     !current.selectedLevel() && current.completionFlags() == hoverFlags &&
                     current.mapStates() == hoverMapStates;
          hoverOk &= current.pointerX() == 206 && current.pointerY() == 204;
          hoverOk &= push(SDL_QUIT);
        }
      });
  if (!hoverResult || *hoverResult || !hoverOk) {
    std::cerr << "SDL frontend hover activated a node/footer or changed construction\n"; return 1;
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
      {.maxFrames = 6, .finalFramePath = {}}, [&](const auto& current, int frame) {
        focusOk &= softwareCursorActive();
        if (frame == 0) focusOk &= mouse(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT, 42, 43);
        if (frame == 1) {
          focusOk &= current.pointerX() == 42 && current.pointerY() == 43;
          focusOk &= current.cursorState() == webo::app::CursorState::MouseDown && current.rightHeld();
          focus(SDL_WINDOWEVENT_FOCUS_LOST);
          focusOk &= push(SDL_KEYDOWN);
          focusOk &= motion(500, 400, 10, 10);
          focusOk &= mouse(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, 320, 333);
          focusOk &= mouse(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT, 99, 99);
        }
        if (frame == 2) {
          focusOk &= !current.focused() && current.cursorState() == webo::app::CursorState::Hidden;
          focusOk &= !current.leftHeld() && !current.rightHeld();
          focusOk &= current.pointerX() == 42 && current.pointerY() == 43;
          focusOk &= current.screen() == webo::shell::Screen::Menu && current.constructionCounter() == 3;
          focus(SDL_WINDOWEVENT_FOCUS_GAINED);
        }
        if (frame == 3) {
          focusOk &= current.focused() && current.cursorState() == webo::app::CursorState::NoLemming;
          focusOk &= current.pointerX() == 42 && current.pointerY() == 43;
          focusOk &= current.screen() == webo::shell::Screen::Menu && current.constructionCounter() == 3;
          focusOk &= push(SDL_KEYDOWN);
        }
        if (frame == 4) {
          focusOk &= current.screen() == webo::shell::Screen::Confirm && current.constructionCounter() == 4;
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
  std::cout << "PASS: focused SDL frontend Return/map input, cursor states, close, and frame cap\n";
  return 0;
#endif
}
