// camera-entry-001: frontend selection of the installed MM_BASH_002, frozen vs current.
//
// One driver, two builds. cameraentry_before links the authentic entry-snapshot
// main.cpp / AppEntry.h kept under the ignored build/camera-entry-001/frozen/ and
// must observe the old behaviour (canonical yaw32, no opening camera move).
// cameraentry_after links the current sources and must observe the serialized
// selected hatch's base yaw229 with the existing entry animation, Retry without
// the animation, and map re-entry with it again. Production files are never swapped.
//
// Every expectation is a *static-camera equivalent*: the same public application is
// run again with an explicit --camera file (which bypasses CameraIntro and the entry
// metadata) whose operands are calculated below from the retained intro equations,
// not by calling CameraIntro. A camera file cannot carry the intro's wrapped phase
// in yaw_units (the controller wraps yaw_units into [0,256)), so the phase goes in
// through yaw_offset with yaw_units 0; (0 + offset) / 256 is then bit-identical to
// the intro's wrapped yawTurns.
//
// Progress, when MM_BASH_002 is not selectable on a fresh map, is written to a
// temporary clone save as SOURCE-TEST setup using the accepted node mapping and the
// unchanged map rules. It is not natural gameplay and not a new unlock policy.
// Dummy SDL drivers; runs end through observer-queued SDL_QUIT. No original code.

#include "app/AppEntry.h"
#include "app/MapScene.h"
#include "assets/Box.h"
#include "assets/Png.h"
#include "save/SaveStore.h"
#include "save/SavedGame.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

namespace {

#ifdef CAMERAENTRY_FROZEN
constexpr bool kFrozen = true;
constexpr const char* kSide = "before";
#else
constexpr bool kFrozen = false;
constexpr const char* kSide = "after";
#endif

constexpr double kFrameSeconds = 0.25;  // exact in binary: intro clock = 0.25 * frame
constexpr const char* kLevel = "MM_BASH_002";

bool bitsEqual(double a, double b) {
  return std::bit_cast<std::uint64_t>(a) == std::bit_cast<std::uint64_t>(b);
}

bool pushClick(int x, int y) {
  SDL_Event e{};
  e.type = SDL_MOUSEBUTTONDOWN;
  e.button.type = SDL_MOUSEBUTTONDOWN;
  e.button.button = SDL_BUTTON_LEFT;
  e.button.x = x;
  e.button.y = y;
  return SDL_PushEvent(&e) == 1;
}
bool pushEscape() {
  SDL_Event e{};
  e.type = SDL_KEYDOWN;
  e.key.type = SDL_KEYDOWN;
  e.key.keysym.scancode = SDL_SCANCODE_ESCAPE;
  e.key.keysym.sym = SDLK_ESCAPE;
  return SDL_PushEvent(&e) == 1;
}
bool pushQuit() {
  SDL_Event e{};
  e.type = SDL_QUIT;
  return SDL_PushEvent(&e) == 1;
}

// What the level observer saw at one frame. Pose fields exist only where the
// read-only observation seam in AppEntry.h exists (the current build).
struct Seen {
  int frame = 0;
  std::uint64_t tick = 0;
  bool intro = false;
  bool menuOpen = false;
  std::string level;
  Image image;
#ifndef CAMERAENTRY_FROZEN
  app::CameraParams pose;
  std::array<double, 3> pick{};
#endif
};

Seen capture(const app::LevelFrameView& f) {
  Seen s;
  s.frame = f.frame;
  s.tick = static_cast<std::uint64_t>(f.world.tickCount());
  s.intro = f.cameraIntroActive;
  s.menuOpen = f.levelMenuOpen;
  s.level = std::string(f.levelName);
  s.image = f.image;
#ifndef CAMERAENTRY_FROZEN
  s.pose = f.displayedPose;
  s.pick = f.cameraPickReference;
#endif
  return s;
}

std::size_t changedPixels(const Image& a, const Image& b) {
  if (a.width != b.width || a.height != b.height || a.rgba.size() != b.rgba.size())
    return static_cast<std::size_t>(-1);
  std::size_t n = 0;
  for (std::size_t i = 0; i + 3 < a.rgba.size(); i += 4)
    if (!std::equal(a.rgba.begin() + static_cast<std::ptrdiff_t>(i),
                    a.rgba.begin() + static_cast<std::ptrdiff_t>(i) + 3,
                    b.rgba.begin() + static_cast<std::ptrdiff_t>(i)))
      ++n;
  return n;
}

// The retained opening-move equations, written out independently of CameraIntro
// (docs/scratch/level-entry-camera-intro.md): a pull-back of 350 units that
// eases out over two seconds, and a yaw swing that starts after one second.
// Static equivalent: yaw_units 0 plus a yaw_offset carrying the wrapped phase.
struct StaticOperands {
  double yawUnits = 0.0;
  double yawOffset = 0.0;
  double eyeDistance = 0.0;
};
StaticOperands entryOperands(int frame, double baseYaw) {
  const double t = kFrameSeconds * frame;
  const double q = t / 2.0;
  double extra = 0.0;
  if (q < 1.0) extra = 350.0 - (2.0 - q) * q * 350.0;
  double turns = (baseYaw + 128.0) / 256.0;
  const double u = (t - 1.0) / 2.0;
  if (u > 0.0) turns += u < 0.5 ? 2.0 * u * u : 2.0 * ((2.0 - u) * u - 0.5);
  while (turns > 1.0) turns -= 1.0;
  return {0.0, turns * 256.0, 377.516 + extra};
}

std::vector<char*> argvOf(std::vector<std::string>& args) {
  std::vector<char*> argv;
  for (auto& a : args) argv.push_back(a.data());
  return argv;
}

}  // namespace

