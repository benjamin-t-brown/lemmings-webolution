// terrain-render-001: terrain a skill removes or adds reaches the level view.
//
// SCRIPTED CLONE OBSERVATION and regression test; not human play and not original-fidelity evidence.
//
// Two parts, both on installed retail levels:
//
//   scene   TubeScene on real level data, driven the way the application drives it (build from the .LVL, then
//           TubeScene::syncTerrain(world.terrain()) once per frame). A pristine terrain must leave the render
//           pixel-identical; each edit (Bomber crater, Builder slab, Digger shaft, Basher tunnel) is made through
//           the sim's own Terrain mutators, and then
//             - the drawn collision cells equal the simulation's cells (0 cells of tolerance),
//             - no rebuilt block asked for a vertex the key table lacked,
//             - the picture changed, and (almost) only inside the screen box of the changed cells.
//   app     the public webo::app::runApplication (--level L --sim --native, owned 200 ms clock). The only inputs are
//           SDL events queued from the read-only LevelFrameObserver: a plate key, a pointer motion onto a visible
//           lemming and a left click, i.e. the hoveredActor() + World::assign() path a player uses. Each skill
//           runs twice with identical inputs, once normally and once with --no-terrain-sync (the view before
//           terrain-render-001); the sim is deterministic so the two differ only in the drawn terrain. The images
//           are written under build/terrain-render-001/ for review.
//
// Usage: test_terrainrender scene <dir> | app <dir> <level> <plateIndex> <label>
#include "app/AppEntry.h"
#include "app/Camera.h"
#include "app/Raster.h"
#include "app/TubeScene.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif
#ifndef WEBOLUTION_SOURCE_DIR
#define WEBOLUTION_SOURCE_DIR "."
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

