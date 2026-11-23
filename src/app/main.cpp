#include "app/FrameProfile.h"
#include "app/RasterRows.h"
#include "app/FrameConsumption.h"
#include "app/Replay.hpp"
// Ordinary startup opens the connected frontend. Game data lookup also serves
// diagnostics and replays, with --game-dir and WEBOLUTION_GAME_DIR taking precedence.
//
// --direct-level  Load MM_CLIMB_002 with injected walkers.
// --sim           Display live World actors in the direct-level view.
// --sim-ticks N   Seed that World with N ticks before drawing.
// --camera JSON   Supply camera pose and projection parameters.
// --window-scale  Override the saved presentation scale (1, 2 or 3).
// --native        Render at 640x480.
// --shot PNG      Save a single headless frame.
// --window-shot PNG  Save the final CPU frame when --frames N is reached.
//                  SDL_VIDEODRIVER=dummy supports offscreen presentation.
// --geom route    Draw 0x43A4EC's INN and three OUT passes. Other geometry
//                  options expose the wall, surface, PolyList and cellmesh banks.
// --route TERM    Isolate a tube term: swap, noinn, centre, texbank or keepwall.
// --tile-detail   Select hi or low cylinder skins from the archive.
// --flat          Unwrap the skin into a 2D view.
// --hud-ticks N   Add an overlay from a separate World to a headless shot.
//                  --hud-out and --hud-colour override its values and tint.
// --pause-caption  Overlay the pause panel. --caption-no-text omits
//                  ^Font glyphs. Alpha and fill defaults are provisional.
// --cursor-fixture  Centre the NoLemming software cursor on a headless shot.

#include "app/Camera.h"
#include "app/CameraIntro.h"
#include "app/EntryStartCue.h"
#include "app/LevelStartSequence.h"
#include "app/LemmingAnim.h"
#include "app/LemmingMotion.h"
#include "app/LevelMenu.h"
#include "assets/GameData.h"
#include "app/AppEntry.h"
#include "app/GameDataLocation.h"
#include "app/MapScene.h"
#include "app/FrontendSession.h"
#include "app/FrontendWindow.h"
#include "app/BackdropScene.h"
#include "app/SceneFrame.h"
#include "app/CameraInput.h"
#include "app/Pick.h"
#include "app/RuntimePick.h"
#include "app/RuntimeCursor.h"
#include "app/LevelOutcome.h"
#include "app/Raster.h"
#include "app/SimClock.h"
#include "app/RuntimeHud.h"
#include "app/RuntimeSkillPanel.h"
#include "app/SkillPanel.h"
#include "app/TubeScene.h"
#include "assets/Bitmap.h"
#include "assets/TextureCache.h"
#include "assets/Box.h"
#include "assets/DiscGameData.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "assets/Png.h"
#include "audio/CuePlayback.h"
#include "sim/RetailLevel.h"
#include "shell/HudText.h"
#include "shell/PauseCaption.h"
#include "sim/World.h"

#include <cstdint>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <limits>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "app/SdlWindow.hpp"
#ifdef WEBOLUTION_WINDOWS_ICON
#include "app/WindowsWindowIcon.h"
#endif
#endif
#ifdef __EMSCRIPTEN__
#include "web/Browser.h"
#endif

namespace fs = std::filesystem;
using namespace webo;

namespace {

// Process-wide CRT generator for branch-1 scenery start frames.
// Native 0x0045D06C and 0x004451E5 seed from the unpaused timer T0 via
// srand(ftol(ms * 32767.0)) (0x00447580). The clone uses its monotonic
// application milliseconds excluding simulation pause.
struct SceneryRandom {
  app::CrtRand rng;
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  double pausedMs = 0.0;
  double unpausedMs() const {
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() - pausedMs;
  }
  void seed() { rng.seedFromMs(unpausedMs()); }
  void begin() { start = std::chrono::steady_clock::now(); pausedMs = 0.0; seed(); }
};
SceneryRandom& sceneryRandom() { static SceneryRandom instance; return instance; }

struct Args {
  std::string level = "MM_CLIMB_002";
  bool native = false;
  std::string shot;
  std::string windowShot;
  std::string camera;
  std::string recordDir;
  std::string replayFile;
  bool invalidReplayOption = false;
  std::string geom = "route";
  std::string tiles = "flipped";
  std::string order = "inner-first";
  std::string wall = "full";
  std::string route;
  bool noDepth = false;
  bool noAudio = false;
  bool flat = false;
  bool highDetailTiles = true;
  fs::path gameDir;
  int frames = 0;
  bool framesSpecified = false;
  int windowScale = 0;  // omitted: use the saved choice in ordinary play
  bool invalidWindowScale = false;
  bool help = false;
  bool version = false;
  bool map = false;
  bool frontend = false;
  bool mapFlagsSpecified = false;
  std::string mapFlags;
  bool noInjectedWalkers = false;
  bool noTerrainSync = false;  // Diagnostic: draw terrain as loaded.
  bool noActorAnim = false;    // Diagnostic: draw every lemming at walk entry 0.
  bool sceneOff = false;
  std::string walkers;
  bool sim = false;
  bool simTicksSpecified = false;
  int simTicks = 0;
  int hudTicks = -1;
  std::string hudColour;
  int hudOut = -1;
  bool pauseCaption = false;
  bool skillPanel = false;
  bool cursorFixture = false;
  int captionAlpha = -1;
  std::string captionFill;
  bool captionNoText = false;
  std::string c3WriteMask, c3WriteEvents;
};

Args parseArgs(int argc, char** argv) {
  Args a;
  // Bare startup, --game-dir, --no-audio, --window-scale and --record default
  // to the frontend. Other tokens select diagnostics unless --frontend is explicit.
  bool defaultFrontend = true;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg != "--game-dir" && arg != "--no-audio" && arg != "--window-scale" && arg != "--record") defaultFrontend = false;
    const auto next = [&]() -> std::string { return i + 1 < argc ? argv[++i] : std::string{}; };
    if (arg == "--help" || arg == "-h") a.help = true;
    else if (arg == "--version") a.version = true;
    else if (arg == "--map") a.map = true;
    else if (arg == "--frontend") a.frontend = true;
    else if (arg == "--direct-level") defaultFrontend = false;
    else if (arg == "--map-flags") { a.map = true; a.mapFlagsSpecified = true; a.mapFlags = next(); }
    else if (arg == "--level") a.level = next();
    else if (arg == "--record") { a.recordDir = next(); a.invalidReplayOption |= a.recordDir.empty(); }
    else if (arg == "--replay") { a.replayFile = next(); a.invalidReplayOption |= a.replayFile.empty(); }
    else if (arg == "--native") a.native = true;
    else if (arg == "--shot") a.shot = next();
    else if (arg == "--window-shot") a.windowShot = next();
    else if (arg == "--frames") { a.framesSpecified = true; a.frames = std::atoi(next().c_str()); }
    else if (arg == "--window-scale") {
      const auto value = next();
      if (value == "1" || value == "2" || value == "3") a.windowScale = value[0] - '0';
      else a.invalidWindowScale = true;
    }
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
    else if (arg == "--no-audio") a.noAudio = true;
    else if (arg == "--flat") a.flat = true;
    else if (arg == "--no-injected-walkers") a.noInjectedWalkers = true;
    else if (arg == "--no-terrain-sync") a.noTerrainSync = true;
    else if (arg == "--no-actor-anim") a.noActorAnim = true;
    else if (arg == "--scene-off") a.sceneOff = true;
    else if (arg == "--walkers") a.walkers = next();
    else if (arg == "--sim") a.sim = true;
    else if (arg == "--sim-ticks") {
      a.sim = a.simTicksSpecified = true;
      a.simTicks = std::atoi(next().c_str());
    }
    else if (arg == "--hud-ticks") a.hudTicks = std::atoi(next().c_str());
    else if (arg == "--hud-out") a.hudOut = std::atoi(next().c_str());
    else if (arg == "--hud-colour") a.hudColour = next();
    else if (arg == "--pause-caption") a.pauseCaption = true;
    else if (arg == "--skill-panel") a.skillPanel = true;
    else if (arg == "--cursor-fixture") a.cursorFixture = true;
    else if (arg == "--caption-alpha") a.captionAlpha = std::atoi(next().c_str());
    else if (arg == "--caption-fill") a.captionFill = next();
    else if (arg == "--caption-no-text") a.captionNoText = true;
    else if (arg == "--game-dir") {
      a.gameDir = next();
      if (a.gameDir.empty()) defaultFrontend = false;
    }
    else if (arg == "--c3-write-mask") a.c3WriteMask = next();
    else if (arg == "--c3-write-events") a.c3WriteEvents = next();
  }
  a.frontend = a.frontend || defaultFrontend;
  return a;
}

void printHelp() {
  std::puts("Lemmings Webolution " WEBOLUTION_VERSION_STRING "\n"
            "webolution [--game-dir PATH] [--frontend | --direct-level | --level ID | --map] [options]\n"
            "--version prints the build version.\n"
            "No options, or only --game-dir/--no-audio/--window-scale, starts the connected frontend.\n"
            "Any other option keeps its diagnostic route unless --frontend is explicit.\n"
            "--direct-level opens the previous MM_CLIMB_002 injected-walker diagnostic.\n"
            "--window-scale 1|2|3 overrides the saved Window size for this run.\n"
            "Without a saved Window size, ordinary play uses Auto on desktop or Fill on web.\n"
            "--record DIR records each level attempt as DIR/*.replay (default via Play-Clone.ps1).\n"
            "--replay FILE checks a recording headless at full speed, without opening a save.\n"
            "Diagnostics, bounded runs and observers default to 1x. CPU screenshots keep their resolution.\n"
            "Window: right mouse + NUMPAD4/6 revolves; mouse drag revolves/elevates;\n"
            "SPACE or CTRL zooms; BACKSPACE flips. Default geometry is route.\n"
            "--shot renders one headless frame; --frames N runs N window frames\n"
            "(use SDL_VIDEODRIVER=dummy for a headless smoke run).\n"
            "--window-shot PNG with --frames N saves the final presented window frame.\n"
            "--sim draws live skill counts; click the first eight plates or use keys 1-8\n"
            "to select a skill. Left/right-click a hatch to slow/speed releases; hold to repeat.\n"
            "Double-click Nuke to end releases and explode the remaining lemmings.\n"
            "--frontend starts New Game -> Continue? -> map -> a simulated retail level.\n"
            "Use RETURN at the first two screens, then click an unlocked map node.\n"
            "New Game, Play Current Game and Quit are connected; other menu actions\n"
            "are incomplete; no movies. Progress is stored in the clone's own original-layout\n"
            "SavedGame.dat (1504 bytes) at %LOCALAPPDATA%\\LemmingsWebolution\\SavedGame.dat\n"
            "(Windows); it is saved after each won level and by New Game, and loaded at start.\n"
            "Use Play Current Game to continue (New Game resets it). The installed game's\n"
            "SavedGame.dat is never read, imported or written.\n"
            "--map [--map-flags FILE] [--shot PNG] renders the conditional 640x480 map base layer;\n"
            "FILE is exactly 102 signed decimal int32 flags, whitespace-separated. Omitted flags are\n"
            "all-zero SOURCE fixture data. No selection, title, cursor, save adapter or map controls.\n"
            "Sound: the simulated level plays eight cues with retained static play-site evidence:\n"
            "Ohno (bomber fuse end) and FallSplat (long-fall splat; the sim's Splat is mapped to\n"
            "FallSplat), Yippee (saved at the exit), Letsgo (first Let's Go text frame), Mine (work strokes),\n"
            "Explode (blast), Chink (last three builder bricks), and Action (successful assignment). Changeop plays\n"
            "on skill/Nuke selection and provisionally on pause, speed and hatch-rate controls.\n"
            "Fluid entry, lever/pad activation and stored hazard attack samples also play.\n"
            "Level music starts after the entry cue, then continues through pause.\n"
            "Level effects pan by screen position and fade on the cylinder's back side; UI and Letsgo stay centred.\n"
            "Gain curves, supplemental triggers and repeated-SFX overlap are provisional.\n"
            "No claim of native audio fidelity. A missing audio\n"
            "device or WAV is reported on stderr and the game continues. --no-audio disables sound.");
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
// Truncate projected rectangles toward zero, as the native box accessors do.
app::PickBox pickBoxFromPlacement(const app::ActorSpritePlacement& placement) {
  return app::PickBox{
      static_cast<int>(placement.left),
      static_cast<int>(placement.top),
      static_cast<int>(placement.right),
      static_cast<int>(placement.bottom),
  };
}

// Hover and click share a 32x32 cursor box (0x0045578C) in render pixels;
// pickBoxOverlaps adjusts it by 4 px per edge. Provisional clone ranking
// uses nearest displayed body centre after eligibility checks. Ties keep World order.
std::optional<std::size_t> hoveredActor(const sim::World& world, const app::TubeScene& scene,
                        const app::SceneOptions& options, const AssetLibrary& library,
                        const app::CameraBasis& camera, const app::Raster& sceneRaster,
                        int x, int y, sim::Skill skill, bool firstLevelSupport) {
  // SOURCE stock gate shared by both consumers; -1 remains unlimited.
  if (world.setup().skills[static_cast<std::size_t>(skill)] == 0) return std::nullopt;
  const auto boxes = scene.injectedWalkerScreenBoxes(camera, options, library, sceneRaster);
  std::vector<app::ScreenPickCandidate> candidates;
  std::size_t walker = 0;
  const auto& lemmings = world.lemmings();
  for (std::size_t i = 0; i < lemmings.size(); ++i) {
    if (!lemmings[i].alive())
      continue;
    if (walker < boxes.size() && boxes[walker] && walker < options.injectedWalkers.size()) {
      const auto& placement = *boxes[walker];
      const auto support = firstLevelSupport ?
          app::firstLevelClimberSupport(lemmings[i], skill, world.terrain()) : std::nullopt;
      const bool eligible = lemmings[i].state == sim::State::AntiGrav
          ? world.canAssign(i, skill)
          : app::passesKnownPickExclusions(lemmings[i], skill) && support.value_or(true);
      candidates.push_back({
          reinterpret_cast<void*>(static_cast<std::uintptr_t>(i)),
          pickBoxFromPlacement(placement),
          eligible,
          {(placement.left + placement.right) * 0.5,
           (placement.top + placement.bottom) * 0.5}});
    }
    ++walker;
  }
  const auto* hit = app::pickNearestScreenLemming(candidates, x, y);
  if (!hit) return std::nullopt;
  return static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(hit->id));
}
#endif

}  // namespace

