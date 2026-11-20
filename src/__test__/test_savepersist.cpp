// Check FrontendSession persistence against the recovered 0x45F43C/0x45F94D layout.
// Inject LevelOutcome through completeLevel and use temporary or in-memory stores.

#include "app/FrontendSession.h"

#include "assets/Box.h"
#include "save/SaveStore.h"
#include "save/SavedGame.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

namespace {
int checks = 0, failures = 0;
void check(bool ok, std::string_view what) {
  ++checks;
  if (!ok) { ++failures; std::cerr << "FAIL: " << what << '\n'; }
}

// Source-test sink whose state outlives the store, so a second session over the
// same SharedState is a "restart".
struct SharedState {
  std::optional<std::vector<std::byte>> bytes;
  bool failWrites = false;
  int reads = 0, writes = 0;
};
struct SharedSink final : save::SaveSink {
  explicit SharedSink(std::shared_ptr<SharedState> s) : state(std::move(s)) {}
  std::expected<std::optional<std::vector<std::byte>>, std::string> read() override {
    ++state->reads;
    return state->bytes;
  }
  std::expected<void, std::string> write(std::span<const std::byte> data) override {
    ++state->writes;
    if (state->failWrites) return std::unexpected("injected write failure");
    state->bytes = std::vector<std::byte>(data.begin(), data.end());
    return {};
  }
  std::shared_ptr<SharedState> state;
};
std::shared_ptr<save::SaveStore> storeOver(const std::shared_ptr<SharedState>& state) {
  return std::make_shared<save::SaveStore>(std::make_unique<SharedSink>(state));
}

std::vector<std::byte> readBytes(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  std::vector<std::byte> out;
  for (std::istreambuf_iterator<char> it(in), end; it != end; ++it) out.push_back(static_cast<std::byte>(*it));
  return out;
}
std::vector<std::size_t> differing(const std::vector<std::byte>& a, const std::vector<std::byte>& b) {
  std::vector<std::size_t> out;
  if (a.size() != b.size()) return {static_cast<std::size_t>(-1)};
  for (std::size_t i = 0; i < a.size(); ++i) if (a[i] != b[i]) out.push_back(i);
  return out;
}
const app::LevelOutcome kWin{"MM_CLIMB_002", 1, 1, 1, 10, true};
const app::LevelOutcome kLoss{"MM_CLIMB_002", 0, 10, 1, 0, false};
std::array<std::int32_t, 102> freshStates() {
  std::array<std::int32_t, 102> s;
  s.fill(1);
  s[0] = s[1] = s[2] = 3;
  return s;
}
bool newGame(app::FrontendSession& s) {
  // Row 0 is Start New Game / Yes; normalise focus because page replacement may reset it.
  while (s.menuFocus() != 0 && s.moveMenuFocus(-1)) {}
  if (s.pressReturn() != shell::Outcome::Handled || s.screen() != shell::Screen::Confirm) return false;
  while (s.menuFocus() != 0 && s.moveMenuFocus(-1)) {}
  return s.pressReturn() == shell::Outcome::Handled && s.screen() == shell::Screen::Map;
}
bool playCurrent(app::FrontendSession& s) {
  return s.screen() == shell::Screen::Menu && s.moveMenuFocus(1) &&
         s.pressReturn() == shell::Outcome::Handled && s.screen() == shell::Screen::Map;
}
}  // namespace

