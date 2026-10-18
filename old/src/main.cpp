// Lemmings Webolution.
//
// Runs the simulation and draws it. With --level <id> the terrain is rebuilt
// from the level's surface lattice (see PLAN.md 1.6); without it, a hand-built
// test level is used so the simulation stays playable on a bare checkout.
// Lemmings are drawn with the original's artwork when an installation is
// reachable. Point WEBOLUTION_GAME_DIR at one, or accept the default below.
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
//   Esc             pause in-level / back in menus / quit from main
//
//   --level <id>    skip the shell and load that level (debug bypass)
//   --lang <name>   force a LANGUAGE\*.LTX (default UK when present)

#include "assets/Bitmap.h"
#include "assets/Png.h"
#include "audio/Mixer.h"
#include "gfx/Gl.h"
#include "assets/CollisionGrid.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/LevelObjects.h"
#include "assets/SurfaceMesh.h"
#include "gfx/CylinderSkin.h"
#include "gfx/Font.h"
#include "gfx/LemmingSprites.h"
#include "gfx/Renderer.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "shell/Shell.h"
#include "sim/World.h"

#include <optional>

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

// Measured off the running original, not chosen: it steps its lemmings five
// times a second, on a flat 200 ms period that does not move with the frame
// rate. See sim::World::Rules::ticksPerSecond, tools/harness/measure_tick.py
// and PLAN.md 2.2. Keep the two in step.
constexpr int kTicksPerSecond = 5;

// Where to look for the game data when WEBOLUTION_GAME_DIR is not set. The
// engine never ships data; it reads an existing installation.
constexpr const char* kDefaultGameDir = "C:/progs/Lemmings Revolution";

void applyLevelParams(sim::LevelSetup& s, const ReleaseRates& rates, int releaseRate,
                      const LevelInfo* info) {
  s.totalLemmings = info ? static_cast<int>(info->totalLemmings) : 20;
  s.saveTarget = info ? static_cast<int>(info->saveTarget) : 10;
  if (info)
    s.timeLimitTicks = static_cast<int>(info->timeLimitSeconds) * kTicksPerSecond;
  // The fallback is a 1.2 second interval, in ticks -- it only applies when
  // RELEASERATE.PC is missing, which no real install is.
  s.releaseIntervalTicks = rates.empty() ? (kTicksPerSecond * 6 + 4) / 5
                                         : rates.intervalTicks(releaseRate, kTicksPerSecond);
}

// Fallback when a .LVL has no readable Entrance.a3d / Exit.a3d transform:
// drop an entrance above the leftmost solid cell of the highest occupied row,
// and an exit on solid ground roughly opposite.
void placeSpawnPoints(sim::LevelSetup& s) {
  const auto& t = s.terrain;
  int topY = -1, leftX = 0, rightX = 0;
  for (int y = 0; y < t.height(); ++y) {
    for (int x = 0; x < t.width(); ++x) {
      if (!t.solid(x, y))
        continue;
      if (topY < 0 || y < topY) {
        topY = y;
        leftX = x;
        rightX = x;
      } else if (y == topY) {
        leftX = std::min(leftX, x);
        rightX = std::max(rightX, x);
      }
    }
  }
  if (topY < 0) {
    // Empty mask: fall back to the middle of the sky so the level still runs.
    s.entrances.push_back({t.width() / 4, t.height() / 4});
    // Round 11: measured exit AABB is 2 x 3 inclusive, not a guessed 12 x 14.
    s.exits.push_back({3 * t.width() / 4, t.height() / 2, 2, 3});
    return;
  }
  const int enterY = std::max(0, topY - 10);
  s.entrances.push_back({leftX, enterY});

  // Exit on a standable cell (solid with empty above) far around the tube
  // from the entrance. Box covers the standable feet row (bestY - 1) inside
  // the measured 2 x 3 inclusive AABB.
  int bestX = leftX, bestY = topY, bestDist = -1;
  for (int y = 1; y < t.height(); ++y) {
    for (int x = 0; x < t.width(); ++x) {
      if (!t.solid(x, y) || t.solid(x, y - 1))
        continue;
      const int dx = std::abs(t.wrapX(x - leftX + t.width() / 2) - t.width() / 2);
      if (dx > bestDist) {
        bestDist = dx;
        bestX = x;
        bestY = y;
      }
    }
  }
  s.exits.push_back({bestX - 1, bestY - 2, 2, 3});
}

