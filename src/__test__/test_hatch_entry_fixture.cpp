#include "HatchEntryFixture.h"
#include "assets/Box.h"
#include "assets/Png.h"

#include <fstream>
#include <iostream>
#include <iterator>

int main(int argc, char** argv) {
  if (argc != 6) return 2;
  try {
    const std::filesystem::path game(argv[1]);
    auto library = webo::AssetLibrary::open(game / "lemmings.box", game / "patch.box");
    if (!library) throw std::runtime_error(library.error());
    const int ticks = std::stoi(argv[2]), milliseconds = std::stoi(argv[3]);
    const auto closed = webo::encodePng(webo::test::hatchEntryFixture(*library, {}, ticks, 0));
    std::ifstream file(argv[4], std::ios::binary);
    const std::vector<std::uint8_t> expected{std::istreambuf_iterator<char>(file), {}};
    if (closed != expected)
      throw std::runtime_error("zero-time entry fixture differs from existing headless composition");
    auto frame = webo::test::hatchEntryFixture(*library, {}, ticks, milliseconds,
        false, "MM_CLIMB_002", true, milliseconds / 1000.0);
    auto written = webo::writePng(argv[5], frame);
    if (!written) throw std::runtime_error(written.error());
    std::cout << "Entry fixture: closed control matches; opening sampled at " << milliseconds << " ms\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