namespace {

int g_failures = 0;
void expect(bool ok, const std::string& what) {
  if (!ok) {
    std::cerr << "FAIL: terrainrender: " << what << '\n';
    ++g_failures;
  }
}

// ---------------------------------------------------------------------------------------------------------------
// pixels

std::size_t diffPixels(const Image& a, const Image& b) {
  if (a.width != b.width || a.height != b.height || a.rgba.size() != b.rgba.size()) return a.pixelCount();
  std::size_t n = 0;
  for (std::size_t i = 0; i < a.pixelCount(); ++i)
    if (a.rgba[i * 4] != b.rgba[i * 4] || a.rgba[i * 4 + 1] != b.rgba[i * 4 + 1] ||
        a.rgba[i * 4 + 2] != b.rgba[i * 4 + 2])
      ++n;
  return n;
}

Image fromFramebuffer(const app::Framebuffer& fb) {
  Image img;
  img.width = fb.width;
  img.height = fb.height;
  img.rgba.resize(static_cast<std::size_t>(fb.width) * fb.height * 4);
  for (std::size_t i = 0, n = static_cast<std::size_t>(fb.width) * fb.height; i < n; ++i) {
    img.rgba[i * 4] = fb.rgb[i * 3];
    img.rgba[i * 4 + 1] = fb.rgb[i * 3 + 1];
    img.rgba[i * 4 + 2] = fb.rgb[i * 3 + 2];
    img.rgba[i * 4 + 3] = 255;
  }
  return img;
}

// Left: a. Middle: b. Right: pixels that differ, in red over a dimmed a.
Image sideBySide(const Image& a, const Image& b) {
  Image out;
  out.width = a.width * 3;
  out.height = a.height;
  out.rgba.assign(static_cast<std::size_t>(out.width) * out.height * 4, 255);
  for (int y = 0; y < a.height; ++y)
    for (int x = 0; x < a.width; ++x) {
      const std::size_t s = (static_cast<std::size_t>(y) * a.width + x) * 4;
      const bool same = a.rgba[s] == b.rgba[s] && a.rgba[s + 1] == b.rgba[s + 1] && a.rgba[s + 2] == b.rgba[s + 2];
      for (int c = 0; c < 3; ++c) {
        out.rgba[(static_cast<std::size_t>(y) * out.width + x) * 4 + c] = a.rgba[s + c];
        out.rgba[(static_cast<std::size_t>(y) * out.width + a.width + x) * 4 + c] = b.rgba[s + c];
        out.rgba[(static_cast<std::size_t>(y) * out.width + 2 * a.width + x) * 4 + c] =
            same ? static_cast<std::uint8_t>(a.rgba[s + c] / 3) : (c == 0 ? 255 : 0);
      }
    }
  return out;
}

void save(const fs::path& path, const Image& img) {
  if (auto r = writePng(path, img); !r) std::cerr << "terrainrender: cannot write " << path << ": " << r.error() << '\n';
}

// ---------------------------------------------------------------------------------------------------------------
// scene part

app::CameraBasis facing(app::CameraParams params, int column) {
  // The pass window's centre bucket is (selectorTurns - 0.5) * 32 = yaw / 8 (tubePassBounds), i.e. the view is
  // centred on collision column `yaw`.
  params.yawUnits = static_cast<double>(((column % 256) + 256) % 256);
  return app::buildCamera(params);
}

// Screen box of a set of changed cells (column, file row), both shells, padded.
struct Box {
  double minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;
  bool valid() const { return minX <= maxX && minY <= maxY; }
  bool contains(int x, int y) const { return x >= minX && x <= maxX && y >= minY && y <= maxY; }
};

bool projectPoint(const app::CameraBasis& c, const Vec3& v, double& sx, double& sy) {
  const auto& m = c.worldToView;
  const double px = m[0] * v.x + m[3] * v.y + m[6] * v.z + m[9];
  const double py = m[1] * v.x + m[4] * v.y + m[7] * v.z + m[10];
  const double pz = m[2] * v.x + m[5] * v.y + m[8] * v.z + m[11];
  if (pz <= 0.0) return false;
  sx = c.cx + c.focalX * px / pz;
  sy = c.cy - c.focalY * py / pz;
  return true;
}

// The `w x h` terrain window with the most bashable cells (Earth or Built; steel is skipped by every remover),
// scanning columns from `from`, first maximum wins. Needs at least `minPercent` of the window.
std::optional<std::pair<int, int>> findSolidWindow(const sim::Terrain& t, int w, int h, int from, int minPercent) {
  int best = -1;
  std::pair<int, int> at{0, 0};
  for (int dx = 0; dx < t.width(); ++dx) {
    const int x0 = (from + dx) % t.width();
    for (int y0 = 8; y0 + h <= t.height() - 8; ++y0) {
      int hit = 0;
      for (int y = y0; y < y0 + h; ++y)
        for (int x = x0; x < x0 + w; ++x) {
          const sim::Material m = t.at(t.wrapX(x), y);
          if (m == sim::Material::Earth || m == sim::Material::Built) ++hit;
        }
      if (hit > best) {
        best = hit;
        at = {x0, y0};
      }
    }
  }
  if (best < 0 || best * 100 < w * h * minPercent) return std::nullopt;
  return at;
}

// Padded screen box of changed cells (column, file row) under `camera`, over both shells.
Box screenBox(const app::CameraBasis& camera, const CylinderMesh& mesh, int gridHeight,
              const std::vector<std::pair<int, int>>& cells, double pad) {
  Box box;
  for (const auto& [cx, cy] : cells)
    for (int layer = 0; layer < 2; ++layer)
      for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 2; ++j) {
          const auto v = cylinderTemplateVertex(mesh, cx + i, cy + j, layer, gridHeight);
          double sx = 0, sy = 0;
          if (v && projectPoint(camera, *v, sx, sy)) {
            box.minX = std::min(box.minX, sx);
            box.maxX = std::max(box.maxX, sx);
            box.minY = std::min(box.minY, sy);
            box.maxY = std::max(box.maxY, sy);
          }
        }
  box.minX -= pad;
  box.minY -= pad;
  box.maxX += pad;
  box.maxY += pad;
  return box;
}

std::size_t diffPixelsIn(const Image& a, const Image& b, const Box& box) {
  if (a.width != b.width || a.height != b.height) return a.pixelCount();
  std::size_t n = 0;
  for (int y = 0; y < a.height; ++y)
    for (int x = 0; x < a.width; ++x) {
      if (!box.contains(x, y)) continue;
      const std::size_t s = (static_cast<std::size_t>(y) * a.width + x) * 4;
      if (a.rgba[s] != b.rgba[s] || a.rgba[s + 1] != b.rgba[s + 1] || a.rgba[s + 2] != b.rgba[s + 2]) ++n;
    }
  return n;
}

