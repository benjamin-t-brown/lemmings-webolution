// Reads user-owned .SPR entries in place; emits counts and hashes, never assets.
#include "lemmings/box_archive.hpp"
#include "lemmings/sprite_resource.hpp"

#include <bit>
#include <cctype>
#include <iostream>
#include <stdexcept>

namespace {
struct Hash {
    std::uint64_t value = 14695981039346656037ULL;
    void integer(std::uint64_t word, unsigned bytes) {
        for (unsigned i = 0; i < bytes; ++i)
            value = (value ^ ((word >> (i * 8)) & 0xff)) * 1099511628211ULL;
    }
    void name(const std::string& text) {
        for (unsigned char byte : text) integer(byte, 1);
        integer(0xff, 1);
    }
};
std::uint64_t hash_name(const std::string& text) { Hash hash; hash.name(text); return hash.value; }
void probe(const lemmings::BoxArchiveIndex& archive, std::size_t index) {
    const auto bytes = archive.read_entry(index);
    const auto sprite = lemmings::parse_sprite_resource(bytes);
    const auto metadata = lemmings::sprite_animation_clips(sprite);
    std::cout << hash_name(archive.entries[index].name) << ' ' << bytes.size()
              << ' ' << sprite.clips.size();
    for (std::size_t i = 0; i < sprite.clips.size(); ++i) {
        const auto& clip = sprite.clips[i];
        if (metadata[i].frame_count != static_cast<std::int32_t>(clip.frames.size()) ||
            std::bit_cast<std::uint64_t>(metadata[i].speed) != clip.speed_bits_10 ||
            metadata[i].name != clip.name) throw std::runtime_error("animation metadata mismatch");
        std::cout << ' ' << clip.frames.size() << ' ' << hash_name(clip.name)
                  << ' ' << clip.speed_bits_10;
        for (const auto& frame : clip.frames) {
            Hash numeric, names;
            for (const auto& piece : frame.pieces) {
                for (auto value : piece.values_00) numeric.integer(value, 4);
                for (auto value : piece.double_bits_10) numeric.integer(value, 8);
                for (auto value : piece.values_30) numeric.integer(value, 4);
                names.name(piece.texture_name);
            }
            std::cout << ' ' << frame.pieces.size() << ' ' << numeric.value << ' ' << names.value;
        }
    }
    std::cout << ' ' << sprite.triple_arrays.size();
    for (const auto& array : sprite.triple_arrays) {
        Hash hash;
        for (const auto& triple : array.triples)
            for (auto value : triple) hash.integer(value, 8);
        std::cout << ' ' << array.type << ' ' << array.triples.size() << ' ' << hash.value;
    }
    std::cout << '\n';
}
}
int main(int argc, char** argv) {
    try {
        if (argc < 2) throw std::invalid_argument("usage: sprite_resource_probe archive.box [patch.box]");
        for (int i = 1; i < argc; ++i) {
            const auto archive = lemmings::read_box_archive_index(argv[i]);
            for (std::size_t j = 0; j < archive.entries.size(); ++j) {
                auto name = archive.entries[j].name;
                for (auto& letter : name) letter = static_cast<char>(std::tolower(static_cast<unsigned char>(letter)));
                if (name.ends_with(".spr")) probe(archive, j);
            }
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
