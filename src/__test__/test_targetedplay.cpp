// targeted-play-001 assessment driver (SCRIPTED CLONE OBSERVATION, not human play, not original fidelity).
//
// Modes (see CMakeLists.txt registrations):
//   prepare 0 <dir>  new run dir; copy the retained, hash-verified node-0 save of playable-expansion-001 run 20261103-003501
//   play S <dir>     S=1 MM_BLOCK_001_A (node 3), terrain-informed Blocker plans, at most 3 attempts
//                    S=2 MM_W_012 (node 8, four levers), at most 2 attempts, only if node 8 is selectable on the earned map
//   restart S <dir>  fresh process over the same isolated save; UNVERIFIED unless the play invocation won
//
// Public inputs only (SDL events through the production runApplication). Targeting protocol, from the traced loop order in
// src/app/main.cpp (events are drained BEFORE runtimeSim.advance and the LevelFrameObserver runs AFTER composing the frame):
//   1. push SDL_MOUSEMOTION at pixel P            (sets pointerX/Y; no button, so no fast-forward latch)
//   2. next observed frame: LevelFrameView::hoveredActor is the production hoveredActor() for P at that frame's world
//   3. only if hover == intended: push SDL_MOUSEBUTTONDOWN at P. It is consumed at the top of the next iteration against the
//      same world (no sim.advance in between), so the recipient equals the observed hover by construction of the loop.
//   4. next observed frame: stock change + which actor entered the skill state = ACTUAL recipient (World::assign is atomic).
// Nothing here calls assign, writes a World/actor/terrain/stock/flag/progress/clock. Fast-forward is only clicked after the
// targeting window closed. Outcomes are REPORTED, never asserted: exit code = driver integrity only.
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
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifndef TP_SOURCE_DIR
#define TP_SOURCE_DIR "."
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
constexpr const char* kSourceSaveRel = "build/playable-expansion-001/runs/20261103-003501/save/LemmingsWebolution/SavedGame.dat";
constexpr const char* kSourceSaveFnv = "c05382077a330a32";  // accepted report playable-expansion-001, step 1 restart-result.md
constexpr long long kSourceSaveBytes = 1504;

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
bool pushMotion(int x, int y) {
  SDL_Event e{};
  e.type = SDL_MOUSEMOTION;
  e.motion.type = SDL_MOUSEMOTION;
  e.motion.state = 0;
  e.motion.x = x;
  e.motion.y = y;
  return SDL_PushEvent(&e) == 1;
}
bool pushSkillKey(int index) {
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
using Flags = std::array<std::int32_t, 102>;
std::string arr(const Flags& a) {
  std::ostringstream os;
  for (std::size_t i = 0; i < a.size(); ++i) os << (i ? "," : "") << a[i];
  return os.str();
}

// ---- terrain helpers (read-only) --------------------------------------------------------------------------
int wrapX(const sim::Terrain& t, int x) { const int w = std::max(1, t.width()); return ((x % w) + w) % w; }
bool solidWrap(const sim::Terrain& t, int x, int y) {
  if (y < 0 || y >= t.height() || t.width() <= 0) return false;
  return t.solid(wrapX(t, x), y);
}
int wrapDiff(const sim::Terrain& t, int a, int b) {
  const int w = std::max(1, t.width());
  int d = ((a - b) % w + w) % w;
  if (d > w / 2) d -= w;
  return d;
}
bool wallAhead(const sim::Lemming& l, const sim::Terrain& t, int reach) {
  const int dir = static_cast<int>(l.facing);
  for (int k = 1; k <= reach; ++k)
    if (solidWrap(t, l.x + dir * k, l.y - 3) && solidWrap(t, l.x + dir * k, l.y - 4)) return true;
  return false;
}
int floorDepth(const sim::Terrain& t, int x, int y) {
  for (int dy = -3; dy <= 80; ++dy)
    if (solidWrap(t, x, y + dy)) return dy;
  return 99;
}
bool hazardColumn(const sim::Terrain& t, int x, int y) {
  const int cx = wrapX(t, x);
  for (int dy = 0; dy <= 80; ++dy) {
    const int yy = y + dy;
    if (yy < 0 || yy >= t.height()) continue;
    if (t.waterHazard(cx, yy) || t.zoneWater(cx, yy) || t.zoneAcid(cx, yy)) return true;
  }
  return false;
}
struct Edge { bool found = false; int k = 0; int depth = 0; bool hazard = false; };
Edge edgeAhead(const sim::Lemming& l, const sim::Terrain& t, int maxK, int minDepth) {
  Edge e;
  const int dir = static_cast<int>(l.facing);
  const int d0 = floorDepth(t, l.x, l.y);
  for (int k = 1; k <= maxK; ++k) {
    const int d = floorDepth(t, l.x + dir * k, l.y);
    if (d < d0 - 2) break;
    if (d - d0 >= minDepth) { e.found = true; e.k = k; e.depth = d - d0; e.hazard = hazardColumn(t, l.x + dir * k, l.y); return e; }
  }
  return e;
}
int bandOf(int y) { return y / 16; }

// ---- plans -------------------------------------------------------------------------------------------------
struct Plan {
  int cap = 1;                 // intentional Blockers this attempt
  bool leverMode = false;      // node 8 attempt 2: Blocker left of a lever so a walker passes it, turns, and passes it again
  bool naturalOnly = false;    // node 8 attempt 1: no skill use, observe natural lever contact
  std::set<int> prefer, forbid;  // y/16 bands
  std::string reason = "initial";
};

struct Target { std::size_t idx = 0; std::string why; };

std::optional<Target> blockerSite(const Plan& p, int placedSoFar, const std::set<int>& usedBands, std::size_t idx, const sim::Lemming& l,
                                  const sim::Terrain& t, const std::vector<sim::Lever>& levers, int stock) {
  if (stock <= 0 || l.state != sim::State::Walker || placedSoFar >= p.cap || p.naturalOnly) return std::nullopt;
  const int band = bandOf(l.y);
  if (usedBands.count(band)) return std::nullopt;  // never two permanent Blockers on one ledge band (they trap walkers between them)
  if (p.leverMode) {
    for (std::size_t i = 0; i < levers.size(); ++i) {
      const int dx = wrapDiff(t, l.x, levers[i].x);
      const int dy = l.y - levers[i].y;
      if (dx >= -16 && dx <= -6 && dy >= 0 && dy <= levers[i].height) {
        std::ostringstream os;
        os << "left of lever " << i << " dx=" << dx << " dy=" << dy;
        return Target{idx, os.str()};
      }
    }
    return std::nullopt;
  }
  if (p.forbid.count(band)) return std::nullopt;
  if (!p.prefer.empty() && !p.prefer.count(band) && !p.prefer.count(band - 1) && !p.prefer.count(band + 1)) return std::nullopt;
  const Edge e = edgeAhead(l, t, 8, 12);
  if (!e.found || e.k < 4) return std::nullopt;
  if (!(e.hazard || e.depth >= 24)) return std::nullopt;
  if (wallAhead(l, t, 3)) return std::nullopt;
  std::ostringstream os;
  os << "drop edge k=" << e.k << " depth=" << e.depth << " hazardBelow=" << e.hazard << " band=" << band;
  return Target{idx, os.str()};
}

// ---- records -------------------------------------------------------------------------------------------------
struct TargetRecord {
  int n = 0;
  std::size_t intended = 0;
  std::string why;
  int probes = 0;
  std::vector<std::string> probeLog;
  long decisionTick = -1, hoverTick = -1, clickTick = -1, resolveTick = -1;
  int px = -1, py = -1, cursorX = -1, cursorY = -1;
  std::string hover = "-", recipient = "-", klass = "pending";
  int stockBefore = -1, stockAfter = -1;
  std::string intendedStateBefore, intendedPos;
};
struct BlockerTrack {
  std::size_t idx = 0; int x = 0, y = 0; long tick0 = 0, tickEnd = -1; bool ended = false;
  int turnsPerTick = 0, passPerTick = 0, passGap = 0, turnsGap = 0;
  std::set<std::size_t> turned;
};
struct DeathRec { long tick; std::size_t idx; std::string from, to; int x, y; };

struct Attempt {
  int number = 1;
  Plan plan;
  double wall0 = 0.0, watchdogSeconds = 0.0;
  int frames = 0;
  bool ffClicked = false, ffSkippedNatural = false;
  long ffTick = -1;
  std::size_t placedCount = 0;
  std::set<int> usedBands;
  long lastPlaceTick = -1;
  // targeting state machine
  int phase = 0;  // 0 idle, 1 probing, 2 resolving
  TargetRecord cur;
  int probeIndex = 0, motionFrame = -1, clickFrame = -1;
  std::vector<sim::State> snapState;
  std::map<std::size_t, int> cooldownUntilTick;
  std::vector<TargetRecord> targets;
  bool keyPressed = false;
  int lastKeyFrame = -100;
  // observation / sampling
  long lastTick = -1;
  long framesObserved = 0, ticksObservedNormal = 0, ticksSkippedNormal = 0, ticksSkippedFF = 0, maxGap = 0, framesNoTickAdvance = 0;
  std::vector<std::pair<long, long>> skipRanges;
  bool endingShot = false, watchdog = false, quitSent = false, tracksFinal = false;
  double lastSampleWall = -100.0;
  int lastSpawned = 0, lastSaved = 0, lastDead = 0, lastActive = 0;
  long lastRemainingTicks = 0;
  std::array<int, 8> stockStart{}, stockEnd{};
  int total = 0, need = 0;
  long limitTicks = 0;
  std::vector<sim::State> prevState;
  std::vector<sim::Facing> prevFacing;
  std::vector<int> prevX, prevY;
  long prevTick = -1;
  std::vector<int> leverState;
  int leverToggles = 0;
  std::vector<BlockerTrack> blockers;
  std::vector<DeathRec> deaths;
  std::vector<std::pair<int, int>> strandedCheck;
  std::string endActors;
  std::map<int, int> strandBandCount;  // filled at end: band -> active walkers
};
struct AttemptResult {
  Attempt a;
  bool resultsSeen = false, success = false;
  int saved = 0, totalOut = 0, required = 0, secondsRemaining = 0;
  double wallEnd = 0.0;
};

struct StepDef { int step; const char* label; std::size_t node; int maxAttempts; bool lever; };
constexpr StepDef kSteps[2] = {{1, "node3-blocker", 3, 3, false}, {2, "node8-lever", 8, 2, true}};

struct Ctx {
  fs::path dir;
  StepDef step{};
  std::ofstream trace, samples;
  int nextAttempt = 1;
  Plan nextPlan;
  Attempt att;
  std::vector<AttemptResult> results;
  bool attemptOpen = false, levelEntered = false;
  std::string enteredLevel;
  std::string anomaly, targetLevel, blocked;
  std::size_t targetNode = 0;
  int feCalls = 0, menuClicks = 0, mapClicks = 0, contClicks = 0, lastPush = -100;
  double lastProgressWall = 0.0;
  int resultsHandledFor = 0, resultAction = 0, lastScreen = -1;
  bool mapSeen = false, returnedToMap = false;
  int clickPixX = -1, clickPixY = -1;
  Flags flagsAtMap{}, statesAtMap{}, flagsAfter{}, statesAfter{};
  int saveCountAtMap = -1;
  std::optional<std::string> saveErrorAtMap;
  bool terrainDumped = false;
  void line(double w, int frame, long tick, const std::string& kind, const std::string& detail) {
    trace << fmt(w, 3) << ',' << nextAttempt << ',' << frame << ',' << tick << ',' << kind << ',' << detail << '\n';
    trace.flush();
  }
};

const char* phaseName(app::LevelEndPhase p) {
  return p == app::LevelEndPhase::Running ? "Running" : p == app::LevelEndPhase::Pending ? "Pending" : "Results";
}
void shot(Ctx& c, const Image& image, const std::string& name) {
  if (auto r = writePng(c.dir / name, image); !r) c.anomaly += "png " + name + ": " + r.error() + "; ";
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

// ---- tracking (read-only) -------------------------------------------------------------------------------
void trackFrame(Ctx& c, Attempt& a, const app::LevelFrameView& f) {
  const auto& world = f.world;
  const auto& lemmings = world.lemmings();
  const auto& t = world.terrain();
  const long tick = static_cast<long>(world.tickCount());
  const bool contiguous = a.prevTick >= 0 && tick == a.prevTick + 1;
  if (a.prevState.size() != lemmings.size()) {
    a.prevState.assign(lemmings.size(), sim::State::Faller);
    a.prevFacing.assign(lemmings.size(), sim::Facing::Right);
    a.prevX.assign(lemmings.size(), 0);
    a.prevY.assign(lemmings.size(), 0);
    for (std::size_t i = 0; i < lemmings.size(); ++i) { a.prevState[i] = lemmings[i].state; a.prevFacing[i] = lemmings[i].facing; a.prevX[i] = lemmings[i].x; a.prevY[i] = lemmings[i].y; }
  }
  const auto& levers = world.setup().levers;
  if (a.leverState.size() != levers.size()) {
    a.leverState.assign(levers.size(), 0);
    for (std::size_t i = 0; i < levers.size(); ++i) a.leverState[i] = levers[i].state;
  }
  for (std::size_t i = 0; i < levers.size(); ++i)
    if (levers[i].state != a.leverState[i]) {
      ++a.leverToggles;
      std::ostringstream os;
      os << "lever=" << i << ";state=" << a.leverState[i] << "->" << levers[i].state << ";at=" << levers[i].x << '/' << levers[i].y
         << ";prev_tick=" << a.prevTick << ";tick_gap=" << (a.prevTick >= 0 ? tick - a.prevTick : -1) << ";lemmings_in_aabb=";
      for (std::size_t j = 0; j < lemmings.size(); ++j) {
        const auto& l = lemmings[j];
        if (!l.alive()) continue;
        if (std::abs(wrapDiff(t, l.x, levers[i].x)) <= levers[i].width + 3 && std::abs(l.y - levers[i].y) <= levers[i].height + 3)
          os << j << '(' << sim::stateName(l.state) << ",face=" << static_cast<int>(l.facing) << ",pos=" << l.x << '/' << l.y << ")";
      }
      c.line(wallSeconds(), f.frame, tick, "lever-toggle", os.str());
      a.leverState[i] = levers[i].state;
    }
  for (std::size_t i = 0; i < lemmings.size(); ++i) {
    const auto& l = lemmings[i];
    const sim::State ps = a.prevState[i];
    if (l.state != ps) {
      if (l.state == sim::State::Blocker) {
        BlockerTrack b; b.idx = i; b.x = l.x; b.y = l.y; b.tick0 = tick;
        a.blockers.push_back(b);
        c.line(wallSeconds(), f.frame, tick, "state-blocker", "lemming=" + std::to_string(i) + ";from=" + std::string(sim::stateName(ps)) + ";pos=" + std::to_string(l.x) + "/" + std::to_string(l.y));
      }
      if ((l.state == sim::State::Dead || l.state == sim::State::Splatter) && a.deaths.size() < 60) {
        a.deaths.push_back({tick, i, std::string(sim::stateName(ps)), std::string(sim::stateName(l.state)), a.prevX[i], a.prevY[i]});
        c.line(wallSeconds(), f.frame, tick, "state-death", "lemming=" + std::to_string(i) + ";from=" + std::string(sim::stateName(ps)) + ";to=" + std::string(sim::stateName(l.state)) + ";last_pos=" + std::to_string(a.prevX[i]) + "/" + std::to_string(a.prevY[i]) + ";pos=" + std::to_string(l.x) + "/" + std::to_string(l.y));
      }
    }
  }
  // Blocker interaction, resolved only across CONSECUTIVE ticks (gap==1); otherwise counted as sampling-gap unresolved.
  for (auto& b : a.blockers) {
    if (b.ended) continue;
    if (!(b.idx < lemmings.size() && lemmings[b.idx].state == sim::State::Blocker)) { b.ended = true; b.tickEnd = tick; continue; }
    for (std::size_t j = 0; j < lemmings.size(); ++j) {
      if (j == b.idx || !lemmings[j].alive()) continue;
      const auto& l = lemmings[j];
      if (l.state != sim::State::Walker || a.prevState[j] != sim::State::Walker) continue;
      const int dx = wrapDiff(t, l.x, b.x), pdx = wrapDiff(t, a.prevX[j], b.x);
      if (std::abs(l.y - b.y) > 8 || std::abs(a.prevY[j] - b.y) > 8) continue;
      const bool turned = a.prevFacing[j] != l.facing;
      const bool signChange = (pdx < 0 && dx > 0) || (pdx > 0 && dx < 0);
      if (std::abs(pdx) > 14 && std::abs(dx) > 14) continue;
      if (turned && std::abs(dx) <= 14) {
        if (contiguous) ++b.turnsPerTick; else ++b.turnsGap;
        if (b.turned.insert(j).second)
          c.line(wallSeconds(), f.frame, tick, "blocker-turn", "blocker=" + std::to_string(b.idx) + ";lemming=" + std::to_string(j) + ";dx=" + std::to_string(pdx) + "->" + std::to_string(dx) + ";consecutive_tick=" + std::to_string(contiguous));
      } else if (signChange && std::abs(dx) <= 8 && std::abs(pdx) <= 8) {
        if (contiguous) {
          ++b.passPerTick;
          c.line(wallSeconds(), f.frame, tick, "blocker-pass-PER-TICK", "blocker=" + std::to_string(b.idx) + ";lemming=" + std::to_string(j) + ";dx=" + std::to_string(pdx) + "->" + std::to_string(dx) + ";face=" + std::to_string(static_cast<int>(l.facing)) + ";dy=" + std::to_string(l.y - b.y) + ";prev_dy=" + std::to_string(a.prevY[j] - b.y));
        } else {
          ++b.passGap;
          c.line(wallSeconds(), f.frame, tick, "blocker-sign-change-GAP-UNRESOLVED", "blocker=" + std::to_string(b.idx) + ";lemming=" + std::to_string(j) + ";dx=" + std::to_string(pdx) + "->" + std::to_string(dx) + ";tick_gap=" + std::to_string(tick - a.prevTick));
        }
      }
    }
  }
  for (std::size_t i = 0; i < lemmings.size(); ++i) { a.prevState[i] = lemmings[i].state; a.prevFacing[i] = lemmings[i].facing; a.prevX[i] = lemmings[i].x; a.prevY[i] = lemmings[i].y; }
  a.prevTick = tick;
}

void finalizeTracks(Ctx& c, Attempt& a, const app::LevelFrameView& f, const char* why) {
  if (a.tracksFinal) return;
  a.tracksFinal = true;
  const long tick = static_cast<long>(f.world.tickCount());
  const auto& lemmings = f.world.lemmings();
  const auto& t = f.world.terrain();
  for (auto& b : a.blockers) {
    if (!b.ended) { b.ended = true; b.tickEnd = tick; }
    int stranded = 0;  // active walkers on the same ledge band within 40 cells at the end
    for (std::size_t j = 0; j < lemmings.size(); ++j)
      if (j != b.idx && lemmings[j].state == sim::State::Walker && bandOf(lemmings[j].y) == bandOf(b.y) && std::abs(wrapDiff(t, lemmings[j].x, b.x)) <= 40) ++stranded;
    a.strandBandCount[bandOf(b.y)] += stranded;
    std::ostringstream os;
    os << "lemming=" << b.idx << ";why=" << why << ";at=" << b.x << '/' << b.y << ";ticks_standing=" << (b.tickEnd - b.tick0) << ";turned_near=" << b.turned.size()
       << ";turns_per_tick=" << b.turnsPerTick << ";turns_gap=" << b.turnsGap << ";pass_per_tick=" << b.passPerTick << ";sign_change_gap_unresolved=" << b.passGap
       << ";walkers_on_band_within40_at_end=" << stranded;
    c.line(wallSeconds(), f.frame, tick, "blocker-summary", os.str());
  }
}

void beginAttempt(Ctx& c, const app::LevelFrameView& f) {
  c.att = Attempt{};
  c.attemptOpen = true;
  c.att.number = c.nextAttempt;
  c.att.plan = c.nextPlan;
  c.att.wall0 = wallSeconds();
  const auto& s = f.world.setup();
  c.att.stockStart = s.skills;
  c.att.total = s.totalLemmings;
  c.att.need = s.saveTarget;
  c.att.limitTicks = s.timeLimitTicks;
  c.att.watchdogSeconds = 420.0;
  if (!c.levelEntered) { c.levelEntered = true; c.enteredLevel = std::string(f.levelName); }
  if (!c.terrainDumped) {  // read-only hazard/solid dump for the report ('h' = hazard cell inside solid, '~' hazard in open space)
    c.terrainDumped = true;
    std::ofstream os(c.dir / "terrain-hazard.txt");
    const auto& t = f.world.terrain();
    os << f.levelName << ' ' << t.width() << 'x' << t.height() << " '#' solid '~' hazard(open) 'h' hazard(in solid) 'H' hatch 'E' exit 'L' lever; row index = y\n";
    std::vector<std::string> rows(static_cast<std::size_t>(t.height()), std::string(static_cast<std::size_t>(t.width()), '.'));
    for (int y = 0; y < t.height(); ++y)
      for (int x = 0; x < t.width(); ++x) {
        const bool hz = t.waterHazard(x, y) || t.zoneWater(x, y) || t.zoneAcid(x, y);
        rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = hz ? (t.solid(x, y) ? 'h' : '~') : (t.solid(x, y) ? '#' : '.');
      }
    auto mark = [&](int x, int y, char ch) { if (y >= 0 && y < t.height() && x >= 0 && x < t.width()) rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = ch; };
    for (const auto& e : s.entrances) mark(e.x, e.y, 'H');
    for (const auto& e : s.exits) mark(e.x, e.y, 'E');
    for (const auto& l : s.levers) mark(l.x, l.y, 'L');
    for (std::size_t y = 0; y < rows.size(); ++y) os << y << '\t' << rows[y] << '\n';
  }
  std::ostringstream os;
  os << "attempt=" << c.att.number << ";plan_cap=" << c.att.plan.cap << ";lever_mode=" << c.att.plan.leverMode << ";natural_only=" << c.att.plan.naturalOnly << ";prefer=";
  for (int b : c.att.plan.prefer) os << b << '/';
  os << ";forbid=";
  for (int b : c.att.plan.forbid) os << b << '/';
  os << ";revision_reason=" << c.att.plan.reason << ";level=" << f.levelName << ";limit_ticks=" << s.timeLimitTicks << ";total=" << s.totalLemmings << ";need=" << s.saveTarget
     << ";levers=" << s.levers.size();
  c.line(wallSeconds(), f.frame, static_cast<long>(f.world.tickCount()), "attempt-begin", os.str());
  shot(c, f.image, "attempt" + std::to_string(c.att.number) + "-first-frame.png");
}

std::string probeStr(int px, int py, const app::LevelFrameView& f) {
  return "(" + std::to_string(px) + "," + std::to_string(py) + ")->cursor(" + std::to_string(f.cursorX) + "," + std::to_string(f.cursorY) + ")hover=" +
         (f.hoveredActor ? std::to_string(*f.hoveredActor) : "-") + "@tick" + std::to_string(f.world.tickCount());
}

void levelObserver(Ctx& c, const app::LevelFrameView& f) {
  if (f.frame == 0) beginAttempt(c, f);
  Attempt& a = c.att;
  if (a.quitSent) return;
  ++a.frames;
  const double w = wallSeconds() - a.wall0;
  const long tick = static_cast<long>(f.world.tickCount());
  const bool ffNow = a.ffClicked;
  if (a.lastTick >= 0) {  // sampling-gap accounting: every skipped tick range is recorded
    const long gap = tick - a.lastTick;
    ++a.framesObserved;
    if (gap == 0) ++a.framesNoTickAdvance;
    if (gap > 1) {
      (ffNow ? a.ticksSkippedFF : a.ticksSkippedNormal) += gap - 1;
      a.maxGap = std::max(a.maxGap, gap);
      if (a.skipRanges.size() < 40) a.skipRanges.emplace_back(a.lastTick + 1, tick - 1);
    }
    if (gap >= 1 && !ffNow) ++a.ticksObservedNormal;
  }
  a.lastTick = tick;
  a.lastSpawned = f.world.spawned(); a.lastSaved = f.world.saved(); a.lastDead = f.world.dead(); a.lastActive = f.world.activeCount();
  a.lastRemainingTicks = static_cast<long>(f.world.timeRemainingTicks());
  a.stockEnd = f.world.setup().skills;

  trackFrame(c, a, f);

  if (w - a.lastSampleWall >= 5.0) {
    a.lastSampleWall = w;
    c.samples << a.number << ',' << fmt(w, 2) << ',' << f.frame << ',' << tick << ',' << a.lastSpawned << ',' << a.lastActive << ',' << a.lastSaved << ',' << a.lastDead
              << ',' << a.lastRemainingTicks << ',' << phaseName(f.endPhase) << ',' << f.selectedSkillIndex << ',' << a.stockEnd[3] << ',' << ffNow << '\n';
    c.samples.flush();
  }
  if (w > a.watchdogSeconds) {
    a.watchdog = true; a.quitSent = true;
    c.line(wallSeconds(), f.frame, tick, "watchdog", "wall_s=" + fmt(w) + ";saved=" + std::to_string(a.lastSaved));
    c.line(wallSeconds(), f.frame, tick, "actors-at-watchdog", actorDump(f));
    finalizeTracks(c, a, f, "watchdog");
    pushQuit();
    return;
  }
  if (f.endPhase != app::LevelEndPhase::Running) {
    if (!a.endingShot) {
      a.endingShot = true;
      c.line(wallSeconds(), f.frame, tick, "end-phase", std::string(phaseName(f.endPhase)) + ";saved=" + std::to_string(a.lastSaved) + ";dead=" + std::to_string(a.lastDead) + ";remaining_ticks=" + std::to_string(a.lastRemainingTicks));
      a.endActors = actorDump(f);
      c.line(wallSeconds(), f.frame, tick, "actors-at-end", a.endActors);
      finalizeTracks(c, a, f, "level ended");
      shot(c, f.image, "attempt" + std::to_string(a.number) + "-ending.png");
    }
    return;
  }
  if (f.paused || f.levelMenuOpen) return;  // pause is never pressed by this driver
  if (f.cameraIntroActive && w < 30.0) return;

  const auto& lemmings = f.world.lemmings();
  const auto& stock = f.world.setup().skills;
  const int skill = 3;

  // ---- resolve the click pushed on the previous frame --------------------------------------------------------
  if (a.phase == 2) {
    if (f.frame <= a.clickFrame) return;
    TargetRecord& r = a.cur;
    r.resolveTick = tick;
    r.stockAfter = stock[skill];
    std::vector<std::size_t> became;
    for (std::size_t j = 0; j < lemmings.size() && j < a.snapState.size(); ++j)
      if (lemmings[j].state == sim::State::Blocker && a.snapState[j] != sim::State::Blocker) became.push_back(j);
    if (r.stockAfter >= r.stockBefore || became.empty()) {
      r.klass = (r.stockAfter < r.stockBefore) ? "AMBIGUOUS (stock spent, no new Blocker identified)" : "REJECTED (no stock change)";
      r.recipient = became.empty() ? "-" : std::to_string(became[0]);
      a.cooldownUntilTick[r.intended] = static_cast<int>(tick) + 25;
    } else if (became.size() == 1) {
      r.recipient = std::to_string(became[0]);
      r.klass = (became[0] == r.intended && r.hover == std::to_string(r.intended)) ? "CONFIRMED (hover==intended==recipient)" : "MISDIRECTED (recipient != intended)";
      ++a.placedCount;
      a.lastPlaceTick = tick;
      a.usedBands.insert(bandOf(lemmings[became[0]].y));
    } else {
      r.recipient = "multiple";
      r.klass = "AMBIGUOUS (multiple new Blockers)";
      ++a.placedCount;
    }
    std::ostringstream os;
    os << "n=" << r.n << ";intended=" << r.intended << ";hover=" << r.hover << ";recipient=" << r.recipient << ";class=" << r.klass << ";cursor=" << r.cursorX << ',' << r.cursorY
       << ";pixel=" << r.px << ',' << r.py << ";skill=Blocker;stock=" << r.stockBefore << "->" << r.stockAfter << ";intended_state_before=" << r.intendedStateBefore << '@' << r.intendedPos
       << ";decision_tick=" << r.decisionTick << ";hover_obs_tick=" << r.hoverTick << ";click_push_tick=" << r.clickTick << ";resolve_obs_tick=" << r.resolveTick
       << ";delay_decision_to_resolve_ticks=" << (r.resolveTick - r.decisionTick) << ";delay_hover_obs_to_resolve_obs_ticks=" << (r.resolveTick - r.hoverTick)
       << ";probes=" << r.probes << ";why=" << r.why;
    c.line(wallSeconds(), f.frame, tick, "assignment-result", os.str());
    shot(c, f.image, "attempt" + std::to_string(a.number) + "-assign" + std::to_string(r.n) + ".png");
    a.targets.push_back(r);
    a.phase = 0;
    // fall through: may immediately plan another action below
  }

  // ---- policy gating: normal speed until the targeting window closes, then the public Fast-forward plate --------
  const Plan& plan = a.plan;
  const bool noMoreSkill = plan.naturalOnly || a.placedCount >= static_cast<std::size_t>(plan.cap) || stock[skill] <= 0;
  const bool tailDone = a.lastPlaceTick >= 0 ? tick >= a.lastPlaceTick + 100 : true;
  const bool windowClosedNow = tick >= 450 || (noMoreSkill && tailDone && a.phase == 0);
  if (a.phase == 0 && windowClosedNow && !a.ffClicked) {
    a.ffClicked = true;
    a.ffTick = tick;
    pushClick(kFastForwardX, kFastForwardY);
    c.line(wallSeconds(), f.frame, tick, "click-fast-forward", "x=54;y=348;public SDL left click on plate 8; targeting window closed (placed=" + std::to_string(a.placedCount) + ")");
    return;
  }
  if (a.ffClicked || noMoreSkill) return;

  // ---- probing: evaluate the hover the production picker reported for the pixel pushed on the previous frame ----
  if (a.phase == 1) {
    if (f.frame <= a.motionFrame) return;
    TargetRecord& r = a.cur;
    ++r.probes;
    r.hoverTick = tick;
    r.hover = f.hoveredActor ? std::to_string(*f.hoveredActor) : "-";
    r.cursorX = f.cursorX; r.cursorY = f.cursorY;
    if (r.probeLog.size() < 12) r.probeLog.push_back(probeStr(r.px, r.py, f));
    const auto& li = lemmings[r.intended];
    const auto still = blockerSite(plan, static_cast<int>(a.placedCount), a.usedBands, r.intended, li, f.world.terrain(), f.world.setup().levers, stock[skill]);
    const bool cursorOk = f.cursorX == r.px && f.cursorY == r.py;
    if (!still) {  // intended actor no longer satisfies the site rule
      c.line(wallSeconds(), f.frame, tick, "target-dropped", "intended=" + std::to_string(r.intended) + ";state=" + std::string(sim::stateName(li.state)) + ";pos=" + std::to_string(li.x) + "/" + std::to_string(li.y));
      a.cooldownUntilTick[r.intended] = static_cast<int>(tick) + 15;
      a.phase = 0;
      return;
    }
    if (cursorOk && f.hoveredActor && *f.hoveredActor == r.intended && f.selectedSkillIndex == skill) {
      r.clickTick = tick;
      a.snapState.assign(lemmings.size(), sim::State::Faller);
      for (std::size_t j = 0; j < lemmings.size(); ++j) a.snapState[j] = lemmings[j].state;
      r.stockBefore = stock[skill];
      r.intendedStateBefore = sim::stateName(li.state);
      r.intendedPos = std::to_string(li.x) + "/" + std::to_string(li.y);
      pushClick(r.px, r.py);
      a.clickFrame = f.frame;
      a.phase = 2;
      c.line(wallSeconds(), f.frame, tick, "click-pushed", "intended=" + std::to_string(r.intended) + ";hover_confirmed=" + r.hover + ";pixel=" + std::to_string(r.px) + "," + std::to_string(r.py) + ";probes=" + std::to_string(r.probes));
      return;
    }
    if (r.probes >= 9) {  // pixel search exhausted: NOT a successful targeting, no click was made
      c.line(wallSeconds(), f.frame, tick, "targeting-unresolved", "intended=" + std::to_string(r.intended) + ";last_hover=" + r.hover + ";probes=" + std::to_string(r.probes) + ";no click pushed");
      r.klass = "UNRESOLVED (no pixel produced hover==intended; no click)";
      a.targets.push_back(r);
      a.cooldownUntilTick[r.intended] = static_cast<int>(tick) + 60;
      a.phase = 0;
      return;
    }
    // next candidate pixel around the intended actor's CURRENT projected box
    static const int off[9][2] = {{0, 0}, {-8, 0}, {8, 0}, {0, -8}, {0, 8}, {-14, -14}, {14, -14}, {-14, 14}, {14, 14}};
    for (const auto& view : f.actors) {
      if (view.index != r.intended) continue;
      const int cx = static_cast<int>((view.placement.left + view.placement.right) / 2);
      const int cy = static_cast<int>((view.placement.top + view.placement.bottom) / 2);
      const int k = r.probes;  // 1..8
      r.px = cx + off[k][0]; r.py = cy + off[k][1];
      pushMotion(r.px, r.py);
      a.motionFrame = f.frame;
      return;
    }
    c.line(wallSeconds(), f.frame, tick, "target-dropped", "intended=" + std::to_string(r.intended) + ";no projected box this frame");
    a.cooldownUntilTick[r.intended] = static_cast<int>(tick) + 15;
    a.phase = 0;
    return;
  }

  // ---- idle: select Blocker, then choose an intended actor ----------------------------------------------------
  if (f.selectedSkillIndex != skill) {
    if (f.frame - a.lastKeyFrame >= 3) { pushSkillKey(skill); a.lastKeyFrame = f.frame; c.line(wallSeconds(), f.frame, tick, "key-select-skill", "Blocker;key=4"); }
    return;
  }
  for (const auto& view : f.actors) {
    if (view.index >= lemmings.size()) continue;
    const auto& l = lemmings[view.index];
    if (!l.active()) continue;
    if (auto it = a.cooldownUntilTick.find(view.index); it != a.cooldownUntilTick.end() && tick < it->second) continue;
    const auto site = blockerSite(plan, static_cast<int>(a.placedCount), a.usedBands, view.index, l, f.world.terrain(), f.world.setup().levers, stock[skill]);
    if (!site) continue;
    const int cx = static_cast<int>((view.placement.left + view.placement.right) / 2);
    const int cy = static_cast<int>((view.placement.top + view.placement.bottom) / 2);
    if (cx < 100 || cx >= 640 || cy < 0 || cy >= 400) continue;
    a.cur = TargetRecord{};
    a.cur.n = static_cast<int>(a.targets.size()) + 1;
    a.cur.intended = view.index;
    a.cur.why = site->why;
    a.cur.decisionTick = tick;
    a.cur.px = cx; a.cur.py = cy;
    a.phase = 1;
    pushMotion(cx, cy);
    a.motionFrame = f.frame;
    c.line(wallSeconds(), f.frame, tick, "target-decision", "intended=" + std::to_string(view.index) + ";state=" + std::string(sim::stateName(l.state)) + ";pos=" + std::to_string(l.x) + "/" + std::to_string(l.y) + ";face=" + std::to_string(static_cast<int>(l.facing)) + ";box_centre=" + std::to_string(cx) + "," + std::to_string(cy) + ";why=" + site->why);
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
    int conf = 0, mis = 0, amb = 0, rej = 0, unres = 0;
    for (const auto& t : a.targets) {
      if (t.klass.rfind("CONFIRMED", 0) == 0) ++conf;
      else if (t.klass.rfind("MISDIRECTED", 0) == 0) ++mis;
      else if (t.klass.rfind("AMBIGUOUS", 0) == 0) ++amb;
      else if (t.klass.rfind("REJECTED", 0) == 0) ++rej;
      else ++unres;
    }
    os << "attempt " << a.number << " outcome=" << classify(r) << " wall_s=" << fmt(r.wallEnd - a.wall0, 1) << " sim_ticks=" << a.lastTick << " spawned=" << a.lastSpawned
       << " saved=" << a.lastSaved << " dead=" << a.lastDead << " active=" << a.lastActive << " need=" << a.need << " total=" << a.total << " plan_cap=" << a.plan.cap
       << " natural_only=" << a.plan.naturalOnly << " lever_mode=" << a.plan.leverMode << " assignments_CONFIRMED=" << conf << " MISDIRECTED=" << mis << " AMBIGUOUS=" << amb
       << " REJECTED=" << rej << " UNRESOLVED=" << unres << " lever_toggles=" << a.leverToggles << " ff_click_tick=" << a.ffTick << " frames_observed=" << a.framesObserved
       << " frames_without_tick_advance=" << a.framesNoTickAdvance << " ticks_skipped_normal_speed=" << a.ticksSkippedNormal << " ticks_skipped_after_FF=" << a.ticksSkippedFF
       << " max_tick_gap=" << a.maxGap << " results_screen=" << r.resultsSeen;
    if (r.resultsSeen) os << " result_saved=" << r.saved << "/" << r.totalOut << " required=" << r.required << " success_flag=" << r.success;
    os << "\n  revision_reason=" << a.plan.reason << "\n  deaths=" << a.deaths.size() << ":";
    for (const auto& d : a.deaths) os << " [t" << d.tick << " lem" << d.idx << ' ' << d.from << "->" << d.to << " last@" << d.x << '/' << d.y << ']';
    os << "\n  skipped_tick_ranges(first 40):";
    for (const auto& s : a.skipRanges) os << ' ' << s.first << '-' << s.second;
    os << '\n';
    for (const auto& t : a.targets) {
      os << "  target " << t.n << " intended=" << t.intended << " hover=" << t.hover << " recipient=" << t.recipient << " class=" << t.klass << " probes=" << t.probes << " stock=" << t.stockBefore << "->" << t.stockAfter
         << " decision_tick=" << t.decisionTick << " click_tick=" << t.clickTick << " resolve_tick=" << t.resolveTick << " why=" << t.why << '\n';
      for (const auto& p : t.probeLog) os << "    probe " << p << '\n';
    }
    for (const auto& b : a.blockers)
      os << "  blocker lemming=" << b.idx << " at=" << b.x << '/' << b.y << " ticks_standing=" << ((b.tickEnd >= 0 ? b.tickEnd : a.lastTick) - b.tick0) << " turned=" << b.turned.size()
         << " turns_per_tick=" << b.turnsPerTick << " turns_gap=" << b.turnsGap << " pass_per_tick=" << b.passPerTick << " sign_change_gap_unresolved=" << b.passGap << '\n';
    os << "  end_actors=" << a.endActors << '\n';
  }
}

// ---- identity / saves ------------------------------------------------------------------------------------
void writeIdentity(const fs::path& file, const fs::path& game, const std::string& extra) {
  std::ofstream os(file);
  os << "driver=test_targetedplay (scripted clone observation)\n" << extra << "built=" << __DATE__ << ' ' << __TIME__ << "\n";
#ifdef __VERSION__
  os << "compiler=" << __VERSION__ << "\n";
#endif
  os << "sdl_compiled=" << SDL_MAJOR_VERSION << '.' << SDL_MINOR_VERSION << '.' << SDL_PATCHLEVEL << "\ngame_dir=" << game.string() << "\n";
  const char* files[] = {"src/app/main.cpp", "src/app/AppEntry.h", "src/app/RuntimePick.cpp", "src/app/RuntimeHud.cpp", "src/sim/World.cpp", "src/sim/RetailLevel.cpp", "src/save/SaveStore.cpp", "src/__test__/test_targetedplay.cpp"};
  for (const char* f : files) os << "fnv1a64 " << f << " = " << fnvHex(fs::path(TP_SOURCE_DIR) / f) << "\n";
  os << "reference exe is never opened or executed by this driver\n";
}
std::string normalSaveState() {
  auto base = save::localAppDataDirectory();
  if (!base) return "unresolved";
  const auto p = save::cloneSavePathUnder(*base);
  std::error_code ec;
  if (!fs::exists(p, ec)) return "absent:" + p.string();
  return "present:" + p.string() + " bytes=" + std::to_string(fs::file_size(p, ec)) + " fnv=" + fnvHex(p);
}
std::shared_ptr<save::SaveStore> storeFor(const fs::path& runDir, const fs::path& game) {
  const auto file = save::cloneSavePathUnder(runDir / "save");
  return std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(file, game));
}
int runApp(const std::string& gameDir, app::FrontendWindowObserver fo, app::LevelFrameObserver lo, std::shared_ptr<save::SaveStore> store) {
  std::vector<std::string> args{"targeted-play", "--game-dir", gameDir};
  std::vector<char*> argv;
  for (auto& s : args) argv.push_back(s.data());
  app::ApplicationRunOptions options;  // production wall clock
  options.saveStore = std::move(store);
  return app::runApplication(static_cast<int>(argv.size()), argv.data(), std::move(fo), std::move(lo), options);
}
std::map<std::string, std::string> readKv(const fs::path& p) {
  std::map<std::string, std::string> kv;
  std::ifstream in(p);
  std::string line;
  while (std::getline(in, line)) {
    const auto eq = line.find('=');
    if (eq != std::string::npos && line.compare(0, 8, "attempt ") != 0 && line.compare(0, 2, "  ") != 0) kv.emplace(line.substr(0, eq), line.substr(eq + 1));
  }
  return kv;
}
std::optional<fs::path> currentRunDir(const fs::path& root) {
  std::ifstream in(root / "current-run.txt");
  std::string stamp;
  if (!in || !std::getline(in, stamp) || stamp.empty()) return std::nullopt;
  return root / "runs" / stamp;
}
std::optional<std::pair<int, int>> pixelFor(const AssetLibrary& lib, const Flags& flags, std::size_t node) {
  auto map = app::MapScene::build(lib, flags);
  if (!map) return std::nullopt;
  for (int y = 0; y < 480; ++y)
    for (int x = 0; x < 640; ++x)
      if (auto n = map->pickNode(x, y); n && *n == node) return std::make_pair(x, y);
  return std::nullopt;
}

// ---- modes ---------------------------------------------------------------------------------------------
int prepareMode(const fs::path& root, const fs::path& game) {
  std::error_code ec;
  const auto now = std::chrono::system_clock::now();
  const std::time_t tt = std::chrono::system_clock::to_time_t(now);
  char buf[64];
  std::strftime(buf, sizeof buf, "%Y%m%d-%H%M%S", std::gmtime(&tt));
  const fs::path runDir = root / "runs" / buf;
  fs::create_directories(runDir, ec);
  std::ofstream(root / "current-run.txt") << buf << '\n';
  const fs::path src = fs::path(TP_SOURCE_DIR) / kSourceSaveRel;
  const fs::path dst = save::cloneSavePathUnder(runDir / "save");
  fs::create_directories(dst.parent_path(), ec);
  const bool exists = fs::exists(src, ec);
  const long long bytes = exists ? static_cast<long long>(fs::file_size(src, ec)) : -1;
  const std::string fnv = exists ? fnvHex(src) : "-";
  const bool verified = exists && bytes == kSourceSaveBytes && fnv == kSourceSaveFnv;
  bool copied = false;
  if (verified) copied = fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
  std::ofstream os(runDir / "run-id.txt");
  os << "stamp=" << buf << "\nisolated_save=" << dst.string() << "\nsave_source=" << src.string() << "\nsave_source_exists=" << exists << "\nsave_source_bytes=" << bytes
     << "\nsave_source_fnv=" << fnv << "\nexpected_bytes=" << kSourceSaveBytes << "\nexpected_fnv=" << kSourceSaveFnv
     << "\nexpected_provenance=playable-expansion-001 run 20261103-003501 step-1 earn-node0 (public node-0 win, restart-result.md VERIFIED)\nverified=" << verified << "\ncopied=" << copied
     << "\ncopy_fnv=" << (copied ? fnvHex(dst) : "-") << "\nsource_untouched_fnv_after=" << (exists ? fnvHex(src) : "-") << '\n';
  writeIdentity(runDir / "identity-prepare.txt", game, "mode=prepare\n");
  std::cout << "TargetedPlay prepare: run " << buf << " source save verified=" << verified << " copied=" << copied << '\n';
  return 0;  // a missing/mismatched source is a REPORTED blocker (play steps print BLOCKED), not a driver fault
}

int playMode(const AssetLibrary& lib, int stepNo, const fs::path& root, const std::string& gameDir) {
  const fs::path game(gameDir);
  std::error_code ec;
  const StepDef step = kSteps[stepNo - 1];
  const auto cur = currentRunDir(root);
  if (!cur) { std::cerr << "no current-run.txt: prepare did not run\n"; return 1; }
  const fs::path runDir = *cur;
  const fs::path stepDir = runDir / ("step-" + std::to_string(stepNo) + "-" + step.label);
  fs::remove_all(stepDir, ec);
  fs::create_directories(stepDir, ec);
  writeIdentity(stepDir / "identity.txt", game, "mode=play step=" + std::to_string(stepNo) + " run=" + runDir.filename().string() + "\n");

  Ctx c;
  c.dir = stepDir; c.step = step; c.targetNode = step.node;
  c.trace.open(stepDir / "action-trace.csv");
  c.samples.open(stepDir / "samples.csv");
  c.trace << "wall_s,attempt_slot,frame,sim_tick,kind,detail\n";
  c.samples << "attempt,wall_s,frame,sim_tick,spawned,active,saved,dead,remaining_ticks,end_phase,selected_skill,blocker_stock,ff_latched\n";
  c.nextPlan = Plan{};
  if (step.lever) { c.nextPlan.cap = 0; c.nextPlan.naturalOnly = true; c.nextPlan.reason = "initial: natural flow, no skill, observe lever contact"; }
  else { c.nextPlan.cap = 1; c.nextPlan.reason = "initial: ONE Blocker at a hazard-below drop edge (A1 of three)"; }

  const std::string normalBefore = normalSaveState();
  const auto saveFile = save::cloneSavePathUnder(runDir / "save");
  auto store = storeFor(runDir, game);
  const bool saveExistedAtStart = fs::exists(saveFile, ec);
  const std::string saveFnvAtStart = saveExistedAtStart ? fnvHex(saveFile) : "-";
  c.lastProgressWall = wallSeconds();
  const double startWall = wallSeconds();

  const auto decideTarget = [&](const app::FrontendSession& s) {
    c.statesAtMap = s.mapStates();
    c.flagsAtMap = s.completionFlags();
    auto map = app::MapScene::build(lib, c.flagsAtMap);
    std::ostringstream os;
    os << "step=" << stepNo << " (" << step.label << ")\nflags_at_map=" << arr(c.flagsAtMap) << "\nstates_at_map=" << arr(c.statesAtMap) << '\n';
    os << "selectable nodes (state 2/3):";
    for (std::size_t n = 0; n < 102; ++n) if (c.statesAtMap[n] == 2 || c.statesAtMap[n] == 3) os << ' ' << n << (c.statesAtMap[n] == 2 ? "(done)" : "");
    os << '\n';
    const auto st = c.statesAtMap[c.targetNode];
    if (st != 2 && st != 3) c.blocked = "target node " + std::to_string(c.targetNode) + " is not selectable (map state " + std::to_string(st) + "): prerequisite not earned";
    else if (auto name = map ? map->levelForNode(c.targetNode) : std::nullopt) {
      c.targetLevel = *name;
      if (auto px = pixelFor(lib, c.flagsAtMap, c.targetNode)) { c.clickPixX = px->first; c.clickPixY = px->second; }
      else c.blocked = "no map pixel picks node " + std::to_string(c.targetNode);
    } else c.blocked = "node has no level";
    os << (c.blocked.empty() ? "target: node " + std::to_string(c.targetNode) + " " + c.targetLevel + " at pixel " + std::to_string(c.clickPixX) + "," + std::to_string(c.clickPixY) + "\n" : "BLOCKED: " + c.blocked + "\n");
    std::ofstream(stepDir / "earned-map.txt") << os.str();
    c.line(wallSeconds(), 0, 0, "map-decision", c.blocked.empty() ? "target=" + std::to_string(c.targetNode) + ":" + c.targetLevel : "BLOCKED:" + c.blocked);
  };

  const auto frontend = [&](const app::FrontendSession& s, int frame) {
    ++c.feCalls;
    if (c.lastScreen != static_cast<int>(s.screen())) {
      c.lastScreen = static_cast<int>(s.screen());
      c.lastProgressWall = wallSeconds();
      c.line(wallSeconds(), frame, 0, "frontend-screen", "screen=" + std::to_string(c.lastScreen) + ";counter=" + std::to_string(s.constructionCounter()));
    }
    if (wallSeconds() - c.lastProgressWall > 60.0) { c.anomaly += "frontend stalled >60 s; "; pushQuit(); return; }
    const bool canPush = c.feCalls - c.lastPush >= 5;
    switch (s.screen()) {
      case shell::Screen::Menu:
        if (c.resultsHandledFor > 0 || !c.blocked.empty()) { pushQuit(); break; }
        if (canPush && c.menuClicks < 3) { ++c.menuClicks; c.lastPush = c.feCalls; pushClick(kPlayCurrentX, kPlayCurrentY); c.line(wallSeconds(), frame, 0, "click-play-current-game", "x=320;y=363"); }
        else if (canPush) { c.anomaly += "Play Current Game click did not leave the menu; "; pushQuit(); }
        break;
      case shell::Screen::Map:
        if (c.resultsHandledFor > 0) {
          if (!c.returnedToMap) {
            c.returnedToMap = true;
            c.flagsAfter = s.completionFlags(); c.statesAfter = s.mapStates();
            c.saveCountAtMap = s.saveCount(); c.saveErrorAtMap = s.saveError();
            shot(c, s.composeFrame(), "map-after-results.png");
            c.line(wallSeconds(), frame, 0, "map-after-results", "saveCount=" + std::to_string(s.saveCount()) + ";flag_node=" + std::to_string(s.completionFlags()[c.targetNode]));
          }
          pushQuit();
          break;
        }
        if (!c.mapSeen) { c.mapSeen = true; shot(c, s.composeFrame(), "map-before.png"); decideTarget(s); }
        if (!c.blocked.empty()) { pushQuit(); break; }
        if (canPush && c.mapClicks < 3) { ++c.mapClicks; c.lastPush = c.feCalls; pushClick(c.clickPixX, c.clickPixY); c.line(wallSeconds(), frame, 0, "click-map-node", "node=" + std::to_string(c.targetNode) + ";x=" + std::to_string(c.clickPixX) + ";y=" + std::to_string(c.clickPixY)); }
        else if (canPush) { c.anomaly += "map click did not select a level; "; pushQuit(); }
        break;
      case shell::Screen::Results: {
        const int attemptNo = static_cast<int>(c.results.size());
        if (c.resultsHandledFor < attemptNo + 1 && c.attemptOpen) {
          AttemptResult r;
          r.a = c.att; r.wallEnd = wallSeconds(); r.resultsSeen = true;
          if (const auto& o = s.outcome()) { r.success = o->success; r.saved = o->saved; r.totalOut = o->total; r.required = o->required; r.secondsRemaining = o->secondsRemaining; }
          c.results.push_back(r);
          c.attemptOpen = false;
          c.resultsHandledFor = static_cast<int>(c.results.size());
          shot(c, s.composeFrame(), "attempt" + std::to_string(r.a.number) + "-results-screen.png");
          const std::string cls = classify(r);
          c.line(wallSeconds(), frame, 0, "results-screen", cls + ";saved=" + std::to_string(r.saved) + "/" + std::to_string(r.totalOut) + ";required=" + std::to_string(r.required) + ";saveCount=" + std::to_string(s.saveCount()) + ";saveError=" + (s.saveError() ? *s.saveError() : "-"));
          c.lastPush = -100; c.contClicks = 0;
          if (cls == "WIN") c.resultAction = 1;
          else if (r.a.number < step.maxAttempts) {
            c.resultAction = 2;
            c.nextAttempt = r.a.number + 1;
            Plan p;
            std::ostringstream why;
            why << "attempt " << r.a.number << " " << cls << ": saved " << r.a.lastSaved << "/" << r.a.need << " dead=" << r.a.lastDead << " active_at_end=" << r.a.lastActive
                << " confirmed_assignments=";
            int conf = 0;
            for (const auto& t : r.a.targets) if (t.klass.rfind("CONFIRMED", 0) == 0) ++conf;
            why << conf;
            if (step.lever) {
              p.leverMode = true; p.cap = 1;
              why << " lever_toggles=" << r.a.leverToggles << ": natural flow did not toggle a lever -> next: ONE Blocker 6-16 cells left of a lever so a left-walker passes it facing -1, turns, and passes it again facing +1";
            } else {
              // evidence-driven: bands where a previous Blocker stood while walkers were stranded beside it are forbidden;
              // bands where walkers died become preferred; more than 0 deaths allows a 2nd Blocker (on a different band only).
              for (const auto& kv : r.a.strandBandCount) if (kv.second >= 3) p.forbid.insert(kv.first);
              std::set<int> deathBands;
              for (const auto& d : r.a.deaths) deathBands.insert(bandOf(d.y));
              for (int b : deathBands) if (!p.forbid.count(b)) p.prefer.insert(b);
              p.cap = r.a.deaths.empty() ? 1 : 2;
              why << " forbid_bands(stranded>=3)=";
              for (int b : p.forbid) why << b << '/';
              why << " prefer_bands(death sites)=";
              for (int b : p.prefer) why << b << '/';
            }
            p.reason = why.str();
            c.nextPlan = p;
            c.line(wallSeconds(), frame, 0, "revision", p.reason);
          } else c.resultAction = 3;
        }
        if (c.resultAction == 3) { pushQuit(); break; }
        if (c.feCalls - c.lastPush >= 5) {
          if (c.contClicks < 3) { ++c.contClicks; c.lastPush = c.feCalls; pushClick(kContinueX, kContinueY); c.line(wallSeconds(), frame, 0, c.resultAction == 1 ? "click-continue" : "click-retry", "x=320;y=360"); }
          else { c.anomaly += "results click did not leave the results screen; "; pushQuit(); }
        }
        break;
      }
      default: c.anomaly += "unexpected frontend screen; "; pushQuit(); break;
    }
  };
  // A level leg is progress (it has its own watchdog); without this the 60 s frontend stall guard fires on return from a long leg.
  const auto level = [&](const app::LevelFrameView& f) { c.lastProgressWall = wallSeconds(); levelObserver(c, f); };

  std::cout << "TargetedPlay step " << stepNo << " (" << step.label << ") run " << runDir.filename().string() << " save " << saveFile.string() << '\n';
  const int status = runApp(gameDir, frontend, level, store);
  const double totalWall = wallSeconds() - startWall;
  const std::string normalAfter = normalSaveState();
  if (c.attemptOpen) { AttemptResult r; r.a = c.att; r.wallEnd = wallSeconds(); c.results.push_back(r); c.attemptOpen = false; }
  bool won = false;
  for (const auto& r : c.results) if (classify(r) == "WIN") won = true;
  std::string outcome;
  if (!c.blocked.empty()) outcome = "BLOCKED (prerequisite not selectable)";
  else if (!c.levelEntered) outcome = "BLOCKED (frontend never entered the level)";
  else if (won) outcome = "WIN";
  else if (!c.results.empty() && c.results.back().a.watchdog) outcome = "INCOMPLETE (watchdog)";
  else outcome = "LOSS";
  const bool saveExists = fs::exists(saveFile, ec);
  std::ofstream res(stepDir / "play-result.txt");
  res << "step=" << stepNo << "\nlabel=" << step.label << "\nrun=" << runDir.filename().string() << "\nlevel=" << c.targetLevel << "\nnode=" << c.targetNode << "\noutcome=" << outcome
      << "\nwon=" << (won ? 1 : 0) << "\nblocked=" << (c.blocked.empty() ? "-" : c.blocked) << "\nrunApplication_status=" << status << "\nentered_level=" << c.enteredLevel
      << "\ntotal_wall_s=" << fmt(totalWall, 1) << "\nsave_file=" << saveFile.string() << "\nsave_existed_at_start=" << saveExistedAtStart << "\nsave_fnv_at_start=" << saveFnvAtStart
      << "\nsave_exists=" << saveExists << "\nsave_bytes=" << (saveExists ? static_cast<long long>(fs::file_size(saveFile, ec)) : -1) << "\nsave_fnv=" << (saveExists ? fnvHex(saveFile) : "-")
      << "\nsave_count_at_map=" << c.saveCountAtMap << "\nsave_error_at_map=" << (c.saveErrorAtMap ? *c.saveErrorAtMap : "-") << "\nreturned_to_map=" << (c.returnedToMap ? 1 : 0)
      << "\nflags_at_map_before=" << arr(c.flagsAtMap) << "\nstates_at_map_before=" << arr(c.statesAtMap) << "\nflags_after=" << arr(c.flagsAfter) << "\nstates_after=" << arr(c.statesAfter)
      << "\nnormal_save_before=" << normalBefore << "\nnormal_save_after=" << normalAfter << "\nnormal_save_unchanged=" << (normalBefore == normalAfter ? 1 : 0)
      << "\nanomaly=" << (c.anomaly.empty() ? "-" : c.anomaly) << '\n';
  writeAttempts(c, res);
  std::ofstream md(stepDir / "result.md");
  md << "# Step " << stepNo << " " << step.label << ": " << c.targetLevel << " (node " << c.targetNode << ")\n\nOutcome: **" << outcome << "**\n\n```\n";
  writeAttempts(c, md);
  md << "```\n";
  std::cout << "TargetedPlay step " << stepNo << " " << c.targetLevel << " (node " << c.targetNode << "): " << outcome << " wall=" << fmt(totalWall, 1) << "s attempts=" << c.results.size()
            << " status=" << status << " anomaly=" << (c.anomaly.empty() ? "-" : c.anomaly) << (c.blocked.empty() ? "" : " blocked=" + c.blocked) << '\n';
  writeAttempts(c, std::cout);
  bool integrity = status == 0 && res && md && c.trace && c.samples;
  if (c.blocked.empty() && c.levelEntered) integrity = integrity && !c.results.empty() && c.enteredLevel == c.targetLevel;
  if (!integrity) std::cerr << "FAIL: driver integrity (status=" << status << ", entered=" << c.enteredLevel << ")\n";
  return integrity ? 0 : 1;
}

int restartMode(int stepNo, const fs::path& root, const std::string& gameDir) {
  const fs::path game(gameDir);
  const auto cur = currentRunDir(root);
  if (!cur) { std::cerr << "no current-run.txt\n"; return 1; }
  const StepDef step = kSteps[stepNo - 1];
  const fs::path stepDir = *cur / ("step-" + std::to_string(stepNo) + "-" + step.label);
  std::ofstream md(stepDir / "restart-result.md");
  const auto kv = readKv(stepDir / "play-result.txt");
  if (kv.empty() || !kv.count("won") || kv.at("won") != "1") {
    md << "# Step " << stepNo << " restart: UNVERIFIED\n\nNo genuine completion was observed by the play invocation (outcome=" << (kv.count("outcome") ? kv.at("outcome") : "no play result") << "); persistence not inspected.\n";
    std::cout << "TargetedPlay restart step " << stepNo << ": UNVERIFIED (no genuine completion)\n";
    return md ? 0 : 1;
  }
  const std::size_t node = static_cast<std::size_t>(std::stoul(kv.at("node")));
  const std::string saveFile = kv.at("save_file");
  struct Seen { int calls = 0; bool map = false; Flags flags{}, states{}; std::optional<std::string> loadError; int saveCount = -1; int clicks = 0; int lastPush = -100; } seen;
  const auto frontend = [&](const app::FrontendSession& s, int) {
    ++seen.calls;
    if (seen.calls > 600) { pushQuit(); return; }
    if (s.screen() == shell::Screen::Menu) {
      if (seen.calls - seen.lastPush >= 5 && seen.clicks < 3) { ++seen.clicks; seen.lastPush = seen.calls; pushClick(kPlayCurrentX, kPlayCurrentY); seen.loadError = s.saveLoadError(); }
      else if (seen.clicks >= 3) pushQuit();
    } else if (s.screen() == shell::Screen::Map) {
      if (!seen.map) {
        seen.map = true; seen.flags = s.completionFlags(); seen.states = s.mapStates(); seen.saveCount = s.saveCount(); seen.loadError = s.saveLoadError();
        if (auto r = writePng(stepDir / "restart-map.png", s.composeFrame()); !r) std::cerr << r.error() << '\n';
      }
      pushQuit();
    } else pushQuit();
  };
  const std::string before = fnvHex(saveFile);
  const int status = runApp(gameDir, frontend, {}, storeFor(*cur, game));
  const std::string after = fnvHex(saveFile);
  const bool flagSet = node < 102 && seen.flags[node] == 1;
  const bool matches = arr(seen.flags) == kv.at("flags_after") && arr(seen.states) == kv.at("states_after");
  std::error_code ec;
  const bool ok = status == 0 && seen.map && flagSet && matches && before == after;
  md << "# Step " << stepNo << " restart (fresh process, same isolated save)\n\nrunApplication status=" << status << " reachedMap=" << seen.map << " loadError=" << (seen.loadError ? *seen.loadError : "-")
     << "\nnode " << node << " completion flag after restart = " << (node < 102 ? seen.flags[node] : -1) << "\nflags=" << arr(seen.flags) << "\nstates=" << arr(seen.states)
     << "\nflags/states equal the play invocation's map: " << matches << "\nsave file bytes=" << (fs::exists(saveFile, ec) ? static_cast<long long>(fs::file_size(saveFile, ec)) : -1)
     << " fnv before=" << before << " after=" << after << " (unchanged: " << (before == after) << ")\nRESULT: " << (ok ? "VERIFIED" : "FAILED") << "\n";
  std::cout << "TargetedPlay restart step " << stepNo << ": " << (ok ? "VERIFIED" : "FAILED (persistence finding)") << " flag=" << (node < 102 ? seen.flags[node] : -1) << '\n';
  return status == 0 && md ? 0 : 1;
}

}  // namespace
#endif

