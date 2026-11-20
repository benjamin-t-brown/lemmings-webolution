// Drive MM_W_012 through runApplication using SDL input. Start from Menu
// "Play Current Game", enter map node 8, wait for lever (183,60), and re-enter after results.
// Log the lever, receivers 5/6 and zone mask through a read-only frame observer.
//
// Usage: play <variant> <dir> | restart <variant> <dir>
// Both variants use an isolated save. ordinary-camera uses hatch yaw and the intro.
// stable-camera uses --frontend --native --camera <json> to specify the camera pose.
#include "app/AppEntry.h"
#include "app/MapScene.h"
#include "assets/Box.h"
#include "assets/Png.h"
#include "save/SaveStore.h"
#include "sim/World.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifndef FLA_SOURCE_DIR
#define FLA_SOURCE_DIR "."
#endif

#if WEBOLUTION_HAVE_SDL2
namespace fs = std::filesystem;
using namespace webo;

namespace {

using SteadyClock = std::chrono::steady_clock;
const SteadyClock::time_point kProcessStart = SteadyClock::now();
double wallSeconds() { return std::chrono::duration<double>(SteadyClock::now() - kProcessStart).count(); }

constexpr int kFastForwardX = 54, kFastForwardY = 348;
constexpr int kContinueX = 320, kContinueY = 360;
constexpr int kPlayCurrentX = 320, kPlayCurrentY = 363;
constexpr std::size_t kTargetNode = 8;
constexpr const char* kTargetLevel = "MM_W_012";
constexpr int kLeverX = 183, kLeverY = 60;
constexpr int kReceiverA = 5, kReceiverB = 6;
// Accepted size and FNV checksums for private save copies.
constexpr const char* kSaveFnvs[2] = {"980473be299a98f3", "163ecc74695712a2"};
constexpr const char* kCameraJson = "{\"viewport\": [640, 480], \"yaw_units\": 160, \"level_eye_height\": 96}\n";

bool pushQuit() { SDL_Event e{}; e.type = SDL_QUIT; return SDL_PushEvent(&e) == 1; }
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

std::string fnvHex(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return "unreadable";
  std::uint64_t h = 1469598103934665603ull;
  char c;
  while (in.get(c)) { h ^= static_cast<unsigned char>(c); h *= 1099511628211ull; }
  std::ostringstream os;
  os << std::hex << h;
  return os.str();
}
std::string fmt(double v, int digits = 3) {
  std::ostringstream os;
  os.setf(std::ios::fixed);
  os.precision(digits);
  os << v;
  return os.str();
}
std::string normalSaveState() {
  auto base = save::localAppDataDirectory();
  if (!base) return "unresolved";
  const auto p = save::cloneSavePathUnder(*base);
  std::error_code ec;
  if (!fs::exists(p, ec)) return "absent:" + p.string();
  return "present:" + p.string() + " bytes=" + std::to_string(fs::file_size(p, ec)) + " fnv=" + fnvHex(p);
}
std::shared_ptr<save::SaveStore> storeFor(const fs::path& dir, const fs::path& game) {
  const auto file = save::cloneSavePathUnder(dir / "save");
  return std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(file, game));
}
std::vector<std::string> argsFor(const std::string& variant, const fs::path& dir, const std::string& gameDir) {
  if (variant == "stable-camera")
    return {"fluid-lever-assessment", "--frontend", "--native", "--camera", (dir / "stable-camera.json").string(), "--game-dir", gameDir};
  return {"fluid-lever-assessment", "--game-dir", gameDir};
}
int runApp(const std::vector<std::string>& args, app::FrontendWindowObserver fo, app::LevelFrameObserver lo,
           std::shared_ptr<save::SaveStore> store) {
  std::vector<std::string> copy = args;
  std::vector<char*> argv;
  for (auto& s : copy) argv.push_back(s.data());
  app::ApplicationRunOptions options;  // production wall clock
  options.saveStore = std::move(store);
  return app::runApplication(static_cast<int>(argv.size()), argv.data(), std::move(fo), std::move(lo), options);
}
std::optional<std::pair<int, int>> pixelFor(const AssetLibrary& lib, const std::array<std::int32_t, 102>& flags, std::size_t node) {
  auto map = app::MapScene::build(lib, flags);
  if (!map) return std::nullopt;
  for (int y = 0; y < 480; ++y)
    for (int x = 0; x < 640; ++x)
      if (auto n = map->pickNode(x, y); n && *n == node) return std::make_pair(x, y);
  return std::nullopt;
}
std::string arr(const std::array<std::int32_t, 102>& a) {
  std::ostringstream os;
  for (std::size_t i = 0; i < a.size(); ++i) os << (i ? "," : "") << a[i];
  return os.str();
}

