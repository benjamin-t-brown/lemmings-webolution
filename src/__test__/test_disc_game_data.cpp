#include "assets/Box.h"
#include "assets/DiscGameData.h"
#include "save/SaveStore.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <span>
#include <stdexcept>
#include <string_view>

namespace {
namespace fs = std::filesystem;

constexpr std::size_t kSector = 2048;
constexpr std::size_t kPatchLba = 20, kBaseLba = 21;

void require(bool ok, std::string_view message) {
  if (!ok) throw std::runtime_error(std::string(message));
}

void integer(std::span<char> target, std::uint32_t value, std::size_t width = 4) {
  for (std::size_t i = 0; i < width; ++i)
    target[i] = static_cast<char>(value >> (i * 8));
}

void bothEndian(std::span<char> target, std::uint32_t value, std::size_t width = 4) {
  integer(target, value, width);
  std::reverse_copy(target.begin(), target.begin() + width, target.begin() + width);
}

std::string box(std::size_t size, char fill) {
  // One entry "X" with a payload that can cross a sector boundary.
  std::string bytes(35 + size, fill);
  bytes.replace(0, 6, "LEMBOX");
  auto fields = std::span(bytes);
  integer(fields.subspan(6), 1);
  integer(fields.subspan(10), 5);
  integer(fields.subspan(14), 1);
  bytes[18] = 'X';
  integer(fields.subspan(19), 1);
  integer(fields.subspan(23), 35);
  integer(fields.subspan(27), 1);
  integer(fields.subspan(31), size);
  return bytes;
}

std::size_t record(std::span<char> target, std::string_view name, std::size_t lba,
                   std::size_t size, bool directory) {
  const auto length = 33 + name.size() + (name.size() % 2 == 0);
  target[0] = static_cast<char>(length);
  bothEndian(target.subspan(2), lba);
  bothEndian(target.subspan(10), size);
  target[25] = directory ? 2 : 0;
  target[32] = static_cast<char>(name.size());
  std::ranges::copy(name, target.begin() + 33);
  return length;
}

std::string identifier(std::string_view name, bool joliet) {
  std::string bytes;
  for (auto c : name) {
    if (joliet) bytes += '\0';
    bytes += c;
  }
  return bytes;
}

struct Layout {
  std::size_t stride, offset;
  bool joliet = false;
};

std::string image(Layout layout, std::string_view base, std::string_view patch,
                  bool omitBase = false) {
  // Leave one declared sector beyond both the game files and the directories.
  const std::uint32_t volumeBlocks = layout.joliet ? 25 : 24;
  std::string bytes(volumeBlocks * layout.stride, '\0');
  const auto sector = [&](std::size_t lba) {
    return std::span(bytes).subspan(lba * layout.stride + layout.offset, kSector);
  };
  const auto descriptor = [&](std::size_t lba, unsigned type) {
    auto data = sector(lba);
    data[0] = static_cast<char>(type);
    std::ranges::copy(std::string_view("CD001"), data.begin() + 1);
    data[6] = 1;
    return data;
  };
  const auto volume = [&](std::span<char> data, std::size_t root) {
    bothEndian(data.subspan(80), volumeBlocks);
    bothEndian(data.subspan(128), kSector, 2);
    record(data.subspan(156), std::string_view("\0", 1), root, kSector, true);
  };
  volume(descriptor(16, 1), layout.joliet ? 23 : 18);
  if (layout.joliet) {
    auto supplementary = descriptor(17, 2);
    std::ranges::copy(std::string_view("%/E"), supplementary.begin() + 88);
    volume(supplementary, 19);
    descriptor(18, 255);
  } else {
    descriptor(17, 255);
  }
  const auto root = sector(layout.joliet ? 19 : 18);
  std::size_t pos = record(root, std::string_view("\0", 1), layout.joliet ? 19 : 18, kSector, true);
  pos += record(root.subspan(pos), std::string_view("\1", 1), layout.joliet ? 19 : 18, kSector, true);
  if (!omitBase)
    pos += record(root.subspan(pos), identifier("lemmings.box;1", layout.joliet),
                  kBaseLba, base.size(), false);
  record(root.subspan(pos), identifier("PATCH.BOX;1", layout.joliet), kPatchLba, patch.size(), false);
  std::ranges::copy(patch, sector(kPatchLba).begin());
  std::ranges::copy(base.substr(0, kSector), sector(kBaseLba).begin());
  std::ranges::copy(base.substr(kSector), sector(kBaseLba + 1).begin());
  return bytes;
}

void write(const fs::path& path, std::string_view bytes) {
  std::ofstream file(path, std::ios::binary);
  file.write(bytes.data(), bytes.size());
  file.close();
  require(bool(file), "write synthetic CD image");
}

std::string read(const fs::path& path) {
  std::ifstream file(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

void dataPaths(const fs::path& scratch) {
  const auto loose = scratch / "loose";
  fs::create_directory(loose);
  write(loose / "LEMMINGS.BOX", box(32, 'b'));
  write(loose / "PaTcH.BoX", box(16, 'p'));
  fs::create_directory(loose / "DECOY.BOX");
  require(webo::resolveGameDataFile(loose / "LEMMINGS.BOX") == loose / "LEMMINGS.BOX",
          "exact box filename is preserved");
  require(webo::resolveGameDataFile(loose / "lemmings.box") == loose / "LEMMINGS.BOX",
          "box resolution returns the on-disk spelling");
  require(webo::resolveGameDataFile(loose / "missing.box") == loose / "missing.box",
          "missing filenames still reach the caller's error checks");
  require(webo::resolveGameDataFile(loose / "decoy.box") == loose / "decoy.box",
          "case-insensitive lookup ignores directories");
  auto required = webo::AssetLibrary::openRequired(loose / "lemmings.box", loose / "patch.box");
  require(required && required->base().path().filename() == "LEMMINGS.BOX" &&
              required->patch() && required->patch()->path().filename() == "PaTcH.BoX",
          "required archives open despite the differing case");
  auto optional = webo::AssetLibrary::open(loose / "lemmings.box", loose / "patch.box");
  require(optional && optional->patch() && optional->patch()->path().filename() == "PaTcH.BoX",
          "optional patch lookup also ignores filename case");

  const auto xdg = fs::absolute(scratch) / "xdg";
  const auto home = fs::absolute(scratch) / "home";
  const auto defaultData = home / ".local" / "share";
  const auto selected = webo::save::xdgDataDirectory(xdg, home);
  require(selected == xdg, "XDG_DATA_HOME takes precedence over HOME");
  require(webo::save::cloneSavePathUnder(*selected) == xdg / "LemmingsWebolution" / "SavedGame.dat",
          "the save is placed under the selected data root");
  require(webo::save::xdgDataDirectory({}, home) == defaultData, "empty XDG falls back to HOME");
  require(webo::save::xdgDataDirectory("relative-data", home) == defaultData,
          "relative XDG falls back to HOME");
  require(!webo::save::xdgDataDirectory({}, {}) &&
              !webo::save::xdgDataDirectory("relative-data", "relative-home"),
          "no save root is selected without an absolute location");
  std::cout << "Case-tolerant archive lookup and XDG data root selection passed.\n";
}

void synthetic(const fs::path& scratch) {
  const auto base = box(2300, 'b'), patch = box(16, 'p');
  const std::array layouts{Layout{2048, 0}, Layout{2352, 16}, Layout{2352, 24, true}};
  const auto good = [&](std::string_view name, std::string_view bytes) {
    const auto folder = scratch / name;
    fs::create_directory(folder);
    const auto input = folder / "synthetic.bin";
    const auto output = folder / "GameData";
    write(input, bytes);
#ifdef _WIN32
    // Simulate an earlier termination before the cache directory was published.
    const auto abandoned = folder / ".disc-data.tmp";
    fs::create_directory(abandoned);
    write(abandoned / "lemmings.box.tmp", "partial");
#endif
    const auto copied = webo::installDiscGameData(input, output);
    require(bool(copied), copied ? "" : copied.error());
    require(read(output / "lemmings.box") == base && read(output / "patch.box") == patch,
            "synthetic ISO payloads match across physical sectors");
    require(std::distance(fs::directory_iterator(folder), fs::directory_iterator{}) == 2,
            "successful import leaves only its input and complete cache");
  };
  for (std::size_t i = 0; i < layouts.size(); ++i) {
    const auto suffix = std::to_string(i);
    auto contents = image(layouts[i], base, patch);
    good("layout-" + suffix, contents);
    contents.resize(contents.size() - layouts[i].stride);
    good("truncated-tail-" + suffix, contents);
  }
  const auto bad = [&](std::string_view name, std::string_view bytes, std::string_view error) {
    const auto folder = scratch / name;
    fs::create_directory(folder);
    const auto input = folder / "synthetic.iso";
    write(input, bytes);
    const auto result = webo::installDiscGameData(input, folder / "GameData");
    require(!result && result.error().contains(error), result ? "invalid image accepted" : result.error());
    require(std::distance(fs::directory_iterator(folder), fs::directory_iterator{}) == 1,
            "failed import leaves only its input image");
  };
  bad("missing-base", image(layouts[0], base, patch, true), "Missing LEMMINGS.BOX");
  auto truncated = image(layouts[1], base, patch);
  truncated.resize((kBaseLba + 1) * layouts[1].stride);
  bad("truncated-box", truncated, "Truncated CD image file extent");
  auto truncatedDirectory = image(layouts[2], base, patch);
  // Cut inside the primary root directory sector, after the box extents and the Joliet root.
  truncatedDirectory.resize(truncatedDirectory.size() - layouts[2].stride - 1);
  bad("truncated-directory", truncatedDirectory, "Truncated CD image sector");
  auto invalidPatch = patch;
  invalidPatch[0] = 'X';
  auto invalid = image(layouts[0], base, invalidPatch);
  bad("invalid-box", invalid, "Invalid game data file");
  invalid.resize(invalid.size() - layouts[0].stride);
  bad("invalid-box-truncated-tail", invalid, "Invalid game data file");
  std::cout << "Synthetic ISO 9660 cases passed (2048, Mode 1, Mode 2 with Joliet, missing, "
               "truncated box, truncated directory, trimmed tail, invalid).\n";
}
}  // namespace

int main(int argc, char** argv) {
  try {
    require(argc == 3, "expected scratch directory and disc fixture path");
    const fs::path scratch = argv[1], fixture = argv[2];
    dataPaths(scratch);
    synthetic(scratch);
    if (fs::is_regular_file(fixture)) {
      const auto copied = webo::installDiscGameData(fixture, scratch / "retail");
      require(bool(copied), copied ? "" : copied.error());
    } else {
      std::cout << "SKIP retail disc extraction: " << fixture << " is unavailable.\n";
    }
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
