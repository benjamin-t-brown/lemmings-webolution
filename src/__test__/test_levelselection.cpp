// level-selection-case-001 regression driver (SCRIPTED CLONE OBSERVATION, not human play, not original fidelity).
//
//   frontend <node> <dir>   ordinary startup through the PUBLIC webo::app::runApplication: bare invocation
//                           (+ --game-dir), SDL events queued from the read-only FrontendWindowObserver:
//                           click Play Current Game, click the map pixel of <node>, then watch the level
//                           observer. Nodes 0..2 run on a fresh (zero-progress) isolated save. Any other node
//                           needs a DIAGNOSTIC PREPARED SAVE FIXTURE in <dir>/<label>/save: the single earlier
//                           node whose completion flag makes <node> selectable is marked complete. That is a
//                           loading-test fixture only; it is not an earned unlock, win or natural play.
//                           Built twice: LSC_FROZEN=1 links the task-entry main.cpp/RetailLevel.cpp copies and
//                           must REPRODUCE the failure for a mixed-case node; the current build must enter the
//                           canonical level and compose frames. Node 0 (MM_CLIMB_002) must load in both.
//   census <dir>            (current build only) all 102 installed map spellings: resolve, load, verify identity,
//                           plus focused resolver checks (partial ids, missing, ambiguity, precedence).
//
// Nothing here mutates a World, actor, terrain, stock, flag or the normal/installed saves.
#include "app/AppEntry.h"
#include "app/MapScene.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
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
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

