#include "Progress.h"

#include <cstdlib>
#include <fstream>
#include <format>

namespace webo::shell {
namespace {

constexpr char kMagic[8] = {'W', 'E', 'B', 'O', 'P', 'R', 'O', 'G'};
constexpr std::uint32_t kVersion = 1;

}  // namespace

void Progress::resizeFor(const LevelTable& table) {
  const std::size_t n = static_cast<std::size_t>(kGroups) * kSlotsPerGroup;
  if (slots.size() != n) {
    slots.resize(n, SlotState::Locked);
  }
  (void)table;
}

void Progress::newGame(const LevelTable& table) {
  resizeFor(table);
  const int g0 = 0;
  const int s0 = firstPlayableSlot(table, g0);
  if (s0 >= 0)
    slots[index(g0, s0)] = SlotState::Unlocked;
}

bool Progress::canPlay(int group, int slot) const {
  if (group < 0 || group >= kGroups || slot < 0 || slot >= kSlotsPerGroup)
    return false;
  if (slots.size() < index(group, slot) + 1)
    return false;
  const SlotState st = slots[index(group, slot)];
  return st == SlotState::Unlocked || st == SlotState::Completed;
}

SlotState Progress::state(int group, int slot) const {
  if (group < 0 || group >= kGroups || slot < 0 || slot >= kSlotsPerGroup)
    return SlotState::Locked;
  if (slots.size() < index(group, slot) + 1)
    return SlotState::Locked;
  return slots[index(group, slot)];
}

void Progress::complete(const LevelTable& table, int group, int slot) {
  if (!canPlay(group, slot) && state(group, slot) != SlotState::Locked) {
    // allow completing unlocked
  }
  if (group < 0 || group >= kGroups || slot < 0 || slot >= kSlotsPerGroup)
    return;
  resizeFor(table);
  slots[index(group, slot)] = SlotState::Completed;
  int ng = 0, ns = 0;
  if (nextPlayable(table, group, slot, ng, ns)) {
    if (slots[index(ng, ns)] == SlotState::Locked)
      slots[index(ng, ns)] = SlotState::Unlocked;
  }
}

int firstPlayableSlot(const LevelTable& table, int group) {
  if (group < 0 || group >= static_cast<int>(table.groups.size()))
    return -1;
  const auto& g = table.groups[static_cast<std::size_t>(group)];
  for (int i = 0; i < static_cast<int>(g.size()) && i < Progress::kSlotsPerGroup; ++i) {
    if (g[static_cast<std::size_t>(i)].id.empty())
      continue;
    if (g[static_cast<std::size_t>(i)].id == "blank")
      continue;
    return i;
  }
  return -1;
}

bool nextPlayable(const LevelTable& table, int group, int slot, int& outGroup, int& outSlot) {
  for (int g = group; g < static_cast<int>(table.groups.size()) && g < Progress::kGroups; ++g) {
    const auto& row = table.groups[static_cast<std::size_t>(g)];
    const int start = (g == group) ? slot + 1 : 0;
    for (int s = start; s < static_cast<int>(row.size()) && s < Progress::kSlotsPerGroup; ++s) {
      const auto& id = row[static_cast<std::size_t>(s)].id;
      if (id.empty() || id == "blank")
        continue;
      outGroup = g;
      outSlot = s;
      return true;
    }
  }
  return false;
}

std::filesystem::path defaultProgressPath() {
  if (const char* dir = std::getenv("WEBOLUTION_GAME_DIR"))
    return std::filesystem::path(dir) / "webolution_progress.dat";
  return std::filesystem::path("webolution_progress.dat");
}

std::expected<void, std::string> saveProgress(const Progress& p, const std::filesystem::path& path) {
  std::ofstream out(path, std::ios::binary);
  if (!out)
    return std::unexpected(std::format("progress: cannot write {}", path.string()));
  out.write(kMagic, 8);
  const std::uint32_t ver = kVersion;
  out.write(reinterpret_cast<const char*>(&ver), 4);
  const std::uint32_t langLen = static_cast<std::uint32_t>(p.language.size());
  out.write(reinterpret_cast<const char*>(&langLen), 4);
  out.write(p.language.data(), static_cast<std::streamsize>(p.language.size()));
  out.write(reinterpret_cast<const char*>(&p.musicVolume), 4);
  out.write(reinterpret_cast<const char*>(&p.sfxVolume), 4);
  const std::uint32_t n = static_cast<std::uint32_t>(p.slots.size());
  out.write(reinterpret_cast<const char*>(&n), 4);
  for (SlotState s : p.slots) {
    const std::uint8_t b = static_cast<std::uint8_t>(s);
    out.write(reinterpret_cast<const char*>(&b), 1);
  }
  if (!out)
    return std::unexpected("progress: write failed");
  return {};
}

std::expected<Progress, std::string> loadProgress(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in)
    return std::unexpected(std::format("progress: cannot read {}", path.string()));
  char magic[8] = {};
  in.read(magic, 8);
  if (std::string_view(magic, 8) != std::string_view(kMagic, 8))
    return std::unexpected("progress: bad magic");
  std::uint32_t ver = 0;
  in.read(reinterpret_cast<char*>(&ver), 4);
  if (ver != kVersion)
    return std::unexpected(std::format("progress: unsupported version {}", ver));
  Progress p;
  std::uint32_t langLen = 0;
  in.read(reinterpret_cast<char*>(&langLen), 4);
  if (langLen > 64)
    return std::unexpected("progress: language too long");
  p.language.resize(langLen);
  in.read(p.language.data(), static_cast<std::streamsize>(langLen));
  in.read(reinterpret_cast<char*>(&p.musicVolume), 4);
  in.read(reinterpret_cast<char*>(&p.sfxVolume), 4);
  std::uint32_t n = 0;
  in.read(reinterpret_cast<char*>(&n), 4);
  if (n > 5 * 25)
    return std::unexpected("progress: too many slots");
  p.slots.resize(n);
  for (std::uint32_t i = 0; i < n; ++i) {
    std::uint8_t b = 0;
    in.read(reinterpret_cast<char*>(&b), 1);
    p.slots[i] = static_cast<SlotState>(b);
  }
  if (!in)
    return std::unexpected("progress: truncated");
  return p;
}

}  // namespace webo::shell
