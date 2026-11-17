// actor-anim-001 (playtest P4-P6): lemmings draw the strip of their own state, advance it with level
// time, freeze it while paused, and a lit bomber fuse shows a countdown.
//
// SCRIPTED CLONE OBSERVATION and regression test; not human play and not original-fidelity evidence.
//
//   units  The lemming sheet (SPRITES\LEMMINGSHI.SPR) from the installed archive: every clip the state map uses
//          exists with the temporal counts the retained live reads give, each mapped state draws a clip whose first
//          image differs from the walk clip's, and the tracker advances/holds/wraps on those real clip lengths.
//   app    The public webo::app::runApplication (--level L --sim --native, owned 200 ms clock). Inputs are SDL events
//          queued from the read-only LevelFrameObserver: a plate key, a pointer motion onto a lemming and a left
//          click (the hoveredActor() + World::assign() path a player uses), and a camera grab that follows the
//          lemming. The run is repeated with --no-actor-anim (the view before actor-anim-001: walk entry 0 for every
//          state); the sim is deterministic so the two runs differ only in the drawn strip. Crops of the target in
//          both runs are written to <dir> for review.
//   pause  MM_CLIMB_002: walkers cycle through several temporal entries; with the pause key down they hold the same
//          entry for every paused frame; after resume they move again.
//
// Usage: test_actoranimstate units <dir> | app <dir> <level> <plate> <label> <preState> <expectState> <expectClip>
//        | pause <dir>
#include "app/AppEntry.h"
#include "app/LemmingAnim.h"
#include "app/TubeScene.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "assets/Sprite.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

namespace {

int g_failures = 0;
void expect(bool ok, const std::string& what) {
  if (!ok) {
    std::cerr << "FAIL: actoranimstate: " << what << '\n';
    ++g_failures;
  }
}

void save(const fs::path& path, const Image& img) {
  if (auto r = writePng(path, img); !r)
    expect(false, "cannot write " + path.string() + ": " + r.error());
}

// ---------------------------------------------------------------------------------------------------------------
// units

int unitsMain(const fs::path& outDir, const fs::path& gameDir) {
  auto lib = AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!lib) {
    std::cerr << "FAIL: actoranimstate: open lemmings.box\n";
    return 1;
  }
  auto sheet = loadSpriteSheet(*lib, "SPRITES\\LEMMINGSHI.SPR");
  expect(sheet.has_value(), "load LEMMINGSHI.SPR");
  if (!sheet) return 1;

  std::ofstream table(outDir / "clips.txt");
  table << "clip temporalEntries directionsAtEntry0 rate\n";
  for (const Sprite& s : sheet->sprites)
    table << s.name << ' ' << s.animations.size() << ' '
          << (s.animations.empty() ? 0 : s.animations[0].frames.size()) << ' ' << s.rate << '\n';
  table.close();
  std::cout << "units: " << sheet->sprites.size() << " clips, table in " << (outDir / "clips.txt").string() << '\n';

  // Temporal counts read live (docs/lessons/data-and-simulation.md, test_actoranim): walk 12, bash 24, build 16,
  // dig 20, mine 25, hoist flip 14, umbopen 8, shrug 8.
  const std::map<std::string, std::size_t> evidenced{{"walk", 12}, {"bash", 24}, {"build", 16}, {"dig", 20},
                                                      {"mine", 25}, {"flip", 14}, {"umbopen", 8}, {"shrug", 8}};
  for (const auto& [name, count] : evidenced) {
    const Sprite* s = sheet->find(name);
    expect(s != nullptr, "clip " + name + " exists");
    if (s) expect(s->animations.size() == count, "clip " + name + " has " + std::to_string(count) + " temporal entries, got " +
                                                    std::to_string(s->animations.size()));
  }

