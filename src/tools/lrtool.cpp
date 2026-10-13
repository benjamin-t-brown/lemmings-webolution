// lrtool -- inspect and extract a Lemmings Revolution asset archive.
//
//   lrtool info    <game-dir>
//   lrtool list    <game-dir> [substring]
//   lrtool extract <game-dir> <name> <out-file>
//   lrtool dump    <game-dir> <out-dir> [substring]
//   lrtool lvlmap  <game-dir> <level> <out.png>
//
// <game-dir> is a Lemmings Revolution installation containing lemmings.box
// (and optionally patch.box, which overrides entries of the same name).

#include "assets/Box.h"
#include "assets/GameData.h"
#include "assets/Bitmap.h"
#include "assets/Cylinder.h"
#include "assets/LevelIndex.h"
#include "assets/Lrz.h"
#include "assets/Png.h"
#include "assets/Sprite.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <numbers>
#include <ranges>
#include <utility>
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
               "  lrtool dump    <game-dir> <out-dir> [substring]\n"
               "  lrtool lvlmap  <game-dir> <level> <out.png>\n";
  return 2;
}

// Unrolls a level's cylinder surface to a flat image, so the geometry can be
// looked at instead of inferred. This is a diagnostic, not a level loader: it
// plots the vertices and the ring lattice, and joins consecutive vertices only
// where they land near each other in the unrolled plane. It deliberately does
// not fill faces -- the surface's topology is still unknown (see Cylinder.h),
// and filling would mean asserting one.
int cmdLvlMap(const fs::path& gameDir, std::string_view levelName, const fs::path& out) {
  auto lib = webo::AssetLibrary::open(gameDir / "lemmings.box", gameDir / "patch.box");
  if (!lib) {
    std::cerr << "error: " << lib.error() << "\n";
    return 1;
  }

  // Accept a bare name, with or without the folder and extension.
  std::string wanted(levelName);
  std::ranges::transform(wanted, wanted.begin(),
                         [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
  std::string resolved;
  for (const auto& candidate : webo::levelFileNames(*lib))
    if (candidate.find(wanted) != std::string::npos) {
      resolved = candidate;
      break;
    }
  if (resolved.empty()) {
    std::cerr << std::format("error: no level matching '{}'\n", levelName);
    return 1;
  }

  auto blob = lib->read(resolved);
  if (!blob) {
    std::cerr << "error: " << blob.error() << "\n";
    return 1;
  }
  auto mesh = webo::findCylinderMesh(*blob);
  if (!mesh) {
    std::cerr << "error: " << mesh.error() << "\n";
    return 1;
  }

  constexpr int kScale = 6;
  const int h = static_cast<int>(mesh->height()) * kScale + 1;
  const int w = static_cast<int>(mesh->angularSteps) * kScale * 4;

  webo::Image img;
  img.width = w;
  img.height = h;
  img.rgba.assign(static_cast<std::size_t>(w) * h * 4, 0);
  const auto plot = [&](int x, int y, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    if (x < 0 || y < 0 || x >= w || y >= h)
      return;
    std::uint8_t* p = img.rgba.data() + (static_cast<std::size_t>(y) * w + x) * 4;
    p[0] = r;
    p[1] = g;
    p[2] = b;
    p[3] = 255;
  };

  // The ring lattice: 32 angular steps and the ring heights, with the 8x3
  // tile boundaries picked out more brightly.
  for (std::uint32_t s = 0; s < mesh->angularSteps; ++s) {
    const int x = static_cast<int>(static_cast<double>(s) / mesh->angularSteps * w);
    const bool tile = mesh->stepsPerTileColumn() && s % mesh->stepsPerTileColumn() == 0;
    for (int y = 0; y < h; ++y)
      plot(x, y, tile ? 70 : 34, tile ? 70 : 34, tile ? 90 : 44);
  }
  for (std::uint32_t r = 0; r < mesh->ringCount; ++r) {
    const int y = h - 1 - static_cast<int>(r * mesh->ringSpacing()) * kScale;
    const bool tile = mesh->bandsPerTileRow() && r % mesh->bandsPerTileRow() == 0;
    for (int x = 0; x < w; ++x)
      plot(x, y, tile ? 70 : 34, tile ? 70 : 34, tile ? 90 : 44);
  }

  // Unroll a vertex: angle around the tube to X, height to Y.
  const auto project = [&](const webo::Vec3& v) {
    double a = std::atan2(-v.x, v.z) / (2.0 * std::numbers::pi);
    if (a < 0)
      a += 1.0;
    return std::pair{static_cast<int>(a * w), h - 1 - static_cast<int>(v.y) * kScale};
  };

  // Join consecutive vertices when they are close in the unrolled plane. A
  // jump to a new column exceeds the threshold and is left undrawn rather
  // than guessed at.
  const int limit = kScale * 5;
  for (std::size_t i = 1; i < mesh->surface.size(); ++i) {
    const auto [x0, y0] = project(mesh->surface[i - 1]);
    const auto [x1, y1] = project(mesh->surface[i]);
    if (std::abs(x1 - x0) > limit || std::abs(y1 - y0) > limit)
      continue;
    const int steps = std::max(std::abs(x1 - x0), std::abs(y1 - y0));
    for (int k = 0; k <= steps; ++k) {
      const double t = steps ? static_cast<double>(k) / steps : 0.0;
      plot(static_cast<int>(x0 + (x1 - x0) * t), static_cast<int>(y0 + (y1 - y0) * t), 60, 130, 90);
    }
  }

  for (const auto& v : mesh->surface) {
    const auto [x, y] = project(v);
    plot(x, y, 240, 200, 90);
    plot(x + 1, y, 240, 200, 90);
    plot(x, y + 1, 240, 200, 90);
    plot(x + 1, y + 1, 240, 200, 90);
  }

  if (auto r = webo::writePng(out, img); !r) {
    std::cerr << "error: " << r.error() << "\n";
    return 1;
  }
  const auto skinned = std::ranges::count_if(mesh->tileTextures,
                                             [](const std::string& s) { return !s.empty(); });
  std::cout << std::format("{}: {} surface vertices, {} skinned tiles -> {} ({}x{})\n", resolved,
                           mesh->surface.size(), skinned, out.string(), w, h);
  return 0;
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

int cmdLevels(const fs::path& dir) {
  auto lib = openGame(dir);
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }

  auto infos = webo::loadLevelMap(*lib);
  auto table = webo::loadLevelTable(*lib);
  auto rates = webo::loadReleaseRates(*lib);
  if (!infos) { std::cerr << infos.error() << "\n"; return 1; }
  if (!table) { std::cerr << table.error() << "\n"; return 1; }
  if (!rates) { std::cerr << rates.error() << "\n"; return 1; }

  std::map<std::string, const webo::LevelInfo*> byId;
  for (const auto& i : *infos)
    byId[i.id] = &i;

  std::cout << std::format("{} level records, {} slots in {} groups, {} release rates\n\n",
                           infos->size(), table->slotCount(), table->groups.size(), rates->size());

  for (std::size_t g = 0; g < table->groups.size(); ++g) {
    std::size_t playable = 0;
    for (const auto& s : table->groups[g])
      if (s.id != "blank")
        ++playable;
    std::cout << std::format("group {} -- {} slots, {} playable\n", g, table->groups[g].size(),
                             playable);
    std::size_t shown = 0;
    for (const auto& s : table->groups[g]) {
      if (s.id == "blank")
        continue;
      if (shown++ >= 4)
        break;
      const auto it = byId.find(s.id);
      if (it == byId.end()) {
        std::cout << std::format("    {:<18} (no record)\n", s.id);
        continue;
      }
      const auto* i = it->second;
      std::cout << std::format("    {:<18} {:<26} {:>4}s  save {:>3} of {:>3}\n", i->id, i->name,
                               i->timeLimitSeconds, i->saveTarget, i->totalLemmings);
    }
    if (playable > 4)
      std::cout << std::format("    ... and {} more\n", playable - 4);
  }

  std::cout << std::format("\nrelease rate 1 = {:.3f}s per lemming, 99 = {:.3f}s\n",
                           rates->front(), rates->back());

  const auto langs = webo::availableLanguages(*lib);
  std::cout << std::format("languages: {}\n", langs.size());
  for (const auto& l : langs) {
    auto t = webo::loadStrings(*lib, l);
    if (!t) {
      std::cout << std::format("  {:<8} ERROR {}\n", l, t.error());
      continue;
    }
    std::cout << std::format("  {:<8} {:>4} strings   e.g. \"{}\"\n", l, t->size(), (*t)[2]);
  }
  return 0;
}

int cmdSprites(const fs::path& dir, std::string_view filter) {
  auto lib = openGame(dir);
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }

  std::size_t files = 0, failed = 0, totalFrames = 0, missingSheets = 0;
  for (const auto& name : webo::spriteSheetNames(*lib)) {
    if (!containsCI(name, filter))
      continue;
    auto sheet = webo::loadSpriteSheet(*lib, name);
    if (!sheet) {
      std::cerr << "  ! " << sheet.error() << "\n";
      ++failed;
      continue;
    }
    ++files;
    totalFrames += sheet->frameCount();

    std::string names;
    for (std::size_t i = 0; i < sheet->sprites.size() && i < 6; ++i) {
      if (i)
        names += ", ";
      names += sheet->sprites[i].name;
    }
    if (sheet->sprites.size() > 6)
      names += ", ...";

    std::cout << std::format("{:<24} {:>3} sprites {:>5} frames{}  {}\n",
                             fs::path(name).filename().string(), sheet->sprites.size(),
                             sheet->frameCount(),
                             sheet->palette.empty()
                                 ? std::string("          ")
                                 : std::format("  {:>2} colours", sheet->palette.size()),
                             names);

    for (const auto& s : sheet->sprites)
      for (const auto& a : s.animations)
        for (const auto& f : a.frames)
          if (!lib->contains(webo::sheetAssetName(f.sheet)))
            ++missingSheets;
  }

  std::cout << std::format("\n{} files, {} frames, {} failed, {} frames referencing a missing sheet\n",
                           files, totalFrames, failed, missingSheets);
  return (failed || missingSheets) ? 1 : 0;
}

// Composite one animation's frames into a horizontal strip, so the rectangles
// can be checked by eye rather than only by arithmetic.
int cmdAnim(const fs::path& dir, std::string_view sprName, std::string_view spriteName,
            const fs::path& out, std::size_t animIndex) {
  auto lib = openGame(dir);
  if (!lib) {
    std::cerr << lib.error() << "\n";
    return 1;
  }
  auto sheet = webo::loadSpriteSheet(*lib, sprName);
  if (!sheet) {
    std::cerr << sheet.error() << "\n";
    return 1;
  }
  const webo::Sprite* sprite = sheet->find(spriteName);
  if (!sprite) {
    std::cerr << std::format("no sprite '{}'; available:", spriteName);
    for (const auto& s : sheet->sprites)
      std::cerr << " " << s.name;
    std::cerr << "\n";
    return 1;
  }
  if (animIndex >= sprite->animations.size()) {
    std::cerr << std::format("sprite '{}' has {} animations\n", spriteName,
                             sprite->animations.size());
    return 1;
  }
  const auto& frames = sprite->animations[animIndex].frames;
  if (frames.empty()) {
    std::cerr << "animation has no frames\n";
    return 1;
  }

  int stripW = 0, stripH = 0;
  for (const auto& f : frames) {
    stripW += static_cast<int>(f.width) + 2;
    stripH = std::max(stripH, static_cast<int>(f.height));
  }

  webo::Image strip;
  strip.width = stripW;
  strip.height = stripH;
  strip.rgba.assign(static_cast<std::size_t>(stripW) * stripH * 4, 0);

  std::map<std::string, webo::Image> cache;
  int penX = 0;
  for (const auto& f : frames) {
    const std::string asset = webo::sheetAssetName(f.sheet);
    auto it = cache.find(asset);
    if (it == cache.end()) {
      auto img = webo::loadTexture(*lib, asset);
      if (!img) {
        std::cerr << "  ! " << img.error() << "\n";
        return 1;
      }
      it = cache.emplace(asset, std::move(*img)).first;
    }
    const webo::Image& src = it->second;

    for (std::uint32_t y = 0; y < f.height; ++y) {
      const std::uint32_t sy = f.top + y;
      if (sy >= static_cast<std::uint32_t>(src.height))
        break;
      for (std::uint32_t x = 0; x < f.width; ++x) {
        const std::uint32_t sx = f.left + x;
        if (sx >= static_cast<std::uint32_t>(src.width))
          break;
        const std::uint8_t* s = src.rgba.data() + (static_cast<std::size_t>(sy) * src.width + sx) * 4;
        std::uint8_t* d = strip.rgba.data() +
                          (static_cast<std::size_t>(y) * stripW + penX + x) * 4;
        std::memcpy(d, s, 4);
      }
    }
    penX += static_cast<int>(f.width) + 2;
  }

  if (auto r = webo::writePng(out, strip); !r) {
    std::cerr << r.error() << "\n";
    return 1;
  }
  std::cout << std::format("{} '{}' animation {}: {} frames -> {} ({}x{})\n", sprName, spriteName,
                           animIndex, frames.size(), out.string(), stripW, stripH);
  return 0;
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
  if (cmd == "anim") {
    if (argc < 6)
      return usage();
    return cmdAnim(dir, argv[3], argv[4], argv[5], argc > 6 ? std::stoul(argv[6]) : 0);
  }
  if (cmd == "sprites")
    return cmdSprites(dir, argc > 3 ? argv[3] : "");
  if (cmd == "levels")
    return cmdLevels(dir);
  if (cmd == "png") {
    if (argc < 4)
      return usage();
    return cmdPng(dir, argv[3], argc > 4 ? argv[4] : "");
  }
  if (cmd == "lvlmap") {
    if (argc < 5)
      return usage();
    return cmdLvlMap(argv[2], argv[3], argv[4]);
  }
  if (cmd == "dump") {
    if (argc < 4)
      return usage();
    return cmdDump(dir, argv[3], argc > 4 ? argv[4] : "");
  }
  return usage();
}