namespace {

[[maybe_unused]] int g_failures = 0;
[[maybe_unused]] void check(bool ok, const std::string& what) {
  if (!ok) { ++g_failures; std::cerr << "FAIL: " << what << '\n'; }
}

std::string upper(std::string s) {
  for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  return s;
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

std::string normalSaveState() {
  auto base = save::localAppDataDirectory();
  if (!base) return "unresolved";
  const auto p = save::cloneSavePathUnder(*base);
  std::error_code ec;
  if (!fs::exists(p, ec)) return "absent:" + p.string();
  return "present:" + p.string() + " bytes=" + std::to_string(fs::file_size(p, ec)) + " fnv=" + fnvHex(p);
}

#if WEBOLUTION_HAVE_SDL2
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

std::string flagList(const std::array<std::int32_t, 102>& flags) {
  std::string out;
  for (std::size_t i = 0; i < flags.size(); ++i)
    if (flags[i] != 0) out += (out.empty() ? "" : ",") + std::to_string(i);
  return out.empty() ? "none" : out;
}

// The only earlier node whose completion makes `node` selectable (state 3) on the map.
std::optional<std::size_t> unlockingNode(const AssetLibrary& lib, std::size_t node) {
  for (std::size_t i = 0; i < 102; ++i) {
    if (i == node) continue;
    std::array<std::int32_t, 102> flags{};
    flags[i] = 1;
    auto map = app::MapScene::build(lib, flags);
    if (map && map->states()[node] == 3) return i;
  }
  return std::nullopt;
}

int frontendMode(const AssetLibrary& lib, std::size_t node, const fs::path& root, const std::string& gameDir) {
#ifdef LSC_FROZEN
  const char* kBuild = "before";
#else
  const char* kBuild = "after";
#endif
  const std::string label = std::string(kBuild) + "-node" + std::to_string(node);
  const fs::path dir = root / label;
  std::error_code ec;
  fs::create_directories(dir, ec);
  const fs::path stderrLog = dir / "stderr.log";
  if (!std::freopen(stderrLog.string().c_str(), "w", stderr)) return 1;  // capture application diagnostics

  auto base = app::MapScene::build(lib, {});
  if (!base) { std::cerr << "map: " << base.error() << '\n'; return 1; }
  const auto expectedSpelling = base->levelForNode(node);
  if (!expectedSpelling) { std::cout << "node has no level\n"; return 1; }

  // Diagnostic prepared-save fixture (only for nodes the fresh map does not expose).
  std::array<std::int32_t, 102> prepared{};
  std::optional<std::size_t> unlock;
  const fs::path saveFile = save::cloneSavePathUnder(dir / "save");
  fs::remove(saveFile, ec);
  if (node >= 3) {
    unlock = unlockingNode(lib, node);
    if (!unlock) { std::cout << "no single node unlocks " << node << '\n'; return 1; }
    prepared[*unlock] = 1;
    save::SavedGame sg;
    const auto& mapping = app::MapScene::nodeMapping();
    sg.markComplete(mapping[*unlock][0], mapping[*unlock][1]);
    const auto bytes = sg.serialize();
    fs::create_directories(saveFile.parent_path(), ec);
    std::ofstream out(saveFile, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!out) { std::cout << "cannot write fixture\n"; return 1; }
  }
  // Never the installed or the normal save.
  const auto saveCanon = fs::weakly_canonical(saveFile, ec).string();
  const auto gameCanon = fs::weakly_canonical(fs::path(gameDir), ec).string();
  if (saveCanon.rfind(gameCanon, 0) == 0) { std::cout << "refusing: save path under the installed game dir\n"; return 1; }

  auto map = app::MapScene::build(lib, prepared);
  if (!map) { std::cerr << "map: " << map.error() << '\n'; return 1; }
  // Map pixel of the node: the matching pixel nearest the centroid of all its pixels.
  long long sx = 0, sy = 0, n = 0;
  for (int y = 0; y < 480; ++y)
    for (int x = 0; x < 640; ++x)
      if (auto p = map->pickNode(x, y); p && *p == node) { sx += x; sy += y; ++n; }
  if (n == 0) { std::cout << "node " << node << " is not selectable on the map\n"; return 1; }
  int px = -1, py = -1;
  long long best = -1;
  for (int y = 0; y < 480; ++y)
    for (int x = 0; x < 640; ++x)
      if (auto p = map->pickNode(x, y); p && *p == node) {
        const long long dx = x * n - sx, dy = y * n - sy, d = dx * dx + dy * dy;
        if (best < 0 || d < best) { best = d; px = x; py = y; }
      }

  struct Seen {
    int calls = 0, menuClicks = 0, lastPush = -100, mapSeenCall = -1, mapClicks = 0, clickCall = -1;
    bool map = false;
    std::array<std::int32_t, 102> flags{}, states{};
    std::optional<std::string> loadError, selected;
    int levelFrames = 0, firstFrame = -1, width = 0, height = 0, totalLemmings = 0, saveTarget = 0;
    std::size_t litPixels = 0;
    std::string levelName;
    bool paused = false;
  } seen;
  const auto frontend = [&](const app::FrontendSession& s, int) {
    ++seen.calls;
    if (seen.calls > 900) { pushQuit(); return; }
    const bool canPush = seen.calls - seen.lastPush >= 5;
    if (s.screen() == shell::Screen::Menu) {
      if (canPush && seen.menuClicks < 3) {
        ++seen.menuClicks; seen.lastPush = seen.calls; pushClick(kPlayCurrentX, kPlayCurrentY);
        seen.loadError = s.saveLoadError();
      } else if (canPush) pushQuit();
    } else if (s.screen() == shell::Screen::Map) {
      if (!seen.map) {
        seen.map = true; seen.mapSeenCall = seen.calls;
        seen.flags = s.completionFlags(); seen.states = s.mapStates(); seen.loadError = s.saveLoadError();
      }
      if (s.selectedLevel()) seen.selected = s.selectedLevel();
      if (canPush && seen.mapClicks < 1) {
        ++seen.mapClicks; seen.lastPush = seen.calls; seen.clickCall = seen.calls; pushClick(px, py);
      } else if (canPush && seen.calls - seen.clickCall > 60) pushQuit();
    } else {
      if (s.selectedLevel()) seen.selected = s.selectedLevel();
      pushQuit();
    }
  };
  const auto levelObserver = [&](const app::LevelFrameView& v) {
    if (seen.levelFrames == 0) {
      seen.firstFrame = v.frame;
      seen.levelName = std::string(v.levelName);
      seen.totalLemmings = v.world.setup().totalLemmings;
      seen.saveTarget = v.world.setup().saveTarget;
      seen.width = v.image.width;
      seen.height = v.image.height;
      seen.paused = v.paused;
    }
    ++seen.levelFrames;
    if (seen.levelFrames == 3) {
      for (std::size_t i = 0; i + 3 < v.image.rgba.size(); i += 4)
        if (v.image.rgba[i] || v.image.rgba[i + 1] || v.image.rgba[i + 2]) ++seen.litPixels;
    }
    if (seen.levelFrames >= 3) pushQuit();
  };

  const std::string normalBefore = normalSaveState();
  std::vector<std::string> args{"level-selection", "--game-dir", gameDir};  // ordinary startup: no diagnostics
  std::vector<char*> argv;
  for (auto& s : args) argv.push_back(s.data());
  app::ApplicationRunOptions options;  // frameSeconds 0: production wall clock; audio off; isolated save
  options.saveStore = std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(saveFile, fs::path(gameDir)));
  const int status = app::runApplication(static_cast<int>(argv.size()), argv.data(), frontend, levelObserver, options);
  std::fflush(stderr);
  const std::string normalAfter = normalSaveState();

  std::string log;
  {
    std::ifstream in(stderrLog);
    std::ostringstream os;
    os << in.rdbuf();
    log = os.str();
  }
  const std::string entering = "frontend: entering ";
  std::string enteredId = "-";
  if (const auto at = log.find(entering); at != std::string::npos) {
    const auto from = at + entering.size();
    const auto to = log.find(" from map", from);
    if (to != std::string::npos) enteredId = log.substr(from, to - from);
  }
  std::string firstError = "-";
  for (const char* key : {"level: ", "sim: ", "scene: ", "runtime HUD: "}) {
    const auto at = log.find(std::string("\n") + key);
    const auto start = at != std::string::npos ? at + 1 : (log.rfind(key, 0) == 0 ? 0 : std::string::npos);
    if (start == std::string::npos) continue;
    const auto end = log.find('\n', start);
    firstError = log.substr(start, end == std::string::npos ? std::string::npos : end - start);
    break;
  }

  const std::string canonical = upper(*expectedSpelling);
  const bool entered = seen.levelFrames > 0;
  const bool canonicalEntered = entered && seen.levelName == canonical;
  const bool mixedCase = *expectedSpelling != canonical;
  const bool flagsAsPrepared = seen.map && seen.flags == prepared;
  const bool normalUnchanged = normalBefore == normalAfter;

  std::ofstream res(dir / "result.txt");
  res << "build=" << kBuild << "\nnode=" << node << "\nmap_level_spelling=" << *expectedSpelling
      << "\ncanonical_archive_id=" << canonical << "\nmixed_case=" << (mixedCase ? 1 : 0)
      << "\ninvocation=runApplication(level-selection --game-dir <game>) frameSeconds=0 no diagnostic options"
      << "\nsave_file=" << saveFile.string()
      << "\nfixture=" << (unlock ? "DIAGNOSTIC PREPARED SAVE (loading test only; not earned progress): completion flag set on node " +
                                       std::to_string(*unlock) + " (group " + std::to_string(app::MapScene::nodeMapping()[*unlock][0]) +
                                       ", slot " + std::to_string(app::MapScene::nodeMapping()[*unlock][1]) + ")"
                                 : std::string("none (fresh zero-progress isolated save; nodes 0-2 are selectable at start)"))
      << "\nprepared_flags=" << flagList(prepared) << "\nflags_seen_on_map=" << (seen.map ? flagList(seen.flags) : "map not reached")
      << "\nflags_as_prepared=" << (flagsAsPrepared ? 1 : 0) << "\nsave_load_error=" << (seen.loadError ? *seen.loadError : "-")
      << "\nmap_state_of_node=" << (seen.map ? std::to_string(seen.states[node]) : "-")
      << "\nclick_pixel=" << px << "," << py << "\nmap_clicks=" << seen.mapClicks
      << "\nselected_level_state=" << (seen.selected ? *seen.selected : "-") << "\nselected_id_from_log=" << enteredId
      << "\nrunApplication_status=" << status << "\nlevel_frames_observed=" << seen.levelFrames
      << "\nobserved_level_name=" << (entered ? seen.levelName : "-") << "\ncanonical_level_entered=" << (canonicalEntered ? 1 : 0)
      << "\nframe_size=" << seen.width << "x" << seen.height << "\nlit_pixels_frame3=" << seen.litPixels
      << "\nworld_total_lemmings=" << seen.totalLemmings << "\nworld_save_target=" << seen.saveTarget
      << "\nfirst_error=" << firstError << "\nnormal_save_before=" << normalBefore << "\nnormal_save_after=" << normalAfter
      << "\nnormal_save_unchanged=" << (normalUnchanged ? 1 : 0) << '\n';
  res.close();

  std::cout << "LevelSelection[" << label << "] spelling=" << *expectedSpelling << " status=" << status
            << " entered=" << (canonicalEntered ? canonical : std::string("no")) << " frames=" << seen.levelFrames
            << " error=" << firstError << "\n";

  bool ok = seen.map && flagsAsPrepared && normalUnchanged && seen.mapClicks == 1;
#ifdef LSC_FROZEN
  // Frozen task-entry build: the mixed-case node must FAIL through the frontend with the lookup error and
  // never enter the simulation; every other node (e.g. MM_CLIMB_002) must still load.
  if (mixedCase) {
    ok = ok && !entered && status != 0 && enteredId == *expectedSpelling &&
         log.find("level: no .LVL matching " + *expectedSpelling) != std::string::npos;
    if (!ok) std::cerr << "frozen build did not reproduce the expected failure\n";
    return ok ? 0 : 2;
  }
#endif
  ok = ok && canonicalEntered && seen.litPixels > 0 && seen.width == 640 && seen.height == 480 && seen.totalLemmings > 0 &&
       (enteredId == *expectedSpelling);
  if (!ok) std::cerr << "level was not entered and rendered through the frontend\n";
  return ok ? 0 : 1;
}
#endif  // WEBOLUTION_HAVE_SDL2

#ifndef LSC_FROZEN
// ---- census + focused resolver checks (current build) -------------------------------------------------------------
int censusMode(const AssetLibrary& lib, const fs::path& root) {
  fs::create_directories(root);
  std::ofstream os(root / "census.txt");
  auto map = app::MapScene::build(lib, {});
  if (!map) { std::cerr << "map: " << map.error() << '\n'; return 1; }
  const auto names = levelFileNames(lib);
  std::set<std::string> canonicalPaths;
  for (const auto& n : names) canonicalPaths.insert(normaliseAssetName(n));
  os << "physical level entries=" << names.size() << " effective (case-insensitive) levels=" << canonicalPaths.size() << '\n';

  int exactSpellingHits = 0, mixed = 0, loaded = 0;
  std::set<std::string> distinct;
  for (std::size_t node = 0; node < 102; ++node) {
    const auto spelling = map->levelForNode(node);
    if (!spelling) { check(false, "node " + std::to_string(node) + " has no level"); continue; }
    const std::string expected = upper(*spelling);
    // independent expectation: the archive holds exactly this canonical path
    check(canonicalPaths.count("COMPILED\\" + expected + ".LVL") == 1, "archive holds COMPILED\\" + expected + ".LVL");
    bool legacyExact = false;
    for (const auto& n : names) if (n.find(*spelling) != std::string::npos) { legacyExact = true; break; }
    if (legacyExact) ++exactSpellingHits;
    if (*spelling != expected) ++mixed;
    auto resolved = sim::resolveRetailLevel(lib, *spelling);
    auto setup = sim::loadRetailLevel(lib, *spelling);
    const bool identity = resolved && upper(resolved->id) == expected &&
                          normaliseAssetName(resolved->file) == "COMPILED\\" + expected + ".LVL";
    if (setup) ++loaded;
    check(resolved.has_value(), "resolve " + *spelling + (resolved ? "" : ": " + resolved.error()));
    check(identity, "canonical identity of " + *spelling);
    check(setup.has_value(), "loadRetailLevel " + *spelling + (setup ? "" : ": " + setup.error()));
    distinct.insert(expected);
    os << "node " << node << " " << *spelling << " -> " << (resolved ? resolved->id : "FAIL") << " via "
       << (resolved ? (resolved->match == sim::LevelMatch::ExactId ? "exact-id"
                       : resolved->match == sim::LevelMatch::Substring ? "substring" : "case-insensitive") : "-")
       << " legacy_case_sensitive_match=" << (legacyExact ? 1 : 0) << " loaded=" << (setup ? 1 : 0) << '\n';
  }
  os << "map spellings=102 distinct canonical=" << distinct.size() << " mixed-case spellings=" << mixed
     << " legacy case-sensitive matches=" << exactSpellingHits << " loaded=" << loaded << '\n';
  check(distinct.size() == 102, "102 distinct canonical levels");
  check(loaded == 102, "all 102 map spellings load");
  check(mixed == 102 - exactSpellingHits, "spellings failing the legacy match are exactly the mixed-case ones");

  // Focused behaviour -----------------------------------------------------------------------------------------
  // Supported partial id (case-sensitive substring), archive-path form, and the default id.
  auto partial = sim::resolveRetailLevel(lib, "CLIMB_002");
  check(partial && partial->id == "MM_CLIMB_002" && partial->match == sim::LevelMatch::Substring, "partial id CLIMB_002 -> MM_CLIMB_002");
  auto pathForm = sim::resolveRetailLevel(lib, "\\MM_CLIMB_002.LVL");
  check(pathForm && pathForm->id == "MM_CLIMB_002", "archive-path form resolves");
  auto lower = sim::resolveRetailLevel(lib, "mm_climb_002");
  check(lower && lower->id == "MM_CLIMB_002" && lower->match == sim::LevelMatch::ExactId, "lower-case exact id");
  auto lowerPartial = sim::resolveRetailLevel(lib, "climb_002");
  check(lowerPartial && lowerPartial->id == "MM_CLIMB_002", "unambiguous lower-case partial id resolves case-insensitively");
  // Missing and empty ids fail rather than guess.
  check(!sim::resolveRetailLevel(lib, "NOT_A_REAL_LEVEL_ID"), "missing id fails");
  check(!sim::resolveRetailLevel(lib, "not_a_real_level_id"), "missing lower-case id fails");
  check(!sim::resolveRetailLevel(lib, ""), "empty id fails");
  check(!sim::loadRetailLevel(lib, "not_a_real_level_id"), "loadRetailLevel on a missing id fails");
  // Ambiguity: a lower-case substring that names many levels is an error, not the first hit.
  auto ambiguous = sim::resolveRetailLevel(lib, "mm_");
  check(!ambiguous && ambiguous.error().find("ambiguous") != std::string::npos, "ambiguous lower-case substring is rejected");
  os << "ambiguous 'mm_': " << (ambiguous ? "RESOLVED (unexpected)" : ambiguous.error().substr(0, 160)) << '\n';
  // Precedence: an id that is a substring of another level's id resolves to itself, for every such pair.
  int prefixPairs = 0;
  for (const auto& a : canonicalPaths)
    for (const auto& b : canonicalPaths) {
      if (a == b) continue;
      const auto ida = a.substr(a.find('\\') + 1, a.size() - a.find('\\') - 5);
      const auto idb = b.substr(b.find('\\') + 1, b.size() - b.find('\\') - 5);
      if (idb.find(ida) == std::string::npos) continue;
      ++prefixPairs;
      auto r = sim::resolveRetailLevel(lib, ida);
      check(r && r->id == ida, "exact id " + ida + " is not captured by longer id " + idb);
      auto rl = sim::resolveRetailLevel(lib, [&] { std::string t = ida; for (auto& c : t) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); return t; }());
      check(rl && rl->id == ida, "lower-case exact id " + ida + " is not captured by longer id " + idb);
    }
  os << "ids contained in another level's id (exact-id precedence pairs checked)=" << prefixPairs << '\n';
  // Archive canonical identity / patch precedence: every spelling of one path reads the same bytes, which are
  // what the resolved path reads (AssetLibrary::read is case-insensitive and patch.box wins).
  int spellingsChecked = 0;
  for (const auto& n : names) {
    auto direct = lib.read(n);
    auto viaUpper = lib.read(normaliseAssetName(n));
    check(direct && viaUpper && *direct == *viaUpper, "case variants of " + n + " read identical bytes");
    ++spellingsChecked;
  }
  std::map<std::string, int> spellings;
  for (const auto& n : names) ++spellings[normaliseAssetName(n)];
  int multi = 0;
  for (const auto& [key, count] : spellings) {
    if (count < 2) continue;
    ++multi;
    const auto id = key.substr(key.find('\\') + 1, key.size() - key.find('\\') - 5);
    auto r = sim::resolveRetailLevel(lib, id);
    check(r && normaliseAssetName(r->file) == key, "case-variant duplicates of " + key + " resolve to one level");
  }
  os << "archive spellings checked=" << spellingsChecked << " paths with more than one spelling=" << multi << '\n';
  os << "failures=" << g_failures << '\n';
  std::cout << "LevelSelection census: spellings=102 distinct=" << distinct.size() << " mixed-case=" << mixed
            << " loaded=" << loaded << " failures=" << g_failures << " -> " << (root / "census.txt").string() << '\n';
  return g_failures == 0 ? 0 : 1;
}
#endif

}  // namespace