struct DiffStat { long changed = 0; int minX = 9999, minY = 9999, maxX = -1, maxY = -1; };
// Viewport rows only (y 40..329): excludes the HUD strip and the skill panel; the cursor and lemmings are NOT excluded.
DiffStat diffViewport(const Image& a, const Image& b) {
  DiffStat d;
  if (a.width != b.width || a.height != b.height) { d.changed = -1; return d; }
  for (int y = 40; y < 330 && y < a.height; ++y)
    for (int x = 0; x < a.width; ++x) {
      const std::size_t i = (static_cast<std::size_t>(y) * a.width + x) * 4;
      if (a.rgba[i] != b.rgba[i] || a.rgba[i + 1] != b.rgba[i + 1] || a.rgba[i + 2] != b.rgba[i + 2]) {
        ++d.changed;
        d.minX = std::min(d.minX, x); d.maxX = std::max(d.maxX, x);
        d.minY = std::min(d.minY, y); d.maxY = std::max(d.maxY, y);
      }
    }
  return d;
}
std::string diffStr(const DiffStat& d) {
  if (d.changed < 0) return "size-mismatch";
  std::ostringstream os;
  os << d.changed << " px";
  if (d.changed > 0) os << " bbox x" << d.minX << ".." << d.maxX << " y" << d.minY << ".." << d.maxY;
  return os.str();
}

struct GateSnap { bool found = false; double h = 0, high = 0, phase = 0; int request = -1, done = -1; bool armed = false, animated = false; };
GateSnap gateById(const sim::World& w, int id) {
  GateSnap s;
  for (const auto& g : w.setup().gates)
    if (g.id == id) {
      s.found = true; s.h = g.height; s.high = g.high; s.phase = g.wavePhase; s.request = g.request; s.done = g.done;
      s.armed = g.armed; s.animated = g.animated;
      return s;
    }
  return s;
}
const sim::Lever* leverNamed(const sim::World& w) {
  for (const auto& l : w.setup().levers)
    if (l.x == kLeverX && l.y == kLeverY) return &l;
  return nullptr;
}
std::string gateStr(const GateSnap& g) {
  if (!g.found) return "absent";
  return "h=" + fmt(g.h) + "/" + fmt(g.high, 1) + " req=" + std::to_string(g.request) + " done=" + std::to_string(g.done) +
         " armed=" + std::to_string(g.armed) + " phase=" + fmt(g.phase);
}

struct Entry {
  int number = 0;
  double startWall = 0;
  bool seenFirst = false, initialShot = false, pullShot = false, rise1 = false, rise2 = false, settledShot = false,
       settledLater = false, ffClicked = false, reloadShot = false, quitPushed = false;
  int postIntroFrames = 0;
  int leverToggles = 0, leverToggleNotFirst = 0;
  int prevLeverState = 0;
  std::vector<int> prevAllLevers;
  double pullClockS = -1, settleClockS = -1;
  long frames = 0;
  std::uint64_t lastTick = 0;
  Image initial, settled, pullImg;
  bool haveInitial = false, haveSettled = false, havePull = false;
};