enum class LegDisposition { Exit, Completed, RetryLevel, ReturnToMap };

static int runApplicationLeg(Args args, app::FrontendWindowObserver observer,
                            app::LevelFrameObserver levelObserver, app::ApplicationRunOptions runOptions,
                            app::FrontendSession* persistentFrontend,
                            const AssetLibrary* library,
                            [[maybe_unused]] app::SdlWindow* sharedWindow,
                            [[maybe_unused]] audio::CuePlayback* cues,
                            std::optional<app::LevelOutcome>& completed, LegDisposition& disposition) {
#ifdef __EMSCRIPTEN__
  const bool logLevelDetails = true;
#else
  const char* diagnosticSetting = std::getenv("WEBOLUTION_NATIVE_DIAGNOSTICS");
  const bool verbose = diagnosticSetting && std::string_view(diagnosticSetting) == "1";
  // Keep observer-driven diagnostics in the level output.
  const bool logLevelDetails = verbose || observer || levelObserver;
#endif
  if (!std::isfinite(runOptions.frameSeconds) || runOptions.frameSeconds < 0 ||
      (runOptions.frameSeconds > 0 && args.frames <= 0)) {
    std::fprintf(stderr, "test frame clock requires finite positive seconds and --frames\n");
    return 1;
  }
  if (!args.windowShot.empty() && (args.frames <= 0 || !args.shot.empty() || args.map)) {
    std::fprintf(stderr, "--window-shot requires --frames N > 0 in level window mode, without --shot\n");
    return 1;
  }
  if (args.frontend && args.map) {
    std::fprintf(stderr, "--frontend starts a new game; --map/--map-flags are separate fixture modes\n");
    return 1;
  }

  std::optional<AssetLibrary> diagnosticLibrary;
  if (!library) {
    auto opened = AssetLibrary::open(args.gameDir / "lemmings.box", args.gameDir / "patch.box");
    if (!opened) {
      std::fprintf(stderr, "assets: %s\n", opened.error().c_str());
      return 1;
    }
    diagnosticLibrary.emplace(std::move(*opened));
    library = &*diagnosticLibrary;
  }

  TextureCacheScope textureCache(*library);

  // Frontend selection feeds the retail-level loader below.
  if (args.frontend) {
    std::optional<app::FrontendSession> localFrontend;
    auto* frontend = persistentFrontend;
    if (!frontend) {
      auto created = app::FrontendSession::create(*library);
      if (!created) { std::fprintf(stderr, "frontend: %s\n", created.error().c_str()); return 1; }
      localFrontend = std::move(*created);
      frontend = &*localFrontend;
    }
    if (!args.shot.empty()) {
      if (auto written = writePng(args.shot, frontend->composeFrame()); !written) {
        std::fprintf(stderr, "frontend screenshot: %s\n", written.error().c_str()); return 1;
      }
      return 0;
    }
    // Retry retains selectedLevel and skips the frontend window and its music.
#if WEBOLUTION_HAVE_SDL2
    if (cues && !frontend->selectedLevel()) cues->playFrontendMusic();
#endif
    auto selected = app::runFrontendWindow(*frontend, {args.frames, args.windowShot}, std::move(observer), sharedWindow);
    if (!selected) { std::fprintf(stderr, "frontend: %s\n", selected.error().c_str()); return 1; }
    if (!*selected) return 0; // closed or reached the requested frontend frame cap
    args.level = **selected;
    args.sim = true;
    args.simTicks = 0;
    if (logLevelDetails)
      std::fprintf(stderr, "frontend: entering %s from map (SOURCE SDL adapter)\n", args.level.c_str());
  }

  // Map diagnostics bypass level, simulation and 3D setup.
  if (args.map) {
    std::array<std::int32_t, 102> flags{};
    if (args.mapFlagsSpecified && args.mapFlags.empty()) {
      std::fprintf(stderr, "map flags: --map-flags requires a file path\n"); return 1;
    }
    if (!args.mapFlags.empty()) {
      auto loaded = app::readMapFlags(args.mapFlags);
      if (!loaded) { std::fprintf(stderr, "map flags: %s\n", loaded.error().c_str()); return 1; }
      flags = *loaded;
    }
    auto map = app::MapScene::build(*library, flags);
    if (!map) { std::fprintf(stderr, "map: %s\n", map.error().c_str()); return 1; }
    if (!args.shot.empty()) {
      if (auto written = writePng(args.shot, map->image()); !written) {
        std::fprintf(stderr, "map screenshot: %s\n", written.error().c_str()); return 1;
      }
      std::printf("wrote %s (640x480 map SOURCE composition)\n", args.shot.c_str());
      return 0;
    }
#if WEBOLUTION_HAVE_SDL2
    if (cues) {
      cues->start(*library, runOptions.enableAudio && !args.noAudio);
      cues->playFrontendMusic();
    }
    auto& presentation = *sharedWindow;
    if (auto ready = presentation.ensure("Lemmings Webolution " WEBOLUTION_VERSION_STRING
                                         " — conditional map base layer (SOURCE)"); !ready) {
      std::fprintf(stderr, "SDL: %s\n", ready.error().c_str()); return 1;
    }
    auto* renderer = presentation.renderer();
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, 640, 480);
    if (!texture || SDL_UpdateTexture(texture, nullptr, map->image().rgba.data(), 640*4) != 0) {
      std::fprintf(stderr, "SDL map texture: %s\n", SDL_GetError());
      if (texture) SDL_DestroyTexture(texture);
      return 1;
    }
    SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
    bool running = true;
    do {
#ifdef __EMSCRIPTEN__
      web::nextFrame();
#endif
      SDL_Event event; while (SDL_PollEvent(&event)) if (event.type == SDL_QUIT ||
          (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)) running = false;
      SDL_RenderClear(renderer); SDL_RenderCopy(renderer, texture, nullptr, nullptr); SDL_RenderPresent(renderer);
      if (args.frames > 0) running = false;
    } while (running);
    SDL_DestroyTexture(texture);
    return 0;
#else
    std::fprintf(stderr, "SDL2 unavailable: map window mode is not built (use --shot for headless rendering)\n");
    return 1;
#endif
  }

  // Use the same archive resolver as sim::loadRetailLevel. Case-insensitive
  // matches such as MM_Block_Bomb_A take the archive spelling for downstream consumers.
  const auto resolvedLevel = sim::resolveRetailLevel(*library, args.level);
  if (!resolvedLevel) {
    std::fprintf(stderr, "level: %s\n", resolvedLevel.error().c_str());
    return 1;
  }
  if (resolvedLevel->match != sim::LevelMatch::Substring && resolvedLevel->id != args.level)
    args.level = resolvedLevel->id;
  auto blob = library->read(resolvedLevel->file);
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
  std::optional<app::EntranceExitPresentation> entranceExitPresentation;
  std::optional<app::LeverPresentation> leverPresentation;
  std::optional<app::PressurePadPresentation> pressurePadPresentation;
  std::optional<app::CrabPresentation> crabPresentation;
  std::optional<app::NamedHazardPresentation> namedHazardPresentation;
  std::optional<app::GravityPresentation> gravityPresentation;
  std::optional<app::GadgetPresentation> gadgetPresentation;
  if (!args.sceneOff) {
    auto built = app::BackdropScene::build(*library, *blob, args.level);
    if (built) {
      backdrop.emplace(std::move(*built));
      auto effects = backdrop->entranceExitPresentation(*blob);
      if (effects) entranceExitPresentation.emplace(std::move(*effects));
      else std::fprintf(stderr, "hatch/exit presentation: %s\n", effects.error().c_str());
      auto levers = backdrop->leverPresentation(*blob);
      if (levers) leverPresentation.emplace(std::move(*levers));
      else std::fprintf(stderr, "lever presentation: %s\n", levers.error().c_str());
      auto pads = backdrop->pressurePadPresentation(*blob);
      if (pads) pressurePadPresentation.emplace(std::move(*pads));
      else std::fprintf(stderr, "pressure pad presentation: %s\n", pads.error().c_str());
      auto crabs = backdrop->crabPresentation(*blob);
      if (crabs) crabPresentation.emplace(std::move(*crabs));
      else std::fprintf(stderr, "crab presentation: %s\n", crabs.error().c_str());
      auto hazards = backdrop->namedHazardPresentation(*blob);
      if (hazards) namedHazardPresentation.emplace(std::move(*hazards));
      else std::fprintf(stderr, "named hazard presentation: %s\n", hazards.error().c_str());
      auto gravity = backdrop->gravityPresentation(*blob);
      if (gravity) gravityPresentation.emplace(std::move(*gravity));
      else std::fprintf(stderr, "%s\n", gravity.error().c_str());
      auto gadgets = backdrop->gadgetPresentation(*blob);
      if (gadgets) {
        gadgetPresentation.emplace(std::move(*gadgets));
        backdrop->animateGadgets(*gadgetPresentation, 0.0);
      } else { std::fprintf(stderr, "%s\n", gadgets.error().c_str()); return 1; }
      // Play start (0x004451E5 analogue): reseed from the application clock before the
      // first scenery draw of this level.
      backdrop->setRandom(&sceneryRandom().rng);
      if (runOptions.scenerySeed) sceneryRandom().rng.srand(*runOptions.scenerySeed);
      else sceneryRandom().seed();
    }
    else std::fprintf(stderr,"backdrop: unsupported/invalid scene: %s\n",built.error().c_str());
  }
  bool spriteSeedReported = false;

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
    // Draw the route alone unless --route keepwall requests the constant lattice too.
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
    // keepwall leaves the constant lattice enabled.
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

  // With --sim, live World actors replace the supplied diagnostic walkers.
  app::LemmingMotionTracker lemmingMotion;
  std::optional<sim::World> world;
  int terrainHeight = 0;
  if (args.sim) {
    auto setup = sim::loadRetailLevel(*library, args.level);
    if (!setup) {
      std::fprintf(stderr, "sim: %s\n", setup.error().c_str());
      return 1;
    }
    // Connected play gates the 1.3 s first countdown on entry presentation.
    // Recurring release intervals still come from World.
    if (persistentFrontend) setup->releaseArmSeconds = 0.0;
    world.emplace(*setup);
    world->setDeathObserver([&](const sim::Lemming& lemming, bool explosion,
        std::span<const std::array<int, 2>> removed) {
      scene->emitDeathParticles(lemming, explosion, removed, world->levelClockMs(),
                                world->terrain().height(), *library, world->nukeRequested());
    });
    world->setForcedDeathObserver([&](const sim::Lemming& victim) {
      auto sample = terrainSampleFromLemming(victim, world->terrain().height());
      const auto& lemmings = world->lemmings();
      const auto lemming = std::ranges::find_if(lemmings, [&](const auto& l) { return &l == &victim; });
      if (lemming != lemmings.end()) {
        sample.z = lemmingMotion.positionOf(lemming - lemmings.begin(), *world).z;
      } else {
        const auto& rats = world->rats();
        const auto rat = std::ranges::find_if(rats, [&](const auto& r) { return &r.actor == &victim; });
        if (rat != rats.end())
          sample.z = lemmingMotion.ratPositionOf(rat - rats.begin(), *world).z;
      }
      scene->emitForcedDeathFlight(victim, sample, world->levelClockMs());
    });
    world->setTerrainCutObserver([&](const sim::Lemming& lemming,
        std::span<const std::array<int, 2>> removed) {
      scene->emitTerrainDebris(lemming, removed, world->levelClockMs(), world->terrain().height());
    });
    world->setMinerRefusalObserver([&](const sim::Lemming& lemming) {
      scene->emitMinerAxe(lemming, world->levelClockMs(), world->terrain().height());
    });
    for (int i = 0; i < args.simTicks; ++i)
      world->tick();
    scene->advanceDeathParticles(world->levelClockMs());
    terrainHeight = world->terrain().height();
    // READ 0x40BAA0 / 0x45CC09: camera height uses the loaded cylinder rows.
    // Explicit diagnostic camera files keep their own supplied operands.
    if (args.frontend && args.camera.empty()) params.levelEyeHeight = terrainHeight;
    options.injectedWalkers = app::injectedWalkersFromWorld(*world, terrainHeight);

    if (logLevelDetails || args.simTicksSpecified) {
      const auto& lemmings = world->lemmings();
      const auto liveCount = static_cast<std::size_t>(std::ranges::count_if(
          lemmings, [](const sim::Lemming& lemming) { return lemming.alive(); }));
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
  }

  // Connected entry: 0x00444D1E seeds yaw from the selected hatch base and
  // resets zoom. Keep the borrowed LVL storage alive through ordered parsing.
  // Diagnostic routes and explicit camera files retain their supplied operands.
  if (args.frontend && args.camera.empty()) {
    auto entryCamera = readSelectedHatchCamera(*blob);
    if (!entryCamera) {
      std::fprintf(stderr, "level %s: %s\n", args.level.c_str(), entryCamera.error().c_str());
      return 1;
    }
    params.yawUnits = entryCamera->yawUnits;
    params.zoomShift = 0;
  }
  app::CameraIntro cameraIntro(args.frontend && args.camera.empty() &&
      (!persistentFrontend || persistentFrontend->levelIntroEnabled()));

  const auto prepareScene = [&] {
    app::profile::Scope profilePrepare(app::profile::Preparation);
    // Supply fluid state in sim::LevelSetup::gates order. Without a World the
    // fluid pass is empty. Use BackdropScene::kProvisionalFluidDepthPolicy.
    if (backdrop && world) {
      std::vector<app::BackdropScene::FluidGateState> fluid;
      for (const auto& g : world->setup().gates) {
        // Installed receivers start [+0x150] at zero. The level clock supplies the
        // kind-2 four-second alpha phase.
        fluid.push_back({g.lane, g.height, g.wavePhase, g.animated,
                         (world->levelClockMs() % 4000) / 1000.0});
      }
      backdrop->setFluidState(std::move(fluid));
      if (crabPresentation) backdrop->setCrabStates(*crabPresentation, world->setup().crabs);
      if (namedHazardPresentation) backdrop->setNamedHazardStates(*namedHazardPresentation, world->setup().namedHazards);
      if (gadgetPresentation) backdrop->animateGadgets(*gadgetPresentation, world->levelClockMs()/1000.0);
      if (leverPresentation) {
        std::vector<std::pair<int,int>> faces;
        for (const auto& lever : world->setup().levers)
          if (lever.linked) faces.emplace_back(lever.serializedId, lever.state);
        backdrop->setLeverFaces(*leverPresentation, faces);
      }
      if (pressurePadPresentation) {
        std::vector<std::pair<int,int>> states;
        for (const auto& pad : world->setup().pressurePads)
          states.emplace_back(pad.id, pad.state);
        backdrop->setPressurePadStates(*pressurePadPresentation, states);
      }
    }
    // Sync edited terrain when its version changes.
    if (world && !args.noTerrainSync) {
      scene->syncTerrain(world->terrain(), world->setup().counterGates);
      scene->syncCounterGates(world->setup().counterGates);
    }
  };
  std::vector<double> fluidDrownSceneDepth, pipeSceneDepth;
  bool hasPipes = false;
  if (backdrop) backdrop->visitRoots([&](const auto& source, const auto&) {
    hasPipes |= source.stage == app::BackdropScene::Stage::FluidPipe;
  });
  // Primary is the immutable paired-primary tree. Doors, balloons, hazards,
  // levers and opaque gadget parts are LegacyUnattributed. Pad beams, flipper
  // coils, fluids and the secondary tree draw in uncached passes after the tube.
  std::optional<app::Raster> primaryCache, tubeCache;
  std::optional<app::CameraBasis> primaryPose, tubePose, previousCamera;
  app::BackdropStats primaryStats;
  std::size_t tubeTerrainRevision = 0;
  const auto sameCamera = [](const app::CameraBasis& a, const app::CameraBasis& b) {
    return a.worldToView == b.worldToView && a.projectionVector == b.projectionVector &&
        a.inverseProjectionVector == b.inverseProjectionVector && a.eyeWorld == b.eyeWorld &&
        a.focalX == b.focalX && a.focalY == b.focalY && a.width == b.width &&
        a.height == b.height && a.cx == b.cx && a.cy == b.cy && a.selectorTurns == b.selectorTurns;
  };
  const bool darkBlueSky = backdrop && resolvedLevel->id == "MM_ALL_8_001";
  using TeleportHoverBindings = std::span<const app::GadgetPresentation::TeleportHoverBinding>;
  const auto teleportHover = gadgetPresentation ? TeleportHoverBindings{gadgetPresentation->teleportHover} :
                                                  TeleportHoverBindings{};
  const bool trackSceneHover = args.sim &&
      ((leverPresentation && !leverPresentation->bindings.empty()) ||
       (pressurePadPresentation && !pressurePadPresentation->bindings.empty()) || !teleportHover.empty());
  auto renderFrame = [&](const app::CameraParams& frameParams, app::Raster& target, bool scenePrepared = false) {
    app::profile::Scope profilePrepare(app::profile::Preparation);
    const auto frameCamera = app::buildCamera(frameParams);
    app::BackdropStats backdropStats;
    app::SceneRasterObserver observer;
    std::vector<std::uint8_t> c3Mask;
    std::ofstream c3Out;
    const bool c3Diagnostic=!args.c3WriteMask.empty()||!args.c3WriteEvents.empty();
    const bool primaryHit = !c3Diagnostic && primaryCache && sameCamera(*primaryPose, frameCamera);
    // Draw moving poses directly. Populate the pose caches on a repeated pose.
    const bool cachePose = !previousCamera || sameCamera(*previousCamera, frameCamera);
    previousCamera = frameCamera;
    target.trackSceneHighlights(false);
    if (!primaryHit) target.clear(0, 0, 0);
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
    if (!scenePrepared) prepareScene();
    fluidDrownSceneDepth.clear();
    options.fluidDrownSceneDepth = {};
    options.pipeSceneDepth = {};
    auto geometryOptions = options;
    if (!options.composeActorsBeforeRoute) geometryOptions.injectedWalkers.clear();
    const bool fluidDrowning = std::any_of(options.injectedWalkers.begin(), options.injectedWalkers.end(),
        [](const auto& l) { return l.drownFluidKind >= 0; });
    std::vector<double> afterFluidDepth;
    app::scheduleSceneFrame(
          [&] {
            if (!backdrop) return;
            if (primaryHit) {
              target = *primaryCache;
              backdropStats = primaryStats;
            } else {
              backdrop->renderStage(target, frameCamera, *library, backdropStats,
                  app::BackdropScene::Stage::Primary, c3Diagnostic ? &observer : nullptr);
              if (!c3Diagnostic && cachePose) {
                primaryCache = target; primaryPose = frameCamera; primaryStats = backdropStats;
              }
            }
          },
         [&] {
           target.trackSceneHighlights(trackSceneHover);
           if (backdrop) backdrop->renderStage(target, frameCamera, *library, backdropStats,
               app::BackdropScene::Stage::LegacyUnattributed);
         },
         [&] {
           // Cache the nearest opaque tube fragments and merge after the animated legacy stage.
           if (!geometryOptions.depthTest || geometryOptions.flat || geometryOptions.composeActorsBeforeRoute || c3Diagnostic) {
             scene->render(target, frameCamera, geometryOptions, library);
             return;
           }
           const auto revision = scene->terrainBlocksRebuilt() + scene->counterGateRevision();
           const auto renderTube = [&](app::Raster& destination) {
             app::detail::RasterCommandBatch batch(frameCamera.height);
             scene->render(destination, frameCamera, geometryOptions, library);
             app::profile::Scope rasterTime(app::profile::TubeRaster);
             batch.flush();
           };
           if (!tubeCache || tubeTerrainRevision != revision || !sameCamera(*tubePose, frameCamera)) {
             if (!cachePose) {
               renderTube(target);
               return;
             }
             if (!tubeCache || tubeCache->framebuffer().width != frameCamera.width ||
                 tubeCache->framebuffer().height != frameCamera.height) {
               tubeCache.emplace(frameCamera.width, frameCamera.height);
               tubeCache->trackSceneHighlights(trackSceneHover);
             }
             tubeCache->clear(0, 0, 0);
             tubeCache->resetDrawCounts();
             renderTube(*tubeCache);
             tubePose = frameCamera; tubeTerrainRevision = revision;
           }
           target.compositeOpaque(*tubeCache);
         },
         [&] {
           // Pad beams and flipper coils add light to the tube colour. The original
           // draws them inside its tube walker. Composite them over the completed
           // opaque tube here, using its depth to keep foreground ledges solid.
           if (backdrop) backdrop->renderStage(target, frameCamera, *library, backdropStats,
               app::BackdropScene::Stage::GadgetEffect);
           // 0x4418C2: draw all receiver pipes after the tube and before water.
           // Include their opaque depth in the drowning-actor scenery snapshot.
           if (backdrop) {
             if (hasPipes)
               pipeSceneDepth.assign(target.depthBuffer().size(), std::numeric_limits<double>::infinity());
             backdrop->renderStage(target, frameCamera, *library, backdropStats,
                 app::BackdropScene::Stage::FluidPipe, nullptr, hasPipes ? &pipeSceneDepth : nullptr);
             if (hasPipes) options.pipeSceneDepth = pipeSceneDepth;
           }
           if (fluidDrowning) fluidDrownSceneDepth = target.depthBuffer();
           if (backdrop) backdrop->renderStage(target, frameCamera, *library, backdropStats, app::BackdropScene::Stage::FluidGate);
           if (fluidDrowning) afterFluidDepth = target.depthBuffer();
         },
         [&] {
           if (app::ScopedFrameConsumption::observing())
             app::detail::frameSpriteCoverage().assign(std::size_t(frameCamera.width) * frameCamera.height, 0);
           std::vector<std::uint8_t> spriteObserveAll;
           if (app::ScopedFrameConsumption::observing())
             spriteObserveAll.assign(std::size_t(frameCamera.width) * frameCamera.height, 1);
           app::SceneRasterObserver spriteCoverage;
           spriteCoverage.mask = &spriteObserveAll;
           if (app::ScopedFrameConsumption::observing()) spriteCoverage.accepted = [&](const app::SceneRasterWriteEvent& e) {
             if (backdrop->spriteState(e.source)) app::detail::frameSpriteCoverage()[e.pixel] = 1;
           };
           if (backdrop) backdrop->renderStage(target, frameCamera, *library, backdropStats,
               app::BackdropScene::Stage::Secondary,
               app::ScopedFrameConsumption::observing() ? &spriteCoverage : nullptr);
           if (world) scene->renderCounterGates(target, frameCamera, *library, world->setup().counterGates);
         });
    if (darkBlueSky) {
      // Provisional clone sky fills uncovered pixels after water blends with the black clear.
      auto& rgb = target.mutableFramebuffer().rgb;
      const auto& depth = target.depthBuffer();
      for (std::size_t p = 0; p < depth.size(); ++p) {
        if (depth[p] != std::numeric_limits<double>::infinity()) continue;
        const std::size_t i = 3 * p;
        if (rgb[i] != 0 || rgb[i + 1] != 0 || rgb[i + 2] != 0) continue;
        rgb[i] = 4;
        rgb[i + 1] = 8;
        rgb[i + 2] = 24;
      }
    }
    if (fluidDrowning) {
      // Provisional drowning occlusion includes all opaque stages, including
      // the later secondary pass. The additive figure can show through its fluid.
      const auto& completed = target.depthBuffer();
      for (std::size_t p = 0; p < fluidDrownSceneDepth.size(); ++p)
        if (completed[p] != afterFluidDepth[p]) fluidDrownSceneDepth[p] = completed[p];
      options.fluidDrownSceneDepth = fluidDrownSceneDepth;
    }
    // Test actors against the completed scene depth.
    scene->drawExplosionDecals(target, frameCamera);
    if (options.drawCellRoute && !options.composeActorsBeforeRoute)
      scene->drawInjectedWalkers(target, frameCamera, options, *library);
    scene->drawDeathParticles(target, frameCamera, *library, options.pipeSceneDepth);
    if (backdrop) backdrop->renderStage(target, frameCamera, *library, backdropStats,
        app::BackdropScene::Stage::TeleportEffect);
#ifndef __EMSCRIPTEN__
    if (logLevelDetails && !args.sceneOff) {
      std::fprintf(stderr, "backdrop: enabled=%d roots=%zu records=%zu triangles=%zu billboards=%zu "
                   "null-materials=%zu null-triangles=%zu predicate-rejected=%zu "
                   "missing-named-textures=%zu rejected=%zu pixels-depth-written=%zu\n",
                   backdrop.has_value(), backdropStats.roots, backdropStats.meshRecords,
                   backdropStats.meshTriangles, backdropStats.billboards, backdropStats.nullMaterials,
                   backdropStats.nullMaterialTriangles, backdropStats.predicateRejected,
                   backdropStats.missingTextures, backdropStats.rejected, backdropStats.pixels);
    }
#endif
    if (logLevelDetails && backdrop && !spriteSeedReported && backdrop->firstSpriteDrawRandomState()) {
      spriteSeedReported = true;
      const char* seedSource = runOptions.scenerySeed ? "supplied test seed" :
          "seeded from the application clock; flame phases are not reproducible";
      std::fprintf(stderr, "backdrop: sprite scenery generator state at first draw = 0x%08x (%s)\n",
                   *backdrop->firstSpriteDrawRandomState(), seedSource);
    }
    return frameCamera;
  };
  app::Raster raster(params.width, params.height);
  const auto camera = renderFrame(params, raster);

  if (logLevelDetails) {
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
  }
  for (const auto& missing : scene->missingTiles())
    std::fprintf(stderr, "tile: %s\n", missing.c_str());

  if (!args.shot.empty()) {
    Image image = toImage(raster.framebuffer());
    if (args.skillPanel) {
      // Draw the headless skill panel.
      if (auto panel = app::drawRetailSkillPanel(image, *library, args.level); !panel) {
        std::fprintf(stderr, "skill-panel: %s\n", panel.error().c_str());
        return 1;
      }
    }
    if (args.hudTicks >= 0) {
      // Use a separate World for the headless HUD overlay.
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
      // Draw the pause caption over the HUD.
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
    if (args.cursorFixture) {
      auto cursor = app::RuntimeCursor::load(*library);
      if (!cursor) { std::fprintf(stderr, "cursor: %s\n", cursor.error().c_str()); return 1; }
      cursor->draw(image, app::CursorState::NoLemming, image.width / 2, image.height / 2);
    }
    if (auto written = writePng(args.shot, image); !written) {
      std::fprintf(stderr, "screenshot: %s\n", written.error().c_str());
      return 1;
    }
    std::printf("wrote %s (%dx%d)\n", args.shot.c_str(), image.width, image.height);
    return 0;
  }

#if WEBOLUTION_HAVE_SDL2
  // Start the process audio adapter here if the frontend has not started it.
  if (cues) {
    cues->start(*library, runOptions.enableAudio && !args.noAudio);
    cues->prepareLevelMusic();
    if (!(args.sim && world)) cues->playLevelMusic(); // Scene-only diagnostics have no entry clock.
  }
  auto& presentation = *sharedWindow;
  if (persistentFrontend) presentation.updateWindowScale(persistentFrontend->windowScale());
  if (auto ready = presentation.ensure(
      "Lemmings Webolution " WEBOLUTION_VERSION_STRING
      " — right mouse + NUMPAD4/6 revolve; drag mouse; SPACE/CTRL zoom; BACKSPACE flip"); !ready) {
    std::fprintf(stderr, "SDL: %s\n", ready.error().c_str()); return 1;
  }
  SDL_Renderer* presenter = presentation.renderer();
  // RGBX uploads use the first three bytes of the composed HUD pixels.
  bool uploadRgbx = args.sim && world;
  const auto makeTexture = [&](Uint32 format) {
    return SDL_CreateTexture(presenter, format, SDL_TEXTUREACCESS_STREAMING, 640, 480);
  };
  std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> ownedTexture(
      makeTexture(uploadRgbx ? SDL_PIXELFORMAT_RGBX32 : SDL_PIXELFORMAT_RGB24), SDL_DestroyTexture);
  if (!ownedTexture && uploadRgbx) {
    uploadRgbx = false;
    ownedTexture.reset(makeTexture(SDL_PIXELFORMAT_RGB24));
  }
  std::vector<std::uint8_t> rgbFallback;
  SDL_Texture* texture = ownedTexture.get();
  if (!texture) { std::fprintf(stderr, "SDL texture: %s\n", SDL_GetError()); return 1; }
  if (SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest) != 0) {
    std::fprintf(stderr, "SDL texture scale: %s\n", SDL_GetError()); return 1;
  }

  params.width = 640; params.height = 480; params.aspect = 4.0 / 3.0;
  app::CameraController controls(params, terrainHeight > 0 ? terrainHeight :
                                 static_cast<int>(params.levelEyeHeight));
  app::Raster windowRaster(640, 480);
  bool running = true, leftDown = false, focused = true;
  Uint8 dragButton = 0;
  app::TouchInput touch;
  app::TouchInput menuTouch;
  bool touchDragging = false;
  std::optional<app::TouchPoint> touchMotion, touchHold;
  std::optional<app::TouchPoint> touchCursor;
  std::optional<std::size_t> touchHatch;
  std::vector<app::TouchAction> pinchZooms;
  // The logical SDL pointer drives hover, clicks and zoom. Camera drag freezes it.
  int pointerX = 320, pointerY = 240;
  const auto scenePointer = [&] {
    return touchCursor ? std::pair{app::touchPixel(touchCursor->x), app::touchPixel(touchCursor->y)}
                       : std::pair{pointerX, pointerY};
  };
  app::PendingCameraEvents pending;
  bool examineDragMotionPending = false;
  int rendered = 0;
  const Uint64 frequency = SDL_GetPerformanceFrequency();
  Uint64 previous = SDL_GetPerformanceCounter();
  app::FixedCameraClock fixedClock;
  // The 144 Hz camera clock is independent of simulation and redraw cadence.
  // Connected entry starts at tick 0. --sim-ticks can seed the direct-level World.
  app::RuntimeSimState runtimeSim;
  bool entryVoicePending = true;
  const auto uiClick = [&] { if (cues) cues->playUiClick(); };
  // Audio events are taken after each simulation update and successful skill
  // assignment. Snapshot the current camera; active one-shots keep their gains.
  const std::function<void()> drainCues = [&] {
    if (cues && world) {
      const auto camera = app::buildCamera(cameraIntro.pose(controls.camera()));
      cues->drain(*world, [&](const std::array<double, 3>& position) {
        return audio::spatialSoundGains(camera, scene->soundWorldPosition(position, terrainHeight));
      });
    }
  };
  if (world && backdrop && gadgetPresentation) {
    world->setGadgetObserver([&](sim::World::GadgetUse use, std::size_t index, const sim::Lemming& actor) {
      const bool teleport = use != sim::World::GadgetUse::SpeedPad;
      const auto sample = backdrop->triggerGadget(*gadgetPresentation, teleport, index,
          use == sim::World::GadgetUse::TeleportArrival, static_cast<int>(actor.facing),
          world->levelClockMs()/1000.0);
      if (cues && !sample.empty()) {
        const auto camera = app::buildCamera(cameraIntro.pose(controls.camera()));
        const std::array<double,3> position{double(actor.x), double(terrainHeight-1-actor.y), .75};
        cues->playGadgetSample(sample, audio::spatialSoundGains(camera, scene->soundWorldPosition(position, terrainHeight)));
      }
    });
  }
  struct HudIcon {
    const char* asset;
    app::SkillIconPlacement placement;
  };
  // The 640x480 HUD quads set up at 0x44CB3E, 0x44CB8D and 0x44CBDC.
  constexpr std::array hudIcons{
      HudIcon{"TEXTURES\\^OUT.BMP", {557, 15, 0, 0, 26, 32}},
      HudIcon{"TEXTURES\\^SAVED.BMP", {560, 53, 0, 0, 20, 32}},
      HudIcon{"TEXTURES\\^TIME.BMP", {579, 395, 0, 0, 26, 32}},
  };
  const auto overStatusHud = [&](int x, int y, std::span<const shell::HudLine> lines,
                                 std::span<const shell::GlyphCell> cells) {
    return std::ranges::any_of(hudIcons, [&](const auto& icon) {
      const auto& rect = icon.placement;
      return app::pointerOverSceneBounds(x, y, rect.destX, rect.destY,
          rect.destX + rect.width, rect.destY + rect.height);
    }) || std::ranges::any_of(lines, [&](const auto& line) {
      const auto box = shell::lineBox(line, cells);
      return app::pointerOverSceneBounds(x, y, box.x0, box.y0, box.x1, box.y1);
    });
  };
  std::array<Image, hudIcons.size()> runtimeHudIcons;
  std::optional<Image> runtimeFont;
  app::LemmingAnimTracker lemmingAnim;
  std::optional<app::RetailSkillPanel> runtimePanel;
  std::optional<app::RuntimeReleaseRateControl> releaseControl;
  std::optional<app::SkillPanelIconAnimation> panelIconAnimation;
  std::optional<app::RuntimeCursor> runtimeCursor;
  std::optional<app::EntryStartCue> entryCue;
  std::optional<app::LevelStartSequence> levelStart;
  double hatchOpeningProgress = 0.0;
  std::optional<app::LevelMenu> levelMenu;
  std::string entryCueText;
  if (args.sim && world) {
    // Direct play uses the short entry clock for music. The connected frontend owns the title and release gate.
    entryCue.emplace(persistentFrontend && persistentFrontend->levelIntroEnabled());
    auto loadedFont = loadTexture(*library, "TEXTURES\\^FONT.BMP");
    auto loadedPanel = app::RetailSkillPanel::create(*library, args.level);
    auto loadedCursor = app::RuntimeCursor::load(*library);
    if (!loadedFont || !loadedPanel || !loadedCursor) {
      std::fprintf(stderr, "runtime HUD: %s\n",
                    (!loadedFont ? loadedFont.error() : !loadedPanel ? loadedPanel.error() : loadedCursor.error()).c_str());
      return 1;
    }
    runtimeFont = std::move(*loadedFont);
    for (std::size_t i = 0; i < hudIcons.size(); ++i) {
      auto icon = loadTexture(*library, hudIcons[i].asset);
      if (icon) runtimeHudIcons[i] = std::move(*icon);
      else std::fprintf(stderr, "runtime HUD icon: %s\n", icon.error().c_str());
    }
    auto loadedRelease = app::RuntimeReleaseRateControl::create(*library, args.level);
    if (!loadedRelease) {
      std::fprintf(stderr, "release control: %s\n", loadedRelease.error().c_str());
      return 1;
    }
    releaseControl = std::move(*loadedRelease);
    runtimePanel = std::move(*loadedPanel);
    auto loadedIcons = app::SkillPanelIconAnimation::create(*library);
    if (!loadedIcons) { std::fprintf(stderr, "panel animation: %s\n", loadedIcons.error().c_str()); return 1; }
    panelIconAnimation = std::move(*loadedIcons);
    runtimeCursor = std::move(*loadedCursor);
    if (persistentFrontend) {
      auto strings = loadStrings(*library, "UK");
      if (!strings || strings->size() <= 71) {
        std::fprintf(stderr, "entry cue: missing localized string71\n");
        return 1;
      }
      entryCueText = (*strings)[71]; // READ45CAEA pushes hex47: Let's Go!, not Continue/name.
      double doorSeconds = 0.0;
      if (entranceExitPresentation)
        for (const auto& hatch : entranceExitPresentation->entrances)
          doorSeconds = std::max(doorSeconds, hatch.animation.duration);
      levelStart.emplace(doorSeconds);
      auto loadedMenu = app::LevelMenu::load(*library);
      if (!loadedMenu) {
        std::fprintf(stderr, "level menu: %s\n", loadedMenu.error().c_str());
        return 1;
      }
      levelMenu = std::move(*loadedMenu);
    }
  }
  // The source skill panel uses live counts and selection in the HUD layout.
  app::RuntimeSkillSelection skillSelection;
  app::RuntimeNukeInput nukeInput;
  std::optional<app::ReplayRecorder> recorder;
  if (!args.recordDir.empty() && world && releaseControl) {
    auto made = app::ReplayRecorder::create(args.recordDir, args.level, persistentFrontend != nullptr,
                                           args.simTicks, *world, logLevelDetails);
    if (made) recorder.emplace(std::move(*made));
    else std::fprintf(stderr, "record: %s; playing without recording\n", made.error().c_str());
  }
  const auto recordCommands = [&] {
    if (recorder) recorder->commands(*world, runtimeSim, skillSelection);
  };
  std::optional<Image> finalWindowFrame;
  auto displayedCamera = app::buildCamera(cameraIntro.pose(controls.camera()));
  auto displayedPickReference = controls.pickReference();
  auto displayedScenePose = cameraIntro.pose(controls.camera());
  // After a discarded loop frame, a click refreshes invalid scene depth.
  bool windowSceneCurrent = true;
  bool pointerInWindow = true;
  // First-draw sprite latches consume RNG in dispatch order. Until every sprite
  // has latched, retain every full draw (including permanently hidden nodes).
  // Skipping must not defer a latch or change the wall-clock seeding policy.
  bool scenerySpritesReady = false;
  const int previousCursorVisibility = SDL_ShowCursor(SDL_QUERY);
  if (runtimeCursor) SDL_ShowCursor(SDL_DISABLE);
  const auto menuOpen = [&] { return levelMenu && levelMenu->isOpen(); };
  // Only the last presented frame can offer the plate as a touch target.
  bool touchMenuPresented = false;
  const auto cancelFastForwardHold = [&] {
    leftDown = false;
    touchHold.reset();
    runtimeSim.setFastForward(false);
  };
  const auto updateFastForwardHold = [&] {
    // Provisional clone input: hold button 0 over plate 8, including dragging onto it.
    const auto onPlate = [](int x, int y) {
      return app::runtimeSkillPanelCommand(x, y) == app::RuntimePanelCommand::FastForward;
    };
    const bool active = (leftDown && onPlate(pointerX, pointerY)) ||
        (touchHold && onPlate(app::touchPixel(touchHold->x), app::touchPixel(touchHold->y)));
    if (active && !runtimeSim.fastForward()) uiClick(); // Original plate 8 has no play call.
    runtimeSim.setFastForward(active);
  };
  const auto ensureDisplayedScene = [&] {
    if (!windowSceneCurrent) {
      displayedCamera = renderFrame(displayedScenePose, windowRaster, true);
      windowSceneCurrent = true;
    }
  };
  const auto pickActorAt = [&](int x, int y, sim::Skill skill) {
    // Use the displayed scene even if its observer did not inspect pixels.
    ensureDisplayedScene();
    return hoveredActor(*world, *scene, options, *library, displayedCamera, windowRaster,
                        x, y, skill, args.level == "MM_CLIMB_002");
  };
  const auto assignActor = [&](std::optional<std::size_t> hit, sim::Skill skill) {
    // Record the resolved simulation target, never the picking pixels.
    const auto before = recorder && hit ? std::optional{world->lemmings()[*hit]} : std::nullopt;
    const bool assigned = hit && world->assign(*hit, skill);
    if (before) recorder->assignment(*hit, skill, assigned, *before);
    drainCues();
    // Preserve standard pause's resume on successful assignment.
    if (assigned && runtimeSim.paused()) {
      runtimeSim.resume();
      previous = SDL_GetPerformanceCounter();
    }
  };
  const auto zoomAtPointer = [&](int x, int y) -> std::optional<app::CameraZoomPose> {
    if (controls.camera().zoomShift != 0 || rendered == 0 ||
        x < 0 || x >= displayedCamera.width || y < 0 || y >= displayedCamera.height ||
        (runtimePanel && app::runtimeSkillPanelIndex(x, y) >= 0))
      return std::nullopt;
    ensureDisplayedScene();
    const auto pixel = std::size_t(y) * displayedCamera.width + x;
    const double actorDepth = windowRaster.actorDepth().depthAt(pixel);
    const double depth = std::isfinite(actorDepth) ? actorDepth : windowRaster.depthBuffer()[pixel];
    return app::cursorZoomPose(displayedCamera, cameraIntro.pose(controls.camera()),
        x + 0.5, y + 0.5, depth);
  };
  std::optional<Uint64> releasePressedAt;
  const auto hatchPressTarget = [&](int x, int y, int button, bool actorCanReceiveSkill)
      -> std::optional<std::size_t> {
    if (!releaseControl || !backdrop || app::runtimeSkillPanelIndex(x, y) >= 0) return std::nullopt;
    ensureDisplayedScene();
    const auto targets = backdrop->hatchHoverTargets(displayedCamera, windowRaster);
    const auto hit = app::RuntimeReleaseRateControl::pressTarget(targets, x, y, button, actorCanReceiveSkill);
    return hit ? std::optional{targets[*hit].index} : std::nullopt;
  };
  const auto startReleaseHold = [&](std::optional<std::size_t> hatch, int button) {
    if (!hatch || !releaseControl->press(hatch, button, *world)) return false;
    releasePressedAt = SDL_GetPerformanceCounter();
    nukeInput.reset();
    uiClick();
    return true;
  };
  const auto tryHatchPress = [&](int x, int y, int button, bool actorCanReceiveSkill) {
    return startReleaseHold(hatchPressTarget(x, y, button, actorCanReceiveSkill), button);
  };
  const auto advanceReleaseHold = [&](double elapsed) {
    if (!releaseControl || !releaseControl->holding()) return false;
    ensureDisplayedScene();
    const auto targets = backdrop->hatchHoverTargets(displayedCamera, windowRaster);
    const auto [x, y] = scenePointer();
    return releaseControl->advance(elapsed, targets, x, y, *world);
  };
  const auto clearTouchHatch = [&] {
    if (!touchHatch) return;
    releaseControl->reset();
    touchHatch.reset();
    touchCursor.reset();
  };
  const auto cancelGameplayHolds = [&] {
    nukeInput.reset();
    if (releaseControl) releaseControl->reset();
    cancelFastForwardHold();
  };
  const auto cameraDragging = [&] { return dragButton || touchDragging; };
  const auto beginCameraDrag = [&](Uint8 button, bool fromTouch = false) {
    if (fromTouch) {
      touchDragging = true;
      pending.clearMouse();
      return;
    }
    dragButton = button;
    SDL_SetRelativeMouseMode(SDL_TRUE);
  };
  const auto endCameraDrag = [&](bool fromTouch = false) {
    if (fromTouch) {
      touchDragging = false;
      return;
    }
    dragButton = 0;
    SDL_SetRelativeMouseMode(SDL_FALSE);
    // Restore the frozen pointer in window pixels for subsequent motion.
    const int scale = presentation.scale();
    SDL_WarpMouseInWindow(SDL_RenderGetWindow(presenter),
                          pointerX * scale + scale / 2, pointerY * scale + scale / 2);
  };
  const auto cancelCameraDrag = [&] {
    dragButton = 0;
    pending.clear();
    examineDragMotionPending = false;
    SDL_SetRelativeMouseMode(SDL_FALSE);
  };
  const auto resetTouch = [&] {
    touch.reset();
    menuTouch.reset();
    endCameraDrag(true);
    touchMotion.reset();
    pinchZooms.clear();
    clearTouchHatch();
    touchCursor.reset();
    if (touchHold) {
      touchHold.reset();
      updateFastForwardHold();
    }
  };
  const auto resetMenuInput = [&] {
    cancelGameplayHolds();
    cancelCameraDrag();
    resetTouch();
    fixedClock = app::FixedCameraClock{};
    previous = SDL_GetPerformanceCounter();
  };
  const auto openMenu = [&] {
    if (persistentFrontend &&
        persistentFrontend->openLevelMenu() != shell::Outcome::Handled) return;
    levelMenu->open();
    if (!runtimeSim.paused()) runtimeSim.togglePause();
    uiClick(); // Provisional pause feedback.
    resetMenuInput();
  };
  bool menuDismissedThisFrame = false;
  const auto activateMenu = [&](app::LevelMenuAction action) {
    switch (action) {
      case app::LevelMenuAction::Resume:
      case app::LevelMenuAction::Examine: {
        const bool examine = action == app::LevelMenuAction::Examine;
        levelMenu->close();
        if (persistentFrontend)
          (void)persistentFrontend->closeLevelMenu(examine);
        if (!examine) runtimeSim.resume();
        else if (!runtimeSim.examining()) runtimeSim.toggleExaminePause();
        resetMenuInput();
        uiClick();
        menuDismissedThisFrame = true; break;
      }
      case app::LevelMenuAction::Retry:
        disposition = LegDisposition::RetryLevel; running = false; break;
      case app::LevelMenuAction::MapQuit:
        disposition = LegDisposition::ReturnToMap; running = false; break;
      case app::LevelMenuAction::SoundOptions:
        persistentFrontend->openSoundOptions();
        break;
      case app::LevelMenuAction::None: break;
    }
  };
  app::profile::Run frameProfile;
  const char* profilePath = std::getenv("WEBOLUTION_FRAME_PROFILE");
  SDL_RendererInfo rendererInfo{};
  if ((logLevelDetails || (profilePath && *profilePath)) &&
      SDL_GetRendererInfo(presenter, &rendererInfo) == 0)
    std::fprintf(stderr, "presenter: %s flags=0x%x software=%d vsync=%d; simulation frames do not sleep\n",
        rendererInfo.name, rendererInfo.flags, bool(rendererInfo.flags & SDL_RENDERER_SOFTWARE),
        bool(rendererInfo.flags & SDL_RENDERER_PRESENTVSYNC));
  const auto processLevelEvents = [&] {
    bool displayChanged = false;
    const auto routeEvent = [&](const SDL_Event& event, bool fromTouch = false) {
      // A hold can span Escape or a page change, including its consumed event batch.
      if (persistentFrontend) app::updateFrontendCursorInput(*persistentFrontend, event);
      if (event.type == SDL_QUIT) { disposition = LegDisposition::Exit; running = false; return; }
      else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
        cancelGameplayHolds();
        focused = false;
        cancelCameraDrag();
        resetTouch();
      } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
        focused = true;
      } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_LEAVE) {
        pointerInWindow = false;
        if (releaseControl && !touchHatch) releaseControl->reset();
        if (!touchHold) runtimeSim.setFastForward(false);
      } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_ENTER) {
        pointerInWindow = true;
      } else if (menuDismissedThisFrame) {
        // SOURCE event-batch ownership: a dismissal must not let already-queued
        // menu clicks/keys fall through into gameplay. Still service close/focus.
        return;
      } else if (menuOpen()) {
        // Own ALL input while the menu is open, including misses, unsupported
        // rows and gameplay keys. No skill assignment, zoom or relative grab.
        if (!focused) return;
        if (event.type == SDL_MOUSEMOTION) {
          pointerX = event.motion.x; pointerY = event.motion.y;
        } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
          pointerX = event.button.x; pointerY = event.button.y;
        }
        if (event.type == SDL_KEYDOWN && !event.key.repeat && event.key.keysym.scancode == SDL_SCANCODE_O) {
          // READ45D575: O closes all UI2 pages directly to paused UI1.
          levelMenu->close();
          (void)persistentFrontend->closeLevelMenu(true);
          resetMenuInput();
          menuDismissedThisFrame = true;
        } else if (persistentFrontend->screen() == shell::Screen::SoundOptions) {
          app::handleSoundOptionsInput(*persistentFrontend, event, presentation);
          if (persistentFrontend->screen() == shell::Screen::Dialog) {
            levelMenu->open(); // caller 1F is rebuilt with Resume focused
            menuDismissedThisFrame = true;
          }
        } else if (event.type == SDL_MOUSEMOTION) {
          levelMenu->hover(pointerX, pointerY);
        } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
          activateMenu(levelMenu->actionAt(pointerX, pointerY));
        } else if (event.type == SDL_KEYDOWN && !event.key.repeat) {
          const auto key = event.key.keysym.scancode;
          if (key == SDL_SCANCODE_ESCAPE) activateMenu(app::LevelMenuAction::Resume);
          else if (key == SDL_SCANCODE_RETURN) activateMenu(levelMenu->focusedAction());
          else if (key == SDL_SCANCODE_UP || key == SDL_SCANCODE_DOWN)
            levelMenu->moveFocus(key == SDL_SCANCODE_UP ? -1 : 1);
        }
      } else if (runtimeSim.examining() &&
                 (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)) {
        if (fromTouch) {
          pointerX = event.button.x; pointerY = event.button.y;
        } else if (focused && event.type == SDL_MOUSEBUTTONDOWN && !dragButton &&
            (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT)) {
          pointerX = event.button.x; pointerY = event.button.y;
          beginCameraDrag(event.button.button);
        } else if (event.type == SDL_MOUSEBUTTONUP && dragButton == event.button.button) {
          endCameraDrag();
        }
      } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
        if (focused && !dragButton && tryHatchPress(event.button.x, event.button.y, SDL_BUTTON_RIGHT, false)) {
          pointerX = event.button.x; pointerY = event.button.y;
          updateFastForwardHold();
        } else {
          cancelGameplayHolds();
          if (focused) beginCameraDrag(SDL_BUTTON_RIGHT);
        }
      } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
        if (releaseControl) releaseControl->release(event.button.button);
        if (dragButton == SDL_BUTTON_RIGHT) {
          endCameraDrag();
        } else if (focused) {
          pointerX = event.button.x; pointerY = event.button.y;
        }
      } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        if (releaseControl) releaseControl->release(event.button.button);
        cancelFastForwardHold();
        if (args.sim && world && focused && !dragButton) {
          pointerX = event.button.x; pointerY = event.button.y;
          nukeInput.release(pointerX, pointerY, event.button.timestamp, *world);
        } else nukeInput.reset();
      } else if (args.sim && world && event.type == SDL_MOUSEBUTTONDOWN &&
                 event.button.button == SDL_BUTTON_LEFT) {
        // Consume panel clicks before actor picking, including command plates.
        if (focused && !dragButton) {
          leftDown = true;
          pointerX = event.button.x; pointerY = event.button.y;
          nukeInput.press(pointerX, pointerY);
          updateFastForwardHold();
          const int panelIndex = app::runtimeSkillPanelIndex(pointerX, pointerY);
          const auto panelCounts = app::runtimeSkillPanelCounts(*world);
          if ((panelIndex >= 0 && panelIndex < 8 && panelCounts[panelIndex] > 0) ||
              (panelIndex == 9 && world->setup().nukeStock > 0 && !world->nukeRequested()))
            uiClick(); // 0x45AC5C selects stocked skills and Nuke via Changeop.
          if (!skillSelection.consumePanelClick(pointerX, pointerY,
                                                panelCounts)) {
            const auto skill = skillSelection.selectedSkill();
            const auto hit = pickActorAt(pointerX, pointerY, skill);
            const bool actorCanReceiveSkill = hit && world->canAssign(*hit, skill);
            if (!tryHatchPress(pointerX, pointerY, SDL_BUTTON_LEFT, actorCanReceiveSkill))
              assignActor(hit, skill);
          }
        }
      } else if (event.type == SDL_MOUSEMOTION) {
        if (focused) {
          if (!touchDragging && !touchMotion && (dragButton || !runtimeSim.examining())) {
            pending.mouse(double(event.motion.xrel) / presentation.scale(),
                          double(event.motion.yrel) / presentation.scale());
            if (runtimeSim.examining() && (event.motion.xrel || event.motion.yrel))
              examineDragMotionPending = true;
          }
          if (!cameraDragging()) {
            pointerX = event.motion.x; pointerY = event.motion.y;
            advanceReleaseHold(0.0);
            leftDown = leftDown && (event.motion.state & SDL_BUTTON_LMASK);
            updateFastForwardHold();
          }
        }
      } else if (event.type == SDL_MOUSEWHEEL && focused && pointerInWindow && runtimeSim.examining()) {
        const double y = event.wheel.preciseY != 0 ? event.wheel.preciseY : event.wheel.y;
        const double direction = event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -y : y;
        if (direction != 0 && (direction > 0) != (controls.camera().zoomShift != 0))
          pending.zoom(zoomAtPointer(pointerX, pointerY));
      } else if (event.type == SDL_KEYDOWN && !event.key.repeat) {
        const auto key = event.key.keysym.scancode;
        if (focused && (key == SDL_SCANCODE_SPACE || key == SDL_SCANCODE_LCTRL || key == SDL_SCANCODE_RCTRL))
          pending.zoom(zoomAtPointer(pointerX, pointerY));
        if (focused && key == SDL_SCANCODE_BACKSPACE) pending.flip();
        if (key == SDL_SCANCODE_ESCAPE) {
          if (persistentFrontend && !focused) return;
          if (args.sim && world && runtimeSim.paused()) {
            if (dragButton == SDL_BUTTON_LEFT) endCameraDrag();
            resetTouch();
            runtimeSim.resume();
            previous = SDL_GetPerformanceCounter();
            uiClick();
          } else if (levelMenu) openMenu();
          else running = false; // Preserve standalone diagnostic Escape-close.
        }
        if (levelMenu && focused && key == SDL_SCANCODE_O && runtimeSim.paused()) {
          openMenu();
        }
        const bool examineKey = key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER || key == SDL_SCANCODE_PAUSE;
        if (args.sim && world && focused && (examineKey || key == SDL_SCANCODE_P)) {
          resetTouch();
          if (examineKey) runtimeSim.toggleExaminePause();
          else runtimeSim.togglePause();
          if (runtimeSim.examining()) {
            cancelGameplayHolds();
            if (!dragButton && !examineDragMotionPending) pending.clearMouse();
          } else if (dragButton == SDL_BUTTON_LEFT) endCameraDrag();
          previous = SDL_GetPerformanceCounter();
          uiClick();
        }
        if (args.sim && world && focused && !runtimeSim.examining()) {
          int selected = -1;
          switch (key) {
            case SDL_SCANCODE_1: selected = 0; break;
            case SDL_SCANCODE_2: selected = 1; break;
            case SDL_SCANCODE_3: selected = 2; break;
            case SDL_SCANCODE_4: selected = 3; break;
            case SDL_SCANCODE_5: selected = 4; break;
            case SDL_SCANCODE_6: selected = 5; break;
            case SDL_SCANCODE_7: selected = 6; break;
            case SDL_SCANCODE_8: selected = 7; break;
            default: break;
          }
          if (skillSelection.selectAvailableIndex(selected, app::runtimeSkillPanelCounts(*world))) {
            uiClick();
            if (logLevelDetails)
              std::fprintf(stderr, "click-to-assign: skill %s (SOURCE input adapter)\n",
                           std::string(sim::skillName(skillSelection.selectedSkill())).c_str());
          }
        }
      }
      recordCommands();
    };
    const auto routeTouch = [&](const app::TouchAction& action, Uint32 timestamp,
                                std::optional<std::size_t> downActor = std::nullopt) {
      using Kind = app::TouchActionKind;
      if (action.kind == Kind::None || menuDismissedThisFrame) return;
      if (action.kind == Kind::Tap) {
        routeEvent(app::touchButton(SDL_MOUSEBUTTONDOWN, action.point, timestamp), true);
        routeEvent(app::touchButton(SDL_MOUSEBUTTONUP, action.point, timestamp), true);
        return;
      }
      if (menuOpen()) return;
      switch (action.kind) {
        case Kind::Assign:
          pointerX = app::touchPixel(action.point.x);
          pointerY = app::touchPixel(action.point.y);
          assignActor(downActor, skillSelection.selectedSkill());
          break;
        case Kind::BeginDrag:
        case Kind::Drag:
        case Kind::EndDrag:
          if (action.kind == Kind::BeginDrag) beginCameraDrag(SDL_BUTTON_RIGHT, true);
          if (!touchMotion) touchMotion.emplace();
          touchMotion->x += action.delta.x;
          touchMotion->y += action.delta.y;
          if (action.kind == Kind::EndDrag) endCameraDrag(true);
          break;
        case Kind::CancelDrag:
          endCameraDrag(true);
          touchMotion.reset();
          break;
        case Kind::BeginHold:
        case Kind::MoveHold:
          if (action.kind == Kind::BeginHold) nukeInput.reset();
          touchHold = action.point;
          pointerX = app::touchPixel(action.point.x);
          pointerY = app::touchPixel(action.point.y);
          updateFastForwardHold();
          break;
        case Kind::EndHold:
        case Kind::CancelHold:
          touchHold.reset();
          updateFastForwardHold();
          break;
        case Kind::Precision:
          touchCursor = action.point;
          break;
        case Kind::EndPrecision: {
          touchCursor.reset();
          const int x = app::touchPixel(action.point.x), y = app::touchPixel(action.point.y);
          if (!runtimeSim.examining() && app::runtimeSkillPanelIndex(x, y) < 0) {
            const auto skill = skillSelection.selectedSkill();
            if (const auto hit = pickActorAt(x, y, skill); hit && world->canAssign(*hit, skill)) {
              nukeInput.reset();
              assignActor(hit, skill);
            }
          }
          break;
        }
        case Kind::CancelPrecision:
          touchCursor.reset();
          break;
        case Kind::HatchSlide: {
          if (!touchHatch) break;
          touchCursor = action.point;
          releaseControl->reset();
          if (action.hatchSlide == app::TouchHatchSlide::Neutral) break;
          const int button = action.hatchSlide == app::TouchHatchSlide::Up ? SDL_BUTTON_RIGHT : SDL_BUTTON_LEFT;
          ensureDisplayedScene();
          const auto targets = backdrop->hatchHoverTargets(displayedCamera, windowRaster);
          // The origin stays on the selected hatch while the finger slides.
          const auto [x, y] = scenePointer();
          if (app::hoveredHatch(targets, x, y, touchHatch)) startReleaseHold(touchHatch, button);
          break;
        }
        case Kind::EndHatchHold:
          clearTouchHatch();
          break;
        case Kind::ZoomIn:
        case Kind::ZoomOut:
          pinchZooms.push_back(action);
          break;
        default: break;
      }
      recordCommands();
    };
    // Sample the idle clock before draining so it cannot overtake an event
    // enqueued after the last poll. Events themselves advance to their own time.
    const auto pollStarted = SDL_GetTicks();
    SDL_Event event;
    while (running && SDL_PollEvent(&event)) {
      if (app::isTouchMouseEvent(event)) continue;
      displayChanged |= event.type == SDL_WINDOWEVENT &&
                        event.window.event == SDL_WINDOWEVENT_DISPLAY_CHANGED;
      const auto screen = persistentFrontend ? persistentFrontend->screen() : shell::Screen::Level;
      if (app::isFingerEvent(event.type)) {
        const auto point = presentation.touchCoordinates(event.tfinger);
        if (!point || !focused || menuDismissedThisFrame) continue;
        const app::TouchId id{event.tfinger.touchId, event.tfinger.fingerId};
        // A contact on the plate belongs to the menu even if a scene gesture
        // is in progress. Its motion and lift never enter the scene recognizer.
        if (menuTouch.tracking(id) ||
            (event.type == SDL_FINGERDOWN && touchMenuPresented && !menuOpen() &&
             levelMenu->touchButtonAt(app::touchPixel(point->x), app::touchPixel(point->y)))) {
          if (presentation.feedTouch(menuTouch, event.tfinger, *point).kind ==
              app::TouchActionKind::Tap) {
            openMenu();
            recordCommands();
          }
          continue;
        }
        if (dragButton || leftDown || (releaseControl && releaseControl->holding() && !touchHatch))
          continue;
        const auto timestamp = event.tfinger.timestamp;
        routeTouch(touch.advance(timestamp), timestamp);
        using Target = app::TouchTarget;
        auto target = Target::Tap;
        std::optional<std::size_t> downActor;
        if (event.type == SDL_FINGERDOWN && touch.idle() && !menuOpen()) {
          const int x = app::touchPixel(point->x), y = app::touchPixel(point->y);
          const int plate = runtimePanel ? app::runtimeSkillPanelIndex(x, y) : -1;
          if (!runtimePanel) target = Target::DragOnly;
          else if (plate == 8 && !runtimeSim.examining()) target = Target::Holdable;
          else if (plate < 0) {
            // A scene touch interrupts the Nuke plate's double-tap sequence.
            nukeInput.reset();
            const auto skill = skillSelection.selectedSkill();
            downActor = pickActorAt(x, y, skill);
            const bool canAssign = downActor && world->canAssign(*downActor, skill);
            const auto hatch = hatchPressTarget(x, y, SDL_BUTTON_LEFT, canAssign);
            if (canAssign) target = runtimeSim.examining() ? Target::DragOnly : Target::AssignableActor;
            else if (hatch && !runtimeSim.examining()) {
              touchHatch = hatch;
              target = Target::Hatch;
            } else {
              const auto cells = shell::fontCells();
              const auto lines = shell::layoutHud(app::runtimeHudValues(*world), {}, cells);
              target = hatch || overStatusHud(x, y, lines, cells) ? Target::DragOnly : Target::Scene;
            }
          }
        }
        routeTouch(presentation.feedTouch(touch, event.tfinger, *point, target), timestamp, downActor);
        if (touch.idle()) clearTouchHatch();
      } else {
        if (event.type == SDL_MOUSEBUTTONDOWN) resetTouch();
        presentation.gameCoordinates(event);
        routeEvent(event);
      }
      if (persistentFrontend && persistentFrontend->screen() != screen) resetTouch();
    }
    if (running && focused && !menuOpen()) routeTouch(touch.advance(pollStarted), pollStarted);
