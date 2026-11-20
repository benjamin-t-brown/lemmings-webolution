// Compare session restart with and without a FileSaveSink in a temporary directory.
// Create a new game, select MM_CLIMB_002, submit a synthetic winning LevelOutcome
// through completeLevel, then open a fresh session and check progress.

#include "app/FrontendSession.h"

#include "assets/Box.h"
#include "save/SaveStore.h"
#include "save/SavedGame.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

namespace {
#ifdef SAVEDEMO_BEFORE
constexpr bool kBefore = true;
constexpr const char* kMode = "before";
#else
constexpr bool kBefore = false;
constexpr const char* kMode = "after";
#endif

std::expected<app::FrontendSession, std::string> openSession(const AssetLibrary& library,
                                                              const fs::path& file,
                                                              const fs::path& game) {
#ifdef SAVEDEMO_BEFORE
  (void)file;
  (void)game;
  return app::FrontendSession::create(library);
#else
  return app::FrontendSession::create(
      library, std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(file, game)));
#endif
}

std::vector<std::byte> readBytes(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  std::vector<std::byte> out;
  for (std::istreambuf_iterator<char> it(in), end; it != end; ++it) out.push_back(static_cast<std::byte>(*it));
  return out;
}

bool newGame(app::FrontendSession& s) {
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
    std::cout << "SaveDemo[" << kMode << "]: SKIPPED (set WEBOLUTION_GAME_DIR to retail assets)\n";
    return 0;
  }
  const fs::path game(configured);
  auto library = AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!library) { std::cerr << library.error() << '\n'; return 1; }

  std::error_code ec;
  const fs::path root = fs::temp_directory_path(ec) / (std::string("webolution-savedemo-") + kMode);
  fs::remove_all(root, ec);
  const fs::path file = save::cloneSavePathUnder(root);
  const auto zeroImage = save::SavedGame{}.serialize();
  const app::LevelOutcome win{"MM_CLIMB_002", 1, 1, 1, 10, true};  // SOURCE-TEST setup

  std::ostringstream report;
  int failures = 0;
  const auto observe = [&](bool ok, const std::string& what) {
    report << (ok ? "ok   " : "FAIL ") << what << '\n';
    if (!ok) ++failures;
  };

  std::array<std::int32_t, 102> firstFlags{}, firstStates{};
  Image firstMapImage;
  {
    auto first = openSession(*library, file, game);
    if (!first) { std::cerr << first.error() << '\n'; return 1; }
    observe(newGame(*first), "run 1: New Game Yes reaches the map");
    observe(first->clickMap(206, 204) && first->completeLevel(*library, win).has_value(),
            "run 1: SOURCE-TEST injected win accepted by completeLevel (node 0)");
    observe(first->completionFlags()[0] == 1, "run 1: in-memory completionFlags[0] == 1");
    firstFlags = first->completionFlags();
    firstStates = first->mapStates();
    observe(first->click(320, 360) && first->screen() == shell::Screen::Map, "run 1: Continue returns to the map");
    firstMapImage = first->image();
  }  // the session is discarded here: process "exit"

  const bool exists = fs::exists(file, ec);
  const auto bytes = exists ? readBytes(file) : std::vector<std::byte>{};
  std::vector<std::size_t> differing;
  if (bytes.size() == zeroImage.size())
    for (std::size_t i = 0; i < bytes.size(); ++i) if (bytes[i] != zeroImage[i]) differing.push_back(i);
  report << "file " << file.string() << " exists=" << exists << " size=" << bytes.size() << " differing_offsets=[";
  for (const auto o : differing) report << " 0x" << std::hex << o << std::dec;
  report << " ]\n";

  {
    auto second = openSession(*library, file, game);
    if (!second) { std::cerr << second.error() << '\n'; return 1; }
    observe(playCurrent(*second), "run 2 (restart): Play Current Game reaches the map");
    const bool sameFlags = second->completionFlags() == firstFlags;
    const bool sameStates = second->mapStates() == firstStates;
    const bool sameImage = second->image().rgba == firstMapImage.rgba;
    report << "restart: completionFlags[0]=" << second->completionFlags()[0]
           << " same_flags=" << sameFlags << " same_mapStates=" << sameStates
           << " same_map_image=" << sameImage << '\n';
    if (kBefore) {
      observe(!exists, "BEFORE: no SavedGame.dat exists after completing a level");
      observe(second->completionFlags()[0] == 0 && !sameFlags && !sameStates,
              "BEFORE: the restarted clone shows only the initial map (progress lost)");
    } else {
      observe(exists && bytes.size() == 1504, "AFTER: a 1504-byte SavedGame.dat exists");
      observe(differing == std::vector<std::size_t>{0x00C},
              "AFTER: file differs from the zero image only at offset 0x00C (C of record 0)");
      observe(sameFlags && sameStates && sameImage,
              "AFTER: restart shows node 0 complete, identical to the in-memory result of run 1");
    }
  }
  fs::remove_all(root, ec);

  report << "SaveDemo[" << kMode << "]: " << failures << " failures\n";
  std::cout << report.str();
#ifdef SAVEDEMO_REPORT_DIR
  {
    std::error_code dirEc;
    fs::create_directories(SAVEDEMO_REPORT_DIR, dirEc);
    std::ofstream out(fs::path(SAVEDEMO_REPORT_DIR) / (std::string("demo-") + kMode + ".txt"));
    out << report.str();
  }
#endif
  return failures == 0 ? 0 : 1;
}