  // Every state's clip exists, and the states that must look different from a walker draw a different strip.
  const Sprite* walk = sheet->find("walk");
  expect(walk && !walk->animations.empty() && !walk->animations[0].frames.empty(), "walk clip has images");
  if (!walk || walk->animations.empty() || walk->animations[0].frames.empty()) return 1;
  const SpriteFrame& walkFirst = walk->animations[0].frames[0];
  const auto sameImage = [](const SpriteFrame& a, const SpriteFrame& b) {
    return a.sheet == b.sheet && a.uLeft == b.uLeft && a.uTop == b.uTop && a.uRight == b.uRight && a.uBottom == b.uBottom;
  };
  using sim::State;
  const State states[] = {State::Faller, State::Walker, State::Climber, State::Hoister, State::Floater, State::Blocker,
                          State::Builder, State::Shrugger, State::Basher, State::Miner, State::Digger,
                          State::Exploder, State::Splatter, State::Bounce, State::Drown, State::Teleporting};
  for (State st : states) {
    sim::Lemming l;
    l.state = st;
    const char* name = app::lemmingClipName(l);
    const Sprite* s = sheet->find(name);
    expect(s != nullptr && !s->animations.empty(), std::string("state ") + std::string(sim::stateName(st)) + " clip " + name + " exists");
    if (!s || s->animations.empty() || s->animations[0].frames.empty()) continue;
    if (st != State::Walker)
      expect(!sameImage(s->animations[0].frames[0], walkFirst),
             std::string("state ") + std::string(sim::stateName(st)) + " draws a strip other than walk");
    std::cout << "units: state " << sim::stateName(st) << " -> clip " << name << " (" << s->animations.size()
              << " temporal entries, loops " << app::lemmingClipLoops(name) << ")\n";
  }
  const State keyStates[] = {State::Climber, State::Blocker, State::Builder, State::Basher, State::Miner, State::Digger,
                             State::Faller, State::Floater, State::Exploder};
  std::set<std::string> keyClips;
  for (State st : keyStates) {
    sim::Lemming l;
    l.state = st;
    keyClips.insert(app::lemmingClipName(l));
  }
  expect(keyClips.size() == std::size(keyStates), "climber, blocker, builder, basher, miner, digger, faller, floater and exploder each draw their own clip");

  // The countdown digit: whole seconds, rounded up, 0 when no fuse is lit.
  {
    sim::Lemming l;
    expect(app::bomberCountdownDigit(l) == 0, "no fuse -> no digit");
    l.fuseMs = 5000;
    expect(app::bomberCountdownDigit(l) == 5, "5000 ms reads 5");
    l.fuseMs = 4001;
    expect(app::bomberCountdownDigit(l) == 5, "4001 ms still reads 5");
    l.fuseMs = 4000;
    expect(app::bomberCountdownDigit(l) == 4, "4000 ms reads 4");
    l.fuseMs = 1;
    expect(app::bomberCountdownDigit(l) == 1, "1 ms reads 1");
    l.fuseMs = 0;
    expect(app::bomberCountdownDigit(l) == 0, "run out -> no digit");
  }

