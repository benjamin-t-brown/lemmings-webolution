// The small metadata files: which levels exist, what they are called, how long
// you get, and the localised interface strings.
//
//   COMPILED\LEVELMAP.DAT   125 level records, in a flat list
//   LEVELS\LEVELS.MAP       the same 125 as 5 groups of 25 -- the difficulty
//                           tiers -- in play order
//   COMPILED\RELEASERATE.PC 99 doubles: seconds per lemming for rates 1..99
//   LANGUAGE\*.LTX          232 interface strings per language
//
// Two corrections to the published notes, both found by parsing the real
// files: RELEASERATE.PC is doubles rather than (u32, float) pairs, and
// LEVELS.MAP is five counted groups rather than one flat list of 125.

#pragma once

#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace webo {

class AssetLibrary;

// One row of LEVELMAP.DAT. Unused slots are present and named "blank".
struct LevelInfo {
  std::string id;                      // e.g. "MM_CLIMB_002"; matches a .LVL
  std::string name;                    // e.g. "Just Climb"
  std::uint32_t timeLimitSeconds = 0;
  std::uint32_t saveTarget = 0;        // how many must be rescued
  std::uint32_t totalLemmings = 0;

  bool isBlank() const { return id == "blank"; }
};

// One entry of LEVELS.MAP. The three trailing integers are not understood yet:
// across the retail data `a` is always 1, `c` always 0, and `b` is 1 for just
// two slots.
struct LevelSlot {
  std::string id;
  std::uint32_t a = 0, b = 0, c = 0;
};

struct LevelTable {
  std::vector<std::vector<LevelSlot>> groups;  // 5 groups of 25 in retail data
  std::size_t slotCount() const;
};

// Which Windows ANSI codepage a localisation was authored in.
enum class Codepage {
  Windows1252,  // western europe: UK, France, Germany, Italy, Spain
  Windows1250,  // central europe: Poland, Czech
  Windows1255,  // Hebrew
};

Codepage codepageForLanguage(std::string_view language);

// Interface strings, converted to UTF-8 on load. A '^' inside a string is the
// game's own line break and is left alone.
class StringTable {
public:
  static std::expected<StringTable, std::string> parse(std::span<const std::byte> ltx,
                                                       Codepage cp = Codepage::Windows1252);

  std::size_t size() const { return strings_.size(); }
  // Out of range yields an empty view rather than throwing: missing text
  // should never take the game down.
  std::string_view operator[](std::size_t index) const;

  const std::vector<std::string>& all() const { return strings_; }

private:
  std::vector<std::string> strings_;
};

std::expected<std::vector<LevelInfo>, std::string> parseLevelMap(std::span<const std::byte>);
std::expected<LevelTable, std::string> parseLevelTable(std::span<const std::byte>);
std::expected<std::vector<double>, std::string> parseReleaseRates(std::span<const std::byte>);

// Read-and-parse helpers against an archive.
std::expected<std::vector<LevelInfo>, std::string> loadLevelMap(const AssetLibrary&);
std::expected<LevelTable, std::string> loadLevelTable(const AssetLibrary&);
std::expected<std::vector<double>, std::string> loadReleaseRates(const AssetLibrary&);
std::expected<StringTable, std::string> loadStrings(const AssetLibrary&, std::string_view language);

// Language files present in the retail archives, base plus patch.
std::vector<std::string> availableLanguages(const AssetLibrary&);

}  // namespace webo
