// playable-expansion-001 assessment driver (SCRIPTED CLONE OBSERVATION, not human play, not original fidelity).
//
// One executable, several CTest registrations (see CMakeLists.txt):
//   survey 0 <dir>   read-only: map link/unlock graph, shortest earned routes, terrain dumps of the core levels
//   play S <dir>     step S through the PUBLIC webo::app::runApplication (bare ordinary startup, SDL events queued
//                    from the read-only FrontendWindowObserver/LevelFrameObserver, production wall clock, isolated
//                    FileSaveSink). All steps of one verifier run share <dir>/runs/<stamp>/save:
//                      1 earn node 0 (MM_CLIMB_002, Climber) -- the genuine prerequisite win that unlocks nodes 3 and 4
//                      2 core: MM_BLOCK_001_A (node 3), Blocker policy, at most two attempts
//                      3 core: MM_BUILD       (node 4), Builder policy, at most two attempts
//                      4 gadget: the nearest genuinely selectable lever/teleport/speed-pad level on the earned map
//                        (node 8 MM_W_012 is a candidate, not assumed), one attempt, Blocker policy
//   restart S <dir>  a second OS process: fresh runApplication over the same isolated save; UNVERIFIED unless the
//                    play invocation won.
//
// Nothing here mutates a World, actor, terrain, stock, flag, progress or clock. Inputs are SDL events only.
// The fast-forward plate is clicked through the public input path and recorded in the trace.
// Outcomes (win/loss/blocked/watchdog) are REPORTED, never asserted: the exit code reflects driver integrity only.
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
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifndef PE_SOURCE_DIR
#define PE_SOURCE_DIR "."
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
using Flags = std::array<std::int32_t, 102>;

std::string arr(const Flags& a) {
  std::ostringstream os;
  for (std::size_t i = 0; i < a.size(); ++i) os << (i ? "," : "") << a[i];
  return os.str();
}

// ---- decoded level facts (read-only) -------------------------------------------------------------
struct LevelFacts {
  std::string level;
  std::size_t node = 0;
  bool ok = false;
  std::string error;
  int total = 0, need = 0, levers = 0, teleports = 0, pads = 0;
  long hazardCells = 0;
  std::array<int, 8> skills{};
  std::string summary;
};

LevelFacts inspect(const AssetLibrary& lib, std::size_t node, const std::string& level) {
  LevelFacts f;
  f.node = node;
  f.level = level;
  auto setup = sim::loadRetailLevel(lib, level);
  if (!setup) { f.error = setup.error(); return f; }
  const auto& t = setup->terrain;
  for (int y = 0; y < t.height(); ++y)
    for (int x = 0; x < t.width(); ++x)
      if (t.waterHazard(x, y) || t.zoneWater(x, y) || t.zoneAcid(x, y)) ++f.hazardCells;
  f.ok = true;
  f.total = setup->totalLemmings;
  f.need = setup->saveTarget;
  f.skills = setup->skills;
  f.levers = static_cast<int>(setup->levers.size());
  f.teleports = static_cast<int>(setup->teleports.size());
  f.pads = static_cast<int>(setup->speedPads.size());
  std::ostringstream os;
  os << level << " node=" << node << " lemmings=" << f.total << " need=" << f.need
     << " time=" << (setup->timeLimitTicks / 5) << "s hatches=" << setup->entrances.size()
     << " exits=" << setup->exits.size() << " levers=" << f.levers << " teleports=" << f.teleports
     << " speedPads=" << f.pads << " hazardCells=" << f.hazardCells << " stock=[";
  for (int i = 0; i < 8; ++i) os << (i ? "," : "") << kSkillNames[i] << ":" << setup->skills[static_cast<std::size_t>(i)];
  os << "]";
  f.summary = os.str();
  return f;
}

bool isGadget(const LevelFacts& f) { return f.ok && (f.levers > 0 || f.teleports > 0 || f.pads > 0); }

// ---- map unlock graph (read-only; MapScene::build with hypothetical in-memory flags) ----------------
struct MapGraph {
  std::vector<std::vector<int>> links;  // links[i]: nodes that become selectable (state 3) when node i is completed
  std::array<std::int32_t, 102> baseStates{};
};

MapGraph computeLinks(const AssetLibrary& lib) {
  MapGraph g;
  g.links.assign(102, {});
  auto base = app::MapScene::build(lib, {});
  if (!base) return g;
  g.baseStates = base->states();
  for (std::size_t i = 0; i < 102; ++i) {
    Flags fl{};
    fl[i] = 1;
    auto m = app::MapScene::build(lib, fl);
    if (!m) continue;
    for (std::size_t n = 0; n < 102; ++n)
      if (n != i && g.baseStates[n] == 1 && m->states()[n] == 3) g.links[i].push_back(static_cast<int>(n));
  }
  return g;
}

// Minimum number of genuine completions needed before `node` is selectable, from a given set of selectable nodes.
std::vector<int> winsNeeded(const MapGraph& g, const Flags& states) {
  std::vector<int> dist(102, -1);
  std::queue<int> q;
  for (int i = 0; i < 102; ++i)
    if (states[static_cast<std::size_t>(i)] == 2 || states[static_cast<std::size_t>(i)] == 3) { dist[static_cast<std::size_t>(i)] = 0; q.push(i); }
  while (!q.empty()) {
    const int n = q.front();
    q.pop();
    for (int m : g.links[static_cast<std::size_t>(n)])
      if (dist[static_cast<std::size_t>(m)] < 0) { dist[static_cast<std::size_t>(m)] = dist[static_cast<std::size_t>(n)] + 1; q.push(m); }
  }
  return dist;
}

std::string chainTo(const MapGraph& g, const Flags& states, int node) {
  // parent search again (BFS with parents) to print the exact completion chain
  std::vector<int> parent(102, -2);
  std::queue<int> q;
  for (int i = 0; i < 102; ++i)
    if (states[static_cast<std::size_t>(i)] == 2 || states[static_cast<std::size_t>(i)] == 3) { parent[static_cast<std::size_t>(i)] = -1; q.push(i); }
  while (!q.empty()) {
    const int n = q.front();
    q.pop();
    for (int m : g.links[static_cast<std::size_t>(n)])
      if (parent[static_cast<std::size_t>(m)] == -2) { parent[static_cast<std::size_t>(m)] = n; q.push(m); }
  }
  if (node < 0 || parent[static_cast<std::size_t>(node)] == -2) return "unreachable";
  std::vector<int> path;
  for (int n = node; n >= 0; n = parent[static_cast<std::size_t>(n)]) path.push_back(n);
  std::reverse(path.begin(), path.end());
  std::string s;
  for (std::size_t i = 0; i < path.size(); ++i) s += (i ? " -> " : "") + std::to_string(path[i]);
  return s;
}

