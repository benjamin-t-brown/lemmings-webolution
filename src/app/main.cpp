#include "app/FrameProfile.h"
#include "app/RasterRows.h"
#include "app/FrameConsumption.h"
#include "app/Replay.hpp"
// Lemmings Webolution -- 3D-first entry point.
//
// Ordinary startup enters the connected frontend. Explicit diagnostics load
// one retail level, build the tube from stored .LVL and archive data, and draw
// it through the verified A1 camera. The flat unwrapped view is a debug flag.
//
//   --frontend       explicitly enter the connected frontend with other options
//   --direct-level   request the previous bare-launch injected-walker diagnostic
//   --window-scale <1|2|3> integer presentation only; ordinary interactive default 2
//   --level <id>     level to load (default MM_CLIMB_002)
//   --native         render at the original's 640x480
//   --camera <json>  camera state params (the A1 fixture/params format)
//   --shot <png>     render one frame headless and write it
//   --window-shot <png> with --frames N, save the final presented CPU frame;
//                     SDL_VIDEODRIVER=dummy exercises the loop without a desktop window.
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
//   --no-audio       do not open an audio device or play music/sound cues
//   --flat           debug: unwrap the skin to 2D instead of the 3D view
//   --sim            tick a sim::World (D3a's loadRetailLevel) instead of the
//                     fixed injected-walker fixtures, and draw its live
//                     lemmings through the same drawInjectedWalkers path
//                     (TubeScene::injectedWalkersFromWorld). Direct-level
//                     diagnostics retain fixtures unless simulation is requested.
//   --sim-ticks <N>  diagnostic setup: call World::tick() exactly
//                     N times before rendering, independent of frame count.
//                     Implies --sim. Prints the tick, lemming count and every
//                     live lemming's cell to stderr. Also seeds a direct-level
//                     window; natural --frontend entry explicitly resets it.
//   --hud-ticks <N>  headless only (with --shot): after the frame, tick a separate
//                     sim::World N times and overlay the Out / Saved counters and the
//                     timer (shell/HudText.h, C2g). Does not change the scene or the
//                     walkers. Off by default.
//   --hud-out N       with --hud-ticks: replace the sim's Out count (diagnostic only; used to
//                     show the layout against a reference that reads a different number).
//   --hud-colour R,G,B  with --hud-ticks: the Out / timer glyph colour (UNREAD default
//                     255,255,255; exists only for the sensitivity run).
//   --pause-caption   headless only (with --shot): overlay the paused-frame caption panel
//   --cursor-fixture  headless only: center NoLemming software cursor (runtime wiring test)
//                     (shell/PauseCaption.h, C2i panel, C2j text via the ^Font path).
//   --caption-no-text  with --pause-caption: the C2i panel only (the "before" render).
//   --caption-alpha N  with --pause-caption: panel alpha (HYPOTHESIS 128 of 255; sensitivity only)
//   --caption-fill R,G,B  with --pause-caption: panel colour (HYPOTHESIS 0,0,0; sensitivity only)
//   SOURCE input adapter, windowed --sim only (not native-validated):
//                     click the first eight skill plates or use keys 1-8 for
//                     Climber, Floater, Bomber, Blocker,
//                     Builder, Basher, Miner, Digger in that enum order, and
//                     a left click outside the plates assigns the skill to the lemming
//                     under a 32x32 cursor box centered on the click. Hover and
//                     click share the same bounded eligibility/ranking path.
//                     Counts and selected plate update; icon poses remain the C1
//                     fixture. Runtime command controls use the connected panel path.
//
// No live game and no captured arena data are used. Everything comes from the
// install named by WEBOLUTION_GAME_DIR (default Lemmings Revolution/ from repo root).

#include "app/Camera.h"
#include "app/CameraIntro.h"
#include "app/EntryStartCue.h"
#include "app/LevelStartSequence.h"
#include "app/LemmingAnim.h"
#include "app/LemmingMotion.h"
#include "app/LevelMenu.h"
#include "assets/GameData.h"
#include "app/AppEntry.h"
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
#endif
#ifdef __EMSCRIPTEN__
#include "web/Browser.h"
#endif

namespace fs = std::filesystem;
using namespace webo;

namespace {

constexpr const char* kDefaultGameDir = "Lemmings Revolution";

// scene-branch-impl-001: the process-wide CRT generator that draws first-frame choices for
// branch-1 sprite scenery. The original seeds it from the unpaused millisecond timer T0 at
// main-loop entry (0x0045D06C) and again at play start (0x004451E5), both through
// srand(ftol(ms * 32767.0)) (0x00447580); the clone's analogue is this monotonic clock,
// measured from application start with sim-paused time excluded. Never a constant seed.
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
  std::string gameDir;
  int frames = 0;
  bool framesSpecified = false;
  int windowScale = 0;  // omitted: ordinary interactive 2x, automation 1x
  bool invalidWindowScale = false;
  bool help = false;
  bool map = false;
  bool frontend = false;
  bool mapFlagsSpecified = false;
  std::string mapFlags;
  bool noInjectedWalkers = false;
  bool noTerrainSync = false;  // diagnostic: draw the terrain as loaded (the view before terrain-render-001)
  bool noActorAnim = false;    // diagnostic: every lemming draws walk entry 0 (the view before actor-anim-001)
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
  // Portable CLI policy, not a recovered original startup sequence. Only a
  // bare invocation, game-directory, --window-scale or --no-audio acquire the frontend
  // default. Other tokens (including ignored unknown options) keep diagnostic
  // routing; an explicit --frontend remains authoritative regardless of order.
  bool defaultFrontend = true;
  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg != "--game-dir" && arg != "--no-audio" && arg != "--window-scale" && arg != "--record") defaultFrontend = false;
    const auto next = [&]() -> std::string { return i + 1 < argc ? argv[++i] : std::string{}; };
    if (arg == "--help" || arg == "-h") a.help = true;
    if (arg == "--map") a.map = true;
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
  if (a.gameDir.empty()) {
    const char* env = std::getenv("WEBOLUTION_GAME_DIR");
    a.gameDir = env ? env : kDefaultGameDir;
  }
  return a;
}

