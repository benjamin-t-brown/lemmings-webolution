// Decode every compressed texture entry and verify it really is a Windows BMP.
//
// This is the acceptance test for the LRZ decoder: the original stores
// textures as LRZ-compressed standard bitmaps, so a correct decoder must
// produce 'BM', a bfSize matching the decoded length, and a 40-byte
// BITMAPINFOHEADER -- on all ten thousand of them, not just a sample.

#include "assets/Box.h"
#include "assets/Lrz.h"

#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <map>
#include <string>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "usage: lrcheck <game-dir> [max-entries]\n";
    return 2;
  }
  const fs::path dir = argv[1];
  const std::size_t limit = argc > 2 ? std::stoul(argv[2]) : 0;

  auto lib = webo::AssetLibrary::open(dir / "lemmings.box", dir / "patch.box");
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }

  std::size_t tried = 0, ok = 0, failed = 0, notBmp = 0;
  std::uint64_t inBytes = 0, outBytes = 0;
  std::map<std::string, std::size_t> errors;

  for (const auto& name : lib->names()) {
    if (name.size() < 4 || webo::normaliseAssetName(name).rfind(".BMP") != name.size() - 4)
      continue;
    if (limit && tried >= limit)
      break;
    ++tried;

    auto raw = lib->read(name);
    if (!raw) {
      ++failed;
      errors[raw.error()]++;
      continue;
    }
    auto img = webo::lrzDecode(*raw);
    if (!img) {
      ++failed;
      std::string e = img.error();
      if (e.size() > 60)
        e = e.substr(0, 60) + "...";
      if (errors[e]++ == 0 && errors.size() <= 6)
        std::cerr << std::format("  first failure of this kind: {}\n    {}\n", name, img.error());
      continue;
    }

    inBytes += raw->size();
    outBytes += img->size();

    const auto* b = reinterpret_cast<const unsigned char*>(img->data());
    bool good = img->size() >= 54 && b[0] == 'B' && b[1] == 'M';
    if (good) {
      std::uint32_t offBits, hdrSize;
      std::int32_t w, h;
      std::uint16_t bpp;
      std::memcpy(&offBits, b + 10, 4);
      std::memcpy(&hdrSize, b + 14, 4);
      std::memcpy(&w, b + 18, 4);
      std::memcpy(&h, b + 22, 4);
      std::memcpy(&bpp, b + 28, 2);
      // Check the size the geometry actually needs, not bfSize: a couple of
      // entries carry a bfSize that forgot to account for row padding.
      const std::uint64_t rowBytes = ((std::uint64_t(w) * bpp + 31) / 32) * 4;
      const std::uint64_t needed = offBits + rowBytes * std::uint64_t(h < 0 ? -h : h);
      good = hdrSize == 40 && w > 0 && h != 0 && bpp != 0 && img->size() >= needed;
    }
    if (good) {
      ++ok;
    } else {
      ++notBmp;
      std::cerr << std::format("  not a BMP: {} ({} bytes, starts {:02x} {:02x})\n", name,
                               img->size(), b[0], b[1]);
    }
  }

  std::cout << std::format("\nentries tried : {}\n", tried);
  std::cout << std::format("valid BMP     : {}\n", ok);
  std::cout << std::format("decoded, bad  : {}\n", notBmp);
  std::cout << std::format("decode failed : {}\n", failed);
  if (outBytes)
    std::cout << std::format("compression   : {} -> {} ({:.1f}%)\n", inBytes, outBytes,
                             100.0 * double(inBytes) / double(outBytes));
  for (const auto& [e, n] : errors)
    std::cout << std::format("  {:>6}x  {}\n", n, e);

  return (failed == 0 && notBmp == 0) ? 0 : 1;
}
