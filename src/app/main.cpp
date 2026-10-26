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
//   --sim            tick a sim::World (D3a's loadRetailLevel) instead of the
//                     fixed injected-walker fixtures, and draw its live
//                     lemmings through the same drawInjectedWalkers path
//                     (TubeScene::injectedWalkersFromWorld). Off by default;
//                     every pre-existing command line is unaffected.
//   --sim-ticks <N>  headless only (with --shot): call World::tick() exactly
//                     N times before rendering, independent of frame count.
//                     Implies --sim. Prints the tick, lemming count and every
//                     live lemming's cell to stderr.
//   --hud-ticks <N>  headless only (with --shot): after the frame, tick a separate
//                     sim::World N times and overlay the Out / Saved counters and the
//                     timer (shell/HudText.h, C2g). Does not change the scene or the
//                     walkers. Off by default.
//   --hud-out N       with --hud-ticks: replace the sim's Out count (diagnostic only; used to
//                     show the layout against a reference that reads a different number).
//   --hud-colour R,G,B  with --hud-ticks: the Out / timer glyph colour (UNREAD default
//                     255,255,255; exists only for the sensitivity run).
//   --pause-caption   headless only (with --shot): overlay the paused-frame caption panel
//                     (shell/PauseCaption.h, C2i panel, C2j text via the ^Font path).
//   --caption-no-text  with --pause-caption: the C2i panel only (the "before" render).
//   --caption-alpha N  with --pause-caption: panel alpha (HYPOTHESIS 128 of 255; sensitivity only)
//   --caption-fill R,G,B  with --pause-caption: panel colour (HYPOTHESIS 0,0,0; sensitivity only)
//   Debug stand-in, windowed --sim only (not the original skill panel):
//                     keys 1-8 select Climber, Floater, Bomber, Blocker,
//                     Builder, Basher, Miner, Digger in that enum order, and
//                     a left click assigns the selected skill to the lemming
//                     under a 32x32 cursor box centered on the click. Logs
//                     the index, skill and World::assign's return to stderr.
//
// No live game and no captured arena data are used. Everything comes from the
// install named by WEBOLUTION_GAME_DIR (default C:/progs/Lemmings Revolution).

#include "app/Camera.h"
#include "app/BackdropScene.h"
#include "app/SceneFrame.h"
#include "app/CameraInput.h"
#include "app/Pick.h"
#include "app/Raster.h"
#include "app/SimClock.h"
#include "app/SkillPanel.h"
#include "app/TubeScene.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#include "shell/HudText.h"
#include "shell/PauseCaption.h"
#include "sim/World.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <format>
#include <iterator>
#include <optional>
#include <regex>
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
  bool noInjectedWalkers = false;
  bool sceneOff = false;
  std::string walkers;
  bool sim = false;
  int simTicks = 0;
  int hudTicks = -1;
  std::string hudColour;
  int hudOut = -1;
  bool pauseCaption = false;
  bool skillPanel = false;
  int captionAlpha = -1;
  std::string captionFill;
  bool captionNoText = false;
  std::string c3WriteMask, c3WriteEvents;
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
    else if (arg == "--no-injected-walkers") a.noInjectedWalkers = true;
    else if (arg == "--scene-off") a.sceneOff = true;
    else if (arg == "--walkers") a.walkers = next();
    else if (arg == "--sim") a.sim = true;
    else if (arg == "--sim-ticks") { a.sim = true; a.simTicks = std::atoi(next().c_str()); }
    else if (arg == "--hud-ticks") a.hudTicks = std::atoi(next().c_str());
    else if (arg == "--hud-out") a.hudOut = std::atoi(next().c_str());
    else if (arg == "--hud-colour") a.hudColour = next();
    else if (arg == "--pause-caption") a.pauseCaption = true;
    else if (arg == "--skill-panel") a.skillPanel = true;
    else if (arg == "--caption-alpha") a.captionAlpha = std::atoi(next().c_str());
    else if (arg == "--caption-fill") a.captionFill = next();
    else if (arg == "--caption-no-text") a.captionNoText = true;
    else if (arg == "--game-dir") a.gameDir = next();
    else if (arg == "--c3-write-mask") a.c3WriteMask = next();
    else if (arg == "--c3-write-events") a.c3WriteEvents = next();
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

