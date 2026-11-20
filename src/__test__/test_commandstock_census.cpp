// Report serialized values_27d0[0..10] for every effective installed level.
// 0x0045B86D reads eleven words into game+0x27D0..+0x2C. The Armageddon
// guard indexes stock[9] (0x004532E2 tail, 0x0045AC5C).
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "lemmings/compiled_level.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

namespace {
std::string idOf(const std::string& name) {
  auto s = name.find_last_of("\\/");
  auto base = s == std::string::npos ? name : name.substr(s + 1);
  auto dot = base.rfind('.');
  return dot == std::string::npos ? base : base.substr(0, dot);
}
}  // namespace

int main() try {
  const char* gameDir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!gameDir || !*gameDir) {
    std::cout << "SKIP: WEBOLUTION_GAME_DIR unset\n";
    return 0;
  }
  auto lib = AssetLibrary::open(fs::path(gameDir) / "lemmings.box", fs::path(gameDir) / "patch.box");
  if (!lib) {
    std::cerr << "FAIL open archives: " << lib.error() << '\n';
    return 1;
  }
  std::vector<std::string> names = levelFileNames(*lib);
  std::sort(names.begin(), names.end());
  std::ostringstream out;
  out << "gameplay-commands-research-001 stock census (source-built; not native evidence)\n";
  out << "columns: level stock[0..10] (values_27d0)\n";
  std::size_t read = 0, failed = 0, with8 = 0, with9 = 0, with10 = 0;
  std::map<std::uint32_t, std::size_t> hist9;
  std::vector<std::string> ids9;
  for (const auto& name : names) {
    auto bytes = lib->read(name);
    if (!bytes) {
      ++failed;
      out << "LEVEL " << idOf(name) << " READ_FAIL " << bytes.error() << '\n';
      continue;
    }
    try {
      const std::span<const std::uint8_t> view(reinterpret_cast<const std::uint8_t*>(bytes->data()),
                                               bytes->size());
      const auto level = lemmings::read_compiled_level(view, /*require_eof=*/false);
      const auto& v = level.gadgets.values_27d0;
      ++read;
      out << "LEVEL " << idOf(name);
      for (std::size_t k = 0; k < v.size(); ++k) out << ' ' << v[k];
      out << '\n';
      if (v.size() > 8 && v[8] != 0) ++with8;
      if (v.size() > 9) {
        ++hist9[v[9]];
        if (v[9] != 0) {
          ++with9;
          ids9.push_back(idOf(name));
        }
      }
      if (v.size() > 10 && v[10] != 0) ++with10;
    } catch (const std::exception& e) {
      ++failed;
      out << "LEVEL " << idOf(name) << " PARSE_FAIL " << e.what() << '\n';
    }
  }
  out << "SUMMARY levels=" << names.size() << " read=" << read << " failed=" << failed
      << " stock8_nonzero=" << with8 << " stock9_nonzero=" << with9 << " word10_nonzero=" << with10 << '\n';
  for (const auto& [value, n] : hist9) out << "HIST stock9=" << value << " levels=" << n << '\n';
  out << "STOCK9_NONZERO_LEVELS";
  for (const auto& id : ids9) out << ' ' << id;
  out << '\n';
  const fs::path dir = COMMAND_CENSUS_DIR;
  fs::create_directories(dir);
  std::ofstream(dir / "stock_census.txt") << out.str();
  std::cout << out.str();
  if (names.empty() || read == 0) {
    std::cerr << "FAIL: no installed level could be read\n";
    return 1;
  }
  std::cout << "PASS commandstock_census\n";
  return 0;
} catch (const std::exception& e) {
  std::cerr << "FAIL commandstock_census: " << e.what() << '\n';
  return 1;
}