#ifdef __EMSCRIPTEN__
    // Consume wheel events over the canvas only while they control examine zoom.
    SDL_EventState(SDL_MOUSEWHEEL, runtimeSim.examining() && focused && !menuOpen() ? SDL_ENABLE : SDL_IGNORE);
#endif
    if (running) {
      const auto choice = persistentFrontend ? std::optional{persistentFrontend->windowScale()} : std::nullopt;
      presentation.updateWindowScale(choice, displayChanged);
    }
  };
  while (running && (args.frames <= 0 || rendered < args.frames)) {
    menuDismissedThisFrame = false;
#ifdef __EMSCRIPTEN__
    if (args.sim && world) {
      // A click can use the already displayed scene while the next frame is pending.
      while (running && !web::waitForLevelFrameOrClick()) processLevelEvents();
      if (!running) break;
    } else {
      web::nextFrame();
    }
#endif
    app::profile::Run::Frame measuredFrame(frameProfile, args.level.c_str(), rendered, options.injectedWalkers.size());
    processLevelEvents();
    if (!running) break; // Do not compose or observe another frame after close.
    app::profile::Scope profileSim(app::profile::Simulation);
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    app::CameraInput input;
    input.uiAllowsCamera = !menuOpen();
    input.uiState = menuOpen() ? 2 : 0;
    input.numpad4 = focused && !menuOpen() && keys[SDL_SCANCODE_KP_4] != 0;
    input.numpad6 = focused && !menuOpen() && keys[SDL_SCANCODE_KP_6] != 0;
    input.leftShift = focused && !menuOpen() && keys[SDL_SCANCODE_LSHIFT] != 0;
    const Uint64 now = SDL_GetPerformanceCounter();
    double elapsed = static_cast<double>(now - previous) / static_cast<double>(frequency);
    previous = now;
    if (args.frames > 0) elapsed = app::FixedCameraClock::stepSeconds;
    if (runOptions.frameSeconds > 0) elapsed = runOptions.frameSeconds;
    const bool ownedClock = args.frames > 0 || runOptions.frameSeconds > 0;
    const auto [cursorX, cursorY] = scenePointer();
    if (panelIconAnimation) {
      const int hover = focused && !menuOpen() && !cameraDragging() ?
          app::runtimeSkillPanelIndex(cursorX, cursorY) : -1;
      // UI animation uses frame time independently of simulation pause and fast-forward.
      // Provisional policy suspends it while a menu is open or focus is lost.
      panelIconAnimation->advanceIcons(focused && !menuOpen() ? elapsed : 0.0,
                                 skillSelection.selectedIndex(), hover);
    }
    if (releaseControl && releaseControl->holding() && focused && !menuOpen()) {
      // A press already steps once. Count only elapsed time since that press
      // on its first frame. Injected input enters at the frame boundary.
      double holdElapsed = elapsed;
      if (releasePressedAt) {
        holdElapsed = ownedClock ? 0.0 : std::min(elapsed,
            static_cast<double>(now - *releasePressedAt) / static_cast<double>(frequency));
      }
      // One request per rendered frame, even if a long frame catches up several
      // repeats. SkipPlaying discards overlap; nothing accumulates while held.
      if (advanceReleaseHold(holdElapsed)) uiClick();
    }
    releasePressedAt.reset();
    recordCommands();
    fixedClock.advance(elapsed);
    bool updated = false;
    while (fixedClock.takeStep()) {
      // Apply queued examine or touch motion even if release preceded this tick.
      input.rightMouseDown = cameraDragging() || examineDragMotionPending || touchMotion.has_value();
      if (touchMotion) {
        pending.mouse(touchMotion->x, touchMotion->y);
        touchMotion.reset();
      }
      input = pending.consumeInto(input);
      if (!pinchZooms.empty() && input.uiAllowsCamera) {
        // Apply the coalesced keyboard/wheel edge first. Each pinch edge then
        // sees the preceding zoom state, even within a single camera tick.
        if (input.zoomPressed) {
          controls.toggleZoom(input.zoomPose);
          input.zoomPressed = false;
        }
        for (const auto& zoom : pinchZooms) {
          const bool zoomIn = zoom.kind == app::TouchActionKind::ZoomIn;
          if (zoomIn != (controls.camera().zoomShift != 0))
            controls.toggleZoom(zoomAtPointer(app::touchPixel(zoom.point.x), app::touchPixel(zoom.point.y)));
        }
      }
      pinchZooms.clear();
      examineDragMotionPending = false;
      controls.update(input, app::FixedCameraClock::stepSeconds);
      updated = true;
    }
    bool simTicked = false;
    // 45CC09 advances the entry cue after input and before simulation.
    // Start the prepared music once when the cue expires (0x45CEE6).
    if (entryCue && entryCue->active()) {
      entryCue->advanceBeforeFrame(elapsed, focused && !runtimeSim.paused());
      if (!entryCue->active() && cues) cues->playLevelMusic();
    }
    if (levelStart) levelStart->advance(elapsed, focused && !runtimeSim.paused());
    if (args.sim && world) {
      // Convert elapsed time to integer ms and cap wall-clock stalls at 100 ms.
      // --frames and frameSeconds use the supplied budget without the cap.
      // Capture confirmed motion before stepping and after each World update.
      lemmingMotion.observe(*world);
      const auto afterUpdate = [&] {
        drainCues(); lemmingMotion.observe(*world);
        scene->advanceDeathParticles(world->levelClockMs());
      };
      // Hold hatch release until both entry text and the full door clip have finished.
      const bool releaseEnabled = !levelStart || levelStart->releaseEnabled(entryCue && entryCue->active());
      simTicked = runtimeSim.advanceElapsed(*world, elapsed, persistentFrontend != nullptr, afterUpdate,
                                            ownedClock ? 0 : app::RuntimeSimState::kStallCapMs, releaseEnabled);
      if (recorder && simTicked) recorder->advanced(*world, runtimeSim, releaseEnabled);
      // Advance animation with the admitted integer-ms budget times the pass
      // count, or zero when paused. Provisional frame-end transitions receive
      // the full animation delta.
      if (const SpriteSheet* lemmingSheet = scene->lemmingSprites(); lemmingSheet && !args.noActorAnim) {
        const double levelDt = !simTicked ? 0.0 : runtimeSim.lastPassMs() / 1000.0 *
            (runtimeSim.fastForward() ? app::RuntimeSimState::kFastForwardPasses : 1);
        lemmingAnim.advance(*world, std::array<const SpriteSheet*, 3>{
            lemmingSheet, scene->lemmingSprites(1), scene->lemmingSprites(2)}, levelDt);
      }
      options.injectedWalkers = app::injectedWalkersFromWorld(
          *world, terrainHeight, args.noActorAnim ? nullptr : &lemmingAnim, &lemmingMotion);
      if (backdrop && entranceExitPresentation) {
        // Connected entry samples the full clip (25 keys/s) with provisional cue
        // sequencing. Direct diagnostics normalize door progress by releaseArmSeconds.
        const double arm = world->setup().releaseArmSeconds;
        hatchOpeningProgress = levelStart ? levelStart->openingProgress() :
            (arm > 0 ? world->levelClockMs()/1000.0/arm : 1.0);
        const auto& end = runtimeSim.endTransition();
        const bool departing = end.phase() == app::LevelEndPhase::Pending;
        backdrop->setEntranceExitPresentation(*entranceExitPresentation, hatchOpeningProgress,
            departing ? 7.0 - end.timerSeconds() : 0.0, world->levelClockMs() / 1000.0,
            departing ? lemmingMotion.exitBalloonSwayScale(*world) : 1.0);
      }
      if (persistentFrontend && runtimeSim.endTransition().snapshot()) {
        const auto& result = *runtimeSim.endTransition().snapshot();
        completed = app::LevelOutcome{args.level, result.saved, world->setup().totalLemmings,
            result.needed, static_cast<int>(world->timeRemainingTicks() / world->rules().ticksPerSecond),
            result.success};
        disposition = LegDisposition::Completed;
        break;
      }
    }
    profileSim.stop();
    // Scenery clock: frame ms from the existing frame clock (0x0045CFF9 -> 0x004423D2);
    // advances only in the in-level (--sim) state and never while paused.
    if (runtimeSim.paused()) sceneryRandom().pausedMs += elapsed * 1000.0;
    if (updated || simTicked || (args.sim && world)) {
      if (backdrop) {
        backdrop->advanceScenery(elapsed * 1000.0, args.sim && static_cast<bool>(world), runtimeSim.paused());
        if (gravityPresentation) backdrop->animateGravity(*gravityPresentation);
      }
      params = controls.camera();
      const auto displayedPose = cameraIntro.pose(params);
      displayedScenePose = displayedPose;
      displayedCamera = app::buildCamera(displayedPose);
      displayedPickReference = controls.pickReference();
      prepareScene();
      windowSceneCurrent = false;
      // Sample the pose before advancing intro time. Pause freezes the intro clock.
      cameraIntro.advanceAfterFrame(elapsed, runtimeSim.paused());
      Image image;
      std::vector<app::LevelActorView> actors;
      std::vector<app::HatchHoverTarget> hatches;
      bool touchMenuDrawn = false;
      // This state-only view is never offered to an ordinary observer. The
      // explicit automation scope opts into inspectLevelFrame() for pixels,
      // hover, visible actor boxes and hatch depth; all other fields are ready.
      const auto cueFrame = persistentFrontend && entryCue ? entryCue->frame() : std::nullopt;
      std::optional<app::LevelFrameView> pendingView;
      if (args.sim && world && runtimeFont && runtimePanel)
        pendingView.emplace(app::LevelFrameView{rendered, *world, runtimeSim.paused(), skillSelection.selectedIndex(), actors, image,
                         std::nullopt, cursorX, cursorY, app::CursorState::Hidden,
                         runtimeSim.endTransition().phase(), runtimeSim.endTransition().timerSeconds(),
                         cameraIntro.active(), persistentFrontend && entryCue ? entryCue->seconds() : 0.0,
                         cueFrame, menuOpen(), args.level,
                         persistentFrontend ? std::optional<int>{persistentFrontend->constructionCounter()} : std::nullopt,
                         cues && cues->deviceReady(), cues ? cues->drained() : 0, cues ? cues->routed() : 0,
                         displayedPose, displayedPickReference,
                         scene->terrainCellsChanged(), scene->terrainBlocksRebuilt(), hatchOpeningProgress,
                         std::nullopt, std::nullopt,
                         hatches});
      const std::function<void()> materialize = [&] {
        if (windowSceneCurrent) return;
        displayedCamera = renderFrame(displayedPose, windowRaster, true);
        windowSceneCurrent = true;
        if (pendingView) {
          auto& view = *pendingView;
          app::profile::Scope profileHud(app::profile::Hud);
          image = toImage(windowRaster.framebuffer());
          const int panelIndex = app::runtimeSkillPanelIndex(cursorX, cursorY);
          const bool canHover = focused && !menuOpen() && !cameraDragging() && panelIndex < 0;
          const bool cursorInWindow = touchCursor.has_value() || pointerInWindow;
          const auto hudCells = shell::fontCells();
          const auto hudLines = shell::layoutHud(app::runtimeHudValues(*world), {}, hudCells);
          const bool pointerOnHud = !teleportHover.empty() && canHover && cursorInWindow &&
              overStatusHud(cursorX, cursorY, hudLines, hudCells);
          const bool switchHover = canHover && cursorInWindow && backdrop &&
            backdrop->drawSwitchHover(image, displayedCamera, windowRaster, world->setup(),
                leverPresentation ? &*leverPresentation : nullptr,
                pressurePadPresentation ? &*pressurePadPresentation : nullptr,
                pointerOnHud ? TeleportHoverBindings{} : teleportHover, cursorX, cursorY,
                [&](const sim::OccupancySensor& sensor) {
                  scene->drawHoveredSensor(image, displayedCamera, windowRaster, sensor);
                });
          hatches = backdrop ? backdrop->hatchHoverTargets(displayedCamera, windowRaster) :
              std::vector<app::HatchHoverTarget>{};
          for (auto& h : hatches)
            h.remaining = world->remainingFromEntrance(h.index);
          std::optional<std::size_t> heldHatch;
          if (releaseControl && cursorInWindow && focused && !menuOpen() && !cameraDragging()) {
            heldHatch = touchHatch && touchCursor
                ? app::hoveredHatch(hatches, cursorX, cursorY, touchHatch)
                : releaseControl->heldTarget(hatches, cursorX, cursorY);
          }
          const auto selectedSkill = skillSelection.selectedSkill();
          auto hover = canHover && !heldHatch ?
              hoveredActor(*world, *scene, options, *library, displayedCamera, windowRaster,
                  cursorX, cursorY, selectedSkill, args.level == "MM_CLIMB_002") : std::nullopt;
          const bool actorCanReceiveSkill = hover && world->canAssign(*hover, selectedSkill);
          // Idle feedback follows left-click precedence. While adjusting a
          // hatch, the cursor and rate follow the held target.
          auto hatchHover = heldHatch;
          if (!hatchHover && releaseControl && canHover && cursorInWindow)
            hatchHover = app::RuntimeReleaseRateControl::pressTarget(
                hatches, cursorX, cursorY, SDL_BUTTON_LEFT, actorCanReceiveSkill);
          if (hatchHover) hover.reset();
          // The picker can resolve a refused click. Only a skill recipient gets the body redraw.
          if (hover && actorCanReceiveSkill && cursorInWindow && cursorX >= 0 && cursorX < image.width &&
              cursorY >= 0 && cursorY < image.height) {
            // Injected walkers contain the live actors in World order.
            const auto& lemmings = world->lemmings();
            const auto walker = std::count_if(lemmings.begin(), lemmings.begin() + *hover,
                                              [](const sim::Lemming& l) { return l.alive(); });
            if (scene->drawHoveredWalker(image, walker, displayedCamera, options, *library, windowRaster))
              view.markedActor = hover;
          }
          if (hatchHover) {
            const auto& target = hatches[*hatchHover];
            const auto adjustingRate = heldHatch
                ? std::optional{world->setup().entrances[target.index].releaseRate} : std::nullopt;
            app::drawHatchHover(image, *runtimeFont, target, adjustingRate);
          } else if (canHover && cursorInWindow && backdrop && !hover && !switchHover) {
            if (const auto exit = backdrop->exitHoverTarget(displayedCamera, windowRaster, cursorX, cursorY))
              app::drawExitHover(image, *runtimeFont, *exit, world->setup().saveTarget);
          }
          app::drawRuntimeSkillPanel(image, *runtimePanel, *world, skillSelection, runtimeSim.fastForward(),
                                     &*panelIconAnimation);
          for (std::size_t i = 0; i < hudIcons.size(); ++i)
            app::compositeIconRegion(image, runtimeHudIcons[i], hudIcons[i].placement);
          shell::drawHud(image, *runtimeFont, hudLines, hudCells);
          app::drawRuntimeNukeFeedback(image, *runtimeFont, *world, nukeInput.armed());
          // Draw a countdown over each lemming with a lit fuse.
          if (!args.noActorAnim && std::any_of(world->lemmings().begin(), world->lemmings().end(),
                          [](const sim::Lemming& l) { return l.alive() && l.fuseMs > 0; })) {
            const auto fuseBoxes = scene->injectedWalkerScreenBoxes(displayedCamera, options, *library, windowRaster);
            std::vector<app::LevelActorView> fuseActors;
            std::size_t fuseWalker = 0;
            for (std::size_t i = 0; i < world->lemmings().size(); ++i) {
              if (!world->lemmings()[i].alive()) continue;
              if (fuseWalker < fuseBoxes.size() && fuseBoxes[fuseWalker])
                fuseActors.push_back({i, *fuseBoxes[fuseWalker]});
              ++fuseWalker;
            }
            app::drawBomberCountdowns(image, *runtimeFont, *world, fuseActors, windowRaster.mutableDepthBuffer());
          }
          if (cueFrame) {
            app::drawEntryStartCue(image, *runtimeFont, entryCueText, *cueFrame);
            // Request Letsgo on the first drawn cue frame, once per level entry or retry.
            if (entryVoicePending) {
              entryVoicePending = false;
              if (cues) cues->play(sim::SoundCue::LevelStart);
              view.audioCuesDrained = cues ? cues->drained() : 0;
              view.audioCuesRouted = cues ? cues->routed() : 0;
            }
          }
          if (const int remainingMs = world->nukeCountdownMs(); remainingMs > 0) {
            // 44279A..4427B2 centres the Nuke digit at (320,240); 414132 adds (5,6).
            // 413F15/41419F expands it during each second. Provisional 640x480 adapter
            // uses mode 0, dx=trunc(150*phase), dy=trunc(195*phase) and additive ^Font.
            // Provisional countdown rounds the World timer up to display 5..1.
            const int digit = (remainingMs + 999) / 1000;
            const int phaseMs = digit * 1000 - remainingMs;
            const int dx = 150 * phaseMs / 1000, dy = 195 * phaseMs / 1000;
            const app::EntryCueFrame countdown{{320 - dx, 240 - dy, 330 + dx, 252 + dy}, true};
            app::drawEntryStartCue(image, *runtimeFont, std::to_string(digit), countdown);
          }
          if (menuOpen() && persistentFrontend->screen() == shell::Screen::SoundOptions) {
            persistentFrontend->setWindowScaleMenu(presentation.windowScaleMenu());
            image = persistentFrontend->renderSoundOptions(image);
          } else if (menuOpen()) {
            levelMenu->draw(image);
          } else if (levelMenu && presentation.receivedTouch()) {
            levelMenu->drawTouchButton(image);
            touchMenuDrawn = true;
          }
          const auto counts = app::runtimeSkillPanelCounts(*world);
          const int panelAvailability = panelIndex >= 0 && panelIndex < 8 ? counts[panelIndex] : 0;
          const auto cursorState = menuOpen() ? persistentFrontend->cursorState() :
              app::runtimeCursorState(focused, cameraDragging(), hover.has_value(),
                  runtimeSim.paused(), heldHatch ? -1 : panelIndex, panelAvailability, hatchHover.has_value());
          runtimeCursor->draw(image, cursorState, cursorX, cursorY);
          // READ45E2B5/45E342: cue is below the later cursor and pause caption.
          if (!menuOpen()) {
            if (runtimeSim.examining()) shell::drawExamineCaption(image, *runtimeFont, shell::fontCells());
            else if (runtimeSim.paused()) shell::drawCaption(image, *runtimeFont, shell::fontCells());
          }
          if (levelObserver) {
            const auto boxes = scene->injectedWalkerScreenBoxes(displayedCamera, options, *library, windowRaster);
            actors.clear();
            std::size_t walker = 0;
            for (std::size_t i = 0; i < world->lemmings().size(); ++i) {
              if (!world->lemmings()[i].alive()) continue;
              if (walker < boxes.size() && boxes[walker]) {
                const auto& drawn = options.injectedWalkers[walker];
                actors.push_back({i, *boxes[walker], drawn.clip, drawn.walkTemporalIndex,
                                 {drawn.plus8X, drawn.plus8Y, drawn.plus8Z}});
              }
              ++walker;
            }
          }
          view.actors = actors;
          view.hoveredActor = hover;
          view.cursorState = cursorState;
          view.hoveredHatch = hatchHover ? std::optional<std::size_t>{hatches[*hatchHover].index} : std::nullopt;
          view.hatches = hatches;
        }
      };
      if (!scenerySpritesReady && backdrop) {
        scenerySpritesReady = true;
        const auto visit = [&](const auto& self, const SceneNode& node) -> void {
          if (const auto* state = backdrop->spriteState(node.sourceOffset); state && !state->firstDrawDone)
            scenerySpritesReady = false;
          for (const auto& child : node.children) self(self, child);
        };
        backdrop->visitRoots([&](const auto&, const auto& root) { visit(visit, root); });
      }
      const bool finalShot = !args.windowShot.empty() && rendered + 1 == args.frames;
      const bool skipUnconsumed = app::ScopedFrameConsumption::skipUnconsumed() &&
          (args.frames > 0 || runOptions.frameSeconds > 0) && pendingView &&
          (!backdrop || scenerySpritesReady) && !finalShot &&
          args.c3WriteMask.empty() && args.c3WriteEvents.empty();
      if (!skipUnconsumed) materialize();
      if (levelObserver && pendingView) {
        app::profile::Scope profileObserver(app::profile::Other);
        app::detail::PendingFrameInspection inspection(materialize);
        levelObserver(*pendingView);
      }
      if (windowSceneCurrent) {
        const void* pixels = windowRaster.framebuffer().rgb.data();
        if (pendingView) {
          if (finalShot) finalWindowFrame = std::move(image);
          const auto& composed = finalShot ? *finalWindowFrame : image;
          pixels = composed.rgba.data();
          if (!uploadRgbx) {
            rgbFallback.resize(composed.pixelCount() * 3);
            for (std::size_t i = 0; i < composed.pixelCount(); ++i)
              std::copy_n(composed.rgba.data()+i*4, 3, rgbFallback.data()+i*3);
            pixels = rgbFallback.data();
          }
        }
        if (finalShot && !finalWindowFrame) finalWindowFrame = toImage(windowRaster.framebuffer());
        { app::profile::Scope scope(app::profile::Upload); SDL_UpdateTexture(texture, nullptr, pixels, 640 * (uploadRgbx ? 4 : 3)); }
        { app::profile::Scope scope(app::profile::Copy); SDL_RenderClear(presenter); SDL_RenderCopy(presenter, texture, nullptr, nullptr); }
        { app::profile::Scope scope(app::profile::Present); SDL_RenderPresent(presenter); }
        touchMenuPresented = touchMenuDrawn;
        lemmingMotion.frameDrawn();
      }
      ++rendered;
    }
#ifndef __EMSCRIPTEN__
    else { app::profile::Scope scope(app::profile::Idle); SDL_Delay(1); }
#endif
  }
  // End this level's hold after recording the last simulation budget.
  cancelFastForwardHold();
  recordCommands();
  SDL_SetRelativeMouseMode(SDL_FALSE);
