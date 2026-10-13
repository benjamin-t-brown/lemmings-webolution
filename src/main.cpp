// Lemmings Webolution.
//
// Runs the simulation and draws it. The level is a built-in test level for
// now: .LVL is still being reverse engineered (see PLAN.md 1.6), so there is
// no real level to load yet, but everything downstream of it is real --
// including the lemmings, which are drawn with the original's own artwork
// when an installation is reachable. Point WEBOLUTION_GAME_DIR at one, or
// accept the default below. Without it the game still runs and draws
// placeholder boxes, so the simulation stays testable on a bare checkout.
//
//   left click      assign the selected skill to the lemming under the cursor
//   1..8            choose a skill
//   A D / left right  pan
//   C               toggle the cylinder projection
//   T               toggle the level's skin textures
//   up down         raise and lower the cylinder camera
//   page up/down    move it closer and further
//
// Sound cues (Letsgo, Chink, Ohno, Explode, Splat, Yippee) play when an
// installation is reachable and the audio device opens. Missing audio is
// silent, not fatal -- same rule as missing sprites.
//
//   --cylinder      start in the cylinder projection
//   --native        open at the original's 640x480
//   --size WxH      open at a given size
//   space           pause
//   .               single step while paused
//   N               nuke
//   R               restart
//   Esc             quit
//
//   --level <id>    take the level's parameters from LEVELMAP.DAT, e.g.
//                   --level MM_BLOCK_001_A. The terrain stays the built-in
//                   one; only the lemming count, save target and time limit
//                   come from the data.

#include "assets/Bitmap.h"
#include "assets/Png.h"
#include "audio/Mixer.h"
#include "gfx/Gl.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "gfx/CylinderSkin.h"
#include "gfx/LemmingSprites.h"
#include "gfx/Renderer.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "sim/World.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <string>
#include <vector>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

namespace {

using namespace webo;
namespace fs = std::filesystem;

constexpr int kTicksPerSecond = 20;

// Where to look for the game data when WEBOLUTION_GAME_DIR is not set. The
// engine never ships data; it reads an existing installation.
constexpr const char* kDefaultGameDir = "C:/progs/Lemmings Revolution";

// A hand-built level, until .LVL gives us real ones: a floor, some pillars to
// climb and bash, a gap to build over, and a pit at the seam so the
// wraparound gets exercised by just playing.
//
// The terrain is hand-built, but the level's *parameters* need not be
// invented. LEVELMAP.DAT states the lemming count, the save target and the
// time limit for all 125 slots, so those are taken from there whenever a
// record is supplied, and only the shape of the ground is ours.
sim::LevelSetup makeTestLevel(const ReleaseRates& rates, int releaseRate,
                              const LevelInfo* info) {
  sim::LevelSetup s;
  const int w = 320, h = 200;
  s.terrain = sim::Terrain(w, h);

  s.terrain.fillBox(0, 150, w, h - 150, sim::Material::Earth);
  s.terrain.fillBox(40, 120, 10, 30, sim::Material::Earth);    // a step
  s.terrain.fillBox(110, 90, 12, 60, sim::Material::Earth);    // a wall to climb
  s.terrain.fillBox(180, 120, 40, 30, sim::Material::Rock);    // rock to bash
  s.terrain.fillBox(260, 140, 30, 10, sim::Material::Steel);   // steel, resists
  s.terrain.clearBox(150, 150, 20, 50);                        // a gap to build over
  s.terrain.clearBox(310, 150, 20, 50);                        // a gap across the seam

  s.entrances.push_back({20, 130});
  s.exits.push_back({230, 138, 12, 14});
  s.totalLemmings = info ? static_cast<int>(info->totalLemmings) : 20;
  s.saveTarget = info ? static_cast<int>(info->saveTarget) : 10;
  if (info)
    s.timeLimitTicks = static_cast<int>(info->timeLimitSeconds) * kTicksPerSecond;
  // Real spacing, from RELEASERATE.PC, when the data is there. Rate 50 is
  // mid-curve. Falls back to a plausible constant otherwise.
  s.releaseIntervalTicks =
      rates.empty() ? 24 : rates.intervalTicks(releaseRate, kTicksPerSecond);
  return s;
}

constexpr std::array<sim::Skill, 8> kSkills = {
    sim::Skill::Climber, sim::Skill::Floater, sim::Skill::Bomber, sim::Skill::Blocker,
    sim::Skill::Builder, sim::Skill::Basher,  sim::Skill::Miner,  sim::Skill::Digger,
};

struct App {
  SDL_Window* window = nullptr;
  SDL_GLContext context = nullptr;
  gfx::Renderer renderer;
  gfx::LemmingSprites sprites;
  ReleaseRates releaseRates;
  int releaseRate = 50;
  std::vector<LevelInfo> levelInfos;
  std::string levelId;      // empty = use the built-in defaults
  std::string levelName;
  sim::World world;

