// Drive Basher assignments through webo::app::runApplication on MM_BASH_002.
// Use 200 ms frames and SDL mouse input after confirming hoveredActor(); the click calls World::assign().
//
// Usage: test_basherfix probe <dir> <gap> | assert <dir> | away <dir>
// Probe takes a cell gap. Assert tunnels from two cells short. Away starts 64 cells short on the ledge.
#include "app/AppEntry.h"
#include "sim/World.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#if WEBOLUTION_HAVE_SDL2
namespace fs = std::filesystem;
using namespace webo;

namespace {

constexpr int kBasherSkillIndex = 5;
constexpr int kFrames = 1700;  // 340 s of simulation at one 200 ms tick per frame (level limit is 300 s)

bool pushKey(int index) {  // keys 1..8 select panel plates 0..7
  SDL_Event e{};
  e.type = SDL_KEYDOWN;
  e.key.type = SDL_KEYDOWN;
  e.key.state = SDL_PRESSED;
  e.key.keysym.scancode = static_cast<SDL_Scancode>(SDL_SCANCODE_1 + index);
  e.key.keysym.sym = static_cast<SDL_Keycode>(SDLK_1 + index);
  return SDL_PushEvent(&e) == 1;
}
bool pushMotion(int x, int y) {
  SDL_Event e{};
  e.type = SDL_MOUSEMOTION;
  e.motion.type = SDL_MOUSEMOTION;
  e.motion.x = x;
  e.motion.y = y;
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
// Right button press/release: the production camera grab (hold the right button and drag to turn the view).
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
// Relative motion while the right button is held: CameraController turns the view by -(10/24) yaw units
// per pixel of xrel.
bool pushDrag(int xrel) {
  SDL_Event e{};
  e.type = SDL_MOUSEMOTION;
  e.motion.type = SDL_MOUSEMOTION;
  e.motion.x = 320;
  e.motion.y = 240;
  e.motion.xrel = xrel;
  return SDL_PushEvent(&e) == 1;
}
bool pushQuit() {
  SDL_Event e{};
  e.type = SDL_QUIT;
  return SDL_PushEvent(&e) == 1;
}

char glyph(sim::Material m) {
  switch (m) {
    case sim::Material::Empty: return '.';
    case sim::Material::Earth: return '#';
    case sim::Material::Rock: return 'R';
    case sim::Material::Steel: return 'S';
    case sim::Material::Built: return 'B';
  }
  return '?';
}

// Return the gap to the nearest column with eight solid, non-steel cells at y .. y-7,
// or -1 if none is in range. This finds the 16-column MM_BASH_002 earth pillars
// (rows 7..71) and skips the diagonal strokes and steel seam (columns 245..12).
constexpr int kWallLimit = 96;
int wallDistance(const sim::Terrain& t, const sim::Lemming& l, int limit = kWallLimit) {
  const int dir = static_cast<int>(l.facing);
  for (int d = 1; d <= limit; ++d) {
    bool tall = true;
    for (int dy = 0; dy < 8 && tall; ++dy) {
      const sim::Material m = t.at(t.wrapX(l.x + d * dir), l.y - dy);
      tall = sim::isSolid(m) && m != sim::Material::Steel;
    }
    if (tall) return d;
  }
  return -1;
}

std::string dumpRegion(const sim::Terrain& t, int x, int y, int dir) {
  std::ostringstream os;
  const int x0 = dir > 0 ? x - 4 : x - 24, x1 = dir > 0 ? x + 24 : x + 4;
  os << "columns " << x0 << ".." << x1 << " (lemming column " << x << ", facing " << dir << ")\n";
  for (int row = y - 8; row <= y + 3; ++row) {
    os << (row == y ? "y=" : "  ") << row << (row == y ? ">" : " ") << ' ';
    for (int cx = x0; cx <= x1; ++cx) os << (cx == x ? (t.solid(cx, row) ? 'L' : 'l') : glyph(t.at(t.wrapX(cx), row)));
    os << '\n';
  }
  return os.str();
}

// Drag the camera to follow the target. Release the drag before assignment and click only on a matching hover.
struct Aim {
  std::size_t target = static_cast<std::size_t>(-1);
  bool held = false;
  double prevErr = 0.0;
  bool panned = false;
  int worse = 0;
};

struct Result {
  bool ok = true;
  std::string failure;
  int frames = 0;
  int gap = 1;
  double panSign = 1.0;  // positive for the 256-minus-column yaw convention
  // click bookkeeping
  std::size_t target = static_cast<std::size_t>(-1);
  bool selected = false, clicked = false, assigned = false;
  int assignTick = -1, assignX = 0, assignY = 0, assignDir = 0, assignGap = -2;
  int stockBefore = -1, stockAfter = -1;
  std::string regionAtAssign, regionAtEnd;
  std::size_t solidAtAssign = 0, solidLatest = 0;
  // lemming trace
  std::vector<std::string> trace;
  std::vector<std::string> log;
  int basherTicks = 0, firstWalkerAfter = -1;
  int basherX0 = 0, basherX1 = 0, basherY1 = 0;
  bool steelHit = false;
  // followers: lemmings other than the target that crossed the target's start column while the
  // tunnel stood (their x passed assignX + dir * 3 in the bashing direction)
  std::vector<std::size_t> crossed;
  int exited = 0, dead = 0;
  int total = 0, need = 0;
  int endTick = -1;
  std::string note;
  // later assignments (same click policy, further walls), and the level outcome
  int laterClicks = 0, laterAssigned = 0, stockPrev = -1;
  bool leveled = false;
  int saved = 0;
  bool won = false;
  std::string laterLog;
};

}  // namespace

int main(int argc, char** argv) {
  const char* driver = std::getenv("SDL_VIDEODRIVER");
  if (!driver || std::string(driver) != "dummy") {
    std::cerr << "basherfix: refusing to run without SDL_VIDEODRIVER=dummy\n";
    return 1;
  }
  const char* gameDirEnv = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDirEnv || !*gameDirEnv || !fs::is_regular_file(fs::path(gameDirEnv) / "lemmings.box")) {
    std::cout << "basherfix: SKIPPED (retail assets unavailable)\n";
    return 0;
  }
  const std::string mode = argc > 1 ? argv[1] : "assert";
  const fs::path outDir = argc > 2 ? fs::path(argv[2]) : fs::path("build/basher-fix-001");
  std::error_code ec;
  fs::create_directories(outDir, ec);