  // The tracker on a real World: a walker advances one entry per 0.06 s, holds at dt 0, wraps at the clip length.
  auto setup = sim::loadRetailLevel(*lib, "MM_CLIMB_002");
  expect(setup.has_value(), "loadRetailLevel MM_CLIMB_002");
  if (!setup) return 1;
  sim::World world(*setup);
  std::optional<std::size_t> walker;
  for (int t = 0; t < 400 && !walker; ++t) {
    world.tick();
    for (std::size_t i = 0; i < world.lemmings().size(); ++i)
      if (world.lemmings()[i].state == State::Walker) { walker = i; break; }
  }
  expect(walker.has_value(), "a lemming reached the Walker state in 400 ticks");
  if (!walker) return 1;
  app::LemmingAnimTracker tracker;
  tracker.advance(world, *sheet, 0.0);
  auto pose = tracker.poseOf(*walker);
  expect(std::strcmp(pose.clip, "walk") == 0 && pose.temporalIndex == 0, "a fresh walker starts walk entry 0");
  tracker.advance(world, *sheet, 0.07);
  expect(tracker.poseOf(*walker).temporalIndex == 1, "0.07 s of level time advances one entry");
  tracker.advance(world, *sheet, 0.0);
  tracker.advance(world, *sheet, 0.0);
  expect(tracker.poseOf(*walker).temporalIndex == 1, "zero elapsed (paused) holds the entry");
  std::set<int> seen;
  for (int i = 0; i < 40; ++i) {
    tracker.advance(world, *sheet, 0.05);
    const int idx = tracker.poseOf(*walker).temporalIndex;
    expect(idx >= 0 && idx < 12, "walk index stays inside the 12-entry strip");
    seen.insert(idx);
  }
  expect(seen.size() == 12, "two seconds of walking visits all 12 walk entries (saw " + std::to_string(seen.size()) + ")");
  if (g_failures) return 1;
  std::cout << "PASS: actoranimstate units\n";
  return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// app

bool pushKey(SDL_Scancode sc, SDL_Keycode sym) {
  SDL_Event e{};
  e.type = SDL_KEYDOWN;
  e.key.type = SDL_KEYDOWN;
  e.key.state = SDL_PRESSED;
  e.key.keysym.scancode = sc;
  e.key.keysym.sym = sym;
  return SDL_PushEvent(&e) == 1;
}
bool pushPlate(int index) { return pushKey(static_cast<SDL_Scancode>(SDL_SCANCODE_1 + index), static_cast<SDL_Keycode>(SDLK_1 + index)); }
bool pushMotion(int x, int y, int xrel = 0) {
  SDL_Event e{};
  e.type = SDL_MOUSEMOTION;
  e.motion.type = SDL_MOUSEMOTION;
  e.motion.x = x;
  e.motion.y = y;
  e.motion.xrel = xrel;
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
bool pushRight(bool down, int x, int y) {
  SDL_Event e{};
  e.type = down ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
  e.button.type = e.type;
  e.button.button = SDL_BUTTON_RIGHT;
  e.button.state = down ? SDL_PRESSED : SDL_RELEASED;
  e.button.x = x;
  e.button.y = y;
  return SDL_PushEvent(&e) == 1;
}
bool pushQuit() {
  SDL_Event e{};
  e.type = SDL_QUIT;
  return SDL_PushEvent(&e) == 1;
}

constexpr int kFrames = 420;       // 84 s of simulation at one 200 ms tick per frame
constexpr int kStateFrames = 12;   // frames of the expected state recorded before the run ends
constexpr int kCrop = 64;          // crop size around the target

sim::State parseState(const std::string& s) {
  for (int i = 0; i <= static_cast<int>(sim::State::Teleporting); ++i)
    if (sim::stateName(static_cast<sim::State>(i)) == s) return static_cast<sim::State>(i);
  return sim::State::Dead;
}

Image crop(const Image& src, int cx, int cy) {
  Image out;
  out.width = kCrop;
  out.height = kCrop;
  out.rgba.assign(static_cast<std::size_t>(kCrop) * kCrop * 4, 255);
  for (int y = 0; y < kCrop; ++y)
    for (int x = 0; x < kCrop; ++x) {
      const int sx = cx - kCrop / 2 + x, sy = cy - kCrop / 2 + y;
      if (sx < 0 || sy < 0 || sx >= src.width || sy >= src.height) continue;
      for (int c = 0; c < 4; ++c)
        out.rgba[(static_cast<std::size_t>(y) * kCrop + x) * 4 + c] =
            src.rgba[(static_cast<std::size_t>(sy) * src.width + sx) * 4 + c];
    }
  return out;
}

std::size_t diffPixels(const Image& a, const Image& b) {
  if (a.width != b.width || a.height != b.height) return a.pixelCount();
  std::size_t n = 0;
  for (std::size_t i = 0; i < a.pixelCount(); ++i)
    if (a.rgba[i * 4] != b.rgba[i * 4] || a.rgba[i * 4 + 1] != b.rgba[i * 4 + 1] || a.rgba[i * 4 + 2] != b.rgba[i * 4 + 2]) ++n;
  return n;
}

struct Sample {
  int frame = 0;
  int sinceClick = 0;
  int x = 0, y = 0;
  sim::State state = sim::State::Dead;
  std::string clip;
  int temporal = 0;
  int digit = 0;
  std::size_t countdownPixels = 0;  // (255,230,80) pixels in the band above the sprite
  Image crop;
};

struct AppRun {
  std::string failure;
  int frames = 0;
  bool clicked = false;
  int clickFrame = -1, stateFrame = -1, endFrame = -1;
  int stockBefore = -1, stockAfter = -1;
  bool explosionCompleted = false;
  Image stateImage, finalImage;
  std::vector<Sample> samples;                 // the target, every frame it is on screen from the click on
  std::set<std::string> otherClips;            // clips drawn for any lemming that is not the target
  std::size_t fallerFramesDrawn = 0;           // frames-with-a-faller drawn as "fall"
  std::size_t nonWalkClipsDrawn = 0;           // every actor-frame drawn with a clip other than walk
  std::size_t actorFrames = 0;
};

AppRun runApp(const std::string& level, int plate, sim::State pre, sim::State want, bool noAnim, const char* gameDir) {
  AppRun r;
  std::size_t target = static_cast<std::size_t>(-1);
  std::optional<std::size_t> aim;
  bool aimingGrab = false;
  int followFrames = 0;
  const auto observer = [&](const app::LevelFrameView& f) {
    ++r.frames;
    const int calls = r.frames;
    const auto& w = f.world;
    const auto& lemmings = w.lemmings();
    if (calls == 1) {
      if (!pushPlate(plate)) r.failure = "could not queue the plate key";
      r.stockBefore = w.setup().skills[plate];
    }
    if (calls == 2 && f.selectedSkillIndex != plate) r.failure = "the plate was not selected";

    for (const auto& a : f.actors) {
      if (a.index >= lemmings.size()) continue;
      ++r.actorFrames;
      if (std::strcmp(a.clip, "walk") != 0) ++r.nonWalkClipsDrawn;
      if (lemmings[a.index].state == sim::State::Faller && std::strcmp(a.clip, "fall") == 0) ++r.fallerFramesDrawn;
      if (a.index != target) r.otherClips.insert(a.clip);
    }

    if (!r.clicked && calls >= 3 && !f.cameraIntroActive && !f.levelMenuOpen) {
      // Pause on the same simulation state in both runs, then use the public camera and
      // picker while stationary. Animated sprite boxes need not have the walk[0] centre.
      // A successful assignment resumes through the production input path.
      if (!aim) {
        for (std::size_t i = 0; i < lemmings.size(); ++i) {
          if (lemmings[i].alive() && lemmings[i].state == pre) {
            aim = i;
            pushKey(SDL_SCANCODE_P, SDLK_p);
            break;
          }
        }
      }
      if (aim && f.paused) {
        double delta = std::remainder(lemmings[*aim].x - f.displayedPose.yawUnits, 256.0);
        if (std::abs(delta) > 0.5) {
          if (!aimingGrab) pushRight(true, f.cursorX, f.cursorY);
          aimingGrab = true;
          pushMotion(f.cursorX, f.cursorY, static_cast<int>(std::lround(-delta * 2.4)));
          return;
        }
        if (aimingGrab) {
          pushRight(false, f.cursorX, f.cursorY);
          aimingGrab = false;
          return;
        }
        for (const auto& a : f.actors) {
          if (a.index != *aim) continue;
          const auto& l = lemmings[a.index];
          if (!l.alive() || l.state != pre) continue;
          const int px = static_cast<int>((a.placement.left + a.placement.right) / 2);
          const int py = static_cast<int>((a.placement.top + a.placement.bottom) / 2);
          if (px < 100 || px >= 640 || py < 0 || py >= 480) continue;
          pushMotion(px, py);
          break;
        }
        if (f.hoveredActor == aim && f.selectedSkillIndex == plate && lemmings[*f.hoveredActor].state == pre) {
          pushClick(f.cursorX, f.cursorY);
          r.clicked = true;
          r.clickFrame = calls;
          target = *f.hoveredActor;
          // Park the public cursor away from the actor before grabbing, so its hand image
          // does not cover the very animation/countdown these retained frames demonstrate.
          pushMotion(620, 460);
          // Hold the camera on the target with the player's grab (right button + relative motion).
          pushRight(true, 620, 460);
        }
      }
    }
    if (r.clicked && target < lemmings.size()) {
      // Keep the target's column inside the production actor submission window.
      const auto& t = lemmings[target];
      const auto& t0 = w.terrain();
      const double col = t.x;
      double delta = std::fmod(col - f.displayedPose.yawUnits, static_cast<double>(t0.width()));
      if (delta >= t0.width() / 2.0) delta -= t0.width();
      if (delta < -t0.width() / 2.0) delta += t0.width();
      if (calls > r.clickFrame) pushMotion(f.cursorX, f.cursorY, static_cast<int>(std::lround(-delta * 2.4)));

      for (const auto& a : f.actors) {
        if (a.index != target) continue;
        Sample s;
        s.frame = calls;
        s.sinceClick = calls - r.clickFrame;
        s.x = t.x; s.y = t.y;
        s.state = t.state;
        s.clip = a.clip;
        s.temporal = a.temporalIndex;
        s.digit = app::bomberCountdownDigit(t);
        const int cx = static_cast<int>((a.placement.left + a.placement.right) / 2);
        const int top = static_cast<int>(a.placement.top);
        const int cy = static_cast<int>((a.placement.top + a.placement.bottom) / 2);
        for (int y = std::max(0, top - 34); y < std::min(f.image.height, top + 2); ++y)
          for (int x = std::max(0, cx - 24); x < std::min(f.image.width, cx + 24); ++x) {
            const auto* p = f.image.rgba.data() + (static_cast<std::size_t>(y) * f.image.width + x) * 4;
            if (p[0] == 255 && p[1] == 230 && p[2] == 80) ++s.countdownPixels;
          }
        s.crop = crop(f.image, cx, cy - 8);
        r.samples.push_back(std::move(s));
        if (t.state == want) {
          if (r.stateFrame < 0) {
            r.stateFrame = calls;
            r.stockAfter = w.setup().skills[plate];
            r.stateImage = f.image;
          }
          ++followFrames;
        }
      }
      r.explosionCompleted = want == sim::State::Exploder && r.stateFrame >= 0 && t.state == sim::State::Dead;
      // A bomber runs through the actual blast/removal; others record kStateFrames of their state.
      const bool done = want == sim::State::Exploder ? r.explosionCompleted
                                                      : (followFrames >= kStateFrames);
      if (done && r.endFrame < 0) {
        r.endFrame = calls;
        r.finalImage = f.image;
        pushRight(false, f.cursorX, f.cursorY);
        pushQuit();
      }
    }
    if (calls >= kFrames - 1) pushQuit();
  };

  std::vector<std::string> args{"actoranimstate", "--level", level, "--sim", "--native", "--no-audio", "--frames",
                                std::to_string(kFrames), "--game-dir", gameDir};
  if (noAnim) args.push_back("--no-actor-anim");
  std::vector<char*> raw;
  for (auto& a : args) raw.push_back(a.data());
  app::ApplicationRunOptions options;
  options.frameSeconds = 0.2;
  const int status = app::runApplication(static_cast<int>(raw.size()), raw.data(), {}, observer, options);
  if (status != 0 && r.failure.empty()) r.failure = "runApplication returned failure";
  return r;
}

Image scaled(const Image& src, int k) {
  Image out;
  out.width = src.width * k;
  out.height = src.height * k;
  out.rgba.resize(static_cast<std::size_t>(out.width) * out.height * 4);
  for (int y = 0; y < out.height; ++y)
    for (int x = 0; x < out.width; ++x)
      for (int c = 0; c < 4; ++c)
        out.rgba[(static_cast<std::size_t>(y) * out.width + x) * 4 + c] =
            src.rgba[(static_cast<std::size_t>(y / k) * src.width + x / k) * 4 + c];
  return out;
}

// Row 1: animated run, row 2: --no-actor-anim run, columns: consecutive frames of the target from the first frame it
// is in the wanted state. Crops are scaled 2x.
Image contactSheet(const AppRun& anim, const AppRun& plain, sim::State want, int columns) {
  const auto firstIdx = [&](const AppRun& r) -> std::size_t {
    for (std::size_t i = 0; i < r.samples.size(); ++i)
      if (r.samples[i].state == want) return i;
    return r.samples.size();
  };
  const std::size_t a0 = firstIdx(anim), p0 = firstIdx(plain);
  const int k = 2, cell = kCrop * k;
  Image sheet;
  sheet.width = cell * columns;
  sheet.height = cell * 2;
  sheet.rgba.assign(static_cast<std::size_t>(sheet.width) * sheet.height * 4, 255);
  const auto blit = [&](const Image& img, int col, int row) {
    for (int y = 0; y < cell; ++y)
      for (int x = 0; x < cell; ++x)
        for (int c = 0; c < 4; ++c)
          sheet.rgba[(static_cast<std::size_t>(row * cell + y) * sheet.width + col * cell + x) * 4 + c] =
              img.rgba[(static_cast<std::size_t>(y) * img.width + x) * 4 + c];
  };
  for (int c = 0; c < columns; ++c) {
    if (a0 + c < anim.samples.size()) blit(scaled(anim.samples[a0 + c].crop, k), c, 0);
    if (p0 + c < plain.samples.size()) blit(scaled(plain.samples[p0 + c].crop, k), c, 1);
  }
  return sheet;
}

int appMain(const fs::path& outDir, const fs::path& gameDir, const std::string& level, int plate, const std::string& label,
            const std::string& preName, const std::string& wantName, const std::string& wantClip) {
  const sim::State pre = parseState(preName), want = parseState(wantName);
  if (pre == sim::State::Dead || want == sim::State::Dead) {
    std::cerr << "FAIL: actoranimstate: unknown state name\n";
    return 1;
  }
  const std::string gd = gameDir.string();
  const AppRun anim = runApp(level, plate, pre, want, false, gd.c_str());
  const AppRun plain = runApp(level, plate, pre, want, true, gd.c_str());
  std::ofstream trace(outDir / (label + "-frames.csv"));
  trace << "mode,frame,sinceClick,state,x,y,clip,temporal,countdown,countdownPixels\n";
  for (const auto& [mode, run] : {std::pair{"animated", &anim}, std::pair{"legacy", &plain}})
    for (const Sample& s : run->samples)
      trace << mode << ',' << s.frame << ',' << s.sinceClick << ',' << sim::stateName(s.state) << ','
            << s.x << ',' << s.y << ',' << s.clip << ',' << s.temporal << ',' << s.digit << ',' << s.countdownPixels << '\n';

  const auto report = [&](const char* name, const AppRun& r) {
    std::cout << "app " << level << '/' << label << ' ' << name << ": frames " << r.frames << " click@" << r.clickFrame
              << " state@" << r.stateFrame << " end@" << r.endFrame << " stock " << r.stockBefore << "->" << r.stockAfter
              << " actor-frames " << r.actorFrames << " non-walk " << r.nonWalkClipsDrawn << " faller-as-fall "
              << r.fallerFramesDrawn << (r.failure.empty() ? "" : " FAILURE ") << r.failure << '\n';
  };
  report("anim", anim);
  report("plain", plain);
  expect(anim.failure.empty(), label + ": anim run: " + anim.failure);
  expect(plain.failure.empty(), label + ": plain run: " + plain.failure);
  expect(anim.clicked && plain.clicked, label + ": a public click assigned the skill in both runs");
  expect(anim.stockAfter == anim.stockBefore - 1, label + ": the skill count dropped by one");
  expect(anim.stateFrame >= 0, label + ": the target reached state " + wantName);
  expect(anim.stateFrame - anim.clickFrame == plain.stateFrame - plain.clickFrame,
         label + ": both runs saw the same simulation");
  if (anim.stateFrame < 0) return 1;

  // BEFORE: with --no-actor-anim every actor in every frame is drawn walk entry 0.
  expect(plain.nonWalkClipsDrawn == 0, label + ": before the change no actor was drawn with a clip other than walk");
  for (const Sample& s : plain.samples)
    expect(s.clip == "walk" && s.temporal == 0, label + ": before: " + std::string(sim::stateName(s.state)) + " drew " + s.clip +
                                                    "[" + std::to_string(s.temporal) + "]");

  // AFTER: the target's strip follows its state and the entry moves with level time.
  std::set<int> temporals;
  std::set<std::string> clips;
  std::size_t wrongClip = 0, inState = 0;
  for (const Sample& s : anim.samples) {
    if (s.state != want) continue;
    ++inState;
    clips.insert(s.clip);
    if (want == sim::State::Floater ? (s.clip != "umbopen" && s.clip != "float") : s.clip != wantClip) ++wrongClip;
    temporals.insert(s.temporal);
  }
  std::cout << "app " << level << '/' << label << ": " << inState << " frames in state " << wantName << ", clips";
  for (const auto& c : clips) std::cout << ' ' << c;
  std::cout << ", temporal entries seen";
  for (int t : temporals) std::cout << ' ' << t;
  std::cout << '\n';
  expect(inState >= 4, label + ": the state was on screen for at least 4 frames (" + std::to_string(inState) + ")");
  expect(wrongClip == 0, label + ": every frame in state " + wantName + " drew clip " + wantClip);
  expect(clips.count(want == sim::State::Floater ? std::string("umbopen") : wantClip) == 1,
         label + ": clip " + wantClip + " was drawn");
  expect(temporals.size() >= 2, label + ": the strip advanced (" + std::to_string(temporals.size()) + " distinct entries)");

  // Pixels: in the wanted state the animated frame differs from the walk-entry-0 frame.
  std::size_t differing = 0, compared = 0;
  for (const Sample& a : anim.samples) {
    if (a.state != want) continue;
    for (const Sample& p : plain.samples)
      if (p.sinceClick == a.sinceClick) {
        expect(p.state == a.state && p.x == a.x && p.y == a.y,
               label + ": equal state and position at equal elapsed time since assignment");
        ++compared;
        if (diffPixels(a.crop, p.crop) >= 30) ++differing;
      }
  }
  std::cout << "app " << level << '/' << label << ": " << differing << "/" << compared << " state frames differ from the walk frame by >= 30 pixels\n";
  expect(compared > 0 && differing * 2 >= compared, label + ": the drawn picture differs from the old walk picture");

  // Floater scenario: the same run also shows the plain fall strip for a faller.
  if (want == sim::State::Floater) {
    expect(anim.fallerFramesDrawn > 0, label + ": a Faller was drawn with the fall clip");
    std::set<int> fallTemporals;
    for (const Sample& s : anim.samples)
      if (s.state == sim::State::Faller && s.clip == "fall") fallTemporals.insert(s.temporal);
    std::cout << "app " << level << '/' << label << ": fall entries seen for the target:";
    for (int t : fallTemporals) std::cout << ' ' << t;
    std::cout << '\n';
  }

  // Bomber: a countdown above the lemming that decreases to the explosion, visible in the pixels.
  if (want == sim::State::Exploder) {
    std::vector<int> digits;
    std::map<int, std::size_t> pixelsAtDigit;
    Image firstOf[6];
    bool haveFirst[6] = {};
    for (const Sample& s : anim.samples) {
      if (s.digit <= 0 || s.digit > 5) continue;
      if (digits.empty() || digits.back() != s.digit) digits.push_back(s.digit);
      pixelsAtDigit[s.digit] = std::max(pixelsAtDigit[s.digit], s.countdownPixels);
      if (!haveFirst[s.digit]) { haveFirst[s.digit] = true; firstOf[s.digit] = s.crop; }
    }
    std::cout << "app " << level << '/' << label << ": countdown digits seen on screen:";
    for (int d : digits) std::cout << ' ' << d << "(" << pixelsAtDigit[d] << "px)";
    std::cout << '\n';
    expect(digits.size() >= 4, label + ": at least four different countdown digits were shown (" + std::to_string(digits.size()) + ")");
    expect(std::is_sorted(digits.begin(), digits.end(), std::greater<int>()) && std::adjacent_find(digits.begin(), digits.end()) == digits.end(),
           label + ": the countdown only decreases");
    // Asset measurement from development-1: the narrow '1' has 17 opaque pixels; others >=29.
    for (int d : digits) expect(pixelsAtDigit[d] >= (d == 1 ? 17u : 20u), label + ": digit " + std::to_string(d) + " is drawn (" + std::to_string(pixelsAtDigit[d]) + " px)");
    // Different digits are different pictures above the lemming.
    for (std::size_t i = 1; i < digits.size(); ++i)
      expect(diffPixels(firstOf[digits[i - 1]], firstOf[digits[i]]) > 0, label + ": digit " + std::to_string(digits[i - 1]) + " and " + std::to_string(digits[i]) + " differ");
    // The explosion state follows the last digit.
    bool exploded = false;
    for (const Sample& s : anim.samples) exploded |= s.state == sim::State::Exploder && s.clip == "explode";
    expect(exploded, label + ": the fused lemming went on to the exploder state and its explode clip");
    expect(anim.explosionCompleted && plain.explosionCompleted, label + ": both routes continued to the actual blast and actor removal");
    // One crop per distinct digit side by side, 2x, for review.
    Image row;
    row.width = kCrop * 2 * static_cast<int>(std::max<std::size_t>(digits.size(), 1));
    row.height = kCrop * 2;
    row.rgba.assign(static_cast<std::size_t>(row.width) * row.height * 4, 255);
    for (std::size_t i = 0; i < digits.size(); ++i) {
      const Image big = scaled(firstOf[digits[i]], 2);
      for (int y = 0; y < big.height; ++y)
        for (int x = 0; x < big.width; ++x)
          for (int c = 0; c < 4; ++c)
            row.rgba[(static_cast<std::size_t>(y) * row.width + static_cast<int>(i) * big.width + x) * 4 + c] =
                big.rgba[(static_cast<std::size_t>(y) * big.width + x) * 4 + c];
    }
    save(outDir / (label + "-countdown.png"), row);
  }

  save(outDir / (label + "-before-after.png"), contactSheet(anim, plain, want, kStateFrames));
  save(outDir / (label + "-animated-frame.png"), anim.stateImage);
  save(outDir / (label + "-legacy-frame.png"), plain.stateImage);
  if (want == sim::State::Exploder) save(outDir / (label + "-blast-frame.png"), anim.finalImage);
  if (want == sim::State::Climber)
    save(outDir / "walker-before-after.png", contactSheet(anim, plain, sim::State::Walker, kStateFrames));
  if (want == sim::State::Floater)
    save(outDir / "faller-before-after.png", contactSheet(anim, plain, sim::State::Faller, kStateFrames));
  if (g_failures) return 1;
  std::cout << "PASS: actoranimstate app " << label << '\n';
  return 0;
}

// ---------------------------------------------------------------------------------------------------------------
// pause

int pauseMain(const fs::path& outDir, const fs::path& gameDir) {
  std::vector<std::map<std::size_t, int>> frames;  // per observed frame: lemming index -> temporal entry (walkers only)
  std::vector<bool> paused;
  std::string failure;
  Image running, pausedImage, heldImage;
  int calls = 0;
  const auto observer = [&](const app::LevelFrameView& f) {
    ++calls;
    std::map<std::size_t, int> m;
    for (const auto& a : f.actors)
      if (std::strcmp(a.clip, "walk") == 0 && f.world.lemmings()[a.index].state == sim::State::Walker) m[a.index] = a.temporalIndex;
    frames.push_back(m);
    paused.push_back(f.paused);
    if (calls == 60) running = f.image;
    if (calls == 70) pausedImage = f.image;
    if (calls == 79) heldImage = f.image;
    if (calls == 60 || calls == 80) pushKey(SDL_SCANCODE_P, SDLK_p);  // pause at 60, resume at 80
    if (calls >= 100) pushQuit();
  };
  const std::string gd = gameDir.string();
  std::vector<std::string> args{"actoranimstate", "--level", "MM_CLIMB_002", "--sim", "--native", "--no-audio", "--frames", "140",
                                "--game-dir", gd};
  std::vector<char*> raw;
  for (auto& a : args) raw.push_back(a.data());
  app::ApplicationRunOptions options;
  options.frameSeconds = 0.2;
  const int status = app::runApplication(static_cast<int>(raw.size()), raw.data(), {}, observer, options);
  expect(status == 0, "runApplication returned success");

  std::set<int> seenBefore, seenAfter;
  std::size_t pausedFrames = 0, heldFrames = 0;
  std::map<std::size_t, int> held;
  int firstPaused = -1, lastPaused = -1;
  for (std::size_t i = 0; i < frames.size(); ++i) {
    if (!paused[i]) {
      if (firstPaused < 0) for (const auto& [idx, t] : frames[i]) seenBefore.insert(t);
      else if (lastPaused >= 0) for (const auto& [idx, t] : frames[i]) seenAfter.insert(t);
      continue;
    }
    if (firstPaused < 0) { firstPaused = static_cast<int>(i); held = frames[i]; }
    lastPaused = static_cast<int>(i);
    ++pausedFrames;
    if (frames[i] == held && !held.empty()) ++heldFrames;
  }
  std::cout << "pause: frames " << frames.size() << ", paused frames " << pausedFrames << " (first " << firstPaused << ", last "
            << lastPaused << "), held identical " << heldFrames << ", walker entries seen before pause " << seenBefore.size()
            << ", after resume " << seenAfter.size() << '\n';
  expect(seenBefore.size() >= 4, "running walkers drew at least 4 different walk entries (" + std::to_string(seenBefore.size()) + ")");
  expect(pausedFrames >= 10, "the pause key paused at least 10 frames (" + std::to_string(pausedFrames) + ")");
  expect(heldFrames == pausedFrames, "while paused every walker held its walk entry (" + std::to_string(heldFrames) + "/" +
                                          std::to_string(pausedFrames) + ")");
  expect(seenAfter.size() >= 2, "after resume the walkers animate again (" + std::to_string(seenAfter.size()) + " entries)");
  if (running.width > 0 && pausedImage.width > 0) {
    save(outDir / "pause-running.png", running);
    save(outDir / "pause-held.png", pausedImage);
    save(outDir / "pause-held-later.png", heldImage);
  }
  if (g_failures) return 1;
  std::cout << "PASS: actoranimstate pause\n";
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  const char* driver = std::getenv("SDL_VIDEODRIVER");
  if (!driver || std::string(driver) != "dummy") {
    std::cerr << "actoranimstate: refusing to run without SDL_VIDEODRIVER=dummy\n";
    return 1;
  }
  const char* gameDirEnv = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDirEnv || !*gameDirEnv || !fs::is_regular_file(fs::path(gameDirEnv) / "lemmings.box")) {
    std::cout << "actoranimstate: SKIPPED (retail assets unavailable)\n";
    return 0;
  }
  const std::string mode = argc > 1 ? argv[1] : "units";
  const fs::path outDir = argc > 2 ? fs::path(argv[2]) : fs::path("build/actor-anim-001");
  std::error_code ec;
  fs::create_directories(outDir, ec);
  if (mode == "units") return unitsMain(outDir, gameDirEnv);
  if (mode == "pause") return pauseMain(outDir, gameDirEnv);
  if (mode == "app" && argc > 8) return appMain(outDir, gameDirEnv, argv[3], std::atoi(argv[4]), argv[5], argv[6], argv[7], argv[8]);
  std::cerr << "usage: test_actoranimstate units <dir> | pause <dir> | app <dir> <level> <plate> <label> <preState> <expectState> <expectClip>\n";
  return 1;
}
#else
#include <iostream>
int main() {
  std::cerr << "actoranimstate: requires SDL2\n";
  return 1;
}
#endif
