#include "Box.h"

#include <algorithm>
#include <cstring>
#include <format>
#include <set>

namespace webo {
namespace {

constexpr char kMagic[6] = {'L', 'E', 'M', 'B', 'O', 'X'};
constexpr std::uint32_t kMaxEntries = 1u << 22;     // sanity bound
constexpr std::uint32_t kMaxNameBlock = 1u << 26;

std::uint32_t readU32(const std::byte* p) {
  std::uint32_t v;
  std::memcpy(&v, p, 4);
  return v;  // the format is little-endian; so is every target we build for
}

}  // namespace

std::string normaliseAssetName(std::string_view name) {
  std::string out;
  out.reserve(name.size());
  for (char c : name) {
    if (c == '/')
      c = '\\';
    else if (c >= 'a' && c <= 'z')
      c = static_cast<char>(c - 'a' + 'A');
    out.push_back(c);
  }
  return out;
}

std::expected<Box, std::string> Box::open(const std::filesystem::path& path) {
  std::error_code ec;
  const auto fileSize = std::filesystem::file_size(path, ec);
  if (ec)
    return std::unexpected(std::format("{}: {}", path.string(), ec.message()));

  std::ifstream in(path, std::ios::binary);
  if (!in)
    return std::unexpected(std::format("{}: cannot open", path.string()));

  std::byte header[14];
  if (!in.read(reinterpret_cast<char*>(header), sizeof header))
    return std::unexpected(std::format("{}: truncated header", path.string()));
  if (std::memcmp(header, kMagic, sizeof kMagic) != 0)
    return std::unexpected(std::format("{}: not a LEMBOX archive", path.string()));

  const std::uint32_t count = readU32(header + 6);
  const std::uint32_t nameBlock = readU32(header + 10);
  if (count == 0 || count > kMaxEntries)
    return std::unexpected(std::format("{}: implausible entry count {}", path.string(), count));
  if (nameBlock > kMaxNameBlock)
    return std::unexpected(std::format("{}: implausible name block {}", path.string(), nameBlock));

  std::vector<std::byte> names(nameBlock);
  if (!in.read(reinterpret_cast<char*>(names.data()), static_cast<std::streamsize>(nameBlock)))
    return std::unexpected(std::format("{}: truncated name block", path.string()));

  Box box;
  box.entries_.reserve(count);
  std::size_t o = 0;
  for (std::uint32_t i = 0; i < count; ++i) {
    if (o + 4 > names.size())
      return std::unexpected(std::format("{}: name block overran at entry {}", path.string(), i));
    const std::uint32_t len = readU32(names.data() + o);
    o += 4;
    if (o + len > names.size())
      return std::unexpected(std::format("{}: name {} overruns block", path.string(), i));
    Entry e;
    e.name.assign(reinterpret_cast<const char*>(names.data() + o), len);
    e.offset = 0;
    e.size = 0;
    box.entries_.push_back(std::move(e));
    o += len;
  }

  // Two parallel tables, each preceded by its own copy of the count.
  const auto readTable = [&](const char* what,
                             std::vector<std::uint32_t>& out) -> std::expected<void, std::string> {
    std::byte c[4];
    if (!in.read(reinterpret_cast<char*>(c), 4))
      return std::unexpected(std::format("{}: truncated {} count", path.string(), what));
    const std::uint32_t n = readU32(c);
    if (n != count)
      return std::unexpected(
          std::format("{}: {} count {} != entry count {}", path.string(), what, n, count));
    out.resize(n);
    if (!in.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(n) * 4))
      return std::unexpected(std::format("{}: truncated {} table", path.string(), what));
    return {};
  };

  std::vector<std::uint32_t> offsets, sizes;
  if (auto r = readTable("offset", offsets); !r)
    return std::unexpected(r.error());
  if (auto r = readTable("size", sizes); !r)
    return std::unexpected(r.error());

  for (std::uint32_t i = 0; i < count; ++i) {
    box.entries_[i].offset = offsets[i];
    box.entries_[i].size = sizes[i];
  }

