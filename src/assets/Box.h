// Reader for the LEMBOX archive format used by Lemmings Revolution.
//
// Layout, all little-endian (see PLAN.md 1.2):
//
//     char[6] "LEMBOX"
//     u32     count
//     u32     nameBlockBytes
//     u8[nameBlockBytes]   count x (u32 len, char[len])   -- no NUL terminators
//     u32 count ; u32[count] offsets   -- absolute, from the start of the file
//     u32 count ; u32[count] sizes
//     payload
//
// Nothing inside a .box is compressed; individual *entries* may be (texture
// entries are LRZ). Decoding those is a separate layer.

#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace webo {

// Archive names are stored uppercase with backslash separators. Lookups accept
// any case and either separator; this is what they are normalised to.
std::string normaliseAssetName(std::string_view name);

class Box {
public:
  struct Entry {
    std::string name;      // as stored, e.g. "TEXTURES\\SHEETS\\~ADDITIVE0.BMP"
    std::uint32_t offset;  // absolute file offset of the payload
    std::uint32_t size;
  };

  struct Validation {
    bool ok = false;
    std::size_t gaps = 0;           // entries not contiguous with the previous
    std::uint64_t payloadEnd = 0;   // end of the last entry
    std::uint64_t fileSize = 0;
    std::vector<std::string> problems;
  };

  Box() = default;
  Box(Box&&) noexcept = default;
  Box& operator=(Box&&) noexcept = default;
  Box(const Box&) = delete;
  Box& operator=(const Box&) = delete;

  static std::expected<Box, std::string> open(const std::filesystem::path& path);

  const std::filesystem::path& path() const { return path_; }
  std::span<const Entry> entries() const { return entries_; }
  std::size_t count() const { return entries_.size(); }

  // Null when absent. Accepts any case and either path separator.
  const Entry* find(std::string_view name) const;
  bool contains(std::string_view name) const { return find(name) != nullptr; }

  std::expected<std::vector<std::byte>, std::string> read(const Entry& entry) const;
  std::expected<std::vector<std::byte>, std::string> read(std::string_view name) const;

  // Confirms the directory covers the payload exactly: entries ascending, no
  // overlap, nothing past EOF. A correct parse of a healthy archive reports
  // ok with zero gaps and payloadEnd == fileSize.
  Validation validate() const;

private:
  std::filesystem::path path_;
  mutable std::ifstream stream_;
  std::vector<Entry> entries_;
  std::unordered_map<std::string, std::size_t> index_;  // normalised name -> entry
  std::uint64_t dataStart_ = 0;
  std::uint64_t fileSize_ = 0;
};

// The game ships a base archive plus a patch archive that overrides entries of
// the same name and adds a few new ones. Reads resolve against the patch first.
class AssetLibrary {
public:
  static std::expected<AssetLibrary, std::string> open(
      const std::filesystem::path& base,
      const std::filesystem::path& patch = {});

  std::expected<std::vector<std::byte>, std::string> read(std::string_view name) const;
  bool contains(std::string_view name) const;

  // Every name visible through the overlay, sorted, each appearing once.
  std::vector<std::string> names() const;

  const Box& base() const { return base_; }
  const Box* patch() const { return hasPatch_ ? &patch_ : nullptr; }

  // Number of patch entries that shadow a base entry.
  std::size_t overrideCount() const;

private:
  Box base_;
  Box patch_;
  bool hasPatch_ = false;
};

}  // namespace webo
