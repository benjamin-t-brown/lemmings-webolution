#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace webo {

// Extract a single, exact legacy filename from a RAR 2.0 archive or SFX.
// Solid predecessors are CRC-checked without retaining their output.
std::expected<std::vector<std::byte>, std::string> extractRar20(
    std::span<const std::byte> archive, std::string_view entry);

using Rar20CrcObserver = void (*)(std::string_view entry, std::uint32_t crc32);

// Write the selected member to an explicit path and return its verified CRC32.
// The optional observer reports CRCs for each discarded solid predecessor.
std::expected<std::uint32_t, std::string> extractRar20File(
    const std::filesystem::path& archive, std::string_view entry,
    const std::filesystem::path& output, Rar20CrcObserver onPredecessor = nullptr);

}  // namespace webo

// Emscripten FS entry point, callable through ccall before main.
extern "C" int webolution_extract_rar20(const char* archive, const char* entry, const char* output);
