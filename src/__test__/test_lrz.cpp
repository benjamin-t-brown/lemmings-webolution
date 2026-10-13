// Tests for the LRZ decoder.
//
// Needs real game data: LRZ streams are not something you can meaningfully
// synthesise by hand, and the point of the decoder is that it reproduces the
// original's output exactly. Without WEBOLUTION_GAME_DIR the suite skips.
//
// tools/lrz-oracle calls the original executable's own routine; the decoder
// has been verified byte-identical against it. This test re-checks the
// properties that hold for every entry.

#include "assets/Box.h"
#include "assets/Lrz.h"

#include <cstdlib>
#include <cstring>
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

std::uint32_t le32(const std::byte* p) {
  std::uint32_t v;
  std::memcpy(&v, p, 4);
  return v;
}

std::int32_t lei32(const std::byte* p) {
  std::int32_t v;
  std::memcpy(&v, p, 4);
  return v;
}

std::uint16_t le16(const std::byte* p) {
  std::uint16_t v;
  std::memcpy(&v, p, 2);
  return v;
}

// A decoded texture must be a Windows BMP whose pixel data actually fits.
// Note: some entries carry a bfSize that omits row padding (TEXTURES\^OUT.BMP
// and ^TIME.BMP say 2550 for a 26x32x24 image that really needs 2614), so the
// length is checked against the computed size, not against bfSize.
bool looksLikeBmp(const std::vector<std::byte>& d, std::string& why) {
  if (d.size() < 54) {
    why = "shorter than a BMP header";
    return false;
  }
  if (std::to_integer<char>(d[0]) != 'B' || std::to_integer<char>(d[1]) != 'M') {
    why = "no BM magic";
    return false;
  }
  const std::uint32_t offBits = le32(d.data() + 10);
  const std::uint32_t hdrSize = le32(d.data() + 14);
  if (hdrSize != 40) {
    why = std::format("header size {} (expected 40)", hdrSize);
    return false;
  }
  const std::int32_t w = lei32(d.data() + 18);
  const std::int32_t h = lei32(d.data() + 22);
  const std::uint16_t bpp = le16(d.data() + 28);
  if (w <= 0 || h == 0 || bpp == 0) {
    why = std::format("implausible {}x{} at {}bpp", w, h, bpp);
    return false;
  }
  const std::uint64_t rowBytes = ((static_cast<std::uint64_t>(w) * bpp + 31) / 32) * 4;
  const std::uint64_t needed = offBits + rowBytes * static_cast<std::uint64_t>(h < 0 ? -h : h);
  if (d.size() < needed) {
    why = std::format("{} bytes but {}x{}x{} needs {}", d.size(), w, h, bpp, needed);
    return false;
  }
  return true;
}

}  // namespace

int main() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir) {
    std::cout << "lrz: SKIPPED (set WEBOLUTION_GAME_DIR to enable)\n";
    return 0;
  }

  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box",
                                      fs::path(dir) / "patch.box");
  if (!lib) {
    std::cerr << "  FAIL  " << lib.error() << "\n";
    return 1;
  }

  std::size_t decoded = 0, bytesIn = 0, bytesOut = 0;
  const std::size_t kSample = 400;

  for (const auto& name : lib->names()) {
    if (name.size() < 4 || webo::normaliseAssetName(name).rfind(".BMP") != name.size() - 4)
      continue;
    if (decoded >= kSample)
      break;

    auto raw = lib->read(name);
    if (!raw) {
      check(false, std::format("read {}", name));
      break;
    }

    // The predicted size must agree with what we actually produce.
    auto predicted = webo::lrzDecodedSize(*raw);
    if (!predicted) {
      check(false, std::format("size of {}: {}", name, predicted.error()));
      break;
    }

    auto img = webo::lrzDecode(*raw);
    if (!img) {
      check(false, std::format("decode {}: {}", name, img.error()));
      break;
    }
    if (img->size() != *predicted) {
      check(false, std::format("{}: produced {} but predicted {}", name, img->size(), *predicted));
      break;
    }

    std::string why;
    if (!looksLikeBmp(*img, why)) {
      check(false, std::format("{}: {}", name, why));
      break;
    }

    bytesIn += raw->size();
    bytesOut += img->size();
    ++decoded;
  }

  ++g_checks;
  if (decoded != kSample) {
    ++g_failures;
    std::cerr << std::format("  FAIL  only decoded {} of {} entries\n", decoded, kSample);
  }

  std::cout << std::format("lrz: {} entries decoded, {} -> {} bytes ({:.0f}% of original)\n",
                           decoded, bytesIn, bytesOut,
                           bytesOut ? 100.0 * double(bytesIn) / double(bytesOut) : 0.0);
  std::cout << std::format("{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
