// Lemmings Webolution.
//
// Runs the simulation and draws it. The level is a built-in test level for
// now: .LVL is still being reverse engineered (see PLAN.md 1.6), so there is
// no real level to load yet, but everything downstream of it is real.
//
//   left click      assign the selected skill to the lemming under the cursor
//   1..8            choose a skill
//   arrow keys / A D  pan
//   space           pause
//   .               single step while paused
//   N               nuke
//   R               restart
//   Esc             quit

#include "assets/Bitmap.h"
#include "assets/Png.h"
#include "gfx/Gl.h"
#include "gfx/Renderer.h"
#include "sim/World.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <format>
#include <string>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

namespace {

using namespace webo;

constexpr int kTicksPerSecond = 20;

// A hand-built level, until .LVL gives us real ones: a floor, some pillars to
// climb and bash, a gap to build over, and a pit at the seam so the
// wraparound gets exercised by just playing.
sim::LevelSetup makeTestLevel() {
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
  s.totalLemmings = 20;
  s.saveTarget = 10;
  s.releaseIntervalTicks = 24;
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
  sim::World world;

  bool running = true;
  bool paused = false;
  bool stepOnce = false;
  int skill = 7;  // digger
  float camX = 0;
  float camWidth = 320;
  std::uint64_t lastTick = 0;
  bool terrainDirty = true;

  void restart() {
    world = sim::World(makeTestLevel());
    terrainDirty = true;
  }

  void handleKey(SDL_Keycode k) {
    switch (k) {
      case SDLK_ESCAPE: running = false; break;
      case SDLK_SPACE: paused = !paused; break;
      case SDLK_PERIOD: stepOnce = true; break;
      case SDLK_n: world.nuke(); break;
      case SDLK_r: restart(); break;
      case SDLK_LEFT: case SDLK_a: camX -= 16; break;
      case SDLK_RIGHT: case SDLK_d: camX += 16; break;
      default:
        if (k >= SDLK_1 && k <= SDLK_8)
          skill = static_cast<int>(k - SDLK_1);
        break;
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
      if (world.assign(*hit, kSkills[static_cast<std::size_t>(skill)]))
        terrainDirty = true;
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
      if (due)
        lastTick = now;
      stepOnce = false;
      terrainDirty = true;  // skills edit terrain; cheap enough at this size
    }

    int vw = 0, vh = 0;
    SDL_GetWindowSize(window, &vw, &vh);
    renderer.beginFrame(vw, vh, {0.06f, 0.07f, 0.12f, 1});
    renderer.setCamera(camX, 0, camWidth);

    if (terrainDirty) {
      renderer.uploadTerrain(world.terrain());
      terrainDirty = false;
    }
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

      const float x = static_cast<float>(l.x) - 2;
      const float y = static_cast<float>(l.y) - 8;
      renderer.drawQuad(x, y, 4, 8, c);
      // A tuft so facing is visible at this size.
      renderer.drawQuad(x + (l.facing == sim::Facing::Right ? 3 : -1), y, 2, 2,
                        {0.3f, 0.9f, 0.3f, 1});
    }

    // Exits and entrances, so the goal is obvious.
    for (const auto& ex : world.setup().exits)
      renderer.drawQuad(static_cast<float>(ex.x), static_cast<float>(ex.y),
                        static_cast<float>(ex.width), static_cast<float>(ex.height),
                        {0.2f, 0.9f, 0.9f, 0.55f});
    for (const auto& en : world.setup().entrances)
      renderer.drawQuad(static_cast<float>(en.x) - 4, static_cast<float>(en.y) - 6, 8, 6,
                        {0.9f, 0.9f, 0.2f, 0.55f});

    renderer.endFrame();
    if (present)
      SDL_GL_SwapWindow(window);

    const std::string title = std::format(
        "Lemmings Webolution  --  {}  |  out {} saved {}/{} dead {}  |  tick {}{}",
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
    }
  }

  // A headless mode so the simulation can be exercised in CI, and so that
  // "does it run" is answerable without a display.
  if (headless) {
    sim::World w(makeTestLevel());
    for (int i = 0; i < headlessTicks; ++i)
      w.tick();
    std::printf("ticks=%llu active=%d saved=%d dead=%d solid=%zu\n",
                static_cast<unsigned long long>(w.tickCount()), w.activeCount(), w.saved(),
                w.dead(), w.terrain().solidCells());
    return 0;
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
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

  g_app.window = SDL_CreateWindow("Lemmings Webolution", SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, 1280, 800,
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

  g_app.restart();
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

  SDL_GL_DeleteContext(g_app.context);
  SDL_DestroyWindow(g_app.window);
  SDL_Quit();
  return 0;
}
