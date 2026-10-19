// Lemmings Webolution -- 3D-first entry point.
//
// This is the A4 replacement executable. It loads one retail level from an
// existing installation, builds the tube from the stored .LVL and archive
// data only, and draws it through the verified A1 camera. The flat unwrapped
// view is a debug flag (--flat), not the default.
//
//   --level <id>     level to load (default MM_CLIMB_002)
//   --native         render at the original's 640x480
//   --camera <json>  camera state params (the A1 fixture/params format)
//   --shot <png>     render one frame headless and write it
//   --geom <what>    wall | wall+surface | wall+polylist | cellmesh | route
//                    `route` is the original's own tube submission
//                    (0x43A4EC): the per-cell mesh through the INN pass and the
//                    three OUT passes, and nothing else. `cellmesh` is the
//                    older diagnostic that adds both banks over the wall.
//   --tiles <mode>   flipped | unflipped  (A2 texture-row addressing)
//   --order <mode>   inner-first | outer-first
//   --route <term>   diagnostic single-term mutation of the route:
//                    swap | noinn | centre | texbank
//   --tile-detail    hi | low  (the cylinder skin's archive variant; hi is
//                    what the retained originals were captured with)
//   --no-depth       disable the software depth test (diagnostic)
//   --flat           debug: unwrap the skin to 2D instead of the 3D view
//
// No live game and no captured arena data are used. Everything comes from the
// install named by WEBOLUTION_GAME_DIR (default C:/progs/Lemmings Revolution).

#include "app/Camera.h"
#include "app/CameraInput.h"
#include "app/Raster.h"
#include "app/TubeScene.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <vector>

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

namespace fs = std::filesystem;
using namespace webo;

namespace {

constexpr const char* kDefaultGameDir = "C:/progs/Lemmings Revolution";

struct Args {
  std::string level = "MM_CLIMB_002";
  bool native = false;
  std::string shot;
  std::string camera;
  std::string geom = "route";
  std::string tiles = "flipped";
  std::string order = "inner-first";
  std::string wall = "full";
  std::string route;
  bool noDepth = false;
  bool flat = false;
  bool highDetailTiles = true;
  std::string gameDir;
  int frames = 0;
  bool help = false;
};

Args parseArgs(int argc, char** argv) {
  Args a;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    const auto next = [&]() -> std::string { return i + 1 < argc ? argv[++i] : std::string{}; };
    if (arg == "--help" || arg == "-h") a.help = true;
    if (arg == "--level") a.level = next();
    else if (arg == "--native") a.native = true;
    else if (arg == "--shot") a.shot = next();
    else if (arg == "--frames") a.frames = std::atoi(next().c_str());
    else if (arg == "--camera") a.camera = next();
    else if (arg == "--geom") a.geom = next();
    else if (arg == "--tiles") a.tiles = next();
    else if (arg == "--order") a.order = next();
    else if (arg == "--wall") a.wall = next();
    else if (arg == "--route") a.route = next();
    else if (arg == "--tile-detail") {
      const std::string v = next();
      if (v == "low") a.highDetailTiles = false;
      else a.highDetailTiles = true;
    }
    else if (arg == "--no-depth") a.noDepth = true;
    else if (arg == "--flat") a.flat = true;
    else if (arg == "--game-dir") a.gameDir = next();
  }
  if (a.gameDir.empty()) {
    const char* env = std::getenv("WEBOLUTION_GAME_DIR");
    a.gameDir = env ? env : kDefaultGameDir;
  }
  return a;
}

void printHelp() {
  std::puts("webolution [--level ID] [--native] [--geom route|wall|...] [--shot PNG] [--frames N]\n"
            "Window: right mouse + NUMPAD4/6 revolves; mouse drag revolves/elevates;\n"
            "SPACE or CTRL zooms; BACKSPACE flips. Default geometry is route.\n"
            "--shot renders one headless frame; --frames N runs N window frames\n"
            "(use SDL_VIDEODRIVER=dummy for a headless smoke run).");
}

Image toImage(const app::Framebuffer& fb) {
  Image image;
  image.width = fb.width;
  image.height = fb.height;
  image.rgba.resize(static_cast<std::size_t>(fb.width) * fb.height * 4);
  for (std::size_t i = 0, n = static_cast<std::size_t>(fb.width) * fb.height; i < n; ++i) {
    image.rgba[i * 4] = fb.rgb[i * 3];
    image.rgba[i * 4 + 1] = fb.rgb[i * 3 + 1];
    image.rgba[i * 4 + 2] = fb.rgb[i * 3 + 2];
    image.rgba[i * 4 + 3] = 255;
  }
  return image;
}

}  // namespace

