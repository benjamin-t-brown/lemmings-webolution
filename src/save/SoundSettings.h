#pragma once

#include "save/SaveStore.h"

namespace webo::save {

// Clone settings beside SavedGame.dat. The original's volume doubles belong
// to its audio manager and have no traced disk fields.
struct SoundSettings {
  int music = 30;
  int sfx = 100;
  bool operator==(const SoundSettings&) const = default;

  static std::expected<SoundSettings, std::string> load(SaveSink& file);
  std::expected<void, std::string> save(SaveSink& file) const;
};

std::filesystem::path soundSettingsPathUnder(const std::filesystem::path& localAppData);

}  // namespace webo::save