int main() {
  const char* configured = std::getenv("WEBOLUTION_GAME_DIR");
  if (!configured || !*configured) {
    std::cout << "SavePersist: SKIPPED (set WEBOLUTION_GAME_DIR to retail assets)\n";
    return 0;
  }
  const fs::path game(configured);
  auto library = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!library) { std::cerr << library.error() << '\n'; return 1; }
  const auto zeroImage = save::SavedGame{}.serialize();

  // Complete a level and reload progress in a fresh session from a temporary file.
  std::error_code ec;
  const fs::path root = fs::temp_directory_path(ec) / "webolution-savepersist-test";
  fs::remove_all(root, ec);
  const fs::path file = save::cloneSavePathUnder(root);
  const auto fileStore = [&] {
    return std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(file, game));
  };
  std::array<std::int32_t, 102> firstFlags{}, firstStates{};
  Image firstMapImage;
  {
    auto first = app::FrontendSession::create(*library, fileStore());
    check(first.has_value(), "session with an absent save file is created");
    if (!first) return 1;
    check(!first->saveLoadError() && first->saveCount() == 0 && !fs::exists(file) &&
          first->mapStates() == freshStates(), "absent file: empty progress, no write on create");
    check(newGame(*first), "New Game Yes reaches the map");
    check(first->saveCount() == 1 && readBytes(file) == zeroImage,
          "New Game Yes writes the 1504-byte zero image");
    check(first->clickMap(206, 204) && first->completeLevel(*library, kWin).has_value(),
          "SOURCE-TEST injected win is accepted by completeLevel");
    check(first->saveCount() == 2 && !first->saveError(), "a win saves exactly once after the commit");
    const auto bytes = readBytes(file);
    check(bytes.size() == 1504 && differing(bytes, zeroImage) == std::vector<std::size_t>{0x00C} &&
          bytes[0x00C] == std::byte{1}, "saved file differs from the zero image only at 0x00C");
    check(first->screen() == shell::Screen::Results && first->outcome() && first->completionFlags()[0] == 1,
          "results screen and in-memory progress are shown");
    check(first->click(320, 360) && first->screen() == shell::Screen::Map, "Continue returns to the map");
    check(first->saveCount() == 2, "Continue does not save");
    firstFlags = first->completionFlags();
    firstStates = first->mapStates();
    firstMapImage = first->image();
  }  // process "exit"
  {
    auto restarted = app::FrontendSession::create(*library, fileStore());
    check(restarted.has_value(), "restart over the saved file creates a session");
    if (!restarted) return 1;
    check(restarted->saveCount() == 0 && !restarted->saveLoadError(), "loading never writes");
    check(restarted->completionFlags() == firstFlags && restarted->mapStates() == firstStates,
          "restart: completionFlags and mapStates equal the in-memory result of the first run");
    check(playCurrent(*restarted) && restarted->image().rgba == firstMapImage.rgba,
          "restart: Play Current Game presents the same completed map image");
    // New Game must reset to the zero-flag map, not the loaded one.
    check(restarted->click(320, 455) && restarted->screen() == shell::Screen::Menu && newGame(*restarted),
          "New Game Yes is reachable after restart");
    check(restarted->completionFlags() == std::array<std::int32_t, 102>{} &&
          restarted->mapStates() == freshStates(), "New Game restores the zero-flag map, not the loaded one");
    check(restarted->saveCount() == 1 && readBytes(file) == zeroImage, "New Game Yes rewrites the zero image");
  }
  check(!fs::exists(file.string() + ".tmp"), "no temporary file remains");
  fs::remove_all(root, ec);

  // Loss and abandonment leave the save intact.
  {
    auto state = std::make_shared<SharedState>();
    auto s = app::FrontendSession::create(*library, storeOver(state));
    if (!s) return 1;
    check(newGame(*s) && s->saveCount() == 1 && state->writes == 1, "baseline: one write for New Game");
    check(s->clickMap(206, 204) && s->completeLevel(*library, kLoss).has_value(), "SOURCE-TEST injected loss");
    check(state->writes == 1 && s->saveCount() == 1 && s->completionFlags()[0] == 0, "a loss does not save");
    check(s->click(320, 360) && s->selectedLevel(),
          "retry re-arms the level without saving");
    check(state->writes == 1, "retry does not save");
    s->abandonLevelToMap();
    check(state->writes == 1, "writes stay at one after retry/abandon");
  }

  // A failed write keeps the session and results screen.
  {
    auto state = std::make_shared<SharedState>();
    auto s = app::FrontendSession::create(*library, storeOver(state));
    if (!s) return 1;
    check(newGame(*s), "failure scenario: New Game");
    const auto before = state->bytes;
    state->failWrites = true;
    check(s->clickMap(206, 204) && s->completeLevel(*library, kWin).has_value(),
          "a failing sink does not make completeLevel fail");
    check(s->saveError().has_value() && s->screen() == shell::Screen::Results && s->outcome() &&
          s->completionFlags()[0] == 1, "failed save reports an error and keeps the results and progress");
    check(state->bytes == before, "failed save leaves the prior good bytes intact");
  }

  // A corrupt file produces a visible diagnostic and an empty session without being overwritten.
  {
    auto state = std::make_shared<SharedState>();
    state->bytes = std::vector<std::byte>(1503, std::byte{0x5A});
    const auto corrupt = *state->bytes;
    auto s = app::FrontendSession::create(*library, storeOver(state));
    if (!s) return 1;
    check(s->saveLoadError().has_value() && s->mapStates() == freshStates() &&
          s->completionFlags() == std::array<std::int32_t, 102>{}, "corrupt file: empty session with a diagnostic");
    check(state->writes == 0 && *state->bytes == corrupt, "corrupt file is not overwritten by loading");
    check(playCurrent(*s) && state->writes == 0 && *state->bytes == corrupt,
          "nothing replaces the corrupt file until a save of new state");
    check(s->clickMap(206, 204) && s->completeLevel(*library, kWin).has_value() && state->writes == 1 &&
          state->bytes && state->bytes->size() == 1504, "a successful save then replaces it");
  }

  // Flag, A, B and unmapped records survive load, completion and save.
  {
    auto crafted = save::SavedGame{};
    crafted.flagRaw = 0x01020304;
    for (std::size_t n = 0; n < save::kRecordCount; ++n)
      crafted.records[n] = {static_cast<std::int32_t>(1000 + n), static_cast<std::int32_t>(-5 - static_cast<int>(n)), 0};
    crafted.records[save::SavedGame::recordIndex(4, 0)].c = 77;  // group 4 slot 0 is not a map node
    auto state = std::make_shared<SharedState>();
    state->bytes = crafted.serialize();
    auto s = app::FrontendSession::create(*library, storeOver(state));
    if (!s) return 1;
    check(!s->saveLoadError() && s->completionFlags() == std::array<std::int32_t, 102>{},
          "unmapped nonzero C does not create map progress");
    check(playCurrent(*s) && s->clickMap(206, 204) && s->completeLevel(*library, kWin).has_value(),
          "complete node 0 over a crafted image");
    auto expected = crafted;
    expected.markComplete(0, 0);
    check(state->bytes && *state->bytes == expected.serialize(),
          "only C of record 0 changed: flag, A, B and unmapped records preserved");
  }

  std::cout << "SavePersist: " << checks << " checks, " << failures << " failures\n";
  return failures == 0 ? 0 : 1;
}
