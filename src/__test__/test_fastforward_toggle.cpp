// Public input into the source-built application on installed MM_CLIMB_002.
// Owned clock and read-only frame observer; no original execution or World mutation.
#include "app/AppEntry.h"
#include "app/RuntimeSkillPanel.h"
#include "assets/Box.h"
#include "assets/Png.h"
#include "sim/World.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool ok, const char* message) {
  if (!ok) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
void push(SDL_Event e) { check(SDL_PushEvent(&e) == 1, "enqueue public SDL event"); }
void button(Uint32 type, Uint8 which, int x = 50, int y = 348) {
  SDL_Event e{}; e.type = type; e.button.button = which;
  e.button.state = type == SDL_MOUSEBUTTONDOWN ? SDL_PRESSED : SDL_RELEASED;
  e.button.x = x; e.button.y = y; push(e);
}
void motion(Uint32 held = 0, int x = 320, int y = 240) {
  SDL_Event e{}; e.type = SDL_MOUSEMOTION; e.motion.state = held;
  e.motion.x = x; e.motion.y = y; push(e);
}
void click(int x = 50, int y = 348) {
  button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT, x, y);
  button(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT, x, y);
  motion();  // cursor outside the plate when checking its artwork
}
void pause() {
  SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.keysym.sym = SDLK_p;
  e.key.keysym.scancode = SDL_SCANCODE_P; push(e);
}
void focus(Uint8 kind) { SDL_Event e{}; e.type = SDL_WINDOWEVENT; e.window.event = kind; push(e); }
void quit() { SDL_Event e{}; e.type = SDL_QUIT; push(e); }
std::string bytes(const fs::path& p) {
  std::ifstream in(p, std::ios::binary); return {std::istreambuf_iterator<char>(in), {}};
}
struct SavedFile {
  fs::path path; bool existed; std::string data;
  explicit SavedFile(fs::path p) : path(std::move(p)), existed(fs::exists(path)), data(bytes(path)) {}
  bool unchanged() const { return existed == fs::exists(path) && data == bytes(path); }
};
Image canvas(unsigned char shade) {
  Image im; im.width = 640; im.height = 480; im.rgba.assign(640 * 480 * 4, shade);
  for (std::size_t i = 3; i < im.rgba.size(); i += 4) im.rgba[i] = 255;
  return im;
}
}

