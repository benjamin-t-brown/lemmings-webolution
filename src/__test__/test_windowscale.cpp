// Owned clone process only: public SDL input and presenter readback. No original
// execution, injected code, OS input automation, or original capture.
#define SDL_MAIN_HANDLED
#include "app/AppEntry.h"
#include "app/RuntimePick.h"
#include "app/SdlWindow.hpp"
#include "assets/Png.h"
#include "sim/World.h"
#include <SDL.h>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures = 0, scale = 1, creates = 0, presents = 0, quitAfter = 0;
SDL_Window* window = nullptr;
fs::path output;
std::ofstream audit;
std::string captureName;
Image expected;
Image lastPresented;
bool frontendReadback = false;
void check(bool b, const char* why) {
  if (!b) { ++failures; std::cerr << "FAIL: " << why << '\n'; }
}
void quit() { SDL_Event e{}; e.type = SDL_QUIT; check(SDL_PushEvent(&e) == 1, "queue quit"); }
void key(SDL_Keycode k) {
  SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.keysym.sym = k;
  e.key.keysym.scancode = SDL_GetScancodeFromKey(k);
  check(SDL_PushEvent(&e) == 1, "queue key");
}
void pointer(int x, int y, bool click = false) {
  SDL_Event e{};
  // Test the last physical pixel of each logical pixel, not just its origin.
  if (click) {
    e.type = SDL_MOUSEBUTTONDOWN; e.button.button = SDL_BUTTON_LEFT;
    e.button.x = x * scale + scale - 1; e.button.y = y * scale + scale - 1;
  } else {
    e.type = SDL_MOUSEMOTION;
    e.motion.x = x * scale + scale - 1; e.motion.y = y * scale + scale - 1;
  }
  check(SDL_PushEvent(&e) == 1, "queue physical pointer");
}
void retain(std::string name, const Image& image) {
  check(image.width == 640 && image.height == 480, "CPU image stays 640x480");
  check(writePng(output / (name + "-cpu.png"), image).has_value(), "write CPU image");
  captureName = std::move(name); expected = image;
}
Image readWindow(SDL_Renderer* r) {
  Image image; SDL_GetRendererOutputSize(r, &image.width, &image.height);
  image.rgba.resize(static_cast<std::size_t>(image.width) * image.height * 4);
  check(SDL_RenderReadPixels(r, nullptr, SDL_PIXELFORMAT_RGBA32,
        image.rgba.data(), image.width * 4) == 0, "read actual window presenter");
  return image;
}
void capture(const Image& image) {
  check(image.width == 640 * scale && image.height == 480 * scale, "whole presenter at integer size");
  std::size_t mismatches = 0;
  if (!expected.rgba.empty() && image.width == 640 * scale && image.height == 480 * scale) {
    for (int y = 0; y < image.height; ++y) for (int x = 0; x < image.width; ++x)
      for (int c = 0; c < 3; ++c)
        mismatches += image.rgba[(y * image.width + x) * 4 + c] !=
                      expected.rgba[((y / scale) * 640 + x / scale) * 4 + c];
    check(mismatches == 0, "every window RGB pixel is an exact nearest-neighbour CPU pixel");
  }
  audit << captureName << ',' << scale << ',' << image.width << ',' << image.height << ',' << mismatches << '\n';
  check(writePng(output / (captureName + "-window.png"), image).has_value(), "retain presenter capture");
  captureName.clear(); expected = {};
}
int run(std::vector<std::string> args, app::FrontendWindowObserver front = {},
        app::LevelFrameObserver level = {}, double dt = 0.0) {
  std::vector<char*> av; for (auto& a : args) av.push_back(a.data());
  app::ApplicationRunOptions options; options.frameSeconds = dt;
  // No default store: no installed or normal save is read or written.
  options.saveStore = std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(
      output / "private" / "SavedGame.dat", fs::path(std::getenv("WEBOLUTION_GAME_DIR"))));
  return app::runApplication(static_cast<int>(av.size()), av.data(), front, level, options);
}
std::vector<std::string> args() {
  return {"window-scale-test", "--game-dir", std::getenv("WEBOLUTION_GAME_DIR"), "--no-audio"};
}
std::string bytes(const fs::path& p) {
  std::ifstream f(p, std::ios::binary); return {std::istreambuf_iterator<char>(f), {}};
}
}
extern "C" SDL_Window* __real_SDL_CreateWindow(const char*, int, int, int, int, Uint32);
extern "C" void __real_SDL_RenderPresent(SDL_Renderer*);
extern "C" SDL_Window* __wrap_SDL_CreateWindow(const char* t, int x, int y, int w, int h, Uint32 flags) {
  window = __real_SDL_CreateWindow(t,x,y,w,h,flags);
  if (window) {
    ++creates;
    check(w == 640 * scale && h == 480 * scale, "requested integer window dimensions");
    int aw = 0, ah = 0; SDL_GetWindowSize(window, &aw, &ah);
    check(aw == w && ah == h, "actual client dimensions match request");
    audit << "create," << scale << ',' << w << ',' << h << '\n';
  }
  return window;
}
extern "C" void __wrap_SDL_RenderPresent(SDL_Renderer* r) {
  if (frontendReadback) lastPresented = readWindow(r);
  else if (!captureName.empty()) capture(readWindow(r));
  __real_SDL_RenderPresent(r);
  if (quitAfter > 0 && ++presents >= quitAfter) quit();
}

