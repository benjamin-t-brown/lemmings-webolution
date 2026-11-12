// playable-baseline-001 assessment driver (SCRIPTED CLONE OBSERVATION, not human play, not original fidelity).
//
// One executable, several CTest registrations (see CMakeLists.txt):
//   survey 0 <dir>   read-only decoded-level survey + build/source identity
//   play N <dir>     route N (1 = MM_CLIMB_002, 2/3 = levels chosen by a data rule) through the
//                    PUBLIC webo::app::runApplication: bare ordinary startup (frontend default),
//                    SDL events queued from the read-only FrontendWindowObserver/LevelFrameObserver,
//                    natural wall-clock level time (no frameSeconds, no --frames), an isolated
//                    FileSaveSink under <dir>/route-N/save. Outcome (win/loss/watchdog) is REPORTED,
//                    never asserted: the exit code only reflects driver integrity.
//   restart N <dir>  a second OS process: fresh runApplication over the same isolated save file,
//                    inspects persisted progress through the frontend. UNVERIFIED unless play won.
//
// Nothing here mutates a World, actor, terrain, stock, flag or save. The only inputs are SDL events.
// The fast-forward plate is clicked through the public input path and recorded in the trace.
#include "app/AppEntry.h"
#include "app/MapScene.h"
#include "app/RuntimeSkillPanel.h"
#include "assets/Box.h"
#include "assets/Png.h"
#include "save/SaveStore.h"
#include "save/SavedGame.h"
#include "sim/RetailLevel.h"
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
#include <bit>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifndef PB_SOURCE_DIR
#define PB_SOURCE_DIR "."
#endif

#if WEBOLUTION_HAVE_SDL2
namespace fs = std::filesystem;
using namespace webo;

