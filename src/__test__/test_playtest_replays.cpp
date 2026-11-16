#include "app/Replay.hpp"
#include "assets/Box.h"

#include <algorithm>
#include <vector>

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "usage: test_playtest_replays <replay-directory> <game-directory>\n";
    return 1;
  }
  namespace fs = std::filesystem;
  std::error_code ec;
  const fs::path directory(argv[1]);
  std::vector<fs::path> files;
  if (fs::exists(directory, ec)) {
    fs::recursive_directory_iterator it(directory, ec), end;
    while (!ec && it != end) {
      if (it->is_regular_file(ec) && it->path().extension() == ".replay") files.push_back(it->path());
      if (!ec) it.increment(ec);
    }
  }
  if (ec) { std::cerr << "replays: " << ec.message() << '\n'; return 1; }
  std::sort(files.begin(), files.end());
  if (files.empty()) { std::cout << "playtest replays: 0 files (nothing promoted yet)\n"; return 0; }
  auto library = webo::AssetLibrary::open(fs::path(argv[2]) / "lemmings.box", fs::path(argv[2]) / "patch.box");
  if (!library) { std::cerr << library.error() << '\n'; return 1; }
  int failed = 0;
  for (const auto& file : files) {
    std::cout << "Checking " << file.string() << '\n';
    failed += webo::app::replayLevel(*library, file, std::cout) != 0;
  }
  std::cout << "playtest replays: " << files.size() - failed << '/' << files.size() << " passed\n";
  return failed ? 1 : 0;
}
