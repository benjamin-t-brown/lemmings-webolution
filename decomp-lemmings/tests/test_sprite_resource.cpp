#include "lemmings/sprite_resource.hpp"

#include <bit>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {
using Bytes = std::vector<std::uint8_t>;
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void number(Bytes& bytes, std::uint64_t value, unsigned width = 4) {
    for (unsigned i = 0; i < width; ++i) bytes.push_back(static_cast<std::uint8_t>(value >> (8 * i)));
}
void text(Bytes& bytes, std::string_view value) {
    number(bytes, value.size());
    bytes.insert(bytes.end(), value.begin(), value.end());
}
void rejects(const Bytes& bytes) {
    try { (void)lemmings::parse_sprite_resource(bytes); }
    catch (const std::invalid_argument&) { return; }
    throw std::runtime_error("accepted malformed sprite");
}
Bytes populated() {
    Bytes bytes;
    number(bytes, 2); // clips
    number(bytes, 2); // first clip's frames
    number(bytes, 1); // first frame's pieces
    for (unsigned i = 0; i < 4; ++i) number(bytes, 0x80000000u + i);
    for (unsigned i = 0; i < 4; ++i) number(bytes, 0x7ff0000000000001ULL + i, 8);
    for (unsigned i = 0; i < 4; ++i) number(bytes, 0xffffffffu - i);
    text(bytes, "texture\\sheet.bmp");
    number(bytes, 0); // empty second frame
    text(bytes, std::string_view("walk\0ignored", 12));
    number(bytes, std::bit_cast<std::uint64_t>(10.0), 8);
    number(bytes, 0); // empty second clip
    text(bytes, "empty");
    number(bytes, 0x8000000000000000ULL, 8); // negative zero
    number(bytes, 2); // triple arrays
    number(bytes, 2);
    number(bytes, 0);
    for (unsigned i = 0; i < 48; ++i) number(bytes, 0x3ff0000000000000ULL + i, 8);
    return bytes;
}
}

int main() {
    try {
        const auto bytes = populated();
        const auto sprite = lemmings::parse_sprite_resource(bytes);
        check(sprite.clips.size() == 2 && sprite.clips[0].frames.size() == 2, "clip/frame structure");
        const auto& piece = sprite.clips[0].frames[0].pieces.at(0);
        check(piece.values_00[3] == 0x80000003 && piece.values_30[3] == 0xfffffffc, "integer fields");
        check(piece.double_bits_10[3] == 0x7ff0000000000004ULL, "serialized NaN payloads");
        check(piece.texture_name == "texture\\sheet.bmp", "texture name");
        check(sprite.clips[0].name == "walk" && sprite.clips[1].frames.empty(), "C name and empty clip");
        check(sprite.triple_arrays.size() == 2 && sprite.triple_arrays[0].triples.empty() &&
              sprite.triple_arrays[1].triples[1][2] == 0x3ff0000000000005ULL, "triple tail");
        const auto clips = lemmings::sprite_animation_clips(sprite);
        check(clips[0].frame_count == 2 && clips[0].speed == 10.0 && clips[0].name == "walk", "playback metadata");
        check(std::bit_cast<std::uint64_t>(clips[1].speed) == 0x8000000000000000ULL, "negative zero metadata");
        for (std::size_t size = 0; size < bytes.size(); ++size)
            rejects(Bytes(bytes.begin(), bytes.begin() + size));
        auto extra = bytes; extra.push_back(0); rejects(extra);
        auto bad = bytes; for (unsigned i = 0; i < 4; ++i) bad[i] = 0xff; rejects(bad);
        bad = bytes; for (unsigned i = 4; i < 8; ++i) bad[i] = 0xff; rejects(bad);
        bad = bytes; for (unsigned i = 8; i < 12; ++i) bad[i] = 0xff; rejects(bad);
        check(lemmings::parse_sprite_resource(Bytes(8, 0)).clips.empty(), "empty root");
        Bytes palettes;
        number(palettes, 0); number(palettes, 2); number(palettes, 1);
        for (unsigned i = 0; i < 256 * 3; ++i) number(palettes, i, 8);
        number(palettes, 0xffffffff);
        const auto arrays = lemmings::parse_sprite_resource(palettes).triple_arrays;
        check(arrays[0].triples.size() == 256 && arrays[0].triples[255][2] == 767 &&
              arrays[1].type == 0xffffffff && arrays[1].triples.empty(), "triple type dispatch");
        // Name exactly at capacity consumes all 128 bytes and stores 127.
        Bytes boundary;
        number(boundary, 1); number(boundary, 0); text(boundary, std::string(128, 'x'));
        number(boundary, 0, 8); number(boundary, 0);
        check(lemmings::parse_sprite_resource(boundary).clips[0].name.size() == 127, "clip name capacity");
        // Oversized declared name leaves payload unread, matching 0x47D940.
        boundary.clear(); number(boundary, 1); number(boundary, 0); number(boundary, 129);
        number(boundary, 0x123456789abcdef0ULL, 8); number(boundary, 0);
        const auto oversized = lemmings::parse_sprite_resource(boundary);
        check(oversized.clips[0].name.empty() && oversized.clips[0].speed_bits_10 == 0x123456789abcdef0ULL,
              "oversized name must leave payload unread");
        std::cout << "sprite resource structure, fields, metadata, bounded strings and truncation passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