int main(int argc, char** argv) {
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir) { std::cout << "LevelSelection: SKIPPED (set WEBOLUTION_GAME_DIR)\n"; return 0; }
  if (argc < 3) { std::cerr << "usage: test_levelselection frontend <node> <dir> | census <dir>\n"; return 1; }
  const std::string mode = argv[1];
  auto library = AssetLibrary::open(fs::path(gameDir) / "lemmings.box", fs::path(gameDir) / "patch.box");
  if (!library) { std::cerr << "assets: " << library.error() << '\n'; return 1; }
#ifndef LSC_FROZEN
  if (mode == "census") return censusMode(*library, fs::path(argv[2]));
#endif
#if WEBOLUTION_HAVE_SDL2
  if (mode == "frontend") {
    if (argc < 4) { std::cerr << "usage: test_levelselection frontend <node> <dir>\n"; return 1; }
    const char* video = std::getenv("SDL_VIDEODRIVER");
    if (!video || std::string(video) != "dummy") { std::cerr << "LevelSelection: refusing to run without SDL_VIDEODRIVER=dummy\n"; return 1; }
    const auto node = static_cast<std::size_t>(std::atoi(argv[2]));
    if (node >= 102) { std::cerr << "node must be 0..101\n"; return 1; }
    return frontendMode(*library, node, fs::path(argv[3]), gameDir);
  }
#endif
  std::cerr << "unknown mode " << mode << '\n';
  return 1;
}