#if WEBOLUTION_HAVE_SDL2
// Toward-zero truncation. Pick.h documents that the original's screen-box
// fields are truncated doubles; this debug path uses that conversion on the
// placement the accessor returns. Not a new exe read.
app::PickBox pickBoxFromPlacement(const app::ActorSpritePlacement& placement) {
  return app::PickBox{
      static_cast<int>(placement.left),
      static_cast<int>(placement.top),
      static_cast<int>(placement.right),
      static_cast<int>(placement.bottom),
  };
}

// Debug click-to-assign. Not the original skill panel. The cursor is a 32x32
// box centered on the window click (sprite size named in Pick.h at
// 0x0045578C); pickBoxOverlaps applies the +/-4 adjustment itself. The window
// and the raster are both 640x480, so the click is in the placement's pixel
// space. priority and distanceSquared stay 0, the inherited D2a limitation:
// first eligible overlapping candidate in list order wins only when every
// candidate is tied on both keys.
void debugClickToAssign(sim::World& world, const app::TubeScene& scene,
                        const app::SceneOptions& options, const AssetLibrary& library,
                        const app::CameraBasis& camera, int x, int y, sim::Skill skill) {
  const auto boxes = scene.injectedWalkerScreenBoxes(camera, options, library);
  std::vector<app::PickCandidate> candidates;
  std::size_t walker = 0;
  const auto& lemmings = world.lemmings();
  for (std::size_t i = 0; i < lemmings.size(); ++i) {
    if (!lemmings[i].alive())
      continue;
    if (walker < boxes.size() && boxes[walker]) {
      app::PickCandidate candidate;
      candidate.id = reinterpret_cast<void*>(static_cast<std::uintptr_t>(i));
      candidate.box = pickBoxFromPlacement(*boxes[walker]);
      candidate.eligible = true;
      candidate.priority = 0;
      candidate.distanceSquared = 0.0;
      candidates.push_back(candidate);
    }
    ++walker;
  }
  const app::PickBox cursor{x - 16, y - 16, x + 16, y + 16};
  const app::PickCandidate* hit = app::pickHoveredLemming(candidates, cursor);
  const std::string skillText(sim::skillName(skill));
  if (hit == nullptr) {
    std::fprintf(stderr, "click-to-assign: no lemming at (%d,%d) skill=%s\n", x, y,
                 skillText.c_str());
    return;
  }
  const auto index = static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(hit->id));
  const bool assigned = world.assign(index, skill);
  std::fprintf(stderr, "click-to-assign: index=%zu skill=%s assign=%d\n", index, skillText.c_str(),
               assigned ? 1 : 0);
}
#endif

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
  std::optional<app::BackdropScene> backdrop;
  if (!args.sceneOff) {
    auto built = app::BackdropScene::build(*library, *blob, args.level);
    if (built) backdrop.emplace(std::move(*built));
    else std::fprintf(stderr,"backdrop: unsupported/invalid scene: %s\n",built.error().c_str());
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
  if (options.drawCellRoute && args.level.find("MM_CLIMB_002") != std::string::npos &&
      !args.noInjectedWalkers) {
    options.injectedWalkers = app::defaultMmClimb002InjectedWalkers();
  }
  if (!args.walkers.empty()) {
    std::ifstream input(args.walkers, std::ios::binary);
    if (!input) {
      std::fprintf(stderr, "walkers: cannot open %s\n", args.walkers.c_str());
      return 1;
    }
    const std::string json((std::istreambuf_iterator<char>(input)), {});
    const std::string number = R"([-+]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?)";
    const std::regex document(R"(^\s*\{\s*"walkers"\s*:\s*\[\s*(?:\{\s*"plus8"\s*:\s*\[\s*)" +
                              number + R"(\s*,\s*)" + number + R"(\s*,\s*)" + number +
                              R"(\s*\]\s*,\s*"facing"\s*:\s*-?[0-9]+\s*\}\s*(?:,\s*\{\s*"plus8"\s*:\s*\[\s*)" +
                              number + R"(\s*,\s*)" + number + R"(\s*,\s*)" + number +
                              R"(\s*\]\s*,\s*"facing"\s*:\s*-?[0-9]+\s*\}\s*)*\s*)?\]\s*\}\s*$)");
    if (!std::regex_match(json, document)) {
      std::fprintf(stderr, "walkers: expected {\"walkers\":[{\"plus8\":[x,y,z],\"facing\":n},...]}\n");
      return 1;
    }
    const std::regex walker("\\{\\s*\"plus8\"\\s*:\\s*\\[\\s*(" + number +
                            ")\\s*,\\s*(" + number + ")\\s*,\\s*(" + number +
                            ")\\s*\\]\\s*,\\s*\"facing\"\\s*:\\s*(-?[0-9]+)\\s*\\}");
    std::vector<app::SceneOptions::InjectedNativeWalker> loaded;
    for (std::sregex_iterator it(json.begin(), json.end(), walker), end; it != end; ++it) {
      app::SceneOptions::InjectedNativeWalker row;
      try {
        row.plus8X = std::stod((*it)[1].str()); row.plus8Y = std::stod((*it)[2].str());
        row.plus8Z = std::stod((*it)[3].str()); row.facing = std::stoi((*it)[4].str());
      } catch (...) {
        std::fprintf(stderr, "walkers: invalid numeric value\n"); return 1;
      }
      if (!std::isfinite(row.plus8X) || !std::isfinite(row.plus8Y) || !std::isfinite(row.plus8Z)) {
        std::fprintf(stderr, "walkers: coordinates must be finite\n"); return 1;
      }
      loaded.push_back(row);
    }
    options.injectedWalkers = std::move(loaded);
  }

  // D3b: a ticking sim::World, built by the D3a production loader, replaces
  // the fixed injected-walker fixtures above -- but only when --sim/--sim-ticks
  // is given. Every pre-existing command line (no --sim) leaves
  // options.injectedWalkers exactly as the blocks above already set it.
  std::optional<sim::World> world;
  int terrainHeight = 0;
  if (args.sim) {
    auto setup = sim::loadRetailLevel(*library, args.level);
    if (!setup) {
      std::fprintf(stderr, "sim: %s\n", setup.error().c_str());
      return 1;
    }
    world.emplace(*setup);
    for (int i = 0; i < args.simTicks; ++i)
      world->tick();
    terrainHeight = world->terrain().height();
    options.injectedWalkers = app::injectedWalkersFromWorld(*world, terrainHeight);

    const auto& lemmings = world->lemmings();
    std::size_t liveCount = 0;
    for (const auto& l : lemmings)
      if (l.alive()) ++liveCount;
    std::fprintf(stderr, "sim: tick=%d lemmings=%zu live=%zu spawned=%d saved=%d dead=%d\n",
                args.simTicks, lemmings.size(), liveCount, world->spawned(), world->saved(),
                world->dead());
    for (std::size_t i = 0; i < lemmings.size(); ++i) {
      const sim::Lemming& l = lemmings[i];
      std::fprintf(stderr, "sim: lemming %zu cell=(%d,%d) facing=%d state=%s alive=%d\n", i,
                  l.x, l.y, static_cast<int>(l.facing),
                  std::string(sim::stateName(l.state)).c_str(), l.alive() ? 1 : 0);
    }
  }

  auto renderFrame = [&](const app::CameraParams& frameParams, app::Raster& target) {
    const auto frameCamera = app::buildCamera(frameParams);
    target.clear(0, 0, 0);
    app::BackdropStats backdropStats;
    app::SceneRasterObserver observer;
    std::vector<std::uint8_t> c3Mask;
    std::ofstream c3Out;
    const bool c3Diagnostic=!args.c3WriteMask.empty()||!args.c3WriteEvents.empty();
    if(c3Diagnostic){
      if(args.c3WriteMask.empty()||args.c3WriteEvents.empty()||frameCamera.width!=640||frameCamera.height!=480){std::fprintf(stderr,"C3 diagnostic requires both paths and a 640x480 frame\n");std::exit(2);}
      std::ifstream mf(args.c3WriteMask,std::ios::binary);c3Mask.assign(std::istreambuf_iterator<char>(mf),{});
      if(!mf||c3Mask.size()!=640u*480u){std::fprintf(stderr,"C3 mask must be raw 640x480 bytes\n");std::exit(2);}
      c3Out.open(args.c3WriteEvents,std::ios::binary);if(!c3Out){std::fprintf(stderr,"cannot open C3 event output\n");std::exit(2);}
      c3Out<<"kind,source,node,block,triangleIndex,pixel,beforeR,beforeG,beforeB,beforeA,afterR,afterG,afterB,afterA,depthBefore,depthAfter,depthAssigned\n";
      observer.mask=&c3Mask;
      observer.candidate=[&](auto s,auto n,auto b,auto nt){c3Out<<"candidate,"<<s<<','<<n<<','<<b<<','<<nt<<",,,,,,\n";};
      observer.accepted=[&](const app::SceneRasterWriteEvent& e){c3Out<<std::setprecision(17)<<"write,"<<e.source<<",,,"<<e.triangle<<','<<e.pixel;
        for(auto v:e.colorBefore)c3Out<<','<<unsigned(v);
        for(auto v:e.colorAfter)c3Out<<','<<unsigned(v);
        c3Out<<','<<e.depthBefore<<','<<e.depthAfter<<','<<e.depthAssigned<<'\n';};
    }
     app::scheduleSceneFrame(
          [&] { if (backdrop) backdrop->renderStage(target, frameCamera, *library, backdropStats, app::BackdropScene::Stage::Primary,c3Diagnostic?&observer:nullptr); },
         [&] { if (backdrop) backdrop->renderStage(target, frameCamera, *library, backdropStats, app::BackdropScene::Stage::LegacyUnattributed); },
         [&] { scene->render(target, frameCamera, options, &*library); },
         [&] { if (backdrop) backdrop->renderStage(target, frameCamera, *library, backdropStats, app::BackdropScene::Stage::Secondary); });
    if (!args.sceneOff) std::fprintf(stderr,"backdrop: enabled=%d roots=%zu records=%zu triangles=%zu billboards=%zu null-materials=%zu null-triangles=%zu predicate-rejected=%zu missing-named-textures=%zu rejected=%zu pixels-depth-written=%zu\n",backdrop.has_value(),backdropStats.roots,backdropStats.meshRecords,backdropStats.meshTriangles,backdropStats.billboards,backdropStats.nullMaterials,backdropStats.nullMaterialTriangles,backdropStats.predicateRejected,backdropStats.missingTextures,backdropStats.rejected,backdropStats.pixels);
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
    Image image = toImage(raster.framebuffer());
    if (args.skillPanel) {
      // C1t opt-in headless render; all panel drawing lives in SkillPanel.cpp.
      if (auto panel = app::drawRetailSkillPanel(image, *library, args.level); !panel) {
        std::fprintf(stderr, "skill-panel: %s\n", panel.error().c_str());
        return 1;
      }
    }
    if (args.hudTicks >= 0) {
      // C2g: HUD overlay from a separate World, so the scene above is untouched.
      auto setup = sim::loadRetailLevel(*library, args.level);
      auto sheet = loadTexture(*library, "TEXTURES\\^FONT.BMP");
      if (!setup || !sheet) {
        std::fprintf(stderr, "hud: %s\n", (!setup ? setup.error() : sheet.error()).c_str());
        return 1;
      }
      sim::World hudWorld(*setup);
      for (int i = 0; i < args.hudTicks; ++i)
        hudWorld.tick();
      shell::HudValues values;
      values.out = hudWorld.spawned() - hudWorld.saved() - hudWorld.dead();
      if (args.hudOut >= 0)
        values.out = args.hudOut;
      values.saved = hudWorld.saved();
      values.saveTarget = setup->saveTarget;
      values.secondsRemaining =
          static_cast<int>(hudWorld.timeRemainingTicks() / hudWorld.rules().ticksPerSecond);
      shell::HudParams hudParams;
      if (!args.hudColour.empty()) {
        int r = 0, g = 0, b = 0;
        if (std::sscanf(args.hudColour.c_str(), "%d,%d,%d", &r, &g, &b) != 3) {
          std::fprintf(stderr, "hud: --hud-colour wants R,G,B\n");
          return 1;
        }
        hudParams.white = {static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g),
                           static_cast<std::uint8_t>(b)};
      }
      const auto lines = shell::layoutHud(values, hudParams, shell::fontCells());
      shell::drawHud(image, *sheet, lines, shell::fontCells());
      std::fprintf(stderr, "hud: tick=%d out=%d saved=%d target=%d seconds=%d\n", args.hudTicks,
                   values.out, values.saved, values.saveTarget, values.secondsRemaining);
      static const char* const names[] = {"out", "saved", "timer"};
      for (std::size_t i = 0; i < lines.size(); ++i) {
        const auto box = shell::lineBox(lines[i], shell::fontCells());
        std::fprintf(stderr, "hud-box: %s %d %d %d %d \"%s\"\n", names[i], box.x0, box.y0, box.x1,
                     box.y1, lines[i].text.c_str());
      }
    }
    if (args.pauseCaption) {
      // C2i: paused frames only; the panel is drawn over the finished frame and HUD.
      shell::CaptionParams caption;
      if (args.captionAlpha >= 0)
        caption.alpha = args.captionAlpha;
      if (!args.captionFill.empty()) {
        int r = 0, g = 0, b = 0;
        if (std::sscanf(args.captionFill.c_str(), "%d,%d,%d", &r, &g, &b) != 3) {
          std::fprintf(stderr, "caption: --caption-fill wants R,G,B\n");
          return 1;
        }
        caption.fill = {static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g),
                        static_cast<std::uint8_t>(b)};
      }
      if (args.captionNoText) {
        shell::drawCaptionPanel(image, caption);
      } else {
        auto font = loadTexture(*library, "TEXTURES\\^FONT.BMP");
        if (!font) {
          std::fprintf(stderr, "caption: %s\n", font.error().c_str());
          return 1;
        }
        shell::drawCaption(image, *font, shell::fontCells(), caption);
      }
      std::fprintf(stderr, "caption: rect %d %d %d %d alpha %d/%d fill %d,%d,%d\n", caption.rect.x0,
                   caption.rect.y0, caption.rect.x1, caption.rect.y1, caption.alpha,
                   caption.alphaScale, caption.fill[0], caption.fill[1], caption.fill[2]);
    }
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
  // D3b: the sim ticks at its own measured 200 ms period (SimClock.h),
  // independent of this window's 144 Hz camera clock and its render rate --
  // only active when --sim was given; windowed mode always starts the
  // World at tick 0 regardless of --sim-ticks, which is a headless-only,
  // deterministic-render concern (see this file's own --sim-ticks comment).
  app::SimClock simClock;
  // Debug stand-in for a skill panel. Keys 1-8 follow sim::Skill's declaration
  // order. This is not the original panel and is ignored unless --sim is on.
  sim::Skill debugAssignSkill = sim::Skill::Climber;
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
      } else if (args.sim && world && event.type == SDL_MOUSEBUTTONDOWN &&
                 event.button.button == SDL_BUTTON_LEFT) {
        // Debug only. A command line without --sim never enters this branch.
        if (focused) {
          debugClickToAssign(*world, *scene, options, *library,
                             app::buildCamera(controls.camera()), event.button.x, event.button.y,
                             debugAssignSkill);
        }
      } else if (event.type == SDL_MOUSEMOTION) {
        if (focused) pending.mouse(event.motion.xrel, event.motion.yrel);
      } else if (event.type == SDL_KEYDOWN && !event.key.repeat) {
        const auto key = event.key.keysym.scancode;
        if (focused && (key == SDL_SCANCODE_SPACE || key == SDL_SCANCODE_LCTRL || key == SDL_SCANCODE_RCTRL)) pending.zoom();
        if (focused && key == SDL_SCANCODE_BACKSPACE) pending.flip();
        if (key == SDL_SCANCODE_ESCAPE) running = false;
        if (args.sim && world && focused) {
          sim::Skill selected = debugAssignSkill;
          bool choose = false;
          switch (key) {
            case SDL_SCANCODE_1: selected = sim::Skill::Climber; choose = true; break;
            case SDL_SCANCODE_2: selected = sim::Skill::Floater; choose = true; break;
            case SDL_SCANCODE_3: selected = sim::Skill::Bomber; choose = true; break;
            case SDL_SCANCODE_4: selected = sim::Skill::Blocker; choose = true; break;
            case SDL_SCANCODE_5: selected = sim::Skill::Builder; choose = true; break;
            case SDL_SCANCODE_6: selected = sim::Skill::Basher; choose = true; break;
            case SDL_SCANCODE_7: selected = sim::Skill::Miner; choose = true; break;
            case SDL_SCANCODE_8: selected = sim::Skill::Digger; choose = true; break;
            default: break;
          }
          if (choose) {
            debugAssignSkill = selected;
            std::fprintf(stderr,
                         "click-to-assign: debug skill %s (stand-in, not the original skill panel)\n",
                         std::string(sim::skillName(debugAssignSkill)).c_str());
          }
        }
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
    bool simTicked = false;
    if (args.sim && world) {
      simClock.advance(elapsed);
      while (simClock.takeStep()) {
        world->tick();
        simTicked = true;
      }
      if (simTicked)
        options.injectedWalkers = app::injectedWalkersFromWorld(*world, terrainHeight);
    }
    if (updated || simTicked) {
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
