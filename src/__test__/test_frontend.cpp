// Load an installed background, font and GERMAN.LTX strings through the frontend
// texture and .LTX readers. Requires WEBOLUTION_GAME_DIR.

#include "assets/Bitmap.h"
#include "assets/Box.h"
#include "assets/GameData.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string_view what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

void testFrontendAssetsLoad() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir) {
    std::cout << "frontend assets: SKIPPED (set WEBOLUTION_GAME_DIR to enable)\n";
    return;
  }
  std::cout << "frontend assets\n";

  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box", fs::path(dir) / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  // Representative background texture. No alpha sigil on the file name (no
  // leading ^/_/$/+/~), so it needs no paired .ALF and should decode opaque.
  {
    const char* name = "TEXTURES\\MAIN_LOGO.BMP";
    check(lib->contains(name), std::format("{} present in the archive", name));
    auto img = webo::loadTexture(*lib, name);
    if (!img) {
      check(false, std::format("{}: {}", name, img.error()));
    } else {
      std::cout << std::format("    {}: {}x{}, alpha {}\n", name, img->width, img->height,
                               webo::alphaModeName(img->alpha));
      check(img->width > 0 && img->height > 0, std::format("{}: non-zero dimensions", name));
      check(img->alpha == webo::AlphaMode::Opaque, std::format("{}: opaque (no sigil)", name));
      check(img->rgba.size() == img->pixelCount() * 4, std::format("{}: rgba buffer sized", name));
    }
  }

  // In the font texture a leading '_' selects color 000000 as transparent.
  {
    const char* name = "TEXTURES\\_FONT.BMP";
    check(lib->contains(name), std::format("{} present in the archive", name));
    auto img = webo::loadTexture(*lib, name);
    if (!img) {
      check(false, std::format("{}: {}", name, img.error()));
    } else {
      std::cout << std::format("    {}: {}x{}, alpha {}\n", name, img->width, img->height,
                               webo::alphaModeName(img->alpha));
      check(img->width > 0 && img->height > 0, std::format("{}: non-zero dimensions", name));
      check(img->alpha == webo::AlphaMode::KeyBlack,
            std::format("{}: colour-keyed on black ('_')", name));

      bool anyTransparent = false;
      for (std::size_t i = 0; i < img->pixelCount(); ++i) {
        if (img->rgba[i * 4 + 3] == 0) {
          anyTransparent = true;
          break;
        }
      }
      check(anyTransparent, std::format("{}: colour-key actually fires on a real glyph sheet", name));
    }
  }

  // Load GERMAN.LTX by name through loadStrings().
  {
    const auto langs = webo::availableLanguages(*lib);
    check(std::ranges::find(langs, "GERMAN") != langs.end(),
          "GERMAN is one of the available languages");
    auto strings = webo::loadStrings(*lib, "GERMAN");
    if (!strings) {
      check(false, std::format("GERMAN.LTX: {}", strings.error()));
    } else {
      std::cout << std::format("    GERMAN.LTX: {} strings, e.g. [1]=\"{}\"\n", strings->size(),
                               (*strings)[1]);
      check(strings->size() > 0, "GERMAN.LTX: a non-zero string count");
      // A leading NUL leaves string index 0 empty. Check a populated index.
      check(!(*strings)[1].empty(), "GERMAN.LTX: at least one non-empty string ([1])");
    }
  }
}

}  // namespace

int main() {
  testFrontendAssetsLoad();

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