int windowScale(const Args& args, bool observing) {
  // PROVISIONAL clone UI policy for V9. Diagnostic captures, bounded runs and
  // observer-driven automation retain their existing presentation unless the
  // caller explicitly chooses a scale. The raster and CPU outputs never scale.
  if (args.windowScale) return args.windowScale;
  return args.framesSpecified || !args.shot.empty() || args.map || observing ? 1 : 2;
}

void printHelp() {
  std::puts("webolution [--game-dir PATH] [--frontend | --direct-level | --level ID | --map] [options]\n"
            "No options, or only --game-dir/--no-audio/--window-scale, starts the connected frontend.\n"
            "Any other option keeps its diagnostic route unless --frontend is explicit.\n"
            "--direct-level opens the previous MM_CLIMB_002 injected-walker diagnostic.\n"
            "--window-scale 1|2|3 chooses crisp integer window scaling (ordinary play: 2).\n"
            "--record DIR records each level attempt as DIR/*.replay (default via Play-Clone.ps1).\n"
            "--replay FILE checks a recording headless at full speed, without opening a save.\n"
            "Bounded runs, map diagnostics and observers default to 1; CPU screenshots keep their resolution.\n"
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
            "Music loops in the frontend/map and levels, continues through pause, and restarts on Retry.\n"
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

// Shared SOURCE hover/click adapter after skill-panel routing. The cursor is a 32x32
// box centered on the window click (sprite size named in Pick.h at
// 0x0045578C); pickBoxOverlaps applies the +/-4 adjustment itself. Presentation
// maps the pointer to the raster's 640x480 placement coordinates.
// PROVISIONAL playable-v1 policy: rank by
// displayed body-box centre distance to the cursor dot, retaining the existing
// visibility, stock, support and skill exclusions. Exact ties keep World order.
// Full native visibility, facing and applicability are still incomplete.
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
      candidates.push_back({
          reinterpret_cast<void*>(static_cast<std::uintptr_t>(i)),
          pickBoxFromPlacement(placement),
          app::passesKnownPickExclusions(lemmings[i], skill) && support.value_or(true),
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

static int runApplicationLeg(int argc, char** argv, app::FrontendWindowObserver observer,
                            app::LevelFrameObserver levelObserver, app::ApplicationRunOptions runOptions,
                            app::FrontendSession* persistentFrontend,
                            [[maybe_unused]] app::SdlWindow* sharedWindow,
                            [[maybe_unused]] audio::CuePlayback* cues,
                            std::optional<app::LevelOutcome>& completed, LegDisposition& disposition) {
  Args args = parseArgs(argc, argv);
  if (args.help) { printHelp(); return 0; }
  if (args.invalidWindowScale) { std::fprintf(stderr, "--window-scale requires 1, 2 or 3\n"); return 1; }
  const bool observing = bool(observer) || bool(levelObserver);
  const int scale = windowScale(args, observing);
#ifdef __EMSCRIPTEN__
  const bool logLevelDetails = true;
#else
  const char* diagnosticSetting = std::getenv("WEBOLUTION_NATIVE_DIAGNOSTICS");
  const bool verbose = diagnosticSetting && std::string_view(diagnosticSetting) == "1";
  // Keep diagnostics for observer-driven tools, including the sweep's child-log tail.
  const bool logLevelDetails = verbose || observing;
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

  auto library = AssetLibrary::open(fs::path(args.gameDir) / "lemmings.box",
                                    fs::path(args.gameDir) / "patch.box");
  if (!library) {
    std::fprintf(stderr, "assets: %s\n", library.error().c_str());
    return 1;
  }

  TextureCacheScope textureCache(*library);

  // parseArgs resolves ordinary/explicit frontend entry for both app layers.
  // Selection feeds the same retail loader/sim below, never
  // a second executable, a captured world, or the original installation's save.
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
    // Retry retains selectedLevel and skips the frontend window. Keep its
    // level bed until the new level starts; do not briefly play menu music.
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

  // B3j: explicit source-fixture map display. This early branch deliberately
  // bypasses level/sim/3D setup; all old commands retain their prior path.
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
    app::SdlWindow presentation(scale);
    if (auto ready = presentation.ensure("Lemmings Webolution — conditional map base layer (SOURCE)"); !ready) {
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

  // The same resolution sim::loadRetailLevel uses below, so the application and
  // the simulation can never pick different archive levels for one id. The map
  // names some levels in mixed case ("MM_Block_Bomb_A"); when the id only matches
  // case-insensitively, continue with the archive's own spelling so every later
  // use of args.level (skill panel, outcome, observers) sees the canonical id.
  // Ids that already matched keep the caller's text, as before.
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
      sceneryRandom().seed();
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
  app::LemmingMotionTracker lemmingMotion;
  std::optional<sim::World> world;
  int terrainHeight = 0;
  if (args.sim) {
    auto setup = sim::loadRetailLevel(*library, args.level);
    if (!setup) {
      std::fprintf(stderr, "sim: %s\n", setup.error().c_str());
      return 1;
    }
    // R1/R2: connected play arms only after the entry presentation below.
    // Its full door clip replaces the old derived door-arm delay; retain the
    // 1.3 s first countdown and every recurring interval. Diagnostic World
    // fixtures keep their existing timing contract.
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
    // fluid-surface-impl-001: feed the live receiver state of the accepted shared-dt world to the
    // scene (stream order == sim::LevelSetup::gates order). Without --sim there is no world and
    // the fluid pass draws nothing. Depth policy: BackdropScene::kProvisionalFluidDepthPolicy.
    if (backdrop && world) {
      std::vector<app::BackdropScene::FluidGateState> fluid;
      for (const auto& g : world->setup().gates) {
        // All installed receivers start [+0x150] at zero and receive every
        // shared-dt update. The existing level clock therefore samples the
        // kind-2 four-second alpha phase without another simulation clock.
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
    // terrain-render-001: skill edits to sim::Terrain reach the drawn tube here; a no-op unless
    // the terrain version moved since the last frame.
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
  // levers, pads and flipper bases are LegacyUnattributed. Flipper coils,
  // fluids and the secondary tree draw in uncached passes after the tube.
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
  auto renderFrame = [&](const app::CameraParams& frameParams, app::Raster& target, bool scenePrepared = false) {
    app::profile::Scope profilePrepare(app::profile::Preparation);
    const auto frameCamera = app::buildCamera(frameParams);
    app::BackdropStats backdropStats;
    app::SceneRasterObserver observer;
    std::vector<std::uint8_t> c3Mask;
    std::ofstream c3Out;
    const bool c3Diagnostic=!args.c3WriteMask.empty()||!args.c3WriteEvents.empty();
    const bool primaryHit = !c3Diagnostic && primaryCache && sameCamera(*primaryPose, frameCamera);
    // While rotating, rasterize straight into the live buffers. Populate the
    // exact-pose caches when a pose repeats. Existing exact hits remain valid.
    const bool cachePose = !previousCamera || sameCamera(*previousCamera, frameCamera);
    previousCamera = frameCamera;
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
         [&] { if (backdrop) backdrop->renderStage(target, frameCamera, *library, backdropStats, app::BackdropScene::Stage::LegacyUnattributed); },
         [&] {
           // Tube materials are opaque and triangle() keeps strictly nearer Z.
           // Cache the independently rendered nearest tube fragment, then merge
           // against the animated legacy stage at exactly the old tube slot.
           if (!geometryOptions.depthTest || geometryOptions.flat || geometryOptions.composeActorsBeforeRoute || c3Diagnostic) {
             scene->render(target, frameCamera, geometryOptions, &*library);
             return;
           }
           const auto revision = scene->terrainBlocksRebuilt() + scene->counterGateRevision();
           const auto renderTube = [&](app::Raster& destination) {
             app::detail::RasterCommandBatch batch(frameCamera.height);
             scene->render(destination, frameCamera, geometryOptions, &*library);
             app::profile::Scope rasterTime(app::profile::TubeRaster);
             batch.flush();
           };
           if (!tubeCache || tubeTerrainRevision != revision || !sameCamera(*tubePose, frameCamera)) {
             if (!cachePose) {
               renderTube(target);
               return;
             }
             if (!tubeCache || tubeCache->framebuffer().width != frameCamera.width ||
                 tubeCache->framebuffer().height != frameCamera.height)
               tubeCache.emplace(frameCamera.width, frameCamera.height);
             tubeCache->clear(0, 0, 0);
             tubeCache->resetDrawCounts();
             renderTube(*tubeCache);
             tubePose = frameCamera; tubeTerrainRevision = revision;
           }
           target.compositeOpaque(*tubeCache);
         },
         [&] {
           // The flipper coils add light to the tube colour. The original draws
           // them inside its tube walker. Composite them over the completed
           // opaque tube here, using its depth to keep foreground ledges solid.
           if (backdrop) backdrop->renderStage(target, frameCamera, *library, backdropStats,
               app::BackdropScene::Stage::GravityEffect);
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
      // Provisional Playtest 13 C5 sky. Fill uncovered pixels after the water
      // blends against the retail black clear.
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
      // U2 PROVISIONAL: additive drowning is visible through its water, while
      // all opaque scenery (including the later secondary pass) still hides it.
      // This does not change the fluid depth policy or ordinary actor depth.
      const auto& completed = target.depthBuffer();
      for (std::size_t p = 0; p < fluidDrownSceneDepth.size(); ++p)
        if (completed[p] != afterFluidDepth[p]) fluidDrownSceneDepth[p] = completed[p];
      options.fluidDrownSceneDepth = fluidDrownSceneDepth;
    }
    // R3: actors test against completed scenery, fluids and edited terrain.
    scene->drawExplosionDecals(target, frameCamera);
    if (options.drawCellRoute && !options.composeActorsBeforeRoute)
      scene->drawInjectedWalkers(target, frameCamera, options, *library);
    scene->drawDeathParticles(target, frameCamera, *library, options.pipeSceneDepth);
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
      std::fprintf(stderr, "backdrop: sprite scenery generator state at first draw = 0x%08x "
                   "(seeded from the application clock; flame phases are not reproducible)\n",
                   *backdrop->firstSpriteDrawRandomState());
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
  // Frontend runs already started the adapter in runApplication (idempotent);
  // direct-level runs start it here, still once per process.
  if (cues) {
    cues->start(*library, runOptions.enableAudio && !args.noAudio);
    cues->playLevelMusic();
  }
  app::SdlWindow localWindow(scale);
  auto& presentation = sharedWindow ? *sharedWindow : localWindow;
  if (auto ready = presentation.ensure(
      "Lemmings Webolution — right mouse + NUMPAD4/6 revolve; drag mouse; SPACE/CTRL zoom; BACKSPACE flip"); !ready) {
    std::fprintf(stderr, "SDL: %s\n", ready.error().c_str()); return 1;
  }
  SDL_Renderer* presenter = presentation.renderer();
  // The composed HUD image already has RGB in its first three bytes.
  // RGBX ignores its fourth byte exactly as the former RGB24 upload did.
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
  bool running = true, leftDown = false, rightDown = false, focused = true;
  // The logical SDL pointer drives hover, clicks and zoom. Right-drag freezes it.
  int pointerX = 320, pointerY = 240;
  app::PendingCameraEvents pending;
  int rendered = 0;
  const Uint64 frequency = SDL_GetPerformanceFrequency();
  Uint64 previous = SDL_GetPerformanceCounter();
  app::FixedCameraClock fixedClock;
  // D3b: the sim ticks at its own measured 200 ms period (SimClock.h),
  // independent of this window's 144 Hz camera clock and its render rate --
  // only active when --sim was given. Natural frontend entry resets the World
  // to tick0; a direct-level diagnostic may seed it with --sim-ticks.
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
      entryCue.emplace(persistentFrontend->levelIntroEnabled());
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
  // SOURCE input adapter: retained C1 geometry with live counts/selection.
  app::RuntimeSkillSelection skillSelection;
  app::RuntimeNukeInput nukeInput;
  std::optional<app::ReplayRecorder> recorder;
  if (!args.recordDir.empty() && world && releaseControl) {
    auto made = app::ReplayRecorder::create(args.recordDir, args.level, persistentFrontend != nullptr,
                                           args.simTicks, releaseControl->current(), logLevelDetails);
    if (made) recorder.emplace(std::move(*made));
    else std::fprintf(stderr, "record: %s; playing without recording\n", made.error().c_str());
  }
  const auto recordCommands = [&] {
    if (recorder) recorder->commands(*world, runtimeSim, skillSelection,
                                    releaseControl->current(), releaseControl->rateHatch());
  };
  std::optional<Image> finalWindowFrame;
  auto displayedCamera = app::buildCamera(cameraIntro.pose(controls.camera()));
  auto displayedPickReference = controls.pickReference();
  auto displayedScenePose = cameraIntro.pose(controls.camera());
  // Preserve the original initial input contract. Only an actually discarded
  // loop frame invalidates depth and requires a later click to refresh it.
  bool windowSceneCurrent = true;
  bool pointerInWindow = true;
  // First-draw sprite latches consume RNG in dispatch order. Until every sprite
  // has latched, retain every full draw (including permanently hidden nodes).
  // Skipping must not defer a latch or change the wall-clock seeding policy.
  bool scenerySpritesReady = false;
  const int previousCursorVisibility = SDL_ShowCursor(SDL_QUERY);
  if (runtimeCursor) SDL_ShowCursor(SDL_DISABLE);
  const auto menuOpen = [&] { return levelMenu && levelMenu->isOpen(); };
  const auto cancelFastForwardHold = [&] {
    leftDown = false;
    runtimeSim.setFastForward(false);
  };
  const auto updateFastForwardHold = [&] {
    // PROVISIONAL Playtest 13 C3: plate 8 is a hold, including drag-onto.
    const bool active = leftDown &&
        app::runtimeSkillPanelCommand(pointerX, pointerY) == app::RuntimePanelCommand::FastForward;
    if (active && !runtimeSim.fastForward()) uiClick(); // Original plate 8 has no play call.
    runtimeSim.setFastForward(active);
  };
  const auto zoomAtPointer = [&]() -> std::optional<app::CameraZoomPose> {
    if (controls.camera().zoomShift != 0 || rendered == 0 ||
        pointerX < 0 || pointerX >= displayedCamera.width ||
        pointerY < 0 || pointerY >= displayedCamera.height ||
        (runtimePanel && app::runtimeSkillPanelIndex(pointerX, pointerY) >= 0))
      return std::nullopt;
    if (!windowSceneCurrent) {
      displayedCamera = renderFrame(displayedScenePose, windowRaster, true);
      windowSceneCurrent = true;
    }
    const auto pixel = std::size_t(pointerY) * displayedCamera.width + pointerX;
    const double actorDepth = windowRaster.actorDepth().depthAt(pixel);
    const double depth = std::isfinite(actorDepth) ? actorDepth : windowRaster.depthBuffer()[pixel];
    return app::cursorZoomPose(displayedCamera, cameraIntro.pose(controls.camera()),
        pointerX + 0.5, pointerY + 0.5, depth);
  };
  // Clicks and held input use the same displayed hatch geometry and actor
  // precedence as hover. A hatch right-click is consumed before camera grab.
  const auto hatchAtPointer = [&](int x, int y) -> std::optional<std::size_t> {
    if (!world || !backdrop || !focused || rightDown || menuOpen() ||
        app::runtimeSkillPanelIndex(x, y) >= 0) return std::nullopt;
    if (!windowSceneCurrent) {
      displayedCamera = renderFrame(displayedScenePose, windowRaster, true);
      windowSceneCurrent = true;
    }
    if (hoveredActor(*world, *scene, options, *library, displayedCamera, windowRaster,
                     x, y, skillSelection.selectedSkill(), args.level == "MM_CLIMB_002"))
      return std::nullopt;
    const auto targets = backdrop->hatchHoverTargets(displayedCamera, windowRaster);
    const auto hit = app::hoveredHatch(targets, x, y);
    return hit ? std::optional<std::size_t>{targets[*hit].index} : std::nullopt;
  };
  const auto resetMenuInput = [&] {
    nukeInput.reset();
    if (releaseControl) releaseControl->reset();
    cancelFastForwardHold();
    rightDown = false; pending.clear(); fixedClock = app::FixedCameraClock{};
    SDL_SetRelativeMouseMode(SDL_FALSE);
    previous = SDL_GetPerformanceCounter();
  };
  const auto openMenu = [&] {
    if (persistentFrontend &&
        persistentFrontend->openLevelMenu() != shell::Outcome::Handled) return;
    levelMenu->open();
    if (!runtimeSim.paused()) runtimeSim.togglePause();
    uiClick(); // PROVISIONAL pause feedback; simulation/menu policy is unchanged.
    resetMenuInput();
  };
  bool menuDismissedThisFrame = false;
  const auto activateMenu = [&](app::LevelMenuAction action) {
    switch (action) {
      case app::LevelMenuAction::Resume:
        levelMenu->close();
        if (persistentFrontend)
          (void)persistentFrontend->closeLevelMenu(false);
        runtimeSim.resume(); resetMenuInput();
        uiClick();
        menuDismissedThisFrame = true; break;
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
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      presentation.gameCoordinates(event);
      // A hold can span Escape or a page change, including its consumed event batch.
      if (persistentFrontend) app::updateFrontendCursorInput(*persistentFrontend, event);
      if (event.type == SDL_QUIT) { disposition = LegDisposition::Exit; running = false; break; }
      else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
        nukeInput.reset();
        if (releaseControl) releaseControl->reset();
        cancelFastForwardHold();
        focused = false; rightDown = false; pending.clear();
        SDL_SetRelativeMouseMode(SDL_FALSE);
      } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
        focused = true;
      } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_LEAVE) {
        pointerInWindow = false;
        runtimeSim.setFastForward(false);
      } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_ENTER) {
        pointerInWindow = true;
      } else if (menuDismissedThisFrame) {
        // SOURCE event-batch ownership: a dismissal must not let already-queued
        // menu clicks/keys fall through into gameplay. Still service close/focus.
        continue;
      } else if (menuOpen()) {
        // Own ALL input while the menu is open, including misses, unsupported
        // rows and gameplay keys. No skill assignment, zoom or relative grab.
        if (!focused) continue;
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
          app::handleSoundOptionsInput(*persistentFrontend, event);
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
        if (!running) break;
      } else if (focused && !rightDown && releaseControl &&
                 event.type == SDL_MOUSEBUTTONDOWN &&
                 (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT) &&
                 releaseControl->press(hatchAtPointer(event.button.x, event.button.y), event.button.button, *world)) {
        pointerX = event.button.x; pointerY = event.button.y;
        if (event.button.button == SDL_BUTTON_LEFT) leftDown = true;
        updateFastForwardHold();
        nukeInput.reset();
        uiClick(); // Initial hatch press only; held changes are coalesced below.
      } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
        nukeInput.reset();
        if (releaseControl) releaseControl->reset();
        cancelFastForwardHold();
        if (focused) { rightDown = true; SDL_SetRelativeMouseMode(SDL_TRUE); }
      } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT) {
        if (releaseControl) releaseControl->release(event.button.button);
        rightDown = false; SDL_SetRelativeMouseMode(SDL_FALSE);
        // SDL moves its own cursor through the drag and restores it there. Put
        // it back on the frozen pointer so motion continues from that spot.
        const int scale = presentation.scale();
        SDL_WarpMouseInWindow(SDL_RenderGetWindow(presenter),
                              pointerX * scale + scale / 2, pointerY * scale + scale / 2);
      } else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        if (releaseControl) releaseControl->release(event.button.button);
        cancelFastForwardHold();
        if (args.sim && world && focused && !rightDown) {
          pointerX = event.button.x; pointerY = event.button.y;
          nukeInput.release(pointerX, pointerY, event.button.timestamp, *world);
        } else nukeInput.reset();
      } else if (args.sim && world && event.type == SDL_MOUSEBUTTONDOWN &&
                 event.button.button == SDL_BUTTON_LEFT) {
        // Consume panel clicks before actor picking, including command plates.
        if (focused && !rightDown) {
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
            // A click consumes the previous frame's exact scene depth even if
            // its observer did not inspect pixels. No simulation advances here.
            if (!windowSceneCurrent) {
              displayedCamera = renderFrame(displayedScenePose, windowRaster, true);
              windowSceneCurrent = true;
            }
            const auto hit = hoveredActor(*world, *scene, options, *library,
                displayedCamera, windowRaster,
                pointerX, pointerY, skillSelection.selectedSkill(),
                args.level == "MM_CLIMB_002");
            // Record the resolved simulation target, never the picking pixels.
            const auto before = recorder && hit ? std::optional{world->lemmings()[*hit]} : std::nullopt;
            const bool assigned = hit && world->assign(*hit, skillSelection.selectedSkill());
            if (before) recorder->assignment(*hit, skillSelection.selectedSkill(), assigned, *before);
            drainCues();
            // READ 0x455756: only a successful assignment resumes native UI1.
            // Failed picks/assignments and panel selections leave pause intact.
            if (assigned && runtimeSim.paused()) {
              runtimeSim.resume();
              previous = SDL_GetPerformanceCounter();
            }
          }
        }
      } else if (event.type == SDL_MOUSEMOTION) {
        if (focused) {
          pending.mouse(double(event.motion.xrel) / presentation.scale(),
                        double(event.motion.yrel) / presentation.scale());
          if (!rightDown) {
            pointerX = event.motion.x; pointerY = event.motion.y;
            if (releaseControl && releaseControl->holding())
              releaseControl->advance(0.0, hatchAtPointer(pointerX, pointerY), *world);
            leftDown = leftDown && (event.motion.state & SDL_BUTTON_LMASK);
            updateFastForwardHold();
          }
        }
      } else if (event.type == SDL_KEYDOWN && !event.key.repeat) {
        const auto key = event.key.keysym.scancode;
        if (focused && (key == SDL_SCANCODE_SPACE || key == SDL_SCANCODE_LCTRL || key == SDL_SCANCODE_RCTRL))
          pending.zoom(zoomAtPointer());
        if (focused && key == SDL_SCANCODE_BACKSPACE) pending.flip();
        if (key == SDL_SCANCODE_ESCAPE) {
          if (persistentFrontend && !focused) continue;
          if (args.sim && world && runtimeSim.paused()) {
            runtimeSim.resume();
            previous = SDL_GetPerformanceCounter();
            uiClick();
          } else if (levelMenu) openMenu();
          else running = false; // Preserve standalone diagnostic Escape-close.
        }
        if (levelMenu && focused && key == SDL_SCANCODE_O && runtimeSim.paused()) {
          openMenu();
        }
        if (args.sim && world && focused && key == SDL_SCANCODE_P) {
          runtimeSim.togglePause();
          previous = SDL_GetPerformanceCounter();
          uiClick();
        }
        if (args.sim && world && focused) {
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
    input.rightMouseDown = rightDown;
    const Uint64 now = SDL_GetPerformanceCounter();
    double elapsed = static_cast<double>(now - previous) / static_cast<double>(frequency);
    previous = now;
    if (args.frames > 0) elapsed = app::FixedCameraClock::stepSeconds;
    if (runOptions.frameSeconds > 0) elapsed = runOptions.frameSeconds;
    if (panelIconAnimation) {
      const int hover = focused && !menuOpen() && !rightDown ?
          app::runtimeSkillPanelIndex(pointerX, pointerY) : -1;
      // R7: UI animation continues while simulation is paused and is never
      // multiplied by fast-forward. Menu/focus suspension is provisional.
      panelIconAnimation->advanceIcons(focused && !menuOpen() ? elapsed : 0.0,
                                 skillSelection.selectedIndex(), hover);
    }
    if (releaseControl && releaseControl->holding() && focused && !menuOpen()) {
      const int previousRate = releaseControl->current();
      releaseControl->advance(elapsed, hatchAtPointer(pointerX, pointerY), *world);
      // One request per rendered frame, even if a long frame catches up several
      // repeats. SkipPlaying discards overlap; nothing accumulates while held.
      if (releaseControl->current() != previousRate) uiClick();
    }
    recordCommands();
    fixedClock.advance(elapsed);
    bool updated = false;
    while (fixedClock.takeStep()) {
      input = pending.consumeInto(input);
      controls.update(input, app::FixedCameraClock::stepSeconds);
      updated = true;
    }
    bool simTicked = false;
    // READ45CC09: title clock updates after input, before simulation/drawing.
    // Playtest 10 Z3 restores the load-time countdown independently of the voice.
    if (entryCue) entryCue->advanceBeforeFrame(elapsed, focused && !runtimeSim.paused());
    if (levelStart) levelStart->advance(elapsed, focused && !runtimeSim.paused());
    if (args.sim && world) {
      // shared-timing-impl-001: explicit elapsed-time world update (integer ms, 100 ms stall
      // cap, level-second splitting, lemmings then lever contact). The cap applies to the
      // production wall clock; --frames and frameSeconds are owned deterministic clocks whose
      // declared step is simulated as given. Camera/input cadence above is unchanged.
      const bool ownedClock = args.frames > 0 || runOptions.frameSeconds > 0;
      // P7: observe each confirmed update, including every fast-forward pass,
      // using the existing World clock. This never supplies time to simulation.
      lemmingMotion.observe(*world);
      const auto afterUpdate = [&] {
        drainCues(); lemmingMotion.observe(*world);
        scene->advanceDeathParticles(world->levelClockMs());
      };
      // The restored text clock is identical to the former release-only delay;
      // keep the accepted release predicate, including fast-forward passes.
      const bool releaseEnabled = !levelStart || levelStart->releaseEnabled(entryCue && entryCue->active());
      simTicked = runtimeSim.advanceElapsed(*world, elapsed, persistentFrontend != nullptr, afterUpdate,
                                            ownedClock ? 0 : app::RuntimeSimState::kStallCapMs, releaseEnabled);
      if (recorder && simTicked) recorder->advanced(*world, runtimeSim, releaseEnabled);
      // actor-anim-001: every lemming draws its own clip, advanced by level time every frame
      // (not only on a simulation tick) and frozen while paused. Fast-forward runs the world
      // kFastForwardPasses times per frame. Reuse its accepted integer-ms budget, including
      // its stall cap and owned-clock exception; this does not change simulation timing.
      // PROVISIONAL: a transition observed at frame end receives this frame's whole budget.
      if (const SpriteSheet* lemmingSheet = scene->lemmingSprites(); lemmingSheet && !args.noActorAnim) {
        const double levelDt = !simTicked ? 0.0 : runtimeSim.lastPassMs() / 1000.0 *
            (runtimeSim.fastForward() ? app::RuntimeSimState::kFastForwardPasses : 1);
        lemmingAnim.advance(*world, std::array<const SpriteSheet*, 3>{
            lemmingSheet, scene->lemmingSprites(1), scene->lemmingSprites(2)}, levelDt);
      }
      options.injectedWalkers = app::injectedWalkersFromWorld(
          *world, terrainHeight, args.noActorAnim ? nullptr : &lemmingAnim, &lemmingMotion);
      if (backdrop && entranceExitPresentation) {
        // R2: connected play samples the full installed door duration (25
        // keys/s); its phase relative to the cue is provisional. The direct
        // diagnostic retains the historical P9 arm-normalized sampling.
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
      // Native pose is evaluated at the old intro clock before its increment;
      // pause freezes the opening move independently of camera input updates.
      cameraIntro.advanceAfterFrame(elapsed, runtimeSim.paused());
      Image image;
      std::vector<app::LevelActorView> actors;
      std::vector<app::HatchHoverTarget> hatches;
      // This state-only view is never offered to an ordinary observer. The
      // explicit automation scope opts into inspectLevelFrame() for pixels,
      // hover, visible actor boxes and hatch depth; all other fields are ready.
      const auto cueFrame = entryCue ? entryCue->frame() : std::nullopt;
      std::optional<app::LevelFrameView> pendingView;
      if (args.sim && world && runtimeFont && runtimePanel)
        pendingView.emplace(app::LevelFrameView{rendered, *world, runtimeSim.paused(), skillSelection.selectedIndex(), actors, image,
                         std::nullopt, pointerX, pointerY, app::CursorState::Hidden,
                         runtimeSim.endTransition().phase(), runtimeSim.endTransition().timerSeconds(),
                         cameraIntro.active(), entryCue ? entryCue->seconds() : 0.0, cueFrame, menuOpen(), args.level,
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
          const int panelIndex = app::runtimeSkillPanelIndex(pointerX, pointerY);
          const bool canHover = focused && !menuOpen() && !rightDown && panelIndex < 0;
          const auto selectedSkill = skillSelection.selectedSkill();
          const auto hover = canHover ?
              hoveredActor(*world, *scene, options, *library, displayedCamera, windowRaster,
                  pointerX, pointerY, selectedSkill, args.level == "MM_CLIMB_002") : std::nullopt;
          // The picker can resolve a refused click. Only a skill recipient gets the body redraw.
          if (hover && pointerInWindow && pointerX >= 0 && pointerX < image.width &&
              pointerY >= 0 && pointerY < image.height && world->canAssign(*hover, selectedSkill)) {
            // Injected walkers contain the live actors in World order.
            const auto& lemmings = world->lemmings();
            const auto walker = std::count_if(lemmings.begin(), lemmings.begin() + *hover,
                                              [](const sim::Lemming& l) { return l.alive(); });
            if (scene->drawHoveredWalker(image, walker, displayedCamera, options, *library, windowRaster))
              view.markedActor = hover;
          }
          hatches = backdrop ? backdrop->hatchHoverTargets(displayedCamera, windowRaster) :
              std::vector<app::HatchHoverTarget>{};
          for (auto& h : hatches)
            h.remaining = world->remainingFromEntrance(h.index);
          // Actor selection keeps precedence, matching hatchAtPointer input.
          const auto hatchHover = canHover && !hover ? app::hoveredHatch(hatches, pointerX, pointerY) : std::nullopt;
          if (hatchHover) {
            const auto adjustingRate = releaseControl && releaseControl->holding()
                ? std::optional{releaseControl->current()} : std::nullopt;
            app::drawHatchHover(image, *runtimeFont, hatches[*hatchHover], adjustingRate);
          }
          app::drawRuntimeSkillPanel(image, *runtimePanel, *world, skillSelection, runtimeSim.fastForward(),
                                     &*panelIconAnimation);
          for (std::size_t i = 0; i < hudIcons.size(); ++i)
            app::compositeIconRegion(image, runtimeHudIcons[i], hudIcons[i].placement);
          app::drawRuntimeHud(image, *runtimeFont, *world, false);
          app::drawRuntimeNukeFeedback(image, *runtimeFont, *world, nukeInput.armed());
          // actor-anim-001 (P6): the bomber countdown above each lemming whose fuse is lit.
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
            // A12 supersedes Y13: request Letsgo on the first drawn text frame.
            // Each entry/Retry owns this latch, including silent attempts; neither
            // simulation passes nor later pause/menu redraws can retrigger it.
            if (entryVoicePending) {
              entryVoicePending = false;
              if (cues) cues->play(sim::SoundCue::LevelStart);
              view.audioCuesDrained = cues ? cues->drained() : 0;
              view.audioCuesRouted = cues ? cues->routed() : 0;
            }
          }
          if (const int remainingMs = world->nukeCountdownMs(); remainingMs > 0) {
            // A3: 44279A..4427B2 anchors the nuke digit at (320,240), centred;
            // 414132 adds (5,6). 413F15/41419F expand it during each second;
            // PROVISIONAL 640x480 adapter: fix the native mode factor to zero:
            // dx=trunc(150*phase), dy=trunc(195*phase), additive ^Font text.
            // PROVISIONAL boundary adapter: ceil the existing integer-ms timer
            // to 5..1 (no native blank '6' at t=5 or extra digit at exact seconds).
            // Reading World alone keeps pause, speed and explosion timing intact.
            const int digit = (remainingMs + 999) / 1000;
            const int phaseMs = digit * 1000 - remainingMs;
            const int dx = 150 * phaseMs / 1000, dy = 195 * phaseMs / 1000;
            const app::EntryCueFrame countdown{{320 - dx, 240 - dy, 330 + dx, 252 + dy}, true};
            app::drawEntryStartCue(image, *runtimeFont, std::to_string(digit), countdown);
          }
          if (menuOpen() && persistentFrontend->screen() == shell::Screen::SoundOptions)
            image = persistentFrontend->renderSoundOptions(image);
          else if (levelMenu) levelMenu->draw(image);
          const auto counts = app::runtimeSkillPanelCounts(*world);
          const int panelAvailability = panelIndex >= 0 && panelIndex < 8 ? counts[panelIndex] : 0;
          const auto cursorState = menuOpen() ? persistentFrontend->cursorState() :
              app::runtimeCursorState(focused, rightDown, hover.has_value(),
                  runtimeSim.paused(), panelIndex, panelAvailability, hatchHover.has_value());
          runtimeCursor->draw(image, cursorState, pointerX, pointerY);
          // READ45E2B5/45E342: cue is below the later cursor and pause caption.
          if (runtimeSim.paused() && !menuOpen()) shell::drawCaption(image, *runtimeFont, shell::fontCells());
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
  const auto args = parseArgs(argc, argv);
  if (args.invalidWindowScale) { std::fprintf(stderr, "--window-scale requires 1, 2 or 3\n"); return 1; }
  if (args.invalidReplayOption || (!args.recordDir.empty() && !args.replayFile.empty())) {
    std::fprintf(stderr, "--record DIR and --replay FILE require paths and cannot be combined\n"); return 1;
  }
  if (!args.replayFile.empty() && !args.help) {
    auto library = AssetLibrary::open(fs::path(args.gameDir) / "lemmings.box", fs::path(args.gameDir) / "patch.box");
    if (!library) { std::fprintf(stderr, "replay assets: %s\n", library.error().c_str()); return 1; }
    return app::replayLevel(*library, args.replayFile);
  }
  if (!args.recordDir.empty() && !args.help &&
      ((!args.frontend && !args.sim) || args.map || !args.shot.empty() || args.simTicks < 0 || args.simTicks > 1000000)) {
    std::fprintf(stderr, "--record requires frontend play or --level ID --sim, without --map/--shot\n"); return 1;
  }
#if !defined(__EMSCRIPTEN__) && !defined(WEBOLUTION_APP_NO_MAIN)
  if (!args.help && !args.map && args.shot.empty()) {
    std::puts("Lemmings Webolution (native)");
    std::fflush(stdout);
  }
#endif
  // Read the clone image once before any gameplay clock starts. Attempt setup
  // later uses this cached identity and never hashes inside a measured frame.
  if (!args.recordDir.empty()) (void)app::replayBuildIdentity();
  sceneryRandom().begin();  // main-loop entry seed (0x0045D06C analogue)
  std::optional<app::LevelOutcome> completed;
  LegDisposition disposition = LegDisposition::Exit;
  // One cue adapter, hence one Mixer and one device, for the whole process:
  // it outlives every frontend and gameplay leg below.
  audio::CuePlayback* cues = nullptr;
  save::SoundSettings sound;
  std::shared_ptr<save::SaveSink> soundFile;
  if (options.useDefaultSaveStore && !options.saveStore && !args.help && args.shot.empty()) {
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
#endif
  if (!args.frontend || !args.shot.empty() || args.help || args.map)
    return runApplicationLeg(argc, argv, std::move(observer), std::move(levelObserver), options,
                             nullptr, nullptr, cues, completed, disposition);
  auto library = AssetLibrary::open(fs::path(args.gameDir) / "lemmings.box",
                                    fs::path(args.gameDir) / "patch.box");
  if (!library) { std::fprintf(stderr, "assets: %s\n", library.error().c_str()); return 1; }
  // Progress store: injected by tests, or the clone's own per-user SavedGame.dat
  // for the CLI. Never the installed game directory's save.
  auto store = options.saveStore;
  if (!store && options.useDefaultSaveStore) {
    if (auto made = save::makeDefaultSaveStore(fs::path(args.gameDir)); made) {
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
  // Own the window and renderer across menu, levels, retries and results. Legs
  // borrow them and own only their texture; no video shutdown between pages.
  SdlWindow presentation(windowScale(args, bool(observer) || bool(levelObserver)));
  auto* sharedWindow = &presentation;
#else
  SdlWindow* sharedWindow = nullptr;
#endif
  session->configureSound(sound, std::move(soundFile), std::move(preview));
  // Keep this session alive across level/results/map/retry legs. Every level
  // still uses the same loader and SDL runtime; no recursive app invocation.
  for (;;) {
    completed.reset();
    disposition = LegDisposition::Exit;
    const int status = runApplicationLeg(argc, argv, observer, levelObserver, options, &*session, sharedWindow, cues, completed, disposition);
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
