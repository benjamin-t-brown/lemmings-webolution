#include "app/AppEntry.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#include "HatchEntryFixture.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

bool pushKey() {
  SDL_Event event{};
  event.type = SDL_KEYDOWN;
  event.key.type = SDL_KEYDOWN;
  event.key.keysym.sym = SDLK_RETURN;
  return SDL_PushEvent(&event) == 1;
}

bool pushQuit() {
  SDL_Event event{};
  event.type = SDL_QUIT;
  return SDL_PushEvent(&event) == 1;
}

int setGameDir(const char* path) {
  // Match the application's std::getenv CRT. SDL's Windows environment API
  // need not update the environment cached by the executable's UCRT.
#ifdef _WIN32
  return _putenv_s("WEBOLUTION_GAME_DIR", path);
#else
  return setenv("WEBOLUTION_GAME_DIR", path, 1);
#endif
}

bool pushMapClick(int x = 206, int y = 204) {
  SDL_Event event{};
  event.type = SDL_MOUSEBUTTONDOWN;
  event.button.type = SDL_MOUSEBUTTONDOWN;
  event.button.button = SDL_BUTTON_LEFT;
  event.button.x = x;
  event.button.y = y;
  return SDL_PushEvent(&event) == 1;
}

std::vector<char> bytes(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

int invoke(const std::vector<std::string>& args,
           webo::app::FrontendWindowObserver observer = {},
           webo::app::LevelFrameObserver levelObserver = {}) {
  std::vector<char*> argv;
  argv.reserve(args.size());
  for (const auto& arg : args)
    argv.push_back(const_cast<char*>(arg.c_str()));
  return webo::app::runApplication(static_cast<int>(argv.size()), argv.data(),
                                   std::move(observer), std::move(levelObserver));
}

}  // namespace

