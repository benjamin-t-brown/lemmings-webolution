// Observe the clone's public SDL input and quit path with link wrappers.
#include "app/AppEntry.h"
#include "assets/Png.h"
#include "sim/World.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;
namespace {
int creates = 0, destroys = 0;
Uint32 currentId = 0;
std::ofstream audit;
int failures = 0;
void check(bool value, const char* message) {
  if (!value) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
void key(SDL_Keycode sym) {
  SDL_Event e{}; e.type = SDL_KEYDOWN; e.key.keysym.sym = sym;
  e.key.keysym.scancode = SDL_GetScancodeFromKey(sym);
  check(SDL_PushEvent(&e) == 1, "queue key");
}
void click(int x, int y) {
  SDL_Event e{}; e.type = SDL_MOUSEBUTTONDOWN;
  e.button.button = SDL_BUTTON_LEFT; e.button.x = x; e.button.y = y;
  check(SDL_PushEvent(&e) == 1, "queue click");
}
void closeWindow() {
  SDL_Event e{}; e.type = SDL_QUIT;
  check(SDL_PushEvent(&e) == 1, "queue watchdog close");
}
std::string bytes(const fs::path& p) {
  std::ifstream in(p, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}
struct SavedFile {
  fs::path path;
  bool existed;
  std::string data;
  explicit SavedFile(fs::path p) : path(std::move(p)), existed(fs::exists(path)), data(bytes(path)) {}
  bool unchanged() const { return existed == fs::exists(path) && data == bytes(path); }
};
void frame(const fs::path& out, const char* name, const Image& image) {
  auto written = writePng(out / (std::string(name) + ".png"), image);
  check(written.has_value(), "retain clone image");
  auto* window = SDL_GetWindowFromID(currentId);
  check(window != nullptr, "window exists during every leg");
  int w = 0, h = 0;
  SDL_GetWindowSize(window, &w, &h);
  check(w == 640 && h == 480 && image.width == 640 && image.height == 480,
        "preserve frontend and level presentation size");
  audit << name << ',' << currentId << ',' << creates << ',' << destroys << ',' << w << ',' << h << '\n';
}
}

extern "C" SDL_Window* __real_SDL_CreateWindow(const char*, int, int, int, int, Uint32);
extern "C" void __real_SDL_DestroyWindow(SDL_Window*);
extern "C" SDL_Window* __wrap_SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags) {
  auto* window = __real_SDL_CreateWindow(title, x, y, w, h, flags);
  if (window) { ++creates; currentId = SDL_GetWindowID(window); audit << "create," << currentId << ',' << creates << ',' << destroys << ',' << w << ',' << h << '\n'; }
  return window;
}
extern "C" void __wrap_SDL_DestroyWindow(SDL_Window* window) {
  if (window) { ++destroys; audit << "destroy," << SDL_GetWindowID(window) << ',' << creates << ',' << destroys << ",0,0\n"; }
  __real_SDL_DestroyWindow(window);
}

int main(int argc, char** argv) {
  if (argc < 3 || !std::getenv("SDL_VIDEODRIVER") ||
      std::string(std::getenv("SDL_VIDEODRIVER")) != "dummy" || !std::getenv("WEBOLUTION_GAME_DIR")) return 2;
  const bool baseline = std::string(argv[1]) == "baseline";
  const fs::path out = fs::path(argv[2]) / ("run-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
  fs::create_directories(out);
  std::cout << "evidence: " << out.string() << '\n';
  audit.open(out / "windows.csv"); audit << "event,id,creates,destroys,width,height\n";
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  SavedFile installed(game / "SavedGame.dat");
  auto local = save::localAppDataDirectory();
  check(local.has_value(), "resolve normal save for preservation check");
  if (!local) return 2;
  SavedFile normal(save::cloneSavePathUnder(*local));
  app::ApplicationRunOptions options;
  options.frameSeconds = 10.0;
  options.saveStore = std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out / "SavedGame.dat", game));
  const auto invoke = [&](app::FrontendWindowObserver front, app::LevelFrameObserver level = {}) {
    std::vector<std::string> args{"window-quit-test", "--frontend", "--frames", "300", "--game-dir", game.string()};
    std::vector<char*> av; for (auto& a : args) av.push_back(a.data());
    return app::runApplication(static_cast<int>(av.size()), av.data(), front, level, options);
  };
  int stage = 0, levelFrames = 0;
  bool survivedQuit = false;
  const auto front = [&](const app::FrontendSession& s, int) {
    using shell::Screen;
    switch (stage) {
      case 0: check(s.screen() == Screen::Menu, "ordinary menu"); frame(out,"menu",s.composeFrame()); key(SDLK_RETURN); ++stage; break;
      case 1: check(s.screen() == Screen::Confirm, "confirmation"); key(SDLK_RETURN); ++stage; break;
      case 2: check(s.screen() == Screen::Map, "initial map"); frame(out,"map-before",s.composeFrame()); click(206,204); ++stage; break;
      case 3: check(s.screen() == Screen::Results && s.outcome() && !s.outcome()->success, "natural timeout results"); frame(out,"results",s.composeFrame()); click(320,420); ++stage; break;
      case 4: check(s.screen() == Screen::Map && !s.selectedLevel(), "results Quit returns to map"); frame(out,"map-after",s.composeFrame()); click(320,455); ++stage; break;
      case 5: check(s.screen() == Screen::Menu && s.saveCount() == 1, "return menu preserves save behavior"); frame(out,"menu-return",s.composeFrame()); click(320,450); ++stage; break;
      default: survivedQuit = true; frame(out,"quit-still-running",s.composeFrame()); closeWindow(); break;
    }
  };
  const auto level = [&](const app::LevelFrameView& f) {
    check(stage == 3 && f.levelName == "MM_CLIMB_002", "installed first level through production loader");
    if (++levelFrames == 1) frame(out,"level",f.image);
    if (levelFrames == 2) frame(out,"level-settled",f.image);
  };
  check(invoke(front,level) == 0, "round trip and mouse Quit exit status zero");
  check(stage == 6 && levelFrames > 0, "complete public-input round trip");
  check(survivedQuit == baseline, "mouse Quit terminates without a watchdog event");
  check(creates == (baseline ? 3 : 1) && destroys == creates, "single window across all legs and final cleanup");
  check(SDL_WasInit(SDL_INIT_VIDEO) == 0, "video shuts down on final application exit");
  const int roundCreates = creates, roundDestroys = destroys;
  bool sentQuit = false, keyboardSurvived = false;
  check(invoke([&](const app::FrontendSession& s, int) {
    check(s.screen() == shell::Screen::Menu, "keyboard Quit remains on menu until exit");
    if (!sentQuit) { for (int i=0; i<4; ++i) key(SDLK_DOWN); key(SDLK_RETURN); sentQuit = true; }
    else { keyboardSurvived = true; check(s.menuFocus() == 4, "keyboard selected Quit row"); closeWindow(); }
  }) == 0, "keyboard Quit exit status zero");
  check(keyboardSurvived == baseline, "keyboard Quit terminates without watchdog");
  check(creates == roundCreates + 1 && destroys == roundDestroys + 1, "keyboard route cleanup");
  check(SDL_WasInit(SDL_INIT_VIDEO) == 0, "keyboard Quit releases video");
  check(installed.unchanged() && normal.unchanged(), "installed and normal saves untouched");
  std::ofstream summary(out / "summary.txt");
  summary << "baseline=" << baseline << "\nround_creates=" << roundCreates << "\nround_destroys=" << roundDestroys
          << "\nmouse_quit_survived=" << survivedQuit << "\nkeyboard_quit_survived=" << keyboardSurvived
          << "\ninstalled_save_unchanged=" << installed.unchanged() << "\nnormal_save_unchanged=" << normal.unchanged()
          << "\nlevel_frames=" << levelFrames << "\nfailures=" << failures << '\n';
  return failures ? 1 : 0;
}
