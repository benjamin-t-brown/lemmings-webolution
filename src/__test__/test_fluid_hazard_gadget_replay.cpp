// Replay the two Blocker placements from the historical MM_W_012 WIN 11/20:
// build/fluid-hazard-001/route-regressions/20261114-062255/step-4-gadget/action-trace.csv.
// empty-receiver-fall-001: that win depended on walking across dry receivers.
// The same placements must now lose enough actors to make winning impossible.
// This supplements the unchanged wall-clock assessment driver. Only SDL inputs
// are written; World, stock, progress and clocks are never mutated by callbacks.
#include "app/AppEntry.h"
#include "app/MapScene.h"
#include "assets/Png.h"
#include "sim/World.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;
namespace {
int failures = 0;
void check(bool ok, const std::string& why) {
  if (!ok) { ++failures; std::cerr << "FAIL: " << why << '\n'; }
}
std::string bytes(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}
struct PreservedFile {
  fs::path path;
  bool existed;
  std::string data;
  explicit PreservedFile(fs::path p)
      : path(std::move(p)), existed(fs::exists(path)), data(bytes(path)) {}
  bool unchanged() const { return existed == fs::exists(path) && data == bytes(path); }
};
void click(int x, int y) {
  SDL_Event event{};
  event.type = SDL_MOUSEBUTTONDOWN;
  event.button.button = SDL_BUTTON_LEFT;
  event.button.state = SDL_PRESSED;
  event.button.x = x;
  event.button.y = y;
  check(SDL_PushEvent(&event) == 1, "queue public click");
}
void quit() {
  SDL_Event event{};
  event.type = SDL_QUIT;
  check(SDL_PushEvent(&event) == 1, "queue quit");
}
}

