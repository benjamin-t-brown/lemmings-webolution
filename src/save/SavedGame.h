#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace webo::save {

// Original SavedGame.dat layout, reader 0x45F43C and writer 0x45F94D.
// Reference EXE SHA-256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f.
// LE int32 flag at 0x000. A, B, C at 0x004 + 12 * (group * 25 + slot) + {0, 4, 8}.
// The 5 x 25 records and flag occupy 1504 bytes. C marks completion.
// The flag, A and B have unknown meanings and are preserved verbatim.
inline constexpr std::size_t kGroupCount = 5;
inline constexpr std::size_t kSlotsPerGroup = 25;
inline constexpr std::size_t kRecordCount = kGroupCount * kSlotsPerGroup;  // 125
inline constexpr std::size_t kMapNodeCount = 102;
inline constexpr std::size_t kSavedGameSize = 4 + kRecordCount * 12;  // 1504

// One {group, slot, base} entry of the VA 0x4B3948 table (MapScene::nodeMapping).
using NodeMapping = std::array<std::array<std::int32_t, 3>, kMapNodeCount>;

struct SaveRecord {
  std::int32_t a = 0;
  std::int32_t b = 0;
  std::int32_t c = 0;  // completion; the original only tests C != 0
  bool operator==(const SaveRecord&) const = default;
};

// Clone policy: accept exactly kSavedGameSize bytes. The original
// reads short files into an incomplete image and ignores trailing bytes.
struct SavedGameError {
  enum class Kind { Empty, TooShort, TooLong } kind = Kind::Empty;
  std::size_t size = 0;
  std::string message() const;
};

struct SavedGame {
  // Raw int32 at 0x000. Preserve its value across saves. The original
  // normalizes it to 0 or 1.
  std::int32_t flagRaw = 0;
  std::array<SaveRecord, kRecordCount> records{};

  static std::expected<SavedGame, SavedGameError> parse(std::span<const std::byte> bytes);
  // Always kSavedGameSize bytes, little-endian regardless of host byte order.
  std::vector<std::byte> serialize() const;

  // True when (group, slot) names one of the 5 x 25 records.
  static bool validKey(std::int32_t group, std::int32_t slot);
  static std::size_t recordIndex(std::int32_t group, std::int32_t slot) {
    return static_cast<std::size_t>(group) * kSlotsPerGroup + static_cast<std::size_t>(slot);
  }

  // Mirrors 0x485E55: C := 1 for the winning (group, slot). False if out of range.
  bool markComplete(std::int32_t group, std::int32_t slot);

  // Project C != 0 into the 102 map-node flags (1 / 0).
  std::array<std::int32_t, kMapNodeCount> nodeFlags(const NodeMapping& mapping) const;
  // Merge flags by (group, slot). A zero flag clears C, while a nonzero flag
  // sets C if needed and preserves a nonzero C verbatim. Keep unmapped records,
  // the file flag and fields A and B intact.
  void mergeNodeFlags(const NodeMapping& mapping,
                      const std::array<std::int32_t, kMapNodeCount>& flags);
};

}  // namespace webo::save