// ---- terrain helpers (read-only) ------------------------------------------------------------------
int wrapX(const sim::Terrain& t, int x) {
  const int w = std::max(1, t.width());
  return ((x % w) + w) % w;
}
bool solidWrap(const sim::Terrain& t, int x, int y) {
  if (y < 0 || y >= t.height() || t.width() <= 0) return false;
  return t.solid(wrapX(t, x), y);
}
int wrapDiff(const sim::Terrain& t, int a, int b) {  // signed a-b on the cylinder
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

bool dropAhead(const sim::Lemming& l, const sim::Terrain& t) {
  const int dir = static_cast<int>(l.facing);
  for (int k = 1; k <= 2; ++k) {
    bool ground = false;
    for (int dy = 2; dy <= 9 && !ground; ++dy) ground = solidWrap(t, l.x + dir * k, l.y + dy);
    if (!ground) return true;
  }
  return false;
}

// First dy >= -3 with a solid cell at (x, y+dy); 99 if none within 80 rows.
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
// Nearest column ahead (1..maxK) whose floor is at least minDepth below this lemming's own floor. Stops at a wall.
Edge edgeAhead(const sim::Lemming& l, const sim::Terrain& t, int maxK, int minDepth) {
  Edge e;
  const int dir = static_cast<int>(l.facing);
  const int d0 = floorDepth(t, l.x, l.y);
  for (int k = 1; k <= maxK; ++k) {
    const int d = floorDepth(t, l.x + dir * k, l.y);
    if (d < d0 - 2) break;  // a wall rises ahead
    if (d - d0 >= minDepth) {
      e.found = true;
      e.k = k;
      e.depth = d - d0;
      e.hazard = hazardColumn(t, l.x + dir * k, l.y);
      return e;
    }
  }
  return e;
}
// Column offset (> k) of ground at roughly this lemming's height on the far side of a gap, or 0.
int farSide(const sim::Lemming& l, const sim::Terrain& t, int k, int maxJ) {
  const int dir = static_cast<int>(l.facing);
  const int d0 = floorDepth(t, l.x, l.y);
  for (int j = k + 2; j <= k + maxJ; ++j) {
    const int d = floorDepth(t, l.x + dir * j, l.y);
    if (d - d0 >= -8 && d - d0 <= 8) return j;
  }
  return 0;
}

// ---- attempt records -----------------------------------------------------------------------------
struct BlockerTrack {
  std::size_t idx = 0;
  int x = 0, y = 0;
  long tick0 = 0;
  long tickEnd = -1;
  int frameSeen = 0;
  bool ended = false;
  std::set<std::size_t> turned, passed;
  std::size_t fieldCells = 0;
};
struct BuilderTrack {
  std::size_t idx = 0;
  int x0 = 0, y0 = 0, dir = 1;
  long tick0 = 0;
  long tickEnd = -1;
  int frames = 0;
  bool done = false;
  int bricksFirst = -1, bricksMin = 999;
  int xEnd = 0, yEnd = 0;
  int newCells = -1;
  int wx0 = 0, wy0 = 0, ww = 0, wh = 0;
  std::vector<std::uint8_t> snap;
  std::set<std::size_t> crossers;
};
struct Transition { int frame = -1; std::size_t idx = 0; int skill = -1; };

struct Attempt {
  int number = 1;
  int revMode = 0;  // 0 base policy, 1 relaxed (nothing placed last time), 2 more (placed but not enough)
  double wall0 = 0.0, watchdogSeconds = 0.0;
  int frames = 0;
  bool ffClicked = false, keyPreselected = false;
  int ffFrame = -1;
  long ffTick = -1;
  int lastActionFrame = -100, lastPlaceFrame = -1000;
  struct Pending { int frame; std::size_t lemming; int skill; int stockBefore; std::string why; } pendingData{};
  bool pending = false;
  std::map<std::pair<std::size_t, int>, int> tries;
  std::map<std::size_t, int> cooldownUntil;
  std::vector<std::pair<int, int>> sites;  // assigned positions of the policy skill
  int placed = 0;
  int clicks = 0, assignOk = 0, assignFail = 0, pickedIntended = 0, pickedOther = 0, pickedNone = 0;
  bool endingShot = false, watchdog = false, quitSent = false, tracksFinal = false;
  double lastSampleWall = -100.0;
  long lastTick = 0;
  int lastSpawned = 0, lastSaved = 0, lastDead = 0, lastActive = 0;
  long lastRemainingTicks = 0;
  std::array<int, 8> stockStart{}, stockEnd{};
  int total = 0, need = 0;
  long limitTicks = 0;
  double endObservedWall = -1.0;
  // read-only observation memory
  std::vector<sim::State> prevState;
  std::vector<sim::Facing> prevFacing;
  std::vector<int> prevX;
  std::vector<int> prevFaster;
  std::vector<int> leverState;
  Transition lastTransition;
  std::vector<BlockerTrack> blockers;
  std::vector<BuilderTrack> builders;
  int leverToggles = 0, speedBoosts = 0, jumps = 0, deathsLogged = 0;
};

struct AttemptResult {
  Attempt a;
  bool resultsSeen = false;
  bool success = false;
  int saved = 0, totalOut = 0, required = 0, secondsRemaining = 0;
  double wallEnd = 0.0;
};

struct StepDef { int step; const char* label; int policy; int maxAttempts; std::size_t node; };
// policy: 0 earn (baseline reactive Climber), 1 Blocker, 2 Builder, 3 gadget level (Blocker policy + lever watch)
constexpr StepDef kSteps[4] = {
    {1, "earn-node0", 0, 2, 0}, {2, "core-blocker", 1, 2, 3}, {3, "core-builder", 2, 2, 4}, {4, "gadget", 3, 1, 8}};

struct Ctx {
  fs::path dir;
  StepDef step{};
  std::ofstream trace, samples;
  int nextAttempt = 1;
  int nextRev = 0;
  std::string nextRevReason = "-";
  Attempt att;
  std::vector<AttemptResult> results;
  bool attemptOpen = false;
  bool levelEntered = false;
  std::string enteredLevel;
  std::optional<int> enteredCounter;
  std::string anomaly;
  std::size_t targetNode = 0;
  std::string targetLevel;
  std::string blocked;  // prerequisite failure text, if the target could not be selected
  int feCalls = 0, menuClicks = 0, mapClicks = 0, contClicks = 0, lastPush = -100;
  double lastProgressWall = 0.0;
  int resultsHandledFor = 0;
  int resultAction = 0;  // 0 none, 1 Continue (win), 2 Retry (first loss), 3 quit (final loss)
  int lastScreen = -1;
  bool mapSeen = false, returnedToMap = false;
  int clickPixX = -1, clickPixY = -1;
  Flags flagsAtMap{}, statesAtMap{}, flagsAfter{}, statesAfter{};
  int saveCountAtMap = -1;
  std::optional<std::string> saveErrorAtMap;
  void line(double w, int frame, long tick, const std::string& kind, const std::string& detail) {
    trace << fmt(w, 3) << ',' << nextAttempt << ',' << frame << ',' << tick << ',' << kind << ',' << detail << '\n';
    trace.flush();
  }
};

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

int policySkill(int policy) { return policy == 2 ? 4 : (policy == 1 || policy == 3) ? 3 : 0; }

// ---- policies (read World/terrain only) -------------------------------------------------------------
struct Intent { int skill = -1; std::string why; };

// Strategy for the earn step: the accepted baseline reactive Climber policy (strategy 1) / blanket (strategy 2).
Intent earnIntent(const Attempt& a, const sim::Lemming& l, const sim::Terrain& t, const std::array<int, 8>& stock) {
  Intent in;
  if (l.state != sim::State::Walker) return in;
  const bool wall = wallAhead(l, t, 3);
  const bool blanket = a.number >= 2;
  if (blanket) {
    if (!l.canClimb && stock[0] > 0) { in.skill = 0; in.why = "blanket climber"; return in; }
    return in;
  }
  if (wall && !l.canClimb && stock[0] > 0) { in.skill = 0; in.why = "wall ahead >=3 high"; }
  return in;
}

// Blocker: stand a few cells before a real drop edge so followers turn instead of walking off.
Intent blockerIntent(const Attempt& a, const sim::Lemming& l, const sim::Terrain& t, const std::array<int, 8>& stock) {
  Intent in;
  if (stock[3] <= 0 || l.state != sim::State::Walker) return in;
  const int cap = a.revMode == 2 ? 8 : a.revMode == 1 ? 6 : 4;
  if (a.placed >= cap) return in;
  const int maxK = a.revMode == 1 ? 10 : 8;
  const int minK = a.revMode == 1 ? 2 : 4;
  const int minDepth = a.revMode == 1 ? 8 : 12;
  const Edge e = edgeAhead(l, t, maxK, minDepth);
  if (!e.found || e.k < minK) return in;
  if (a.revMode != 1 && !(e.hazard || e.depth >= 24)) return in;
  if (wallAhead(l, t, 3)) return in;
  const int spacing = a.revMode == 2 ? 14 : 24;
  for (const auto& s : a.sites)
    if (std::abs(wrapDiff(t, s.first, l.x)) < spacing && std::abs(s.second - l.y) <= 10) return in;
  in.skill = 3;
  std::ostringstream os;
  os << "drop edge k=" << e.k << " depth=" << e.depth << " hazardBelow=" << e.hazard;
  in.why = os.str();
  return in;
}

// Builder: bridge a gap that has ground at a similar height on the far side, or stair over a wall.
Intent builderIntent(const Attempt& a, const sim::Lemming& l, const sim::Terrain& t, const std::array<int, 8>& stock) {
  Intent in;
  if (stock[4] <= 0 || l.state != sim::State::Walker) return in;
  const int cap = a.revMode == 2 ? 9 : 6;
  if (a.placed >= cap) return in;
  const int minGapFrames = a.revMode == 1 ? 10 : 25;
  if (a.lastPlaceFrame >= 0 && a.frames - a.lastPlaceFrame < minGapFrames) return in;
  for (const auto& b : a.builders)  // one builder at a time near this site
    if (!b.done && std::abs(wrapDiff(t, b.x0, l.x)) < 20 && std::abs(b.y0 - l.y) <= 12) return in;
  const int maxK = a.revMode == 1 ? 5 : 3;
  const Edge e = edgeAhead(l, t, maxK, a.revMode == 1 ? 8 : 12);
  if (e.found) {
    const int j = farSide(l, t, e.k, a.revMode == 1 ? 80 : 50);
    if (j > 0) {
      std::ostringstream os;
      os << "gap edge k=" << e.k << " depth=" << e.depth << " far-side ground at +" << j;
      in.skill = 4;
      in.why = os.str();
      return in;
    }
    return in;
  }
  if (wallAhead(l, t, a.revMode == 1 ? 3 : 2)) {
    in.skill = 4;
    in.why = "wall ahead (stair over)";
  }
  return in;
}

Intent chooseIntent(const Ctx& c, const Attempt& a, const sim::Lemming& l, const sim::Terrain& t,
                    const std::array<int, 8>& stock) {
  switch (c.step.policy) {
    case 0: return earnIntent(a, l, t, stock);
    case 2: return builderIntent(a, l, t, stock);
    default: return blockerIntent(a, l, t, stock);
  }
}

// ---- read-only observation tracking -----------------------------------------------------------------
void finalizeBuilder(Ctx& c, Attempt& a, BuilderTrack& b, const sim::World& w, long tick, int frame, const char* why) {
  if (b.newCells >= 0) return;
  int n = 0;
  const auto& t = w.terrain();
  for (int yy = 0; yy < b.wh; ++yy)
    for (int xx = 0; xx < b.ww; ++xx) {
      const bool now = solidWrap(t, b.wx0 + xx, b.wy0 + yy);
      const bool was = b.snap[static_cast<std::size_t>(yy * b.ww + xx)] != 0;
      if (now && !was) ++n;
    }
  b.newCells = n;
  std::ostringstream os;
  os << "lemming=" << b.idx << ";why=" << why << ";builder_frames=" << b.frames << ";bricks_first=" << b.bricksFirst
     << ";bricks_min=" << (b.bricksMin == 999 ? -1 : b.bricksMin) << ";start=" << b.x0 << '/' << b.y0
     << ";end=" << b.xEnd << '/' << b.yEnd << ";dir=" << b.dir << ";new_solid_cells=" << n
     << ";others_crossed_past_end=" << b.crossers.size();
  c.line(wallSeconds(), frame, tick, "builder-summary", os.str());
  (void)a;
}

void finalizeBlocker(Ctx& c, BlockerTrack& b, long tick, int frame, const char* why) {
  std::ostringstream os;
  os << "lemming=" << b.idx << ";why=" << why << ";at=" << b.x << '/' << b.y << ";ticks_standing="
     << ((b.tickEnd >= 0 ? b.tickEnd : tick) - b.tick0) << ";turned_near=" << b.turned.size()
     << ";passed_through=" << b.passed.size() << ";field_cells_seen=" << b.fieldCells;
  c.line(wallSeconds(), frame, tick, "blocker-summary", os.str());
}

void trackFrame(Ctx& c, Attempt& a, const app::LevelFrameView& f) {
  const auto& world = f.world;
  const auto& lemmings = world.lemmings();
  const auto& t = world.terrain();
  const long tick = static_cast<long>(world.tickCount());
  if (a.prevState.size() != lemmings.size()) {
    a.prevState.assign(lemmings.size(), sim::State::Faller);
    a.prevFacing.assign(lemmings.size(), sim::Facing::Right);
    a.prevX.assign(lemmings.size(), 0);
    a.prevFaster.assign(lemmings.size(), 0);
    for (std::size_t i = 0; i < lemmings.size(); ++i) {
      a.prevState[i] = lemmings[i].state;
      a.prevFacing[i] = lemmings[i].facing;
      a.prevX[i] = lemmings[i].x;
    }
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
      os << "lever=" << i << ";state=" << a.leverState[i] << "->" << levers[i].state << ";at=" << levers[i].x << '/'
         << levers[i].y << ";lemmings_in_aabb=";
      for (std::size_t j = 0; j < lemmings.size(); ++j) {
        const auto& l = lemmings[j];
        if (!l.alive()) continue;
        if (std::abs(wrapDiff(t, l.x, levers[i].x)) <= levers[i].width + 3 && std::abs(l.y - levers[i].y) <= levers[i].height + 3)
          os << j << '(' << sim::stateName(l.state) << ",face=" << static_cast<int>(l.facing) << ")";
      }
      c.line(wallSeconds(), f.frame, tick, "lever-toggle", os.str());
      a.leverState[i] = levers[i].state;
    }

  for (std::size_t i = 0; i < lemmings.size(); ++i) {
    const auto& l = lemmings[i];
    const sim::State ps = a.prevState[i];
    if (l.state != ps) {
      if (l.state == sim::State::Blocker) {
        BlockerTrack b;
        b.idx = i; b.x = l.x; b.y = l.y; b.tick0 = tick; b.frameSeen = f.frame;
        a.blockers.push_back(b);
        a.lastTransition = {f.frame, i, 3};
        std::ostringstream os;
        os << "lemming=" << i << ";from=" << sim::stateName(ps) << ";pos=" << l.x << '/' << l.y << ";face=" << static_cast<int>(l.facing);
        c.line(wallSeconds(), f.frame, tick, "state-blocker", os.str());
      } else if (l.state == sim::State::Builder) {
        BuilderTrack b;
        b.idx = i; b.x0 = l.x; b.y0 = l.y; b.dir = static_cast<int>(l.facing); b.tick0 = tick;
        b.bricksFirst = l.bricksLeft;
        b.wx0 = l.x - (b.dir > 0 ? 2 : 62);
        b.wy0 = std::max(0, l.y - 30);
        b.ww = 65;
        b.wh = std::min(40, t.height() - b.wy0);
        b.snap.assign(static_cast<std::size_t>(b.ww * b.wh), 0);
        for (int yy = 0; yy < b.wh; ++yy)
          for (int xx = 0; xx < b.ww; ++xx) b.snap[static_cast<std::size_t>(yy * b.ww + xx)] = solidWrap(t, b.wx0 + xx, b.wy0 + yy) ? 1 : 0;
        a.builders.push_back(std::move(b));
        a.lastTransition = {f.frame, i, 4};
        std::ostringstream os;
        os << "lemming=" << i << ";from=" << sim::stateName(ps) << ";pos=" << l.x << '/' << l.y << ";face=" << static_cast<int>(l.facing)
           << ";bricks=" << l.bricksLeft;
        c.line(wallSeconds(), f.frame, tick, "state-builder", os.str());
      }
      if ((l.state == sim::State::Dead || l.state == sim::State::Splatter) && a.deathsLogged < 40) {
        ++a.deathsLogged;
        std::ostringstream os;
        os << "lemming=" << i << ";from=" << sim::stateName(ps) << ";to=" << sim::stateName(l.state) << ";pos=" << l.x << '/' << l.y;
        c.line(wallSeconds(), f.frame, tick, "state-death", os.str());
      }
    }
    if (l.fasterMs > 0 && a.prevFaster[i] == 0) {
      ++a.speedBoosts;
      c.line(wallSeconds(), f.frame, tick, "speedpad-boost", "lemming=" + std::to_string(i) + ";pos=" + std::to_string(l.x) + "/" + std::to_string(l.y));
    }
    if (l.alive() && std::abs(wrapDiff(t, l.x, a.prevX[i])) > 24 && a.jumps < 20) {
      ++a.jumps;
      c.line(wallSeconds(), f.frame, tick, "position-jump", "lemming=" + std::to_string(i) + ";from_x=" + std::to_string(a.prevX[i]) + ";to=" + std::to_string(l.x) + "/" + std::to_string(l.y) + ";state=" + std::string(sim::stateName(l.state)));
    }
  }

  // Blocker effects: facing reversals near a standing blocker, and any lemming crossing it.
  for (auto& b : a.blockers) {
    if (b.ended) continue;
    const bool stillBlocker = b.idx < lemmings.size() && lemmings[b.idx].state == sim::State::Blocker;
    if (!stillBlocker) {
      b.ended = true;
      b.tickEnd = tick;
      finalizeBlocker(c, b, tick, f.frame, "no longer a Blocker");
      continue;
    }
    b.fieldCells = std::max(b.fieldCells, t.blockerFieldCells());
    for (std::size_t j = 0; j < lemmings.size(); ++j) {
      if (j == b.idx || !lemmings[j].alive()) continue;
      const auto& l = lemmings[j];
      const int dx = wrapDiff(t, l.x, b.x);
      if (std::abs(dx) > 14 || std::abs(l.y - b.y) > 8) continue;
      if (l.state == sim::State::Walker && a.prevFacing[j] != l.facing && b.turned.insert(j).second)
        c.line(wallSeconds(), f.frame, tick, "blocker-turn", "blocker=" + std::to_string(b.idx) + ";lemming=" + std::to_string(j) + ";dx=" + std::to_string(dx) + ";face=" + std::to_string(static_cast<int>(a.prevFacing[j])) + "->" + std::to_string(static_cast<int>(l.facing)));
      const int pdx = wrapDiff(t, a.prevX[j], b.x);
      if (l.state == sim::State::Walker && ((pdx < 0 && dx > 0) || (pdx > 0 && dx < 0)) && std::abs(dx) <= 6 && b.passed.insert(j).second)
        c.line(wallSeconds(), f.frame, tick, "blocker-crossed", "blocker=" + std::to_string(b.idx) + ";lemming=" + std::to_string(j) + ";dx=" + std::to_string(pdx) + "->" + std::to_string(dx));
    }
  }

  // Builder effects.
  for (auto& b : a.builders) {
    if (b.idx >= lemmings.size()) continue;
    const auto& bl = lemmings[b.idx];
    if (!b.done) {
      if (bl.state == sim::State::Builder) {
        ++b.frames;
        b.xEnd = bl.x; b.yEnd = bl.y;
        b.bricksMin = std::min(b.bricksMin, bl.bricksLeft);
      } else {
        b.done = true;
        b.tickEnd = tick;
        b.xEnd = bl.x; b.yEnd = bl.y;
        finalizeBuilder(c, a, b, world, tick, f.frame, "left Builder state");
      }
    } else {
      for (std::size_t j = 0; j < lemmings.size(); ++j) {
        if (j == b.idx || !lemmings[j].alive()) continue;
        const auto& l = lemmings[j];
        const int ahead = wrapDiff(t, l.x, b.xEnd) * b.dir;
        if (ahead >= 1 && ahead <= 40 && std::abs(l.y - b.yEnd) <= 8 && b.crossers.insert(j).second)
          c.line(wallSeconds(), f.frame, tick, "builder-follower-past-end", "builder=" + std::to_string(b.idx) + ";lemming=" + std::to_string(j) + ";pos=" + std::to_string(l.x) + "/" + std::to_string(l.y));
      }
    }
  }

  for (std::size_t i = 0; i < lemmings.size(); ++i) {
    a.prevState[i] = lemmings[i].state;
    a.prevFacing[i] = lemmings[i].facing;
    a.prevX[i] = lemmings[i].x;
    a.prevFaster[i] = lemmings[i].fasterMs;
  }
}

void finalizeTracks(Ctx& c, Attempt& a, const app::LevelFrameView& f, const char* why) {
  if (a.tracksFinal) return;
  a.tracksFinal = true;
  const long tick = static_cast<long>(f.world.tickCount());
  for (auto& b : a.blockers)
    if (!b.ended) { b.ended = true; finalizeBlocker(c, b, tick, f.frame, why); }
  for (auto& b : a.builders) {
    if (!b.done) { b.done = true; b.tickEnd = tick; }
    finalizeBuilder(c, a, b, f.world, tick, f.frame, why);
  }
}

void beginAttempt(Ctx& c, const app::LevelFrameView& f) {
  c.att = Attempt{};
  c.attemptOpen = true;
  c.att.number = c.nextAttempt;
  c.att.revMode = c.step.policy == 0 ? 0 : c.nextRev;
  c.att.wall0 = wallSeconds();
  const auto& s = f.world.setup();
  c.att.stockStart = s.skills;
  c.att.total = s.totalLemmings;
  c.att.need = s.saveTarget;
  c.att.limitTicks = s.timeLimitTicks;
  const double limitSeconds = s.timeLimitTicks > 0 ? s.timeLimitTicks / 5.0 : 600.0;
  c.att.watchdogSeconds = std::min(900.0, std::max(120.0, limitSeconds / 3.0 + 90.0));
  if (!c.levelEntered) {
    c.levelEntered = true;
    c.enteredLevel = std::string(f.levelName);
    c.enteredCounter = f.frontendConstructionCounter;
  }
  std::ostringstream os;
  os << "attempt=" << c.att.number << ";policy=" << c.step.policy << ";revMode=" << c.att.revMode << ";revision_reason=" << c.nextRevReason
     << ";level=" << f.levelName << ";watchdog_s=" << fmt(c.att.watchdogSeconds, 0) << ";limit_ticks=" << s.timeLimitTicks
     << ";total=" << s.totalLemmings << ";need=" << s.saveTarget << ";levers=" << s.levers.size() << ";teleports=" << s.teleports.size()
     << ";speedpads=" << s.speedPads.size();
  c.line(wallSeconds(), f.frame, static_cast<long>(f.world.tickCount()), "attempt-begin", os.str());
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

  trackFrame(c, a, f);

  if (w - a.lastSampleWall >= 2.0) {
    a.lastSampleWall = w;
    c.samples << a.number << ',' << fmt(w, 2) << ',' << f.frame << ',' << tick << ',' << fmt(tick * 0.2, 1) << ','
              << a.lastSpawned << ',' << a.lastActive << ',' << a.lastSaved << ',' << a.lastDead << ','
              << a.lastRemainingTicks << ',' << phaseName(f.endPhase) << ',' << f.paused << ',' << f.selectedSkillIndex;
    for (int i = 0; i < 8; ++i) c.samples << ',' << a.stockEnd[static_cast<std::size_t>(i)];
    c.samples << ',';
    for (std::size_t i = 0; i < a.leverState.size(); ++i) c.samples << (i ? "/" : "") << a.leverState[i];
    c.samples << '\n';
    c.samples.flush();
  }
  if (w > a.watchdogSeconds) {
    a.watchdog = true;
    a.quitSent = true;
    c.line(wallSeconds(), f.frame, tick, "watchdog", "wall_s=" + fmt(w) + ";saved=" + std::to_string(a.lastSaved));
    c.line(wallSeconds(), f.frame, tick, "actors-at-watchdog", actorDump(f));
    finalizeTracks(c, a, f, "watchdog");
    shot(c, f.image, "attempt" + std::to_string(a.number) + "-watchdog.png");
    pushQuit();
    return;
  }
  if (f.endPhase != app::LevelEndPhase::Running) {
    if (!a.endingShot) {
      a.endingShot = true;
      a.endObservedWall = w;
      c.line(wallSeconds(), f.frame, tick, "end-phase", std::string(phaseName(f.endPhase)) + ";saved=" + std::to_string(a.lastSaved) +
             ";dead=" + std::to_string(a.lastDead) + ";remaining_ticks=" + std::to_string(a.lastRemainingTicks));
      c.line(wallSeconds(), f.frame, tick, "actors-at-end", actorDump(f));
      finalizeTracks(c, a, f, "level ended");
      shot(c, f.image, "attempt" + std::to_string(a.number) + "-ending.png");
    }
    return;
  }
  if (f.paused || f.levelMenuOpen) return;  // never pressed by this driver
  if (f.cameraIntroActive && w < 30.0) return;

  // Resolve the previous click: stock consumption, which lemming actually changed state, hovered snapshot.
  if (a.pending) {
    const auto& p = a.pendingData;
    if (f.frame <= p.frame) return;
    const int stockNow = f.world.setup().skills[static_cast<std::size_t>(p.skill)];
    const bool spent = p.stockBefore >= 0 && stockNow < p.stockBefore;
    const bool picked = a.lastTransition.frame >= p.frame && a.lastTransition.skill == p.skill;
    if (spent && !picked && p.skill >= 3 && f.frame < p.frame + 4) return;  // wait a few frames for the state change
    std::string pk = "-";
    if (picked) pk = std::to_string(a.lastTransition.idx);
    const std::string hov = f.hoveredActor ? std::to_string(*f.hoveredActor) : "-";
    if (spent) {
      ++a.assignOk;
      if (p.skill >= 3) {
        if (!picked) ++a.pickedNone;
        else if (a.lastTransition.idx == p.lemming) ++a.pickedIntended;
        else ++a.pickedOther;
      }
    } else {
      ++a.assignFail;
      a.cooldownUntil[p.lemming] = f.frame + 8;
      if (p.skill == policySkill(c.step.policy) && a.placed > 0) { --a.placed; if (!a.sites.empty()) a.sites.pop_back(); }
    }
    c.line(wallSeconds(), f.frame, tick, spent ? "assign-ok" : "assign-rejected",
           std::string("intended=") + std::to_string(p.lemming) + ";picked=" + pk + ";hovered_snapshot=" + hov + ";skill=" +
           kSkillNames[p.skill] + ";stock=" + std::to_string(p.stockBefore) + "->" + std::to_string(stockNow) + ";why=" + p.why);
    if (spent && a.assignOk <= 6) shot(c, f.image, "attempt" + std::to_string(a.number) + "-assign" + std::to_string(a.assignOk) + ".png");
    a.pending = false;
  }

  if (!a.ffClicked) {  // public Fast-forward plate, recorded; latch is never cleared by the clone
    a.ffClicked = true;
    a.ffFrame = f.frame;
    a.ffTick = tick;
    a.lastActionFrame = f.frame;
    pushClick(kFastForwardX, kFastForwardY);
    c.line(wallSeconds(), f.frame, tick, "click-fast-forward", "x=54;y=348;public SDL left click on plate 8");
    return;
  }
  if (f.frame - a.lastActionFrame < 2) return;
  const int wantSkill = policySkill(c.step.policy);
  if (!a.keyPreselected && f.selectedSkillIndex != wantSkill) {
    a.keyPreselected = true;
    pushSkillKey(wantSkill);
    a.lastActionFrame = f.frame;
    c.line(wallSeconds(), f.frame, tick, "key-select-skill", std::string(kSkillNames[wantSkill]) + ";key=" + std::to_string(wantSkill + 1));
    return;
  }

  const auto& lemmings = f.world.lemmings();
  const auto& stock = f.world.setup().skills;
  for (const auto& view : f.actors) {
    if (view.index >= lemmings.size()) continue;
    const auto& l = lemmings[view.index];
    if (!l.active()) continue;
    const Intent in = chooseIntent(c, a, l, f.world.terrain(), stock);
    if (in.skill < 0) continue;
    if (a.tries[{view.index, in.skill}] >= 3) continue;
    if (auto it = a.cooldownUntil.find(view.index); it != a.cooldownUntil.end() && f.frame < it->second) continue;
    const int x = static_cast<int>((view.placement.left + view.placement.right) / 2);
    const int y = static_cast<int>((view.placement.top + view.placement.bottom) / 2);
    if (x < 100 || x >= 640 || y < 0 || y >= 400) continue;
    if (f.selectedSkillIndex != in.skill) {
      if (a.keyPreselected && a.tries[{view.index, 99}]++ >= 6) continue;
      pushSkillKey(in.skill);
      a.lastActionFrame = f.frame;
      a.keyPreselected = true;
      c.line(wallSeconds(), f.frame, tick, "key-select-skill", std::string(kSkillNames[in.skill]) + ";key=" + std::to_string(in.skill + 1));
      return;
    }
    ++a.tries[{view.index, in.skill}];
    if (in.skill == wantSkill) { a.sites.emplace_back(l.x, l.y); ++a.placed; a.lastPlaceFrame = a.frames; }
    a.pending = true;
    a.pendingData = {f.frame, view.index, in.skill, stock[static_cast<std::size_t>(in.skill)], in.why};
    a.lastActionFrame = f.frame;
    ++a.clicks;
    pushClick(x, y);
    c.line(wallSeconds(), f.frame, tick, "click-lemming", "intended=" + std::to_string(view.index) + ";skill=" + kSkillNames[in.skill] +
           ";x=" + std::to_string(x) + ";y=" + std::to_string(y) + ";state=" + std::string(sim::stateName(l.state)) + ";pos=" +
           std::to_string(l.x) + "/" + std::to_string(l.y) + ";face=" + std::to_string(static_cast<int>(l.facing)) + ";why=" + in.why);
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
    os << "attempt " << a.number << " revMode=" << a.revMode << " outcome=" << classify(r) << " wall_s=" << fmt(r.wallEnd - a.wall0, 1)
       << " sim_ticks=" << a.lastTick << " sim_s=" << fmt(a.lastTick * 0.2, 1) << " spawned=" << a.lastSpawned << " saved=" << a.lastSaved
       << " dead=" << a.lastDead << " active=" << a.lastActive << " need=" << a.need << " total=" << a.total << " remaining_ticks="
       << a.lastRemainingTicks << " clicks=" << a.clicks << " assign_ok=" << a.assignOk << " assign_rejected=" << a.assignFail
       << " picked_intended=" << a.pickedIntended << " picked_other=" << a.pickedOther << " picked_none=" << a.pickedNone
       << " blockers_standing_seen=" << a.blockers.size() << " builders_seen=" << a.builders.size() << " lever_toggles=" << a.leverToggles
       << " speedpad_boosts=" << a.speedBoosts << " position_jumps=" << a.jumps << " ff_click_tick=" << a.ffTick
       << " watchdog_s=" << fmt(a.watchdogSeconds, 0) << " results_screen=" << r.resultsSeen;
    if (r.resultsSeen) os << " result_saved=" << r.saved << "/" << r.totalOut << " required=" << r.required << " success_flag=" << r.success
                          << " seconds_remaining=" << r.secondsRemaining;
    os << " stock_start=[";
    for (int i = 0; i < 8; ++i) os << (i ? "," : "") << a.stockStart[static_cast<std::size_t>(i)];
    os << "] stock_end=[";
    for (int i = 0; i < 8; ++i) os << (i ? "," : "") << a.stockEnd[static_cast<std::size_t>(i)];
    os << "]\n";
    for (const auto& b : a.blockers)
      os << "  blocker lemming=" << b.idx << " at=" << b.x << '/' << b.y << " ticks_standing=" << ((b.tickEnd >= 0 ? b.tickEnd : a.lastTick) - b.tick0)
         << " turned_near=" << b.turned.size() << " passed_through=" << b.passed.size() << " field_cells_seen=" << b.fieldCells << '\n';
    for (const auto& b : a.builders)
      os << "  builder lemming=" << b.idx << " start=" << b.x0 << '/' << b.y0 << " dir=" << b.dir << " frames=" << b.frames << " bricks_first="
         << b.bricksFirst << " bricks_min=" << (b.bricksMin == 999 ? -1 : b.bricksMin) << " end=" << b.xEnd << '/' << b.yEnd
         << " new_solid_cells=" << b.newCells << " others_past_end=" << b.crossers.size() << '\n';
  }
}

// ---- identity / saves --------------------------------------------------------------------------------
void writeIdentity(const fs::path& file, const fs::path& game, const std::string& extra) {
  std::ofstream os(file);
  os << "driver=test_playableexpansion (scripted clone observation)\n" << extra;
  os << "built=" << __DATE__ << ' ' << __TIME__ << "\n";
#ifdef __VERSION__
  os << "compiler=" << __VERSION__ << "\n";
#endif
  os << "sdl_compiled=" << SDL_MAJOR_VERSION << '.' << SDL_MINOR_VERSION << '.' << SDL_PATCHLEVEL << "\n";
  os << "game_dir=" << game.string() << "\n";
  os << "lemmings.box_bytes=" << (fs::exists(game / "lemmings.box") ? static_cast<long long>(fs::file_size(game / "lemmings.box")) : -1)
     << " patch.box_bytes=" << (fs::exists(game / "patch.box") ? static_cast<long long>(fs::file_size(game / "patch.box")) : -1) << "\n";
  const char* files[] = {"src/app/main.cpp", "src/app/AppEntry.h", "src/app/FrontendSession.cpp", "src/app/MapScene.cpp",
      "src/app/RuntimeSkillPanel.cpp", "src/app/RuntimePick.cpp", "src/sim/World.cpp", "src/sim/RetailLevel.cpp",
      "src/save/SaveStore.cpp", "src/__test__/test_playableexpansion.cpp"};
  for (const char* f : files) os << "fnv1a64 " << f << " = " << fnvHex(fs::path(PE_SOURCE_DIR) / f) << "\n";
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

int runApp(const std::string& gameDir, app::FrontendWindowObserver fo, app::LevelFrameObserver lo,
           std::shared_ptr<save::SaveStore> store) {
  std::vector<std::string> args{"playable-expansion", "--game-dir", gameDir};  // ordinary startup: no diagnostics
  std::vector<char*> argv;
  for (auto& s : args) argv.push_back(s.data());
  app::ApplicationRunOptions options;  // frameSeconds stays 0: production wall clock
  options.saveStore = std::move(store);
  return app::runApplication(static_cast<int>(argv.size()), argv.data(), std::move(fo), std::move(lo), options);
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

// ---- modes -------------------------------------------------------------------------------------------
void dumpTerrain(const AssetLibrary& lib, const std::string& level, const fs::path& out) {
  auto setup = sim::loadRetailLevel(lib, level);
  std::ofstream os(out);
  if (!setup) { os << "load failed: " << setup.error() << '\n'; return; }
  const auto& t = setup->terrain;
  std::vector<std::string> rows(static_cast<std::size_t>(t.height()), std::string(static_cast<std::size_t>(t.width()), '.'));
  for (int y = 0; y < t.height(); ++y)
    for (int x = 0; x < t.width(); ++x) {
      char ch = '.';
      if (t.solid(x, y)) ch = '#';
      else if (t.waterHazard(x, y) || t.zoneWater(x, y) || t.zoneAcid(x, y)) ch = '~';
      rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = ch;
    }
  auto mark = [&](int x, int y, char ch) {
    if (y >= 0 && y < t.height() && x >= 0 && x < t.width()) rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = ch;
  };
  for (const auto& e : setup->entrances) mark(e.x, e.y, 'H');
  for (const auto& e : setup->exits) mark(e.x, e.y, 'E');
  for (const auto& l : setup->levers) mark(l.x, l.y, 'L');
  os << level << " terrain " << t.width() << "x" << t.height() << " ('#' solid, '~' water/acid zone, 'H' hatch, 'E' exit, 'L' lever; read-only dump)\n";
  for (const auto& e : setup->entrances) os << "hatch " << e.x << "," << e.y << '\n';
  for (const auto& e : setup->exits) os << "exit " << e.x << "," << e.y << '\n';
  for (const auto& l : setup->levers) os << "lever " << l.x << "," << l.y << " " << l.width << "x" << l.height << " state=" << l.state << '\n';
  for (const auto& r : rows) os << r << '\n';
}

int surveyMode(const AssetLibrary& lib, const fs::path& root, const fs::path& game) {
  const fs::path dir = root / "survey";
  std::error_code ec;
  fs::create_directories(dir, ec);
  writeIdentity(dir / "identity.txt", game, "mode=survey\n");
  auto map = app::MapScene::build(lib, {});
  if (!map) { std::cerr << "survey: " << map.error() << '\n'; return 1; }
  const auto graph = computeLinks(lib);
  std::ofstream os(dir / "map-graph.txt");
  os << "Unlock rule (src/app/MapScene.cpp MapScene::build): a node is selectable if state 3 (fresh: nodes 0,1,2) or 2 (completed);\n"
        "completing node i (flag != 0) sets states of its two link records to 3. links[i] below are derived read-only with in-memory flags.\n";
  for (std::size_t i = 0; i < 102; ++i) {
    os << "node " << i << " " << (map->levelForNode(i) ? *map->levelForNode(i) : "-") << " links:";
    for (int m : graph.links[i]) os << ' ' << m;
    os << '\n';
  }
  const auto dist = winsNeeded(graph, map->states());
  std::vector<LevelFacts> facts;
  for (std::size_t n = 0; n < 102; ++n) {
    auto name = map->levelForNode(n);
    facts.push_back(name ? inspect(lib, n, *name) : LevelFacts{});
  }
  os << "\nShortest earned routes from the fresh map (genuine completions required before selectable):\n";
  for (int target : {3, 4, 8}) {
    os << "node " << target << " " << (facts[static_cast<std::size_t>(target)].ok ? facts[static_cast<std::size_t>(target)].level : "?")
       << ": wins needed before selectable = " << dist[static_cast<std::size_t>(target)] << "; chain " << chainTo(graph, map->states(), target) << '\n';
  }
  os << "\nGadget levels (lever/teleport/speed-pad) by wins needed from the fresh map:\n";
  std::vector<std::pair<int, std::size_t>> gadgets;
  for (std::size_t n = 0; n < 102; ++n) if (isGadget(facts[n]) && dist[n] >= 0) gadgets.emplace_back(dist[n], n);
  std::sort(gadgets.begin(), gadgets.end());
  for (const auto& [d, n] : gadgets) os << "  wins=" << d << " chain " << chainTo(graph, map->states(), static_cast<int>(n)) << " :: " << facts[n].summary << '\n';
  std::ofstream ls(dir / "level-facts.txt");
  for (std::size_t n = 0; n < 102; ++n) ls << (facts[n].ok ? facts[n].summary : "node " + std::to_string(n) + " no level/failed " + facts[n].error) << '\n';
  for (std::size_t n : {std::size_t{0}, std::size_t{3}, std::size_t{4}, std::size_t{8}})
    if (auto name = map->levelForNode(n)) dumpTerrain(lib, *name, dir / ("terrain-node" + std::to_string(n) + ".txt"));
  std::cout << "PlayableExpansion survey: node3 wins=" << dist[3] << " chain " << chainTo(graph, map->states(), 3) << "; node4 wins=" << dist[4]
            << " chain " << chainTo(graph, map->states(), 4) << "; node8 wins=" << dist[8] << " chain " << chainTo(graph, map->states(), 8)
            << " -> " << (dir / "map-graph.txt").string() << '\n';
  return os && ls ? 0 : 1;
}

int playMode(const AssetLibrary& lib, int stepNo, const fs::path& root, const std::string& gameDir) {
  const fs::path game(gameDir);
  std::error_code ec;
  const StepDef step = kSteps[stepNo - 1];
  fs::path runDir;
  if (stepNo == 1) {
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    char buf[64];
    std::strftime(buf, sizeof buf, "%Y%m%d-%H%M%S", std::gmtime(&tt));
    runDir = root / "runs" / buf;
    fs::create_directories(runDir, ec);
    if (ec) { std::cerr << "cannot create " << runDir << '\n'; return 1; }
    std::ofstream cur(root / "current-run.txt");
    cur << buf << '\n';
    std::ofstream ident(runDir / "run-id.txt");
    ident << "stamp=" << buf << "\nisolated_save=" << save::cloneSavePathUnder(runDir / "save").string()
          << "\nsave_provenance=created fresh by step 1 of this run (no prepared flags, no copied file); later steps only add genuine wins\n";
  } else {
    const auto cur = currentRunDir(root);
    if (!cur) { std::cerr << "no current-run.txt: step 1 did not run\n"; return 1; }
    runDir = *cur;
  }
  const fs::path stepDir = runDir / ("step-" + std::to_string(stepNo) + "-" + step.label);
  fs::remove_all(stepDir, ec);
  fs::create_directories(stepDir, ec);
  if (ec) { std::cerr << "cannot create " << stepDir << '\n'; return 1; }
  writeIdentity(stepDir / "identity.txt", game, "mode=play step=" + std::to_string(stepNo) + " run=" + runDir.filename().string() + "\n");

  Ctx c;
  c.dir = stepDir;
  c.step = step;
  c.targetNode = step.node;
  c.trace.open(stepDir / "action-trace.csv");
  c.samples.open(stepDir / "samples.csv");
  c.trace << "wall_s,attempt_slot,frame,sim_tick,kind,detail\n";
  c.samples << "attempt,wall_s,frame,sim_tick,sim_s,spawned,active,saved,dead,remaining_ticks,end_phase,paused,selected_skill,"
               "stock0,stock1,stock2,stock3,stock4,stock5,stock6,stock7,lever_states\n";

  const std::string normalBefore = normalSaveState();
  const auto saveFile = save::cloneSavePathUnder(runDir / "save");
  auto store = storeFor(runDir, game);
  const bool saveExistedAtStart = fs::exists(saveFile, ec);
  const std::string saveFnvAtStart = saveExistedAtStart ? fnvHex(saveFile) : "-";

  c.lastProgressWall = wallSeconds();
  const double startWall = wallSeconds();
  std::optional<MapGraph> graph;

  const auto decideTarget = [&](const app::FrontendSession& s) {
    c.statesAtMap = s.mapStates();
    c.flagsAtMap = s.completionFlags();
    auto map = app::MapScene::build(lib, c.flagsAtMap);
    std::ostringstream os;
    os << "step=" << stepNo << " (" << step.label << ")\nflags_at_map=" << arr(c.flagsAtMap) << "\nstates_at_map=" << arr(c.statesAtMap) << '\n';
    os << "selectable nodes (state 2/3):";
    std::vector<LevelFacts> sel;
    for (std::size_t n = 0; n < 102; ++n) {
      const auto st = c.statesAtMap[n];
      if (st != 2 && st != 3) continue;
      os << ' ' << n << (st == 2 ? "(done)" : "");
      if (map) if (auto name = map->levelForNode(n)) sel.push_back(inspect(lib, n, *name));
    }
    os << '\n';
    for (const auto& f : sel) os << "  " << f.summary << (isGadget(f) ? "  <- gadget level" : "") << '\n';
    if (step.policy == 3) {
      std::optional<std::size_t> pick;
      for (const auto& f : sel) if (f.node == 8 && isGadget(f)) pick = f.node;
      if (!pick)
        for (const auto& f : sel) if (isGadget(f) && c.flagsAtMap[f.node] == 0) { pick = f.node; break; }
      if (pick) {
        c.targetNode = *pick;
        os << "gadget choice: node " << *pick << " (nearest genuinely selectable gadget level; node 8 preferred when selectable)\n";
      } else {
        c.blocked = "no genuinely selectable gadget level on the earned map";
        graph = computeLinks(lib);
        const auto dist = winsNeeded(*graph, c.statesAtMap);
        os << "NO selectable gadget level. Remaining prerequisites per gadget level (wins still needed, chain from earned map):\n";
        auto base = app::MapScene::build(lib, {});
        for (std::size_t n = 0; n < 102 && base; ++n) {
          auto name = base->levelForNode(n);
          if (!name) continue;
          const auto f = inspect(lib, n, *name);
          if (isGadget(f) && dist[n] > 0) os << "  node " << n << " " << f.level << " wins_needed=" << dist[n] << " chain " << chainTo(*graph, c.statesAtMap, static_cast<int>(n)) << '\n';
        }
      }
    }
    if (c.blocked.empty()) {
      const auto st = c.statesAtMap[c.targetNode];
      if (st != 2 && st != 3) {
        c.blocked = "target node " + std::to_string(c.targetNode) + " is not selectable (state " + std::to_string(st) + "): prerequisite not earned";
        graph = computeLinks(lib);
        os << "target not selectable; chain from earned map: " << chainTo(*graph, c.statesAtMap, static_cast<int>(c.targetNode)) << '\n';
      } else if (auto name = map ? map->levelForNode(c.targetNode) : std::nullopt) {
        c.targetLevel = *name;
        if (auto px = pixelFor(lib, c.flagsAtMap, c.targetNode)) { c.clickPixX = px->first; c.clickPixY = px->second; }
        else c.blocked = "no map pixel picks node " + std::to_string(c.targetNode);
      } else {
        c.blocked = "node has no level";
      }
    }
    os << (c.blocked.empty() ? "target: node " + std::to_string(c.targetNode) + " " + c.targetLevel + " at pixel " + std::to_string(c.clickPixX) + "," + std::to_string(c.clickPixY) + "\n"
                             : "BLOCKED: " + c.blocked + "\n");
    std::ofstream(stepDir / "earned-map.txt") << os.str();
    c.line(wallSeconds(), 0, 0, "map-decision", c.blocked.empty() ? "target=" + std::to_string(c.targetNode) + ":" + c.targetLevel : "BLOCKED:" + c.blocked);
  };

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
        if (c.resultsHandledFor > 0 || !c.blocked.empty()) { pushQuit(); break; }
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
            c.line(wallSeconds(), frame, 0, "map-after-results", "saveCount=" + std::to_string(s.saveCount()) + ";flag_node=" +
                   std::to_string(c.targetNode < 102 ? s.completionFlags()[c.targetNode] : -1));
          }
          pushQuit();
          break;
        }
        if (!c.mapSeen) {
          c.mapSeen = true;
          shot(c, s.composeFrame(), "map-before.png");
          decideTarget(s);
        }
        if (!c.blocked.empty()) { pushQuit(); break; }
        if (canPush && c.mapClicks < 3) {
          ++c.mapClicks; c.lastPush = c.feCalls;
          pushClick(c.clickPixX, c.clickPixY);
          c.line(wallSeconds(), frame, 0, "click-map-node", "node=" + std::to_string(c.targetNode) + ";level=" + c.targetLevel +
                 ";x=" + std::to_string(c.clickPixX) + ";y=" + std::to_string(c.clickPixY));
        } else if (canPush) { c.anomaly += "map click did not select a level; "; pushQuit(); }
        break;
      case shell::Screen::Results: {
        const int attemptNo = static_cast<int>(c.results.size());
        if (c.resultsHandledFor < attemptNo + 1 && c.attemptOpen) {
          AttemptResult r;
          r.a = c.att;
          r.wallEnd = wallSeconds();
          r.resultsSeen = true;
          if (const auto& o = s.outcome()) {
            r.success = o->success; r.saved = o->saved; r.totalOut = o->total; r.required = o->required;
            r.secondsRemaining = o->secondsRemaining;
          }
          c.results.push_back(r);
          c.attemptOpen = false;
          c.resultsHandledFor = static_cast<int>(c.results.size());
          shot(c, s.composeFrame(), "attempt" + std::to_string(r.a.number) + "-results-screen.png");
          const std::string cls = classify(r);
          c.line(wallSeconds(), frame, 0, "results-screen", cls + ";saved=" + std::to_string(r.saved) + "/" + std::to_string(r.totalOut) +
                 ";required=" + std::to_string(r.required) + ";saveCount=" + std::to_string(s.saveCount()) + ";saveError=" +
                 (s.saveError() ? *s.saveError() : "-"));
          c.lastPush = -100;
          c.contClicks = 0;
          if (cls == "WIN") c.resultAction = 1;
          else if (r.a.number < step.maxAttempts) {
            c.resultAction = 2;
            c.nextAttempt = r.a.number + 1;
            // Revision from retained observation: nothing placed -> relax thresholds; placed but not enough -> more of it.
            c.nextRev = (step.policy == 0) ? 0 : (r.a.placed == 0 || r.a.assignOk == 0) ? 1 : 2;
            c.nextRevReason = "attempt " + std::to_string(r.a.number) + " " + cls + ": policy_skill_assign_ok=" + std::to_string(r.a.assignOk) +
                              " blockers_seen=" + std::to_string(r.a.blockers.size()) + " builders_seen=" + std::to_string(r.a.builders.size()) +
                              " dead=" + std::to_string(r.a.lastDead) + " saved=" + std::to_string(r.a.lastSaved) + "/" + std::to_string(r.a.need);
            c.line(wallSeconds(), frame, 0, "revision", "revMode=" + std::to_string(c.nextRev) + ";" + c.nextRevReason);
          } else {
            c.resultAction = 3;
          }
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
  const auto level = [&](const app::LevelFrameView& f) { levelObserver(c, f); };

  std::cout << "PlayableExpansion step " << stepNo << " (" << step.label << ") run " << runDir.filename().string() << " save " << saveFile.string() << '\n';
  const int status = runApp(gameDir, frontend, level, store);
  const double totalWall = wallSeconds() - startWall;
  const std::string normalAfter = normalSaveState();

  if (c.attemptOpen) {  // an attempt that never reached a results screen (watchdog / quit)
    AttemptResult r;
    r.a = c.att;
    r.wallEnd = wallSeconds();
    c.results.push_back(r);
    c.attemptOpen = false;
  }
  bool won = false;
  for (const auto& r : c.results) if (classify(r) == "WIN") won = true;
  std::string outcome;
  if (!c.blocked.empty()) outcome = "BLOCKED (prerequisite not selectable)";
  else if (!c.levelEntered) outcome = "BLOCKED (frontend never entered the level)";
  else if (won) outcome = "WIN";
  else if (!c.results.empty() && c.results.back().a.watchdog) outcome = "INCOMPLETE (watchdog)";
  else outcome = "LOSS";

  const bool saveExists = fs::exists(saveFile, ec);
  const auto saveBytes = saveExists ? static_cast<long long>(fs::file_size(saveFile, ec)) : -1;
  std::ofstream res(stepDir / "play-result.txt");
  res << "step=" << stepNo << "\nlabel=" << step.label << "\nrun=" << runDir.filename().string() << "\nlevel=" << c.targetLevel << "\nnode=" << c.targetNode
      << "\noutcome=" << outcome << "\nwon=" << (won ? 1 : 0) << "\nblocked=" << (c.blocked.empty() ? "-" : c.blocked)
      << "\nrunApplication_status=" << status << "\nframe_entry=" << (c.levelEntered ? 1 : 0) << "\nentered_level=" << c.enteredLevel
      << "\nentered_frontend_counter=" << (c.enteredCounter ? std::to_string(*c.enteredCounter) : "none") << "\ntotal_wall_s=" << fmt(totalWall, 1)
      << "\nsave_file=" << saveFile.string() << "\nsave_existed_at_start=" << saveExistedAtStart << "\nsave_fnv_at_start=" << saveFnvAtStart
      << "\nsave_exists=" << saveExists << "\nsave_bytes=" << saveBytes << "\nsave_fnv=" << (saveExists ? fnvHex(saveFile) : "-")
      << "\nsave_count_at_map=" << c.saveCountAtMap << "\nsave_error_at_map=" << (c.saveErrorAtMap ? *c.saveErrorAtMap : "-")
      << "\nreturned_to_map=" << (c.returnedToMap ? 1 : 0) << "\nflags_at_map_before=" << arr(c.flagsAtMap) << "\nstates_at_map_before=" << arr(c.statesAtMap)
      << "\nflags_after=" << arr(c.flagsAfter) << "\nstates_after=" << arr(c.statesAfter)
      << "\nnormal_save_before=" << normalBefore << "\nnormal_save_after=" << normalAfter << "\nnormal_save_unchanged=" << (normalBefore == normalAfter ? 1 : 0)
      << "\nanomaly=" << (c.anomaly.empty() ? "-" : c.anomaly) << '\n';
  writeAttempts(c, res);
  std::ofstream md(stepDir / "result.md");
  md << "# Step " << stepNo << " " << step.label << ": " << c.targetLevel << " (node " << c.targetNode << ")\n\nOutcome: **" << outcome << "**\n\n```\n";
  writeAttempts(c, md);
  md << "```\n";

  std::cout << "PlayableExpansion step " << stepNo << " " << c.targetLevel << " (node " << c.targetNode << "): " << outcome << " wall=" << fmt(totalWall, 1)
            << "s attempts=" << c.results.size() << " status=" << status << " anomaly=" << (c.anomaly.empty() ? "-" : c.anomaly)
            << (c.blocked.empty() ? "" : " blocked=" + c.blocked) << '\n';
  writeAttempts(c, std::cout);

  // Driver integrity only: gameplay outcome is reported above and never decides pass/fail.
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
    md << "# Step " << stepNo << " restart: UNVERIFIED\n\nNo genuine completion was observed by the play invocation (outcome="
       << (kv.count("outcome") ? kv.at("outcome") : "no play result") << "); persistence not inspected.\n";
    std::cout << "PlayableExpansion restart step " << stepNo << ": UNVERIFIED (no genuine completion)\n";
    return md ? 0 : 1;
  }
  const std::size_t node = static_cast<std::size_t>(std::stoul(kv.at("node")));
  const std::string saveFile = kv.at("save_file");
  struct Seen { int calls = 0; bool map = false; Flags flags{}, states{}; std::optional<std::string> loadError; int saveCount = -1; int clicks = 0; int lastPush = -100; } seen;
  const auto frontend = [&](const app::FrontendSession& s, int /*frame*/) {
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
        if (auto r = writePng(stepDir / "restart-map.png", s.composeFrame()); !r) std::cerr << r.error() << '\n';
      }
      pushQuit();
    } else { pushQuit(); }
  };
  const std::string before = fnvHex(saveFile);
  const int status = runApp(gameDir, frontend, {}, storeFor(*cur, game));
  const std::string after = fnvHex(saveFile);
  const bool flagSet = node < 102 && seen.flags[node] == 1;
  const bool matches = arr(seen.flags) == kv.at("flags_after") && arr(seen.states) == kv.at("states_after");
  std::error_code ec;
  const bool ok = status == 0 && seen.map && flagSet && matches && before == after;
  md << "# Step " << stepNo << " restart (fresh process, same isolated save)\n\nrunApplication status=" << status << " reachedMap=" << seen.map
     << " loadError=" << (seen.loadError ? *seen.loadError : "-") << "\nnode " << node << " completion flag after restart = " << (node < 102 ? seen.flags[node] : -1)
     << "\nflags/states equal the play invocation's map: " << matches << "\nsaveCount after load (loading never writes) = " << seen.saveCount
     << "\nsave file bytes=" << (fs::exists(saveFile, ec) ? static_cast<long long>(fs::file_size(saveFile, ec)) : -1) << " fnv before=" << before
     << " after=" << after << " (unchanged: " << (before == after) << ")\nRESULT: " << (ok ? "VERIFIED" : "FAILED") << "\n";
  std::cout << "PlayableExpansion restart step " << stepNo << ": " << (ok ? "VERIFIED" : "FAILED (persistence finding)") << " flag=" << (node < 102 ? seen.flags[node] : -1) << '\n';
  return status == 0 && md ? 0 : 1;  // a persistence mismatch is a reported finding, not a driver fault
}

}  // namespace
#endif