int main(int argc, char** argv) {
  if (argc != 3 || !std::getenv("WEBOLUTION_GAME_DIR")) return 2;
  const fs::path seed = argv[1], out = argv[2], game = std::getenv("WEBOLUTION_GAME_DIR");
  const auto local = save::localAppDataDirectory();
  if (!local || !fs::exists(seed)) return 2;
  fs::create_directories(out);
  PreservedFile installed(game / "SavedGame.dat"), normal(save::cloneSavePathUnder(*local));
  const auto privateSave = out / "save/LemmingsWebolution/SavedGame.dat";
  fs::create_directories(privateSave.parent_path());
  fs::copy_file(seed, privateSave, fs::copy_options::overwrite_existing);
  auto store = std::make_shared<save::SaveStore>(
      std::make_unique<save::FileSaveSink>(privateSave, game));
  auto initial = store->load();
  check(bool(initial), "load earned private seed");
  std::ofstream trace(out / "replay.csv");
  trace << "frame,ms,tick,index,x,y,state,saved,dead,action\n";
  std::vector<std::string> args{"fluid-hazard-gadget-replay", "--game-dir", game.string(),
                                "--frontend", "--frames", "3000"};
  std::vector<char*> raw;
  for (auto& arg : args) raw.push_back(arg.data());
  app::ApplicationRunOptions options;
  // The existing bounded integration-test clock makes input sampling repeatable.
  // Production simulation/release/skill rules and fast-forward passes are unchanged.
  options.frameSeconds = 0.02;
  options.saveStore = store;
  bool entered = false, ff = false, selected = false, results = false, won = false, returned = false;
  bool menuClick = false, mapClick = false;
  bool doomed = false, drySplat = false;
  int placements = 0, saved = -1, dead = -1, resultSaved = -1;
  std::array<bool, 2> blockerSeen{};
  std::array<std::uint64_t, 2> attachTicks{};
  const auto frontend = [&](const app::FrontendSession& session, int) {
    if (session.screen() == shell::Screen::Menu && !menuClick) {
      menuClick = true;
      click(320, 363);
    } else if (session.screen() == shell::Screen::Map) {
      if (results) {
        returned = true;
        check(session.completionFlags()[8] == 1, "win persists node 8 completion");
        check(session.saveCount() == 1 && !session.saveError(), "exactly one successful private save");
        check(writePng(out / "map-after.png", session.composeFrame()).has_value(), "retain completed map");
        quit();
      } else if (!mapClick) {
        mapClick = true;
        check(session.completionFlags()[0] == 1 && session.completionFlags()[3] == 1 &&
              session.completionFlags()[8] == 0 && session.mapStates()[8] == 3,
              "earned prerequisite wins unlock the uncompleted gadget level");
        click(273, 195);  // same node-8 map pixel as the recorded route
      }
    } else if (session.screen() == shell::Screen::Results && !results) {
      results = true;
      if (session.outcome()) {
        resultSaved = session.outcome()->saved;
        won = session.outcome()->success;
        check(session.outcome()->total == 20 && session.outcome()->required == 10, "installed result target");
      }
      check(writePng(out / "results.png", session.composeFrame()).has_value(), "retain result pixels");
      click(320, 360);
    }
  };
  const auto observe = [&](const app::LevelFrameView& frame) {
    entered = true;
    check(frame.levelName == "MM_W_012", "production frontend enters installed gadget level");
    const auto& world = frame.world;
    saved = world.saved();
    dead = world.dead();
    check(std::none_of(world.lemmings().begin(),world.lemmings().end(),
          [](const auto& l){return l.state==sim::State::Drown;}),
          "water species does not drown on this route");
    for(const auto& l:world.lemmings())
      drySplat |= l.state==sim::State::Splatter && l.y>=90;
    if(!doomed && blockerSeen[0] && blockerSeen[1] && world.dead()>10 && saved==0) {
      doomed=true;
      check(drySplat,"loss follows a fall to a dry receiver floor");
      check(std::all_of(world.setup().gates.begin(),world.setup().gates.end(),
            [](const auto& g){return g.minY!=4||g.height==0;}),
            "recorded no-lever route leaves all lower receivers empty");
      check(writePng(out/"dry-pit-loss.png",frame.image).has_value(),"retain inevitable loss frame");
      quit(); // Public close once fewer than the required ten can survive.
      return;
    }
    // These two actor IDs/edges are the recorded winning route. Do not place an
    // earlier actor as the adaptive wall-clock driver sometimes does on faster runs.
    const auto emit = [&](std::size_t index, const char* action) {
      if (index >= world.lemmings().size()) return;
      const auto& actor = world.lemmings()[index];
      trace << frame.frame << ',' << world.levelClockMs() << ',' << world.tickCount() << ','
            << index << ',' << actor.x << ',' << actor.y << ',' << sim::stateName(actor.state)
            << ',' << saved << ',' << dead << ',' << action << '\n';
    };
    for (std::size_t n = 0; n < 2; ++n) {
      const auto index = 11 + n;
      if (index < world.lemmings().size() && world.lemmings()[index].state == sim::State::Blocker) {
        if (!blockerSeen[n]) {
          emit(index, "blocker-observed");
          const auto& actor = world.lemmings()[index];
          check(actor.x == (n == 0 ? 214 : 165) && actor.y == 41,
                "Blocker attaches at the exact recorded winning site");
          attachTicks[n] = world.tickCount();
        }
        blockerSeen[n] = true;
      }
      emit(index, "observe");
    }
    if (frame.cameraIntroActive || frame.endPhase != app::LevelEndPhase::Running) return;
    if (!ff) {
      ff = true;
      click(54, 348);  // recorded public fast-forward plate
      return;
    }
    if (!selected) {
      selected = true;
      SDL_Event event{};
      event.type = SDL_KEYDOWN;
      event.key.keysym.sym = SDLK_4;
      event.key.keysym.scancode = SDL_SCANCODE_4;
      check(SDL_PushEvent(&event) == 1, "select Blocker by public key");
      return;
    }
    if (placements >= 2 || (placements == 1 && !blockerSeen[0])) return;
    const std::size_t index = 11 + placements;
    if (index >= world.lemmings().size() || frame.selectedSkillIndex != 3) return;
    const auto& actor = world.lemmings()[index];
    if (actor.state != sim::State::Walker || actor.y != 41) return;
    // Same base-policy edge window (distance 4..8) as blockerIntent. The trace
    // pins the actual sampled placement; no actor position is injected.
    const bool atSite = placements == 0 ?
        actor.facing == sim::Facing::Right && actor.x >= 214 && actor.x <= 218 :
        actor.facing == sim::Facing::Left && actor.x >= 161 && actor.x <= 165;
    if (!atSite) return;
    const auto view = std::find_if(frame.actors.begin(), frame.actors.end(),
                                  [&](const auto& a) { return a.index == index; });
    if (view == frame.actors.end()) return;
    click(int((view->placement.left + view->placement.right) / 2),
          int((view->placement.top + view->placement.bottom) / 2));
    emit(index, "click-blocker");
    ++placements;
    check(writePng(out / ("placement-" + std::to_string(placements) + ".png"), frame.image).has_value(),
          "retain placement pixels");
  };
  const int status = app::runApplication(int(raw.size()), raw.data(), frontend, observe, options);
  check(status == 0 && entered && doomed, "public no-lever route can no longer rescue the required ten");
  check(placements == 2 && blockerSeen[0] && blockerSeen[1], "both recorded actor placements take effect");
  check(!won && saved == 0 && dead > 10, "historical dry-pit WIN becomes an inevitable loss");
  // Read the file through a fresh store, then compare the complete codec image:
  // the failed route must preserve the pinned earned seed completely.
  save::SaveStore reopened(std::make_unique<save::FileSaveSink>(privateSave, game));
  const auto persisted = reopened.load();
  check(bool(persisted), "reopen private save after the losing route");
  if (initial && persisted) {
    auto expected = *initial;
    check(expected.serialize() == persisted->serialize(), "losing route preserves private progress");
  }
  check(installed.unchanged() && normal.unchanged(), "installed and normal saves unchanged");
  std::ofstream(out / "summary.txt") << "failures=" << failures << "\nstatus=" << status
      << "\nwon=" << won << "\ndoomed=" << doomed << "\ndry_splat=" << drySplat << "\nsaved=" << saved << "\nresult_saved=" << resultSaved << "\ndead=" << dead
      << "\nplacements=" << placements << "\nreturned_to_map=" << returned
      << "\nblocker_11_attach_tick=" << attachTicks[0] << "\nblocker_12_attach_tick=" << attachTicks[1]
      << "\ninstalled_save_unchanged=" << installed.unchanged()
      << "\nnormal_save_unchanged=" << normal.unchanged() << '\n';
  return failures ? 1 : 0;
}