int main(int argc, char** argv) {
  if (argc < 3 || !std::getenv("WEBOLUTION_GAME_DIR")) return 2;
  const std::string mode = argv[1];
  const bool before = mode == "before", bench = mode == "bench1" || mode == "bench2";
  output = fs::path(argv[2]) / (mode + "-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
  fs::create_directories(output);
  std::cout << "evidence: " << output.string() << '\n';
  audit.open(output / "audit.csv");
  auto local = save::localAppDataDirectory();
  if (!local) return 2;
  const auto normal = save::cloneSavePathUnder(*local);
  const auto installed = fs::path(std::getenv("WEBOLUTION_GAME_DIR")) / "SavedGame.dat";
  const bool normalExisted = fs::exists(normal), installedExisted = fs::exists(installed);
  const auto normalBefore = bytes(normal), installedBefore = bytes(installed);
  if (bench) {
    scale = mode == "bench1" ? 1 : 2;
    const auto csv = (output / "frames.csv").string();
#ifdef _WIN32
    check(_putenv_s("WEBOLUTION_FRAME_PROFILE", csv.c_str()) == 0, "set frame profiler path");
#else
    check(setenv("WEBOLUTION_FRAME_PROFILE", csv.c_str(), 1) == 0, "set frame profiler path");
#endif
    auto a = args(); a.insert(a.end(), {"--level", "MM_CLIMB_002", "--sim-ticks", "20",
      "--frames", "240", "--window-scale", std::to_string(scale)});
    check(run(a, {}, {}, 1.0 / 60.0) == 0, "bounded installed-level frame benchmark");
  } else {
    for (int factor : {1,2,3}) {
      app::SdlWindow coordinates(factor);
      for (int i = -factor; i <= 640 * factor; ++i) {
        SDL_Event e{}; e.type = SDL_MOUSEMOTION; e.motion.x = i; e.motion.y = i;
        e.motion.xrel = 1; e.motion.yrel = -1;
        coordinates.gameCoordinates(e);
        const int logical = i < 0 ? -1 : i / factor;
        check(e.motion.x == logical && e.motion.y == logical, "pixel-cell mapping including boundaries");
        check(e.motion.xrel == 1 && e.motion.yrel == -1, "relative deltas preserved for floating-point conversion");
      }
    }
    // Real ordinary entry: no frame limit and no observer (which would itself
    // be automation). The presenter queues public SDL_QUIT after its first frame.
    scale = before ? 1 : 2; presents = 0; quitAfter = 1;
    captureName = "ordinary-default";
    check(run(args()) == 0, "ordinary startup exits via SDL_QUIT"); quitAfter = 0;
    if (!before) {
      presents = 0; quitAfter = 1; captureName = "interactive-level-default";
      auto direct = args(); direct.insert(direct.end(), {"--level", "MM_CLIMB_002", "--sim"});
      check(run(direct) == 0, "unbounded interactive level defaults to 2x"); quitAfter = 0;
    }
    for (int requested : {1,2,3}) {
      scale = before ? 1 : requested;
      const auto tag = "scale-" + std::to_string(requested);
      const int initialCreates = creates;
      int frontStage = 0, levelStage = 0, x = 0, y = 0;
      std::optional<std::size_t> hovered;
      int stock = 0, levelFrames = 0;
      auto a = args(); a.insert(a.end(), {"--frontend", "--frames", "180", "--window-scale", std::to_string(requested)});
      frontendReadback = true;
      const auto front = [&](const app::FrontendSession& s, int) {
        switch(frontStage++) {
          case 0: retain(tag + "-menu", s.composeFrame()); capture(lastPresented); key(SDLK_RETURN); break;
          case 1: check(s.screen() == shell::Screen::Confirm, "new-game confirmation"); key(SDLK_RETURN); break;
          case 2: check(s.screen() == shell::Screen::Map, "ordinary map"); pointer(206,204); break;
          case 3:
            check(s.pointerX() == 206 && s.pointerY() == 204, "map hover coordinates match 1x");
            retain(tag + "-map", s.composeFrame()); capture(lastPresented); pointer(206,204,true); break;
          default: check(false, "map click must enter installed first level"); quit(); break;
        }
      };
      const auto level = [&](const app::LevelFrameView& f) {
        frontendReadback = false;
        ++levelFrames;
        check(f.levelName == "MM_CLIMB_002", "map click selects same installed level");
        if (levelStage == 0) {
          if (f.cameraIntroActive || f.actors.empty()) return;
          bool walker = false;
          for (const auto& actor : f.actors)
            walker |= f.world.lemmings()[actor.index].state == sim::State::Walker;
          if (!walker) return;
          stock = f.world.setup().skills[0];
          key(SDLK_p); pointer(50,25,true); ++levelStage;
        } else if (levelStage == 1) {
          check(f.paused && f.cursorX == 50 && f.cursorY == 25 && f.selectedSkillIndex == 0,
                "panel click maps to same skill and logical pointer");
          bool found = false;
          for (const auto& actor : f.actors) {
            x = static_cast<int>((actor.placement.left + actor.placement.right) / 2);
            y = static_cast<int>((actor.placement.top + actor.placement.bottom) / 2);
            const auto& lem = f.world.lemmings()[actor.index];
            if (x > 100 && x < 640 && y >= 50 && y < 480 && lem.state == sim::State::Walker &&
                app::firstLevelClimberSupport(lem, sim::Skill::Climber, f.world.terrain()).value_or(false)) {
              found = true; break;
            }
          }
          check(found, "visible natural actor for hover");
          if (!found) { quit(); return; }
          pointer(x,y); ++levelStage;
        } else if (levelStage == 2) {
          check(f.cursorX == x && f.cursorY == y && f.hoveredActor.has_value(), "lemming hover maps exactly");
          hovered = f.hoveredActor;
          audit << "hover," << scale << ',' << x << ',' << y << ',' << hovered.value_or(99999) << '\n';
          retain(tag + "-hover", f.image); pointer(x,y,true); ++levelStage;
        } else {
          check(hovered && f.world.lemmings()[*hovered].canClimb && f.world.setup().skills[0] == stock - 1,
                "same logical actor click assigns Climber and consumes one skill");
          retain(tag + "-assigned", f.image); ++levelStage; quit();
        }
      };
      check(run(a, front, level, 0.2) == 0, "connected scaled route exits successfully");
      check(frontStage == 4 && levelStage == 4 && levelFrames > 0, "complete map/panel/hover/assignment route");
      check(creates == initialCreates + 1, "one shared window across frontend and level");
      // The first tutorial only stocks Climber (already selected by default).
      // Also prove an actual selection change on the installed Basher tutorial.
      int panelFrames = 0;
      auto panel = args(); panel.insert(panel.end(), {"--level", "MM_BASH_002", "--sim", "--frames", "2",
          "--window-scale", std::to_string(requested)});
      check(run(panel, {}, [&](const app::LevelFrameView& f) {
        if (panelFrames++ == 0) {
          check(f.selectedSkillIndex == 0, "initial selection before Basher click");
          pointer(50,225,true);
        } else {
          check(f.cursorX == 50 && f.cursorY == 225 && f.selectedSkillIndex == 5,
                "physical panel click changes selection to Basher at every scale");
          audit << "panel-basher," << scale << ',' << f.cursorX << ',' << f.cursorY << ',' << f.selectedSkillIndex << '\n';
          retain(tag + "-basher-panel", f.image);
        }
      }, 0.2) == 0 && panelFrames == 2, "installed Basher panel selection route");
    }
    if (!before) {
      scale = 1;
      auto a = args(); a.insert(a.end(), {"--frontend", "--frames", "1", "--window-shot", (output / "bounded-shot.png").string()});
      check(run(a) == 0, "automation stays 1x without explicit scale");
      const auto shot = bytes(output / "bounded-shot.png");
      const auto dimension = [&](int at) {
        unsigned n = 0; for (int i = at; i < at + 4; ++i) n = n * 256 + static_cast<unsigned char>(shot[i]);
        return n;
      };
      check(shot.size() >= 24 && dimension(16) == 640 && dimension(20) == 480,
            "saved screenshot remains 640x480");
      a.insert(a.end(), {"--window-scale", "2", "--window-shot", (output / "scaled-shot.png").string()});
      scale = 2;
      check(run(a) == 0 && bytes(output / "scaled-shot.png") == shot,
            "2x saved screenshot is byte-identical to unscaled automation");
      auto headless = args(); headless.insert(headless.end(), {"--frontend", "--window-scale", "3",
          "--shot", (output / "headless-shot.png").string()});
      const int n = creates;
      check(run(headless) == 0 && creates == n && bytes(output / "headless-shot.png") == shot,
            "scaled headless option creates no window and preserves screenshot bytes");
      for (const auto* invalid : {"0", "4", "-1", "2x", ""}) {
        auto bad = args(); bad.insert(bad.end(), {"--window-scale", invalid});
        const int n = creates; check(run(bad) != 0 && creates == n, "invalid scale rejected before window creation");
      }
    }
  }
  const bool normalUnchanged = fs::exists(normal) == normalExisted && bytes(normal) == normalBefore;
  const bool installedUnchanged = fs::exists(installed) == installedExisted && bytes(installed) == installedBefore;
  check(normalUnchanged, "normal save untouched");
  check(installedUnchanged, "installed save untouched");
  std::ofstream(output / "summary.txt") << "failures=" << failures << "\nnormal_save_unchanged=" << normalUnchanged
      << "\ninstalled_save_unchanged=" << installedUnchanged << '\n';
  return failures ? 1 : 0;
}
