#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace webo::save {

// Original SavedGame.dat image: reader 0x45F43C, writer 0x45F94D, reference exe
// SHA-256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f.
// [disassembly] int32 LE flag at 0x000, then for n = group * 25 + slot the three
// int32 values A, B, C at 0x004 + 12 * n + {0, 4, 8}; 5 x 25 records, 1504 bytes.
// Only C (completion) has a recovered consumer; the meanings of the flag, A and B
// are unknown and every value is kept verbatim. The installed sample is all zero,
// so it confirms size and the zero state only: nonzero field positions rest on
// disassembly, and a byte-layout test proves source consistency, nothing more.
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

// Clone policy, not original behaviour: the original reads short files as
// indeterminate stack data and ignores trailing bytes; the clone rejects both.
struct SavedGameError {
  enum class Kind { Empty, TooShort, TooLong } kind = Kind::Empty;
  std::size_t size = 0;
  std::string message() const;
};

struct SavedGame {
  // Raw int32 at 0x000. The original stores (value != 0) and would rewrite 0/1;
  // the clone keeps the value verbatim so unknown data survives a round trip.
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
  // Merge node flags back by (group, slot): a nonzero flag makes C nonzero (an
  // existing nonzero C is kept verbatim), a zero flag clears C. Records with no
  // map node, A, B and the flag are untouched.
  void mergeNodeFlags(const NodeMapping& mapping,
                      const std::array<std::int32_t, kMapNodeCount>& flags);
};

}  // namespace webo::save