struct Ctx {
  fs::path dir;
  std::string variant;
  std::ofstream frames, events, index;
  std::vector<Entry> entries;
  bool levelEnded = true;  // a frontend callback ran since the last level frame
  int feCalls = 0, lastPush = -100, menuClicks = 0, mapClicks = 0, contClicks = 0;
  int lastScreen = -1;
  double lastProgressWall = 0;
  bool mapSeen = false, blocked = false;
  std::string blockedWhy, anomaly;
  int clickX = -1, clickY = -1;
  std::array<std::int32_t, 102> flagsAtMap{}, statesAtMap{};
  int resultsSeen = 0;
  std::string results;
  int shotCount = 0;
  bool allDone = false;

  void ev(const Entry* e, std::uint64_t tick, const std::string& kind, const std::string& detail) {
    events << fmt(wallSeconds(), 2) << ',' << (e ? e->number : 0) << ',' << tick << ',' << kind << ',' << detail << '\n';
    events.flush();
  }
};

void snap(Ctx& c, const Entry& e, const app::LevelFrameView& f, const std::string& label, const std::string& detail) {
  char name[160];
  std::snprintf(name, sizeof name, "%02d-entry%d-%s.png", ++c.shotCount, e.number, label.c_str());
  if (auto r = writePng(c.dir / name, f.image); !r) c.anomaly += std::string("png ") + name + ": " + r.error() + "; ";
  const auto a = gateById(f.world, kReceiverA), b = gateById(f.world, kReceiverB);
  const auto* lv = leverNamed(f.world);
  c.index << name << " | entry " << e.number << " | " << label << " | wall " << fmt(wallSeconds(), 2) << " s | tick " << f.world.tickCount()
          << " | clock " << fmt(f.world.levelClockMs() / 1000.0, 2) << " s | frame " << f.frame << " | lever(183,60) state=" << (lv ? lv->state : 99)
          << " | id5 " << gateStr(a) << " | id6 " << gateStr(b) << " | zoneCells=" << f.world.terrain().zoneWaterCells()
          << " | camera yaw=" << f.displayedPose.yawUnits << " eye=" << fmt(f.displayedPose.levelEyeHeight, 1) << " zoomShift=" << f.displayedPose.zoomShift
          << " intro=" << f.cameraIntroActive << " | " << detail << '\n';
  c.index.flush();
}