// First `w` columns of floor (solid at row y0+h) under `h` empty rows: where a Builder slab would stand.
std::optional<std::pair<int, int>> findSlabSpot(const sim::Terrain& t, int w, int h, int from) {
  for (int dx = 0; dx < t.width(); ++dx) {
    const int x0 = (from + dx) % t.width();
    for (int y0 = 8; y0 + h < t.height() - 8; ++y0) {
      bool ok = true;
      for (int x = x0; x < x0 + w && ok; ++x) {
        ok = t.solid(t.wrapX(x), y0 + h);
        for (int y = y0; y < y0 + h && ok; ++y) ok = !t.solid(t.wrapX(x), y);
      }
      if (ok) return std::pair{x0, y0};
    }
  }
  return std::nullopt;
}

void sceneCase(const fs::path& outDir, const AssetLibrary& lib, const std::string& level, const std::string& edit,
               const app::CameraParams& cameraParams) {
  const std::string tag = level + "/" + edit;
  auto setup = sim::loadRetailLevel(lib, level);
  expect(setup.has_value(), tag + ": loadRetailLevel");
  if (!setup) return;
  sim::World world(*setup);

  std::string levelPath;
  for (const auto& name : levelFileNames(lib))
    if (name.find(level) != std::string::npos) { levelPath = name; break; }
  auto blob = lib.read(levelPath);
  expect(blob.has_value(), tag + ": read the level");
  if (!blob) return;
  auto mesh = findCylinderMesh(*blob);
  expect(mesh.has_value(), tag + ": cylinder mesh");
  if (!mesh) return;
  auto index = indexLevel(*blob);
  auto scene = app::TubeScene::build(
      lib, *mesh, *blob, index ? std::span<const std::uint32_t>(index->grid0) : std::span<const std::uint32_t>{}, true);
  expect(scene.has_value(), tag + ": build TubeScene");
  if (!scene) return;

  // The production route options (main.cpp: --geom route, no constant lattice).
  app::SceneOptions options;
  options.drawCellRoute = true;
  options.drawWall = false;
  options.drawOuter = false;
  options.drawInner = false;

  auto& terrain = world.terrain();
  const int H = terrain.height();
  const int W = terrain.width();
  expect(H == scene->collisionGrid().height && W == scene->collisionGrid().width, tag + ": terrain and grid sizes agree");

  // 1. A terrain nobody edited leaves the picture identical.
  const auto renderAt = [&](int column) {
    app::Raster raster(cameraParams.width, cameraParams.height);
    raster.clear(0, 0, 0);
    scene->render(raster, facing(cameraParams, column), options, &lib);
    return fromFramebuffer(raster.framebuffer());
  };

  // Where to edit, chosen from the live terrain (never hard-coded cells).
  int ex = 0, ey = 0, ew = 0, eh = 0;
  const int from = 64;
  if (edit == "bomb") {
    auto spot = findSolidWindow(terrain, 7, 7, from, 60);
    expect(spot.has_value(), tag + ": a 7x7 window that is at least 60% bashable exists");
    if (!spot) return;
    ex = spot->first; ey = spot->second; ew = 7; eh = 7;
  } else if (edit == "slab") {
    auto spot = findSlabSpot(terrain, 12, 3, from);
    expect(spot.has_value(), tag + ": a floor with 3 empty rows above exists");
    if (!spot) return;
    ex = spot->first; ey = spot->second; ew = 12; eh = 3;
  } else if (edit == "shaft") {
    // Diggable floors on the shelf levels are a few rows thick, so a digger's shaft through one is wide and shallow.
    auto spot = findSolidWindow(terrain, 8, 3, from, 60);
    expect(spot.has_value(), tag + ": an 8x3 window that is at least 60% bashable exists");
    if (!spot) return;
    ex = spot->first; ey = spot->second; ew = 8; eh = 3;
  } else {  // "tunnel"
    auto spot = findSolidWindow(terrain, 20, 6, from, 60);
    expect(spot.has_value(), tag + ": a 20x6 window that is at least 60% bashable exists");
    if (!spot) return;
    ex = spot->first; ey = spot->second; ew = 20; eh = 6;
  }
  const int centre = (ex + ew / 2) % W;

  const Image pristine = renderAt(centre);
  scene->syncTerrain(terrain);
  expect(scene->terrainCellsChanged() == 0 && scene->terrainBlocksRebuilt() == 0,
         tag + ": a pristine terrain changes no drawn cell (changed " + std::to_string(scene->terrainCellsChanged()) + ")");
  const Image afterPristineSync = renderAt(centre);
  expect(diffPixels(pristine, afterPristineSync) == 0, tag + ": a pristine terrain renders pixel-identically");

  // 2. The edit, through the sim's own terrain mutators.
  std::vector<std::uint8_t> before(static_cast<std::size_t>(W) * H);
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x) before[static_cast<std::size_t>(y) * W + x] = terrain.solid(x, y) ? 1 : 0;
  std::size_t edited = 0;
  if (edit == "bomb") edited = terrain.clearExplosion(ex + 3, ey + 3);
  else if (edit == "slab") edited = terrain.fillBox(ex, ey, ew, eh, sim::Material::Built);
  else edited = terrain.clearBox(ex, ey, ew, eh);
  expect(edited >= 15, tag + ": the sim edit changed at least 15 cells (changed " + std::to_string(edited) + ")");

  std::vector<std::pair<int, int>> changed;  // (column, file row)
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x)
      if ((terrain.solid(x, y) ? 1 : 0) != before[static_cast<std::size_t>(y) * W + x]) changed.push_back({x, H - 1 - y});
  expect(changed.size() == edited, tag + ": the cells that differ are the cells the mutator reported");

  scene->syncTerrain(terrain);
  expect(scene->terrainError().empty(), tag + ": sync error '" + scene->terrainError() + "'");
  expect(scene->terrainCellsChanged() == changed.size(),
         tag + ": the view followed exactly the changed cells (" + std::to_string(scene->terrainCellsChanged()) +
             " vs " + std::to_string(changed.size()) + ")");
  expect(scene->terrainBlocksRebuilt() >= 1 && scene->terrainBlocksRebuilt() <= 12,
         tag + ": only the touched blocks were regenerated (" + std::to_string(scene->terrainBlocksRebuilt()) + ")");
  expect(scene->terrainUnresolved() == 0, tag + ": no rebuilt block asked for a vertex the key table lacked");

  // What lemmings walk on is what is drawn: every grid cell agrees with the sim, none of tolerance.
  std::size_t disagree = 0;
  const auto& grid = scene->collisionGrid();
  for (int y = 0; y < H; ++y)
    for (int x = 0; x < W; ++x)
      if (grid.solid(x, y) != terrain.solid(x, H - 1 - y)) ++disagree;
  expect(disagree == 0, tag + ": drawn collision cells equal the simulation's (" + std::to_string(disagree) + " differ)");

  // A second sync with nothing changed costs nothing and changes nothing.
  const std::size_t blocksNow = scene->terrainBlocksRebuilt();
  scene->syncTerrain(terrain);
  expect(scene->terrainBlocksRebuilt() == blocksNow, tag + ": an unchanged terrain rebuilds nothing");

  // 3. The picture changed, and inside the changed cells' screen box.
  const Image edited3 = renderAt(centre);
  const app::CameraBasis camera = facing(cameraParams, centre);
  const Box box = screenBox(camera, *mesh, H, changed, 48.0);
  expect(box.valid(), tag + ": the changed cells project onto the screen");
  const std::size_t total = diffPixels(pristine, edited3);
  const std::size_t inside = diffPixelsIn(pristine, edited3, box);
  std::cout << "scene " << tag << ": window (" << ex << "," << ey << ") " << ew << "x" << eh << " cells changed "
            << changed.size() << ", blocks rebuilt " << scene->terrainBlocksRebuilt() << ", vertices added "
            << scene->terrainVerticesAdded() << ", screen box [" << box.minX << "," << box.minY << " .. " << box.maxX
            << "," << box.maxY << "] (48 px padding), differing pixels " << total << " (" << inside
            << " inside the box)\n";
  expect(total >= 50, tag + ": the picture changed (" + std::to_string(total) + " pixels)");
  expect(total > 0 && inside * 10 >= total * 9,
         tag + ": at least 90% of the changed pixels lie inside the changed cells' screen box (" + std::to_string(inside) +
             "/" + std::to_string(total) + ")");
  std::string name = level + "-" + edit;
  save(outDir / (name + "-scene.png"), sideBySide(pristine, edited3));
}