  Result r;
  // Probe reads the wall gap from argv[3]. Away starts on the flat y=33 ledge.
  r.gap = argc > 3 ? std::atoi(argv[3]) : (mode == "away" ? 64 : 2);
  const bool playOn = mode == "probe";  // later walls get their own Basher so the level outcome can be seen
  Aim firstAim, laterAim;

  int calls = 0;
  const auto fail = [&](const std::string& why) {
    if (r.ok) r.failure = why;
    r.ok = false;
  };

  const auto observer = [&](const app::LevelFrameView& f) {
    ++calls;
    r.frames = calls;
    const auto& w = f.world;
    const auto& lemmings = w.lemmings();
    const int tick = static_cast<int>(w.tickCount());
    r.total = w.setup().totalLemmings;
    r.need = w.setup().saveTarget;
    r.solidLatest = w.terrain().solidCells();
    if (calls == 1) {
      // Whole-level terrain map (rows top to bottom, all columns), written once for the report.
      std::ostringstream map;
      const auto& t = w.terrain();
      map << "terrain " << t.width() << "x" << t.height() << " (. empty, # earth, S steel, R rock, B built); "
          << "entrances:";
      for (const auto& e : w.setup().entrances) map << " (" << e.x << ',' << e.y << ')';
      map << " exits:";
      for (const auto& e : w.setup().exits) map << " (" << e.x << ',' << e.y << ' ' << e.width << 'x' << e.height << ')';
      map << " skills:";
      for (int s : w.setup().skills) map << ' ' << s;
      map << " total " << w.setup().totalLemmings << " need " << w.setup().saveTarget << " timeLimitTicks "
          << w.setup().timeLimitTicks << "\n";
      for (int row = 0; row < t.height(); ++row) {
        map << (row < 10 ? "  " : row < 100 ? " " : "") << row << ' ';
        for (int cx = 0; cx < t.width(); ++cx) map << glyph(t.at(cx, row));
        map << '\n';
      }
      std::ofstream(outDir / "terrain-map.txt") << map.str();
      if (!pushKey(kBasherSkillIndex)) fail("could not queue the Basher plate key");
      r.selected = true;
      r.stockBefore = w.setup().skills[kBasherSkillIndex];
    }
    if (calls == 2 && f.selectedSkillIndex != kBasherSkillIndex) fail("the Basher plate was not selected");

    if (tick <= 420) {  // per-frame lemming/hover log for the report (probe modes only write it)
      std::ostringstream os;
      os << tick << " hover=" << (f.hoveredActor ? std::to_string(*f.hoveredActor) : "-") << " cursor=" << f.cursorX
         << ',' << f.cursorY << " yaw=" << f.displayedPose.yawUnits << " visible=";
      for (const auto& a : f.actors) os << a.index << ' ';
      os << "|";
      for (std::size_t i = 0; i < lemmings.size(); ++i) {
        const auto& l = lemmings[i];
        if (!l.alive()) continue;
        os << ' ' << i << ':' << l.x << ',' << l.y << (l.facing == sim::Facing::Right ? 'R' : 'L') << ','
           << sim::stateName(l.state) << ",d" << wallDistance(w.terrain(), l);
      }
      r.log.push_back(os.str());
    }

    // Choose a walker at least gap+6 cells from a tall wall. Return its index on a
    // confirmed click, or npos if it leaves walking/falling, turns, or passes the gap.
    constexpr std::size_t npos = static_cast<std::size_t>(-1);
    const auto drive = [&](Aim& aim, int selectLimit) -> std::size_t {
      if (f.cameraIntroActive || f.levelMenuOpen) return npos;
      if (aim.target == npos) {
        for (std::size_t i = 0; i < lemmings.size(); ++i) {
          const auto& c = lemmings[i];
          if (!c.alive() || c.state != sim::State::Walker) continue;
          const int d = wallDistance(w.terrain(), c);
          if (d >= r.gap + 6 && d <= selectLimit) {
            aim.target = i;
            aim.panned = false;
            aim.worse = 0;
            break;
          }
        }
        if (aim.target == npos) return npos;
      }
      const auto& l = lemmings[aim.target];
      const int d = wallDistance(w.terrain(), l);
      const bool walking = l.state == sim::State::Walker;
      if (!l.alive() || (!walking && l.state != sim::State::Faller) || (walking && (d < 0 || d < r.gap))) {
        r.note += "target " + std::to_string(aim.target) + " dropped at tick " + std::to_string(tick) +
                  " (state " + std::string(sim::stateName(l.state)) + ", wall distance " + std::to_string(d) + "); ";
        if (aim.held) pushRight(false, 320, 240);
        aim = Aim{};
        return npos;
      }
      const app::LevelActorView* act = nullptr;
      for (const auto& a : f.actors)
        if (a.index == aim.target) act = &a;
      int px = 0, py = 0;
      bool onScreen = false;
      if (act) {
        px = static_cast<int>((act->placement.left + act->placement.right) / 2);
        py = static_cast<int>((act->placement.top + act->placement.bottom) / 2);
        onScreen = px >= 100 && px < 640 && py >= 0 && py < 480;
      }
      const bool close = walking && d <= r.gap + 4;
      if (close && onScreen) {
        if (aim.held) {
          pushRight(false, px, py);
          aim.held = false;
        }
        pushMotion(px, py);
        if (f.hoveredActor && *f.hoveredActor == aim.target && walking && d == r.gap) {
          if (!pushClick(f.cursorX, f.cursorY)) fail("could not queue the assignment click");
          return aim.target;
        }
        return npos;
      }
      // Aim slightly ahead. Camera::buildCamera and ActorPose agree that yaw locates
      // the near-side column.
      const double ahead = static_cast<double>(l.x + 2 * static_cast<int>(l.facing));
      double delta = std::fmod(ahead - f.displayedPose.yawUnits, 256.0);
      if (delta >= 128.0) delta -= 256.0;
      if (delta < -128.0) delta += 256.0;
      const double err = std::abs(delta);
      if (aim.panned && err > aim.prevErr + 0.5 && err > 8.0) ++aim.worse; else aim.worse = 0;
      if (aim.worse >= 3) {
        r.panSign = -r.panSign;
        aim.worse = 0;
        r.note += "pan sign flipped at tick " + std::to_string(tick) + "; ";
      }
      aim.prevErr = err;
      aim.panned = true;
      const int dx = std::clamp(static_cast<int>(std::lround(-2.4 * delta * r.panSign)), -90, 90);
      if (!aim.held) {
        pushRight(true, 320, 240);
        aim.held = true;
      }
      if (dx != 0) pushDrag(dx);
      return npos;
    };

    if (!r.clicked && calls >= 2) {
      const std::size_t hit = drive(firstAim, kWallLimit);
      if (hit != npos) {
        const auto& l = lemmings[hit];
        r.target = hit;
        r.assignTick = tick;
        r.assignX = l.x;
        r.assignY = l.y;
        r.assignDir = static_cast<int>(l.facing);
        r.assignGap = r.gap;
        r.regionAtAssign = dumpRegion(w.terrain(), l.x, l.y, r.assignDir);
        r.solidAtAssign = w.terrain().solidCells();
        r.stockBefore = w.setup().skills[kBasherSkillIndex];
        r.clicked = true;
        firstAim = Aim{};
      }
    }

    if (r.clicked) {
      const auto& l = lemmings[r.target];
      if (!r.assigned && l.state == sim::State::Basher) {
        r.assigned = true;
        r.stockAfter = w.setup().skills[kBasherSkillIndex];
        r.basherX0 = l.x;
      }
      if (r.assigned || tick - r.assignTick < 4) {
        std::ostringstream os;
        os << tick << ',' << l.x << ',' << l.y << ',' << static_cast<int>(l.facing) << ',' << sim::stateName(l.state)
           << ',' << l.frame << ',' << wallDistance(w.terrain(), l) << ',' << w.terrain().solidCells();
        r.trace.push_back(os.str());
      }
      if (l.state == sim::State::Basher) {
        ++r.basherTicks;
        r.basherX1 = l.x;
        r.basherY1 = l.y;
      }
      if (r.assigned && l.state == sim::State::Walker && r.firstWalkerAfter < 0) {
        r.firstWalkerAfter = tick;
        r.regionAtEnd = dumpRegion(w.terrain(), l.x, l.y, r.assignDir);
        if (r.basherTicks <= 0) fail("never observed a Basher tick");
      }
      // Followers crossing the wall column the basher started at, in the bashing direction.
      for (std::size_t i = 0; i < lemmings.size(); ++i) {
        if (i == r.target || !lemmings[i].alive()) continue;
        const auto& o = lemmings[i];
        const int ahead = (o.x - r.assignX) * r.assignDir;
        if (o.y >= r.assignY - 4 && o.y <= r.assignY + 8 && ahead >= 4 && ahead <= 12 &&
            std::find(r.crossed.begin(), r.crossed.end(), i) == r.crossed.end())
          r.crossed.push_back(i);
      }
    }

    // Use the same hover-confirmed assignment at the second pillar.
    if (playOn && r.firstWalkerAfter >= 0 && r.laterClicks < 4 && w.setup().skills[kBasherSkillIndex] > 0) {
      // Later targets are only picked up within 64 cells of a wall (the pillars and the ledges beside them).
      const std::size_t hit = drive(laterAim, 64);
      if (hit != npos) {
        const auto& l = lemmings[hit];
        ++r.laterClicks;
        r.laterLog += "later click on lemming " + std::to_string(hit) + " at tick " + std::to_string(tick) +
                      " cell (" + std::to_string(l.x) + ',' + std::to_string(l.y) + ") facing " +
                      std::to_string(static_cast<int>(l.facing)) + "; ";
        laterAim = Aim{};
      }
    }
    {
      const int stock = w.setup().skills[kBasherSkillIndex];
      if (r.stockPrev >= 0 && stock < r.stockPrev && r.firstWalkerAfter >= 0) ++r.laterAssigned;
      r.stockPrev = stock;
    }

    int exited = 0, dead = 0;
    for (const auto& l : lemmings) {
      if (l.state == sim::State::Exited) ++exited;
      if (l.state == sim::State::Dead) ++dead;
    }
    r.exited = exited;
    r.dead = dead;
    r.saved = w.saved();
    r.won = w.won();
    bool allDone = static_cast<int>(lemmings.size()) == r.total;
    for (const auto& l : lemmings)
      if (l.alive() && l.state != sim::State::Blocker) allDone = false;
    r.leveled = f.endPhase != app::LevelEndPhase::Running;
    // Stop the assert case when a follower crosses the tunnel and the away case when bashing ends.
    const bool done = mode == "probe" ? (allDone || r.leveled)
                      : mode == "assert" ? (!r.crossed.empty() || tick > r.firstWalkerAfter + 200)
                                         : tick > r.firstWalkerAfter + 5;
    const bool settled = r.clicked && (r.firstWalkerAfter >= 0) && done;
    if (settled || (!r.clicked && calls > 1000) || calls >= kFrames - 1) {
      r.endTick = tick;
      pushQuit();
    }
  };

