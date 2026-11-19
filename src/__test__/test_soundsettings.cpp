#include "save/SoundSettings.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <iostream>

int main(int argc, char** argv) {
  using namespace webo::save;
  if (argc != 2) return 1;
  const std::filesystem::path root(argv[1]); // private CTest work directory
  FileSaveSink progress(cloneSavePathUnder(root));
  const auto settingsPath = soundSettingsPathUnder(root);
  if (settingsPath.parent_path() != progress.target().parent_path() ||
      settingsPath == progress.target()) return 1;

  // Include nonzero unknown fields and unmapped records. An options edit must
  // leave the entire progress image alone.
  auto original = SavedGame{}.serialize();
  for (std::size_t i = 0; i < original.size(); ++i) original[i] = static_cast<std::byte>(i % 251);
  if (!progress.write(original)) return 1;
  const SoundSettings legacy{0, 63};
  const SoundSettings chosen{0, 63, true};
  {
    FileSaveSink first(settingsPath);
    const std::array version1{std::byte{1}, std::byte{0}, std::byte{63}};
    if (!first.write(version1) || SoundSettings::load(first) != legacy) return 1;
    if (!chosen.save(first)) return 1;
  }
  FileSaveSink reopened(settingsPath);
  if (SoundSettings::load(reopened) != chosen) return 1;
  if (SoundSettings{101, 0}.save(reopened) || SoundSettings::load(reopened) != chosen) return 1;
  if (!legacy.save(reopened) || SoundSettings::load(reopened) != legacy) return 1;
  auto after = progress.read();
  if (!after || !*after || !std::ranges::equal(**after, original)) return 1;
  const std::array malformed{std::byte{1}, std::byte{0}, std::byte{101}};
  if (!reopened.write(malformed) || SoundSettings::load(reopened)) return 1;
  const std::array invalidToggle{std::byte{2}, std::byte{0}, std::byte{63}, std::byte{2}};
  if (!reopened.write(invalidToggle) || SoundSettings::load(reopened)) return 1;
  std::cout << "Loaded version 1 and version 2 settings, retained all " << original.size()
            << " progress bytes, rejected invalid settings.\n";
}
