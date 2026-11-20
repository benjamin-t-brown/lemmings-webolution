// Check the SavedGame.dat codec, storage adapter and node mapping.
// The original layout for nonzero fields comes from 0x45F43C / 0x45F94D.
// Read the installed sample and executable as bytes. Write only to temporary storage.

#include "app/MapScene.h"
#include "save/SaveStore.h"
#include "save/SavedGame.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;
using namespace webo;

namespace {
int checks = 0, failures = 0;
void check(bool ok, std::string_view what) {
  ++checks;
  if (!ok) { ++failures; std::cerr << "FAIL: " << what << '\n'; }
}

std::string sha256Hex(const std::vector<std::uint8_t>& data) {
  static constexpr std::uint32_t k[64] = {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
      0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
      0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
      0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
      0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
      0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
      0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
      0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
  auto rotr = [](std::uint32_t x, int n) { return (x >> n) | (x << (32 - n)); };
  std::uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  std::vector<std::uint8_t> msg(data);
  const std::uint64_t bitLen = static_cast<std::uint64_t>(msg.size()) * 8;
  msg.push_back(0x80);
  while ((msg.size() % 64) != 56) msg.push_back(0);
  for (int i = 7; i >= 0; --i) msg.push_back(static_cast<std::uint8_t>((bitLen >> (i * 8)) & 0xFF));
  for (std::size_t off = 0; off < msg.size(); off += 64) {
    std::uint32_t w[64];
    for (std::size_t i = 0; i < 16; ++i)
      w[i] = (static_cast<std::uint32_t>(msg[off + i * 4]) << 24) |
             (static_cast<std::uint32_t>(msg[off + i * 4 + 1]) << 16) |
             (static_cast<std::uint32_t>(msg[off + i * 4 + 2]) << 8) | msg[off + i * 4 + 3];
    for (std::size_t i = 16; i < 64; ++i) {
      const std::uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
      const std::uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
      w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
    for (std::size_t i = 0; i < 64; ++i) {
      const std::uint32_t t1 = hh + (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)) + ((e & f) ^ (~e & g)) + k[i] + w[i];
      const std::uint32_t t2 = (rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
      hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
  }
  std::string out;
  for (const auto word : h) out += std::format("{:08x}", word);
  return out;
}

std::vector<std::uint8_t> readFile(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

std::vector<std::byte> craftedImage() {
  std::vector<std::byte> bytes(save::kSavedGameSize);
  for (std::size_t i = 0; i < bytes.size(); ++i)
    bytes[i] = static_cast<std::byte>((i * 37 + 11) & 0xFF);
  return bytes;
}

std::vector<std::size_t> differingOffsets(const std::vector<std::byte>& a, const std::vector<std::byte>& b) {
  std::vector<std::size_t> out;
  for (std::size_t i = 0; i < std::min(a.size(), b.size()); ++i)
    if (a[i] != b[i]) out.push_back(i);
  return out;
}

// Source-test sink: injected failure and an in-memory store.
struct FailingSink final : save::SaveSink {
  std::expected<std::optional<std::vector<std::byte>>, std::string> read() override {
    return std::optional<std::vector<std::byte>>{};
  }
  std::expected<void, std::string> write(std::span<const std::byte>) override {
    ++writes;
    return std::unexpected("injected write failure");
  }
  int writes = 0;
};

void testCodec() {
  const auto zero = save::SavedGame{}.serialize();
  check(zero.size() == 1504 && save::kSavedGameSize == 1504, "serialized image is 1504 bytes");
  check(std::ranges::all_of(zero, [](std::byte b) { return b == std::byte{0}; }),
        "default image is all zero (the installed-sample shape)");

  for (const std::size_t n : {std::size_t{0}, std::size_t{3}, std::size_t{4}, std::size_t{1503}}) {
    const std::vector<std::byte> shortImage(n);
    const auto parsed = save::SavedGame::parse(shortImage);
    check(!parsed && parsed.error().size == n &&
          parsed.error().kind == (n == 0 ? save::SavedGameError::Kind::Empty
                                         : save::SavedGameError::Kind::TooShort) &&
          !parsed.error().message().empty(),
          std::format("size {} is rejected with a specific error", n));
  }
  for (const std::size_t n : {std::size_t{1505}, std::size_t{4096}}) {
    const auto parsed = save::SavedGame::parse(std::vector<std::byte>(n));
    check(!parsed && parsed.error().kind == save::SavedGameError::Kind::TooLong &&
          parsed.error().size == n, std::format("oversize {} is rejected", n));
  }

  // Byte-identical round trip of a crafted image: nonzero flag, A, B, C (including
  // values other than 1 and negatives) and all unmapped records.
  const auto crafted = craftedImage();
  const auto parsed = save::SavedGame::parse(crafted);
  check(parsed.has_value(), "crafted 1504-byte image parses");
  if (parsed) {
    check(parsed->serialize() == crafted, "crafted image round-trips byte-identically");
    // Read little-endian fields directly from the original save layout.
    const auto at = [&](std::size_t off) {
      return static_cast<std::int32_t>(
          static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(crafted[off])) |
          (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(crafted[off + 1])) << 8) |
          (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(crafted[off + 2])) << 16) |
          (static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(crafted[off + 3])) << 24));
    };
    check(parsed->flagRaw == at(0), "flag is the int32 at 0x000");
    bool fieldsOk = true;
    for (std::size_t n : {std::size_t{0}, std::size_t{1}, std::size_t{37}, std::size_t{124}}) {
      fieldsOk &= parsed->records[n].a == at(4 + 12 * n) && parsed->records[n].b == at(8 + 12 * n) &&
                  parsed->records[n].c == at(12 + 12 * n);
    }
    check(fieldsOk, "A, B, C are the int32 values at 0x004 + 12n + {0, 4, 8}");
  }

  // Endianness is explicit: 0x01020304 is stored 04 03 02 01 on any host.
  save::SavedGame endian;
  endian.flagRaw = 0x01020304;
  endian.records[0].c = -2;
  const auto eb = endian.serialize();
  check(eb[0] == std::byte{4} && eb[1] == std::byte{3} && eb[2] == std::byte{2} && eb[3] == std::byte{1},
        "flag is serialized little-endian");
  check(eb[12] == std::byte{0xFE} && eb[13] == std::byte{0xFF} && eb[14] == std::byte{0xFF} &&
        eb[15] == std::byte{0xFF}, "C = -2 of record 0 is serialized little-endian at 0x00C");

  // Completing (0, 0) differs from the zero image only at 0x00C.
  save::SavedGame completed;
  check(completed.markComplete(0, 0), "markComplete(0,0)");
  const auto cb = completed.serialize();
  check(differingOffsets(zero, cb) == std::vector<std::size_t>{0x00C} && cb[0x00C] == std::byte{1},
        "completing (0,0) changes only offset 0x00C to 1");
  check(!completed.markComplete(5, 0) && !completed.markComplete(0, 25) &&
        !completed.markComplete(-1, 0) && !completed.markComplete(0, -1),
        "out-of-range (group, slot) is refused");
}

void testMapping() {
  const auto& mapping = app::MapScene::nodeMapping();
  std::set<std::size_t> mapped;
  bool keysValid = true;
  for (const auto& entry : mapping) {
    keysValid &= save::SavedGame::validKey(entry[0], entry[1]);
    mapped.insert(save::SavedGame::recordIndex(entry[0], entry[1]));
  }
  check(keysValid, "every mapped node names a valid record");
  check(mapped.size() == 102 && save::kRecordCount - mapped.size() == 23,
        "102 distinct mapped records leave 23 unmapped");
  check(mapping[11][0] == 4 && mapping[11][1] == 4, "node 11 is group 4 slot 4");

  // Mapped completion projects to node flags; unmapped records are ignored by the
  // projection yet survive a merge verbatim.
  auto image = *save::SavedGame::parse(craftedImage());
  for (std::size_t n = 0; n < save::kRecordCount; ++n) image.records[n].c = 0;
  std::vector<save::SaveRecord> unmappedBefore;
  for (std::size_t n = 0; n < save::kRecordCount; ++n)
    if (!mapped.contains(n)) { image.records[n].c = 9; unmappedBefore.push_back(image.records[n]); }
  check(unmappedBefore.size() == 23, "23 unmapped records collected");
  const auto flags = image.nodeFlags(mapping);
  check(std::ranges::all_of(flags, [](std::int32_t f) { return f == 0; }),
        "nonzero C on unmapped records does not create node flags");
  image.markComplete(mapping[11][0], mapping[11][1]);
  const auto flags2 = image.nodeFlags(mapping);
  check(flags2[11] == 1 && std::count(flags2.begin(), flags2.end(), 1) == 1,
        "completing group 4 slot 4 sets exactly node 11");

  auto merged = image;
  auto wanted = flags2;
  wanted[0] = 1;
  merged.mergeNodeFlags(mapping, wanted);
  check(merged.records[save::SavedGame::recordIndex(mapping[0][0], mapping[0][1])].c == 1 &&
        merged.nodeFlags(mapping)[0] == 1 && merged.nodeFlags(mapping)[11] == 1,
        "merge sets C by (group, slot)");
  std::vector<save::SaveRecord> unmappedAfter;
  for (std::size_t n = 0; n < save::kRecordCount; ++n)
    if (!mapped.contains(n)) unmappedAfter.push_back(merged.records[n]);
  check(unmappedAfter == unmappedBefore, "unmapped records survive a merge verbatim");
  check(merged.flagRaw == image.flagRaw, "flag survives a merge verbatim");

  // A nonzero C other than 1 is kept when the node stays complete.
  save::SavedGame odd;
  odd.records[0].c = 7;
  auto oddFlags = odd.nodeFlags(mapping);
  check(oddFlags[0] == 1, "C = 7 projects to a set flag");
  odd.mergeNodeFlags(mapping, oddFlags);
  check(odd.records[0].c == 7, "merge keeps an existing nonzero C verbatim");
}

#ifdef WEBOLUTION_SOURCE_DIR
void testReferenceTable() {
  const fs::path exe = fs::path(WEBOLUTION_SOURCE_DIR) / "Lemmings Revolution" / "Lemmings Revolution.exe";
  if (!fs::is_regular_file(exe)) {
    std::cout << "SavedGame mapping: reference executable not present, mapping check SKIPPED\n";
    return;
  }
  // Read-only: hash first, and read the table only when the registered digest matches.
  const auto bytes = readFile(exe);
  const auto digest = sha256Hex(bytes);
  check(digest == "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f",
        "reference executable matches the registered SHA-256 (" + digest + ")");
  if (digest != "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f") return;
  constexpr std::size_t tableOffset = 0xB3948, entrySize = 12, entries = 102;  // VA 0x4B3948
  if (bytes.size() < tableOffset + entrySize * entries) { check(false, "table lies inside the executable"); return; }
  const auto rd32 = [&](std::size_t off) {
    return static_cast<std::int32_t>(
        static_cast<std::uint32_t>(bytes[off]) | (static_cast<std::uint32_t>(bytes[off + 1]) << 8) |
        (static_cast<std::uint32_t>(bytes[off + 2]) << 16) | (static_cast<std::uint32_t>(bytes[off + 3]) << 24));
  };
  const auto& mapping = app::MapScene::nodeMapping();
  int differences = 0;
  for (std::size_t node = 0; node < entries; ++node) {
    const std::size_t off = tableOffset + node * entrySize;
    const std::array<std::int32_t, 3> exeEntry{rd32(off), rd32(off + 4), static_cast<std::int32_t>(bytes[off + 8])};
    if (exeEntry != mapping[node]) {
      ++differences;
      std::cerr << std::format("MAPPING DIFFERENCE node {}: exe {{{}, {}, {}}} clone {{{}, {}, {}}}\n", node,
                               exeEntry[0], exeEntry[1], exeEntry[2], mapping[node][0], mapping[node][1],
                               mapping[node][2]);
    }
  }
  check(differences == 0, "MapScene mapping equals the 102 entries at VA 0x4B3948 (U3)");
}
#endif

void testStore(const fs::path& root) {
  const fs::path dir = root / "LemmingsWebolution";
  const fs::path file = save::cloneSavePathUnder(root);
  check(file == dir / "SavedGame.dat", "clone path is <LocalAppData>/LemmingsWebolution/SavedGame.dat");

  save::SaveStore store(std::make_unique<save::FileSaveSink>(file));
  auto fresh = store.load();
  check(fresh && fresh->serialize() == save::SavedGame{}.serialize() && !fs::exists(file),
        "missing file loads as the zero state and creates nothing");

  save::SavedGame game;
  game.flagRaw = 5;
  game.records[3] = {11, 22, 33};
  game.records[124] = {-1, -2, 4};
  game.markComplete(0, 0);
  check(store.save(game).has_value(), "save creates the directory and file");
  check(fs::file_size(file) == 1504 && readFile(file).size() == 1504, "stored file is 1504 bytes");
  check(!fs::exists(file.string() + ".tmp"), "no temporary file is left behind");
  auto reloaded = store.load();
  check(reloaded && reloaded->serialize() == game.serialize(), "load returns exactly what was saved");

  // Corrupt sizes: load fails and never modifies the file.
  for (const std::size_t n : {std::size_t{0}, std::size_t{3}, std::size_t{1503}, std::size_t{1505}}) {
    {
      std::ofstream out(file, std::ios::binary | std::ios::trunc);
      std::string junk(n, 'x');
      out.write(junk.data(), static_cast<std::streamsize>(junk.size()));
    }
    const auto before = readFile(file);
    const auto loaded = store.load();
    check(!loaded && !loaded.error().empty() && readFile(file) == before,
          std::format("a {}-byte file fails to load and is not rewritten", n));
  }

  // Restore a good file, then make every write fail in different ways.
  check(store.save(game).has_value(), "a successful save replaces a corrupt file");
  const auto good = readFile(file);
  check(good == [&] { std::vector<std::uint8_t> v; for (auto b : game.serialize()) v.push_back(std::to_integer<std::uint8_t>(b)); return v; }(),
        "replaced file holds the new image");

  save::SavedGame next = game;
  next.markComplete(1, 1);
  fs::create_directory(file.string() + ".tmp");  // temp file cannot be created
  check(!store.save(next).has_value() && readFile(file) == good,
        "failed write (temp path blocked) keeps the prior good file");
  fs::remove(file.string() + ".tmp");

  save::SaveStore failingStore(std::make_unique<FailingSink>());
  check(!failingStore.save(next).has_value(), "injected failing sink reports an error");

  const fs::path blocker = root / "blocker";
  { std::ofstream out(blocker, std::ios::binary); out << "not a directory"; }
  save::SaveStore blocked(std::make_unique<save::FileSaveSink>(blocker / "sub" / "SavedGame.dat"));
  check(!blocked.save(next).has_value(), "unusable directory (parent is a file) reports an error");

  // The destination cannot be replaced (it is a directory): error, no stray temp.
  const fs::path dirTarget = root / "asdir" / "SavedGame.dat";
  fs::create_directories(dirTarget);
  save::SaveStore dirStore(std::make_unique<save::FileSaveSink>(dirTarget));
  check(!dirStore.save(next).has_value() && fs::is_directory(dirTarget) &&
        !fs::exists(dirTarget.string() + ".tmp"), "unreplaceable destination reports an error and cleans up");
  check(!dirStore.load().has_value(), "a non-regular destination is a load error, not a missing file");

  // Check that storage rejects both reads and writes in the installation directory.
  const fs::path installed = root / "installed";
  fs::create_directories(installed);
  save::FileSaveSink guarded(installed / "SavedGame.dat", installed);
  check(!guarded.write(save::SavedGame{}.serialize()).has_value() && !fs::exists(installed / "SavedGame.dat"),
        "a save path inside the installed game directory is refused for writing");
  check(!guarded.read().has_value(), "a save path inside the installed game directory is refused for reading");
  save::FileSaveSink nested(installed / "x" / "y" / "SavedGame.dat", installed);
  check(!nested.write(save::SavedGame{}.serialize()).has_value() && !fs::exists(installed / "x"),
        "nested paths under the installed directory are refused");
  check(save::pathIsInside(installed / "a", installed) && !save::pathIsInside(root / "installed2" / "a", installed) &&
        !save::pathIsInside(root / "other", installed), "pathIsInside matches whole components only");
}
}  // namespace

int main() {
  testCodec();
  testMapping();
#ifdef WEBOLUTION_SOURCE_DIR
  // The installed save is only hashed, never opened for writing.
  const fs::path installedSave = fs::path(WEBOLUTION_SOURCE_DIR) / "Lemmings Revolution" / "SavedGame.dat";
  const auto installedBefore = fs::is_regular_file(installedSave) ? sha256Hex(readFile(installedSave)) : std::string();
  testReferenceTable();
#endif

  std::error_code ec;
  const fs::path root = fs::temp_directory_path(ec) / "webolution-savedgame-test";
  fs::remove_all(root, ec);
  fs::create_directories(root, ec);
  testStore(root);
  fs::remove_all(root, ec);

#ifdef WEBOLUTION_SOURCE_DIR
  const auto installedAfter = fs::is_regular_file(installedSave) ? sha256Hex(readFile(installedSave)) : std::string();
  check(installedBefore == installedAfter, "the installed SavedGame.dat is unchanged");
#endif
  std::cout << "SavedGame: " << checks << " checks, " << failures << " failures\n";
  return failures == 0 ? 0 : 1;
}
