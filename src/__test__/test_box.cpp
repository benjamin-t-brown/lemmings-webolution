// Tests for the LEMBOX reader.
//
// Pure-format tests run everywhere against archives synthesised in a temp dir.
// Tests that need the real thing read WEBOLUTION_GAME_DIR and report a skip
// when it is unset, so the suite stays green on a machine without the game.

#include "assets/Box.h"

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
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

template <typename A, typename B>
void checkEq(const A& a, const B& b, std::string_view what) {
  ++g_checks;
  if (!(a == b)) {
    ++g_failures;
    std::cerr << std::format("  FAIL  {}: {} != {}\n", what, a, b);
  }
}

void put32(std::vector<std::byte>& v, std::uint32_t n) {
  for (int i = 0; i < 4; ++i)
    v.push_back(static_cast<std::byte>((n >> (8 * i)) & 0xFF));
}

void putBytes(std::vector<std::byte>& v, std::string_view s) {
  for (char c : s)
    v.push_back(static_cast<std::byte>(c));
}

// Build a minimal but genuine LEMBOX archive.
std::vector<std::byte> makeBox(const std::vector<std::pair<std::string, std::string>>& files) {
  std::vector<std::byte> names;
  for (const auto& [name, _] : files) {
    put32(names, static_cast<std::uint32_t>(name.size()));
    putBytes(names, name);
  }

  const std::uint32_t count = static_cast<std::uint32_t>(files.size());
  const std::size_t headerAndNames = 14 + names.size();
  const std::size_t tables = 4 + count * 4u + 4 + count * 4u;
  std::uint32_t cursor = static_cast<std::uint32_t>(headerAndNames + tables);

  std::vector<std::uint32_t> offsets, sizes;
  for (const auto& [_, body] : files) {
    offsets.push_back(cursor);
    sizes.push_back(static_cast<std::uint32_t>(body.size()));
    cursor += static_cast<std::uint32_t>(body.size());
  }

  std::vector<std::byte> out;
  putBytes(out, "LEMBOX");
  put32(out, count);
  put32(out, static_cast<std::uint32_t>(names.size()));
  out.insert(out.end(), names.begin(), names.end());
  put32(out, count);
  for (auto o : offsets)
    put32(out, o);
  put32(out, count);
  for (auto s : sizes)
    put32(out, s);
  for (const auto& [_, body] : files)
    putBytes(out, body);
  return out;
}

// std::filesystem::temp_directory_path() just reads TMP/TEMP/TMPDIR, and when
// none are set it happily returns C:\WINDOWS\, which is not writable. That is
// exactly what happens when the suite is launched from make rather than a
// shell. Pick a directory we have actually proven we can write to.
const fs::path& scratchDir() {
  static const fs::path dir = [] {
    std::error_code ec;
    std::vector<fs::path> candidates;
    if (auto t = fs::temp_directory_path(ec); !ec)
      candidates.push_back(t);
    candidates.push_back(fs::current_path(ec));

    for (const auto& base : candidates) {
      const fs::path d = base / "webolution_test";
      fs::create_directories(d, ec);
      std::ofstream probe(d / ".probe");
      if (probe) {
        probe << "ok";
        probe.close();
        fs::remove(d / ".probe", ec);
        return d;
      }
    }
    std::cerr << "  FATAL  no writable scratch directory\n";
    std::exit(2);
  }();
  return dir;
}

fs::path writeTemp(const std::string& stem, const std::vector<std::byte>& data) {
  const fs::path p = scratchDir() / std::format("{}.box", stem);
  std::ofstream f(p, std::ios::binary);
  f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
  f.close();
  if (!f || !fs::exists(p)) {
    std::cerr << std::format("  FATAL  cannot write scratch file {}\n", p.string());
    std::exit(2);
  }
  return p;
}

std::string toString(std::span<const std::byte> b) {
  return std::string(reinterpret_cast<const char*>(b.data()), b.size());
}

void testNameNormalisation() {
  std::cout << "name normalisation\n";
  checkEq(webo::normaliseAssetName("Sprites/Bits.spr"), std::string("SPRITES\\BITS.SPR"),
          "slash and case folded");
  checkEq(webo::normaliseAssetName("A\\b"), std::string("A\\B"), "backslash preserved");
}

void testSyntheticArchive() {
  std::cout << "synthetic archive\n";
  const auto data = makeBox({
      {"SPRITES\\BITS.SPR", "sprite-payload"},
      {"SOUNDS\\POP.WAV", "RIFFwave"},
      {"MUSIC\\LEMMING01.WAV", ""},
  });
  const auto path = writeTemp("basic", data);

  // Box holds the archive open for the life of the object -- reads seek into a
  // 300 MB file, so reopening per read would be absurd. Scope it so the handle
  // is closed before we delete the temp file; Windows refuses otherwise.
  {
  auto box = webo::Box::open(path);
  check(box.has_value(), "opens");
  if (!box) {
    std::cerr << "    " << box.error() << "\n";
    fs::remove(path);
    return;
  }

  checkEq(box->count(), std::size_t{3}, "entry count");

  const auto v = box->validate();
  check(v.ok, "layout validates");
  checkEq(v.gaps, std::size_t{0}, "no gaps");
  checkEq(v.payloadEnd, v.fileSize, "payload covers the file exactly");

  auto a = box->read("SPRITES\\BITS.SPR");
  check(a.has_value() && toString(*a) == "sprite-payload", "reads by exact name");

  auto b = box->read("sprites/bits.spr");
  check(b.has_value() && toString(*b) == "sprite-payload", "reads case/separator insensitively");

  auto empty = box->read("MUSIC\\LEMMING01.WAV");
  check(empty.has_value() && empty->empty(), "zero-length entry");

  check(!box->read("NOPE.BMP").has_value(), "missing entry is an error, not a crash");
  check(box->contains("SOUNDS\\POP.WAV"), "contains()");
  }

  fs::remove(path);
}