int main(int argc, char** argv) {
#if !WEBOLUTION_HAVE_SDL2
  std::cerr << "PlayableExpansion: requires SDL2\n";
  return 1;
#else
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir) { std::cout << "PlayableExpansion: SKIPPED (set WEBOLUTION_GAME_DIR)\n"; return 0; }
  const char* video = std::getenv("SDL_VIDEODRIVER");
  if (!video || std::string(video) != "dummy") { std::cerr << "PlayableExpansion: refusing to run without SDL_VIDEODRIVER=dummy\n"; return 1; }
  if (argc < 4) { std::cerr << "usage: test_playableexpansion survey|play|restart <step> <output-dir>\n"; return 1; }
  const std::string mode = argv[1];
  const int step = std::atoi(argv[2]);
  const fs::path root(argv[3]);
  std::error_code ec;
  fs::create_directories(root, ec);
  auto library = AssetLibrary::open(fs::path(gameDir) / "lemmings.box", fs::path(gameDir) / "patch.box");
  if (!library) { std::cerr << "assets: " << library.error() << '\n'; return 1; }
  if (mode == "survey") return surveyMode(*library, root, fs::path(gameDir));
  if (step < 1 || step > 4) { std::cerr << "step must be 1..4\n"; return 1; }
  if (mode == "play") return playMode(*library, step, root, gameDir);
  if (mode == "restart") return restartMode(step, root, gameDir);
  std::cerr << "unknown mode\n";
  return 1;
#endif
}
