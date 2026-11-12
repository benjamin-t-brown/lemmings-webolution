#include "save/SavedGame.h"

namespace webo::save {
namespace {

std::int32_t readLe32(const std::byte* p) {
  const std::uint32_t value = static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[0])) |
      (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[1])) << 8) |
      (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[2])) << 16) |
      (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(p[3])) << 24);
  return static_cast<std::int32_t>(value);
}

void appendLe32(std::vector<std::byte>& out, std::int32_t signedValue) {
  const auto value = static_cast<std::uint32_t>(signedValue);
  for (int shift = 0; shift < 32; shift += 8)
    out.push_back(static_cast<std::byte>((value >> shift) & 0xFFu));
}

}  // namespace

std::string SavedGameError::message() const {
  const std::string expected = std::to_string(kSavedGameSize);
  switch (kind) {
    case Kind::Empty:
      return "SavedGame.dat is empty (expected exactly " + expected + " bytes)";
    case Kind::TooShort:
      return "SavedGame.dat is truncated: " + std::to_string(size) + " bytes (expected exactly " +
             expected + ")";
    case Kind::TooLong:
      return "SavedGame.dat is oversize: " + std::to_string(size) + " bytes (expected exactly " +
             expected + ")";
  }
  return "SavedGame.dat has an invalid size";
}

std::expected<SavedGame, SavedGameError> SavedGame::parse(std::span<const std::byte> bytes) {
  if (bytes.empty()) return std::unexpected(SavedGameError{SavedGameError::Kind::Empty, 0});
  if (bytes.size() < kSavedGameSize)
    return std::unexpected(SavedGameError{SavedGameError::Kind::TooShort, bytes.size()});
  if (bytes.size() > kSavedGameSize)
    return std::unexpected(SavedGameError{SavedGameError::Kind::TooLong, bytes.size()});
  SavedGame game;
  game.flagRaw = readLe32(bytes.data());
  for (std::size_t n = 0; n < kRecordCount; ++n) {
    const std::byte* p = bytes.data() + 4 + 12 * n;
    game.records[n] = {readLe32(p), readLe32(p + 4), readLe32(p + 8)};
  }
  return game;
}

std::vector<std::byte> SavedGame::serialize() const {
  std::vector<std::byte> out;
  out.reserve(kSavedGameSize);
  appendLe32(out, flagRaw);
  for (const auto& record : records) {  // reader order: A, B, C per record
    appendLe32(out, record.a);
    appendLe32(out, record.b);
    appendLe32(out, record.c);
  }
  return out;
}

bool SavedGame::validKey(std::int32_t group, std::int32_t slot) {
  return group >= 0 && slot >= 0 && static_cast<std::size_t>(group) < kGroupCount &&
         static_cast<std::size_t>(slot) < kSlotsPerGroup;
}

bool SavedGame::markComplete(std::int32_t group, std::int32_t slot) {
  if (!validKey(group, slot)) return false;
  records[recordIndex(group, slot)].c = 1;
  return true;
}

std::array<std::int32_t, kMapNodeCount> SavedGame::nodeFlags(const NodeMapping& mapping) const {
  std::array<std::int32_t, kMapNodeCount> flags{};
  for (std::size_t node = 0; node < kMapNodeCount; ++node) {
    const auto group = mapping[node][0], slot = mapping[node][1];
    if (validKey(group, slot)) flags[node] = records[recordIndex(group, slot)].c != 0 ? 1 : 0;
  }
  return flags;
}

void SavedGame::mergeNodeFlags(const NodeMapping& mapping,
                               const std::array<std::int32_t, kMapNodeCount>& flags) {
  for (std::size_t node = 0; node < kMapNodeCount; ++node) {
    const auto group = mapping[node][0], slot = mapping[node][1];
    if (!validKey(group, slot)) continue;
    auto& c = records[recordIndex(group, slot)].c;
    if (flags[node] != 0) {
      if (c == 0) c = 1;
    } else {
      c = 0;
    }
  }
}

}  // namespace webo::save