#ifdef __EMSCRIPTEN__
  SDL_EventState(SDL_MOUSEWHEEL, SDL_IGNORE);
#endif
  if (runtimeCursor && previousCursorVisibility >= 0) SDL_ShowCursor(previousCursorVisibility);
  if (recorder) {
    const char* reason = disposition == LegDisposition::Completed ? "completed" :
        disposition == LegDisposition::RetryLevel ? "retry" :
        disposition == LegDisposition::ReturnToMap ? "map" : "closed/frame-limit";
    if (auto written = recorder->finish(*world, runtimeSim, reason); !written)
      std::fprintf(stderr, "record: %s; replay remains incomplete\n", written.error().c_str());
    else if (logLevelDetails) std::fprintf(stderr, "record: saved %s\n", written->string().c_str());
  }
  if (!args.windowShot.empty() && disposition == LegDisposition::Exit) {
    if (!finalWindowFrame) {
      std::fprintf(stderr, "window screenshot: exited before the requested final frame\n");
      return 1;
    }
    if (auto written = writePng(args.windowShot, *finalWindowFrame); !written) {
      std::fprintf(stderr, "window screenshot: %s\n", written.error().c_str());
      return 1;
    }
  }
  (void)camera;
  return 0;
#else
  std::fprintf(stderr, "SDL2 unavailable: window mode is not built (use --shot for headless rendering)\n");
  return 1;
