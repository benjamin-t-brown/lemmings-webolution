#include "assets/DiscGameData.h"

#include "assets/Box.h"
#include "assets/Rar20.h"

#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <format>
#include <fstream>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace webo {
namespace {

namespace fs = std::filesystem;

constexpr std::size_t kBlockSize = 2048;
constexpr std::size_t kReadBlocks = 128;
constexpr std::uint32_t kFirstDescriptor = 16;

struct DiscError { std::string message; };

[[noreturn]] void fail(std::string message) { throw DiscError{std::move(message)}; }

struct SectorLayout {
  std::size_t stride, userOffset;
};

using Bytes = std::span<const std::uint8_t>;

bool descriptor(Bytes bytes) {
  return std::string_view(reinterpret_cast<const char*>(bytes.data() + 1), 5) == "CD001" &&
         bytes[6] == 1;
}

// ISO 9660 records store these integers in both byte orders.
std::uint32_t bothEndian(Bytes bytes, std::size_t offset, std::size_t width) {
  std::uint32_t little = 0, big = 0;
  for (std::size_t i = 0; i < width; ++i) {
    little = (little << 8) | bytes[offset + width - 1 - i];
    big = (big << 8) | bytes[offset + width + i];
  }
  if (little != big) fail("Inconsistent ISO 9660 numeric fields.");
  return little;
}

struct Extent {
  std::uint64_t lba;
  std::uint32_t size;
  bool directory;
  bool unsupported;
};

struct DirectoryRecord {
  Extent extent;
  Bytes identifier;
};

DirectoryRecord readRecord(Bytes bytes) {
  if (bytes.size() < 34 || bytes[32] == 0 || 33u + bytes[32] > bytes.size())
    fail("Malformed ISO 9660 directory record.");
  const Extent extent{std::uint64_t(bothEndian(bytes, 2, 4)) + bytes[1],
                      bothEndian(bytes, 10, 4), bool(bytes[25] & 2),
                      bool((bytes[25] & 0x80) || bytes[26] || bytes[27])};
  return {extent, bytes.subspan(33, bytes[32])};
}

// The names we look for are ASCII. Other Joliet characters cannot match them.
std::string discName(Bytes bytes, bool joliet) {
  if (joliet && bytes.size() % 2) fail("Malformed Joliet filename.");
  std::string name;
  for (std::size_t i = 0; i < bytes.size(); ++i) {
    if (joliet && bytes[i++] != 0) return {};
    const auto c = static_cast<char>(bytes[i]);
    name += (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
  }
  if (const auto version = name.rfind(';'); version != std::string::npos &&
      version + 1 < name.size() &&
      name.find_first_not_of("0123456789", version + 1) == std::string::npos)
    name.resize(version);
  return name;
}

struct GameFiles {
  std::optional<Extent> base, rootPatch, patchInstaller;
};

class Iso9660 {
 public:
  explicit Iso9660(const fs::path& path) : input_(path, std::ios::binary | std::ios::ate) {
    const auto length = input_.tellg();
    if (length < 0) fail("Could not open the CD image.");
    size_ = static_cast<std::uint64_t>(length);
    for (const auto layout : {SectorLayout{2048, 0}, SectorLayout{2352, 16}, SectorLayout{2352, 24}}) {
      const auto offset = kFirstDescriptor * layout.stride + layout.userOffset;
      std::array<std::uint8_t, 7> header{};
      if (offset + header.size() <= size_ && readAt(offset, header) && descriptor(header)) {
        layout_ = layout;
        return;
      }
    }
    fail("Missing ISO 9660 volume descriptor. The CD image may be unsupported or truncated.");
  }

  GameFiles findGameFiles() {
    GameFiles files;
    for (bool joliet : {false, true}) {
      const auto& root = roots_[joliet];
      if (!root) continue;
      std::optional<Extent> installer;
      for (const auto& [extent, name] : entries(*root, joliet)) {
        if (extent.directory) {
          if (installer) continue;
          for (const auto& [child, childName] : entries(extent, joliet)) {
            if (!child.directory && childName == "LEMPATCH.EXE") {
              installer = child;
              break;
            }
          }
        } else if (name == "LEMMINGS.BOX") {
          files.base = extent;
        } else if (name == "PATCH.BOX") {
          files.rootPatch = extent;
        }
      }
      if (installer) files.patchInstaller = installer;
    }
    return files;
  }

  // Report whether a recognized descriptor declares more blocks than the file contains.
  bool readVolume() {
    bool shortVolume = false;
    for (std::uint32_t lba = kFirstDescriptor; lba < kFirstDescriptor + 64; ++lba) {
      const auto bytes = sector(lba);
      if (!descriptor(bytes)) fail("Invalid ISO 9660 volume descriptor.");
      if (bytes[0] == 255) {
        if (!roots_[0]) fail("Missing primary ISO 9660 volume descriptor.");
        return shortVolume;
      }
      const auto escape = std::string_view(reinterpret_cast<const char*>(bytes.data() + 88), 3);
      const bool joliet = bytes[0] == 2 && (escape == "%/@" || escape == "%/C" || escape == "%/E");
      if (bytes[0] != 1 && !joliet) continue;
      if (bothEndian(bytes, 128, 2) != kBlockSize)
        fail("Unsupported ISO 9660 logical block size.");
      const auto volumeBlocks = bothEndian(bytes, 80, 4);
      if (volumeBlocks <= lba) fail("Invalid ISO 9660 volume space size.");
      if (volumeBlocks > size_ / layout_.stride) shortVolume = true;
      roots_[joliet] = readRecord(Bytes(bytes).subspan(156, bytes[156])).extent;
    }
    fail("Missing ISO 9660 volume descriptor terminator.");
  }

  void copyFile(const Extent& extent, const fs::path& path, std::uint32_t limit = UINT32_MAX) {
    if (extent.unsupported) fail("Unsupported multi-extent or interleaved ISO file.");
    if (extent.size > limit) fail("LEMPATCH.EXE exceeds the RAR reader's input limit.");
    const auto totalBlocks = (std::uint64_t(extent.size) + kBlockSize - 1) / kBlockSize;
    if (extent.lba + totalBlocks > size_ / layout_.stride) fail("Truncated CD image file extent.");
    std::ofstream out(path, std::ios::binary);
    if (!out) fail("Could not create " + path.string());
    std::vector<std::uint8_t> buffer(kReadBlocks * layout_.stride);
    auto remaining = std::size_t(extent.size);
    for (std::uint64_t first = 0; first < totalBlocks;) {
      const auto count = std::min<std::uint64_t>(kReadBlocks, totalBlocks - first);
      if (!readAt((extent.lba + first) * layout_.stride,
                  std::span(buffer).first(count * layout_.stride)))
        fail("Could not read the requested sectors from the CD image.");
      if (layout_.stride == kBlockSize) {
        const auto length = std::min<std::size_t>(remaining, count * kBlockSize);
        out.write(reinterpret_cast<const char*>(buffer.data()), length);
        remaining -= length;
      } else {
        for (std::size_t i = 0; i < count; ++i) {
          const auto length = std::min(remaining, kBlockSize);
          out.write(reinterpret_cast<const char*>(buffer.data() + i * layout_.stride + layout_.userOffset),
                    length);
          remaining -= length;
        }
      }
      if (!out) fail("Could not write " + path.string());
      first += count;
    }
    out.close();
    if (!out) fail("Could not finish writing " + path.string());
  }

 private:
  bool readAt(std::uint64_t offset, std::span<std::uint8_t> bytes) {
    input_.clear();
    input_.seekg(static_cast<std::streamoff>(offset));
    return bool(input_.read(reinterpret_cast<char*>(bytes.data()), bytes.size()));
  }

  std::array<std::uint8_t, kBlockSize> sector(std::uint64_t lba) {
    std::array<std::uint8_t, kBlockSize> bytes{};
    if (lba >= size_ / layout_.stride ||
        !readAt(lba * layout_.stride + layout_.userOffset, bytes))
      fail("Truncated CD image sector.");
    return bytes;
  }

  std::vector<std::pair<Extent, std::string>> entries(const Extent& dir, bool joliet) {
    if (!dir.directory || dir.unsupported || dir.size > 16 * 1024 * 1024)
      fail("Unsupported ISO directory extent.");
    std::vector<std::pair<Extent, std::string>> result;
    for (std::uint64_t pos = 0; pos < dir.size; pos += kBlockSize) {
      const auto bytes = sector(dir.lba + pos / kBlockSize);
      const auto end = std::min<std::uint64_t>(kBlockSize, dir.size - pos);
      for (std::size_t offset = 0; offset < end && bytes[offset] != 0;) {
        const auto length = bytes[offset];
        if (offset + length > end) fail("ISO 9660 directory record crosses a sector boundary.");
        const auto [extent, name] = readRecord(Bytes(bytes).subspan(offset, length));
        if (name.size() != 1 || name[0] > 1)
          result.emplace_back(extent, discName(name, joliet));
        offset += length;
      }
    }
    return result;
  }

  std::ifstream input_;
  std::uint64_t size_ = 0;
  SectorLayout layout_{};
  std::array<std::optional<Extent>, 2> roots_{};
};

// Read whole blocks from disk and retain only the tail for SHA-256 padding.
class Sha256 {
 public:
  void block(Bytes bytes);

  std::string finish(std::span<std::uint8_t> tail, std::size_t length, std::uint64_t size) {
    tail[length++] = 0x80;
    const std::size_t padded = length <= 56 ? 64 : 128;
    std::fill(tail.begin() + length, tail.begin() + padded, 0);
    for (unsigned i = 0; i < 8; ++i)
      tail[padded - 1 - i] = static_cast<std::uint8_t>((size * 8) >> (i * 8));
    for (std::size_t offset = 0; offset < padded; offset += 64) block(tail.subspan(offset, 64));
    std::string digest;
    for (const auto value : state_) digest += std::format("{:08x}", value);
    return digest;
  }

 private:
  std::array<std::uint32_t, 8> state_{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                                     0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
};

void Sha256::block(Bytes bytes) {
  static constexpr std::uint32_t constants[] = {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
      0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
      0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
      0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
      0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
      0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
      0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
      0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
  std::uint32_t words[64];
  for (std::size_t i = 0; i < 16; ++i)
    words[i] = (std::uint32_t(bytes[i * 4]) << 24) | (std::uint32_t(bytes[i * 4 + 1]) << 16) |
               (std::uint32_t(bytes[i * 4 + 2]) << 8) | bytes[i * 4 + 3];
  for (std::size_t i = 16; i < 64; ++i) {
    const auto x = words[i - 15], y = words[i - 2];
    const auto small0 = std::rotr(x, 7) ^ std::rotr(x, 18) ^ (x >> 3);
    const auto small1 = std::rotr(y, 17) ^ std::rotr(y, 19) ^ (y >> 10);
    words[i] = words[i - 16] + small0 + words[i - 7] + small1;
  }
  auto [a, b, c, d, e, f, g, h] = state_;
  for (std::size_t i = 0; i < 64; ++i) {
    const auto big0 = std::rotr(a, 2) ^ std::rotr(a, 13) ^ std::rotr(a, 22);
    const auto big1 = std::rotr(e, 6) ^ std::rotr(e, 11) ^ std::rotr(e, 25);
    const auto choose = (e & f) ^ (~e & g);
    const auto majority = (a & b) ^ (a & c) ^ (b & c);
    const auto next = h + big1 + choose + constants[i] + words[i];
    h = g;
    g = f;
    f = e;
    e = d + next;
    d = c;
    c = b;
    b = a;
    a = next + big0 + majority;
  }
  const std::array result{a, b, c, d, e, f, g, h};
  for (std::size_t i = 0; i < state_.size(); ++i) state_[i] += result[i];
}

std::expected<std::string, std::string> hashFile(const fs::path& path) {
  constexpr std::size_t kChunk = 64 * 1024;
  std::array<std::uint8_t, kChunk + 64> bytes;
  std::ifstream input(path, std::ios::binary);
  Sha256 hash;
  std::uint64_t size = 0;
  while (input.read(reinterpret_cast<char*>(bytes.data()), kChunk) || input.eof()) {
    const auto count = static_cast<std::size_t>(input.gcount());
    size += count;
    const auto whole = count - count % 64;
    for (std::size_t i = 0; i < whole; i += 64) hash.block(Bytes(bytes).subspan(i, 64));
    if (input.bad()) break;
    if (input.eof()) return hash.finish(std::span(bytes).subspan(whole), count - whole, size);
  }
  return std::unexpected("Could not read game data for SHA-256: " + path.string());
}

#ifdef _WIN32
// Windows releases the exclusive file handle even if the process is killed.
// Only its holder may clear the fixed staging folder left by an interrupted import.
class DiscInstallLock {
 public:
  explicit DiscInstallLock(const fs::path& path) {
    handle_ = CreateFileW(path.c_str(), GENERIC_WRITE | DELETE, 0, nullptr, OPEN_ALWAYS,
                          FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
    if (handle_ != INVALID_HANDLE_VALUE) return;
    const auto error = GetLastError();
    if (error == ERROR_SHARING_VIOLATION || error == ERROR_DELETE_PENDING)
      fail("Another Lemmings Webolution instance is preparing game data. Retry when it finishes.");
    fail("Could not lock the per-user game data folder (Windows error " + std::to_string(error) + ").");
  }
  ~DiscInstallLock() { CloseHandle(handle_); }
  DiscInstallLock(const DiscInstallLock&) = delete;
  DiscInstallLock& operator=(const DiscInstallLock&) = delete;

 private:
  HANDLE handle_;
};
#endif

}  // namespace

std::expected<void, std::string> checkGameBoxHashes(const AssetLibrary& library) {
  constexpr std::string_view baseHash = "74b544beee769b0354e64b25eaec06fe725d50d2df11a94270600ae03a7e6a51";
  constexpr std::string_view patchHash11 = "a393a45e71d797fc5b646a9a235d1f64298129f378c6f4b79f47ec773d00de53";
  constexpr std::string_view patchHash10 = "fd6025ddc5520fbf5c36314f602b9321ca1f088f391800722f85e0dd6ddecf1e";
  const auto check = [](const Box& box, std::string_view known,
                        std::string_view alternate = {}) -> std::expected<void, std::string> {
    auto hash = hashFile(box.path());
    if (!hash) return std::unexpected(hash.error());
    if (*hash != known && *hash != alternate)
      std::fprintf(stderr, "Using %s with an unfamiliar SHA-256 (%s).\n",
                   box.path().filename().string().c_str(), hash->c_str());
    return {};
  };
  if (auto result = check(library.base(), baseHash); !result) return result;
  if (const auto* patch = library.patch()) return check(*patch, patchHash11, patchHash10);
  return {};
}

std::expected<void, std::string> installDiscGameData(const fs::path& image, const fs::path& destination) {
  try {
    Iso9660 iso(image);
    const bool shortVolume = iso.readVolume();
    const auto files = iso.findGameFiles();
    if (!files.base) fail("Missing LEMMINGS.BOX at the root of the CD image.");
    if (!files.patchInstaller && !files.rootPatch)
      fail("Missing LEMPATCH.EXE and root PATCH.BOX in the CD image.");

    fs::create_directories(destination.parent_path());
    struct Staging {
      fs::path path;
      ~Staging() {
        if (path.empty()) return;
        std::error_code ignored;
        fs::remove_all(path, ignored);
      }
    };
#ifdef _WIN32
    const DiscInstallLock lock(destination.parent_path() / ".disc-data.lock");
    const auto stage = destination.parent_path() / ".disc-data.tmp";
    const auto attributes = GetFileAttributesW(stage.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES) {
      if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
        fail("The temporary game data folder is redirected or blocked by a file.");
      fs::remove_all(stage);
    }
#else
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto stage = destination.parent_path() / (".disc-data-" + std::to_string(stamp) + ".tmp");
#endif
    if (!fs::create_directory(stage)) fail("Could not create temporary game data folder.");
    Staging staging{stage};
    const auto stagedBase = stage / "lemmings.box.tmp";
    const auto stagedPatch = stage / "patch.box.tmp";
    if (files.patchInstaller) {
      const auto sfx = stage / "disc-patch.tmp";
      iso.copyFile(*files.patchInstaller, sfx, 64 * 1024 * 1024);
      if (auto patch = extractRar20File(sfx, "patch.box", stagedPatch); !patch) fail(patch.error());
      fs::remove(sfx);
    } else {
      iso.copyFile(*files.rootPatch, stagedPatch);
    }
    iso.copyFile(*files.base, stagedBase);
    fs::rename(stagedBase, stage / "lemmings.box");
    fs::rename(stagedPatch, stage / "patch.box");
    {
      auto boxes = AssetLibrary::openRequired(stage / "lemmings.box", stage / "patch.box");
      if (!boxes) fail(boxes.error());
      if (auto hashes = checkGameBoxHashes(*boxes); !hashes) fail(hashes.error());
    }
    std::error_code error;
    fs::rename(stage, destination, error);
    if (error) {
      // Another process may have completed its own validation and publication.
      if (!AssetLibrary::openRequired(destination / "lemmings.box", destination / "patch.box"))
        fail("Could not publish game data to " + destination.string() + ": " + error.message());
    } else {
      staging.path.clear();
    }
    if (shortVolume)
      std::fputs("Warning: the CD image is shorter than the declared ISO 9660 volume. "
                 "Both game data files were imported.\n", stderr);
    return {};
  } catch (const DiscError& error) {
    return std::unexpected(error.message);
  } catch (const fs::filesystem_error& error) {
    return std::unexpected(error.what());
  }
}

}  // namespace webo
