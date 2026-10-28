#include "app/FrontendSession.h"

#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "app/MapScene.h"
#include "shell/HudText.h"
#include "shell/MenuRender.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

namespace {
int checks = 0, failures = 0;
void check(bool ok, std::string_view what) {
  ++checks;
  if (!ok) { ++failures; std::cerr << "FAIL: " << what << '\n'; }
}

Image expectedResults(const Image& back, const Image& font,
                      const std::vector<std::string>& labels,
                      const std::vector<std::pair<std::string, int>>& lines) {
  // Independent assembly of expected composition: reuse the established low-level
  // menu/text primitives, but this is not a pixel oracle for the retail executable.
  shell::MenuParams buttons;
  buttons.baseY = 349;
  buttons.highlight = -1;
  Image expected = shell::renderMainMenu(back, font, labels, buttons, shell::fontCells());
  std::vector<shell::HudLine> hud;
  for (const auto& [text, y] : lines)
    hud.push_back({text, shell::labelLeft(text, shell::fontCells(), 640), y, {255, 0, 0}});
  shell::drawHud(expected, font, hud, shell::fontCells());
  return expected;
}

app::LevelOutcome outcome(std::string level, int saved, int total, int needed, bool win) {
  return {std::move(level), saved, total, needed, 0, win};
}
}

