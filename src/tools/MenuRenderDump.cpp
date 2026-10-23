// B1w: render the highlight-0 main menu from the real archive and write a PNG.
//
//   MenuRenderDump <game-dir> <out.png> [--xword N] [--rowstep N] [--sheet ARCHIVE_NAME]
//
// The overrides exist only for the three single-input mutation checks; defaults are the
// read/observed values in MenuRender.h.

#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/GameData.h"
#include "assets/Png.h"
#include "shell/MenuRender.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << "usage: MenuRenderDump <game-dir> <out.png> [--xword N] [--rowstep N] [--sheet NAME]\n";
    return 2;
  }
  const fs::path game = argv[1], out = argv[2];
  webo::shell::MenuParams params;
  std::string sheetName = "TEXTURES\\^FONT.BMP";
  for (int i = 3; i + 1 < argc; i += 2) {
    const std::string k = argv[i];
    if (k == "--xword") params.xWord = std::atoi(argv[i + 1]);
    else if (k == "--rowstep") params.rowStep = std::atoi(argv[i + 1]);
    else if (k == "--sheet") sheetName = argv[i + 1];
    else { std::cerr << "unknown option " << k << '\n'; return 2; }
  }
  auto lib = webo::AssetLibrary::open(game / "lemmings.box", game / "patch.box");
  if (!lib) { std::cerr << "cannot open archives: " << lib.error() << '\n'; return 2; }
  auto backdrop = webo::loadTexture(*lib, "TEXTURES\\MAIN_LOGO.BMP");
  if (!backdrop) { std::cerr << backdrop.error() << '\n'; return 2; }
  auto sheet = webo::loadTexture(*lib, sheetName);
  if (!sheet) { std::cerr << sheet.error() << '\n'; return 2; }
  auto strings = webo::loadStrings(*lib, "UK");
  if (!strings) { std::cerr << strings.error() << '\n'; return 2; }
  std::vector<std::string> labels;
  for (int idx : webo::shell::kLabelIndices) labels.emplace_back((*strings)[static_cast<std::size_t>(idx)]);

  const auto img = webo::shell::renderMainMenu(*backdrop, *sheet, labels, params, webo::shell::fontCells());
  auto w = webo::writePng(out, img);
  if (!w) { std::cerr << w.error() << '\n'; return 2; }
  std::cout << "wrote " << out.string() << " sheet=" << sheetName << " xword=" << params.xWord
            << " rowstep=" << params.rowStep << '\n';
  return 0;
}
