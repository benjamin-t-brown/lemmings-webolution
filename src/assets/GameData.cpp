#include "GameData.h"

#include "Box.h"

#include <cstring>
#include <format>

namespace webo {
namespace {

class Reader {
public:
  explicit Reader(std::span<const std::byte> data) : d_(data) {}

  bool ok() const { return ok_; }
  std::size_t pos() const { return pos_; }
  std::size_t remaining() const { return ok_ ? d_.size() - pos_ : 0; }
  bool atEnd() const { return pos_ >= d_.size(); }
  const std::string& error() const { return error_; }

  std::uint8_t u8() {
    if (!need(1))
      return 0;
    return std::to_integer<std::uint8_t>(d_[pos_++]);
  }
  std::uint16_t u16() {
    if (!need(2))
      return 0;
    std::uint16_t v;
    std::memcpy(&v, d_.data() + pos_, 2);
    pos_ += 2;
    return v;
  }
  std::uint32_t u32() {
    if (!need(4))
      return 0;
    std::uint32_t v;
    std::memcpy(&v, d_.data() + pos_, 4);
    pos_ += 4;
    return v;
  }
  double f64() {
    if (!need(8))
      return 0;
    double v;
    std::memcpy(&v, d_.data() + pos_, 8);
    pos_ += 8;
    return v;
  }
  std::string bytes(std::size_t n) {
    if (!need(n))
      return {};
    std::string s(reinterpret_cast<const char*>(d_.data() + pos_), n);
    pos_ += n;
    return s;
  }

  void fail(std::string what) {
    if (ok_) {
      ok_ = false;
      error_ = std::move(what);
    }
  }

private:
  bool need(std::size_t n) {
    if (!ok_)
      return false;
    if (pos_ + n > d_.size()) {
      fail(std::format("ran past the end of the file at offset {}", pos_));
      return false;
    }
    return true;
  }