int main() {
  for (bool success : {false, true}) {
    shell::Frontend state;
    state.press(shell::Key::Return); state.press(shell::Key::Return);
    state.press(shell::Key::MapNodeClick);
    check(state.showResults(success) == shell::Outcome::Handled &&
          state.screen() == shell::Screen::Results && state.uiState() == (success ? 4 : 5) &&
          state.nodeVtable() == (success ? shell::kVtableWinResults : shell::kVtableLossResults),
          "win/loss use distinct READ result UI states and vtables");
    check(state.press(shell::Key::Return) == shell::Outcome::Unobserved &&
          state.screen() == shell::Screen::Results, "no invented initial Return-to-Continue focus");
    check(state.activateResultsContinue() == shell::Outcome::Handled &&
          state.screen() == shell::Screen::Map, "both explicit Continue actions select map");
  }
  {
    shell::Frontend state;
    check(state.activateResultsQuit() == shell::Outcome::Ignored && state.screen() == shell::Screen::Menu,
          "Quit activation outside results is ignored");
    state.press(shell::Key::Return); state.press(shell::Key::Return);
    state.press(shell::Key::MapNodeClick);
    state.showResults(true);
    check(state.activateResultsQuit() == shell::Outcome::Ignored && state.screen() == shell::Screen::Results,
          "Quit activation on win results is ignored");
    state.activateResultsContinue(); state.press(shell::Key::MapNodeClick);
    state.showResults(false);
    check(state.activateResultsQuit() == shell::Outcome::Handled && state.screen() == shell::Screen::Map,
          "Quit activation on loss results returns to map");
  }
  const char* configured = std::getenv("WEBOLUTION_GAME_DIR");
  if (!configured || !*configured) {
    std::cout << "Results: SKIPPED (set WEBOLUTION_GAME_DIR to retail assets)\n";
    return failures ? 1 : 0;
  }
  const fs::path game(configured);
  auto library = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!library) { std::cerr << library.error() << '\n'; return 1; }
  auto back = loadTexture(*library, "TEXTURES\\SMALL_BACK.BMP");
  auto font = loadTexture(*library, "TEXTURES\\^FONT.BMP");
  auto strings = loadStrings(*library, "UK");
  check(back && font && strings, "retail result assets load");
  if (!back || !font || !strings) return 1;
  const auto text = [&](int index) { return std::string((*strings)[index]); };

  using app::ResultAction;
  const auto win = outcome("MM_CLIMB_002", 12, 20, 10, true);
  const auto loss = outcome("MM_CLIMB_002", 9, 20, 10, false);
  auto winScene = app::ResultsScene::build(*library, win);
  auto lossScene = app::ResultsScene::build(*library, loss);
  check(winScene && lossScene, "both result outcomes build");
  if (!winScene || !lossScene) return 1;
  const auto winExpected = expectedResults(*back, *font, {text(47)}, {
    {text(50), 220}, {text(121) + " 10", 280}, {text(122) + " 12", 310}});
  const auto lossTitle = text(49);
  const auto split = lossTitle.find('^');
  const auto lossExpected = expectedResults(*back, *font,
    {text(48), text(47), text(43)}, {
      {lossTitle.substr(0, split), 220}, {lossTitle.substr(split + 1), 249},
      {text(121) + " 10", 280}, {text(122) + " 9", 310}});
  check(winScene->image().width == 640 && winScene->image().height == 480,
        "results use 640x480 canvas");
  check(winScene->image().rgba == winExpected.rgba,
        "win image equals independently assembled expected composition (shared primitives, not native oracle)");
  check(lossScene->image().rgba == lossExpected.rgba,
        "loss image equals independently assembled expected composition (shared primitives, not native oracle)");
  check(winScene->image().rgba != lossScene->image().rgba, "win and loss render differently");
  auto changedCount = app::ResultsScene::build(*library, outcome("MM_CLIMB_002", 11, 20, 10, true));
  check(changedCount && changedCount->image().rgba != winScene->image().rgba,
        "changing saved count changes rendered image");
  auto changedNeeded = app::ResultsScene::build(*library, outcome("MM_CLIMB_002", 12, 20, 11, true));
  check(changedNeeded && changedNeeded->image().rgba != winScene->image().rgba,
        "changing needed count changes rendered image");
  check(winScene->actionAt(320, 360) == ResultAction::Continue &&
        winScene->actionAt(320, 330) == ResultAction::None,
        "win exposes Continue at row1 and leaves row0 blank");
  check(lossScene->actionAt(320, 360) == ResultAction::Retry &&
        lossScene->actionAt(320, 390) == ResultAction::Continue &&
        lossScene->actionAt(320, 420) == ResultAction::Quit &&
        lossScene->actionAt(320, 330) == ResultAction::None,
         "loss exposes Retry, Continue, Quit and no blank row action");
  check(winScene->actionAt(320, 420) == ResultAction::None &&
        winScene->actionAt(320, 390) == ResultAction::None,
        "win results expose no loss-only actions");

  auto make = [&]() -> std::optional<app::FrontendSession> {
    auto session = app::FrontendSession::create(*library);
    if (!session) check(false, session.error());
    return session ? std::optional<app::FrontendSession>(std::move(*session)) : std::nullopt;
  };
  using shell::Screen;
  auto guards = make();
  if (!guards) return 1;
  const auto guardFrame = guards->image().rgba;
  check(!guards->retryLevel() && !guards->abandonLevelToMap() &&
        guards->screen() == Screen::Menu && !guards->selectedLevel() && !guards->outcome() &&
        guards->levelIntroEnabled() && guards->image().rgba == guardFrame,
        "in-level retry and abandon are ignored on a fresh menu without side effects");

  auto s = make();
  if (!s) return 1;
  check(s->levelIntroEnabled(), "level intro preference starts enabled");
  s->pressReturn(); s->pressReturn();
  check(s->screen() == Screen::Map && s->clickMap(206, 204) &&
        s->selectedLevel() == std::optional<std::string>{"MM_CLIMB_002"},
        "menu Return route selects expected map node");
  auto initialFlags = s->completionFlags();
  auto wrong = s->completeLevel(*library, outcome("WRONG_LEVEL", 12, 20, 10, true));
  check(!wrong && s->screen() == Screen::Level && s->completionFlags() == initialFlags &&
        s->selectedLevel() == std::optional<std::string>{"MM_CLIMB_002"},
        "wrong-level outcome rejected without changing state");
  auto inconsistent = s->completeLevel(*library, outcome("MM_CLIMB_002", 21, 20, 10, true));
  check(!inconsistent && s->screen() == Screen::Level && s->completionFlags() == initialFlags &&
        s->selectedLevel() == std::optional<std::string>{"MM_CLIMB_002"},
        "inconsistent result rejected without changing state");
  auto completed = s->completeLevel(*library, win);
  check(completed && s->screen() == Screen::Results && s->completionFlags()[0] == 1 &&
        !s->selectedLevel() && s->levelIntroEnabled(), "win UI4 results sets completion before Continue");
  auto duplicate = s->completeLevel(*library, win);
  check(!duplicate && s->screen() == Screen::Results && s->completionFlags()[0] == 1,
        "duplicate completion outside level rejected");
  check(s->click(320, 360) && s->screen() == Screen::Map &&
        s->completionFlags()[0] == 1 && s->levelIntroEnabled(), "win Continue routes to map and retains completion/intro preference");
  check(s->clickMap(206, 204) && s->selectedLevel() == std::optional<std::string>{"MM_CLIMB_002"},
        "completed map node can be selected for another run");
  check(s->completeLevel(*library, loss) && s->screen() == Screen::Results &&
        s->completionFlags()[0] == 1, "loss UI5 preserves prior completion flag");
  check(s->click(320, 360) && s->screen() == Screen::Level &&
        s->selectedLevel() == std::optional<std::string>{"MM_CLIMB_002"} &&
        s->completionFlags()[0] == 1 && !s->levelIntroEnabled(), "Retry retains progress and disables intro");
  check(s->completeLevel(*library, loss) && s->screen() == Screen::Results &&
        !s->outcome()->success && s->completionFlags()[0] == 1 && !s->levelIntroEnabled(),
        "retry loss retains prior completion and disabled intro preference");
  check(s->click(320, 420) && s->screen() == Screen::Map && s->completionFlags()[0] == 1 &&
        !s->selectedLevel() && !s->outcome() && s->levelIntroEnabled(),
        "loss Quit returns to map, retires selection/outcome, retains flags, and enables intro");
  check(!s->completeLevel(*library, loss) && s->screen() == Screen::Map,
        "stale completion after Quit is rejected");
  auto unlockedMap = app::MapScene::build(*library, s->completionFlags());
  bool selectedDifferentNode = false;
  std::optional<std::size_t> expectedDifferentNode;
  std::optional<std::string> expectedDifferentLevel;
  if (unlockedMap) {
    for (int y = 0; y < 480 && !selectedDifferentNode; ++y) {
      for (int x = 0; x < 640 && !selectedDifferentNode; ++x) {
        const auto node = unlockedMap->pickNode(x, y);
        if (!node || *node == 0) continue;
        const auto expectedLevel = unlockedMap->levelForNode(*node);
        if (!expectedLevel) continue;
        if (s->clickMap(x, y) && s->selectedLevel() == expectedLevel && s->levelIntroEnabled()) {
          selectedDifferentNode = true;
          expectedDifferentNode = *node;
          expectedDifferentLevel = *expectedLevel;
        }
      }
    }
  }
  check(selectedDifferentNode && expectedDifferentNode && expectedDifferentLevel &&
        *expectedDifferentNode != 0 && s->selectedLevel() == expectedDifferentLevel,
        "fresh unlocked different-node selection establishes correct ownership and enables intro");
  if (!expectedDifferentNode || !expectedDifferentLevel) return 1;
  const auto priorCompletionFlags = s->completionFlags();
  check(s->retryLevel() && s->screen() == Screen::Level &&
        s->selectedLevel() == expectedDifferentLevel && !s->outcome() &&
        s->completionFlags() == priorCompletionFlags && !s->levelIntroEnabled(),
        "retry keeps different-node ownership and prior progress, clears result, and disables intro");
  auto differentNodeWin = outcome(*expectedDifferentLevel, 12, 20, 10, true);
  check(s->completeLevel(*library, differentNodeWin) && s->screen() == Screen::Results &&
        s->completionFlags()[*expectedDifferentNode] == 1 && s->completionFlags()[0] == 1,
        "completion after retry awards the explicitly selected different node, not the old node");

  auto completedNode = make();
  if (!completedNode) return 1;
  completedNode->pressReturn(); completedNode->pressReturn();
  check(completedNode->clickMap(206, 204) && completedNode->completeLevel(*library, win) &&
        completedNode->completionFlags()[0] == 1 && completedNode->click(320, 360) &&
        completedNode->screen() == Screen::Map,
        "completed-node fixture wins and continues to map with its flag retained");
  const auto completedMapFrame = completedNode->image().rgba;
  check(completedNode->clickMap(206, 204) && completedNode->selectedLevel() ==
        std::optional<std::string>{"MM_CLIMB_002"} && completedNode->levelIntroEnabled(),
        "previously completed node can be re-entered with map activation enabling intro");
  const auto completedFlags = completedNode->completionFlags();
  check(completedNode->retryLevel() && completedNode->screen() == Screen::Level &&
        completedNode->selectedLevel() == std::optional<std::string>{"MM_CLIMB_002"} &&
        completedNode->completionFlags() == completedFlags && !completedNode->outcome() &&
        !completedNode->levelIntroEnabled(),
        "retry on previously completed node preserves its flag without inventing an outcome");
  check(completedNode->abandonLevelToMap() && completedNode->screen() == Screen::Map &&
        !completedNode->selectedLevel() && !completedNode->outcome() &&
        completedNode->completionFlags() == completedFlags && !completedNode->levelIntroEnabled() &&
        completedNode->image().rgba == completedMapFrame,
        "abandon after retry returns to preserved map image without changing progress or intro preference");
  check(!completedNode->completeLevel(*library, win) && completedNode->screen() == Screen::Map &&
        completedNode->completionFlags() == completedFlags && !completedNode->outcome(),
        "stale completion after in-level abandon is rejected without new flags or outcome");
  check(completedNode->clickMap(206, 204) && completedNode->levelIntroEnabled(),
        "later map activation enables intro after abandon preserved the disabled preference");

  auto fresh = make();
  if (!fresh) return 1;
  fresh->pressReturn(); fresh->pressReturn(); fresh->clickMap(206, 204);
  check(fresh->completeLevel(*library, loss) && fresh->completionFlags()[0] == 0,
        "fresh failure does not unlock completion flag");
  check(fresh->click(320, 420) && fresh->screen() == Screen::Map && !fresh->selectedLevel() &&
        !fresh->outcome() && fresh->completionFlags()[0] == 0 && fresh->levelIntroEnabled(),
        "loss Quit routes to map without stale selection/outcome and retains flags/preference");
  check(fresh->clickMap(206, 204) && fresh->selectedLevel() == std::optional<std::string>{"MM_CLIMB_002"} &&
        fresh->levelIntroEnabled(), "fresh node selection establishes selection and enables intro");
   check(fresh->completeLevel(*library, loss) && fresh->click(320, 360) &&
         fresh->completeLevel(*library, loss) && !fresh->levelIntroEnabled() &&
         fresh->click(320, 390) && fresh->screen() == Screen::Map && !fresh->levelIntroEnabled(),
         "loss Continue remains distinct and does not write intro preference");
   check(fresh->clickMap(206, 204) && fresh->levelIntroEnabled(),
         "map selection re-enables intro after retry/loss Continue");
  check(fresh->completeLevel(*library, outcome("MM_CLIMB_002", 2, 10, 1, false)) &&
        fresh->screen() == Screen::Results && !fresh->outcome()->success &&
        fresh->outcome()->saved == 2 && fresh->completionFlags()[0] == 0,
        "late rescues can meet target without upgrading latched timeout loss");
  fresh->click(320, 390);
  fresh->clickMap(206, 204);
  check(fresh->completeLevel(*library, win) && fresh->completionFlags()[0] == 1,
        "fresh session wins selected node");
  check(fresh->click(320, 360) && fresh->screen() == Screen::Map,
        "return to map after fresh win");
  check(fresh->click(320, 455) && fresh->screen() == Screen::Menu,
        "map footer returns to menu");
  check(fresh->click(320, 333) && fresh->screen() == Screen::Confirm &&
        fresh->click(320, 340) && fresh->screen() == Screen::Menu && fresh->completionFlags()[0] == 1,
        "New Game No retains completion flags");
  check(fresh->click(320, 333) && fresh->screen() == Screen::Confirm &&
        fresh->click(320, 310) && fresh->screen() == Screen::Map &&
        fresh->completionFlags()[0] == 0,
        "new game via footer/menu/confirm resets owned flags and rebuilds initial map");

  std::cout << checks << " checks, " << failures << " failures\n";
  return failures ? 1 : 0;
}