#endif
}

int webo::app::runApplication(int argc, char** argv, FrontendWindowObserver observer,
                            LevelFrameObserver levelObserver, ApplicationRunOptions options) {
  auto args = parseArgs(argc, argv);
  if (args.invalidWindowScale) { std::fprintf(stderr, "--window-scale requires 1, 2 or 3\n"); return 1; }
  if (args.invalidReplayOption || (!args.recordDir.empty() && !args.replayFile.empty())) {
    std::fprintf(stderr, "--record DIR and --replay FILE require paths and cannot be combined\n"); return 1;
  }
  if (args.help) { printHelp(); return 0; }
  if (args.version) { std::puts("Lemmings Webolution " WEBOLUTION_VERSION_STRING); return 0; }
  const auto location = app::locateGameData(args.gameDir);
  args.gameDir = location.directory;
  const bool connected = args.frontend && args.shot.empty() && !args.map;
  const bool interactive = connected && !args.framesSpecified && !observer && !levelObserver;
  if (!args.replayFile.empty()) {
    auto library = AssetLibrary::open(args.gameDir / "lemmings.box", args.gameDir / "patch.box");
    if (!library) { std::fprintf(stderr, "replay assets: %s\n", library.error().c_str()); return 1; }
    return app::replayLevel(*library, args.replayFile);
  }
  if (!args.recordDir.empty() &&
      ((!args.frontend && !args.sim) || args.map || !args.shot.empty() || args.simTicks < 0 || args.simTicks > 1000000)) {
    std::fprintf(stderr, "--record requires frontend play or --level ID --sim, without --map/--shot\n"); return 1;
  }
#if !defined(__EMSCRIPTEN__) && !defined(WEBOLUTION_APP_NO_MAIN)
  if (!args.map && args.shot.empty()) {
    std::puts("Lemmings Webolution (native)");
    std::fflush(stdout);
  }
#endif
  std::optional<AssetLibrary> library;
  if (connected) {
    auto opened = AssetLibrary::openRequired(args.gameDir / "lemmings.box", args.gameDir / "patch.box");
    if (!opened) {
      auto message = opened.error();
      if (!location.executableSearch.empty())
        message += "\nAlso searched beside the Webolution executable in " + location.executableSearch;
      if (!location.discError.empty()) message += "\n" + location.discError;
      app::reportGameDataError(std::move(message), interactive);
      return 1;
    }
#ifndef __EMSCRIPTEN__
    if (interactive && !location.fromDiscCache) {
      if (auto hashes = checkGameBoxHashes(*opened); !hashes) {
        app::reportGameDataError(hashes.error(), interactive);
        return 1;
      }
    }
#endif
    library.emplace(std::move(*opened));
  }
  if (!location.discError.empty())
    std::fprintf(stderr, "CD image: %s\n", location.discError.c_str());
  // Cache the clone executable's recording identity before gameplay clocks start.
  if (!args.recordDir.empty()) (void)app::replayBuildIdentity();
  sceneryRandom().begin();  // main-loop entry seed (0x0045D06C analogue)
  std::optional<app::LevelOutcome> completed;
  LegDisposition disposition = LegDisposition::Exit;
  // One cue adapter, hence one Mixer and one device, for the whole process:
  // it outlives every frontend and gameplay leg below.
  audio::CuePlayback* cues = nullptr;
  save::SoundSettings sound;
  std::shared_ptr<save::SaveSink> soundFile;
  if (options.useDefaultSaveStore && !options.saveStore && args.shot.empty()) {
    if (auto base = save::localAppDataDirectory()) {
      const auto path = save::soundSettingsPathUnder(*base);
      soundFile = std::make_shared<save::FileSaveSink>(path, args.gameDir);
      if (auto loaded = save::SoundSettings::load(*soundFile); loaded)
        sound = *loaded;
      else {
#ifdef __EMSCRIPTEN__
        std::fprintf(stderr, "sound settings: %s\n", loaded.error().c_str());
        return 1;
#else
        std::fprintf(stderr, "sound settings: %s (using defaults)\n", loaded.error().c_str());
#endif
      }
    } else {
      std::fprintf(stderr, "sound settings: %s (memory only)\n", base.error().c_str());
    }
  }
#if WEBOLUTION_HAVE_SDL2
  audio::CuePlayback playback;
  playback.setVolumes(sound.music / 100.0f, sound.sfx / 100.0f);
  if (args.noAudio) playback.disable();
  if (options.cueObserver) playback.setObserver(options.cueObserver);
  cues = &playback;
  // Clone presentation. Direct-level startup is diagnostic even without a frame limit.
  const bool useAutomationScale = !args.frontend || args.framesSpecified || !args.shot.empty() ||
                                  args.map || observer || levelObserver;
#ifdef WEBOLUTION_WINDOWS_ICON
  constexpr auto setIcon = setWindowsWindowIcon;
#else
  constexpr auto setIcon = nullptr;
#endif
  SdlWindow presentation(sound.windowScale, windowScaleOverride(args.windowScale, useAutomationScale),
                         setIcon);
  auto* sharedWindow = &presentation;
#else
  SdlWindow* sharedWindow = nullptr;
#endif
  if (!connected)
    return runApplicationLeg(std::move(args), std::move(observer), std::move(levelObserver), options,
                             nullptr, nullptr, sharedWindow, cues, completed, disposition);
  // Use the injected store or the clone's per-user SavedGame.dat.
  auto store = options.saveStore;
  if (!store && options.useDefaultSaveStore) {
    if (auto made = save::makeDefaultSaveStore(args.gameDir); made) {
      store = *made;
    } else {
      std::fprintf(stderr, "save: %s; progress will not be saved\n", made.error().c_str());
    }
  }
  auto session = FrontendSession::create(*library, store);
  if (!session) { std::fprintf(stderr, "frontend: %s\n", session.error().c_str()); return 1; }
  std::function<void(save::SoundSettings, bool)> preview;
#if WEBOLUTION_HAVE_SDL2
  preview = [&playback](save::SoundSettings levels, bool previewSfx) {
    playback.setVolumes(levels.music / 100.0f, levels.sfx / 100.0f);
    if (previewSfx) playback.playUiClick(); // 48EB0B: Changeop
  };
  // Device failure, a missing WAV or no audio backend only print a diagnostic.
  playback.start(*library, options.enableAudio && !args.noAudio);
#endif
  session->configureSound(sound, std::move(soundFile), std::move(preview));
  // Keep the same frontend session across level, results, map and retry.
  for (;;) {
    completed.reset();
    disposition = LegDisposition::Exit;
    const int status = runApplicationLeg(args, observer, levelObserver, options, &*session,
                                         &*library, sharedWindow, cues, completed, disposition);
    if (status != 0 || disposition == LegDisposition::Exit) return status;
    if (disposition == LegDisposition::RetryLevel) {
      if (!session->retryLevel()) return 1;
      continue;
    }
    if (disposition == LegDisposition::ReturnToMap) {
      if (!session->abandonLevelToMap()) return 1;
      continue;
    }
    if (!completed) return 1; // Completed is the only disposition allowed to publish an outcome.
    if (auto shown = session->completeLevel(*library, *completed); !shown) {
      std::fprintf(stderr, "results: %s\n", shown.error().c_str()); return 1;
    }
  }
}

#ifndef WEBOLUTION_APP_NO_MAIN
int main(int argc, char** argv) {
  webo::app::ApplicationRunOptions options;
  options.useDefaultSaveStore = true;  // ordinary startup persists to the clone's own save
  options.enableAudio = true;          // ordinary startup owns one mixer (--no-audio opts out)
  return webo::app::runApplication(argc, argv, {}, {}, options);
}
#endif
