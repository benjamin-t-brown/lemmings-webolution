#pragma once

#include <expected>
#include <filesystem>
#include <string>

namespace webo {

class AssetLibrary;

// Report unfamiliar SHA-256s for archives that have passed structural validation.
std::expected<void, std::string> checkGameBoxHashes(const AssetLibrary& library);

// Copy and validate the game's two archives, then publish the cache directory
// in a single rename. The destination's parent is the clone's per-user folder.
std::expected<void, std::string> installDiscGameData(
    const std::filesystem::path& image, const std::filesystem::path& destination);

}  // namespace webo
