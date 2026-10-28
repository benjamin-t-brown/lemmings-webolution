// Application-level regression for the tube scene's hidden-actor visibility
// gate. Runs only through SDL's dummy driver; no stock process or desktop.

#include "app/AppEntry.h"
#include "app/Camera.h"
#include "app/Pick.h"
#include "app/TubeScene.h"
#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "sim/World.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
#include <vector>

#ifndef WEBOLUTION_SOURCE_DIR
#define WEBOLUTION_SOURCE_DIR "."
#endif

namespace {

#if WEBOLUTION_HAVE_SDL2
bool pushKey(SDL_Scancode scancode, SDL_Keycode keycode) {
  SDL_Event event{};
  event.type = SDL_KEYDOWN;
  event.key.type = SDL_KEYDOWN;
  event.key.state = SDL_PRESSED;
  event.key.keysym.sym = keycode;
  event.key.keysym.scancode = scancode;
  return SDL_PushEvent(&event) == 1;
}

bool pushClick(int x, int y) {
  SDL_Event event{};
  event.type = SDL_MOUSEBUTTONDOWN;
  event.button.type = SDL_MOUSEBUTTONDOWN;
  event.button.button = SDL_BUTTON_LEFT;
  event.button.state = SDL_PRESSED;
  event.button.x = x;
  event.button.y = y;
  return SDL_PushEvent(&event) == 1;
}

bool pushQuit() {
  SDL_Event event{};
  event.type = SDL_QUIT;
  return SDL_PushEvent(&event) == 1;
}
#endif

}  // namespace