  bool running = true;
  bool paused = false;
  bool stepOnce = false;
  int skill = 7;  // digger
  float camX = 0;
  float camWidth = 320;
  std::uint64_t lastTick = 0;
  bool terrainDirty = true;
  int animTick = 0;
  bool useSprites = true;
  bool cylinder = false;
  bool drawSkin = true;
  gfx::CylinderView view;
  gfx::CylinderSkin skin;
  audio::Mixer mixer;

  const LevelInfo* levelInfo() const {
    for (const auto& i : levelInfos)
      if (i.id == levelId)
        return &i;
    return nullptr;
  }

  void restart() {
    world = sim::World(makeTestLevel(releaseRates, releaseRate, levelInfo()));
    terrainDirty = true;
    // Level-start fanfare. Shell-owned rather than a World cue: restart is
    // not a simulation event, and the original plays this before anyone
    // walks out of the entrance.
    mixer.play("Letsgo");
  }

  // World speaks in SoundCue; the mixer speaks in retail names.
  static const char* cueName(sim::SoundCue c) {
    switch (c) {
      case sim::SoundCue::Assign: return "Chink";
      case sim::SoundCue::OhNo: return "Ohno";
      case sim::SoundCue::Explode: return "Explode";
      case sim::SoundCue::Splat: return "Splat";
      case sim::SoundCue::Exit: return "Yippee";
    }
    return nullptr;
  }

  void drainCues() {
    for (const sim::SoundCue c : world.takeCues())
      if (const char* name = cueName(c))
        mixer.play(name);
  }

  void handleKey(SDL_Keycode k) {
    switch (k) {
      case SDLK_ESCAPE: running = false; break;
      case SDLK_SPACE: paused = !paused; break;
      case SDLK_PERIOD: stepOnce = true; break;
      case SDLK_n: world.nuke(); break;
      case SDLK_r: restart(); break;
      case SDLK_LEFT: case SDLK_a: pan(-16); break;
      case SDLK_RIGHT: case SDLK_d: pan(16); break;
      case SDLK_c: toggleCylinder(); break;
      case SDLK_t: drawSkin = !drawSkin; break;
      case SDLK_UP: view.eyeHeight -= 8; view.lookHeight -= 8; break;
      case SDLK_DOWN: view.eyeHeight += 8; view.lookHeight += 8; break;
      case SDLK_PAGEUP: view.distance -= 16; view.clamp(); break;
      case SDLK_PAGEDOWN: view.distance += 16; view.clamp(); break;
      default:
        if (k >= SDLK_1 && k <= SDLK_8)
          skill = static_cast<int>(k - SDLK_1);
        break;
    }
  }

  float worldWidth() const { return static_cast<float>(world.terrain().width()); }

  // Pans in whichever view is active, keeping the two in step so toggling
  // does not jump: the flat camera's X and the orbit angle mean the same
  // thing, one unwrapped and one wrapped.
  void pan(float dx) {
    camX += dx;
    view.centreOnWorldX(camX + camWidth * 0.5f, worldWidth());
  }