int sceneMain(const fs::path& outDir, const fs::path& gameDir) {
  auto lib = AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!lib) {
    std::cerr << "FAIL: terrainrender: open lemmings.box\n";
    return 1;
  }
  app::CameraParams params;
  std::string error;
  const fs::path cameraPath = fs::path(WEBOLUTION_SOURCE_DIR) / "tools/harness/fixtures/camera_a1/camera-fixture-params.json";
  if (!app::loadCameraParams(cameraPath, params, error)) {
    std::cerr << "FAIL: terrainrender: camera params: " << error << '\n';
    return 1;
  }
  sceneCase(outDir, *lib, "MM_BOMB", "bomb", params);
  sceneCase(outDir, *lib, "MM_BUILD", "slab", params);
  sceneCase(outDir, *lib, "MM_DIG_000", "shaft", params);
  sceneCase(outDir, *lib, "MM_BASH_002", "tunnel", params);
  if (g_failures) return 1;
  std::cout << "PASS: terrainrender scene\n";
  return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// app part

bool pushKey(int index) {
  SDL_Event e{};
  e.type = SDL_KEYDOWN;
  e.key.type = SDL_KEYDOWN;
  e.key.state = SDL_PRESSED;
  e.key.keysym.scancode = static_cast<SDL_Scancode>(SDL_SCANCODE_1 + index);
  e.key.keysym.sym = static_cast<SDL_Keycode>(SDLK_1 + index);
  return SDL_PushEvent(&e) == 1;
}
bool pushMotion(int x, int y) {
  SDL_Event e{};
  e.type = SDL_MOUSEMOTION;
  e.motion.type = SDL_MOUSEMOTION;
  e.motion.x = x;
  e.motion.y = y;
  return SDL_PushEvent(&e) == 1;
}
bool pushClick(int x, int y) {
  SDL_Event e{};
  e.type = SDL_MOUSEBUTTONDOWN;
  e.button.type = SDL_MOUSEBUTTONDOWN;
  e.button.button = SDL_BUTTON_LEFT;
  e.button.state = SDL_PRESSED;
  e.button.x = x;
  e.button.y = y;
  return SDL_PushEvent(&e) == 1;
}
// The camera grab a player uses: right button down plus a relative pointer move (main.cpp: pending.mouse, and
// CameraController::update, yaw -= 10/24 * dx while grabbed), then the button released.
bool pushGrabDrag(int x, int y, int dx) {
  SDL_Event down{};
  down.type = SDL_MOUSEBUTTONDOWN;
  down.button.type = SDL_MOUSEBUTTONDOWN;
  down.button.button = SDL_BUTTON_RIGHT;
  down.button.state = SDL_PRESSED;
  down.button.x = x;
  down.button.y = y;
  SDL_Event move{};
  move.type = SDL_MOUSEMOTION;
  move.motion.type = SDL_MOUSEMOTION;
  move.motion.x = x;
  move.motion.y = y;
  move.motion.xrel = dx;
  return SDL_PushEvent(&down) == 1 && SDL_PushEvent(&move) == 1;
}
bool pushGrabRelease(int x, int y) {
  SDL_Event up{};
  up.type = SDL_MOUSEBUTTONUP;
  up.button.type = SDL_MOUSEBUTTONUP;
  up.button.button = SDL_BUTTON_RIGHT;
  up.button.state = SDL_RELEASED;
  up.button.x = x;
  up.button.y = y;
  return SDL_PushEvent(&up) == 1;
}
bool pushQuit() {
  SDL_Event e{};
  e.type = SDL_QUIT;
  return SDL_PushEvent(&e) == 1;
}