  std::span<const std::byte> d_;
  std::size_t pos_ = 0;
  bool ok_ = true;
  std::string error_;
};

// Each localisation was authored for the Windows ANSI codepage of its locale,
// so a single table would silently corrupt three of the eight languages:
// Polish and Czech need 1250, Hebrew needs 1255. The tables are generated
// rather than transcribed -- see codepages.inc.
#include "codepages.inc"

std::string toUtf8(std::string_view in, Codepage cp) {
  const std::uint16_t* high = kCp1252;
  if (cp == Codepage::Windows1250)
    high = kCp1250;
  else if (cp == Codepage::Windows1255)
    high = kCp1255;

  std::string out;
  out.reserve(in.size());
  for (unsigned char c : in) {
    const std::uint32_t code = (c < 0x80) ? c : high[c - 0x80];
    if (code < 0x80) {
      out.push_back(static_cast<char>(code));
    } else if (code < 0x800) {
      out.push_back(static_cast<char>(0xC0 | (code >> 6)));
      out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    } else {
      out.push_back(static_cast<char>(0xE0 | (code >> 12)));
      out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    }
  }
  return out;
}

constexpr const char* kLevelMapPath = "COMPILED\\LEVELMAP.DAT";
constexpr const char* kLevelTablePath = "LEVELS\\LEVELS.MAP";
constexpr const char* kReleaseRatePath = "COMPILED\\RELEASERATE.PC";

}  // namespace

Codepage codepageForLanguage(std::string_view language) {
  const auto upper = normaliseAssetName(language);
  if (upper == "POLISH" || upper == "CZECH")
    return Codepage::Windows1250;
  if (upper == "HEBREW")
    return Codepage::Windows1255;
  return Codepage::Windows1252;
}

std::size_t LevelTable::slotCount() const {
  std::size_t n = 0;
  for (const auto& g : groups)
    n += g.size();
  return n;
}

std::string_view StringTable::operator[](std::size_t index) const {
  if (index >= strings_.size())
    return {};
  return strings_[index];
}

std::expected<StringTable, std::string> StringTable::parse(std::span<const std::byte> ltx,
                                                          Codepage cp) {
  Reader r(ltx);
  const std::uint32_t count = r.u32();
  if (!r.ok())
    return std::unexpected(r.error());
  if (count > 100000)
    return std::unexpected(std::format("implausible string count {}", count));

  std::vector<std::uint32_t> offsets(count);
  for (std::uint32_t i = 0; i < count; ++i)
    offsets[i] = r.u32();
  if (!r.ok())
    return std::unexpected(r.error());

  StringTable t;
  t.strings_.reserve(count);
  for (std::uint32_t i = 0; i < count; ++i) {
    const std::uint32_t off = offsets[i];
    if (off > ltx.size())
      return std::unexpected(std::format("string {} starts past the end ({})", i, off));
    // NUL terminated; tolerate a final string that runs to the end of file.
    std::size_t end = off;
    while (end < ltx.size() && std::to_integer<char>(ltx[end]) != '\0')
      ++end;
    const auto* p = reinterpret_cast<const char*>(ltx.data() + off);
    t.strings_.push_back(toUtf8(std::string_view(p, end - off), cp));
  }
  return t;
}

std::expected<std::vector<LevelInfo>, std::string> parseLevelMap(std::span<const std::byte> d) {
  Reader r(d);
  std::vector<LevelInfo> out;
  while (!r.atEnd()) {
    LevelInfo info;
    const std::uint32_t idLen = r.u32();
    if (!r.ok())
      return std::unexpected(r.error());
    if (idLen > 4096)
      return std::unexpected(std::format("implausible id length {} at {}", idLen, r.pos()));
    info.id = r.bytes(idLen);

    const std::uint32_t nameLen = r.u32();
    if (!r.ok())
      return std::unexpected(r.error());
    if (nameLen > 4096)
      return std::unexpected(std::format("implausible name length {} at {}", nameLen, r.pos()));
    info.name = toUtf8(r.bytes(nameLen), Codepage::Windows1252);

    info.timeLimitSeconds = r.u32();
    info.saveTarget = r.u32();
    info.totalLemmings = r.u32();
    if (!r.ok())
      return std::unexpected(r.error());
    out.push_back(std::move(info));
  }
  return out;
}

std::expected<LevelTable, std::string> parseLevelTable(std::span<const std::byte> d) {
  Reader r(d);
  LevelTable table;

  // A sequence of counted groups. Each group restarts MFC's per-archive class
  // table, so the first slot in it carries the full class header and the rest
  // use the "same class as before" tag.
  while (!r.atEnd()) {
    const std::uint16_t count = r.u16();
    if (!r.ok())
      return std::unexpected(r.error());
    if (count == 0 || count > 10000)
      return std::unexpected(std::format("implausible group size {} at {}", count, r.pos()));

    std::vector<LevelSlot> group;
    group.reserve(count);
    for (std::uint16_t i = 0; i < count; ++i) {
      const std::uint16_t tag = r.u16();
      if (!r.ok())
        return std::unexpected(r.error());
      if (tag == 0xFFFF) {
        r.u16();  // schema version
        const std::uint16_t nameLen = r.u16();
        if (!r.ok())
          return std::unexpected(r.error());
        const std::string cls = r.bytes(nameLen);
        if (cls != "CLevelSlot")
          return std::unexpected(std::format("unexpected serialised class '{}'", cls));
      } else if (tag != 0x8001) {
        return std::unexpected(
            std::format("unexpected object tag 0x{:04x} at {}", tag, r.pos() - 2));
      }

      LevelSlot slot;
      const std::uint8_t len = r.u8();
      slot.id = r.bytes(len);
      slot.a = r.u32();
      slot.b = r.u32();
      slot.c = r.u32();
      if (!r.ok())
        return std::unexpected(r.error());
      group.push_back(std::move(slot));
    }
    table.groups.push_back(std::move(group));
  }
  return table;
}

std::expected<std::vector<double>, std::string> parseReleaseRates(std::span<const std::byte> d) {
  if (d.size() % 8 != 0)
    return std::unexpected(std::format("{} bytes is not a whole number of doubles", d.size()));
  Reader r(d);
  std::vector<double> out(d.size() / 8);
  for (auto& v : out)
    v = r.f64();
  if (!r.ok())
    return std::unexpected(r.error());
  return out;
}

namespace {

template <typename Fn>
auto loadWith(const AssetLibrary& lib, const char* path, Fn parse)
    -> decltype(parse(std::span<const std::byte>{})) {
  auto bytes = lib.read(path);
  if (!bytes)
    return std::unexpected(bytes.error());
  auto parsed = parse(std::span<const std::byte>(*bytes));
  if (!parsed)
    return std::unexpected(std::format("{}: {}", path, parsed.error()));
  return parsed;
}

}  // namespace

std::expected<std::vector<LevelInfo>, std::string> loadLevelMap(const AssetLibrary& lib) {
  return loadWith(lib, kLevelMapPath, parseLevelMap);
}

std::expected<LevelTable, std::string> loadLevelTable(const AssetLibrary& lib) {
  return loadWith(lib, kLevelTablePath, parseLevelTable);
}

std::expected<std::vector<double>, std::string> loadReleaseRates(const AssetLibrary& lib) {
  return loadWith(lib, kReleaseRatePath, parseReleaseRates);
}

double ReleaseRates::secondsPerLemming(int rate) const {
  if (seconds_.empty())
    return 0;
  const int last = static_cast<int>(seconds_.size());
  const int clamped = rate < 1 ? 1 : (rate > last ? last : rate);
  return seconds_[static_cast<std::size_t>(clamped - 1)];
}

int ReleaseRates::intervalTicks(int rate, int ticksPerSecond) const {
  if (seconds_.empty() || ticksPerSecond <= 0)
    return 1;
  const double ticks = secondsPerLemming(rate) * ticksPerSecond;
  const int rounded = static_cast<int>(ticks + 0.5);
  return rounded < 1 ? 1 : rounded;
}

std::expected<ReleaseRates, std::string> loadReleaseRateTable(const AssetLibrary& lib) {
  auto rates = loadReleaseRates(lib);
  if (!rates)
    return std::unexpected(rates.error());
  return ReleaseRates(std::move(*rates));
}

std::expected<StringTable, std::string> loadStrings(const AssetLibrary& lib,
                                                    std::string_view language) {
  const std::string path = std::format("LANGUAGE\\{}.LTX", language);
  auto bytes = lib.read(path);
  if (!bytes)
    return std::unexpected(bytes.error());
  auto parsed = StringTable::parse(*bytes, codepageForLanguage(language));
  if (!parsed)
    return std::unexpected(std::format("{}: {}", path, parsed.error()));
  return parsed;
}

std::vector<std::string> availableLanguages(const AssetLibrary& lib) {
  std::vector<std::string> out;
  for (const auto& name : lib.names()) {
    const auto upper = normaliseAssetName(name);
    if (!upper.starts_with("LANGUAGE\\") || !upper.ends_with(".LTX"))
      continue;
    out.push_back(upper.substr(9, upper.size() - 9 - 4));
  }
  return out;
}

}  // namespace webo