  void toggleCylinder() {
    cylinder = !cylinder;
    if (cylinder) {
      // Radius from the circumference keeps texels square, so the wrapped
      // view is the flat one rolled up rather than stretched.
      view.radius = worldWidth() / (2.0f * std::numbers::pi_v<float>);
      view.height = static_cast<float>(world.terrain().height());
      view.eyeHeight = view.height * 0.5f;
      view.lookHeight = view.height * 0.5f;
      // Far enough back that the whole tube fits, rather than a guessed
      // multiple of the radius: the test level is far taller than it is wide,
      // and a radius-based distance crops it badly. Take whichever of height
      // and diameter needs more room, plus a margin.
      const float halfFov = std::tan(view.fovY * 0.5f);
      const float forHeight = view.height * 0.5f / halfFov;
      const float forWidth = view.radius / halfFov;
      view.distance = std::max(forHeight, forWidth) * 1.05f + view.radius;
      view.centreOnWorldX(camX + camWidth * 0.5f, worldWidth());
      view.clamp();
    }
  }

  void handleClick(int px, int py) {
    int vw = 0, vh = 0;
    SDL_GetWindowSize(window, &vw, &vh);
    if (vw <= 0 || vh <= 0)
      return;
    const float worldH = camWidth * static_cast<float>(vh) / static_cast<float>(vw);
    const int wx = static_cast<int>(camX + camWidth * static_cast<float>(px) / vw);
    const int wy = static_cast<int>(worldH * static_cast<float>(py) / vh);

    if (const auto hit = world.pick(wx, wy, 8)) {
      if (world.assign(*hit, kSkills[static_cast<std::size_t>(skill)])) {
        terrainDirty = true;
        drainCues();  // Assign / OhNo fire from assign, not from tick
      }
    }
  }

