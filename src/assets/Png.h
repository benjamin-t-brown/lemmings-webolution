// Minimal PNG writer, for looking at decoded assets.
//
// Deliberately dependency-free: it emits a valid zlib stream made of stored
// (uncompressed) deflate blocks, so there is no compressor to get wrong and no
// zlib to link. Output is therefore roughly the size of the raw pixels, which
// is fine for inspecting assets and is not used by the game itself.

#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace webo {

struct Image;

// Writes RGBA, or RGB when every pixel is opaque (which is most of them).
std::expected<void, std::string> writePng(const std::filesystem::path& path, const Image& image);

// The encoded bytes, if you would rather not go through a file.
std::vector<std::uint8_t> encodePng(const Image& image);

}  // namespace webo
