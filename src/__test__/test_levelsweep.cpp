// chunk K <dir> launches one child process per map node under a wall-clock watchdog.
// level N <dir> enters directly through webo::app::runApplication with constant frame
// deltas and no gameplay input. The read-only observer records the settled view.
// report 0 <dir> aggregates all 102 nodes into matrices, contact sheets and groups.
// Load, render and record failures fail the sweep.
#include "app/AppEntry.h"
#include "FrameConsumptionAudit.h"
#include "app/BackdropScene.h"
#include "app/MapScene.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "assets/LevelObjects.h"
#include "assets/Png.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"

#ifndef WEBOLUTION_HAVE_SDL2
#define WEBOLUTION_HAVE_SDL2 0
#endif

#if WEBOLUTION_HAVE_SDL2
#define SDL_MAIN_HANDLED
#include <SDL.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifndef LS_SOURCE_DIR
#define LS_SOURCE_DIR "."
#endif

namespace fs = std::filesystem;
using namespace webo;

namespace {

constexpr int kNodes = 102;
constexpr int kChunkSize = 6;                       // 17 chunks x 6 = 102
constexpr double kFrameSeconds = 1.0;               // seconds per frame
constexpr double kIdleAfterReleaseSeconds = 5.0;    // idle window after the last scripted release
constexpr double kChildWatchdogSeconds = 420.0;     // parent kills a child after this much wall time
const char* kSkillNames[8] = {"Climber", "Floater", "Bomber", "Blocker", "Builder", "Basher", "Miner", "Digger"};

using SteadyClock = std::chrono::steady_clock;
const SteadyClock::time_point kProcessStart = SteadyClock::now();
double wallSeconds() { return std::chrono::duration<double>(SteadyClock::now() - kProcessStart).count(); }

std::string fmt(double v, int digits = 2) {
  std::ostringstream os;
  os.setf(std::ios::fixed);
  os.precision(digits);
  os << v;
  return os.str();
}
std::string pad3(int n) {
  std::ostringstream os;
  os << std::setw(3) << std::setfill('0') << n;
  return os.str();
}
std::string fnvHex(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return "absent";
  std::uint64_t h = 1469598103934665603ull;
  char c;
  while (in.get(c)) { h ^= static_cast<unsigned char>(c); h *= 1099511628211ull; }
  std::ostringstream os;
  os << std::hex << h;
  return os.str();
}
std::string oneLine(std::string s) {
  for (char& c : s) if (c == '\n' || c == '\r' || c == '\t') c = ' ';
  return s;
}

void require(bool ok, const std::string& what) {
  if (!ok) throw std::runtime_error("driver integrity failure: " + what);
}

std::string gameDirFromEnv() {
  const char* env = std::getenv("WEBOLUTION_GAME_DIR");
  require(env && *env, "WEBOLUTION_GAME_DIR required");
  return env;
}

// key=value record, one per line. Values never contain a newline.
using Record = std::vector<std::pair<std::string, std::string>>;
void put(Record& r, const std::string& k, const std::string& v) { r.emplace_back(k, oneLine(v)); }
void writeRecord(const fs::path& path, const Record& r) {
  std::ofstream out(path, std::ios::binary);
  for (const auto& [k, v] : r) out << k << '=' << v << '\n';
}
std::map<std::string, std::string> readRecord(const fs::path& path) {
  std::map<std::string, std::string> m;
  std::ifstream in(path, std::ios::binary);
  std::string line;
  while (std::getline(in, line)) {
    const auto eq = line.find('=');
    if (eq != std::string::npos) m[line.substr(0, eq)] = line.substr(eq + 1);
  }
  return m;
}

// Measure rows [0, kViewRows) separately. The skill panel and HUD can make a blank view appear nonblank.
constexpr int kViewRows = 320;
struct FrameStats {
  int w = 0, h = 0;
  std::size_t distinctAll = 0, distinctView = 0;
  double meanLumView = 0, sdLumView = 0, modalFracView = 0, blackFracAll = 0, blackFracView = 0;
  std::uint64_t hash = 0;
  std::string verdict = "none";
};
FrameStats frameStats(const Image& img) {
  FrameStats s;
  s.w = img.width;
  s.h = img.height;
  if (img.empty() || img.width <= 0 || img.height <= 0) return s;
  const int viewRows = std::min(kViewRows, img.height);
  std::vector<std::uint32_t> all, view;
  all.reserve(img.pixelCount());
  double sum = 0, sumSq = 0;
  std::size_t blackAll = 0, blackView = 0;
  std::uint64_t h = 1469598103934665603ull;
  for (int y = 0; y < img.height; ++y)
    for (int x = 0; x < img.width; ++x) {
      const std::uint8_t* p = &img.rgba[(static_cast<std::size_t>(y) * img.width + x) * 4];
      const std::uint32_t c = (std::uint32_t{p[0]} << 16) | (std::uint32_t{p[1]} << 8) | p[2];
      all.push_back(c);
      h = (h ^ c) * 1099511628211ull;
      const bool black = c == 0;
      blackAll += black;
      if (y < viewRows) {
        view.push_back(c);
        blackView += black;
        const double lum = 0.299 * p[0] + 0.587 * p[1] + 0.114 * p[2];
        sum += lum;
        sumSq += lum * lum;
      }
    }
  s.hash = h;
  const auto distinct = [](std::vector<std::uint32_t> v, std::size_t& modal) {
    std::sort(v.begin(), v.end());
    std::size_t d = 0, run = 0;
    modal = 0;
    for (std::size_t i = 0; i < v.size(); ++i) {
      if (i == 0 || v[i] != v[i - 1]) { ++d; run = 0; }
      modal = std::max(modal, ++run);
    }
    return d;
  };
  std::size_t modalAll = 0, modalView = 0;
  s.distinctAll = distinct(all, modalAll);
  s.distinctView = distinct(view, modalView);
  const double n = static_cast<double>(view.size());
  s.meanLumView = sum / n;
  s.sdLumView = std::sqrt(std::max(0.0, sumSq / n - s.meanLumView * s.meanLumView));
  s.modalFracView = static_cast<double>(modalView) / n;
  s.blackFracAll = static_cast<double>(blackAll) / static_cast<double>(all.size());
  s.blackFracView = static_cast<double>(blackView) / n;
  if (s.distinctView <= 2 || s.modalFracView >= 0.98) s.verdict = "BLANK";
  else if (s.modalFracView >= 0.90 || s.sdLumView < 8.0) s.verdict = "NEAR-UNIFORM";
  else s.verdict = "ok";
  return s;
}
void putStats(Record& r, const std::string& p, const FrameStats& s) {
  put(r, p + "_size", std::to_string(s.w) + "x" + std::to_string(s.h));
  put(r, p + "_distinct_all", std::to_string(s.distinctAll));
  put(r, p + "_distinct_view", std::to_string(s.distinctView));
  put(r, p + "_mean_lum_view", fmt(s.meanLumView, 1));
  put(r, p + "_sd_lum_view", fmt(s.sdLumView, 1));
  put(r, p + "_modal_frac_view", fmt(s.modalFracView, 4));
  put(r, p + "_black_frac_all", fmt(s.blackFracAll, 4));
  put(r, p + "_black_frac_view", fmt(s.blackFracView, 4));
  std::ostringstream hs;
  hs << std::hex << s.hash;
  put(r, p + "_hash", hs.str());
  put(r, p + "_verdict", s.verdict);
}

// 160x120 box-average thumbnail (4x4 boxes) as raw RGB, for the contact sheets.
void writeThumb(const fs::path& path, const Image& img) {
  if (img.width != 640 || img.height != 480) return;
  std::ofstream out(path, std::ios::binary);
  for (int ty = 0; ty < 120; ++ty)
    for (int tx = 0; tx < 160; ++tx) {
      int acc[3] = {0, 0, 0};
      for (int dy = 0; dy < 4; ++dy)
        for (int dx = 0; dx < 4; ++dx) {
          const std::uint8_t* p = &img.rgba[(static_cast<std::size_t>(ty * 4 + dy) * 640 + tx * 4 + dx) * 4];
          for (int c = 0; c < 3; ++c) acc[c] += p[c];
        }
      for (int c = 0; c < 3; ++c) out.put(static_cast<char>(acc[c] / 16));
    }
}

std::optional<std::string> storedIdForNode(const AssetLibrary& lib, int node) {
  auto map = app::MapScene::build(lib, {});
  require(bool(map), "MapScene::build failed");
  return map->levelForNode(static_cast<std::size_t>(node));
}

struct ChildCtx {
  fs::path dir;
  Record rec;
  int frames = 0, settledFrame = -1;
  long settledMs = -1;
  bool settledSeen = false;
  FrameStats settled, last;
  Image lastImage;
  std::string stop = "frame-cap";
  bool quitSent = false;
  long releaseDoneMs = -1;
  double capSeconds = 0;
  int total = 0;
  // Snapshot the last observed frame.
  int spawned = 0, saved = 0, dead = 0, active = 0;
  long clockMs = 0;
  bool finished = false, timedOut = false;
  std::map<std::string, int> states;
  std::string endPhase = "Running";
  std::optional<int> introFrames;
  double firstFrameWall = -1;
};

const char* phaseName(app::LevelEndPhase p) {
  return p == app::LevelEndPhase::Running ? "Running" : p == app::LevelEndPhase::Pending ? "Pending" : "Results";
}

void observe(ChildCtx& c, const app::LevelFrameView& f) {
  if (c.firstFrameWall < 0) c.firstFrameWall = wallSeconds();
  ++c.frames;
  const auto& w = f.world;
  c.spawned = w.spawned();
  c.saved = w.saved();
  c.dead = w.dead();
  c.active = w.activeCount();
  c.clockMs = static_cast<long>(w.levelClockMs());
  c.finished = w.finished();
  c.timedOut = w.timedOut();
  c.endPhase = phaseName(f.endPhase);
  c.states.clear();
  for (const auto& l : w.lemmings()) ++c.states[std::string(sim::stateName(l.state))];
  const bool settled = !f.cameraIntroActive && !f.entryCueFrame && !f.levelMenuOpen;
  if (settled && !c.settledSeen) {
    c.settledSeen = true;
    c.settledFrame = f.frame;
    c.settledMs = c.clockMs;
    c.settled = frameStats(test::inspectedImage(f));
    (void)writePng(c.dir / "frame-settled.png", test::inspectedImage(f));
    writeThumb(c.dir / "thumb.rgb", test::inspectedImage(f));
  }
  if (c.releaseDoneMs < 0 && c.total > 0 && c.spawned >= c.total) c.releaseDoneMs = c.clockMs;
  if (c.quitSent) return;
  const char* why = nullptr;
  if (c.releaseDoneMs >= 0 && c.clockMs >= c.releaseDoneMs + static_cast<long>(kIdleAfterReleaseSeconds * 1000)) why = "idle-window-after-release";
  else if (c.clockMs >= static_cast<long>(c.capSeconds * 1000)) why = "sim-cap";
  else if (f.endPhase != app::LevelEndPhase::Running) why = "end-transition";
  if (why) {
    c.lastImage = test::inspectedImage(f);
    c.stop = why;
    SDL_Event quit{};
    quit.type = SDL_QUIT;
    c.quitSent = SDL_PushEvent(&quit) == 1;
  }
}

int runChild(int node, const fs::path& root) {
  const std::string gameDir = gameDirFromEnv();
  const fs::path dir = root / ("node-" + pad3(node));
  fs::create_directories(dir);
  ChildCtx c;
  c.dir = dir;
  Record& r = c.rec;
  put(r, "node", std::to_string(node));
  put(r, "entry", "DIRECT --level (not frontend reachability)");
  put(r, "timing", "scripted source-test frameSeconds=" + fmt(kFrameSeconds, 2) + " per frame; production clock unchanged");
  const auto finish = [&](const std::string& outcome, const std::string& stage, const std::string& detail) {
    put(r, "outcome", outcome);
    put(r, "failing_stage", stage);
    put(r, "failure_detail", detail);
    put(r, "child_wall_s", fmt(wallSeconds(), 1));
    writeRecord(dir / "result.txt", r);
    return 0;  // exit code 0 indicates the level record was written
  };
  try {
    auto lib = AssetLibrary::open(fs::path(gameDir) / "lemmings.box", fs::path(gameDir) / "patch.box");
    if (!lib) return finish("FAIL", "archive-open", lib.error());
    const auto stored = storedIdForNode(*lib, node);
    if (!stored) return finish("FAIL", "map-node", "levelForNode returned no id");
    put(r, "stored_id", *stored);
    auto resolved = sim::resolveRetailLevel(*lib, *stored);
    if (!resolved) return finish("FAIL", "resolve", resolved.error());
    put(r, "resolved_id", resolved->id);
    put(r, "resolve_match", resolved->match == sim::LevelMatch::ExactId ? "ExactId" : resolved->match == sim::LevelMatch::Substring ? "Substring" : "CaseInsensitive");
    put(r, "file", resolved->file);

    // Map-level facts (LEVELMAP.DAT name).
    if (auto infos = loadLevelMap(*lib)) {
      for (const auto& li : *infos)
        if (li.id == resolved->id || (li.id.size() == resolved->id.size() &&
              std::equal(li.id.begin(), li.id.end(), resolved->id.begin(), [](char a, char b) { return std::toupper(static_cast<unsigned char>(a)) == std::toupper(static_cast<unsigned char>(b)); }))) {
          put(r, "level_name", li.name);
          break;
        }
    }

    // Load result: the production loader (the same call runApplicationLeg makes).
    auto report = sim::loadRetailLevelReport(*lib, *stored);
    if (!report) return finish("FAIL", "load(setup)", report.error());
    const auto& setup = report->setup;
    put(r, "load_result", "ok");
    put(r, "gate_link_diagnostic", report->gateLinkDiagnostic.empty() ? "-" : report->gateLinkDiagnostic);
    put(r, "terrain", std::to_string(setup.terrain.width()) + "x" + std::to_string(setup.terrain.height()));
    put(r, "total_lemmings", std::to_string(setup.totalLemmings));
    put(r, "save_target", std::to_string(setup.saveTarget));
    put(r, "time_limit_s", fmt(setup.timeLimitTicks / 5.0, 1));
    put(r, "hatches", std::to_string(setup.entrances.size()));
    put(r, "exits", std::to_string(setup.exits.size()));
    {
      std::ostringstream hs;
      for (std::size_t i = 0; i < report->hatches.size(); ++i) {
        const auto& h = report->hatches[i];
        hs << (i ? ";" : "") << "n" << h.count << "@rate" << h.clampedRate << "(" << fmt(h.intervalSeconds, 2) << "s)";
      }
      put(r, "hatch_records", hs.str());
    }
    {
      std::ostringstream ss;
      long stock = 0;
      for (int i = 0; i < 8; ++i) {
        ss << (i ? "," : "") << setup.skills[static_cast<std::size_t>(i)];
        stock += std::max(0, setup.skills[static_cast<std::size_t>(i)]);
      }
      put(r, "skills", ss.str());
      put(r, "skills_total", std::to_string(stock));
    }
    put(r, "teleports", std::to_string(setup.teleports.size()));
    put(r, "speedpads", std::to_string(setup.speedPads.size()));
    put(r, "levers", std::to_string(setup.levers.size()));
    put(r, "fluid_gates", std::to_string(setup.gates.size()));
    {
      long water = 0, tramp = 0, anti = 0;
      const auto& t = setup.terrain;
      for (int y = 0; y < t.height(); ++y)
        for (int x = 0; x < t.width(); ++x) {
          if (t.waterHazard(x, y) || t.zoneWater(x, y) || t.zoneAcid(x, y)) ++water;
          if (t.trampoline(x, y)) ++tramp;
          if (t.antiGrav(x, y)) ++anti;
        }
      put(r, "hazard_cells", std::to_string(water));
      put(r, "trampoline_cells", std::to_string(tramp));
      put(r, "antigrav_cells", std::to_string(anti));
    }
    // Read placed gadget model a3d basenames from the level stream.
    auto bytes = lib->read(resolved->file);
    if (!bytes) return finish("FAIL", "read(.LVL)", bytes.error());
    if (auto objs = findLevelObjects(*bytes)) {
      std::map<std::string, int> names;
      for (const auto& p : objs->placed) ++names[p.name];
      std::ostringstream ns;
      bool first = true;
      for (const auto& [n, k] : names) { ns << (first ? "" : ";") << n << "x" << k; first = false; }
      put(r, "gadget_models", ns.str().empty() ? "-" : ns.str());
      put(r, "gadget_model_count", std::to_string(objs->placed.size()));
      put(r, "hatch_fallback", objs->hatchFallbackReason.empty() ? "-" : objs->hatchFallbackReason);
    } else {
      put(r, "gadget_models", "ERROR " + objs.error());
    }
    // Check scenery admission with the same bytes and ID. An empty scene means scenery was omitted.
    {
      auto built = app::BackdropScene::build(*lib, *bytes, *stored);
      if (!built) {
        put(r, "scenery", "BUILD-ERROR");
        put(r, "scenery_detail", built.error());
      } else {
        put(r, "scenery", built->empty() ? "not-admitted(empty partial scene)" : "admitted");
        std::ostringstream ds;
        for (const auto& d : built->diagnostics()) ds << d << " || ";
        put(r, "scenery_detail", ds.str().empty() ? "-" : ds.str());
      }
    }

    // Size the idle window for arming, the first countdown and remaining releases.
    // Hatches share the first hatch's cadence and release in round-robin order.
    const double interval = setup.releaseIntervalSeconds > 0 ? setup.releaseIntervalSeconds : setup.releaseIntervalTicks / 5.0;
    const double releaseEnd = setup.releaseArmSeconds + setup.releaseFirstCountdownSeconds +
                              std::max(0, setup.totalLemmings - 1) * interval;
    const double limit = setup.timeLimitTicks / 5.0;
    c.total = setup.totalLemmings;
    c.capSeconds = limit > 0 ? limit + 2.0 : releaseEnd + 60.0;
    const double needSeconds = std::min(c.capSeconds, releaseEnd + kIdleAfterReleaseSeconds + 10.0);
    const int frameCap = static_cast<int>(std::ceil(needSeconds / kFrameSeconds)) + 60;
    put(r, "release_interval_s", fmt(interval, 3));
    put(r, "expected_release_end_s", fmt(releaseEnd, 1));
    put(r, "sim_cap_s", fmt(c.capSeconds, 1));
    put(r, "frame_cap", std::to_string(frameCap));

    std::vector<std::string> args = {"level-sweep", "--game-dir", gameDir, "--level", *stored, "--sim",
                                     "--frames", std::to_string(frameCap), "--no-audio"};
    std::vector<char*> argv;
    for (auto& a : args) argv.push_back(a.data());
    app::ApplicationRunOptions options;
    options.frameSeconds = kFrameSeconds;
    test::FrameConsumptionAudit audit(dir);
    const double wallStart = wallSeconds();
    const int status = app::runApplication(static_cast<int>(argv.size()), argv.data(), {},
                                           [&](const app::LevelFrameView& f) {
                                             audit.frame(f);
                                             observe(c, f);
                                             // The frame cap is also a terminal image consumer.
                                             if (f.frame + 1 == frameCap) c.lastImage = test::inspectedImage(f);
                                             if (c.quitSent) audit.action("quit");
                                           }, options);
    const double wallRun = wallSeconds() - wallStart;
    put(r, "run_status", std::to_string(status));
    put(r, "frames_observed", std::to_string(c.frames));
    put(r, "run_wall_s", fmt(wallRun, 1));
    put(r, "stop_reason", c.stop);
    put(r, "sim_s", fmt(c.clockMs / 1000.0, 1));
    put(r, "sim_over_wall", wallRun > 0 ? fmt(c.clockMs / 1000.0 / wallRun, 1) : "-");
    put(r, "settled_frame", c.settledSeen ? std::to_string(c.settledFrame) : "never");
    put(r, "settled_sim_ms", c.settledSeen ? std::to_string(c.settledMs) : "-");
    if (c.settledSeen) putStats(r, "settled", c.settled);
    if (c.frames > 0) {
      c.last = frameStats(c.lastImage);
      (void)writePng(dir / "frame-final.png", c.lastImage);
      putStats(r, "final", c.last);
    }
    put(r, "spawned", std::to_string(c.spawned));
    put(r, "expected", std::to_string(c.total));
    put(r, "saved", std::to_string(c.saved));
    put(r, "dead", std::to_string(c.dead));
    put(r, "active", std::to_string(c.active));
    put(r, "world_finished", c.finished ? "1" : "0");
    put(r, "timed_out", c.timedOut ? "1" : "0");
    put(r, "end_phase", c.endPhase);
    {
      std::ostringstream st;
      bool first = true;
      for (const auto& [n, k] : c.states) { st << (first ? "" : ";") << n << ":" << k; first = false; }
      put(r, "final_states", st.str().empty() ? "-" : st.str());
    }
    if (status != 0 && c.frames == 0) return finish("FAIL", "app-entry (runApplication returned nonzero before any frame; see child.log)", "status " + std::to_string(status));
    if (c.frames == 0) return finish("FAIL", "render (no level frame observed)", "status " + std::to_string(status));
    if (status != 0) return finish("FAIL", "run (nonzero status after frames)", "status " + std::to_string(status));
    return finish("completed", "-", "-");
  } catch (const std::exception& e) {
    return finish("EXCEPTION", "driver/exception", e.what());
  }
}

fs::path normalSavePath() {
  const char* la = std::getenv("LOCALAPPDATA");
  return la ? fs::path(la) / "LemmingsWebolution" / "SavedGame.dat" : fs::path();
}

struct ChildResult { DWORD exitCode = 0; bool timedOut = false, launched = false; double wall = 0; };

ChildResult spawnChild(const fs::path& exe, int node, const fs::path& root, const fs::path& logFile) {
  ChildResult res;
  SECURITY_ATTRIBUTES sa{sizeof sa, nullptr, TRUE};
  HANDLE log = CreateFileA(logFile.string().c_str(), GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  STARTUPINFOA si{};
  si.cb = sizeof si;
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdOutput = log;
  si.hStdError = log;
  si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
  PROCESS_INFORMATION pi{};
  std::string cmd = "\"" + exe.string() + "\" level " + std::to_string(node) + " \"" + root.string() + "\"";
  const double t0 = wallSeconds();
  if (CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
    res.launched = true;
    if (WaitForSingleObject(pi.hProcess, static_cast<DWORD>(kChildWatchdogSeconds * 1000)) == WAIT_TIMEOUT) {
      res.timedOut = true;
      TerminateProcess(pi.hProcess, 0xDEAD);
      WaitForSingleObject(pi.hProcess, 10000);
    }
    GetExitCodeProcess(pi.hProcess, &res.exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }
  res.wall = wallSeconds() - t0;
  if (log != INVALID_HANDLE_VALUE) CloseHandle(log);
  return res;
}

std::string interestingLog(const fs::path& logFile) {
  std::ifstream in(logFile);
  std::string line, out;
  std::vector<std::string> keep;
  while (std::getline(in, line)) {
    if (line.rfind("sim: tick=", 0) == 0) continue;
    keep.push_back(line);
  }
  // Last six non-trivial lines: enough for an assertion/abort message or a loader error.
  for (std::size_t i = keep.size() > 6 ? keep.size() - 6 : 0; i < keep.size(); ++i) out += (out.empty() ? "" : " // ") + keep[i];
  return out;
}

int runChunk(int chunk, const fs::path& root) {
  const std::string gameDir = gameDirFromEnv();
  const fs::path chunkDir = root / ("chunk-" + pad3(chunk));
  fs::create_directories(chunkDir);
  char exeBuf[MAX_PATH];
  const DWORD n = GetModuleFileNameA(nullptr, exeBuf, MAX_PATH);
  require(n > 0 && n < MAX_PATH, "own executable path");
  const fs::path exe = exeBuf;
  const fs::path normal = normalSavePath();
  const std::string saveBefore = fnvHex(normal);
  std::ofstream summary(chunkDir / "chunk.txt");
  int visited = 0, withRecord = 0;
  for (int node = chunk * kChunkSize; node < std::min(kNodes, (chunk + 1) * kChunkSize); ++node) {
    const fs::path nodeDir = chunkDir / ("node-" + pad3(node));
    fs::create_directories(nodeDir);
    fs::remove(nodeDir / "result.txt");
    const auto res = spawnChild(exe, node, chunkDir, nodeDir / "child.log");
    ++visited;
    Record p;
    put(p, "node", std::to_string(node));
    put(p, "child_launched", res.launched ? "1" : "0");
    put(p, "child_exit_code", std::to_string(res.exitCode));
    put(p, "child_exit_hex", [&] { std::ostringstream os; os << "0x" << std::hex << res.exitCode; return os.str(); }());
    put(p, "child_watchdog_killed", res.timedOut ? "1" : "0");
    put(p, "child_wall_s_parent", fmt(res.wall, 1));
    put(p, "child_log_tail", interestingLog(nodeDir / "child.log"));
    writeRecord(nodeDir / "parent.txt", p);
    const bool haveRecord = fs::exists(nodeDir / "result.txt");
    if (!haveRecord) {
      // The child died before it could write its own record: record that explicitly, naming the level.
      Record r;
      put(r, "node", std::to_string(node));
      put(r, "outcome", res.timedOut ? "HANG" : "CRASH");
      put(r, "failing_stage", res.timedOut ? "watchdog (child killed after " + fmt(kChildWatchdogSeconds, 0) + " s wall)" : "process exit before result record");
      put(r, "failure_detail", "exit " + std::to_string(res.exitCode));
      writeRecord(nodeDir / "result.txt", r);
    } else {
      ++withRecord;
    }
    summary << "node " << node << " exit=" << res.exitCode << " watchdog=" << res.timedOut << " wall=" << fmt(res.wall, 1)
            << " record=" << haveRecord << '\n';
    summary.flush();
  }
  const std::string saveAfter = fnvHex(normal);
  summary << "normal_save_before=" << saveBefore << " normal_save_after=" << saveAfter
          << " unchanged=" << (saveBefore == saveAfter) << " (report only)\n";
  const int expected = std::min(kNodes, (chunk + 1) * kChunkSize) - chunk * kChunkSize;
  summary << "visited=" << visited << " expected=" << expected << '\n';
  require(visited == expected, "every node of the chunk visited");
  for (int node = chunk * kChunkSize; node < chunk * kChunkSize + expected; ++node)
    require(fs::exists(chunkDir / ("node-" + pad3(node)) / "result.txt"), "result record for node " + std::to_string(node));
  std::cout << "chunk " << chunk << ": " << visited << " levels visited, " << withRecord << " child records, "
            << (expected - withRecord) << " parent-written failure records\n";
  return 0;
}

using Rec = std::map<std::string, std::string>;
std::string field(const Rec& r, const std::string& k, const std::string& d = "-") {
  const auto it = r.find(k);
  return it == r.end() || it->second.empty() ? d : it->second;
}

// 3x5 digit font for contact-sheet labels.
const char* kDigits[10] = {"111101101101111", "010110010010111", "111001111100111", "111001111001111", "101101111001001",
                           "111100111001111", "111100111101111", "111001001001001", "111101111101111", "111101111001111"};
void drawDigit(std::vector<std::uint8_t>& img, int W, int x, int y, int d, int scale) {
  for (int row = 0; row < 5; ++row)
    for (int col = 0; col < 3; ++col)
      if (kDigits[d][row * 3 + col] == '1')
        for (int sy = 0; sy < scale; ++sy)
          for (int sx = 0; sx < scale; ++sx) {
            auto* p = &img[(static_cast<std::size_t>(y + row * scale + sy) * W + x + col * scale + sx) * 4];
            p[0] = 255; p[1] = 255; p[2] = 0; p[3] = 255;
          }
}
void drawNumber(std::vector<std::uint8_t>& img, int W, int x, int y, int n) {
  // dark backing then yellow digits, scale 2
  const std::string s = std::to_string(n);
  for (int yy = 0; yy < 12; ++yy)
    for (int xx = 0; xx < static_cast<int>(s.size()) * 8 + 2; ++xx) {
      auto* p = &img[(static_cast<std::size_t>(y + yy) * W + x + xx) * 4];
      p[0] = p[1] = p[2] = 0; p[3] = 255;
    }
  for (std::size_t i = 0; i < s.size(); ++i) drawDigit(img, W, x + 1 + static_cast<int>(i) * 8, y + 1, s[i] - '0', 2);
}

int runReport(const fs::path& root) {
  const std::string gameDir = gameDirFromEnv();
  std::vector<Rec> rows(kNodes);
  int have = 0;
  for (int node = 0; node < kNodes; ++node) {
    const fs::path nodeDir = root / ("chunk-" + pad3(node / kChunkSize)) / ("node-" + pad3(node));
    auto rec = readRecord(nodeDir / "result.txt");
    for (auto& [k, v] : readRecord(nodeDir / "parent.txt")) rec["p_" + k] = v;
    rec["node"] = std::to_string(node);
    rec["dir"] = nodeDir.generic_string();
    have += fs::exists(nodeDir / "result.txt");
    rows[node] = std::move(rec);
  }
  // Identity
  {
    std::ofstream id(root / "identity.txt");
    id << "compiler=" << __VERSION__ << "\nsdl=" << SDL_MAJOR_VERSION << '.' << SDL_MINOR_VERSION << '.' << SDL_PATCHLEVEL
       << "\nlemmings.box_bytes=" << fs::file_size(fs::path(gameDir) / "lemmings.box")
       << "\npatch.box_bytes=" << fs::file_size(fs::path(gameDir) / "patch.box")
       << "\nframeSeconds=" << kFrameSeconds << "\nidle_after_release_s=" << kIdleAfterReleaseSeconds
       << "\nchild_watchdog_s=" << kChildWatchdogSeconds << "\nchunk_size=" << kChunkSize << '\n';
    const fs::path src = LS_SOURCE_DIR;
    for (const char* f : {"src/app/main.cpp", "src/app/BackdropScene.cpp", "src/app/AppEntry.h", "src/sim/World.cpp",
                          "src/sim/RetailLevel.cpp", "src/__test__/test_levelsweep.cpp"})
      id << "fnv1a64(" << f << ")=" << fnvHex(src / f) << '\n';
    id << "reference_exe_sha256=32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f (identity read from tools.windows.json; the EXE was not executed)\n";
  }
  // matrix.csv
  const std::vector<std::string> cols = {
      "node", "stored_id", "resolved_id", "resolve_match", "level_name", "outcome", "failing_stage", "load_result",
      "scenery", "total_lemmings", "save_target", "time_limit_s", "hatches", "exits", "spawned", "expected", "saved",
      "dead", "active", "final_states", "stop_reason", "world_finished", "timed_out", "end_phase", "skills",
      "teleports", "speedpads", "levers", "fluid_gates", "hazard_cells", "trampoline_cells", "antigrav_cells",
      "gadget_models", "settled_frame", "settled_verdict", "settled_distinct_view", "settled_modal_frac_view",
      "settled_sd_lum_view", "settled_black_frac_view", "final_verdict", "final_hash", "frames_observed", "sim_s",
      "run_wall_s", "child_wall_s", "p_child_exit_code", "p_child_watchdog_killed", "run_status", "release_interval_s",
      "expected_release_end_s", "failure_detail", "p_child_log_tail", "scenery_detail", "gate_link_diagnostic"};
  const auto csvq = [](std::string s) {
    std::string o = "\"";
    for (char c : s) o += c == '"' ? std::string("\"\"") : std::string(1, c);
    return o + "\"";
  };
  {
    std::ofstream csv(root / "matrix.csv");
    for (std::size_t i = 0; i < cols.size(); ++i) csv << (i ? "," : "") << cols[i];
    csv << '\n';
    for (const auto& r : rows) {
      for (std::size_t i = 0; i < cols.size(); ++i) csv << (i ? "," : "") << csvq(field(r,cols[i]));
      csv << '\n';
    }
  }
  // matrix.md (compact) and census.md (skills/gadgets)
  {
    std::ofstream md(root / "matrix.md");
    md << "| node | stored id | resolved | outcome | scenery | rel/exp | saved | dead | exits | hatches | frame (settled) | sim s | wall s |\n"
          "|---:|---|---|---|---|---:|---:|---:|---:|---:|---|---:|---:|\n";
    for (const auto& r : rows) {
      const std::string sc = field(r,"scenery");
      md << "| " << field(r,"node") << " | " << field(r,"stored_id") << " | " << field(r,"resolved_id") << " | " << field(r,"outcome")
         << (field(r,"outcome") == "completed" ? "" : " @" + field(r,"failing_stage")) << " | "
         << (sc.rfind("not-admitted", 0) == 0 ? "not admitted" : sc) << " | " << field(r,"spawned") << "/" << field(r,"expected")
         << " | " << field(r,"saved") << " | " << field(r,"dead") << " | " << field(r,"exits") << " | " << field(r,"hatches") << " | "
         << field(r,"settled_verdict") << " (" << field(r,"settled_distinct_view") << " col, mode " << field(r,"settled_modal_frac_view")
         << ") | " << field(r,"sim_s") << " | " << field(r,"run_wall_s") << " |\n";
    }
    std::ofstream cen(root / "census.md");
    cen << "| node | id | lemm | need | limit s | " << kSkillNames[0];
    for (int i = 1; i < 8; ++i) cen << " | " << kSkillNames[i];
    cen << " | tele | pad | lever | gate | hazard cells | tramp | antigrav | gadget models |\n|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|\n";
    for (const auto& r : rows) {
      std::string sk = field(r,"skills", "-,-,-,-,-,-,-,-");
      std::vector<std::string> parts;
      std::stringstream ss(sk);
      std::string t;
      while (std::getline(ss, t, ',')) parts.push_back(t);
      parts.resize(8, "-");
      cen << "| " << field(r,"node") << " | " << field(r,"resolved_id") << " | " << field(r,"total_lemmings") << " | " << field(r,"save_target")
          << " | " << field(r,"time_limit_s");
      for (const auto& p : parts) cen << " | " << p;
      cen << " | " << field(r,"teleports", "-") << " | " << field(r,"speedpads", "-") << " | " << field(r,"levers", "-") << " | " << field(r,"fluid_gates", "-")
          << " | " << field(r,"hazard_cells", "-") << " | " << field(r,"trampoline_cells", "-") << " | " << field(r,"antigrav_cells", "-") << " | "
          << field(r,"gadget_models") << " |\n";
    }
  }
  // Groups: by (outcome, failing stage) and by frame verdict, scenery admission, release shortfall, duplicate frames.
  {
    std::ofstream g(root / "groups.txt");
    const auto emit = [&](const std::string& title, const std::map<std::string, std::vector<int>>& m) {
      g << "## " << title << '\n';
      std::vector<std::pair<std::string, std::vector<int>>> v(m.begin(), m.end());
      std::sort(v.begin(), v.end(), [](const auto& a, const auto& b) { return a.second.size() > b.second.size(); });
      for (const auto& [k, nodes] : v) {
        g << nodes.size() << " levels: " << k << " -> nodes";
        for (int n : nodes) g << ' ' << n;
        g << '\n';
      }
    };
    std::map<std::string, std::vector<int>> byOutcome, byVerdict, byScenery, byRelease, byMatch, byStop, byDup, bySettle, byTimed, byFinal;
    std::map<std::string, std::vector<int>> hashes;
    for (const auto& r : rows) {
      const int n = std::stoi(field(r,"node"));
      byOutcome[field(r,"outcome") + " @ " + field(r,"failing_stage")].push_back(n);
      byVerdict[field(r,"settled_verdict", "no-settled-frame")].push_back(n);
      byScenery[field(r,"scenery")].push_back(n);
      byMatch[field(r,"resolve_match") + (field(r,"stored_id") == field(r,"resolved_id") ? " (stored==resolved)" : " (case differs)")].push_back(n);
      byStop[field(r,"stop_reason")].push_back(n);
      bySettle[field(r,"settled_frame") == "never" ? "settled never" : "settled"].push_back(n);
      byTimed[field(r,"timed_out") + "/finished=" + field(r,"world_finished")].push_back(n);
      byFinal[field(r,"final_verdict", "no-final-frame")].push_back(n);
      const std::string rel = field(r,"spawned", "?") == field(r,"expected", "!") ? "all released" : "release shortfall (spawned!=expected)";
      byRelease[rel].push_back(n);
      if (r.count("settled_hash")) hashes[field(r,"settled_hash")].push_back(n);
    }
    for (const auto& [h, nodes] : hashes)
      if (nodes.size() > 1) byDup["identical settled frame hash " + h].insert(byDup["identical settled frame hash " + h].end(), nodes.begin(), nodes.end());
    emit("outcome and earliest failing stage", byOutcome);
    emit("settled-frame verdict (view rows 0-319)", byVerdict);
    emit("final-frame verdict", byFinal);
    emit("scenery admission (driver-side BackdropScene::build)", byScenery);
    emit("resolution of stored map id", byMatch);
    emit("lemmings released vs expected", byRelease);
    emit("world finished/timed out", byTimed);
    emit("stop reason", byStop);
    emit("camera settled", bySettle);
    emit("duplicate settled frames", byDup);
  }
  // Contact sheets: 5 x 5 thumbnails (160x120) with a node-number label.
  int sheets = 0;
  const int perSheet = 25, cols5 = 5;
  for (int s = 0; s * perSheet < kNodes; ++s) {
    const int W = cols5 * 160, H = 5 * 120;
    Image sheet;
    sheet.width = W;
    sheet.height = H;
    sheet.rgba.assign(static_cast<std::size_t>(W) * H * 4, 0);
    for (std::size_t i = 3; i < sheet.rgba.size(); i += 4) sheet.rgba[i] = 255;
    for (int k = 0; k < perSheet; ++k) {
      const int node = s * perSheet + k;
      if (node >= kNodes) break;
      const int ox = (k % cols5) * 160, oy = (k / cols5) * 120;
      const fs::path thumb = fs::path(rows[node]["dir"]) / "thumb.rgb";
      std::ifstream in(thumb, std::ios::binary);
      std::vector<char> buf(160 * 120 * 3);
      const bool ok = in.read(buf.data(), static_cast<std::streamsize>(buf.size())).gcount() == static_cast<std::streamsize>(buf.size());
      for (int y = 0; y < 120; ++y)
        for (int x = 0; x < 160; ++x) {
          auto* p = &sheet.rgba[(static_cast<std::size_t>(oy + y) * W + ox + x) * 4];
          if (ok) {
            const char* q = &buf[(static_cast<std::size_t>(y) * 160 + x) * 3];
            p[0] = static_cast<std::uint8_t>(q[0]); p[1] = static_cast<std::uint8_t>(q[1]); p[2] = static_cast<std::uint8_t>(q[2]);
          } else {  // missing frame: magenta/black checker so it cannot pass for a dark level
            const bool chk = ((x / 8) + (y / 8)) & 1;
            p[0] = chk ? 255 : 0; p[1] = 0; p[2] = chk ? 255 : 0;
          }
        }
      drawNumber(sheet.rgba, W, ox + 2, oy + 2, node);
    }
    require(bool(writePng(root / ("contact-sheet-" + std::to_string(s) + ".png"), sheet)), "contact sheet written");
    ++sheets;
  }
  std::ofstream sum(root / "report-summary.txt");
  sum << "records=" << have << " expected=" << kNodes << " contact_sheets=" << sheets << '\n';
  std::cout << "report: " << have << "/" << kNodes << " level records, " << sheets << " contact sheets\n";
  require(have == kNodes, "all 102 level records present");
  // All 102 installed levels must load, finish the sample and render a nonblank settled view.
  std::string bad;
  for (int node = 0; node < kNodes; ++node) {
    auto& r = rows[node];
    if (r["outcome"] != "completed" || r["settled_verdict"] == "BLANK" || r["settled_verdict"] == "none")
      bad += " " + std::to_string(node) + ":" + r["stored_id"] + "=" + r["outcome"] + "/" + r["settled_verdict"];
  }
  if (!bad.empty()) throw std::runtime_error("level sweep failures (node:id=outcome/settled view):" + bad);
  return 0;
}

}  // namespace

int main(int argc, char** argv) try {
  if (argc != 4) { std::cerr << "usage: test_levelsweep chunk|level|report <index> <dir>\n"; return 2; }
  const std::string mode = argv[1];
  const int index = std::atoi(argv[2]);
  const fs::path root = argv[3];
  fs::create_directories(root);
  if (mode == "level") return runChild(index, root);
  if (mode == "chunk") return runChunk(index, root);
  if (mode == "report") return runReport(root);
  std::cerr << "unknown mode\n";
  return 2;
} catch (const std::exception& e) {
  std::cerr << e.what() << '\n';
  return 1;
}

#else
int main() { return 0; }
#endif
