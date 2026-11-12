#include "lemmings/box_archive.hpp"

#include <array>
#include <fstream>
#include <stdexcept>
#include <utility>

namespace lemmings {
namespace {
void read_exact(std::istream& input, char* output, std::size_t count) {
    input.read(output, static_cast<std::streamsize>(count));
    if (input.gcount() != static_cast<std::streamsize>(count)) {
        throw std::runtime_error("truncated box archive");
    }
}

std::uint32_t read_u32(std::istream& input) {
    std::array<std::uint8_t, 4> bytes{};
    read_exact(input, reinterpret_cast<char*>(bytes.data()), bytes.size());
    return static_cast<std::uint32_t>(bytes[0]) |
           (static_cast<std::uint32_t>(bytes[1]) << 8) |
           (static_cast<std::uint32_t>(bytes[2]) << 16) |
           (static_cast<std::uint32_t>(bytes[3]) << 24);
}
}

BoxArchiveIndex read_box_archive_index(const std::filesystem::path& path) {
    const auto file_size = std::filesystem::file_size(path);
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("cannot open box archive");
    std::array<char, 6> magic{};
    read_exact(input, magic.data(), magic.size());
    if (magic != std::array<char, 6>{'L', 'E', 'M', 'B', 'O', 'X'}) {
        throw std::runtime_error("invalid LEMBOX signature");
    }
    const auto count = read_u32(input);
    const auto name_bytes = read_u32(input);
    if (count > 1'000'000 || static_cast<std::uint64_t>(name_bytes) + 14 > file_size) {
        throw std::runtime_error("invalid box index sizes");
    }
    BoxArchiveIndex index{path, {}};
    index.entries.reserve(count);
    std::uint64_t consumed_names = 0;
    for (std::uint32_t i = 0; i < count; ++i) {
        if (consumed_names + 4 > name_bytes) {
            throw std::runtime_error("truncated box name table");
        }
        const auto length = read_u32(input);
        consumed_names += 4;
        if (length > 254 || length > name_bytes - consumed_names) {
            throw std::runtime_error("invalid box entry name length");
        }
        std::string name(length, '\0');
        read_exact(input, name.data(), length);
        consumed_names += length;
        index.entries.push_back(BoxArchiveEntry{std::move(name), 0, 0});
    }
    if (consumed_names != name_bytes || read_u32(input) != count) {
        throw std::runtime_error("box offset table count mismatch");
    }
    for (auto& entry : index.entries) entry.offset = read_u32(input);
    if (read_u32(input) != count) {
        throw std::runtime_error("box size table count mismatch");
    }
    for (auto& entry : index.entries) entry.size = read_u32(input);
    const auto data_start = static_cast<std::uint64_t>(input.tellg());
    std::uint64_t previous_end = data_start;
    for (const auto& entry : index.entries) {
        if (entry.offset < previous_end ||
            static_cast<std::uint64_t>(entry.offset) + entry.size > file_size) {
            throw std::runtime_error("invalid box entry bounds");
        }
        previous_end = static_cast<std::uint64_t>(entry.offset) + entry.size;
    }
    return index;
}

std::vector<std::uint8_t> BoxArchiveIndex::read_entry(std::size_t entry_index) const {
    if (entry_index >= entries.size()) throw std::out_of_range("box entry index");
    const auto& entry = entries[entry_index];
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("cannot open box archive");
    input.seekg(entry.offset);
    std::vector<std::uint8_t> bytes(entry.size);
    read_exact(input, reinterpret_cast<char*>(bytes.data()), bytes.size());
    return bytes;
}

std::optional<std::size_t> BoxArchiveIndex::find_entry(std::string name) const {
    const auto colon = name.find(':');
    if (colon != std::string::npos) {
        if (colon + 2 > name.size()) return std::nullopt;
        name = name.substr(colon + 2);
    }
    for (char& c : name) {
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - ('a' - 'A'));
    }
    for (std::size_t i = 0; i < entries.size(); ++i) {
        if (entries[i].name == name) return i;
    }
    return std::nullopt;
}

std::optional<BoxArchiveLocation> find_box_entry(
    const std::vector<BoxArchiveIndex>& archives, std::string name) {
    for (std::size_t i = 0; i < archives.size(); ++i) {
        if (const auto entry = archives[i].find_entry(name)) {
            return BoxArchiveLocation{i, *entry};
        }
    }
    return std::nullopt;
}

} // namespace lemmings
