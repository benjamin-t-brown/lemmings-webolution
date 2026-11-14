// P14: public SDL motion through runApplication, using installed map/title/font
// assets. The all-progress save is a fixture, not evidence of natural victories.
#include "app/AppEntry.h"
#include "assets/GameData.h"
#include "assets/Lrz.h"
#include "assets/Png.h"
#include "shell/MenuRender.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;
namespace {
int failures = 0;
void check(bool ok, const std::string& message) {
  if (!ok) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
std::string bytes(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}
struct SavedFile {
  fs::path path; bool existed; std::string data;
  explicit SavedFile(fs::path p) : path(std::move(p)), existed(fs::exists(path)), data(bytes(path)) {}
  bool unchanged() const { return existed == fs::exists(path) && data == bytes(path); }
};
void event(Uint32 type, int x = 0, int y = 0, SDL_Keycode key = SDLK_RETURN) {
  SDL_Event e{}; e.type = type;
  if (type == SDL_KEYDOWN) e.key.keysym.sym = key;
  if (type == SDL_MOUSEMOTION) { e.motion.x = x; e.motion.y = y; }
  if (type == SDL_MOUSEBUTTONDOWN) { e.button.x = x; e.button.y = y; e.button.button = SDL_BUTTON_LEFT; }
  if (type == SDL_WINDOWEVENT) e.window.event = static_cast<Uint8>(x);
  check(SDL_PushEvent(&e) == 1, "queue public SDL event");
}
// Independent single-line reference: literal glyph alpha and integer blending,
// deliberately not calling drawHud or composeFrame to build the title oracle.
void text(Image& image, const Image& font, std::string_view title, int x, int y,
          const std::array<int, 3>& colour) {
  for (unsigned char ch : title) {
    check(ch >= 32 && ch <= 126 && ch != '^', "installed title fits recovered ASCII single-line font");
    if (ch < 32 || ch > 126) continue;
    const auto& cell = shell::fontCells()[ch - 32];
    for (int yy = 0; yy < shell::kLineHeight; ++yy) for (int xx = 0; xx < cell.width; ++xx) {
      const int dx = x + xx, dy = y + yy;
      if (dx < 0 || dx >= image.width || dy < 0 || dy >= image.height) continue;
      const auto src = (static_cast<std::size_t>(cell.y + yy) * font.width + cell.x + xx) * 4;
      const auto dst = (static_cast<std::size_t>(dy) * image.width + dx) * 4;
      const int alpha = font.rgba[src + 3];
      for (int c = 0; c < 3; ++c)
        image.rgba[dst + c] = static_cast<std::uint8_t>(
            (image.rgba[dst + c] * (255 - alpha) + colour[c] * alpha + 127) / 255);
    }
    x += cell.width;
  }
}
std::uint64_t hash(const Image& image) {
  std::uint64_t h = 14695981039346656037ull;
  for (auto v : image.rgba) { h ^= v; h *= 1099511628211ull; }
  return h;
}
std::string csv(std::string_view value) {
  std::string quoted = "\"";
  for (char c : value) { if (c == '"') quoted += '"'; quoted += c; }
  return quoted + '"';
}
struct Point { int x = 0, y = 0; };
struct Visit { std::string label; Point point; int node = -1; };
}

int main(int argc, char** argv) {
  if (argc != 3 || !std::getenv("SDL_VIDEODRIVER") ||
      std::string(std::getenv("SDL_VIDEODRIVER")) != "dummy" || !std::getenv("WEBOLUTION_GAME_DIR")) return 2;
  const bool before = std::string(argv[1]) == "baseline";
  const fs::path out = fs::path(argv[2]) / ("run-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()));
  fs::create_directories(out);
  std::cout << "evidence: " << out.string() << '\n';
  const fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  SavedFile installed(game / "SavedGame.dat");
  auto local = save::localAppDataDirectory();
  if (!local) { std::cerr << local.error(); return 2; }
  SavedFile normal(save::cloneSavePathUnder(*local));
  auto lib = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!lib) { std::cerr << lib.error(); return 2; }
  auto infos = loadLevelMap(*lib);
  auto table = loadLevelTable(*lib);
  auto font = loadTexture(*lib, "TEXTURES\\^FONT.BMP");
  auto cursor = app::RuntimeCursor::load(*lib);
  auto packed = lib->read("TEXTURES\\LEVEL MASK.BMP");
  if (!infos || !table || !font || !cursor || !packed) return 2;
  auto raw = lrzDecode(*packed);
  if (!raw || raw->size() < 308278) return 2;
  const auto u32 = [&](std::size_t p) {
    unsigned n = 0; for (int i = 0; i < 4; ++i) n |= std::to_integer<unsigned>((*raw)[p + i]) << (8 * i); return n;
  };
  check(u32(18) == 640 && u32(22) == 480, "native indexed map dimensions");
  const auto offset = u32(10);
  if (offset + 640u * 480u > raw->size()) return 2;
  std::array<Point, 102> points{};
  std::array<int, 102> counts{};
  for (int y = 0; y < 480; ++y) for (int x = 0; x < 640; ++x) {
    const auto n = std::to_integer<unsigned>((*raw)[offset + (479 - y) * 640 + x]);
    if (n < 102) { points[n].x += x; points[n].y += y; ++counts[n]; }
  }
  std::array<std::string, 102> titles{}, ids{};
  std::ofstream audit(out / "frames.csv");
  audit << "route,frame,node,x,y,title,base_hash,composed_hash\n";
  for (std::size_t n = 0; n < 102; ++n) {
    if (!counts[n]) return 2;
    points[n].x /= counts[n]; points[n].y /= counts[n];
    check(std::to_integer<unsigned>((*raw)[offset + (479 - points[n].y) * 640 + points[n].x]) == n,
          "mask centroid is inside node " + std::to_string(n));
    const auto& key = app::MapScene::nodeMapping()[n];
    ids[n] = table->groups[key[0]][key[1]].id;
    for (const auto& info : *infos)
      if (normaliseAssetName(info.id) == normaliseAssetName(ids[n])) titles[n] = info.name;
    check(!titles[n].empty() && titles[n] != ids[n], "real display title for node " + std::to_string(n));
    check(shell::textWidth(titles[n], shell::fontCells()) <= 608, "installed title fits native right edge");
  }
  check(titles[0] == "Just Climb", "first level's known title");
  int visited = 0, levelFrames = 0;
  for (bool all : {false, true}) {
    const std::string route = all ? "all-progress" : "fresh";
    const auto savePath = out / (route + "-SavedGame.dat");
    auto store = std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(savePath, game));
    save::SavedGame fixture;
    if (all) {
      fixture.flagRaw = 37;
      fixture.records[0].a = 123; fixture.records[0].b = -456;
      for (const auto& key : app::MapScene::nodeMapping()) fixture.markComplete(key[0], key[1]);
      check(store->save(fixture).has_value(), "write private progress fixture");
    }
    std::vector<Visit> visits{{"off", {320, 240}, -1}};
    const int limit = all ? 102 : 3;
    for (int n = 0; n < limit; ++n) {
      visits.push_back({"node-" + std::to_string(n), points[n], n});
      visits.push_back({"off-after-" + std::to_string(n), {320, 240}, -1});
    }
    if (!all) visits.push_back({"locked-node-3", points[3], -1});
    visits.push_back({"footer", {320, 455}, -1});
    visits.push_back({"edge", {640, 480}, -1});
    visits.push_back({"node-again", points[0], 0});
    visits.push_back({"unfocused", points[0], -1});
    visits.push_back({"refocused", points[0], 0});
    int stage = 0, index = -1;
    std::array<std::int32_t, 102> flags{}, states{};
    Image base;
    std::string privateBytes;
    app::ApplicationRunOptions options;
    options.saveStore = store;
    std::vector<std::string> args{"map-hover-test", "--frontend", "--frames", "300", "--game-dir", game.string()};
    std::vector<char*> av; for (auto& a : args) av.push_back(a.data());
    const auto front = [&](const app::FrontendSession& s, int) {
      if (stage == 0) {
        check(s.screen() == shell::Screen::Menu, "ordinary application menu");
        if (all) event(SDL_KEYDOWN, 0, 0, SDLK_DOWN);
        event(SDL_KEYDOWN); ++stage; return;
      }
      if (stage == 1 && !all) {
        check(s.screen() == shell::Screen::Confirm, "ordinary New Game confirmation");
        event(SDL_KEYDOWN); ++stage; return;
      }
      stage = 2;
      check(s.screen() == shell::Screen::Map, "map remains active during hover");
      if (s.screen() != shell::Screen::Map) { event(SDL_QUIT); return; }
      if (index < 0) {
        flags = s.completionFlags(); states = s.mapStates(); base = s.image(); privateBytes = bytes(savePath);
        check(flags == fixture.nodeFlags(app::MapScene::nodeMapping()), "loaded fixture progress");
      } else {
        const auto& v = visits[index];
        check(s.pointerX() == v.point.x && s.pointerY() == v.point.y, "public motion reaches session");
        check(s.image().rgba == base.rgba && s.mapStates() == states && s.completionFlags() == flags &&
              !s.selectedLevel() && !s.outcome() && s.saveCount() == (all ? 0 : 1),
              "hover preserves cached map, selection, unlocks and save count");
        Image expected = base;
        if (v.node >= 0 && !before) {
          text(expected, *font, titles[v.node], 30, 20, {0, 0, 0});
          text(expected, *font, titles[v.node], 32, 22, {220, 50, 50});
          check(expected.rgba != base.rgba, "hover adds visible title pixels");
        }
        cursor->draw(expected, s.cursorState(), s.pointerX(), s.pointerY());
        const auto frame = s.composeFrame();
        check(frame.rgba == expected.rgba, route + " " + v.label + " exact composed title/cursor oracle");
        check(bytes(savePath) == privateBytes, "hover never rewrites private save");
        audit << route << ',' << index << ',' << v.node << ',' << v.point.x << ',' << v.point.y << ','
              << csv(v.node < 0 ? "" : titles[v.node]) << ',' << hash(base) << ',' << hash(frame) << '\n';
        if (!all || (v.node >= 0 && v.node < 3))
          check(writePng(out / (route + "-" + v.label + ".png"), frame).has_value(), "retain clone image");
        if (all && v.node >= 0 && v.label == "node-" + std::to_string(v.node)) ++visited;
      }
      if (++index == static_cast<int>(visits.size())) {
        if (all) event(SDL_MOUSEBUTTONDOWN, points[0].x, points[0].y);
        else event(SDL_QUIT);
        return;
      }
      if (visits[index].label == "unfocused") event(SDL_WINDOWEVENT, SDL_WINDOWEVENT_FOCUS_LOST);
      else {
        if (visits[index].label == "refocused") event(SDL_WINDOWEVENT, SDL_WINDOWEVENT_FOCUS_GAINED);
        event(SDL_MOUSEMOTION, visits[index].point.x, visits[index].point.y);
      }
    };
    const auto level = [&](const app::LevelFrameView& view) {
      check(all && view.levelName == ids[0], "hover leaves ordinary first-node launch unchanged");
      ++levelFrames; event(SDL_QUIT);
    };
    check(app::runApplication(static_cast<int>(av.size()), av.data(), front, level, options) == 0,
          "ordinary application hover route exits successfully");
    check(index == static_cast<int>(visits.size()), "every requested motion frame presented");
    check(SDL_WasInit(SDL_INIT_VIDEO) == 0, "application releases video");
  }
  check(visited == 102 && levelFrames > 0, "all 102 selectable nodes plus normal launch exercised");
  check(installed.unchanged() && normal.unchanged(), "installed and normal saves untouched");
  std::ofstream summary(out / "summary.txt");
  summary << "baseline=" << before << "\nselectable_nodes=" << visited << "\nlevel_frames=" << levelFrames
          << "\ninstalled_save_unchanged=" << installed.unchanged() << "\nnormal_save_unchanged=" << normal.unchanged()
          << "\nfailures=" << failures << '\n';
  std::cout << "102 nodes; failures=" << failures << '\n';
  return failures ? 1 : 0;
}