namespace {

using SteadyClock = std::chrono::steady_clock;
const SteadyClock::time_point kProcessStart = SteadyClock::now();
double wallSeconds() { return std::chrono::duration<double>(SteadyClock::now() - kProcessStart).count(); }

constexpr int kFastForwardX = 54, kFastForwardY = 348;  // plate 8 centre (runtimeSkillPanelIndex)
constexpr int kContinueX = 320, kContinueY = 360;       // results Continue / Retry (test_savepersist)
constexpr int kPlayCurrentX = 320, kPlayCurrentY = 363;  // Play Current Game (test_frontendentry)

bool pushQuit() {
  SDL_Event e{};
  e.type = SDL_QUIT;
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
bool pushSkillKey(int index) {  // keys 1..8 select panel plates 0..7
  SDL_Event e{};
  e.type = SDL_KEYDOWN;
  e.key.type = SDL_KEYDOWN;
  e.key.state = SDL_PRESSED;
  e.key.keysym.scancode = static_cast<SDL_Scancode>(SDL_SCANCODE_1 + index);
  e.key.keysym.sym = static_cast<SDL_Keycode>(SDLK_1 + index);
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

std::string fmt(double v, int digits = 2) {
  std::ostringstream os;
  os.setf(std::ios::fixed);
  os.precision(digits);
  os << v;
  return os.str();
}

const char* kSkillNames[8] = {"Climber", "Floater", "Bomber", "Blocker", "Builder", "Basher", "Miner", "Digger"};

// ---- decoded level facts ----------------------------------------------------------------------
struct LevelFacts {
  std::string level;
  std::size_t node = 0;
  int px = -1, py = -1;  // a map pixel that pickNode() maps to this node on the fresh map
  unsigned features = 0;
  std::string summary;
  bool ok = false;
  std::string error;
  int timeLimitTicks = 0, total = 0, need = 0;
  std::array<int, 8> skills{};
};

LevelFacts inspect(const AssetLibrary& lib, std::size_t node, const std::string& level) {
  LevelFacts f;
  f.node = node;
  f.level = level;
  auto setup = sim::loadRetailLevel(lib, level);
  if (!setup) {
    f.error = setup.error();
    // loadRetailLevel matches the archive path by case-sensitive substring. Report whether the upper-cased id
    // (the archive spelling) would have loaded, to separate a case mismatch from a genuinely missing level.
    std::string upper = level;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
    if (upper != level) {
      auto alt = sim::loadRetailLevel(lib, upper);
      f.error += alt ? "; the UPPER-CASE id " + upper + " DOES load (case-sensitive name match)"
                     : "; the UPPER-CASE id " + upper + " also fails: " + alt.error();
    }
    return f;
  }
  const auto& t = setup->terrain;
  long water = 0, tramp = 0, anti = 0;
  for (int y = 0; y < t.height(); ++y)
    for (int x = 0; x < t.width(); ++x) {
      if (t.waterHazard(x, y) || t.zoneWater(x, y) || t.zoneAcid(x, y)) ++water;
      if (t.trampoline(x, y)) ++tramp;
      if (t.antiGrav(x, y)) ++anti;
    }
  f.ok = true;
  f.timeLimitTicks = setup->timeLimitTicks;
  f.total = setup->totalLemmings;
  f.need = setup->saveTarget;
  f.skills = setup->skills;
  for (int i = 0; i < 8; ++i) if (setup->skills[static_cast<std::size_t>(i)] != 0) f.features |= 1u << i;
  if (!setup->teleports.empty()) f.features |= 1u << 8;
  if (!setup->speedPads.empty()) f.features |= 1u << 9;
  if (!setup->levers.empty()) f.features |= 1u << 10;
  if (water > 0) f.features |= 1u << 11;
  if (tramp > 0) f.features |= 1u << 12;
  if (anti > 0) f.features |= 1u << 13;
  if (setup->entrances.size() > 1) f.features |= 1u << 14;
  if (setup->exits.size() > 1) f.features |= 1u << 15;
  std::ostringstream os;
  os << f.level << " node=" << node << " lemmings=" << f.total << " need=" << f.need
     << " time=" << (f.timeLimitTicks / 5) << "s releaseTicks=" << setup->releaseIntervalTicks
     << " terrain=" << t.width() << "x" << t.height() << " hatches=" << setup->entrances.size()
     << " exits=" << setup->exits.size() << " teleports=" << setup->teleports.size()
     << " speedPads=" << setup->speedPads.size() << " levers=" << setup->levers.size()
     << " hazardCells=" << water << " trampolineCells=" << tramp << " antiGravCells=" << anti
     << " stock=[";
  for (int i = 0; i < 8; ++i) os << (i ? "," : "") << kSkillNames[i] << ":" << setup->skills[static_cast<std::size_t>(i)];
  os << "] featureMask=0x" << std::hex << f.features;
  f.summary = os.str();
  return f;
}

std::string featureNames(unsigned m) {
  static const char* names[16] = {"climber", "floater", "bomber", "blocker", "builder", "basher", "miner",
      "digger", "teleport", "speedpad", "lever", "water/acid", "trampoline", "antigrav", "multi-hatch", "multi-exit"};
  std::string s;
  for (int i = 0; i < 16; ++i) if (m & (1u << i)) s += (s.empty() ? "" : "+") + std::string(names[i]);
  return s.empty() ? "none" : s;
}

struct FreshMap {
  std::vector<LevelFacts> selectable;  // frontend-selectable on the fresh (zero-progress) map
  std::string error;
};

FreshMap surveyFreshMap(const AssetLibrary& lib) {
  FreshMap out;
  auto map = app::MapScene::build(lib, {});
  if (!map) { out.error = map.error(); return out; }
  std::map<std::size_t, std::pair<int, int>> first;
  for (int y = 0; y < 480; ++y)
    for (int x = 0; x < 640; ++x)
      if (auto n = map->pickNode(x, y); n && !first.count(*n)) first[*n] = {x, y};
  for (const auto& [node, px] : first) {
    auto name = map->levelForNode(node);
    if (!name) continue;
    auto f = inspect(lib, node, *name);
    f.px = px.first;
    f.py = px.second;
    out.selectable.push_back(std::move(f));
  }
  return out;
}

// Deterministic data rule: route 1 is MM_CLIMB_002. Of the other fresh-map selectable levels pick the
// one adding the most interaction features absent from MM_CLIMB_002, then the one adding the most
// features absent from both (ties: richer level, then lower node).
std::optional<LevelFacts> chooseRoute(const FreshMap& fm, int route, std::string& why) {
  const LevelFacts* r1 = nullptr;
  for (const auto& l : fm.selectable) if (l.level == "MM_CLIMB_002" && l.ok) r1 = &l;
  if (!r1) { why = "MM_CLIMB_002 is not frontend-selectable on the fresh map"; return std::nullopt; }
  if (route == 1) { why = "fixed first route required by the task"; return *r1; }
  const LevelFacts* r2 = nullptr;
  const LevelFacts* r3 = nullptr;
  const auto score = [](unsigned m, unsigned known) { return std::popcount(m & ~known); };
  for (const auto& l : fm.selectable) {
    if (!l.ok || &l == r1) continue;
    if (!r2 || score(l.features, r1->features) > score(r2->features, r1->features) ||
        (score(l.features, r1->features) == score(r2->features, r1->features) &&
         std::popcount(l.features) > std::popcount(r2->features)))
      r2 = &l;
  }
  if (r2) {
    const unsigned known = r1->features | r2->features;
    for (const auto& l : fm.selectable) {
      if (!l.ok || &l == r1 || &l == r2) continue;
      if (!r3 || score(l.features, known) > score(r3->features, known) ||
          (score(l.features, known) == score(r3->features, known) &&
           std::popcount(l.features) > std::popcount(r3->features)))
        r3 = &l;
    }
  }
  const LevelFacts* pick = route == 2 ? r2 : r3;
  if (!pick) { why = "no further frontend-selectable level exists on the fresh map"; return std::nullopt; }
  const unsigned known = route == 2 ? r1->features : (r1->features | r2->features);
  why = "new interaction features vs. earlier routes: " + featureNames(pick->features & ~known) +
        " (all features: " + featureNames(pick->features) + ")";
  return *pick;
}

// ---- trace/result plumbing ---------------------------------------------------------------------
struct Attempt {
  int number = 1;
  int strategy = 1;
  double wall0 = 0.0, watchdogSeconds = 0.0;
  int frames = 0;
  bool ffClicked = false;
  int ffFrame = -1;
  long ffTick = -1;
  int lastActionFrame = -100;
  struct Pending { int frame; std::size_t lemming; int skill; int stockBefore; bool climb, floatr; } pendingData{};
  bool pending = false;
  std::map<std::pair<std::size_t, int>, int> tries;
  std::map<std::size_t, int> cooldownUntil;
  // Bomber policy memory: lemming index -> last observed (x, y) of a lemming this attempt primed. A primed
  // lemming's entry follows it until it dies, so the entry is where the hole is (or will be).
  std::map<std::size_t, std::pair<int, int>> bombSites;
  int bombsPrimed = 0;
  std::array<int, 8> keyTries{};
  int clicks = 0, assignOk = 0, assignFail = 0, onTarget = 0, shots = 0;
  double lastSampleWall = -100.0;
  bool endingShot = false, watchdog = false, quitSent = false;
  double firstActWall = -1.0;
  long lastTick = 0;
  int lastSpawned = 0, lastSaved = 0, lastDead = 0, lastActive = 0;
  long lastRemainingTicks = 0;
  long fastTickStart = -1;
  double fastWallStart = 0.0;
  int endPhase = 0;
  double endObservedWall = -1.0;
  std::array<int, 8> stockStart{}, stockEnd{};
  int total = 0, need = 0;
  long limitTicks = 0;
};

struct AttemptResult {
  Attempt a;
  bool resultsSeen = false;
  bool success = false;
  int saved = 0, totalOut = 0, required = 0, secondsRemaining = 0;
  std::string classification;
  double wallEnd = 0.0;
};

struct Ctx {
  fs::path dir;
  std::ofstream trace, samples;
  int nextAttempt = 1;
  Attempt att;
  std::vector<AttemptResult> results;
  bool levelEntered = false;
  std::string enteredLevel;
  std::optional<int> enteredCounter;
  std::string anomaly;
  // frontend
  int feCalls = 0, menuClicks = 0, mapClicks = 0, contClicks = 0, lastPush = -100;
  double lastProgressWall = 0.0;
  int resultsHandledFor = 0;
  int resultAction = 0;  // 0 none, 1 Continue (win), 2 Retry (first loss), 3 quit (final loss)
  int lastScreen = -1;
  bool returnedToMap = false;
  std::array<std::int32_t, 102> flagsAfter{}, statesAfter{};
  int saveCountAtMap = -1;
  std::optional<std::string> saveErrorAtMap;
  double menuWall = -1, mapWall = -1, levelWall = -1;
  void line(double w, int frame, long tick, const std::string& kind, const std::string& detail) {
    trace << fmt(w, 3) << ',' << nextAttempt << ',' << frame << ',' << tick
          << ',' << kind << ',' << detail << '\n';
    trace.flush();
  }
};

bool solidWrap(const sim::Terrain& t, int x, int y) {
  if (y < 0 || y >= t.height() || t.width() <= 0) return false;
  const int w = t.width();
  return t.solid(((x % w) + w) % w, y);
}

bool wallAhead(const sim::Lemming& l, const sim::Terrain& t, int reach) {
  const int dir = static_cast<int>(l.facing);
  for (int k = 1; k <= reach; ++k)
    if (solidWrap(t, l.x + dir * k, l.y - 3) && solidWrap(t, l.x + dir * k, l.y - 4)) return true;
  return false;
}

bool dropAhead(const sim::Lemming& l, const sim::Terrain& t) {
  const int dir = static_cast<int>(l.facing);
  for (int k = 1; k <= 2; ++k) {
    bool ground = false;
    for (int dy = 2; dy <= 9 && !ground; ++dy) ground = solidWrap(t, l.x + dir * k, l.y + dy);
    if (!ground) return true;
  }
  return false;
}

// Strategy 1 (reactive): spend a skill only where the observed terrain asks for it.
// Strategy 2 (blanket, used for the one retry after a loss): Climber/Floater to every Walker first.
int desiredSkill(const sim::Lemming& l, const sim::Terrain& t, const std::array<int, 8>& stock, int strategy) {
  if (l.state != sim::State::Walker) return -1;
  const bool wall = wallAhead(l, t, 3);
  const auto has = [&](sim::Skill s) { return stock[static_cast<std::size_t>(s)] > 0; };
  if (strategy == 2) {
    if (!l.canClimb && has(sim::Skill::Climber)) return 0;
    if (!l.canFloat && has(sim::Skill::Floater)) return 1;
  } else {
    if (wall && !l.canClimb && has(sim::Skill::Climber)) return 0;
    if (!l.canFloat && has(sim::Skill::Floater) && dropAhead(l, t)) return 1;
  }
  if (wall && !l.canClimb && !has(sim::Skill::Climber)) {
    if (has(sim::Skill::Basher)) return 5;
    if (has(sim::Skill::Miner)) return 6;
  }
  return -1;
}

// Bomber policy (levels whose stock holds Bombers, e.g. MM_BOMB: 20 lemmings, need 10). A primed lemming
// explodes after its fuse and removes a small terrain disc, so the intent is to open holes in the ledges the
// lemmings patrol. A Walker is primed only when no earlier-primed lemming's site is within `spacing` cells on
// the same ledge band (|dy| <= 10) and no wall is directly ahead. Strategy 1 spacing 40 / at most 8 bombs;
// strategy 2 (retry) spacing 70 / at most 5. The cap keeps dead lemmings well under total - need.
bool bomberWanted(const Attempt& a, const sim::Lemming& l, const sim::Terrain& t) {
  if (a.stockStart[2] <= 0 || l.state != sim::State::Walker || l.fuseMs >= 0) return false;
  if (a.bombsPrimed >= (a.strategy == 1 ? 8 : 5)) return false;
  if (wallAhead(l, t, 8)) return false;
  const int spacing = a.strategy == 1 ? 40 : 70;
  const int w = std::max(1, t.width());
  for (const auto& [idx, site] : a.bombSites) {
    const int dx = std::abs(site.first - l.x);
    if (std::min(dx, w - dx) < spacing && std::abs(site.second - l.y) <= 10) return false;
  }
  return true;
}

std::string actorDump(const app::LevelFrameView& f) {
  std::ostringstream os;
  const auto& lemmings = f.world.lemmings();
  for (std::size_t i = 0; i < lemmings.size(); ++i) {
    const auto& l = lemmings[i];
    if (l.state == sim::State::Dead) continue;
    os << (os.tellp() > 0 ? "|" : "") << i << ':' << sim::stateName(l.state) << '@' << l.x << '/' << l.y;
  }
  return os.str().empty() ? "none-alive" : os.str();
}

const char* phaseName(app::LevelEndPhase p) {
  return p == app::LevelEndPhase::Running ? "Running" : p == app::LevelEndPhase::Pending ? "Pending" : "Results";
}

void shot(Ctx& c, const Image& image, const std::string& name) {
  if (auto r = writePng(c.dir / name, image); !r) c.anomaly += "png " + name + ": " + r.error() + "; ";
}

void beginAttempt(Ctx& c, const app::LevelFrameView& f) {
  c.att = Attempt{};
  c.att.number = c.nextAttempt;
  c.att.strategy = c.nextAttempt == 1 ? 1 : 2;
  c.att.wall0 = wallSeconds();
  const auto& s = f.world.setup();
  c.att.stockStart = s.skills;
  c.att.total = s.totalLemmings;
  c.att.need = s.saveTarget;
  c.att.limitTicks = s.timeLimitTicks;
  const double limitSeconds = s.timeLimitTicks > 0 ? s.timeLimitTicks / 5.0 : 600.0;
  // Natural level time is limitSeconds; the latched fast-forward is expected to be several times faster.
  c.att.watchdogSeconds = std::min(900.0, std::max(120.0, limitSeconds / 3.0 + 90.0));
  if (!c.levelEntered) {
    c.levelEntered = true;
    c.enteredLevel = std::string(f.levelName);
    c.enteredCounter = f.frontendConstructionCounter;
    c.levelWall = wallSeconds();
  }
  c.line(wallSeconds(), f.frame, static_cast<long>(f.world.tickCount()), "attempt-begin",
         "attempt=" + std::to_string(c.att.number) + ";strategy=" + std::to_string(c.att.strategy) + ";level=" +
         std::string(f.levelName) + ";watchdog_s=" + fmt(c.att.watchdogSeconds, 0) + ";limit_ticks=" +
         std::to_string(s.timeLimitTicks) + ";total=" + std::to_string(s.totalLemmings) + ";need=" +
         std::to_string(s.saveTarget));
  shot(c, f.image, "attempt" + std::to_string(c.att.number) + "-first-frame.png");
}

void levelObserver(Ctx& c, const app::LevelFrameView& f) {
  if (f.frame == 0) beginAttempt(c, f);
  Attempt& a = c.att;
  if (a.quitSent) return;
  ++a.frames;
  const double w = wallSeconds() - a.wall0;
  const long tick = static_cast<long>(f.world.tickCount());
  a.lastTick = tick;
  a.lastSpawned = f.world.spawned();
  a.lastSaved = f.world.saved();
  a.lastDead = f.world.dead();
  a.lastActive = f.world.activeCount();
  a.lastRemainingTicks = static_cast<long>(f.world.timeRemainingTicks());
  a.stockEnd = f.world.setup().skills;
  a.endPhase = static_cast<int>(f.endPhase);
  for (auto& [idx, site] : a.bombSites)  // read-only: follow each primed lemming until it dies
    if (idx < f.world.lemmings().size() && f.world.lemmings()[idx].state != sim::State::Dead)
      site = {f.world.lemmings()[idx].x, f.world.lemmings()[idx].y};

  if (w - a.lastSampleWall >= 2.0) {
    a.lastSampleWall = w;
    c.samples << a.number << ',' << fmt(w, 2) << ',' << f.frame << ',' << tick << ',' << fmt(tick * 0.2, 1) << ','
              << a.lastSpawned << ',' << a.lastActive << ',' << a.lastSaved << ',' << a.lastDead << ','
              << a.lastRemainingTicks << ',' << phaseName(f.endPhase) << ',' << f.paused << ','
              << f.selectedSkillIndex;
    for (int i = 0; i < 8; ++i) c.samples << ',' << a.stockEnd[static_cast<std::size_t>(i)];
    c.samples << '\n';
    c.samples.flush();
  }
  if (a.ffClicked && a.fastTickStart < 0 && f.frame > a.ffFrame + 1) {
    a.fastTickStart = tick;
    a.fastWallStart = w;
  }
  if (w > a.watchdogSeconds) {
    a.watchdog = true;
    a.quitSent = true;
    c.line(wallSeconds(), f.frame, tick, "watchdog", "wall_s=" + fmt(w) + ";saved=" + std::to_string(a.lastSaved));
    c.line(wallSeconds(), f.frame, tick, "actors-at-watchdog", actorDump(f));
    shot(c, f.image, "attempt" + std::to_string(a.number) + "-watchdog.png");
    pushQuit();
    return;
  }
  if (f.endPhase != app::LevelEndPhase::Running) {
    if (!a.endingShot) {
      a.endingShot = true;
      a.endObservedWall = w;
      c.line(wallSeconds(), f.frame, tick, "end-phase", std::string(phaseName(f.endPhase)) + ";saved=" +
             std::to_string(a.lastSaved) + ";dead=" + std::to_string(a.lastDead) + ";remaining_ticks=" +
             std::to_string(a.lastRemainingTicks));
      c.line(wallSeconds(), f.frame, tick, "actors-at-end", actorDump(f));
      shot(c, f.image, "attempt" + std::to_string(a.number) + "-ending.png");
    }
    return;
  }
  if (f.paused || f.levelMenuOpen) return;  // never pressed by this driver
  if (f.cameraIntroActive && w < 30.0) return;

  // Verify the previous click one frame later (the event is processed at the start of the next frame).
  if (a.pending) {
    if (f.frame <= a.pendingData.frame) return;
    const auto& p = a.pendingData;
    const int stockNow = f.world.setup().skills[static_cast<std::size_t>(p.skill)];
    const bool spent = p.stockBefore >= 0 && stockNow < p.stockBefore;
    bool onTarget = false;
    if (p.lemming < f.world.lemmings().size()) {
      const auto& l = f.world.lemmings()[p.lemming];
      onTarget = (p.skill == 0 && l.canClimb && !p.climb) || (p.skill == 1 && l.canFloat && !p.floatr) ||
                 (p.skill == 2 && l.fuseMs >= 0) ||
                 (p.skill == 5 && l.state == sim::State::Basher) || (p.skill == 6 && l.state == sim::State::Miner);
    }
    if (spent) { ++a.assignOk; if (onTarget) ++a.onTarget; }
    else {
      ++a.assignFail;
      a.cooldownUntil[p.lemming] = f.frame + 8;
      if (p.skill == 2) { a.bombSites.erase(p.lemming); --a.bombsPrimed; }  // the click primed nobody
    }
    c.line(wallSeconds(), f.frame, tick, spent ? "assign-ok" : "assign-rejected",
           std::string("lemming=") + std::to_string(p.lemming) + ";skill=" + kSkillNames[p.skill] + ";stock=" +
           std::to_string(p.stockBefore) + "->" + std::to_string(stockNow) + ";on_target=" + std::to_string(onTarget));
    if (spent && a.shots < 6) {
      ++a.shots;
      shot(c, f.image, "attempt" + std::to_string(a.number) + "-assign" + std::to_string(a.shots) + ".png");
    }
    a.pending = false;
  }

  if (!a.ffClicked) {  // public Fast-forward plate, recorded; latch is never cleared by the clone
    a.ffClicked = true;
    a.ffFrame = f.frame;
    a.ffTick = tick;
    a.lastActionFrame = f.frame;
    pushClick(kFastForwardX, kFastForwardY);
    c.line(wallSeconds(), f.frame, tick, "click-fast-forward", "x=" + std::to_string(kFastForwardX) + ";y=" +
           std::to_string(kFastForwardY) + ";public SDL left click on plate 8");
    return;
  }
  if (f.frame - a.lastActionFrame < 2) return;

  const auto& lemmings = f.world.lemmings();
  const auto& stock = f.world.setup().skills;
  for (const auto& view : f.actors) {
    if (view.index >= lemmings.size()) continue;
    const auto& l = lemmings[view.index];
    if (!l.active()) continue;
    int want = desiredSkill(l, f.world.terrain(), stock, a.strategy);
    if (want < 0 && stock[2] > 0 && bomberWanted(a, l, f.world.terrain())) want = 2;
    if (want < 0) continue;
    if (a.tries[{view.index, want}] >= 3) continue;
    if (auto it = a.cooldownUntil.find(view.index); it != a.cooldownUntil.end() && f.frame < it->second) continue;
    const int x = static_cast<int>((view.placement.left + view.placement.right) / 2);
    const int y = static_cast<int>((view.placement.top + view.placement.bottom) / 2);
    if (x < 100 || x >= 640 || y < 0 || y >= 400) continue;
    if (f.selectedSkillIndex != want) {
      if (a.keyTries[static_cast<std::size_t>(want)]++ >= 6) { a.tries[{view.index, want}] = 99; continue; }
      pushSkillKey(want);
      a.lastActionFrame = f.frame;
      c.line(wallSeconds(), f.frame, tick, "key-select-skill", std::string(kSkillNames[want]) + ";key=" +
             std::to_string(want + 1));
      return;
    }
    ++a.tries[{view.index, want}];
    if (want == 2) { a.bombSites[view.index] = {l.x, l.y}; ++a.bombsPrimed; }
    a.pending = true;
    a.pendingData = {f.frame, view.index, want, stock[static_cast<std::size_t>(want)], l.canClimb, l.canFloat};
    a.lastActionFrame = f.frame;
    ++a.clicks;
    if (a.firstActWall < 0) a.firstActWall = w;
    pushClick(x, y);
    c.line(wallSeconds(), f.frame, tick, "click-lemming", "lemming=" + std::to_string(view.index) + ";skill=" +
           kSkillNames[want] + ";x=" + std::to_string(x) + ";y=" + std::to_string(y) + ";state=" +
           std::string(sim::stateName(l.state)) + ";pos=" + std::to_string(l.x) + "/" + std::to_string(l.y));
    return;
  }
}

std::string classify(const AttemptResult& r) {
  if (r.a.watchdog) return "INCOMPLETE (watchdog)";
  if (!r.resultsSeen) return "INCOMPLETE (no results screen observed)";
  return r.success && r.saved >= r.required ? "WIN" : "LOSS";
}

void writeAttempts(Ctx& c, std::ostream& os) {
  for (const auto& r : c.results) {
    const auto& a = r.a;
    const double simS = a.lastTick * 0.2;
    os << "attempt " << a.number << " strategy=" << (a.strategy == 1 ? "reactive" : "blanket") << " outcome=" << classify(r)
       << " wall_s=" << fmt(r.wallEnd - a.wall0, 1) << " sim_ticks=" << a.lastTick << " sim_s=" << fmt(simS, 1)
       << " spawned=" << a.lastSpawned << " saved=" << a.lastSaved << " dead=" << a.lastDead << " active=" << a.lastActive
       << " need=" << a.need << " total=" << a.total << " remaining_ticks=" << a.lastRemainingTicks
       << " clicks=" << a.clicks << " assign_ok=" << a.assignOk << " assign_rejected=" << a.assignFail
       << " on_target=" << a.onTarget << " ff_click_tick=" << a.ffTick << " watchdog_s=" << fmt(a.watchdogSeconds, 0);
    if (a.fastTickStart >= 0 && a.lastTick > a.fastTickStart && a.frames > 0)
      os << " post_ff_sim_per_wall=" << fmt((a.lastTick - a.fastTickStart) * 0.2 / std::max(0.001, (a.endObservedWall >= 0 ? a.endObservedWall : (r.wallEnd - a.wall0)) - a.fastWallStart), 2);
    os << " results_screen=" << r.resultsSeen;
    if (r.resultsSeen) os << " result_saved=" << r.saved << "/" << r.totalOut << " required=" << r.required
                          << " success_flag=" << r.success << " seconds_remaining=" << r.secondsRemaining;
    os << " stock_start=[";
    for (int i = 0; i < 8; ++i) os << (i ? "," : "") << a.stockStart[static_cast<std::size_t>(i)];
    os << "] stock_end=[";
    for (int i = 0; i < 8; ++i) os << (i ? "," : "") << a.stockEnd[static_cast<std::size_t>(i)];
    os << "]\n";
  }
}

std::string arr(const std::array<std::int32_t, 102>& a) {
  std::ostringstream os;
  for (std::size_t i = 0; i < a.size(); ++i) os << (i ? "," : "") << a[i];
  return os.str();
}

// ---- identity ----------------------------------------------------------------------------------
void writeIdentity(const fs::path& root, const fs::path& game) {
  std::ofstream os(root / "identity.txt");
  os << "driver=test_playablebaseline (scripted clone observation)\n";
  os << "built=" << __DATE__ << ' ' << __TIME__ << "\n";
#ifdef __VERSION__
  os << "compiler=" << __VERSION__ << "\n";
#endif
  os << "sdl_compiled=" << SDL_MAJOR_VERSION << '.' << SDL_MINOR_VERSION << '.' << SDL_PATCHLEVEL << "\n";
  os << "game_dir=" << game.string() << "\n";
  os << "lemmings.box_bytes=" << (fs::exists(game / "lemmings.box") ? static_cast<long long>(fs::file_size(game / "lemmings.box")) : -1)
     << " patch.box_bytes=" << (fs::exists(game / "patch.box") ? static_cast<long long>(fs::file_size(game / "patch.box")) : -1) << "\n";
  const char* files[] = {"src/app/main.cpp", "src/app/AppEntry.h", "src/app/FrontendSession.cpp", "src/app/FrontendWindow.cpp",
      "src/app/RuntimeSkillPanel.cpp", "src/app/RuntimeHud.cpp", "src/app/RuntimePick.cpp", "src/app/SimClock.cpp",
      "src/app/LevelEndTransition.cpp", "src/sim/World.cpp", "src/sim/RetailLevel.cpp", "src/save/SaveStore.cpp",
      "src/__test__/test_playablebaseline.cpp"};
  for (const char* f : files) os << "fnv1a64 " << f << " = " << fnvHex(fs::path(PB_SOURCE_DIR) / f) << "\n";
  os << "reference exe is never opened or executed by this driver\n";
}

// State of the clone's normal per-user save (read-only observation, before/after).
std::string normalSaveState() {
  auto base = save::localAppDataDirectory();
  if (!base) return "unresolved";
  const auto p = save::cloneSavePathUnder(*base);
  std::error_code ec;
  if (!fs::exists(p, ec)) return "absent:" + p.string();
  return "present:" + p.string() + " bytes=" + std::to_string(fs::file_size(p, ec)) + " fnv=" + fnvHex(p);
}

std::shared_ptr<save::SaveStore> storeFor(const fs::path& routeDir, const fs::path& game) {
  const auto file = save::cloneSavePathUnder(routeDir / "save");
  return std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(file, game));
}

int runApp(const std::string& gameDir, app::FrontendWindowObserver fo, app::LevelFrameObserver lo,
           std::shared_ptr<save::SaveStore> store) {
  std::vector<std::string> args{"playable-baseline", "--game-dir", gameDir};  // ordinary startup: no diagnostics
  std::vector<char*> argv;
  for (auto& s : args) argv.push_back(s.data());
  app::ApplicationRunOptions options;  // frameSeconds stays 0: production wall clock
  options.saveStore = std::move(store);
  return app::runApplication(static_cast<int>(argv.size()), argv.data(), std::move(fo), std::move(lo), options);
}

// ---- modes -------------------------------------------------------------------------------------
int surveyMode(const AssetLibrary& lib, const fs::path& root, const fs::path& game) {
  fs::create_directories(root);
  writeIdentity(root, game);
  auto map = app::MapScene::build(lib, {});
  if (!map) { std::cerr << "survey: " << map.error() << '\n'; return 1; }
  std::ofstream os(root / "level-survey.txt");
  os << "Decoded installed levels (read-only; sim::loadRetailLevel + Terrain queries).\n";
  int loaded = 0, failed = 0;
  for (std::size_t node = 0; node < 102; ++node) {
    auto name = map->levelForNode(node);
    if (!name) { os << "node " << node << ": no level name\n"; continue; }
    const auto f = inspect(lib, node, *name);
    if (f.ok) { ++loaded; os << f.summary << " | " << featureNames(f.features) << '\n'; }
    else { ++failed; os << *name << " node=" << node << " LOAD FAILED: " << f.error << '\n'; }
  }
  os << "loaded=" << loaded << " failed=" << failed << "\n";
  const auto fresh = surveyFreshMap(lib);
  os << "\nFrontend-selectable on the fresh (zero-progress) map:\n";
  for (const auto& l : fresh.selectable) os << "  node " << l.node << " at pixel (" << l.px << "," << l.py << "): " << l.summary << " | " << featureNames(l.features) << '\n';
  for (int route = 1; route <= 3; ++route) {
    std::string why;
    const auto pick = chooseRoute(fresh, route, why);
    os << "route " << route << ": " << (pick ? pick->level + " (node " + std::to_string(pick->node) + ")" : "NONE") << " -- " << why << '\n';
  }
  std::cout << "PlayableBaseline survey: loaded=" << loaded << " failed=" << failed
            << " freshSelectable=" << fresh.selectable.size() << " -> " << (root / "level-survey.txt").string() << '\n';
  return loaded > 0 && os ? 0 : 1;
}

int playMode(const AssetLibrary& lib, int route, const fs::path& root, const std::string& gameDir) {
  const fs::path game(gameDir);
  const fs::path routeDir = root / ("route-" + std::to_string(route));
  std::error_code ec;
  fs::remove_all(routeDir, ec);  // task-owned output directory under build/
  fs::create_directories(routeDir, ec);
  if (ec) { std::cerr << "cannot create " << routeDir << '\n'; return 1; }
  writeIdentity(root, game);

  const auto fresh = surveyFreshMap(lib);
  std::string why;
  const auto chosen = chooseRoute(fresh, route, why);
  Ctx c;
  c.dir = routeDir;
  c.trace.open(routeDir / "action-trace.csv");
  c.samples.open(routeDir / "samples.csv");
  c.trace << "wall_s,attempt_slot,frame,sim_tick,kind,detail\n";
  c.samples << "attempt,wall_s,frame,sim_tick,sim_s,spawned,active,saved,dead,remaining_ticks,end_phase,paused,selected_skill,"
               "stock0,stock1,stock2,stock3,stock4,stock5,stock6,stock7\n";
  std::ofstream sel(routeDir / "selection.txt");
  sel << "route " << route << " rationale: " << why << '\n';
  if (!chosen) {
    sel << "BLOCKED: no level could be selected\n";
    std::ofstream res(routeDir / "play-result.txt");
    res << "route=" << route << "\noutcome=BLOCKED (selection)\nwon=0\nreason=" << why << '\n';
    std::cout << "PlayableBaseline route " << route << ": BLOCKED (selection): " << why << '\n';
    return 0;
  }
  sel << chosen->summary << "\n";
  for (const auto& l : fresh.selectable) sel << "candidate: " << l.summary << " | " << featureNames(l.features) << '\n';

  const std::string normalBefore = normalSaveState();
  const auto saveFile = save::cloneSavePathUnder(routeDir / "save");
  auto store = storeFor(routeDir, game);

  const std::string target = chosen->level;
  c.lastProgressWall = wallSeconds();
  const double startWall = wallSeconds();
  const auto frontend = [&](const app::FrontendSession& s, int frame) {
    ++c.feCalls;
    if (c.lastScreen != static_cast<int>(s.screen())) {
      c.lastScreen = static_cast<int>(s.screen());
      c.lastProgressWall = wallSeconds();
      c.line(wallSeconds(), frame, 0, "frontend-screen", "screen=" + std::to_string(static_cast<int>(s.screen())) +
             ";counter=" + std::to_string(s.constructionCounter()) + ";selected=" + (s.selectedLevel() ? *s.selectedLevel() : "-"));
    }
    if (wallSeconds() - c.lastProgressWall > 60.0) {
      c.anomaly += "frontend stalled >60 s on screen " + std::to_string(static_cast<int>(s.screen())) + "; ";
      pushQuit();
      return;
    }
    const bool canPush = c.feCalls - c.lastPush >= 5;
    switch (s.screen()) {
      case shell::Screen::Menu:
        if (c.menuWall < 0) { c.menuWall = wallSeconds(); shot(c, s.composeFrame(), "frontend-menu.png"); }
        if (c.resultsHandledFor > 0) { pushQuit(); break; }
        if (canPush && c.menuClicks < 3) {
          ++c.menuClicks; c.lastPush = c.feCalls;
          pushClick(kPlayCurrentX, kPlayCurrentY);
          c.line(wallSeconds(), frame, 0, "click-play-current-game", "x=320;y=363");
        } else if (canPush) { c.anomaly += "Play Current Game click did not leave the menu; "; pushQuit(); }
        break;
      case shell::Screen::Map:
        if (c.resultsHandledFor > 0) {
          if (!c.returnedToMap) {
            c.returnedToMap = true;
            c.flagsAfter = s.completionFlags();
            c.statesAfter = s.mapStates();
            c.saveCountAtMap = s.saveCount();
            c.saveErrorAtMap = s.saveError();
            shot(c, s.composeFrame(), "map-after-results.png");
            c.line(wallSeconds(), frame, 0, "map-after-results", "saveCount=" + std::to_string(s.saveCount()) +
                   ";flag_node=" + std::to_string(chosen->node < 102 ? s.completionFlags()[chosen->node] : -1));
          }
          pushQuit();
          break;
        }
        if (c.mapWall < 0) { c.mapWall = wallSeconds(); shot(c, s.composeFrame(), "frontend-map-fresh.png"); }
        if (canPush && c.mapClicks < 3) {
          ++c.mapClicks; c.lastPush = c.feCalls;
          pushClick(chosen->px, chosen->py);
          c.line(wallSeconds(), frame, 0, "click-map-node", "node=" + std::to_string(chosen->node) + ";level=" + target +
                 ";x=" + std::to_string(chosen->px) + ";y=" + std::to_string(chosen->py));
        } else if (canPush) { c.anomaly += "map click did not select a level; "; pushQuit(); }
        break;
      case shell::Screen::Results: {
        const int attemptNo = static_cast<int>(c.results.size());
        if (c.resultsHandledFor < attemptNo) {
          c.resultsHandledFor = attemptNo;
          auto& r = c.results.back();
          r.resultsSeen = true;
          if (const auto& o = s.outcome()) {
            r.success = o->success; r.saved = o->saved; r.totalOut = o->total; r.required = o->required;
            r.secondsRemaining = o->secondsRemaining;
          }
          r.classification = classify(r);
          shot(c, s.composeFrame(), "attempt" + std::to_string(r.a.number) + "-results-screen.png");
          c.line(wallSeconds(), frame, 0, "results-screen", r.classification + ";saved=" + std::to_string(r.saved) + "/" +
                 std::to_string(r.totalOut) + ";required=" + std::to_string(r.required) + ";saveCount=" +
                 std::to_string(s.saveCount()) + ";saveError=" + (s.saveError() ? *s.saveError() : "-"));
          c.lastPush = -100;
          c.contClicks = 0;
          if (r.classification == "WIN") c.resultAction = 1;
          else if (r.a.number < 2) { c.resultAction = 2; c.nextAttempt = 2; }
          else c.resultAction = 3;
        }
        if (c.resultAction == 3) { pushQuit(); break; }
        if (c.feCalls - c.lastPush >= 5) {
          if (c.contClicks < 3) {
            ++c.contClicks; c.lastPush = c.feCalls;
            pushClick(kContinueX, kContinueY);
            c.line(wallSeconds(), frame, 0, c.resultAction == 1 ? "click-continue" : "click-retry", "x=320;y=360");
          } else {
            c.anomaly += "results click did not leave the results screen; "; pushQuit();
          }
        }
        break;
      }
      default:
        c.anomaly += "unexpected frontend screen " + std::to_string(static_cast<int>(s.screen())) + "; ";
        pushQuit();
        break;
    }
  };
  const auto level = [&](const app::LevelFrameView& f) {
    const int before = f.frame == 0 ? 1 : 0;
    levelObserver(c, f);
    if (before) {  // register the attempt so the results screen can attach to it
      AttemptResult r;
      r.a = c.att;
      c.results.push_back(r);
    }
    if (!c.results.empty()) {
      auto& r = c.results.back();
      const double keepStart = r.a.wall0;
      r.a = c.att;
      r.a.wall0 = keepStart;
      r.wallEnd = wallSeconds();
      r.a.watchdog = c.att.watchdog;
    }
  };

  std::cout << "PlayableBaseline route " << route << " target " << target << " save " << saveFile.string() << '\n';
  const int status = runApp(gameDir, frontend, level, store);
  const double totalWall = wallSeconds() - startWall;
  const std::string normalAfter = normalSaveState();

  bool won = false;
  for (const auto& r : c.results) if (classify(r) == "WIN") won = true;
  std::string outcome;
  if (!c.levelEntered) outcome = "BLOCKED (frontend never entered the level)";
  else if (won) outcome = "WIN";
  else if (!c.results.empty() && c.results.back().a.watchdog) outcome = "INCOMPLETE (watchdog)";
  else outcome = "LOSS";

  const bool saveExists = fs::exists(saveFile, ec);
  const auto saveBytes = saveExists ? static_cast<long long>(fs::file_size(saveFile, ec)) : -1;
  std::ofstream res(routeDir / "play-result.txt");
  res << "route=" << route << "\nlevel=" << target << "\nnode=" << chosen->node << "\noutcome=" << outcome << "\nwon=" << (won ? 1 : 0)
      << "\nrunApplication_status=" << status << "\nframe_entry=" << (c.levelEntered ? 1 : 0)
      << "\nentered_level=" << c.enteredLevel << "\nentered_frontend_counter=" << (c.enteredCounter ? std::to_string(*c.enteredCounter) : "none")
      << "\ntotal_wall_s=" << fmt(totalWall, 1) << "\nsave_file=" << saveFile.string() << "\nsave_exists=" << saveExists
      << "\nsave_bytes=" << saveBytes << "\nsave_fnv=" << (saveExists ? fnvHex(saveFile) : "-")
      << "\nsave_count_at_map=" << c.saveCountAtMap << "\nsave_error_at_map=" << (c.saveErrorAtMap ? *c.saveErrorAtMap : "-")
      << "\nreturned_to_map=" << (c.returnedToMap ? 1 : 0)
      << "\nflags_after=" << arr(c.flagsAfter) << "\nstates_after=" << arr(c.statesAfter)
      << "\nnormal_save_before=" << normalBefore << "\nnormal_save_after=" << normalAfter
      << "\nnormal_save_unchanged=" << (normalBefore == normalAfter ? 1 : 0) << "\nanomaly=" << (c.anomaly.empty() ? "-" : c.anomaly) << '\n';
  writeAttempts(c, res);
  std::ofstream md(routeDir / "result.md");
  md << "# Route " << route << ": " << target << " (node " << chosen->node << ")\n\nOutcome: **" << outcome << "**\n\n";
  md << "Selection: " << why << "\n\n```\n";
  writeAttempts(c, md);
  md << "```\n";

  std::cout << "PlayableBaseline route " << route << " " << target << ": " << outcome << " wall=" << fmt(totalWall, 1)
            << "s attempts=" << c.results.size() << " status=" << status << " anomaly=" << (c.anomaly.empty() ? "-" : c.anomaly) << '\n';
  writeAttempts(c, std::cout);

  // Driver integrity only: gameplay outcome is reported above and never decides pass/fail.
  bool integrity = status == 0 && res && md && c.trace && c.samples;
  if (c.levelEntered) integrity = integrity && !c.results.empty() && c.enteredLevel == target;
  if (!integrity) std::cerr << "FAIL: driver integrity (status=" << status << ", entered=" << c.enteredLevel << ")\n";
  return integrity ? 0 : 1;
}

std::map<std::string, std::string> readKv(const fs::path& p) {
  std::map<std::string, std::string> kv;
  std::ifstream in(p);
  std::string line;
  while (std::getline(in, line)) {
    const auto eq = line.find('=');
    if (eq != std::string::npos && line.compare(0, 8, "attempt ") != 0) kv.emplace(line.substr(0, eq), line.substr(eq + 1));
  }
  return kv;
}

int restartMode(const AssetLibrary& /*lib*/, int route, const fs::path& root, const std::string& gameDir) {
  const fs::path game(gameDir);
  const fs::path routeDir = root / ("route-" + std::to_string(route));
  std::ofstream md(routeDir / "restart-result.md");
  const auto kv = readKv(routeDir / "play-result.txt");
  if (kv.empty() || !kv.count("won") || kv.at("won") != "1") {
    md << "# Route " << route << " restart: UNVERIFIED\n\nNo genuine completion was observed by the play invocation"
          " (outcome=" << (kv.count("outcome") ? kv.at("outcome") : "no play result") << "); persistence not inspected.\n";
    std::cout << "PlayableBaseline restart route " << route << ": UNVERIFIED (no genuine completion)\n";
    return md ? 0 : 1;
  }
  const std::size_t node = static_cast<std::size_t>(std::stoul(kv.at("node")));
  const std::string saveFile = kv.at("save_file");
  struct Seen { int calls = 0; bool menuClick = false; bool map = false; std::array<std::int32_t, 102> flags{}, states{};
                std::optional<std::string> loadError; int saveCount = -1; int clicks = 0; int lastPush = -100; } seen;
  const auto frontend = [&](const app::FrontendSession& s, int frame) {
    ++seen.calls;
    if (seen.calls > 600) { pushQuit(); return; }
    if (s.screen() == shell::Screen::Menu) {
      if (seen.calls - seen.lastPush >= 5 && seen.clicks < 3) {
        ++seen.clicks; seen.lastPush = seen.calls; pushClick(kPlayCurrentX, kPlayCurrentY);
        seen.loadError = s.saveLoadError();
      } else if (seen.clicks >= 3) pushQuit();
    } else if (s.screen() == shell::Screen::Map) {
      if (!seen.map) {
        seen.map = true; seen.flags = s.completionFlags(); seen.states = s.mapStates(); seen.saveCount = s.saveCount();
        seen.loadError = s.saveLoadError();
        if (auto r = writePng(routeDir / "restart-map.png", s.composeFrame()); !r) std::cerr << r.error() << '\n';
      }
      pushQuit();
    } else { pushQuit(); }
    (void)frame;
  };
  const std::string before = fnvHex(saveFile);
  const int status = runApp(gameDir, frontend, {}, storeFor(routeDir, game));
  const std::string after = fnvHex(saveFile);
  const bool flagSet = node < 102 && seen.flags[node] == 1;
  const bool matches = arr(seen.flags) == kv.at("flags_after") && arr(seen.states) == kv.at("states_after");
  std::error_code ec;
  md << "# Route " << route << " restart (fresh process, same isolated save)\n\n"
     << "runApplication status=" << status << " reachedMap=" << seen.map << " loadError=" << (seen.loadError ? *seen.loadError : "-")
     << "\nnode " << node << " completion flag after restart = " << (node < 102 ? seen.flags[node] : -1)
     << "\nflags/states equal the play invocation's map: " << matches << "\nsaveCount after load (loading never writes) = " << seen.saveCount
     << "\nsave file bytes=" << (fs::exists(saveFile, ec) ? static_cast<long long>(fs::file_size(saveFile, ec)) : -1)
     << " fnv before=" << before << " after=" << after << " (unchanged: " << (before == after) << ")\n"
     << "RESULT: " << (status == 0 && seen.map && flagSet && matches && before == after ? "VERIFIED" : "FAILED") << "\n";
  std::cout << "PlayableBaseline restart route " << route << ": "
            << (status == 0 && seen.map && flagSet && matches && before == after ? "VERIFIED" : "FAILED (persistence finding)")
            << " flag=" << (node < 102 ? seen.flags[node] : -1) << '\n';
  return status == 0 && md ? 0 : 1;  // persistence mismatch is a reported finding, not a driver fault
}

}  // namespace
#endif

int main(int argc, char** argv) {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "PlayableBaseline: requires SDL2\n";
  return 1;
#else
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir) { std::cout << "PlayableBaseline: SKIPPED (set WEBOLUTION_GAME_DIR)\n"; return 0; }
  const char* video = std::getenv("SDL_VIDEODRIVER");
  if (!video || std::string(video) != "dummy") { std::cerr << "PlayableBaseline: refusing to run without SDL_VIDEODRIVER=dummy\n"; return 1; }
  if (argc < 4) { std::cerr << "usage: test_playablebaseline survey|play|restart <route> <output-dir>\n"; return 1; }
  const std::string mode = argv[1];
  const int route = std::atoi(argv[2]);
  const fs::path root(argv[3]);
  std::error_code ec;
  fs::create_directories(root, ec);
  auto library = AssetLibrary::open(fs::path(gameDir) / "lemmings.box", fs::path(gameDir) / "patch.box");
  if (!library) { std::cerr << "assets: " << library.error() << '\n'; return 1; }
  if (mode == "survey") return surveyMode(*library, root, fs::path(gameDir));
  if (route < 1 || route > 3) { std::cerr << "route must be 1..3\n"; return 1; }
  if (mode == "play") return playMode(*library, route, root, gameDir);
  if (mode == "restart") return restartMode(*library, route, root, gameDir);
  std::cerr << "unknown mode\n";
  return 1;
#endif
}
