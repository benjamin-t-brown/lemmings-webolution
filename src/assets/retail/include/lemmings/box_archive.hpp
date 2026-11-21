#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace lemmings {

struct BoxArchiveEntry {
    std::string name;
    std::uint32_t offset = 0;
    std::uint32_t size = 0;
};

// Logical index recovered from the LEMBOX loader at 0x004805B8 and its
// helpers. The original's allocation layout is not modeled.
struct BoxArchiveIndex {
    std::filesystem::path path;
    std::vector<BoxArchiveEntry> entries;

    std::vector<std::uint8_t> read_entry(std::size_t index) const;
    std::optional<std::size_t> find_entry(std::string name) const;
};

BoxArchiveIndex read_box_archive_index(const std::filesystem::path& path);

struct BoxArchiveLocation {
    std::size_t archive;
    std::size_t entry;
};

// The original searches registered archives in order. At startup it registers
// PATCH.BOX before LEMMINGS.BOX, so the patch wins when names overlap.
std::optional<BoxArchiveLocation> find_box_entry(
    const std::vector<BoxArchiveIndex>& archives, std::string name);

} // namespace lemmings