  std::vector<std::string> args{"basherfix", "--level", "MM_BASH_002", "--sim", "--native", "--no-audio", "--frames",
                                std::to_string(kFrames), "--game-dir", gameDirEnv};
  std::vector<char*> raw;
  for (auto& a : args) raw.push_back(a.data());
  app::ApplicationRunOptions options;
  options.frameSeconds = 0.2;
  const int status = app::runApplication(static_cast<int>(raw.size()), raw.data(), {}, observer, options);

  std::ostringstream md;
  md << "basherfix mode=" << mode << " gap=" << r.gap << "\nrunApplication_status=" << status << " frames=" << r.frames
     << " end_tick=" << r.endTick << "\ntarget=" << (r.target == static_cast<std::size_t>(-1) ? -1 : static_cast<long>(r.target))
     << " clicked=" << r.clicked << " assigned_state_basher=" << r.assigned << "\nassign_tick=" << r.assignTick << " cell=("
     << r.assignX << ',' << r.assignY << ") facing=" << r.assignDir << " wall_distance_at_assign=" << r.assignGap
     << "\nbasher_stock " << r.stockBefore << " -> " << r.stockAfter << "\nbasher_ticks=" << r.basherTicks
     << " first_walker_after_tick=" << r.firstWalkerAfter << " basher_x " << r.basherX0 << " -> " << r.basherX1
     << "\nsolid_cells at_assign=" << r.solidAtAssign << " latest=" << r.solidLatest
     << "\nfollowers_crossing=" << r.crossed.size() << " exited=" << r.exited << " dead=" << r.dead << " of " << r.total
     << " need " << r.need << "\nlevel: saved=" << r.saved << " won=" << r.won << " end_phase_left_running=" << r.leveled
     << " later_clicks=" << r.laterClicks << " later_basher_stock_drops=" << r.laterAssigned << "\n"
     << r.laterLog << "\nnotes: " << r.note << "\n\nterrain at assignment:\n" << r.regionAtAssign
     << "\nterrain when the target became a Walker again:\n" << r.regionAtEnd
     << "\ntrace tick,x,y,facing,state,frame,wallDistance,solidCells\n";
  for (const auto& t : r.trace) md << t << '\n';
  std::ofstream(outDir / (mode + "-gap" + std::to_string(r.gap) + ".txt")) << md.str();
  {
    std::ofstream lg(outDir / (mode + "-gap" + std::to_string(r.gap) + "-frames.txt"));
    for (const auto& line : r.log) lg << line << '\n';
  }
  std::cout << md.str();