  // present=false leaves the rendered frame in the back buffer so it can be
  // read back; after a swap the back buffer's contents are undefined.
  void frame(bool present = true) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT)
        running = false;
      else if (e.type == SDL_KEYDOWN)
        handleKey(e.key.keysym.sym);
      else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT)
        handleClick(e.button.x, e.button.y);
    }

    const std::uint64_t now = SDL_GetTicks64();
    if (!paused || stepOnce) {
      const std::uint64_t due = stepOnce ? 1 : (now - lastTick) * kTicksPerSecond / 1000;
      for (std::uint64_t i = 0; i < due && i < 8; ++i)
        world.tick();
      if (due) {
        lastTick = now;
        drainCues();
      }
      stepOnce = false;
      terrainDirty = true;  // skills edit terrain; cheap enough at this size
    }

    int vw = 0, vh = 0;
    SDL_GetWindowSize(window, &vw, &vh);
    renderer.beginFrame(vw, vh, {0.06f, 0.07f, 0.12f, 1});
    renderer.setCamera(camX, 0, camWidth);
    if (cylinder)
      renderer.setCylinder(view, worldWidth(), view.radius);
    else
      renderer.setFlat();

    if (terrainDirty) {
      renderer.uploadTerrain(world.terrain());
      terrainDirty = false;
    }
    // The level's own artwork, behind the simulation. It is scenery: what is
    // solid is the terrain mask, and the mapping between the two is still
    // open, so the mask is drawn over it rather than replaced by it.
    if (drawSkin && skin.ready())
      skin.draw(renderer, worldWidth(), static_cast<float>(world.terrain().height()));

    renderer.drawTerrain();

    for (const auto& l : world.lemmings()) {
      if (!l.alive())
        continue;
      gfx::Colour c{0.35f, 0.45f, 0.95f, 1};
      switch (l.state) {
        case sim::State::Blocker: c = {0.95f, 0.85f, 0.2f, 1}; break;
        case sim::State::Builder: c = {0.95f, 0.5f, 0.2f, 1}; break;
        case sim::State::Digger:
        case sim::State::Basher:
        case sim::State::Miner: c = {0.9f, 0.3f, 0.3f, 1}; break;
        case sim::State::Climber:
        case sim::State::Hoister: c = {0.4f, 0.9f, 0.5f, 1}; break;
        case sim::State::Splatter: c = {0.6f, 0.1f, 0.1f, 1}; break;
        default: break;
      }
      if (l.fuse > 0)
        c = {1.0f, 0.2f, 0.2f, 1};

      // The original's artwork when it loaded, the placeholder box otherwise,
      // so the game still runs without an installation to read.
      const bool drawn = useSprites && sprites.draw(renderer, l.state, static_cast<float>(l.x),
                                                    static_cast<float>(l.y), 8.0f, animTick / 4,
                                                    l.facing == sim::Facing::Left);
      if (!drawn) {
        const float x = static_cast<float>(l.x) - 2;
        const float y = static_cast<float>(l.y) - 8;
        renderer.drawQuad(x, y, 4, 8, c);
        // A tuft so facing is visible at this size.
        renderer.drawQuad(x + (l.facing == sim::Facing::Right ? 3 : -1), y, 2, 2,
                          {0.3f, 0.9f, 0.3f, 1});
      }
    }

    // Exits and entrances, so the goal is obvious.
    for (const auto& ex : world.setup().exits)
      renderer.drawQuad(static_cast<float>(ex.x), static_cast<float>(ex.y),
                        static_cast<float>(ex.width), static_cast<float>(ex.height),
                        {0.2f, 0.9f, 0.9f, 0.55f});
    for (const auto& en : world.setup().entrances)
      renderer.drawQuad(static_cast<float>(en.x) - 4, static_cast<float>(en.y) - 6, 8, 6,
                        {0.9f, 0.9f, 0.2f, 0.55f});

    ++animTick;

    renderer.endFrame();
    if (present)
      SDL_GL_SwapWindow(window);

    const std::string title = std::format(
        "Lemmings Webolution  --  {}  |  {}  |  out {} saved {}/{} dead {}  |  tick {}{}",
        levelName.empty() ? "test level" : levelName,
        sim::skillName(kSkills[static_cast<std::size_t>(skill)]), world.activeCount(),
        world.saved(), world.setup().saveTarget, world.dead(), world.tickCount(),
        paused ? "  [paused]" : "");
    SDL_SetWindowTitle(window, title.c_str());
  }
};

App g_app;

void frameThunk() { g_app.frame(); }

}  // namespace

int main(int argc, char** argv) {
  bool headless = false;
  int headlessTicks = 0;
  std::string shotPath;
  int shotTicks = 120;
  bool startCylinder = false;
  // The original ran at 640x480. That is the reference the renderer is meant
  // to reproduce, so it is one flag away; a larger window is just easier to
  // work in.
  int windowW = 1280;
  int windowH = 800;
  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    if (a == "--headless" && i + 1 < argc) {
      headless = true;
      headlessTicks = std::atoi(argv[++i]);
    } else if (a == "--shot" && i + 1 < argc) {
      // Render a frame and read it back through GL. Desktop screen capture of
      // an OpenGL surface often comes back black, so this is the honest way to
      // check that the renderer actually draws -- and it works in CI.
      shotPath = argv[++i];
    } else if (a == "--shot-ticks" && i + 1 < argc) {
      shotTicks = std::atoi(argv[++i]);
    } else if (a == "--native") {
      windowW = 640;
      windowH = 480;
    } else if (a == "--size" && i + 1 < argc) {
      const std::string spec = argv[++i];
      const auto x = spec.find('x');
      if (x != std::string::npos) {
        windowW = std::max(64, std::atoi(spec.substr(0, x).c_str()));
        windowH = std::max(64, std::atoi(spec.substr(x + 1).c_str()));
      }
    } else if (a == "--cylinder") {
      startCylinder = true;
    } else if (a == "--level" && i + 1 < argc) {
      // A level id from LEVELMAP.DAT, e.g. MM_CLIMB_002. Only its parameters
      // are used so far; the terrain stays the built-in one until .LVL is
      // solved, and the title bar says so.
      g_app.levelId = argv[++i];
    }
  }

  // A headless mode so the simulation can be exercised in CI, and so that
  // "does it run" is answerable without a display.
  if (headless) {
    sim::World w(makeTestLevel(ReleaseRates{}, 50, nullptr));
    for (int i = 0; i < headlessTicks; ++i)
      w.tick();
    std::printf("ticks=%llu active=%d saved=%d dead=%d solid=%zu\n",
                static_cast<unsigned long long>(w.tickCount()), w.activeCount(), w.saved(),
                w.dead(), w.terrain().solidCells());
    return 0;
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
    return 1;
  }