int main(int argc, char** argv) {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "TargetedPlay: requires SDL2\n";
  return 1;
#else
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir) { std::cout << "TargetedPlay: SKIPPED (set WEBOLUTION_GAME_DIR)\n"; return 0; }
  const char* video = std::getenv("SDL_VIDEODRIVER");
  if (!video || std::string(video) != "dummy") { std::cerr << "TargetedPlay: refusing to run without SDL_VIDEODRIVER=dummy\n"; return 1; }
  if (argc < 4) { std::cerr << "usage: test_targetedplay prepare|play|restart <step> <output-dir>\n"; return 1; }
  const std::string mode = argv[1];
  const int step = std::atoi(argv[2]);
  const fs::path root(argv[3]);
  std::error_code ec;
  fs::create_directories(root, ec);
  if (mode == "prepare") return prepareMode(root, fs::path(gameDir));
  auto library = AssetLibrary::open(fs::path(gameDir) / "lemmings.box", fs::path(gameDir) / "patch.box");
  if (!library) { std::cerr << "assets: " << library.error() << '\n'; return 1; }
  if (step < 1 || step > 2) { std::cerr << "step must be 1..2\n"; return 1; }
  if (mode == "play") return playMode(*library, step, root, gameDir);
  if (mode == "restart") return restartMode(step, root, gameDir);
  std::cerr << "unknown mode\n";
  return 1;
#endif
}
