#include "save/SoundSettings.h"

#include <array>

namespace webo::save {

std::filesystem::path soundSettingsPathUnder(const std::filesystem::path& localAppData) {
  return cloneSavePathUnder(localAppData).parent_path() / "SoundSettings.dat";
}

// Clone sidecar: v1 stores volume percentages. v2 also stores unlock-all
// and v3 includes window scale. Absent fields take their defaults.
std::expected<SoundSettings, std::string> SoundSettings::load(SaveSink& file) {
  auto read = file.read();
  if (!read) return std::unexpected(read.error());
  if (!*read) return SoundSettings{};
  const auto& bytes = **read;
  const bool version1 = bytes.size() == 3 && bytes[0] == std::byte{1};
  const bool version2 = bytes.size() == 4 && bytes[0] == std::byte{2};
  const bool version3 = bytes.size() == 5 && bytes[0] == std::byte{3};
  if ((!version1 && !version2 && !version3) ||
      bytes[1] > std::byte{100} || bytes[2] > std::byte{100} ||
      (!version1 && bytes[3] > std::byte{1}) || (version3 && bytes[4] > std::byte{3}))
    return std::unexpected("invalid or unsupported SoundSettings.dat");
  return SoundSettings{std::to_integer<int>(bytes[1]), std::to_integer<int>(bytes[2]),
                       !version1 && bytes[3] == std::byte{1},
                       version3 ? std::to_integer<int>(bytes[4]) : 0};
}

std::expected<void, std::string> SoundSettings::save(SaveSink& file) const {
  if (music < 0 || music > 100 || sfx < 0 || sfx > 100)
    return std::unexpected("volume outside 0..100");
  if (windowScale < 0 || windowScale > 3)
    return std::unexpected("window scale outside 0..3");
  const std::array bytes{std::byte{3}, static_cast<std::byte>(music), static_cast<std::byte>(sfx),
                         static_cast<std::byte>(unlockAllLevels), static_cast<std::byte>(windowScale)};
  return file.write(bytes);
}

}  // namespace webo::save