int main(int argc, char** argv) {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "FrontendEntry test requires SDL2\n";
  return 1;
#else
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir) {
    std::cout << "FrontendEntry: SKIPPED (set WEBOLUTION_GAME_DIR to retail assets)\n";
    return 0;
  }
  // This test must never initialize SDL against the user's desktop, including
  // when launched manually instead of through the configured CTest environment.
  const char* videoDriver = std::getenv("SDL_VIDEODRIVER");
  if (!videoDriver || std::string(videoDriver) != "dummy") {
    std::cerr << "FrontendEntry: refusing to run without SDL_VIDEODRIVER=dummy\n";
    return 1;
  }
  if (argc < 2 || !argv[1] || !*argv[1]) {
    std::cerr << "FrontendEntry: expected test output directory in argv[1]\n";
    return 1;
  }
  const fs::path outDir(argv[1]);
  std::error_code ec;
  fs::create_directories(outDir, ec);
  if (ec) { std::cerr << "cannot create test output directory: " << ec.message() << '\n'; return 1; }

  // A fresh exclusive child lets CTest repeat without overwriting earlier
  // diagnostic evidence or cleaning anything owned by the caller.
  const auto stamp = std::chrono::system_clock::now().time_since_epoch().count();
  const fs::path runDir = outDir / ("run-" + std::to_string(stamp));
  if (!fs::create_directory(runDir, ec) || ec) {
    std::cerr << "FrontendEntry: cannot create fresh output directory\n";
    return 1;
  }
  std::cout << "frontend entry evidence: " << runDir.string() << '\n';
  const fs::path entryShot = runDir / "entry.png";
  const fs::path resetShot = runDir / "entry-reset.png";
  const fs::path expectedShot = runDir / "expected.png";
  const fs::path expectedZeroShot = runDir / "expected-zero.png";
  const fs::path expectedCamera = runDir / "expected-camera.json";
  // Independent settled-pose equivalent of the opening move on frame30:
  // 29 prior frame deltas (pose is computed before increment), no yaw change
  // before intro second1. This does not invoke CameraIntro/buildCamera's intro
  // branch, and preserves the original direct-level diagnostic render route.
  double introSeconds = 0.0;
  for (int i = 0; i < 29; ++i) introSeconds += 1.0 / 144.0;
  const double q = introSeconds / 2.0;
  const double distance = 377.516 + (350.0 - (2.0-q)*q*350.0);
  {
    std::ofstream cameraFile(expectedCamera);
    cameraFile << std::setprecision(17) << "{\"eye_distance\":" << distance << "}\n";
    if (!cameraFile) { std::cerr << "cannot write entry camera oracle\n"; return 1; }
  }

  bool traceOk = true;
  int frameCalls = 0;
  int mapX = 206, mapY = 204;
  webo::Image explicitMenu;
  const auto observer = [&](const webo::app::FrontendSession& session, int frame) {
    ++frameCalls;
    if (frameCalls > 3) traceOk = false;
    const auto screen = session.screen();
    const auto counter = session.constructionCounter();
    if (frame == 0) {
      traceOk &= screen == webo::shell::Screen::Menu && counter == 3;
      traceOk &= session.menuFocus() == 0 && !session.selectedLevel() &&
          std::all_of(session.completionFlags().begin(), session.completionFlags().end(),
                      [](auto flag) { return flag == 0; });
      const auto menu = session.composeFrame();
      if (explicitMenu.rgba.empty()) explicitMenu = menu;
      traceOk &= menu.width == 640 && menu.height == 480 && menu.rgba == explicitMenu.rgba;
      traceOk &= pushKey();
    } else if (frame == 1) {
      traceOk &= screen == webo::shell::Screen::Confirm && counter == 4;
      traceOk &= pushKey();
    } else if (frame == 2) {
      traceOk &= screen == webo::shell::Screen::Map && counter == 5;
      traceOk &= pushMapClick(mapX, mapY);
    } else {
      traceOk = false;
    }
  };

  const int entryResult = invoke({"webolution-test", "--frontend", "--level", "MM_DIG_000",
      "--frames", "30", "--window-shot", entryShot.string(),
      "--game-dir", gameDir}, observer);
  bool ok = entryResult == 0 && traceOk && frameCalls == 3 && fs::is_regular_file(entryShot);
  if (!ok) std::cerr << "FAIL: production frontend entry route (result=" << entryResult
                      << ", observer frames=" << frameCalls << ")\n";

  traceOk = true;
  frameCalls = 0;
  const int resetResult = invoke({"webolution-test", "--frontend", "--level", "MM_DIG_000",
      "--sim-ticks", "20", "--frames", "30", "--window-shot", resetShot.string(),
      "--game-dir", gameDir}, observer);
  ok &= resetResult == 0 && traceOk && frameCalls == 3 && fs::is_regular_file(resetShot);

  const auto render = [&](const fs::path& path, int ticks) {
    const int result = invoke({"webolution-test", "--level", "MM_CLIMB_002", "--native", "--sim-ticks",
        std::to_string(ticks), "--skill-panel", "--hud-ticks", std::to_string(ticks), "--cursor-fixture", "--shot", path.string(),
        "--camera", expectedCamera.string(), "--game-dir", gameDir});
    if (result != 0) return result;
    // P9: the direct diagnostic still renders a closed hatch. Retain that
    // control and compare it to an independent compositor before sampling
    // doors at frame 30's 208 ms. All full-frame equality checks below stay.
    auto library = webo::AssetLibrary::open(fs::path(gameDir) / "lemmings.box",
                                           fs::path(gameDir) / "patch.box");
    webo::app::CameraParams pose;
    std::string error;
    if (!library || !webo::app::loadCameraParams(expectedCamera, pose, error)) return 1;
    const auto closed = webo::encodePng(webo::test::hatchEntryFixture(*library, pose, ticks, 0));
    const auto original = bytes(path);
    if (closed.size() != original.size() ||
        !std::equal(closed.begin(), closed.end(), original.begin(),
                    [](auto a, char b) { return a == static_cast<unsigned char>(b); })) {
      std::cerr << "FAIL: zero-time hatch fixture differs from direct render\n";
      return 1;
    }
    fs::copy_file(path, path.parent_path() / ("closed-" + path.filename().string()));
    // Connected presentation uses frame seconds (30/144), while the unchanged
    // World/HUD clock admits 208 integer ms. The old 208 ms sample fitted a
    // door clip to the World arm interval and encoded playtest R2.
    const auto sampled = webo::test::hatchEntryFixture(*library, pose, ticks, ticks ? 30.0/144.0*1000.0 : 0, true);
    return webo::writePng(path, sampled).has_value() ? 0 : 1;
  };
  const int zeroResult = render(expectedZeroShot, 0);
  const int expectedResult = render(expectedShot, 1);
  const auto actualBytes = bytes(entryShot);
  const auto resetBytes = bytes(resetShot);
  const auto expectedBytes = bytes(expectedShot);
  const auto zeroBytes = bytes(expectedZeroShot);
  ok &= zeroResult == 0 && expectedResult == 0 && !actualBytes.empty() &&
        !expectedBytes.empty() && !zeroBytes.empty();
  ok &= actualBytes == expectedBytes && resetBytes == expectedBytes;
  ok &= expectedBytes != zeroBytes;  // positive control: one sim tick changes the render
  if (actualBytes != expectedBytes)
    std::cerr << "FAIL: frontend-selected level output differs from direct retail render\n";
  if (resetBytes != expectedBytes)
    std::cerr << "FAIL: diagnostic-start entry differs from fresh retail render\n";
  if (expectedBytes == zeroBytes)
    std::cerr << "FAIL: tick-0 positive control matched tick-1 render\n";

  // Unbounded invocations use owned-process observer events, not --frames:
  // that option deliberately retains the direct diagnostic route.
  const auto menuOnly = [&](const std::vector<std::string>& args) {
    int menuCalls = 0, levelCalls = 0;
    bool menuOk = true;
    const int result = invoke(args, [&](const auto& session, int frame) {
      ++menuCalls;
      const auto image = session.composeFrame();
      menuOk &= frame == 0 && session.screen() == webo::shell::Screen::Menu &&
          session.constructionCounter() == 3 && session.menuFocus() == 0 &&
          !session.selectedLevel() && session.focused() && !session.leftHeld() &&
          !session.rightHeld() && session.pointerX() == 320 && session.pointerY() == 240 &&
          std::all_of(session.completionFlags().begin(), session.completionFlags().end(),
                      [](auto flag) { return flag == 0; }) &&
          image.width == explicitMenu.width && image.height == explicitMenu.height &&
          image.rgba == explicitMenu.rgba;
      menuOk &= pushQuit();
    }, [&](const auto&) { ++levelCalls; menuOk &= pushQuit(); });
    const bool passed = result == 0 && menuOk && menuCalls == 1 && levelCalls == 0;
    if (!passed) std::cerr << "FAIL: initial Menu route for " << args.back() << '\n';
    return passed;
  };
  ok &= menuOnly({"webolution-test", "--game-dir", gameDir});
  // Explicit frontend wins in either order, including with diagnostics.
  ok &= menuOnly({"webolution-test", "--frontend", "--direct-level", "--native"});
  ok &= menuOnly({"webolution-test", "--direct-level", "--level", "MM_DIG_000", "--frontend"});

  // Choose another unlocked node so default-level fallback cannot satisfy the
  // no-option handoff. Keep the two existing deterministic render/reset runs.
  auto library = webo::AssetLibrary::open(fs::path(gameDir) / "lemmings.box",
                                         fs::path(gameDir) / "patch.box");
  if (!library) { std::cerr << "FAIL: default-entry assets: " << library.error() << '\n'; return 1; }
  auto map = webo::app::MapScene::build(*library, {});
  if (!map) { std::cerr << "FAIL: default-entry map: " << map.error() << '\n'; return 1; }
  mapX = mapY = -1;
  for (int y = 0; y < 480 && mapX < 0; ++y)
    for (int x = 0; x < 640; ++x)
      if (map->pickNode(x, y) == 1) { mapX = x; mapY = y; break; }
  const auto selectedName = map->levelForNode(1);
  if (mapX < 0 || !selectedName || *selectedName == "MM_CLIMB_002") {
    std::cerr << "FAIL: second unlocked node must identify another retail level\n"; return 1;
  }
  const auto selectedSetup = webo::sim::loadRetailLevel(*library, *selectedName);
  if (!selectedSetup) { std::cerr << "FAIL: selected retail setup\n"; return 1; }
  traceOk = true;
  frameCalls = 0;
  int selectedCalls = 0;
  const int defaultResult = invoke({"webolution-test"}, observer, [&](const auto& frame) {
    ++selectedCalls;
    traceOk &= frame.frame == 0 && frame.levelName == *selectedName &&
        frame.frontendConstructionCounter == 5 && !frame.paused && !frame.levelMenuOpen &&
        frame.selectedSkillIndex == 0 && frame.world.saved() == 0 && frame.world.dead() == 0 &&
        frame.world.setup().totalLemmings == selectedSetup->totalLemmings &&
        frame.world.setup().skills == selectedSetup->skills &&
        frame.world.terrain().width() == selectedSetup->terrain.width() &&
        frame.world.terrain().height() == selectedSetup->terrain.height();
    // The unbounded production clock stays real. The deterministic 20-tick
    // reset and tick-0/tick-1 image controls above establish the fresh reset.
    traceOk &= pushQuit();
  });
  const bool defaultOk = defaultResult == 0 && traceOk && frameCalls == 3 && selectedCalls == 1;
  ok &= defaultOk;
  if (!defaultOk) std::cerr << "FAIL: no-option Menu/confirmation/map/selected simulation route\n";

  // Ordinary startup also reaches the same selected simulation through the
  // SOURCE Play Current Game mouse adapter, without New Game confirmation.
  int mouseMenuCalls = 0, mouseLevelCalls = 0, mouseQuits = 0;
  bool mouseOk = true;
  const int mouseResult = invoke({"webolution-test"}, [&](const auto& session, int frame) {
    ++mouseMenuCalls;
    mouseOk &= !session.selectedLevel() && session.focused() &&
        std::all_of(session.completionFlags().begin(), session.completionFlags().end(),
                    [](auto flag) { return flag == 0; });
    if (frame == 0) {
      mouseOk &= session.screen() == webo::shell::Screen::Menu &&
          session.constructionCounter() == 3 && session.menuFocus() == 0;
      mouseOk &= pushMapClick(320, 363);
    } else if (frame == 1) {
      mouseOk &= session.screen() == webo::shell::Screen::Map &&
          session.constructionCounter() == 4 && session.menuFocus() == 1;
      mouseOk &= pushMapClick(mapX, mapY);
    } else { mouseOk = false; pushQuit(); }
  }, [&](const auto& frame) {
    ++mouseLevelCalls;
    mouseOk &= frame.frame == 0 && frame.levelName == *selectedName &&
        frame.frontendConstructionCounter == 4 && !frame.paused && !frame.levelMenuOpen &&
        frame.selectedSkillIndex == 0 && frame.world.saved() == 0 && frame.world.dead() == 0 &&
        frame.world.setup().totalLemmings == selectedSetup->totalLemmings &&
        frame.world.setup().skills == selectedSetup->skills &&
        frame.world.terrain().width() == selectedSetup->terrain.width() &&
        frame.world.terrain().height() == selectedSetup->terrain.height();
    if (pushQuit()) ++mouseQuits; else mouseOk = false;
  });
  std::cout << "SOURCE ordinary current-game mouse route frontendCallbacks=" << mouseMenuCalls
            << " levelCallbacks=" << mouseLevelCalls << " quitEvents=" << mouseQuits << '\n';
  const bool mousePassed = mouseResult == 0 && mouseOk && mouseMenuCalls == 2 &&
                           mouseLevelCalls == 1 && mouseQuits == 1;
  ok &= mousePassed;
  if (!mousePassed) std::cerr << "FAIL: ordinary current-game mouse/selected fresh simulation route\n";

  // Direct --sim/--sim-ticks preserve their seeded World and have no session.
  for (const bool seeded : {false, true}) {
    int menuCalls = 0, levelCalls = 0;
    bool directOk = true;
    std::vector<std::string> args{"webolution-test", seeded ? "--sim-ticks" : "--sim"};
    if (seeded) args.emplace_back("20");
    const int result = invoke(args,
        [&](const auto&, int) { ++menuCalls; directOk &= pushQuit(); },
        [&](const auto& frame) {
          ++levelCalls;
          directOk &= frame.frame == 0 && frame.levelName == "MM_CLIMB_002" &&
              !frame.frontendConstructionCounter && !frame.cameraIntroActive;
          if (seeded) directOk &= frame.world.tickCount() >= 20;
          directOk &= pushQuit();
        });
    ok &= result == 0 && directOk && menuCalls == 0 && levelCalls == 1;
    if (result != 0 || !directOk || menuCalls != 0 || levelCalls != 1)
      std::cerr << "FAIL: preserved direct simulation route\n";
  }

  const auto fixtureShot = runDir / "fixture.png";
  const auto directShot = runDir / "direct-level.png";
  const auto mapShot = runDir / "map.png";
  const auto expectedMapShot = runDir / "expected-map.png";
  const auto menuShot = runDir / "explicit-menu.png";
  const auto expectedMenuShot = runDir / "expected-menu.png";
  ok &= invoke({"webolution-test", "--shot", fixtureShot.string()}) == 0;
  // Unknown options remain ignored, with their previous diagnostic routing.
  ok &= invoke({"webolution-test", "--direct-level", "--native-017-unknown",
                "--shot", directShot.string()}) == 0;
  ok &= !bytes(fixtureShot).empty() && bytes(fixtureShot) == bytes(directShot);
  ok &= invoke({"webolution-test", "--map", "--shot", mapShot.string()}) == 0;
  ok &= webo::writePng(expectedMapShot.string(), map->image()).has_value();
  ok &= !bytes(mapShot).empty() && bytes(mapShot) == bytes(expectedMapShot);
  ok &= invoke({"webolution-test", "--direct-level", "--frontend", "--shot", menuShot.string()}) == 0;
  ok &= webo::writePng(expectedMenuShot.string(), explicitMenu).has_value();
  ok &= !bytes(menuShot).empty() && bytes(menuShot) == bytes(expectedMenuShot);
  ok &= invoke({"webolution-test", "--frontend", "--map"}) == 1;
  ok &= invoke({"webolution-test", "--map-flags", "", "--frontend"}) == 1;
  ok &= invoke({"webolution-test", "--map-flags", ""}) == 1;

  // No installed files are touched: this directory does not exist. Help exits
  // before opening assets, and both default entry forms report ordinary errors.
  const std::string retailDir(gameDir);
  const auto missingDir = (runDir / "missing-assets").string();
  ok &= invoke({"webolution-test", "--game-dir", missingDir}) == 1;
  if (setGameDir(missingDir.c_str()) != 0) return 1;
  const bool absentOk = invoke({"webolution-test"}) == 1 &&
      invoke({"webolution-test", "--help"}) == 0 && invoke({"webolution-test", "-h"}) == 0;
  if (setGameDir(retailDir.c_str()) != 0) return 1;
  ok &= absentOk;

  if (ok) std::cout << "PASS: production frontend selection reaches matching retail level render\n";
  return ok ? 0 : 1;
#endif
}