void testRejectsGarbage() {
  std::cout << "malformed input\n";
  std::vector<std::byte> bad;
  putBytes(bad, "NOTLEMBOX and then some filler bytes");
  const auto path = writeTemp("bad", bad);
  auto box = webo::Box::open(path);
  check(!box.has_value(), "rejects wrong magic");
  fs::remove(path);

  // Correct magic, but a count that would overrun the name block.
  std::vector<std::byte> trunc;
  putBytes(trunc, "LEMBOX");
  put32(trunc, 4);   // claims 4 entries
  put32(trunc, 8);   // but only 8 bytes of names
  put32(trunc, 100); // one bogus length
  putBytes(trunc, "abcd");
  const auto p2 = writeTemp("trunc", trunc);
  auto box2 = webo::Box::open(p2);
  check(!box2.has_value(), "rejects overrunning name block");
  fs::remove(p2);
}

void testPatchOverlay() {
  std::cout << "patch overlay\n";
  const auto basePath = writeTemp("ovl_base", makeBox({
                                                  {"A.TXT", "base-A"},
                                                  {"B.TXT", "base-B"},
                                              }));
  const auto patchPath = writeTemp("ovl_patch", makeBox({
                                                   {"B.TXT", "patch-B"},
                                                   {"C.TXT", "patch-C"},
                                               }));

  {
  auto lib = webo::AssetLibrary::open(basePath, patchPath);
  check(lib.has_value(), "opens overlay");
  if (lib) {
    auto a = lib->read("A.TXT");
    check(a && toString(*a) == "base-A", "base-only entry comes from base");
    auto b = lib->read("B.TXT");
    check(b && toString(*b) == "patch-B", "patch overrides base");
    auto c = lib->read("C.TXT");
    check(c && toString(*c) == "patch-C", "patch-only entry is visible");
    checkEq(lib->overrideCount(), std::size_t{1}, "override count");
    checkEq(lib->names().size(), std::size_t{3}, "union of names");
  }

  // A missing patch is not an error; the base stands alone.
  auto noPatch = webo::AssetLibrary::open(basePath, basePath.parent_path() / "absent.box");
  check(noPatch.has_value(), "absent patch is tolerated");
  check(noPatch && noPatch->patch() == nullptr, "no patch reported");
  }

  fs::remove(basePath);
  fs::remove(patchPath);
}

void testRealArchive() {
  const char* dir = std::getenv("WEBOLUTION_GAME_DIR");
  if (!dir || !*dir) {
    std::cout << "real archive: SKIPPED (set WEBOLUTION_GAME_DIR to enable)\n";
    return;
  }
  std::cout << "real archive (" << dir << ")\n";

  const fs::path base = fs::path(dir) / "lemmings.box";
  if (!fs::exists(base)) {
    std::cerr << "  FAIL  no lemmings.box in WEBOLUTION_GAME_DIR\n";
    ++g_failures;
    return;
  }

  auto lib = webo::AssetLibrary::open(base, fs::path(dir) / "patch.box");
  check(lib.has_value(), "opens the real archives");
  if (!lib) {
    std::cerr << "    " << lib.error() << "\n";
    return;
  }

  const auto v = lib->base().validate();
  check(v.ok, "lemmings.box layout validates");
  checkEq(v.gaps, std::size_t{0}, "lemmings.box has no gaps");
  checkEq(v.payloadEnd, v.fileSize, "lemmings.box payload covers the file exactly");
  std::cout << std::format("    {} entries, {} bytes\n", lib->base().count(), v.fileSize);

  // A known-good entry: music is plain RIFF, so we can check real content.
  auto wav = lib->read("MUSIC\\LEMMING01.WAV");
  check(wav.has_value(), "reads MUSIC\\LEMMING01.WAV");
  if (wav) {
    check(wav->size() > 44, "wav has more than a header");
    check(std::memcmp(wav->data(), "RIFF", 4) == 0, "wav starts with RIFF");
    check(std::memcmp(wav->data() + 8, "WAVE", 4) == 0, "wav is WAVE");
  }

  if (const webo::Box* p = lib->patch()) {
    const auto pv = p->validate();
    check(pv.ok, "patch.box layout validates");
    checkEq(pv.payloadEnd, pv.fileSize, "patch.box payload covers the file exactly");
    check(lib->overrideCount() > 0, "patch overrides at least one base entry");
    std::cout << std::format("    patch: {} entries, {} override the base\n", p->count(),
                             lib->overrideCount());
  }

  // Every entry must be readable and the right length.
  std::size_t checked = 0;
  for (const auto& e : lib->base().entries()) {
    auto d = lib->base().read(e);
    if (!d || d->size() != e.size) {
      std::cerr << std::format("  FAIL  entry {} unreadable or wrong size\n", e.name);
      ++g_failures;
      break;
    }
    if (++checked >= 500)
      break;  // enough to prove the offsets line up without reading 300 MB
  }
  ++g_checks;
  std::cout << std::format("    spot-read {} entries\n", checked);
}

}  // namespace

int main() {
  testNameNormalisation();
  testSyntheticArchive();
  testRejectsGarbage();
  testPatchOverlay();
  testRealArchive();

  std::error_code ec;
  fs::remove_all(scratchDir(), ec);

  std::cout << std::format("\n{} checks, {} failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
