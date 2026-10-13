// lrtool -- inspect and extract a Lemmings Revolution asset archive.
//
//   lrtool info    <game-dir>
//   lrtool list    <game-dir> [substring]
//   lrtool extract <game-dir> <name> <out-file>
//   lrtool dump    <game-dir> <out-dir> [substring]
//
// <game-dir> is a Lemmings Revolution installation containing lemmings.box
// (and optionally patch.box, which overrides entries of the same name).

#include "assets/Box.h"
#include "assets/Bitmap.h"
#include "assets/Lrz.h"
#include "assets/Png.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

namespace {

int usage() {
  std::cerr << "usage:\n"
               "  lrtool info    <game-dir>\n"
               "  lrtool list    <game-dir> [substring]\n"
               "  lrtool extract <game-dir> <name> <out-file>\n"
               "  lrtool dump    <game-dir> <out-dir> [substring]\n";
  return 2;
}

std::string humanBytes(std::uint64_t n) {
  static const char* unit[] = {"B", "KB", "MB", "GB"};
  double v = static_cast<double>(n);
  int u = 0;
  while (v >= 1024.0 && u < 3) {
    v /= 1024.0;
    ++u;
  }
  return u == 0 ? std::format("{} {}", n, unit[u]) : std::format("{:.1f} {}", v, unit[u]);
}

std::string extensionOf(std::string_view name) {
  const auto dot = name.rfind('.');
  if (dot == std::string_view::npos)
    return "(none)";
  std::string e(name.substr(dot + 1));
  for (char& c : e)
    if (c >= 'a' && c <= 'z')
      c = static_cast<char>(c - 'a' + 'A');
  return e;
}

bool containsCI(std::string_view haystack, std::string_view needle) {
  if (needle.empty())
    return true;
  const auto h = webo::normaliseAssetName(haystack);
  const auto n = webo::normaliseAssetName(needle);
  return h.find(n) != std::string::npos;
}

std::expected<webo::AssetLibrary, std::string> openGame(const fs::path& dir) {
  const fs::path base = dir / "lemmings.box";
  const fs::path patch = dir / "patch.box";
  if (!fs::exists(base))
    return std::unexpected(std::format("{}: no lemmings.box here", dir.string()));
  return webo::AssetLibrary::open(base, patch);
}

void reportBox(const char* label, const webo::Box& box) {
  const auto v = box.validate();
  std::cout << std::format("{:<12} {:>7} entries  {:>10}  data from 0x{:x}\n", label, box.count(),
                           humanBytes(v.fileSize), box.entries().empty() ? 0 : box.entries()[0].offset);
  std::cout << std::format("{:<12} {}", "", v.ok ? "layout OK" : "LAYOUT PROBLEMS");
  std::cout << std::format("  gaps={}  payload ends {}  slack {}\n", v.gaps, v.payloadEnd,
                           v.fileSize - v.payloadEnd);
  for (const auto& p : v.problems)
    std::cout << "             ! " << p << "\n";
}

int cmdInfo(const fs::path& dir) {
  auto lib = openGame(dir);
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }

  reportBox("lemmings.box", lib->base());
  if (const webo::Box* p = lib->patch()) {
    reportBox("patch.box", *p);
    std::cout << std::format("{:<12} overrides {} base entries, adds {}\n", "",
                             lib->overrideCount(), p->count() - lib->overrideCount());
  } else {
    std::cout << "patch.box    not present\n";
  }

  std::map<std::string, std::pair<std::size_t, std::uint64_t>> byExt;
  for (const auto& e : lib->base().entries()) {
    auto& slot = byExt[extensionOf(e.name)];
    slot.first += 1;
    slot.second += e.size;
  }
  std::cout << "\nbase archive by type:\n";
  std::vector<std::pair<std::string, std::pair<std::size_t, std::uint64_t>>> rows(byExt.begin(),
                                                                                 byExt.end());
  std::ranges::sort(rows, [](const auto& a, const auto& b) { return a.second.first > b.second.first; });
  for (const auto& [ext, stat] : rows)
    std::cout << std::format("  {:<8} {:>6}  {:>10}\n", ext, stat.first, humanBytes(stat.second));
  return 0;
}

int cmdList(const fs::path& dir, std::string_view filter) {
  auto lib = openGame(dir);
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }
  std::size_t shown = 0;
  for (const auto& e : lib->base().entries()) {
    if (!containsCI(e.name, filter))
      continue;
    const bool patched = lib->patch() && lib->patch()->contains(e.name);
    std::cout << std::format("{:>10}  {}{}\n", e.size, e.name, patched ? "   [patched]" : "");
    ++shown;
  }
  if (lib->patch())
    for (const auto& e : lib->patch()->entries())
      if (containsCI(e.name, filter) && !lib->base().contains(e.name)) {
        std::cout << std::format("{:>10}  {}   [patch-only]\n", e.size, e.name);
        ++shown;
      }
  std::cerr << std::format("{} entries\n", shown);
  return 0;
}