constexpr int kFrames = 500;  // 100 s of simulation at one 200 ms tick per frame

struct AppRun {
  std::string failure;
  int frames = 0;
  bool clicked = false;
  int clickFrame = -1, editFrame = -1, endFrame = -1;
  int stockBefore = -1, stockAfter = -1;
  std::size_t solid0 = 0, solidEdit = 0, solidEnd = 0;
  std::size_t cellsChangedEnd = 0, blocksRebuiltEnd = 0;
  std::size_t cellsChangedAtClick = 0;
  Image pre, edit, end;  // clickFrame, editFrame, endFrame
  sim::Terrain terrain0;  // as loaded
  int gridHeight = 0;
  std::vector<std::pair<int, int>> changed;  // (column, file row) cells whose solidity differs at endFrame
  app::CameraParams pose;                     // the pose endFrame was drawn with
  double panDelta = 0.0;                      // yaw units the camera grab turned before endFrame
};

AppRun runApp(const std::string& level, int plate, bool wantDiggable, bool noSync, int settleFrames,
              const char* gameDir) {
  AppRun r;
  const auto observer = [&](const app::LevelFrameView& f) {
    ++r.frames;
    const int calls = r.frames;
    const auto& w = f.world;
    const auto& lemmings = w.lemmings();
    if (calls == 1) {
      r.solid0 = w.terrain().solidCells();
      r.terrain0 = w.terrain();
      r.gridHeight = w.terrain().height();
      if (!pushKey(plate)) r.failure = "could not queue the plate key";
      r.stockBefore = w.setup().skills[plate];
    }
    if (calls == 2 && f.selectedSkillIndex != plate) r.failure = "the plate was not selected";

    if (!r.clicked && calls >= 3 && !f.cameraIntroActive && !f.levelMenuOpen) {
      // The first visible walker (standing on diggable floor when the skill needs it), through the production hover.
      for (const auto& a : f.actors) {
        const auto& l = lemmings[a.index];
        if (!l.alive() || l.state != sim::State::Walker) continue;
        if (wantDiggable && !sim::isDiggable(w.terrain().at(l.x, l.y + 1))) continue;
        const int px = static_cast<int>((a.placement.left + a.placement.right) / 2);
        const int py = static_cast<int>((a.placement.top + a.placement.bottom) / 2);
        if (px < 100 || px >= 640 || py < 0 || py >= 480) continue;
        pushMotion(px, py);
        break;
      }
      if (f.hoveredActor && f.selectedSkillIndex == plate) {
        const auto& l = lemmings[*f.hoveredActor];
        const bool ok = l.alive() && l.state == sim::State::Walker &&
                        (!wantDiggable || sim::isDiggable(w.terrain().at(l.x, l.y + 1)));
        if (ok) {
          pushClick(f.cursorX, f.cursorY);
          r.clicked = true;
          r.clickFrame = calls;
          r.cellsChangedAtClick = f.drawnTerrainCellsChanged;
          r.pre = f.image;
        }
      }
    }
    if (r.clicked && r.editFrame < 0) {
      const std::size_t solid = w.terrain().solidCells();
      if (solid != r.solid0) {
        r.editFrame = calls;
        r.solidEdit = solid;
        r.edit = f.image;
        r.stockAfter = w.setup().skills[plate];
      }
    }
    // Two frames before the capture, turn the view onto what the skill changed, with the player's camera grab. The
    // pointer events are queued now, applied by the next frame, and the release by the one after, so the end frame is
    // drawn at rest. The target is the circular mean column of the cells that differ from the loaded terrain; both runs
    // (sync and no-sync) see the same terrain, so both turn by the same amount.
    if (r.editFrame >= 0 && r.endFrame < 0 && calls == r.editFrame + settleFrames - 2) {
      const auto& t = w.terrain();
      int first = -1;
      double sum = 0.0;
      int n = 0;
      for (int y = 0; y < t.height(); ++y)
        for (int x = 0; x < t.width(); ++x)
          if (r.terrain0.solid(x, y) != t.solid(x, y)) {
            if (first < 0) first = x;
            int off = (x - first) % t.width();
            if (off < -t.width() / 2) off += t.width();
            if (off >= t.width() / 2) off -= t.width();
            sum += off;
            ++n;
          }
      if (n > 0) {
        const double target = first + sum / n;
        double delta = std::fmod(target - f.displayedPose.yawUnits, 256.0);
        if (delta >= 128.0) delta -= 256.0;
        if (delta < -128.0) delta += 256.0;
        r.panDelta = delta;
        pushGrabDrag(f.cursorX, f.cursorY, static_cast<int>(std::lround(-delta * 2.4)));
      }
    }
    if (r.editFrame >= 0 && r.endFrame < 0 && calls == r.editFrame + settleFrames - 1) pushGrabRelease(f.cursorX, f.cursorY);
    if (r.editFrame >= 0 && r.endFrame < 0 && calls >= r.editFrame + settleFrames) {
      r.endFrame = calls;
      r.solidEnd = w.terrain().solidCells();
      r.cellsChangedEnd = f.drawnTerrainCellsChanged;
      r.blocksRebuiltEnd = f.drawnTerrainBlocksRebuilt;
      r.end = f.image;
      r.pose = f.displayedPose;
      const auto& t = w.terrain();
      for (int y = 0; y < t.height(); ++y)
        for (int x = 0; x < t.width(); ++x)
          if (r.terrain0.solid(x, y) != t.solid(x, y)) r.changed.push_back({x, t.height() - 1 - y});
      pushQuit();
    }
    if (calls >= kFrames - 1) pushQuit();
  };

  std::vector<std::string> args{"terrainrender", "--level", level, "--sim", "--native", "--no-audio", "--frames",
                                std::to_string(kFrames), "--game-dir", gameDir};
  if (noSync) args.push_back("--no-terrain-sync");
  std::vector<char*> raw;
  for (auto& a : args) raw.push_back(a.data());
  app::ApplicationRunOptions options;
  options.frameSeconds = 0.2;
  const int status = app::runApplication(static_cast<int>(raw.size()), raw.data(), {}, observer, options);
  if (status != 0 && r.failure.empty()) r.failure = "runApplication returned failure";
  return r;
}