  box.dataStart_ = static_cast<std::uint64_t>(in.tellg());
  box.fileSize_ = fileSize;
  box.path_ = path;

  box.index_.reserve(count * 2);
  for (std::size_t i = 0; i < box.entries_.size(); ++i)
    box.index_.emplace(normaliseAssetName(box.entries_[i].name), i);

  in.close();
  box.stream_.open(path, std::ios::binary);
  if (!box.stream_)
    return std::unexpected(std::format("{}: cannot reopen for reading", path.string()));

  return box;
}

const Box::Entry* Box::find(std::string_view name) const {
  const auto it = index_.find(normaliseAssetName(name));
  return it == index_.end() ? nullptr : &entries_[it->second];
}

std::expected<std::vector<std::byte>, std::string> Box::read(const Entry& entry) const {
  if (static_cast<std::uint64_t>(entry.offset) + entry.size > fileSize_)
    return std::unexpected(std::format("{}: runs past end of file", entry.name));

  std::vector<std::byte> out(entry.size);
  if (entry.size == 0)
    return out;

  stream_.clear();
  stream_.seekg(static_cast<std::streamoff>(entry.offset));
  if (!stream_.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(entry.size)))
    return std::unexpected(std::format("{}: short read", entry.name));
  return out;
}

std::expected<std::vector<std::byte>, std::string> Box::read(std::string_view name) const {
  if (const Entry* e = find(name))
    return read(*e);
  return std::unexpected(std::format("{}: not in {}", name, path_.filename().string()));
}

Box::Validation Box::validate() const {
  Validation v;
  v.fileSize = fileSize_;

  std::uint64_t prevEnd = dataStart_;
  for (std::size_t i = 0; i < entries_.size(); ++i) {
    const Entry& e = entries_[i];
    const std::uint64_t end = static_cast<std::uint64_t>(e.offset) + e.size;
    if (e.offset < prevEnd) {
      v.problems.push_back(
          std::format("entry {} ({}) overlaps: offset {} < {}", i, e.name, e.offset, prevEnd));
      break;
    }
    if (e.offset != prevEnd)
      ++v.gaps;
    if (end > fileSize_) {
      v.problems.push_back(std::format("entry {} ({}) runs past EOF: {} > {}", i, e.name, end,
                                       fileSize_));
      break;
    }
    prevEnd = end;
  }
  v.payloadEnd = prevEnd;
  v.ok = v.problems.empty();
  return v;
}

// -- AssetLibrary ----------------------------------------------------------

std::expected<AssetLibrary, std::string> AssetLibrary::open(const std::filesystem::path& base,
                                                            const std::filesystem::path& patch) {
  AssetLibrary lib;
  auto b = Box::open(base);
  if (!b)
    return std::unexpected(b.error());
  lib.base_ = std::move(*b);

  if (!patch.empty() && std::filesystem::exists(patch)) {
    auto p = Box::open(patch);
    if (!p)
      return std::unexpected(p.error());
    lib.patch_ = std::move(*p);
    lib.hasPatch_ = true;
  }
  return lib;
}

std::expected<std::vector<std::byte>, std::string> AssetLibrary::read(
    std::string_view name) const {
  if (hasPatch_)
    if (const Box::Entry* e = patch_.find(name))
      return patch_.read(*e);
  return base_.read(name);
}

bool AssetLibrary::contains(std::string_view name) const {
  return (hasPatch_ && patch_.contains(name)) || base_.contains(name);
}

std::vector<std::string> AssetLibrary::names() const {
  std::set<std::string> all;
  for (const auto& e : base_.entries())
    all.insert(e.name);
  if (hasPatch_)
    for (const auto& e : patch_.entries())
      all.insert(e.name);
  return {all.begin(), all.end()};
}

std::size_t AssetLibrary::overrideCount() const {
  if (!hasPatch_)
    return 0;
  std::size_t n = 0;
  for (const auto& e : patch_.entries())
    if (base_.contains(e.name))
      ++n;
  return n;
}

}  // namespace webo