#if defined(__EMSCRIPTEN__)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  // The cylinder view needs a depth buffer, or the far side of the tube
  // paints over the near side.
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  g_app.window = SDL_CreateWindow("Lemmings Webolution", SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, windowW, windowH,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (!g_app.window) {
    std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  g_app.context = SDL_GL_CreateContext(g_app.window);
  if (!g_app.context) {
    std::fprintf(stderr, "SDL_GL_CreateContext: %s\n", SDL_GetError());
    SDL_DestroyWindow(g_app.window);
    SDL_Quit();
    return 1;
  }
  SDL_GL_SetSwapInterval(1);

  if (auto r = g_app.renderer.init(); !r) {
    std::fprintf(stderr, "renderer: %s\n", r.error().c_str());
    return 1;
  }

  // The original's lemming artwork, if an installation is reachable. Not
  // having one is not an error: the game falls back to placeholder boxes and
  // still runs, which keeps the simulation testable on a bare checkout.
  {
    const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
    const fs::path gameDir = dir ? fs::path(dir) : fs::path(kDefaultGameDir);
    auto lib = AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
    if (!lib) {
      std::fprintf(stderr, "sprites: %s (drawing placeholders)\n", lib.error().c_str());
    } else if (auto r = g_app.sprites.load(*lib, g_app.renderer); !r) {
      std::fprintf(stderr, "sprites: %s (drawing placeholders)\n", r.error().c_str());
    } else {
      for (const auto& m : g_app.sprites.missingSheets())
        std::fprintf(stderr, "sprites: %s\n", m.c_str());
      std::fprintf(stderr, "sprites: loaded %zu from %s\n",
                   g_app.sprites.definitions().sprites.size(), gameDir.string().c_str());

      // Level parameters, so the goal and the lemming count are the real
      // ones even while the terrain is still ours.
      if (auto infos = loadLevelMap(*lib)) {
        g_app.levelInfos = std::move(*infos);
        if (const LevelInfo* info = g_app.levelInfo()) {
          g_app.levelName = info->name;
          std::fprintf(stderr, "level %s \"%s\": save %u of %u, %us\n", info->id.c_str(),
                       info->name.c_str(), info->saveTarget, info->totalLemmings,
                       info->timeLimitSeconds);
        } else if (!g_app.levelId.empty()) {
          std::fprintf(stderr, "level %s: not in LEVELMAP.DAT, using defaults\n",
                       g_app.levelId.c_str());
        }
      } else {
        std::fprintf(stderr, "level map: %s (using defaults)\n", infos.error().c_str());
      }

      // The level's cylinder skin, if a level was named. Its tile textures are
      // the only part of a .LVL the renderer can use today.
      if (!g_app.levelId.empty()) {
        std::string path;
        for (const auto& candidate : levelFileNames(*lib))
          if (candidate.find(g_app.levelId) != std::string::npos) {
            path = candidate;
            break;
          }
        if (path.empty()) {
          std::fprintf(stderr, "skin: no .LVL matching %s\n", g_app.levelId.c_str());
        } else if (auto blob = lib->read(path)) {
          auto mesh = findCylinderMesh(*blob);
          if (!mesh) {
            std::fprintf(stderr, "skin: %s\n", mesh.error().c_str());
          } else if (auto r = g_app.skin.load(*lib, g_app.renderer, *mesh); !r) {
            std::fprintf(stderr, "skin: %s\n", r.error().c_str());
          } else {
            for (const auto& m : g_app.skin.missing())
              std::fprintf(stderr, "skin: %s\n", m.c_str());
            std::fprintf(stderr, "skin: %zu tiles (%zu outer, %zu inner) from %s\n",
                         g_app.skin.tileCount(), g_app.skin.outerCount(),
                         g_app.skin.innerCount(), path.c_str());
          }
        }
      }

      // The spawn interval comes from the game's own curve when it is there.
      if (auto rr = loadReleaseRateTable(*lib)) {
        g_app.releaseRates = std::move(*rr);
        std::fprintf(stderr, "release rate %d: %.2f s per lemming, %d ticks\n", g_app.releaseRate,
                     g_app.releaseRates.secondsPerLemming(g_app.releaseRate),
                     g_app.releaseRates.intervalTicks(g_app.releaseRate, kTicksPerSecond));
      } else {
        std::fprintf(stderr, "release rates: %s (using a fallback interval)\n",
                     rr.error().c_str());
      }

      // Sound cues used by the simulation. Missing any one is not fatal --
      // play() is a no-op for unknown names -- but opening the device is.
      if (auto r = g_app.mixer.open(); !r) {
        std::fprintf(stderr, "audio: %s (silent)\n", r.error().c_str());
      } else {
        int loaded = 0;
        for (const char* cue : {"Letsgo", "Chink", "Ohno", "Explode", "Splat", "Yippee"}) {
          if (auto lr = g_app.mixer.loadFrom(*lib, cue); !lr)
            std::fprintf(stderr, "audio: %s: %s\n", cue, lr.error().c_str());
          else
            ++loaded;
        }
        std::fprintf(stderr, "audio: loaded %d cues at %d Hz\n", loaded,
                     g_app.mixer.sampleRate());
      }
    }
  }

  g_app.restart();
  if (startCylinder)
    g_app.toggleCylinder();
  g_app.lastTick = SDL_GetTicks64();

  if (!shotPath.empty()) {
    for (int i = 0; i < shotTicks; ++i)
      g_app.world.tick();
    g_app.paused = true;
    g_app.frame(false);

    int vw = 0, vh = 0;
    SDL_GL_GetDrawableSize(g_app.window, &vw, &vh);
    Image shot;
    shot.width = vw;
    shot.height = vh;
    shot.rgba.resize(static_cast<std::size_t>(vw) * vh * 4);
    glReadPixels(0, 0, vw, vh, GL_RGBA, GL_UNSIGNED_BYTE, shot.rgba.data());

    // GL reads bottom-up; flip into the top-down order the writer expects.
    const std::size_t stride = static_cast<std::size_t>(vw) * 4;
    for (int y = 0; y < vh / 2; ++y)
      std::swap_ranges(shot.rgba.begin() + y * stride,
                       shot.rgba.begin() + (y + 1) * stride,
                       shot.rgba.begin() + (vh - 1 - y) * stride);

    if (auto r = writePng(shotPath, shot); !r)
      std::fprintf(stderr, "screenshot: %s\n", r.error().c_str());
    else
      std::printf("wrote %s (%dx%d)\n", shotPath.c_str(), vw, vh);

    g_app.mixer.close();
    SDL_GL_DeleteContext(g_app.context);
    SDL_DestroyWindow(g_app.window);
    SDL_Quit();
    return 0;
  }

#if defined(__EMSCRIPTEN__)
  emscripten_set_main_loop(frameThunk, 0, 1);
#else
  while (g_app.running)
    frameThunk();
#endif

  g_app.mixer.close();
  SDL_GL_DeleteContext(g_app.context);
  SDL_DestroyWindow(g_app.window);
  SDL_Quit();
  return 0;
}