int main() {
  const char* gameDirC = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDirC || !*gameDirC) {
    std::cout << "CameraEntry[" << kSide << "]: SKIPPED (set WEBOLUTION_GAME_DIR to retail assets)\n";
    return 0;
  }
  const char* video = std::getenv("SDL_VIDEODRIVER");
  const char* audio = std::getenv("SDL_AUDIODRIVER");
  if (!video || std::string(video) != "dummy" || !audio || std::string(audio) != "dummy") {
    std::cerr << "CameraEntry: refusing to run without SDL dummy video and audio drivers\n";
    return 1;
  }
  const std::string gameDir = gameDirC;
  auto library = AssetLibrary::open(fs::path(gameDir) / "lemmings.box", fs::path(gameDir) / "patch.box");
  if (!library) { std::cerr << "CameraEntry: " << library.error() << '\n'; return 1; }

  const fs::path outDir = fs::path(CAMERA_ENTRY_DIR) / kSide;
  std::error_code ec;
  fs::create_directories(outDir, ec);
  if (ec) { std::cerr << "CameraEntry: cannot create " << outDir.string() << '\n'; return 1; }
  std::ofstream report(outDir / "measurements.txt");
  report << std::setprecision(17);

  int failures = 0;
  const auto expect = [&](bool ok, const std::string& what) {
    report << (ok ? "ok   " : "FAIL ") << what << '\n';
    if (!ok) { ++failures; std::cerr << "FAIL: " << what << '\n'; }
  };
  const auto png = [&](const std::string& name, const Image& image) {
    expect(writePng(outDir / name, image).has_value(), "artifact " + name);
  };
  report << "side=" << kSide << " frozen=" << kFrozen << " level=" << kLevel
         << " frameSeconds=" << kFrameSeconds << '\n';

  // ---- Locate the actual installed map node and, only if needed, its prerequisite.
  auto setup = sim::loadRetailLevel(*library, kLevel);
  if (!setup) { std::cerr << "CameraEntry: " << setup.error() << '\n'; return 1; }
  const int terrainHeight = setup->terrain.height();
  auto zeroMap = app::MapScene::build(*library, {});
  if (!zeroMap) { std::cerr << "CameraEntry: " << zeroMap.error() << '\n'; return 1; }
  std::optional<std::size_t> node;
  for (std::size_t n = 0; n < 102; ++n)
    if (zeroMap->levelForNode(n) == std::string(kLevel)) node = n;
  expect(node.has_value(), "installed map table names a node for MM_BASH_002");
  if (!node) return 1;

  std::array<std::int32_t, 102> flags{};
  std::shared_ptr<save::SaveStore> store;
  std::string setupNote = "fresh map: node selectable, no progress setup";
  if (zeroMap->states()[*node] != 2 && zeroMap->states()[*node] != 3) {
    std::optional<std::size_t> prerequisite;
    for (std::size_t p = 0; p < 102 && !prerequisite; ++p) {
      std::array<std::int32_t, 102> trial{};
      trial[p] = 1;
      auto map = app::MapScene::build(*library, trial);
      if (map && map->states()[*node] == 3) prerequisite = p;
    }
    expect(prerequisite.has_value(), "one completed node unlocks MM_BASH_002 under the unchanged map rules");
    if (!prerequisite) return 1;
    const auto& mapping = app::MapScene::nodeMapping()[*prerequisite];
    save::SavedGame game;
    expect(game.markComplete(mapping[0], mapping[1]), "SOURCE-TEST progress record accepted");
    const auto stamp = std::chrono::system_clock::now().time_since_epoch().count();
    const fs::path root = fs::temp_directory_path(ec) /
                          ("webolution-cameraentry-" + std::string(kSide) + "-" + std::to_string(stamp));
    store = std::make_shared<save::SaveStore>(
        std::make_unique<save::FileSaveSink>(save::cloneSavePathUnder(root), fs::path(gameDir)));
    expect(store->save(game).has_value(), "SOURCE-TEST progress written to a temporary clone save");
    flags = game.nodeFlags(app::MapScene::nodeMapping());
    setupNote = "SOURCE-TEST setup: node " + std::to_string(*prerequisite) + " (group " +
                std::to_string(mapping[0]) + ", slot " + std::to_string(mapping[1]) +
                ") marked complete in a temporary clone save; not natural gameplay";
  }
  report << "node=" << *node << ' ' << setupNote << " terrainHeight=" << terrainHeight << '\n';
  auto selectable = app::MapScene::build(*library, flags);
  if (!selectable) { std::cerr << "CameraEntry: " << selectable.error() << '\n'; return 1; }
  int mapX = -1, mapY = -1;
  for (int y = 0; y < 480 && mapX < 0; ++y)
    for (int x = 0; x < 640; ++x)
      if (selectable->pickNode(x, y) == node) { mapX = x; mapY = y; break; }
  expect(mapX >= 0, "MM_BASH_002 map node is pickable");
  if (mapX < 0) return 1;

  // ---- Independently specified static-camera runs through the public application.
  const auto writeCamera = [&](const std::string& tag, const StaticOperands& op) {
    const fs::path path = outDir / ("camera-" + tag + ".json");
    std::ofstream out(path);
    out << std::setprecision(17) << "{\"yaw_units\":" << op.yawUnits << ",\"yaw_offset\":"
        << op.yawOffset << ",\"eye_distance\":" << op.eyeDistance << ",\"level_eye_height\":"
        << terrainHeight << "}\n";
    expect(bool(out), "camera file " + tag);
    return path;
  };
  const auto runStatic = [&](const std::string& tag, const StaticOperands& op, int lastFrame,
                             std::vector<Seen>& seen) {
    const fs::path camera = writeCamera(tag, op);
    // --frames is the application's precondition for the test frame clock; the
    // observer-queued SDL_QUIT ends every run long before the cap.
    std::vector<std::string> args{"cameraentry-static", "--game-dir", gameDir, "--level", kLevel,
                                  "--sim", "--native", "--geom", "route", "--camera", camera.string(),
                                  "--frames", "40", "--no-audio"};
    auto argv = argvOf(args);
    bool queued = false;
    int callbacks = 0;
    const int result = app::runApplication(
        static_cast<int>(argv.size()), argv.data(), {},
        [&](const app::LevelFrameView& f) {
          ++callbacks;
          seen.push_back(capture(f));
          if (f.frame >= lastFrame && !queued) queued = pushQuit();
        },
        app::ApplicationRunOptions{.frameSeconds = kFrameSeconds});
    const bool ok = result == 0 && queued && callbacks == lastFrame + 1 &&
                    static_cast<int>(seen.size()) == lastFrame + 1;
    expect(ok, "static camera '" + tag + "' public run: exit " + std::to_string(result) +
                   ", callbacks " + std::to_string(callbacks) + ", queued SDL_QUIT " +
                   std::to_string(queued));
#ifndef CAMERAENTRY_FROZEN
    if (!seen.empty()) {
      const auto& pose = seen.back().pose;
      expect(!seen.back().intro && !pose.introActive && bitsEqual(pose.yawUnits, op.yawUnits) &&
                 bitsEqual(pose.yawOffset, op.yawOffset) && bitsEqual(pose.eyeDistance, op.eyeDistance),
             "explicit camera file '" + tag + "' keeps its operands and never arms the intro");
    }
#endif
    return ok;
  };

  constexpr std::array<int, 3> kSamples{0, 3, 6};
  std::vector<Seen> canonical;
  runStatic("canonical-yaw32", {32.0, 128.0, 377.516}, 6, canonical);
  std::array<std::vector<Seen>, 3> entryStatic;
  for (std::size_t i = 0; i < kSamples.size(); ++i) {
    const auto op = entryOperands(kSamples[i], 229.0);
    report << "entry operands frame " << kSamples[i] << ": yaw_units " << op.yawUnits
           << " yaw_offset " << op.yawOffset << " eye_distance " << op.eyeDistance << '\n';
    runStatic("entry-yaw229-frame" + std::to_string(kSamples[i]), op, kSamples[i], entryStatic[i]);
  }

  // ---- Direct diagnostic route: no frontend, no camera file. Unchanged by this task.
  {
    std::vector<std::string> args{"cameraentry-direct", "--game-dir", gameDir, "--level", kLevel,
                                  "--sim", "--native", "--frames", "40", "--no-audio"};
    auto argv = argvOf(args);
    int callbacks = 0;
    bool queued = false;
    Seen direct;
    const int result = app::runApplication(
        static_cast<int>(argv.size()), argv.data(), {},
        [&](const app::LevelFrameView& f) {
          ++callbacks;
          direct = capture(f);
          queued = pushQuit();
        },
        app::ApplicationRunOptions{.frameSeconds = kFrameSeconds});
    bool ok = result == 0 && queued && callbacks == 1 && direct.level == kLevel && !direct.intro;
#ifndef CAMERAENTRY_FROZEN
    ok = ok && bitsEqual(direct.pose.yawUnits, 32.0) && !direct.pose.introActive &&
         direct.pose.zoomShift == 0;
#endif
    expect(ok, "direct --level MM_BASH_002 --sim keeps canonical yaw32 without intro");
  }

  // ---- The ordinary route: startup -> map -> MM_BASH_002 -> Retry -> Quit -> map -> again.
  struct Trace {
    bool ok = true;
    int frontCalls = 0, mapVisits = 0, levelCalls = 0, legs = 0, quits = 0;
    std::vector<Seen> entry;  // leg 1, frames 0..6
    Seen retryFrame0, reentryFrame0;
    void fail(const std::string& why, std::ofstream& log) {
      if (ok) log << "FAIL trace: " << why << '\n';
      ok = false;
    }
  } t;
  const auto front = [&](const app::FrontendSession& s, int) {
    ++t.frontCalls;
    if (!t.ok) { pushQuit(); return; }
    if (s.screen() == shell::Screen::Menu && t.frontCalls == 1) {
      if (!pushClick(320, 363)) t.fail("could not click Play Current Game", report);
    } else if (s.screen() == shell::Screen::Map) {
      ++t.mapVisits;
      if (t.mapVisits > 2 || !pushClick(mapX, mapY)) t.fail("map selection", report);
    } else {
      t.fail("unexpected frontend screen", report);
      pushQuit();
    }
  };
  const auto level = [&](const app::LevelFrameView& f) {
    ++t.levelCalls;
    if (!t.ok) { pushQuit(); return; }
    if (f.levelName != kLevel) { t.fail("wrong level " + std::string(f.levelName), report); pushQuit(); return; }
    if (f.frame == 0) ++t.legs;
    if (t.legs == 1) {
      if (f.frame <= 6) t.entry.push_back(capture(f));
      if (f.frame == 6) { if (!pushEscape()) t.fail("could not open level menu", report); }
      else if (f.frame == 7) {
        if (!f.levelMenuOpen || !pushClick(320, 140)) t.fail("Retry click", report);  // Retry
      } else if (f.frame > 7) t.fail("leg 1 ran past Retry", report);
    } else if (t.legs == 2) {
      if (f.frame == 0) { t.retryFrame0 = capture(f); if (!pushEscape()) t.fail("retry menu", report); }
      else if (f.frame == 1) {
        if (!f.levelMenuOpen || !pushClick(320, 170)) t.fail("Quit-to-map click", report);  // Quit to map
      } else t.fail("leg 2 ran past Quit", report);
    } else if (t.legs == 3) {
      if (f.frame == 0) { t.reentryFrame0 = capture(f); if (pushQuit()) ++t.quits; else t.fail("final quit", report); }
      else t.fail("leg 3 ran past its quit", report);
    } else t.fail("too many legs", report);
  };

  // --frontend with --frames is the connected route the other frame-clock tests use
  // (levelreturn, levelmenuwindow); the cap bounds every leg and is never reached.
  std::vector<std::string> args{"cameraentry-app", "--frontend", "--frames", "40", "--game-dir",
                                gameDir, "--no-audio"};
  auto argv = argvOf(args);
  const int result = app::runApplication(static_cast<int>(argv.size()), argv.data(), front, level,
                                         app::ApplicationRunOptions{.frameSeconds = kFrameSeconds,
                                                                    .saveStore = store});
  report << "public route: exit " << result << " frontendCallbacks " << t.frontCalls
         << " mapSelections " << t.mapVisits << " levelCallbacks " << t.levelCalls
         << " legs " << t.legs << " queuedQuit " << t.quits << '\n';
  expect(result == 0 && t.ok && t.legs == 3 && t.quits == 1 && t.mapVisits == 2 && t.levelCalls == 11 &&
             t.entry.size() == 7,
         "ordinary startup/map/Retry/Quit/re-entry route ends normally through observer SDL_QUIT");
  if (t.entry.size() != 7) { std::cerr << "CameraEntry: incomplete entry trace\n"; return 1; }

  // ---- Compare, per build.
  for (std::size_t i = 0; i < kSamples.size(); ++i) {
    const int k = kSamples[i];
    const Seen& actual = t.entry[static_cast<std::size_t>(k)];
    const Seen& wantEntry = entryStatic[i].back();
    const Seen& wantCanonical = canonical[static_cast<std::size_t>(k)];
    const std::string tag = "frame " + std::to_string(k);
    expect(actual.frame == k, tag + " index");
    expect(actual.tick == wantEntry.tick && actual.tick == wantCanonical.tick,
           tag + " simulation tick agrees with both static runs (" + std::to_string(actual.tick) + ")");
    const auto vsEntry = changedPixels(actual.image, wantEntry.image);
    const auto vsCanonical = changedPixels(actual.image, wantCanonical.image);
    report << tag << ": changed pixels vs entry-yaw229 static " << vsEntry
           << ", vs canonical-yaw32 static " << vsCanonical << '\n';
    png("frontend-frame" + std::to_string(k) + ".png", actual.image);
    png("static-entry-yaw229-frame" + std::to_string(k) + ".png", wantEntry.image);
    png("static-canonical-yaw32-frame" + std::to_string(k) + ".png", wantCanonical.image);
    if (kFrozen) {
      expect(vsCanonical == 0, tag + ": BEFORE renders the canonical yaw32 view");
      expect(vsEntry != 0, tag + ": BEFORE differs from the selected-hatch entry view");
      expect(!actual.intro, tag + ": BEFORE has no opening camera move");
    } else {
      expect(vsEntry == 0, tag + ": AFTER renders the selected yaw229 entry pose");
      expect(vsCanonical != 0 && vsCanonical != static_cast<std::size_t>(-1),
             tag + ": AFTER differs from the canonical yaw32 view");
      expect(actual.intro, tag + ": AFTER has the opening camera move active");
    }
  }