int main(int argc, char** argv) {
  if (argc != 3 || !std::getenv("WEBOLUTION_GAME_DIR") || !std::getenv("SDL_VIDEODRIVER") ||
      std::string(std::getenv("SDL_VIDEODRIVER")) != "dummy") return 2;
  const bool baseline = std::string(argv[1]) == "baseline";
  const fs::path out = fs::path(argv[2]) / ("run-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
  fs::create_directories(out);
  std::cout << "evidence: " << out.string() << '\n';
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  SavedFile installed(game / "SavedGame.dat");
  auto local = save::localAppDataDirectory();
  check(local.has_value(), "resolve normal clone save for preservation check");
  if (!local) return 2;
  SavedFile normal(save::cloneSavePathUnder(*local));
  auto library = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  check(library.has_value(), "open installed assets read-only");
  if (!library) return 2;
  auto panel = app::RetailSkillPanel::create(*library, "MM_CLIMB_002");
  check(panel.has_value(), "load installed plate artwork");
  if (!panel) return 2;

  // Independent artwork references use the pre-existing three-argument draw.
  // Render over both black and white to bound every possible scene background
  // behind this translucent artwork. Choose pixels with disjoint normal/active
  // RGB intervals, so the scene cannot masquerade as the other plate state.
  auto off = canvas(0), on = canvas(0), offWhite = canvas(255), onWhite = canvas(255);
  panel->draw(off, panel->initialCounts(), 0); panel->draw(on, panel->initialCounts(), 8);
  panel->draw(offWhite, panel->initialCounts(), 0); panel->draw(onWhite, panel->initialCounts(), 8);
  std::vector<std::size_t> mask;
  for (int y = 332; y < 364; ++y) for (int x = 18; x < 90; ++x) {
    const auto i = (static_cast<std::size_t>(y) * 640 + x) * 4;
    bool distinguishable = false;
    for (int c = 0; c < 3; ++c)
      if (offWhite.rgba[i+c] + 2 < on.rgba[i+c] || onWhite.rgba[i+c] + 2 < off.rgba[i+c]) distinguishable = true;
    if (distinguishable) mask.push_back(i);
  }
  check(mask.size() > 100, "plate has substantial unambiguous state-dependent artwork");
  const auto plateMatches = [&](const Image& actual, bool active) {
    const auto& low = active ? on : off;
    const auto& high = active ? onWhite : offWhite;
    for (const auto i : mask) for (int c = 0; c < 3; ++c)
      if (actual.rgba[i+c] + 1 < low.rgba[i+c] || actual.rgba[i+c] > high.rgba[i+c] + 1) return false;
    return true;
  };
  std::ofstream trace(out / "trace.csv");
  trace << "leg,frame,clock_ms,delta_ms,inferred_passes,explicit_updates,paused,expected_fast,drawn_fast,plate_pixels_ok,stock_ok,selection\n";
  app::ApplicationRunOptions options; options.frameSeconds = 0.05;
  options.saveStore = std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out / "SavedGame.dat", game));
  const auto invoke = [&](app::LevelFrameObserver observer) {
    std::vector<std::string> args{"fastforward-toggle-test", "--level", "MM_CLIMB_002", "--sim", "--frames", "30", "--game-dir", game.string()};
    std::vector<char*> raw; for (auto& a : args) raw.push_back(a.data());
    return app::runApplication(static_cast<int>(raw.size()), raw.data(), {}, observer, options);
  };
  // An action queued at frame f is consumed before frame f+1's simulation.
  const std::array<bool, 17> fast{false,true,true,true,false,false,true,true,false,false,true,true,true,true,true,true,false};
  std::uint64_t priorMs = 0, priorUpdates = 0;
  std::array<int,8> stock{};
  int seen = 0;
  const int result = invoke([&](const app::LevelFrameView& f) {
    const int n = f.frame; ++seen;
    check(n < static_cast<int>(fast.size()), "observer exits at bounded frame");
    if (n >= static_cast<int>(fast.size())) { quit(); return; }
    const bool expectedFast = baseline ? n > 0 : fast[n];
    const bool expectedPause = n == 7 || n == 8;
    const int passes = expectedPause ? 0 : expectedFast ? 7 : 1;
    const auto ms = f.world.levelClockMs(), updates = f.world.explicitUpdateCount();
    check(ms - priorMs == static_cast<unsigned>(passes * 50), "same-delta pass budget follows each input change");
    // Each due 1000 ms event adds a zero-dt trailing update on this aligned
    // 50 ms clock. Passes and World update calls are distinct (shared timing).
    const auto eventUpdates = ms / 1000 - priorMs / 1000;
    check(updates - priorUpdates == static_cast<unsigned>(passes) + eventUpdates, "World updates include passes plus level-event trailing updates");
    check(f.paused == expectedPause, "pause state follows input");
    check(f.levelName == "MM_CLIMB_002", "production loader entered installed tutorial");
    if (n == 0) stock = f.world.setup().skills;
    const bool stockOk = stock == f.world.setup().skills;
    check(stockOk && f.selectedSkillIndex == 0, "speed commands preserve stock and actor-skill selection");
    const bool drawnFast = plateMatches(f.image, true);
    const bool drawnNormal = plateMatches(f.image, false);
    const bool pixelsOk = drawnFast != drawnNormal && drawnFast == (!baseline && expectedFast);
    check(pixelsOk, "drawn plate matches the actual speed mode");
    trace << "toggle," << n << ',' << ms << ',' << ms-priorMs << ',' << (ms-priorMs)/50 << ',' << updates-priorUpdates
          << ',' << f.paused << ',' << expectedFast << ',' << drawnFast << ',' << pixelsOk << ',' << stockOk << ',' << f.selectedSkillIndex << '\n';
    priorMs = ms; priorUpdates = updates;
    if (n == 0 || n == 1 || n == 4 || n == 6 || n == 8 || n == 10 || n == 16)
      check(writePng(out / ("frame-" + std::to_string(n) + ".png"), f.image).has_value(), "retain clone framebuffer image");
    switch (n) {
      case 0: button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT); motion(SDL_BUTTON_LMASK); break;
      case 1: motion(SDL_BUTTON_LMASK, 51,349); motion(SDL_BUTTON_LMASK, 49,347); motion(SDL_BUTTON_LMASK); break;
      case 2: button(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT); motion(); break;
      case 3: button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT); motion(SDL_BUTTON_LMASK,51,349); motion(SDL_BUTTON_LMASK); break;
      case 4: button(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT); motion(0,50,348); motion(); break;
      case 5: click(); break;
      case 6: pause(); break;
      case 7: click(); break;  // cancel even while paused
      case 8: pause(); break;
      case 9: button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT,5,5); motion(SDL_BUTTON_LMASK,50,348); motion(SDL_BUTTON_LMASK); break;
      case 10: motion(SDL_BUTTON_LMASK,5,5); motion(SDL_BUTTON_LMASK,50,348); motion(SDL_BUTTON_LMASK); break;
      case 11: button(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT); motion(); break;
      case 12: click(50,28); break;  // another plate does not clear the mode
      case 13: button(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT,320,240); click(); break;
      case 14: button(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT,320,240); focus(SDL_WINDOWEVENT_FOCUS_LOST); click(); focus(SDL_WINDOWEVENT_FOCUS_GAINED); break;
      case 15: click(); break;
      default: quit(); break;
    }
  });
  check(result == 0 && seen == 17, "complete bounded on/off/on production route");
  // A second level leg owns a fresh mode and normal plate, never the prior leg's speed.
  seen = 0;
  check(invoke([&](const app::LevelFrameView& f) {
    ++seen; check(f.world.levelClockMs() == 50 && f.world.explicitUpdateCount() == 1, "reload starts at normal speed");
    check(plateMatches(f.image, false), "reload plate is inactive");
    trace << "reload,0," << f.world.levelClockMs() << ",50,1,1,0,0,0,1,1,0\n"; quit();
  }) == 0 && seen == 1, "fresh application leg exits successfully");
  check(installed.unchanged(), "installed save existence and bytes unchanged");
  check(normal.unchanged(), "normal user save existence and bytes unchanged");
  std::ofstream summary(out / "summary.txt");
  summary << "side=" << (baseline ? "baseline" : "production") << "\ndistinguishable_plate_pixels=" << mask.size()
          << "\ninstalled_save_unchanged=" << installed.unchanged() << "\nnormal_save_unchanged=" << normal.unchanged()
          << "\nfailures=" << failures << '\n';
  return failures ? 1 : 0;
}