  if (status != 0) fail("runApplication returned failure");
  if (!r.clicked) fail("no lemming reached the requested wall distance to be clicked");
  if (!r.ok) {
    std::cerr << "FAIL: basherfix: " << r.failure << '\n';
    return 1;
  }

  if (mode == "assert" || mode == "away") {
    bool good = true;
    const auto expect = [&](bool c, const char* what) {
      if (!c) {
        std::cerr << "FAIL: basherfix: " << what << '\n';
        good = false;
      }
    };
    expect(r.assigned, "public click on a walker set the lemming to Basher and it stayed one past the assignment tick");
    expect(r.stockAfter == r.stockBefore - 1, "the Basher count dropped by one");
    expect(r.firstWalkerAfter >= 0, "the Basher ended and the lemming walks again (no endless Basher)");
    if (mode == "assert") {
      // Two cells short of the 16-column pillar, the Basher must swing across the gap and tunnel through.
      expect(r.basherTicks >= 8 * 8, "the Basher kept bashing for many cycles");
      expect(std::abs(r.basherX1 - r.basherX0) >= 16, "the Basher advanced through all 16 columns of the pillar");
      expect(r.solidLatest < r.solidAtAssign, "terrain was removed");
      expect(r.solidAtAssign - r.solidLatest >= 16 * 4, "at least 16 columns x 4 rows were removed");
      expect(!r.crossed.empty(), "a following lemming passed through the tunnel");
    } else {
      // Assigned six cells short: two free swings, then an empty cycle ends it. No tunnel, no endless bash.
      expect(r.basherTicks <= 4 * 8, "an unproductive Basher ends within a few cycles");
      expect(std::abs(r.basherX1 - r.basherX0) <= 2, "it advanced no further than the free swings");
      expect(r.solidLatest >= r.solidAtAssign, "no terrain was removed when no wall was in reach");
    }
    if (!good) return 1;
  }
  std::cout << "PASS: basherfix " << mode << '\n';
  return 0;
}
#else
int main() {
  std::cerr << "basherfix: requires SDL2\n";
  return 1;
}
#endif