int main(int argc, char** argv) {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "LevelVisibility: requires SDL2\n";
  return 1;
#else
  // This must precede every call into runApplication, which initializes SDL.
  const char* driver = std::getenv("SDL_VIDEODRIVER");
  if (!driver || std::string(driver) != "dummy") {
    std::cerr << "LevelVisibility: refusing to run without SDL_VIDEODRIVER=dummy\n";
    return 1;
  }
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir ||
      !std::filesystem::is_regular_file(std::filesystem::path(gameDir) / "lemmings.box")) {
    std::cout << "LevelVisibility: SKIPPED (retail assets unavailable)\n";
    return 0;
  }
  if (argc < 2 || !*argv[1]) {
    std::cerr << "LevelVisibility: expected CMake-provided output directory\n";
    return 1;
  }

  bool ok = true;
  std::string failure;
  int calls = 0;
  std::size_t targetIndex = static_cast<std::size_t>(-1);
  int targetX = 0, targetY = 0;
  const auto fail = [&](const std::string& reason) {
    if (ok) failure = reason;
    ok = false;
  };

  try {
    const std::filesystem::path outputDir(argv[1]);
    std::filesystem::create_directories(outputDir);
    const auto fixture = std::filesystem::path(WEBOLUTION_SOURCE_DIR) /
        "tools/harness/fixtures/camera_a1/camera-fixture-params.json";
    std::ifstream input(fixture, std::ios::binary);
    if (!input) { std::cerr << "LevelVisibility: cannot read camera fixture\n"; return 1; }
    std::string cameraJson((std::istreambuf_iterator<char>(input)), {});
    const std::string yaw = "\"yaw_units\": 32.0";
    const auto yawPosition = cameraJson.find(yaw);
    if (yawPosition == std::string::npos) {
      std::cerr << "LevelVisibility: fixture yaw_units property was not found\n";
      return 1;
    }
    cameraJson.replace(yawPosition, yaw.size(), "\"yaw_units\": 160.0");
    const auto cameraPath = outputDir / "levelvisibility-camera.json";
    std::ofstream cameraOutput(cameraPath, std::ios::binary | std::ios::trunc);
    cameraOutput << cameraJson;
    cameraOutput.close();
    if (!cameraOutput) { std::cerr << "LevelVisibility: cannot write temporary camera\n"; return 1; }

    // Independently prepare the comparison projection. Visibility bypass is
    // diagnostic only; the application run below uses the production default.
    auto library = webo::AssetLibrary::open(std::filesystem::path(gameDir) / "lemmings.box",
                                            std::filesystem::path(gameDir) / "patch.box");
    if (!library) { std::cerr << "LevelVisibility: cannot open retail assets\n"; return 1; }
    std::string levelPath;
    for (const auto& name : webo::levelFileNames(*library))
      if (name.find("MM_CLIMB_002") != std::string::npos) { levelPath = name; break; }
    if (levelPath.empty()) { std::cerr << "LevelVisibility: level blob missing\n"; return 1; }
    auto blob = library->read(levelPath);
    if (!blob) { std::cerr << "LevelVisibility: cannot read level blob\n"; return 1; }
    auto mesh = webo::findCylinderMesh(*blob);
    if (!mesh) { std::cerr << "LevelVisibility: cylinder mesh missing\n"; return 1; }
    auto index = webo::indexLevel(*blob);
    auto scene = webo::app::TubeScene::build(
        *library, *mesh, *blob,
        index ? std::span<const std::uint32_t>(index->grid0) : std::span<const std::uint32_t>{}, true);
    if (!scene) { std::cerr << "LevelVisibility: cannot build tube scene\n"; return 1; }
    webo::app::CameraParams params;
    std::string cameraError;
    if (!webo::app::loadCameraParams(cameraPath, params, cameraError)) {
      std::cerr << "LevelVisibility: invalid temporary camera: " << cameraError << '\n'; return 1;
    }
    const auto observer = [&](const webo::app::LevelFrameView& frame) {
      ++calls;
      if (calls > 2 || frame.frame != calls - 1) fail("unexpected frame sequence");
      if (calls == 1) {
        webo::app::SceneOptions diagnostic;
        diagnostic.drawCellRoute = true;
        diagnostic.injectedWalkers = webo::app::injectedWalkersFromWorld(
            frame.world, frame.world.terrain().height());
        diagnostic.composeSkipActorVisibility = true;
        const auto camera = webo::app::buildCamera(params);
        const auto projectionBoxes = scene->injectedWalkerScreenBoxes(camera, diagnostic, *library);
        auto gated = diagnostic;
        gated.composeSkipActorVisibility = false;
        auto absent = gated;
        absent.injectedWalkers.clear();
        webo::app::Raster visibleRaster(640, 480), gatedRaster(640, 480), absentRaster(640, 480);
        visibleRaster.clear(0, 0, 0); gatedRaster.clear(0, 0, 0); absentRaster.clear(0, 0, 0);
        scene->render(visibleRaster, camera, diagnostic, &*library);
        scene->render(gatedRaster, camera, gated, &*library);
        scene->render(absentRaster, camera, absent, &*library);
        if (gatedRaster.framebuffer().rgb != absentRaster.framebuffer().rgb)
          fail("hidden actor changes rendered pixels");
        if (visibleRaster.framebuffer().rgb == absentRaster.framebuffer().rgb)
          fail("diagnostic bypass does not expose hidden actor pixels");
        if (frame.world.lemmings().empty() || projectionBoxes.empty() || !projectionBoxes[0]) {
          fail("actor 0 has no diagnostic projected box at tick 20");
        } else {
          const auto& p = *projectionBoxes[0];
          targetIndex = 0;
          targetX = static_cast<int>((p.left + p.right) / 2.0);
          targetY = static_cast<int>((p.top + p.bottom) / 2.0);
          if (targetX < 100 || targetX >= 640 || targetY < 0 || targetY >= 480)
            fail("diagnostic actor 0 projection is outside the in-viewport click region");
          bool present = false;
          for (const auto& actor : frame.actors)
            if (actor.index == targetIndex) present = true;
          if (present) fail("production frame includes hidden actor 0");
        }
        // Queue the click even when actor 0 leaked into the frame: the next
        // observation independently catches an erroneous assignment.
        if (!pushKey(SDL_SCANCODE_P, SDLK_p)) fail("could not queue pause key");
        if (targetIndex != static_cast<std::size_t>(-1) &&
            !pushClick(targetX, targetY)) fail("could not queue hidden-actor click");
      } else if (calls == 2) {
        if (frame.hoveredActor || frame.cursorState != webo::app::CursorState::NoLemming ||
            frame.cursorX != targetX || frame.cursorY != targetY)
          fail("hidden actor produced eligible hover feedback");
        if (!frame.paused) fail("click on hidden actor resumed/unpaused the level");
        if (targetIndex >= frame.world.lemmings().size() ||
            frame.world.lemmings()[targetIndex].canClimb)
          fail("click on hidden actor assigned Climber");
        if (frame.world.setup().skills[static_cast<std::size_t>(webo::sim::Skill::Climber)] != 10)
          fail("click on hidden actor changed stock from 10");
        if (!pushQuit()) fail("could not queue SDL_QUIT");
      }
    };

    std::vector<std::string> args{"levelvisibility-test", "--level", "MM_CLIMB_002", "--native",
        "--sim-ticks", "20", "--frames", "8", "--camera", cameraPath.string(),
        "--game-dir", gameDir};
    std::vector<char*> raw;
    for (auto& arg : args) raw.push_back(arg.data());
    if (webo::app::runApplication(static_cast<int>(raw.size()), raw.data(), {}, observer) != 0)
      fail("runApplication returned failure");
  } catch (const std::exception& e) {
    std::cerr << "LevelVisibility: exception: " << e.what() << '\n';
    return 1;
  }

  if (calls != 2) fail("did not observe exactly two frames");
  if (!ok) { std::cerr << "FAIL: LevelVisibility: " << failure << '\n'; return 1; }
  std::cout << "PASS: hidden actor 0 is omitted and cannot be assigned through the application path\n";
  return 0;
#endif
}
