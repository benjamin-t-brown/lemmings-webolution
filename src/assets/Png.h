// PNG writer for asset inspection. Stored deflate blocks require no compressor or zlib dependency.

#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace webo {

struct Image;

// Write RGB for fully opaque pixels and RGBA otherwise.
std::expected<void, std::string> writePng(const std::filesystem::path& path, const Image& image);

// Return the encoded PNG bytes.
std::vector<std::uint8_t> encodePng(const Image& image);

}  // namespace webo
