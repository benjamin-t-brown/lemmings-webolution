// P9-P11 clone observations; no original execution. Baseline uses frozen source.
#include "app/AppEntry.h"
#include "app/BackdropScene.h"
#include "app/HatchAnimation.h"
#include "app/RuntimeHud.h"
#include "app/RuntimePick.h"
#include "app/Raster.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#include "lemmings/compiled_level.hpp"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
using namespace webo;
namespace fs = std::filesystem;
namespace {
#ifdef HATCH_EXIT_BASELINE
constexpr bool baseline = true;
#else
constexpr bool baseline = false;
#endif
int failures = 0;
void check(bool ok, const std::string& s) {
  if (!ok && ++failures < 30)
    std::cerr << "FAIL: " << s << '\n';
}
std::string bytes(const fs::path& p) {
  std::ifstream in(p, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}
struct SaveSnapshot {
  fs::path path;
  bool exists;
  std::string data;
  explicit SaveSnapshot(fs::path p)
      : path(std::move(p)), exists(fs::exists(path)), data(bytes(path)) {}
  bool unchanged() const { return exists == fs::exists(path) && data == bytes(path); }
};
void png(const fs::path& p, const Image& im) {
  check(writePng(p, im).has_value(), "PNG " + p.string());
}
std::size_t changed(const Image& a, const Image& b, int left, int top, int width,
                    int height) {
  std::size_t count = 0;
  for (int y = top; y < top + height; ++y)
    for (int x = left; x < left + width; ++x) {
      auto i = (y * a.width + x) * 4;
      count +=
          !std::equal(a.rgba.begin() + i, a.rgba.begin() + i + 3, b.rgba.begin() + i);
    }
  return count;
}
[[maybe_unused]] Image image(const app::Framebuffer& fb) {
  Image im;
  im.width = fb.width;
  im.height = fb.height;
  for (std::size_t i = 0; i < fb.rgb.size(); i += 3)
    im.rgba.insert(im.rgba.end(), {fb.rgb[i], fb.rgb[i + 1], fb.rgb[i + 2], 255});
  return im;
}
void push(SDL_Event e) { check(SDL_PushEvent(&e) == 1, "queue public SDL input"); }
void click(int x, int y) {
  SDL_Event e{};
  e.type = SDL_MOUSEBUTTONDOWN;
  e.button.button = SDL_BUTTON_LEFT;
  e.button.x = x;
  e.button.y = y;
  push(e);
  e.type = SDL_MOUSEBUTTONUP;
  push(e);
}
void key(SDL_Scancode k) {
  SDL_Event e{};
  e.type = SDL_KEYDOWN;
  e.key.keysym.scancode = k;
  e.key.keysym.sym = SDL_GetKeyFromScancode(k);
  push(e);
}
void quit() {
  SDL_Event e{};
  e.type = SDL_QUIT;
  push(e);
}
AssetLibrary library() {
  fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  auto lib = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!lib)
    throw std::runtime_error(lib.error());
  return std::move(*lib);
}
[[maybe_unused]] std::vector<std::byte> levelBytes(const AssetLibrary& lib,
                                                   const std::string& id) {
  auto r = sim::resolveRetailLevel(lib, id);
  if (!r)
    throw std::runtime_error(r.error());
  auto b = lib.read(r->file);
  if (!b)
    throw std::runtime_error(b.error());
  return std::move(*b);
}
std::string state(const app::LevelFrameView& f) {
  const auto& w = f.world;
  std::ostringstream s;
  s << std::setprecision(17) << f.frame << ',' << w.levelClockMs() << ',' << w.tickCount()
    << ',' << w.explicitUpdateCount() << ',' << w.spawned() << ',' << w.saved() << ','
    << w.dead() << ',' << w.finished() << ',' << w.won() << ',' << w.terrain().version()
    << ',' << int(f.endPhase) << ',' << f.endTimerSeconds << ',' << f.paused;
  for (int stock : w.setup().skills)
    s << ',' << stock;
  for (auto& l : w.lemmings()) {
    s << ';' << l.x << ',' << l.y << ',' << int(l.state) << ',' << int(l.facing) << ','
      << l.fallDistance << ',' << l.frame << ',' << l.bricksLeft << ',' << l.bashCycles
      << ',' << l.fuseMs << ',' << l.fasterMs << ',' << l.speedPadLatched << ','
      << l.canClimb << ',' << l.canFloat << ',' << l.verticalDir << ',' << l.sheetType
      << ',' << l.stepBudgetSeconds;
    for (auto p : l.teleportPadsLatched)
      s << ':' << p;
  }
  return s.str();
}
struct PairTrace {
  std::ofstream out;
  std::ifstream expected;
  bool compare;
  PairTrace(const fs::path& p, const fs::path& b)
      : out(p / "simulation.txt"),
        compare(!baseline && fs::exists(b / "simulation.txt")) {
    if (compare)
      expected.open(b / "simulation.txt");
  }
  void frame(const app::LevelFrameView& f) {
    auto s = state(f);
    out << s << '\n';
    if (compare) {
      std::string want;
      check(bool(std::getline(expected, want)) && want == s,
            "full simulation identity at frame " + std::to_string(f.frame));
    }
  }
  void finish() {
    if (compare) {
      std::string s;
      check(!std::getline(expected, s), "identical frame counts");
    }
  }
};
int assets(const fs::path& out) {
  auto lib = library();
  std::ofstream census(out / "census.txt");
  int levels = 0, hatches = 0, animated = 0, exits = 0;
  for (auto& file : levelFileNames(lib)) {
    auto b = lib.read(file);
    if (!b)
      return 1;
    auto level = lemmings::read_compiled_level(
        {reinterpret_cast<const uint8_t*>(b->data()), b->size()});
#ifndef HATCH_EXIT_BASELINE
    // Exercise the same bindings the application builds, not just the key
    // decoder, on every real installed level and both hatch clip names.
    auto backdrop = app::BackdropScene::build(lib, *b, file);
    check(bool(backdrop), "installed production backdrop loads");
    if (backdrop) {
      auto presentation = backdrop->entranceExitPresentation(*b);
      check(bool(presentation), "installed production presentation loads");
      if (presentation) {
        check(presentation->entrances.size() == level.gadgets.entities_425e50.size(),
              "every installed hatch bound to its production door tracks");
        check(presentation->exits.size() == level.gadgets.entities_426c6b.size(),
              "every installed exit bound to its production root");
      }
    }
#endif
    ++levels;
    census << file;
    for (auto& e : level.gadgets.entities_425e50) {
      ++hatches;
      check(e.model && e.model->object, "hatch inline model");
      if (!e.model || !e.model->object)
        continue;
      auto tree = readSceneNode(*b, e.model->object->source_extent.typed_root_begin);
      check(bool(tree), "hatch tree parses");
      if (!tree)
        continue;
      check(std::abs(tree->root.matrix[10] - e.shorts_98_9a_9c[2] - 2.0) < 1e-6,
            "stored spawn is two units below opening");
      bool found = false;
      for (auto& aux : e.model->object->auxiliary_records)
        if (aux.name == "entrance" || aux.name == "entrance_acid") {
          auto animation = app::readHatchAnimation(aux.serialized_bytes);
          check(bool(animation), "installed door animation parses");
          if (!animation)
            continue;
          check(animation->tracks.size() == 2 &&
                    std::abs(animation->duration - 2.8) < 1e-6,
                "two door tracks, serialized 2.8 second duration");
          for (auto& t : animation->tracks) {
            check((t.node == 2 || t.node == 3) && t.keys.size() == 70,
                  "door node index and key count");
            for (auto& key : t.keys)
              check(key.linear,
                    "installed door keys use linear quaternion interpolation");
          }
          ++animated;
          found = true;
          if (file.find("MM_CLIMB_002") != std::string::npos) {
            std::ofstream keys(out / "MM_CLIMB_002-keys.csv");
            keys << std::setprecision(17)
                 << "node,key,end,sx,sy,sz,x,y,z,w,qx,qy,qz,linear\n";
            for (auto& t : animation->tracks)
              for (std::size_t k = 0; k < t.keys.size(); ++k) {
                auto& key = t.keys[k];
                keys << t.node << ',' << k << ',' << key.end;
                for (auto v : key.scale)
                  keys << ',' << v;
                for (auto v : key.position)
                  keys << ',' << v;
                for (auto v : key.quaternion)
                  keys << ',' << v;
                keys << ',' << key.linear << '\n';
              }
          }
        }
      if (!found) {
        census << " MISSING model=" << e.model->name;
        for (auto& aux : e.model->object->auxiliary_records)
          census << " aux=" << aux.name;
      }
      check(found, "hatch has entrance animation");
    }
    exits += int(level.gadgets.entities_426c6b.size());
    census << " hatches=" << level.gadgets.entities_425e50.size()
           << " exits=" << level.gadgets.entities_426c6b.size() << '\n';
  }
  const std::array<uint8_t, 8> truncated{};
  check(!app::readHatchAnimation(truncated), "malformed animation rejected");
  check(levels == 102 && hatches == 144 && animated == hatches,
        "all installed hatch animations measured");
  std::ofstream(out / "summary.txt")
      << "levels=" << levels << "\nhatches=" << hatches << "\nanimated=" << animated
      << "\nexits=" << exits << "\nfailures=" << failures << '\n';
  return failures ? 1 : 0;
}
int models([[maybe_unused]] const fs::path& out) {
#ifndef HATCH_EXIT_BASELINE
  auto lib = library();
  auto b = levelBytes(lib, "MM_CLIMB_002");
  auto scene = app::BackdropScene::build(lib, b, "MM_CLIMB_002");
  check(bool(scene), "backdrop loads");
  if (!scene)
    return 1;
  auto presentation = scene->entranceExitPresentation(b);
  check(bool(presentation), "installed presentation data loads");
  if (!presentation)
    return 1;
  std::array<double, 12> closed{}, opened{}, raised{};
  auto door = [&](auto& m) {
    scene->visitRoots([&](auto& src, auto& n) {
      if (src.owner == "entities_425e50")
        m = n.children.at(1).matrix;
    });
  };
  std::vector<std::vector<uint8_t>> frames;
  for (double p : {0.0, 0.3, 1.0}) {
    scene->setEntranceExitPresentation(*presentation, p, 0);
    if (p == 0)
      door(closed);
    if (p == 1)
      door(opened);
    app::Raster raster(640, 480);
    raster.clear(0, 0, 0);
    app::BackdropStats stats;
    scene->renderStage(raster, app::buildCamera({}), lib, stats,
                       app::BackdropScene::Stage::LegacyUnattributed);
    frames.push_back(raster.framebuffer().rgb);
    png(out / ("door-" + std::to_string(p) + ".png"), image(raster.framebuffer()));
  }
  check(closed != opened && frames[0] != frames[1] && frames[1] != frames[2],
        "door transforms change rendered pixels");
  check(opened[1] < -0.99, "left door swings down around serialized hinge");
  scene->setEntranceExitPresentation(*presentation, 1, 3);
  scene->visitRoots([&](auto& src, auto& n) {
    if (src.owner == "entities_426c6b")
      raised = n.matrix;
  });
  scene->setEntranceExitPresentation(*presentation, 1, 0);
  scene->visitRoots([&](auto& src, auto& n) {
    if (src.owner == "entities_426c6b")
      check(std::abs(raised[10] - n.matrix[10] - 15) < 1e-9,
            "balloon rises five units per second");
  });
  door(raised);
  check(raised == opened, "absolute sampling avoids accumulated door drift");
  scene->setEntranceExitPresentation(*presentation, 0, 0);
  door(raised);
  check(raised == closed, "retry resets closed doors");
  std::ofstream(out / "summary.txt") << "failures=" << failures << '\n';
#endif
  return failures ? 1 : 0;
}
int application(const fs::path& out) {
  fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  auto local = save::localAppDataDirectory();
  if (!local)
    return 2;
  SaveSnapshot installed(game / "SavedGame.dat"),
      normal(save::cloneSavePathUnder(*local));
  for (std::string id : {"MM_CLIMB_002", "MM_FLOAT"}) {
    auto dir = out / id;
    fs::create_directories(dir);
    PairTrace pair(dir, out.parent_path() / "before" / id);
    std::ofstream trace(dir / "frames.csv");
    trace << "frame,ms,spawned,x,y,state,clip,draw_y\n";
    std::vector<std::string> args{"hatch-exit", "--level",    id,
                                  "--sim",      "--native",   "--frames",
                                  "180",        "--game-dir", game.string()};
    std::vector<char*> raw;
    for (auto& a : args)
      raw.push_back(a.data());
    int falling = 0, landed = 0, pausedFrames = 0, priorY = -1;
    double drawnY = -1;
    Image frozen, closed;
    auto observer = [&](const app::LevelFrameView& f) {
      pair.frame(f);
      bool shot = f.frame == 0 || f.frame == 10 || f.frame == 20 || f.frame == 35 ||
                  f.frame == 45;
      if (f.frame == 0)
        closed = f.image;
      if (id == "MM_CLIMB_002" && f.frame == 45) {
        auto pixels = changed(closed, f.image, 295, 225, 50, 60);
        check(baseline ? pixels == 0 : pixels > 20,
              "door pixels open before any release");
        check(f.world.spawned() == 0,
              "doors finish before first release without changing release time");
      }
      if (f.frame == 11)
        key(SDL_SCANCODE_P);
      if (f.frame == 15)
        key(SDL_SCANCODE_P);
      if (f.paused) {
        if (pausedFrames++ == 0)
          frozen = f.image;
        else
          check(frozen.rgba == f.image.rgba, "pause freezes hatch and actors");
      }
      if (!f.world.lemmings().empty()) {
        auto& l = f.world.lemmings()[0];
        auto a = std::find_if(f.actors.begin(), f.actors.end(),
                              [](auto& a) { return a.index == 0; });
        trace << f.frame << ',' << f.world.levelClockMs() << ',' << f.world.spawned()
              << ',' << l.x << ',' << l.y << ',' << sim::stateName(l.state);
        if (a != f.actors.end()) {
          double y = f.world.terrain().height() - 1 - a->drawnSample[1];
          trace << ',' << a->clip << ',' << y;
          if (l.state == sim::State::Faller) {
            check(std::string(a->clip) == "fall", "released faller draws fall clip");
            check(priorY < 0 || l.y >= priorY, "faller descends");
            check(drawnY < 0 || y >= drawnY - 1e-8, "displayed fall is monotonic");
            priorY = l.y;
            drawnY = y;
            ++falling;
            if (falling % 3 == 1)
              shot = true;
          }
          if (l.state == sim::State::Walker && !landed++) {
            shot = true;
            check(priorY >= 0, "landing follows visible fall");
          }
        }
        trace << '\n';
      }
      if (shot)
        png(dir / ("frame-" + std::to_string(f.frame) + ".png"), f.image);
    };
    app::ApplicationRunOptions options;
    options.frameSeconds = 0.02;
    options.saveStore = std::make_shared<save::SaveStore>(
        std::make_unique<save::FileSaveSink>(dir / "SavedGame.dat", game));
    check(app::runApplication(int(raw.size()), raw.data(), {}, observer, options) == 0,
          "installed entry path exits");
    pair.finish();
    check(falling >= 5 && landed && pausedFrames == 4,
          "visible drop, landing and pause on " + id);
    std::ofstream(dir / "summary.txt")
        << "visible_fall_frames=" << falling << "\nlanded=" << bool(landed)
        << "\npaused_frames=" << pausedFrames << "\npaired=" << pair.compare
        << "\nfailures=" << failures << '\n';
  }
  check(installed.unchanged() && normal.unchanged(), "entry preserves both real saves");
  std::ofstream(out / "summary.txt")
      << "installed_save_unchanged=" << installed.unchanged()
      << "\nnormal_save_unchanged=" << normal.unchanged() << "\nfailures=" << failures
      << '\n';
  return failures ? 1 : 0;
}
int endRoute(const fs::path& out) {
  fs::path game(std::getenv("WEBOLUTION_GAME_DIR"));
  auto local = save::localAppDataDirectory();
  if (!local)
    return 2;
  SaveSnapshot installed(game / "SavedGame.dat"),
      normal(save::cloneSavePathUnder(*local));
  PairTrace pair(out, out.parent_path() / "end-before");
  int stage = 0, pending = 0, pauseFrames = 0;
  bool success = false;
  double timer = 0;
  Image frozen, stationary;
  std::size_t balloonPixels = 0;
  auto front = [&](const app::FrontendSession& s, int) {
    if (s.screen() == shell::Screen::Menu || s.screen() == shell::Screen::Confirm)
      key(SDL_SCANCODE_RETURN);
    else if (s.screen() == shell::Screen::Map)
      click(206, 204);
    else if (s.screen() == shell::Screen::Results) {
      check(pending > 10, "departure frames precede results");
      check(s.outcome() && s.outcome()->success && s.outcome()->saved == 10,
            "natural route wins 10/10");
      success = true;
      png(out / "results.png", s.composeFrame());
      quit();
    }
  };
  auto observer = [&](const app::LevelFrameView& f) {
    pair.frame(f);
    // Retain entry through the ordinary map path as well as the fixed-camera
    // direct-level samples. The existing entry title can cover the first
    // birth; the second release happens after it retires.
    if (f.frame < 5)
      png(out / ("entry-" + std::to_string(f.frame) + ".png"), f.image);
    if (!f.world.lemmings().empty() && f.world.spawned() <= 2 &&
        f.world.lemmings().back().state == sim::State::Faller)
      png(out / ("drop-" + std::to_string(f.world.spawned()) + "-" +
                 std::to_string(f.frame) + ".png"),
          f.image);
    for (auto& a : f.actors) {
      auto& l = f.world.lemmings()[a.index];
      if (!l.active() || l.canClimb ||
          !app::firstLevelClimberSupport(l, sim::Skill::Climber, f.world.terrain())
               .value_or(false))
        continue;
      int x = int((a.placement.left + a.placement.right) / 2),
          y = int((a.placement.top + a.placement.bottom) / 2);
      if (x >= 100 && x < 640 && y >= 0 && y < 480) {
        click(x, y);
        break;
      }
    }
    if (f.endPhase == app::LevelEndPhase::Pending) {
      ++pending;
      if (pending == 1)
        stationary = f.image;
      if (pending == 30) {
        balloonPixels = changed(stationary, f.image, 45, 130, 55, 200);
        check(baseline ? balloonPixels == 0 : balloonPixels > 50,
              "actual balloon pixels depart before results");
      }
      if (stage == 0) {
        stage = 1;
        timer = f.endTimerSeconds;
        key(SDL_SCANCODE_P);
      }
      if (f.paused) {
        check(timer == f.endTimerSeconds, "pause freezes departure timer");
        if (pauseFrames++ == 0)
          frozen = f.image;
        else
          check(frozen.rgba == f.image.rgba, "pause freezes balloon pixels");
        if (pauseFrames == 3) {
          key(SDL_SCANCODE_P);
          stage = 2;
        }
      }
      if (pending == 1 || pending == 7 || pending == 12 || pending == 20 || pending == 30)
        png(out / ("pending-" + std::to_string(pending) + ".png"), f.image);
    }
  };
  std::vector<std::string> args{"hatch-exit-end", "--frontend", "--native",   "--frames",
                                "1500",           "--game-dir", game.string()};
  std::vector<char*> raw;
  for (auto& a : args)
    raw.push_back(a.data());
  app::ApplicationRunOptions options;
  options.frameSeconds = 0.2;
  options.saveStore = std::make_shared<save::SaveStore>(
      std::make_unique<save::FileSaveSink>(out / "SavedGame.dat", game));
  check(app::runApplication(int(raw.size()), raw.data(), front, observer, options) == 0,
        "connected results route exits");
  check(success && pauseFrames == 3, "win and pending pause completed");
  pair.finish();
  check(installed.unchanged() && normal.unchanged(),
        "end route preserves both real saves");
  std::ofstream(out / "summary.txt")
      << "success=" << success << "\npending_frames=" << pending
      << "\npaused_frames=" << pauseFrames << "\nballoon_pixels_changed=" << balloonPixels
      << "\npaired=" << pair.compare
      << "\ninstalled_save_unchanged=" << installed.unchanged()
      << "\nnormal_save_unchanged=" << normal.unchanged() << "\nfailures=" << failures
      << '\n';
  return failures ? 1 : 0;
}
}  // namespace
int main(int argc, char** argv) {
  if (argc != 3 || !std::getenv("WEBOLUTION_GAME_DIR"))
    return 2;
  fs::path out(argv[2]);
  fs::create_directories(out);
  std::string mode(argv[1]);
  if (mode == "assets")
    return assets(out);
  if (!std::getenv("SDL_VIDEODRIVER") ||
      std::string(std::getenv("SDL_VIDEODRIVER")) != "dummy")
    return 2;
  if (mode == "models")
    return models(out);
  if (mode == "end")
    return endRoute(out);
  return application(out);
}
