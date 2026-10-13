// Tests for the metadata files: level list, level table, release rates and
// localised strings.

#include "assets/Box.h"
#include "assets/GameData.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

int g_failures = 0;
int g_checks = 0;

void check(bool cond, std::string_view what) {
  ++g_checks;
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

void put32(std::vector<std::byte>& v, std::uint32_t x) {
  for (int i = 0; i < 4; ++i)
    v.push_back(static_cast<std::byte>((x >> (8 * i)) & 0xFF));
}
void put16(std::vector<std::byte>& v, std::uint16_t x) {
  v.push_back(static_cast<std::byte>(x & 0xFF));
  v.push_back(static_cast<std::byte>(x >> 8));
}
void putStr(std::vector<std::byte>& v, std::string_view s) {
  for (char c : s)
    v.push_back(static_cast<std::byte>(c));
}

void testCodepageSelection() {
  std::cout << "codepage selection\n";
  check(webo::codepageForLanguage("UK") == webo::Codepage::Windows1252, "UK is 1252");
  check(webo::codepageForLanguage("france") == webo::Codepage::Windows1252, "case insensitive");
  check(webo::codepageForLanguage("POLISH") == webo::Codepage::Windows1250, "Polish is 1250");
  check(webo::codepageForLanguage("CZECH") == webo::Codepage::Windows1250, "Czech is 1250");
  check(webo::codepageForLanguage("HEBREW") == webo::Codepage::Windows1255, "Hebrew is 1255");
}

void testStringTable() {
  std::cout << "string table\n";
  // count, offsets, then NUL-terminated bytes.
  std::vector<std::byte> d;
  put32(d, 3);
  const std::uint32_t header = 4 + 3 * 4;
  put32(d, header);           // "Start"   5 chars + NUL
  put32(d, header + 6);       // "Options" 7 chars + NUL
  put32(d, header + 6 + 8);
  putStr(d, "Start");
  d.push_back(std::byte{0});
  putStr(d, "Options");
  d.push_back(std::byte{0});
  // 0xC4/0xF6 are A-diaeresis and o-diaeresis in both 1250 and 1252, so they
  // cannot tell the tables apart. 0x8C can: it is OE in 1252 and S-acute in 1250.
  putStr(d, "\xC4\xF6\x8C");
  d.push_back(std::byte{0});

  auto t = webo::StringTable::parse(d);
  check(t.has_value(), "parses");
  if (!t) {
    std::cerr << "    " << t.error() << "\n";
    return;
  }
  check(t->size() == 3, "count");
  check((*t)[0] == "Start", "first string");
  check((*t)[1] == "Options", "second string");
  check((*t)[2] == "ÄöŒ", "1252 high bytes become UTF-8");
  check((*t)[99].empty(), "out of range is empty, not a crash");

  auto t1250 = webo::StringTable::parse(d, webo::Codepage::Windows1250);
  check(t1250.has_value(), "parses as 1250 too");
  if (t1250)
    check((*t1250)[2] == "ÄöŚ", "1250 maps 0x8C to S-acute, not OE");
}

void testLevelMap() {
  std::cout << "level map\n";
  std::vector<std::byte> d;
  put32(d, 4);
  putStr(d, "E001");
  put32(d, 9);
  putStr(d, "Test Name");
  put32(d, 300);
  put32(d, 10);
  put32(d, 20);

  auto rows = webo::parseLevelMap(d);
  check(rows.has_value(), "parses");
  if (!rows)
    return;
  check(rows->size() == 1, "one row");
  check((*rows)[0].id == "E001", "id");
  check((*rows)[0].name == "Test Name", "name");
  check((*rows)[0].timeLimitSeconds == 300, "time limit");
  check((*rows)[0].saveTarget == 10, "save target");
  check((*rows)[0].totalLemmings == 20, "total lemmings");

  // A truncated record must be reported, not read off the end.
  std::vector<std::byte> truncated(d.begin(), d.begin() + 10);
  check(!webo::parseLevelMap(truncated).has_value(), "rejects a truncated record");
}

void testLevelTable() {
  std::cout << "level table\n";
  std::vector<std::byte> d;
  put16(d, 2);  // group of two slots
  // first slot carries the MFC class header
  put16(d, 0xFFFF);
  put16(d, 1);
  put16(d, 10);
  putStr(d, "CLevelSlot");
  d.push_back(std::byte{4});
  putStr(d, "ONE1");
  put32(d, 1);
  put32(d, 0);
  put32(d, 0);
  // second reuses it
  put16(d, 0x8001);
  d.push_back(std::byte{4});
  putStr(d, "TWO2");
  put32(d, 1);
  put32(d, 1);
  put32(d, 0);

  auto t = webo::parseLevelTable(d);
  check(t.has_value(), "parses");
  if (!t) {
    std::cerr << "    " << t.error() << "\n";
    return;
  }
  check(t->groups.size() == 1, "one group");
  check(t->slotCount() == 2, "two slots");
  check(t->groups[0][0].id == "ONE1" && t->groups[0][1].id == "TWO2", "ids in order");
  check(t->groups[0][1].b == 1, "trailing fields read");

  // A wrong class name should be refused rather than silently misparsed.
  auto bad = d;
  bad[10] = static_cast<std::byte>('X');
  check(!webo::parseLevelTable(bad).has_value(), "rejects an unexpected class name");
}

void testReleaseRates() {
  std::cout << "release rates\n";
  std::vector<std::byte> d;
  const double values[] = {5.0, 2.5};
  for (double v : values) {
    const auto* p = reinterpret_cast<const std::byte*>(&v);
    d.insert(d.end(), p, p + 8);
  }
  auto r = webo::parseReleaseRates(d);
  check(r.has_value() && r->size() == 2, "parses two doubles");
  if (r)
    check((*r)[0] == 5.0 && (*r)[1] == 2.5, "values");

  d.pop_back();
  check(!webo::parseReleaseRates(d).has_value(), "rejects a partial double");
}

void testRealData() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir) {
    std::cout << "real data: SKIPPED (set WEBOLUTION_GAME_DIR to enable)\n";
    return;
  }
  std::cout << "real data\n";
  auto lib = webo::AssetLibrary::open(fs::path(dir) / "lemmings.box", fs::path(dir) / "patch.box");
  if (!lib) {
    check(false, lib.error());
    return;
  }

  auto infos = webo::loadLevelMap(*lib);
  auto table = webo::loadLevelTable(*lib);
  auto rates = webo::loadReleaseRates(*lib);
  check(infos.has_value(), "LEVELMAP.DAT loads");
  check(table.has_value(), "LEVELS.MAP loads");
  check(rates.has_value(), "RELEASERATE.PC loads");
  if (!infos || !table || !rates)
    return;

  check(infos->size() == 125, std::format("125 level records (got {})", infos->size()));
  check(table->groups.size() == 5, std::format("5 groups (got {})", table->groups.size()));
  check(table->slotCount() == 125, std::format("125 slots (got {})", table->slotCount()));
  check(rates->size() == 99, std::format("99 release rates (got {})", rates->size()));

  // Rates run from slowest to fastest and stay positive.
  bool descending = true, positive = true;
  for (std::size_t i = 0; i < rates->size(); ++i) {
    if ((*rates)[i] <= 0)
      positive = false;
    if (i && (*rates)[i] > (*rates)[i - 1])
      descending = false;
  }
  check(descending, "release rates are non-increasing");
  check(positive, "release rates are positive");

  // Every slot must resolve to a record, or the level list is unusable.
  std::vector<std::string> ids;
  for (const auto& i : *infos)
    ids.push_back(i.id);
  std::size_t unresolved = 0;
  for (const auto& g : table->groups)
    for (const auto& s : g)
      if (std::find(ids.begin(), ids.end(), s.id) == ids.end())
        ++unresolved;
  check(unresolved == 0, std::format("every slot resolves ({} did not)", unresolved));

  const auto langs = webo::availableLanguages(*lib);
  check(langs.size() >= 6, std::format("several languages ({})", langs.size()));
  for (const auto& l : langs) {
    auto t = webo::loadStrings(*lib, l);
    check(t.has_value(), std::format("{} loads", l));
    if (t)
      check(t->size() > 100, std::format("{} has strings ({})", l, t->size()));
  }
}

}  // namespace

int main() {
  testCodepageSelection();
  testStringTable();
  testLevelMap();
  testLevelTable();
  testReleaseRates();
  testRealData();

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
