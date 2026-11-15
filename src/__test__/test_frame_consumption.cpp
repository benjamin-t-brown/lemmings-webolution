// Owned clone only: exact demand-render pixels/state and a pick following an
// unrendered frame. No original instructions or normal user save are used.
#include "FrameConsumptionAudit.h"
#include "app/RuntimePick.h"
#include "assets/Png.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <map>
#include <sstream>

using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool ok, const char* message) { if (!ok) { ++failures; std::cerr << "FAIL: " << message << '\n'; } }
void key(SDL_Scancode code) {
  SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.keysym.scancode = code;
  e.key.keysym.sym = SDL_GetKeyFromScancode(code);
  check(SDL_PushEvent(&e) == 1, "push key");
}
void click(int x, int y) {
  SDL_Event e{}; e.type = SDL_MOUSEBUTTONDOWN; e.button.button = SDL_BUTTON_LEFT;
  e.button.x = x; e.button.y = y; check(SDL_PushEvent(&e) == 1, "push click");
  e.type = SDL_MOUSEBUTTONUP; check(SDL_PushEvent(&e) == 1, "push release");
}
struct Result {
  std::string state;
  std::map<int, Image> images;
  int empty = 0, frames = 0, assigned = 0;
};
Result run(const fs::path& out, std::optional<bool> skip) {
  fs::create_directories(out);
  std::optional<app::ScopedFrameConsumption> scope;
  if (skip) scope.emplace(*skip);
  Result result;
  int target = -1, x = 0, y = 0, aimedAt = -1;
  const auto observer = [&](const app::LevelFrameView& f) {
    ++result.frames;
    result.empty += f.image.rgba.empty();
    std::ostringstream state;
    state << std::setprecision(17) << f.frame << ',' << f.world.levelClockMs() << ','
          << f.paused << ',' << f.world.spawned() << ',' << f.world.saved() << ',' << f.world.dead()
          << ',' << f.world.terrain().version() << ',' << int(f.endPhase) << ',' << f.endTimerSeconds;
    for (const auto& l : f.world.lemmings())
      state << ';' << l.x << ',' << l.y << ',' << int(l.state) << ',' << l.canClimb << ',' << l.stepBudgetSeconds;
    result.state += state.str() + '\n';
    // The selection is made only on an inspected frame. The following paused
    // frame is deliberately discarded, then its scene is needed by the click.
    const bool inspect = f.frame % 10 == 0 || f.frame == 119;
    if (inspect) {
      const auto& rendered = app::inspectLevelFrame(f);
      check(!rendered.image.rgba.empty(), "inspection renders complete image");
      result.images.emplace(f.frame, rendered.image);
      const auto& twice = app::inspectLevelFrame(f);
      check(twice.image.rgba == result.images.at(f.frame).rgba, "inspection is idempotent");
      check(writePng(out / ("frame-" + std::to_string(f.frame) + ".png"), rendered.image).has_value(), "retain pixels");
      if (target < 0) for (const auto& actor : rendered.actors) {
        const auto& l = f.world.lemmings()[actor.index];
        if (!l.active() || l.canClimb || !app::firstLevelClimberSupport(l, sim::Skill::Climber, f.world.terrain()).value_or(false)) continue;
        x = int((actor.placement.left + actor.placement.right) / 2);
        y = int((actor.placement.top + actor.placement.bottom) / 2);
        if (x < 100 || x >= 640 || y < 0 || y >= 480) continue;
        target = int(actor.index); aimedAt = f.frame; key(SDL_SCANCODE_P); break;
      }
    }
    if (target >= 0 && f.frame == aimedAt + 1) {
      check(f.paused, "discarded frame remains paused before click");
      click(x, y);
    }
    if (target >= 0 && f.frame == aimedAt + 2) {
      check(f.world.lemmings()[target].canClimb && !f.paused, "pick refresh assigns and resumes after skipped frame");
      ++result.assigned;
    }
  };
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  app::ApplicationRunOptions options;
  options.frameSeconds = 0.2;
  options.saveStore = std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out / "SavedGame.dat", game));
  std::vector<std::string> args{"frame-consumption", "--level", "MM_CLIMB_002", "--sim-ticks", "40", "--native", "--frames", "120", "--no-audio", "--game-dir", game.string(), "--window-shot", (out / "final.png").string()};
  std::vector<char*> raw; for (auto& a : args) raw.push_back(a.data());
  check(app::runApplication(int(raw.size()), raw.data(), {}, observer, options) == 0, "production route exits");
  check(result.frames == 120 && result.assigned == 1, "all frames and one public assignment observed");
  std::ofstream(out / "state.txt") << result.state;
  std::ofstream(out / "work.txt") << "frames=" << result.frames << "\nempty=" << result.empty << '\n';
  return result;
}
std::string bytes(const fs::path& path) { std::ifstream in(path, std::ios::binary); return {std::istreambuf_iterator<char>(in), {}}; }
}
int main(int argc, char** argv) {
  if (argc != 2 || !std::getenv("WEBOLUTION_GAME_DIR") || !std::getenv("SDL_VIDEODRIVER") || std::string(std::getenv("SDL_VIDEODRIVER")) != "dummy") return 2;
  const fs::path out(argv[1]);
  const auto normal = run(out / "default", std::nullopt);
  const auto demand = run(out / "demand", true);
  const auto disabled = run(out / "disabled", false);
  check(normal.empty == 0 && disabled.empty == 0, "default and explicit disabled draw every frame");
  check(demand.empty > 100, "unconsumed frames skip rasterization");
  check(normal.state == demand.state && normal.state == disabled.state, "all gameplay/frame/outcome records byte-identical");
  for (const auto& [frame, image] : normal.images) {
    check(image.rgba == demand.images.at(frame).rgba && image.rgba == disabled.images.at(frame).rgba, "every inspected image exactly identical");
  }
  check(bytes(out / "default/final.png") == bytes(out / "demand/final.png"), "window-shot forces unchanged final composite");
  check(!app::ScopedFrameConsumption::skipUnconsumed(), "automation scope restores default");
  std::ofstream(out / "summary.txt") << "failures=" << failures << '\n';
  return failures ? 1 : 0;
}