sim::Terrain terrainFromMask(const OccupancyMask& mask) {
  sim::Terrain t(mask.width, mask.height);
  for (int y = 0; y < mask.height; ++y)
    for (int x = 0; x < mask.width; ++x)
      if (mask.solid[static_cast<std::size_t>(y) * mask.width + x])
        t.set(x, y, sim::Material::Earth);
  return t;
}

// The collision mask the original keeps in the .LVL is one cell per world
// unit -- 256 x 96 -- so the simulation runs at that resolution and every
// constant in Rules is a world-unit count with no conversion.
constexpr int kPixelsPerUnit = 1;

// A hand-built level for when no .LVL surface is loaded: a floor, pillars,
// a gap, and a pit at the seam so wraparound gets exercised by just playing.
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
  // Floor top is y=150; feet rest at 149. Measured 2 x 3 inclusive box.
  s.exits.push_back({229, 146, 2, 3});
  applyLevelParams(s, rates, releaseRate, info);
  return s;
}

sim::LevelSetup makeSurfaceLevel(const CollisionGrid& mask, const ReleaseRates& rates,
                                 int releaseRate, const LevelInfo* info,
                                 const LevelObjects* objects, const LevelSkills* skills,
                                 double worldH, int pixelsPerUnit = kPixelsPerUnit) {
  sim::LevelSetup s;
  s.terrain = sim::Terrain::fromCollisionGrid(mask);
  if (objects) {
    // The original releases from the hatch's own stored words, not from the
    // Entrance*.a3d translation (webo::HatchRecord, src/sim/World.cpp spawn).
    // Every retail level has at least one hatch record; the translation stays
    // the fallback for the levels and test setups that have none.
    for (const auto& h : objects->hatches) {
      const auto [x, y] = hatchSpawnCell(h, mask.width, mask.height);
      s.entrances.push_back(
          {x, y, sim::Entrance::Source::HatchWords, h.type});
    }
    if (objects->hatches.empty()) {
      for (const auto& e : objects->entrances) {
        const auto [x, y] =
            worldToMaskPixel(e.position, mask.width, mask.height, worldH, pixelsPerUnit);
        s.entrances.push_back({x, y, sim::Entrance::Source::EntranceModel});
      }
    }
    for (const auto& e : objects->exits) {
      // Round 11: object-overlap AABB from the Exit*.a3d pose, not a
      // guessed rectangle and not worldToMaskPixel's Y (off by one for
      // integer ty; wrong axis for this box).
      const ExitTrigger box = exitTriggerFromPose(e, mask.width, mask.height);
      s.exits.push_back({box.x, box.y, box.width, box.height});
    }
    // Round 16: teleport.a3d pads. Pair file order i ↔ i+n/2 (MEASURED on
    // MM_TELE_003: four pads → (0,2)(1,3); two-pad levels → (0,1)). The
    // original links by stream id at object+0x98 via 0x00444BA1; that word
    // is not yet parsed from the .LVL, so the even-count index rule is the
    // sim stand-in. teleport_fx.a3d is skipped by findLevelObjects.
    std::vector<TeleportTrigger> telePads;
    for (const auto& p : objects->placed) {
      if (p.kind != PlacedKind::Teleport)
        continue;
      telePads.push_back(
          teleportTriggerFromPose(p.pose, mask.width, mask.height));
    }
    const std::size_t n = telePads.size();
    if (n >= 2 && (n % 2) == 0) {
      const std::size_t half = n / 2;
      for (std::size_t i = 0; i < n; ++i) {
        const auto& src = telePads[i];
        const auto& dst = telePads[(i + half) % n];
        s.teleports.push_back({
            src.x,
            src.y,
            src.width,
            src.height,
            dst.destCol,
            mask.height - 1 - dst.destWorldY,
        });
      }
    }
    // Round 17: speedup_two_way.a3d → SpeedPad AABB (vtable 0x004B1178).
    for (const auto& p : objects->placed) {
      if (p.kind != PlacedKind::SpeedUp)
        continue;
      const SpeedPadTrigger box =
          speedPadTriggerFromPose(p.pose, mask.width, mask.height);
      s.speedPads.push_back({box.x, box.y, box.width, box.height, 0, true});
    }
    // Round 21: lever.a3d → Lever AABB (vtable 0x004B1288).
    for (const auto& p : objects->placed) {
      if (p.kind != PlacedKind::Lever)
        continue;
      const LeverTrigger box =
          leverTriggerFromPose(p.pose, mask.width, mask.height);
      s.levers.push_back({box.x, box.y, box.width, box.height, 1});
    }
  }
  // Only the full guesser when nothing was read; a partial read is still
  // better than inventing a second entrance next to a real one.
  if (s.entrances.empty() && s.exits.empty())
    placeSpawnPoints(s);
  else if (s.entrances.empty())
    s.entrances.push_back({mask.width / 4, mask.height / 4});
  else if (s.exits.empty())
    s.exits.push_back({3 * mask.width / 4, mask.height / 2, 2, 3});
  if (skills) {
    for (std::size_t i = 0; i < skills->counts.size(); ++i)
      s.skills[i] = static_cast<int>(skills->counts[i]);
  }
  applyLevelParams(s, rates, releaseRate, info);
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
  gfx::Font font;
  unsigned fontTex = 0;
  ReleaseRates releaseRates;
  int releaseRate = 50;
  std::vector<LevelInfo> levelInfos;
  std::string levelId;      // empty = use the built-in defaults
  std::string levelName;
  // When --level loads a surface successfully, restart uses this instead of
  // the hand-built test terrain.
  std::optional<sim::LevelSetup> surfaceLevel;
  sim::World world;
  shell::Shell shell;
  std::optional<AssetLibrary> library;
  fs::path gameDir;
  int playGroup = 0;
  int playSlot = 0;
  bool bypassShell = false;  // --level
  bool resultsShown = false;

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

  bool inMenus() const {
    return !bypassShell && shell.screen() != shell::Screen::Playing;
  }

  void restart() {
    if (surfaceLevel)
      world = sim::World(*surfaceLevel);
    else
      world = sim::World(makeTestLevel(releaseRates, releaseRate, levelInfo()));
    // A level that only stocks climbers (MM_CLIMB_002) has to start on that
    // skill. The panel default is the digger, and a click with an empty
    // stock is refused, so on that level nothing would ever climb.
    skill = 7;
    const auto& counts = world.setup().skills;
    for (int i = 0; i < static_cast<int>(counts.size()); ++i) {
      if (counts[static_cast<std::size_t>(i)] > 0) {
        skill = i;
        break;
      }
    }
    camWidth = static_cast<float>(world.terrain().width());
    camX = 0;
    terrainDirty = true;
    resultsShown = false;
    paused = false;
    // Level-start fanfare. Shell-owned rather than a World cue: restart is
    // not a simulation event, and the original plays this before anyone
    // walks out of the entrance.
    mixer.play("Letsgo");
  }

  void playMenuMusic() {
    if (mixer.has("LEMMING01"))
      mixer.playMusic("LEMMING01", 1.0f);
  }

  void playLevelMusic() {
    const int track = 1 + (playGroup * 25 + playSlot) % 13;
    const std::string cue = std::format("LEMMING{:02}", track);
    if (mixer.has(cue))
      mixer.playMusic(cue, 1.0f);
    else if (mixer.has("LEMMING01"))
      mixer.playMusic("LEMMING01", 1.0f);
  }

  bool loadLevelById(const std::string& id) {
    if (!library || id.empty())
      return false;
    levelId = id;
    if (const LevelInfo* info = levelInfo())
      levelName = info->name;
    else
      levelName = id;

    std::string path;
    for (const auto& candidate : levelFileNames(*library))
      if (candidate.find(id) != std::string::npos) {
        path = candidate;
        break;
      }
    if (path.empty()) {
      std::fprintf(stderr, "level: no .LVL matching %s\n", id.c_str());
      surfaceLevel.reset();
      return false;
    }
    auto blob = library->read(path);
    if (!blob) {
      std::fprintf(stderr, "level: read failed\n");
      return false;
    }
    auto mesh = findCylinderMesh(*blob);
    if (!mesh) {
      std::fprintf(stderr, "level: %s\n", mesh.error().c_str());
      return false;
    }
    skin = gfx::CylinderSkin{};
    if (auto r = skin.load(*library, renderer, *mesh); !r)
      std::fprintf(stderr, "skin: %s\n", r.error().c_str());

    auto foundGrid = findCollisionGrid(*blob);
    if (!foundGrid) {
      std::fprintf(stderr, "terrain: %s\n", foundGrid.error().c_str());
      surfaceLevel.reset();
      return false;
    }
    LevelObjects objects;
    if (auto found = findLevelObjects(*blob))
      objects = std::move(*found);
    LevelSkills skills{};
    const LevelSkills* skillsPtr = nullptr;
    if (auto found = findLevelSkills(*blob)) {
      skills = std::move(*found);
      skillsPtr = &skills;
    }
    surfaceLevel = makeSurfaceLevel(*foundGrid, releaseRates, releaseRate, levelInfo(), &objects,
                                    skillsPtr, mesh->height(), kPixelsPerUnit);
    restart();
    playLevelMusic();
    return true;
  }

  void finishLevel(bool won) {
    if (resultsShown)
      return;
    resultsShown = true;
    if (won && !bypassShell)
      shell.progress().complete(shell.table(), playGroup, playSlot);
    shell::ResultsInfo info;
    info.won = won;
    info.saved = world.saved();
    info.saveTarget = world.setup().saveTarget;
    info.out = world.activeCount();
    const int left = world.setup().timeLimitTicks - static_cast<int>(world.tickCount());
    info.timeLeftSec = std::max(0, left) / kTicksPerSecond;
    if (bypassShell) {
      running = false;
      return;
    }
    shell.showResults(info);
    playMenuMusic();
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

  void reloadLanguage() {
    if (!library)
      return;
    if (auto st = loadStrings(*library, shell.language())) {
      shell.setStrings(std::move(*st));
      shell.clearLanguageDirty();
    }
  }

  void handleKey(SDL_Keycode k) {
    if (inMenus()) {
      if (auto play = shell.handleKey(static_cast<int>(k))) {
        playGroup = play->group;
        playSlot = play->slot;
        if (!loadLevelById(play->levelId))
          shell.returnToMap();
      }
      if (shell.languageDirty()) {
        reloadLanguage();
        shell.applyVolumes(mixer);
      }
      if (shell.quitRequested())
        running = false;
      return;
    }

    switch (k) {
      case SDLK_ESCAPE:
        if (bypassShell)
          running = false;
        else if (!paused)
          paused = true;
        else {
          // Second Esc from pause returns to the map (stock short-Esc quirk
          // is about leaving pause; we keep a deliberate back path).
          paused = false;
          shell.returnToMap();
          playMenuMusic();
        }
        break;
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
      else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT &&
               !inMenus())
        handleClick(e.button.x, e.button.y);
    }

    int vw = 0, vh = 0;
    SDL_GetWindowSize(window, &vw, &vh);

    if (inMenus()) {
      renderer.beginFrame(vw, vh, {0.05f, 0.06f, 0.1f, 1});
      shell.draw(renderer, font, fontTex, vw, vh);
      renderer.endFrame();
      if (present)
        SDL_GL_SwapWindow(window);
      SDL_SetWindowTitle(window, "Lemmings Webolution");
      return;
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

    if (!bypassShell && !resultsShown) {
      if (world.won())
        finishLevel(true);
      else if (world.setup().timeLimitTicks > 0 &&
               static_cast<int>(world.tickCount()) >= world.setup().timeLimitTicks &&
               world.activeCount() == 0)
        finishLevel(false);
    }
    if (inMenus()) {
      // finishLevel may have switched to Results this frame.
      renderer.beginFrame(vw, vh, {0.05f, 0.06f, 0.1f, 1});
      shell.draw(renderer, font, fontTex, vw, vh);
      renderer.endFrame();
      if (present)
        SDL_GL_SwapWindow(window);
      return;
    }

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
      if (l.fuseMs > 0)
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

    // On-canvas HUD from .LTX when the shell has strings.
    {
      renderer.beginScreenUi(vw, vh);
      const float sc = vw >= 900 ? 2.0f : 1.5f;
      const auto& st = shell.strings();
      const std::size_t skillIdx =
          shell::Ltx::SkillClimber + static_cast<std::size_t>(skill);
      std::string_view skillName =
          st.size() > skillIdx ? st[skillIdx]
                               : std::string_view(sim::skillName(kSkills[static_cast<std::size_t>(skill)]));
      const int stock = world.setup().skills[static_cast<std::size_t>(skill)];
      const int timeLeft = world.setup().timeLimitTicks > 0
                               ? std::max(0, world.setup().timeLimitTicks -
                                                 static_cast<int>(world.tickCount())) /
                                     kTicksPerSecond
                               : -1;
      std::string line1 = std::format("{}  {}{}", skillName, stock < 0 ? "" : "x",
                                      stock < 0 ? "" : std::to_string(stock));
      std::string line2 =
          std::format("{} {}   {} {}/{}   {}",
                      st.size() > shell::Ltx::Out ? st[shell::Ltx::Out] : "Out",
                      world.activeCount(),
                      st.size() > shell::Ltx::Saved ? st[shell::Ltx::Saved] : "Saved",
                      world.saved(), world.setup().saveTarget,
                      levelName.empty() ? "test" : levelName);
      if (timeLeft >= 0)
        line2 += std::format("   {} {}s",
                             st.size() > shell::Ltx::TimeRemaining ? st[shell::Ltx::TimeRemaining]
                                                                  : "Time",
                             timeLeft);
      renderer.drawQuad(0, 0, static_cast<float>(vw), 36 * sc / 1.5f, {0, 0, 0, 0.55f});
      renderer.drawText(font, fontTex, 8, 4, sc, line1, {1, 1, 0.85f, 1});
      renderer.drawText(font, fontTex, 8, 4 + gfx::Font::kGlyph * sc + 2, sc * 0.85f, line2,
                        {0.85f, 0.9f, 1, 1});
      if (paused) {
        renderer.drawQuad(0, 0, static_cast<float>(vw), static_cast<float>(vh),
                          {0, 0, 0, 0.45f});
        const std::string_view pause =
            st.size() > shell::Ltx::Paused ? st[shell::Ltx::Paused] : "Paused";
        const int tw = font.measure(pause, sc * 2);
        renderer.drawText(font, fontTex, static_cast<float>(vw - tw) * 0.5f,
                          static_cast<float>(vh) * 0.4f, sc * 2, pause, {1, 1, 1, 1});
      }
    }

    renderer.endFrame();
    if (present)
      SDL_GL_SwapWindow(window);

    const std::string title = std::format(
        "Lemmings Webolution  --  {}  |  out {} saved {}/{}{}",
        levelName.empty() ? "test level" : levelName, world.activeCount(), world.saved(),
        world.setup().saveTarget, paused ? "  [paused]" : "");
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
  std::string forceLang;
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
    } else if (a == "--lang" && i + 1 < argc) {
      forceLang = argv[++i];
    } else if (a == "--level" && i + 1 < argc) {
      g_app.levelId = argv[++i];
      g_app.bypassShell = true;
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

  g_app.font = gfx::Font::make();
  g_app.fontTex = g_app.renderer.uploadImage(g_app.font.atlas);

  // The original's lemming artwork, if an installation is reachable. Not
  // having one is not an error: the game falls back to placeholder boxes and
  // still runs, which keeps the simulation testable on a bare checkout.
  {
    const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
    g_app.gameDir = dir ? fs::path(dir) : fs::path(kDefaultGameDir);
    g_app.shell.setProgressPath(g_app.gameDir / "webolution_progress.dat");
    auto lib = AssetLibrary::open(g_app.gameDir / "lemmings.box", g_app.gameDir / "patch.box");
    if (!lib) {
      std::fprintf(stderr, "sprites: %s (drawing placeholders)\n", lib.error().c_str());
    } else {
      g_app.library = std::move(*lib);
      if (auto r = g_app.sprites.load(*g_app.library, g_app.renderer); !r) {
        std::fprintf(stderr, "sprites: %s (drawing placeholders)\n", r.error().c_str());
      } else {
        std::fprintf(stderr, "sprites: loaded %zu from %s\n",
                     g_app.sprites.definitions().sprites.size(), g_app.gameDir.string().c_str());
      }

      if (auto infos = loadLevelMap(*g_app.library)) {
        g_app.levelInfos = std::move(*infos);
        g_app.shell.setLevelInfos(g_app.levelInfos);
      } else {
        std::fprintf(stderr, "level map: %s\n", infos.error().c_str());
      }

      if (auto table = loadLevelTable(*g_app.library)) {
        g_app.shell.setLevelTable(std::move(*table));
      } else {
        std::fprintf(stderr, "levels map: %s\n", table.error().c_str());
      }

      auto langs = availableLanguages(*g_app.library);
      g_app.shell.setLanguages(langs);
      std::string lang = forceLang;
      if (lang.empty()) {
        if (auto loaded = shell::loadProgress(g_app.shell.progressPath()); loaded) {
          g_app.shell.setProgress(std::move(*loaded));
          lang = g_app.shell.language();
        } else {
          // Prefer English (UK.LTX); fall back to the first available table.
          lang = "UK";
          bool have = false;
          for (const auto& l : langs)
            if (l == "UK")
              have = true;
          if (!have && !langs.empty())
            lang = langs.front();
          g_app.shell.progress().language = lang;
          g_app.shell.progress().newGame(g_app.shell.table());
        }
      } else {
        g_app.shell.progress().language = lang;
      }
      if (auto st = loadStrings(*g_app.library, lang)) {
        g_app.shell.setStrings(std::move(*st));
        std::fprintf(stderr, "language: %s (%zu strings)\n", lang.c_str(),
                     g_app.shell.strings().size());
      } else {
        std::fprintf(stderr, "language: %s\n", st.error().c_str());
      }

      if (auto rr = loadReleaseRateTable(*g_app.library)) {
        g_app.releaseRates = std::move(*rr);
      } else {
        std::fprintf(stderr, "release rates: %s\n", rr.error().c_str());
      }

      if (auto r = g_app.mixer.open(); !r) {
        std::fprintf(stderr, "audio: %s (silent)\n", r.error().c_str());
      } else {
        int loaded = 0;
        for (const char* cue : {"Letsgo", "Chink", "Ohno", "Explode", "Splat", "Yippee"}) {
          if (auto lr = g_app.mixer.loadFrom(*g_app.library, cue); !lr)
            std::fprintf(stderr, "audio: %s: %s\n", cue, lr.error().c_str());
          else
            ++loaded;
        }
        int music = 0;
        for (int i = 1; i <= 13; ++i) {
          const std::string cue = std::format("LEMMING{:02}", i);
          if (auto lr = g_app.mixer.loadMusicFrom(*g_app.library, cue); lr)
            ++music;
        }
        g_app.shell.applyVolumes(g_app.mixer);
        std::fprintf(stderr, "audio: %d sfx, %d music at %d Hz\n", loaded, music,
                     g_app.mixer.sampleRate());
      }
    }
  }

  if (g_app.bypassShell) {
    if (!g_app.levelId.empty() && g_app.library)
      g_app.loadLevelById(g_app.levelId);
    else
      g_app.restart();
    if (startCylinder)
      g_app.toggleCylinder();
  } else {
    g_app.playMenuMusic();
  }
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