void levelObserver(Ctx& c, const app::LevelFrameView& f) {
  c.lastProgressWall = wallSeconds();
  if (c.levelEnded) {
    c.levelEnded = false;
    Entry e;
    e.number = static_cast<int>(c.entries.size()) + 1;
    e.startWall = wallSeconds();
    c.entries.push_back(std::move(e));
    c.ev(&c.entries.back(), f.world.tickCount(), "level-entered", std::string("level=") + std::string(f.levelName) + " lemmings=" + std::to_string(f.world.lemmings().size()));
  }
  Entry& e = c.entries.back();
  ++e.frames;
  const auto& world = f.world;
  const std::uint64_t tick = world.tickCount();
  const auto a = gateById(world, kReceiverA), b = gateById(world, kReceiverB);
  const auto* lv = leverNamed(world);
  const int lvState = lv ? lv->state : 99;
  const double clockS = world.levelClockMs() / 1000.0;

  if (!e.seenFirst) {
    e.seenFirst = true;
    e.prevLeverState = lvState;
    for (const auto& l : world.setup().levers) e.prevAllLevers.push_back(l.state);
    std::ostringstream os;
    os << "levelName=" << f.levelName << " lever(183,60)=" << (lv ? "found" : "MISSING") << " linked=" << (lv ? lv->linked : false)
       << " serializedId=" << (lv ? lv->serializedId : -1) << " initialFace=" << (lv ? lv->initialFace : 0) << " state=" << lvState << " id5{" << gateStr(a)
       << "} id6{" << gateStr(b) << "} levers=" << world.setup().levers.size() << " gates=" << world.setup().gates.size();
    c.ev(&e, tick, "first-frame", os.str());
  }
  if (c.frames.is_open() && (e.frames % 3 == 1 || lvState != e.prevLeverState))
    c.frames << fmt(wallSeconds(), 3) << ',' << e.number << ',' << f.frame << ',' << tick << ',' << fmt(clockS, 3) << ',' << lvState << ',' << a.h << ',' << a.request << ','
             << a.done << ',' << fmt(a.phase) << ',' << b.h << ',' << b.request << ',' << b.done << ',' << world.terrain().zoneWaterCells() << ','
             << f.cameraIntroActive << ',' << f.displayedPose.yawUnits << ',' << world.saved() << ',' << world.dead() << ',' << static_cast<int>(f.endPhase) << '\n';

  // every lever toggle (read-only comparison with the previous frame)
  for (std::size_t i = 0; i < world.setup().levers.size() && i < e.prevAllLevers.size(); ++i) {
    const auto& l = world.setup().levers[i];
    if (l.state != e.prevAllLevers[i]) {
      std::ostringstream os;
      os << "lever@" << l.x << '/' << l.y << " id=" << l.serializedId << " state " << e.prevAllLevers[i] << "->" << l.state << " id5{" << gateStr(a) << "} id6{" << gateStr(b) << '}';
      c.ev(&e, tick, "lever-toggle", os.str());
      e.prevAllLevers[i] = l.state;
    }
  }

  const bool introOver = !f.cameraIntroActive;
  if (introOver) ++e.postIntroFrames;

  if (e.number == 1) {
    if (!e.initialShot && introOver && e.postIntroFrames >= 3 && a.found && a.request == 1 && a.h == 0.0) {
      e.initialShot = true;
      e.initial = f.image; e.haveInitial = true;
      snap(c, e, f, "initial", "before any lever contact; receivers 5/6 drained");
    }
    if (lvState != e.prevLeverState) {
      ++e.leverToggles;
      if (e.leverToggles == 1) {
        e.pullClockS = clockS;
        e.pullImg = f.image; e.havePull = true;
        snap(c, e, f, "lever-pull-1", "first accepted contact of the naturally walking lemming");
      } else {
        ++e.leverToggleNotFirst;
        snap(c, e, f, "lever-toggle-" + std::to_string(e.leverToggles), "repeat lever contact by natural flow");
      }
    }
    if (e.pullClockS >= 0 && a.found) {
      if (!e.rise1 && a.h >= 6.0 && a.h < a.high) {
        e.rise1 = true;
        if (e.haveInitial) c.ev(&e, tick, "diff-vs-initial", "rising-1 " + diffStr(diffViewport(e.initial, f.image)));
        snap(c, e, f, "rising-1", "moving surface h>=6, t_since_pull=" + fmt(clockS - e.pullClockS, 2) + " s");
      }
      if (!e.rise2 && a.h >= 14.0 && a.h < a.high) {
        e.rise2 = true;
        if (e.haveInitial) c.ev(&e, tick, "diff-vs-initial", "rising-2 " + diffStr(diffViewport(e.initial, f.image)));
        snap(c, e, f, "rising-2", "moving surface h>=14, t_since_pull=" + fmt(clockS - e.pullClockS, 2) + " s");
      }
      if (!e.settledShot && a.request == 0 && a.done == 0 && a.h >= a.high) {
        e.settledShot = true;
        e.settleClockS = clockS;
        e.settled = f.image; e.haveSettled = true;
        if (e.haveInitial) c.ev(&e, tick, "diff-vs-initial", "settled " + diffStr(diffViewport(e.initial, f.image)));
        snap(c, e, f, "settled", "request==done==0 and h==high, t_since_pull=" + fmt(clockS - e.pullClockS, 2) + " s");
      }
      if (e.settledShot && !e.settledLater && clockS - e.settleClockS >= 3.0) {
        e.settledLater = true;
        if (e.haveSettled) c.ev(&e, tick, "diff-vs-settled", "settled+3s " + diffStr(diffViewport(e.settled, f.image)));
        snap(c, e, f, "settled-plus-3s", "stability: surface stays raised");
      }
    }
    // Use the ordinary fast-forward control after the fluid settles to reach results.
    if (e.settledLater && !e.ffClicked && f.endPhase == app::LevelEndPhase::Running) {
      e.ffClicked = true;
      pushClick(kFastForwardX, kFastForwardY);
      c.ev(&e, tick, "click-fast-forward", "x=54;y=348 (ordinary in-level button)");
    }
    if (wallSeconds() - e.startWall > 420.0 && !e.quitPushed) {
      e.quitPushed = true;
      c.anomaly += "level 1 watchdog (420 s); ";
      c.ev(&e, tick, "watchdog", "quit");
      pushQuit();
    }
  } else {
    // Compare the first stable frame after re-entry.
    if (!e.reloadShot && introOver && e.postIntroFrames >= 3) {
      e.reloadShot = true;
      std::ostringstream os;
      os << "lever(183,60) state=" << lvState << " id5{" << gateStr(a) << "} id6{" << gateStr(b) << "}";
      c.ev(&e, tick, "reload-state", os.str());
      if (c.entries.front().haveInitial) c.ev(&e, tick, "diff-vs-entry1-initial", diffStr(diffViewport(c.entries.front().initial, f.image)));
      snap(c, e, f, "reload-initial", "second level entry through results -> map -> node 8");
    }
    if (e.reloadShot && e.postIntroFrames >= 40 && !e.quitPushed) {
      e.quitPushed = true;
      c.ev(&e, tick, "reload-later", "lever(183,60) state=" + std::to_string(lvState) + " id5{" + gateStr(a) + "}");
      snap(c, e, f, "reload-plus-frames", "still drained, level running");
      pushQuit();
    }
    if (wallSeconds() - e.startWall > 120.0 && !e.quitPushed) {
      e.quitPushed = true;
      c.anomaly += "level 2 watchdog (120 s); ";
      pushQuit();
    }
  }
  e.prevLeverState = lvState;
  e.lastTick = tick;
}