bool writeFile(const fs::path& out, std::span<const std::byte> data) {
  std::error_code ec;
  if (out.has_parent_path())
    fs::create_directories(out.parent_path(), ec);
  std::ofstream f(out, std::ios::binary);
  if (!f)
    return false;
  f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
  return static_cast<bool>(f);
}

int cmdExtract(const fs::path& dir, std::string_view name, const fs::path& out) {
  auto lib = openGame(dir);
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }
  auto data = lib->read(name);
  if (!data) {
    std::cerr << data.error() << "\n";
    return 1;
  }
  if (!writeFile(out, *data)) {
    std::cerr << std::format("{}: cannot write\n", out.string());
    return 1;
  }
  std::cout << std::format("{} -> {} ({})\n", name, out.string(), humanBytes(data->size()));
  return 0;
}

int cmdDump(const fs::path& dir, const fs::path& outDir, std::string_view filter) {
  auto lib = openGame(dir);
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }
  std::size_t written = 0, failed = 0;
  std::uint64_t bytes = 0;
  for (const auto& name : lib->names()) {
    if (!containsCI(name, filter))
      continue;
    auto data = lib->read(name);
    if (!data) {
      std::cerr << "  ! " << data.error() << "\n";
      ++failed;
      continue;
    }
    // Archive names are Windows-style; rebuild them as a real directory tree.
    fs::path rel;
    std::string part;
    for (char c : name) {
      if (c == '\\' || c == '/') {
        if (!part.empty())
          rel /= part;
        part.clear();
      } else {
        part.push_back(c);
      }
    }
    if (!part.empty())
      rel /= part;

    if (!writeFile(outDir / rel, *data)) {
      std::cerr << std::format("  ! cannot write {}\n", (outDir / rel).string());
      ++failed;
      continue;
    }
    bytes += data->size();
    if (++written % 1000 == 0)
      std::cerr << std::format("  {} files...\n", written);
  }
  std::cout << std::format("wrote {} files ({}) to {}{}\n", written, humanBytes(bytes),
                           outDir.string(), failed ? std::format(", {} failed", failed) : "");
  return failed ? 1 : 0;
}

int cmdPng(const fs::path& dir, const fs::path& outDir, std::string_view filter) {
  auto lib = openGame(dir);
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }
  std::size_t written = 0, failed = 0;
  std::map<std::string, std::size_t> modes;

  for (const auto& name : lib->names()) {
    if (name.size() < 4 || webo::normaliseAssetName(name).rfind(".BMP") != name.size() - 4)
      continue;
    if (!containsCI(name, filter))
      continue;

    auto img = webo::loadTexture(*lib, name);
    if (!img) {
      std::cerr << "  ! " << img.error() << "\n";
      ++failed;
      continue;
    }
    modes[std::string(webo::alphaModeName(img->alpha))]++;

    fs::path rel;
    std::string part;
    for (char c : name) {
      if (c == '\\' || c == '/') {
        if (!part.empty())
          rel /= part;
        part.clear();
      } else {
        part.push_back(c);
      }
    }
    if (!part.empty())
      rel /= part;
    rel.replace_extension(".png");

    if (auto r = webo::writePng(outDir / rel, *img); !r) {
      std::cerr << "  ! " << r.error() << "\n";
      ++failed;
      continue;
    }
    if (++written % 1000 == 0)
      std::cerr << std::format("  {} images...\n", written);
  }

  std::cout << std::format("wrote {} PNGs to {}{}\n", written, outDir.string(),
                           failed ? std::format(", {} failed", failed) : "");
  for (const auto& [m, n] : modes)
    std::cout << std::format("  {:<10} {:>6}\n", m, n);
  return failed ? 1 : 0;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 3)
    return usage();
  const std::string_view cmd = argv[1];
  const fs::path dir = argv[2];

  if (cmd == "info")
    return cmdInfo(dir);
  if (cmd == "list")
    return cmdList(dir, argc > 3 ? argv[3] : "");
  if (cmd == "decode") {
    if (argc < 5)
      return usage();
    auto lib = openGame(dir);
    if (!lib) {
      std::cerr << lib.error() << "\n";
      return 1;
    }
    auto raw = lib->read(argv[3]);
    if (!raw) {
      std::cerr << raw.error() << "\n";
      return 1;
    }
    auto img = webo::lrzDecode(*raw);
    if (!img) {
      std::cerr << img.error() << "\n";
      return 1;
    }
    if (!writeFile(argv[4], *img)) {
      std::cerr << "cannot write\n";
      return 1;
    }
    std::cout << std::format("{} -> {} ({} bytes)\n", argv[3], argv[4], img->size());
    return 0;
  }
  if (cmd == "extract") {
    if (argc < 5)
      return usage();
    return cmdExtract(dir, argv[3], argv[4]);
  }
  if (cmd == "png") {
    if (argc < 4)
      return usage();
    return cmdPng(dir, argv[3], argc > 4 ? argv[4] : "");
  }
  if (cmd == "dump") {
    if (argc < 4)
      return usage();
    return cmdDump(dir, argv[3], argc > 4 ? argv[4] : "");
  }
  return usage();
}
