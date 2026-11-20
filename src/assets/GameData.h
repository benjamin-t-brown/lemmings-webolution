// Level metadata and interface text:
//   COMPILED\LEVELMAP.DAT   125 flat level records
//   LEVELS\LEVELS.MAP       5 counted tiers of 25 slots in play order
//   COMPILED\RELEASERATE.PC 99 doubles, seconds per lemming for rates 1..99
//   LANGUAGE\*.LTX          232 strings per language

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

// One LEVELS.MAP entry. The three trailing integers are unidentified. Retail a is 1,
// c is 0, and b is 1 in two slots.
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
  // Out-of-range lookups return an empty view.
  std::string_view operator[](std::size_t index) const;

  const std::vector<std::string>& all() const { return strings_; }

private:
  std::vector<std::string> strings_;
};

// RELEASERATE.PC stores seconds between releases for rates 1..99 (5.0 down to 0.2).
// Convert to ticks using the caller's tick rate.
class ReleaseRates {
public:
  ReleaseRates() = default;
  explicit ReleaseRates(std::vector<double> seconds) : seconds_(std::move(seconds)) {}

  bool empty() const { return seconds_.empty(); }
  std::size_t size() const { return seconds_.size(); }

  // Rates are 1-based and clamp to the table.
  double secondsPerLemming(int rate) const;

  // Clamp the interval to at least one tick to prevent a zero-interval release.
  int intervalTicks(int rate, int ticksPerSecond) const;

  const std::vector<double>& all() const { return seconds_; }

private:
  std::vector<double> seconds_;
};

std::expected<std::vector<LevelInfo>, std::string> parseLevelMap(std::span<const std::byte>);
std::expected<LevelTable, std::string> parseLevelTable(std::span<const std::byte>);
std::expected<std::vector<double>, std::string> parseReleaseRates(std::span<const std::byte>);

// Read-and-parse helpers against an archive.
std::expected<std::vector<LevelInfo>, std::string> loadLevelMap(const AssetLibrary&);
std::expected<LevelTable, std::string> loadLevelTable(const AssetLibrary&);
std::expected<std::vector<double>, std::string> loadReleaseRates(const AssetLibrary&);
std::expected<ReleaseRates, std::string> loadReleaseRateTable(const AssetLibrary&);
std::expected<StringTable, std::string> loadStrings(const AssetLibrary&, std::string_view language);

// Language files present in the retail archives, base plus patch.
std::vector<std::string> availableLanguages(const AssetLibrary&);

}  // namespace webo
