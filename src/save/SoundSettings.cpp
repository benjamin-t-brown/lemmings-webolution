#include "save/SoundSettings.h"

#include <array>

namespace webo::save {

std::filesystem::path soundSettingsPathUnder(const std::filesystem::path& localAppData) {
  return cloneSavePathUnder(localAppData).parent_path() / "SoundSettings.dat";
}

// Provisional sidecar format. Version 2 appends the unlock-all boolean to the
// music and effects percentages. A version 1 file loads with the option off.
std::expected<SoundSettings, std::string> SoundSettings::load(SaveSink& file) {
  auto read = file.read();
  if (!read) return std::unexpected(read.error());
  if (!*read) return SoundSettings{};
  const auto& bytes = **read;
  const bool version1 = bytes.size() == 3 && bytes[0] == std::byte{1};
  const bool version2 = bytes.size() == 4 && bytes[0] == std::byte{2} && bytes[3] <= std::byte{1};
  if ((!version1 && !version2) || bytes[1] > std::byte{100} || bytes[2] > std::byte{100})
    return std::unexpected("invalid or unsupported SoundSettings.dat");
  return SoundSettings{std::to_integer<int>(bytes[1]), std::to_integer<int>(bytes[2]),
                       version2 && bytes[3] == std::byte{1}};
}

std::expected<void, std::string> SoundSettings::save(SaveSink& file) const {
  if (music < 0 || music > 100 || sfx < 0 || sfx > 100)
    return std::unexpected("volume outside 0..100");
  const std::array bytes{std::byte{2}, static_cast<std::byte>(music), static_cast<std::byte>(sfx),
                         static_cast<std::byte>(unlockAllLevels)};
  return file.write(bytes);
}

}  // namespace webo::save