int main(int argc, char** argv) {
  const Args args = parseArgs(argc, argv);
  if (args.help) { printHelp(); return 0; }

  auto library = AssetLibrary::open(fs::path(args.gameDir) / "lemmings.box",
                                    fs::path(args.gameDir) / "patch.box");
  if (!library) {
    std::fprintf(stderr, "assets: %s\n", library.error().c_str());
    return 1;
  }

  std::string levelPath;
  for (const auto& name : levelFileNames(*library))
    if (name.find(args.level) != std::string::npos) {
      levelPath = name;
      break;
    }
  if (levelPath.empty()) {
    std::fprintf(stderr, "level: no .LVL matching %s\n", args.level.c_str());
    return 1;
  }
  auto blob = library->read(levelPath);
  if (!blob) {
    std::fprintf(stderr, "level: %s\n", blob.error().c_str());
    return 1;
  }
  auto mesh = findCylinderMesh(*blob);
  if (!mesh) {
    std::fprintf(stderr, "cylinder: %s\n", mesh.error().c_str());
    return 1;
  }
  auto index = indexLevel(*blob);

  auto scene = app::TubeScene::build(*library, *mesh, *blob,
                                     index ? std::span<const std::uint32_t>(index->grid0)
                                           : std::span<const std::uint32_t>{},
                                     args.highDetailTiles);
  if (!scene) {
    std::fprintf(stderr, "scene: %s\n", scene.error().c_str());
    return 1;
  }

  app::CameraParams params;
  if (!args.camera.empty()) {
    std::string error;
    if (!app::loadCameraParams(args.camera, params, error)) {
      std::fprintf(stderr, "camera: %s\n", error.c_str());
      return 1;
    }
  } else {
    params.width = args.native ? 640 : 1280;
    params.height = args.native ? 480 : 720;
    params.aspect = static_cast<double>(params.width) / params.height;
  }

  app::SceneOptions options;
  options.tileRowsFlipped = args.tiles != "unflipped";
  options.innerFirst = args.order != "outer-first";
  options.depthTest = !args.noDepth;
  options.fullWall = args.wall != "solid";
  options.flat = args.flat;
  options.drawSurface = args.geom == "wall+surface";
  options.drawPolyList = args.geom == "wall+polylist";
  options.drawCellMesh = args.geom == "cellmesh" || args.geom == "wall+cellmesh";
  options.drawCellRoute = args.geom == "route";
  if (options.drawCellRoute && args.route != "keepwall") {
    // The route is the original's whole tube: the stored constant lattice is
    // not part of that walker, so it is not drawn alongside it. `--route
    // keepwall` is the diagnostic that measures the difference.
    options.drawWall = false;
    options.drawOuter = false;
    options.drawInner = false;
  }
  if (args.route == "swap") {
    options.routeSwapBanks = true;
  } else if (args.route == "noinn") {
    options.routeNoInner = true;
  } else if (args.route == "centre" || args.route == "center") {
    options.routeFlipCentre = true;
  } else if (args.route == "texbank") {
    options.routeTexBankSwap = true;
  } else if (args.route == "keepwall") {
    // handled above: keep the constant lattice alongside the route
  } else if (!args.route.empty()) {
    std::fprintf(stderr, "route: unknown mutation term '%s'\n", args.route.c_str());
    return 1;
  }

  auto renderFrame = [&](const app::CameraParams& frameParams, app::Raster& target) {
    const auto frameCamera = app::buildCamera(frameParams);
    target.clear(0, 0, 0);
    scene->render(target, frameCamera, options);
    return frameCamera;
  };
  app::Raster raster(params.width, params.height);
  const auto camera = renderFrame(params, raster);

  std::printf(
      "level=%s mesh=%ux%u surface=%zu grid0=%zu outerTiles=%zu innerTiles=%zu "
      "surf=%zu poly=%zu cellmesh=%zu+%zu inner=%zu triangles=%zu pixels=%zu camera=%dx%d "
      "yaw=%.4f selector=%.6f\n",
      args.level.c_str(), mesh->angularSteps, mesh->ringCount, mesh->surface.size(),
      index ? index->grid0.size() : 0, scene->outerTiles(), scene->innerTiles(),
      scene->surfaceQuads(), scene->polyListRecords(), scene->cellMesh0Quads(),
      scene->cellMesh4Quads(), scene->cellInnerQuads(), raster.drawnTriangles(),
      raster.drawnPixels(), camera.width, camera.height, app::resolvedYawUnits(params),
      camera.selectorTurns);
  for (const auto& missing : scene->missingTiles())
    std::fprintf(stderr, "tile: %s\n", missing.c_str());

  if (!args.shot.empty()) {
    const Image image = toImage(raster.framebuffer());
    if (auto written = writePng(args.shot, image); !written) {
      std::fprintf(stderr, "screenshot: %s\n", written.error().c_str());
      return 1;
    }
    std::printf("wrote %s (%dx%d)\n", args.shot.c_str(), image.width, image.height);
    return 0;
  }

#if WEBOLUTION_HAVE_SDL2
  SDL_SetMainReady();
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    std::fprintf(stderr, "SDL: %s\n", SDL_GetError()); return 1;
  }
  SDL_Window* window = SDL_CreateWindow(
      "Lemmings Webolution — right mouse + NUMPAD4/6 revolve; drag mouse; SPACE/CTRL zoom; BACKSPACE flip",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
  if (!window) { std::fprintf(stderr, "SDL window: %s\n", SDL_GetError()); SDL_Quit(); return 1; }
  SDL_Renderer* presenter = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (!presenter) { std::fprintf(stderr, "SDL renderer: %s\n", SDL_GetError()); SDL_DestroyWindow(window); SDL_Quit(); return 1; }
  SDL_Texture* texture = SDL_CreateTexture(presenter, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 640, 480);
  if (!texture) { std::fprintf(stderr, "SDL texture: %s\n", SDL_GetError()); SDL_DestroyRenderer(presenter); SDL_DestroyWindow(window); SDL_Quit(); return 1; }

  params.width = 640; params.height = 480; params.aspect = 4.0 / 3.0;
  app::CameraController controls(params);
  app::Raster windowRaster(640, 480);
  bool running = true, rightDown = false, focused = true;
  app::PendingCameraEvents pending;
  int rendered = 0;
  const Uint64 frequency = SDL_GetPerformanceFrequency();
  Uint64 previous = SDL_GetPerformanceCounter();
  app::FixedCameraClock fixedClock;
  while (running && (args.frames <= 0 || rendered < args.frames)) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) running = false;
      else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
        focused = false; rightDown = false; pending.clear();
        SDL_SetRelativeMouseMode(SDL_FALSE);
      } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
        focused = true;
      } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
        if (focused) { rightDown = true; SDL_SetRelativeMouseMode(SDL_TRUE); }
      } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
        rightDown = false; SDL_SetRelativeMouseMode(SDL_FALSE);
      } else if (event.type == SDL_MOUSEMOTION) {
        if (focused) pending.mouse(event.motion.xrel, event.motion.yrel);
      } else if (event.type == SDL_KEYDOWN && !event.key.repeat) {
        const auto key = event.key.keysym.scancode;
        if (focused && (key == SDL_SCANCODE_SPACE || key == SDL_SCANCODE_LCTRL || key == SDL_SCANCODE_RCTRL)) pending.zoom();
        if (focused && key == SDL_SCANCODE_BACKSPACE) pending.flip();
        if (key == SDL_SCANCODE_ESCAPE) running = false;
      }
    }
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    app::CameraInput input;
    input.numpad4 = focused && keys[SDL_SCANCODE_KP_4] != 0;
    input.numpad6 = focused && keys[SDL_SCANCODE_KP_6] != 0;
    input.leftShift = focused && keys[SDL_SCANCODE_LSHIFT] != 0;
    input.rightMouseDown = rightDown;
    const Uint64 now = SDL_GetPerformanceCounter();
    double elapsed = static_cast<double>(now - previous) / static_cast<double>(frequency);
    previous = now;
    if (args.frames > 0) elapsed = app::FixedCameraClock::stepSeconds;
    fixedClock.advance(elapsed);
    bool updated = false;
    while (fixedClock.takeStep()) {
      input = pending.consumeInto(input);
      controls.update(input, app::FixedCameraClock::stepSeconds);
      input.mouseDx = input.mouseDy = 0.0;
      input.zoomPressed = input.backspacePressed = false;
      updated = true;
    }
    if (updated) {
      params = controls.camera();
      renderFrame(params, windowRaster);
      SDL_UpdateTexture(texture, nullptr, windowRaster.framebuffer().rgb.data(), 640 * 3);
      SDL_RenderClear(presenter); SDL_RenderCopy(presenter, texture, nullptr, nullptr); SDL_RenderPresent(presenter);
      ++rendered;
    } else SDL_Delay(1);
  }
  SDL_SetRelativeMouseMode(SDL_FALSE);
  SDL_DestroyTexture(texture); SDL_DestroyRenderer(presenter); SDL_DestroyWindow(window); SDL_Quit();
  (void)camera;
  return 0;
#else
  std::fprintf(stderr, "SDL2 unavailable: window mode is not built (use --shot for headless rendering)\n");
  return 1;
#endif
}