#ifndef CAMERAENTRY_FROZEN
  // Read-only pose seam against independently calculated operands.
  for (int k = 0; k <= 6; ++k) {
    const Seen& s = t.entry[static_cast<std::size_t>(k)];
    expect(s.pose.introActive && s.pose.introSeconds == kFrameSeconds * k &&
               bitsEqual(s.pose.yawUnits, 229.0) && s.pose.yawOffset == 128.0 &&
               s.pose.zoomShift == 0 && bitsEqual(s.pose.levelEyeHeight, terrainHeight) &&
               s.pose.eyeDistance == 377.516,
           "frame " + std::to_string(k) + " pose: yaw229, zoom0, terrain eye height, intro clock " +
               std::to_string(kFrameSeconds * k));
    expect(s.pick == std::array<double, 3>{229.0, static_cast<double>(terrainHeight / 2), 0.5},
           "frame " + std::to_string(k) + " pick reference seeded with yaw and rows/2, 0.5");
  }
  expect(!t.retryFrame0.intro && !t.retryFrame0.pose.introActive && t.retryFrame0.pose.introSeconds == 0.0 &&
             bitsEqual(t.retryFrame0.pose.yawUnits, 229.0) && t.retryFrame0.pose.zoomShift == 0,
         "Retry keeps the selected yaw229 without replaying the intro");
  expect(t.reentryFrame0.intro && t.reentryFrame0.pose.introActive && t.reentryFrame0.pose.introSeconds == 0.0 &&
             bitsEqual(t.reentryFrame0.pose.yawUnits, 229.0),
         "map re-entry restores the intro preference with the selected yaw229");
#else
  expect(!t.retryFrame0.intro && !t.reentryFrame0.intro, "BEFORE: neither Retry nor re-entry has an intro");
#endif
  report << "summary: failures " << failures << '\n';
  if (failures == 0)
    std::cout << "PASS: CameraEntry[" << kSide << "] " << kLevel << " observations match the "
              << (kFrozen ? "frozen canonical yaw32/no-intro" : "selected yaw229/intro") << " expectation\n";
  return failures == 0 ? 0 : 1;
}