int appMain(const fs::path& outDir, const fs::path& gameDir, const std::string& level, int plate,
            const std::string& label) {
  const bool digger = plate == 7;
  const int settle = label == "bomb" ? 12 : (label == "build" ? 60 : 40);
  const std::string gd = gameDir.string();
  const AppRun sync = runApp(level, plate, digger, false, settle, gd.c_str());
  const AppRun plain = runApp(level, plate, digger, true, settle, gd.c_str());

  const auto report = [&](const char* name, const AppRun& r) {
    std::cout << "app " << level << '/' << label << ' ' << name << ": frames " << r.frames << " click@" << r.clickFrame
              << " edit@" << r.editFrame << " end@" << r.endFrame << " stock " << r.stockBefore << "->" << r.stockAfter
              << " solid " << r.solid0 << "->" << r.solidEdit << "->" << r.solidEnd << " drawn-cells-changed "
              << r.cellsChangedEnd << " blocks " << r.blocksRebuiltEnd << (r.failure.empty() ? "" : " FAILURE ") << r.failure
              << '\n';
  };
  report("sync", sync);
  report("nosync", plain);

  expect(sync.failure.empty(), label + ": sync run: " + sync.failure);
  expect(plain.failure.empty(), label + ": no-sync run: " + plain.failure);
  expect(sync.clicked && plain.clicked, label + ": a public click assigned the skill in both runs");
  expect(sync.stockAfter == sync.stockBefore - 1, label + ": the skill count dropped by one");
  expect(sync.editFrame >= 0 && sync.endFrame >= 0, label + ": the skill edited the simulation terrain");
  // The sim is deterministic: both runs click and edit on the same frames and end with the same terrain.
  expect(sync.clickFrame == plain.clickFrame && sync.editFrame == plain.editFrame && sync.endFrame == plain.endFrame &&
             sync.solidEnd == plain.solidEnd,
         label + ": the two runs saw the same simulation");
  if (sync.endFrame < 0 || plain.endFrame < 0) return 1;

  expect(plain.cellsChangedEnd == 0, label + ": the no-sync run never followed an edit");
  expect(sync.cellsChangedEnd > 0 && sync.blocksRebuiltEnd > 0, label + ": the view followed the edit");
  const std::size_t net = sync.solidEnd > sync.solid0 ? sync.solidEnd - sync.solid0 : sync.solid0 - sync.solidEnd;
  expect(sync.cellsChangedEnd >= net, label + ": the view followed at least every net changed cell");

  // Pixels. Before the edit the two runs draw the same terrain; their difference there is the baseline (scenery that
  // reads the wall clock can differ, so it is measured and subtracted rather than assumed zero).
  const std::size_t basePre = diffPixels(sync.pre, plain.pre);
  const std::size_t diffEnd = diffPixels(sync.end, plain.end);
  const std::size_t diffEdit = diffPixels(sync.edit, plain.edit);
  std::cout << "app " << level << '/' << label << ": whole-frame differing pixels sync vs no-sync: before the edit "
            << basePre << ", at the first edited frame " << diffEdit << ", " << settle << " frames later " << diffEnd
            << " (scenery that reads the wall clock makes the baseline non-zero)\n";

  // The same comparison restricted to the screen box of the cells the skill changed (both shells, 40 px padding),
  // projected with the pose the end frame was drawn with.
  auto lib = AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  expect(lib.has_value(), label + ": open lemmings.box");
  if (!lib) return 1;
  std::string levelPath;
  for (const auto& name : levelFileNames(*lib))
    if (name.find(level) != std::string::npos) { levelPath = name; break; }
  auto blob = lib->read(levelPath);
  expect(blob.has_value(), label + ": read the level");
  if (!blob) return 1;
  auto mesh = findCylinderMesh(*blob);
  expect(mesh.has_value(), label + ": cylinder mesh");
  if (!mesh) return 1;
  const Box box = screenBox(app::buildCamera(sync.pose), *mesh, sync.gridHeight, sync.changed, 40.0);
  expect(box.valid() && !sync.changed.empty(), label + ": the changed cells project onto the screen");
  const std::size_t regionPre = diffPixelsIn(sync.pre, plain.pre, box);
  const std::size_t regionEnd = diffPixelsIn(sync.end, plain.end, box);
  std::cout << "app " << level << '/' << label << ": " << sync.changed.size() << " cells changed, screen box ["
            << box.minX << "," << box.minY << " .. " << box.maxX << "," << box.maxY << "], differing pixels inside it: "
            << "before the edit " << regionPre << ", after " << regionEnd << " (camera turned " << sync.panDelta
            << " yaw units by the grab; no-sync run " << plain.panDelta << ")\n";
  expect(sync.panDelta == plain.panDelta, label + ": both runs turned the camera by the same amount");
  expect(regionEnd >= regionPre + 50,
         label + ": inside the changed cells' screen box the synced view differs from the unsynced view after the edit "
                 "(before " + std::to_string(regionPre) + ", after " + std::to_string(regionEnd) + ")");
  // The unsynced view is the old behaviour: terrain unchanged after the edit. Its own pre/end difference is only
  // what moved (lemmings, effects).
  save(outDir / (label + "-1-before-click.png"), sync.pre);
  save(outDir / (label + "-2-after-sync.png"), sync.end);
  save(outDir / (label + "-3-after-nosync.png"), plain.end);
  save(outDir / (label + "-4-sync-vs-nosync.png"), sideBySide(plain.end, sync.end));
  if (g_failures) return 1;
  std::cout << "PASS: terrainrender app " << label << '\n';
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  const char* driver = std::getenv("SDL_VIDEODRIVER");
  if (!driver || std::string(driver) != "dummy") {
    std::cerr << "terrainrender: refusing to run without SDL_VIDEODRIVER=dummy\n";
    return 1;
  }
  const char* gameDirEnv = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDirEnv || !*gameDirEnv || !fs::is_regular_file(fs::path(gameDirEnv) / "lemmings.box")) {
    std::cout << "terrainrender: SKIPPED (retail assets unavailable)\n";
    return 0;
  }
  const std::string mode = argc > 1 ? argv[1] : "scene";
  const fs::path outDir = argc > 2 ? fs::path(argv[2]) : fs::path("build/terrain-render-001");
  std::error_code ec;
  fs::create_directories(outDir, ec);
  if (mode == "scene") return sceneMain(outDir, gameDirEnv);
  if (mode == "app" && argc > 5) return appMain(outDir, gameDirEnv, argv[3], std::atoi(argv[4]), argv[5]);
  std::cerr << "usage: test_terrainrender scene <dir> | app <dir> <level> <plate> <label>\n";
  return 1;
}
#else
#include <iostream>
int main() {
  std::cerr << "terrainrender: requires SDL2\n";
  return 1;
}
#endif