std::optional<fs::path> findSourceSave(std::string& why, std::string& fnv) {
  const fs::path runs = fs::path(FLA_SOURCE_DIR) / "build" / "targeted-play-001" / "runs";
  std::error_code ec;
  std::vector<fs::path> found[2];
  if (fs::exists(runs, ec))
    for (const auto& run : fs::directory_iterator(runs, ec)) {
      const fs::path p = save::cloneSavePathUnder(run.path() / "save");
      if (!fs::exists(p, ec) || fs::file_size(p, ec) != 1504) continue;
      const std::string h = fnvHex(p);
      for (int k = 0; k < 2; ++k)
        if (h == kSaveFnvs[k]) found[k].push_back(p);
    }
  for (int k = 0; k < 2; ++k) {
    if (found[k].empty()) continue;
    std::sort(found[k].begin(), found[k].end());
    fnv = kSaveFnvs[k];
    why = "retained clone save earned by public wins (targeted-play-001), fnv " + fnv + ", " + std::to_string(found[k].size()) + " candidates, newest chosen";
    return found[k].back();
  }
  why = "no retained targeted-play-001 save with a known FNV found";
  return std::nullopt;
}

int playMode(const std::string& variant, const fs::path& root, const std::string& gameDir) {
  const fs::path game(gameDir);
  const fs::path dir = root / variant;
  std::error_code ec;
  fs::remove_all(dir, ec);
  fs::create_directories(dir, ec);
  std::ofstream(dir / "stable-camera.json") << kCameraJson;

  std::string why, srcFnv;
  const auto src = findSourceSave(why, srcFnv);
  const fs::path dst = save::cloneSavePathUnder(dir / "save");
  fs::create_directories(dst.parent_path(), ec);
  bool copied = false;
  if (src) copied = fs::copy_file(*src, dst, fs::copy_options::overwrite_existing, ec);
  const std::string saveFnvStart = copied ? fnvHex(dst) : "-";

  Ctx c;
  c.dir = dir;
  c.variant = variant;
  c.frames.open(dir / "frames.csv");
  c.events.open(dir / "events.csv");
  c.index.open(dir / "image-index.txt");
  c.frames << "wall_s,entry,frame,tick,level_clock_s,lever183_60_state,id5_h,id5_req,id5_done,id5_phase,id6_h,id6_req,id6_done,zone_cells,intro_active,yaw_units,saved,dead,end_phase\n";
  c.events << "wall_s,entry,tick,kind,detail\n";

  auto lib = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!lib) { std::cerr << "assets: " << lib.error() << '\n'; return 1; }
  if (!copied) { c.blocked = true; c.blockedWhy = "private save copy unavailable: " + why; }

  const std::string normalBefore = normalSaveState();
  auto store = storeFor(dir, game);
  c.lastProgressWall = wallSeconds();
  const double startWall = wallSeconds();

  const auto frontend = [&](const app::FrontendSession& s, int frame) {
    ++c.feCalls;
    c.levelEnded = true;
    if (c.lastScreen != static_cast<int>(s.screen())) {
      c.lastScreen = static_cast<int>(s.screen());
      c.lastProgressWall = wallSeconds();
      c.ev(nullptr, 0, "frontend-screen", "screen=" + std::to_string(c.lastScreen) + " frame=" + std::to_string(frame));
      c.lastPush = c.feCalls - 10;
    }
    if (c.blocked) { pushQuit(); return; }
    if (wallSeconds() - c.lastProgressWall > 60.0) { c.anomaly += "frontend stalled >60 s; "; pushQuit(); return; }
    const bool canPush = c.feCalls - c.lastPush >= 5;
    switch (s.screen()) {
      case shell::Screen::Menu:
        if (c.entries.size() >= 1) { pushQuit(); break; }
        if (canPush && c.menuClicks < 3) { ++c.menuClicks; c.lastPush = c.feCalls; pushClick(kPlayCurrentX, kPlayCurrentY); c.ev(nullptr, 0, "click-play-current-game", "x=320;y=363"); }
        else if (canPush) { c.anomaly += "Play Current Game click did not leave the menu; "; pushQuit(); }
        break;
      case shell::Screen::Map: {
        if (c.entries.size() >= 2) { pushQuit(); break; }
        if (!c.mapSeen) {
          c.mapSeen = true;
          if (auto r = writePng(dir / "map-before.png", s.composeFrame()); !r) c.anomaly += "png map: " + r.error() + "; ";
          c.statesAtMap = s.mapStates();
          c.flagsAtMap = s.completionFlags();
          std::ofstream os(dir / "earned-map.txt");
          os << "flags_at_map=" << arr(c.flagsAtMap) << "\nstates_at_map=" << arr(c.statesAtMap) << '\n';
          const auto st = c.statesAtMap[kTargetNode];
          auto map = app::MapScene::build(*lib, c.flagsAtMap);
          if (st != 2 && st != 3) { c.blocked = true; c.blockedWhy = "node 8 not selectable (map state " + std::to_string(st) + ")"; }
          else if (!map || !map->levelForNode(kTargetNode) || *map->levelForNode(kTargetNode) != kTargetLevel) { c.blocked = true; c.blockedWhy = "node 8 is not MM_W_012"; }
          else if (auto px = pixelFor(*lib, c.flagsAtMap, kTargetNode)) { c.clickX = px->first; c.clickY = px->second; }
          else { c.blocked = true; c.blockedWhy = "no map pixel picks node 8"; }
          os << (c.blocked ? "BLOCKED: " + c.blockedWhy : "target node 8 " + std::string(kTargetLevel) + " click pixel " + std::to_string(c.clickX) + "," + std::to_string(c.clickY)) << '\n';
          if (c.blocked) { pushQuit(); break; }
        }
        if (canPush && c.mapClicks < 6) { ++c.mapClicks; c.lastPush = c.feCalls; pushClick(c.clickX, c.clickY); c.ev(nullptr, 0, "click-map-node", "node=8;x=" + std::to_string(c.clickX) + ";y=" + std::to_string(c.clickY)); }
        else if (canPush) { c.anomaly += "map click did not select a level; "; pushQuit(); }
        break;
      }
      case shell::Screen::Results: {
        if (c.resultsSeen < static_cast<int>(c.entries.size())) {
          ++c.resultsSeen;
          std::ostringstream os;
          if (const auto& o = s.outcome()) os << "success=" << o->success << " saved=" << o->saved << "/" << o->total << " required=" << o->required;
          os << " saveCount=" << s.saveCount() << " saveError=" << (s.saveError() ? *s.saveError() : "-");
          c.results += os.str() + "\n";
          c.ev(nullptr, 0, "results-screen", os.str());
          if (auto r = writePng(dir / "results-screen.png", s.composeFrame()); !r) c.anomaly += "png results: " + r.error() + "; ";
          c.lastPush = -100; c.contClicks = 0; c.mapClicks = 0;
        }
        if (canPush) {
          if (c.contClicks < 3) { ++c.contClicks; c.lastPush = c.feCalls; pushClick(kContinueX, kContinueY); c.ev(nullptr, 0, "click-continue-or-retry", "x=320;y=360"); }
          else { c.anomaly += "results click did not leave the results screen; "; pushQuit(); }
        }
        break;
      }
      default: c.anomaly += "unexpected frontend screen; "; pushQuit(); break;
    }
  };
  const auto level = [&](const app::LevelFrameView& f) { levelObserver(c, f); };

  std::cout << "FluidLeverAssessment play " << variant << " save " << dst.string() << " (" << why << ")\n";
  const auto args = argsFor(variant, dir, gameDir);
  const int status = runApp(args, frontend, level, store);
  const double totalWall = wallSeconds() - startWall;
  const std::string normalAfter = normalSaveState();
  const std::string saveFnvEnd = fs::exists(dst, ec) ? fnvHex(dst) : "-";

  const Entry* e1 = c.entries.size() >= 1 ? &c.entries[0] : nullptr;
  const Entry* e2 = c.entries.size() >= 2 ? &c.entries[1] : nullptr;
  std::ofstream res(dir / "result.txt");
  res << "variant=" << variant << "\nargs=";
  for (const auto& a : args) res << a << ' ';
  res << "\nsave_source=" << (src ? src->string() : "-") << "\nsave_provenance=" << why << "\nsave_copied=" << copied << "\nsave_fnv_start=" << saveFnvStart
      << "\nsave_fnv_end=" << saveFnvEnd << "\nnormal_save_before=" << normalBefore << "\nnormal_save_after=" << normalAfter
      << "\nnormal_save_unchanged=" << (normalBefore == normalAfter ? 1 : 0) << "\nblocked=" << (c.blocked ? c.blockedWhy : "-")
      << "\nrunApplication_status=" << status << "\ntotal_wall_s=" << fmt(totalWall, 1) << "\nlevel_entries=" << c.entries.size() << "\nresults_screens=" << c.resultsSeen
      << "\nanomaly=" << (c.anomaly.empty() ? "-" : c.anomaly) << "\nflags_at_map=" << arr(c.flagsAtMap) << "\nstates_at_map=" << arr(c.statesAtMap) << '\n';
  if (e1)
    res << "entry1: frames=" << e1->frames << " initial=" << e1->initialShot << " lever_toggles=" << e1->leverToggles << " repeat_toggles=" << e1->leverToggleNotFirst
        << " rising1=" << e1->rise1 << " rising2=" << e1->rise2 << " settled=" << e1->settledShot << " settled_plus_3s=" << e1->settledLater
        << " pull_clock_s=" << fmt(e1->pullClockS, 2) << " settle_clock_s=" << fmt(e1->settleClockS, 2) << " ff_clicked=" << e1->ffClicked << '\n';
  if (e2) res << "entry2: frames=" << e2->frames << " reload_initial=" << e2->reloadShot << '\n';
  res << "results:\n" << c.results;
  std::cout << "FluidLeverAssessment " << variant << ": entries=" << c.entries.size() << " blocked=" << (c.blocked ? c.blockedWhy : "-") << " anomaly=" << (c.anomaly.empty() ? "-" : c.anomaly)
            << " toggles=" << (e1 ? e1->leverToggles : -1) << " settled=" << (e1 ? e1->settledShot : false) << " reload=" << (e2 ? e2->reloadShot : false)
            << " normal_save_unchanged=" << (normalBefore == normalAfter) << " wall=" << fmt(totalWall, 1) << "s\n";
  // The exit code checks route and output integrity. result.txt records lever contact separately.
  const bool integrity = status == 0 && res && c.events && c.index && normalBefore == normalAfter && (c.blocked || !c.entries.empty());
  if (!integrity) std::cerr << "FAIL: driver integrity (status=" << status << ")\n";
  return integrity ? 0 : 1;
}

