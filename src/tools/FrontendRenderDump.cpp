#include "app/FrontendSession.h"

#include "assets/Box.h"
#include "assets/Png.h"

#include <filesystem>
#include <iostream>
#include <string_view>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "usage: FrontendRenderDump <game-dir> <menu|confirm|map|win-fixture|loss-fixture|completed-map-fixture> <out.png>\n";
    return 1;
  }

  const std::string_view screen = argv[2];
  int returnPresses = 0;
  if (screen == "menu") returnPresses = 0;
  else if (screen == "confirm") returnPresses = 1;
  else if (screen == "map" || screen == "win-fixture" || screen == "loss-fixture" ||
           screen == "completed-map-fixture") returnPresses = 2;
  else {
    std::cerr << "invalid screen: " << screen << '\n';
    return 1;
  }

  const fs::path gameDir = argv[1];
  auto library = webo::AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!library) {
    std::cerr << "cannot open archives: " << library.error() << '\n';
    return 1;
  }

  auto session = webo::app::FrontendSession::create(*library);
  if (!session) {
    std::cerr << session.error() << '\n';
    return 1;
  }
  for (int i = 0; i < returnPresses; ++i) session->pressReturn();
  if (screen == "win-fixture" || screen == "loss-fixture" || screen == "completed-map-fixture") {
    // Explicit offline composition fixture, not a simulated solution or native
    // capture. Exercise the production result/map renderer with known values.
    if (!session->clickMap(206, 204) || session->selectedLevel() != "MM_CLIMB_002") {
      std::cerr << "cannot select fixture level\n"; return 1;
    }
    const bool success = screen != "loss-fixture";
    const webo::app::LevelOutcome outcome{"MM_CLIMB_002", success ? 10 : 0, 10, 1, 0, success};
    if (auto result = session->completeLevel(*library, outcome); !result) {
      std::cerr << result.error() << '\n'; return 1;
    }
    if (screen == "completed-map-fixture") session->click(320, 360);
  }

  const fs::path output = argv[3];
  if (auto written = webo::writePng(output, session->image()); !written) {
    std::cerr << written.error() << '\n';
    return 1;
  }
  return 0;
}