int restartMode(const std::string& variant, const fs::path& root, const std::string& gameDir) {
  const fs::path game(gameDir);
  const fs::path dir = root / variant;
  const fs::path saveFile = save::cloneSavePathUnder(dir / "save");
  std::ofstream md(dir / "restart-result.txt");
  std::error_code ec;
  if (!fs::exists(saveFile, ec)) { md << "UNVERIFIED: no private save from the play invocation\n"; std::cout << "FluidLeverAssessment restart: UNVERIFIED\n"; return md ? 0 : 1; }
  struct Seen { int calls = 0; bool map = false; std::array<std::int32_t, 102> flags{}, states{}; int clicks = 0, lastPush = -100; std::optional<std::string> loadError; } seen;
  const auto frontend = [&](const app::FrontendSession& s, int) {
    ++seen.calls;
    if (seen.calls > 600) { pushQuit(); return; }
    if (s.screen() == shell::Screen::Menu) {
      if (seen.calls - seen.lastPush >= 5 && seen.clicks < 3) { ++seen.clicks; seen.lastPush = seen.calls; pushClick(kPlayCurrentX, kPlayCurrentY); seen.loadError = s.saveLoadError(); }
      else if (seen.clicks >= 3) pushQuit();
    } else if (s.screen() == shell::Screen::Map) {
      if (!seen.map) { seen.map = true; seen.flags = s.completionFlags(); seen.states = s.mapStates(); seen.loadError = s.saveLoadError(); }
      pushQuit();
    } else pushQuit();
  };
  const std::string before = fnvHex(saveFile);
  const int status = runApp(argsFor(variant, dir, gameDir), frontend, {}, storeFor(dir, game));
  const std::string after = fnvHex(saveFile);
  const bool selectable = seen.map && (seen.states[kTargetNode] == 2 || seen.states[kTargetNode] == 3);
  const bool ok = status == 0 && seen.map && selectable && before == after;
  md << "variant=" << variant << "\nrunApplication status=" << status << " reachedMap=" << seen.map << " loadError=" << (seen.loadError ? *seen.loadError : "-")
     << "\nnode 8 map state after restart=" << seen.states[kTargetNode] << " (2/3 = selectable)\nflags=" << arr(seen.flags) << "\nstates=" << arr(seen.states)
     << "\nsave fnv before=" << before << " after=" << after << " (unchanged: " << (before == after) << ")\nRESULT: " << (ok ? "VERIFIED" : "FAILED") << '\n';
  std::cout << "FluidLeverAssessment restart " << variant << ": " << (ok ? "VERIFIED" : "FAILED (finding)") << '\n';
  return status == 0 && md ? 0 : 1;
}

}  // namespace
#endif

int main(int argc, char** argv) {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "FluidLeverAssessment: requires SDL2\n";
  return 1;
#else
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir) { std::cout << "FluidLeverAssessment: SKIPPED (set WEBOLUTION_GAME_DIR)\n"; return 0; }
  const char* video = std::getenv("SDL_VIDEODRIVER");
  if (!video || std::string(video) != "dummy") { std::cerr << "FluidLeverAssessment: refusing to run without SDL_VIDEODRIVER=dummy\n"; return 1; }
  if (argc < 4) { std::cerr << "usage: test_fluidlever_assessment play|restart <variant> <output-dir>\n"; return 1; }
  const std::string mode = argv[1], variant = argv[2];
  const fs::path root(argv[3]);
  if (variant != "ordinary-camera" && variant != "stable-camera") { std::cerr << "unknown variant\n"; return 1; }
  std::error_code ec;
  fs::create_directories(root, ec);
  if (mode == "play") return playMode(variant, root, gameDir);
  if (mode == "restart") return restartMode(variant, root, gameDir);
  std::cerr << "unknown mode\n";
  return 1;
#endif
}
