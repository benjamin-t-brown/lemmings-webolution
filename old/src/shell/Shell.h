// Phase 5 frontend: main menu, confirm, options, level map, results.
//
// Strings come from retail .LTX. Layout is close to the stock flow (five-item
// main menu, Ja/Nein confirm, five difficulty groups) without cloning the
// 3D gold-node map.

#pragma once

#include "Progress.h"
#include "assets/GameData.h"
#include "gfx/Font.h"
#include "gfx/Renderer.h"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace webo::audio {
class Mixer;
}

namespace webo::shell {

enum class Screen {
  Main,
  ConfirmNew,
  Options,
  Map,
  Results,
  Playing,  // shell yields to the play loop
};

// LTX indices measured from LANGUAGE\UK.LTX (same slots in other languages).
namespace Ltx {
constexpr std::size_t StartNew = 125;
constexpr std::size_t PlayCurrent = 126;
constexpr std::size_t Options = 3;
constexpr std::size_t LoadGame = 8;
constexpr std::size_t Quit = 43;
constexpr std::size_t Yes = 21;
constexpr std::size_t No = 22;
constexpr std::size_t Resume = 44;
constexpr std::size_t Continue = 47;
constexpr std::size_t TryAgain = 48;
constexpr std::size_t Failed = 49;
constexpr std::size_t Stormed = 50;
constexpr std::size_t TimeRemaining = 67;
constexpr std::size_t Out = 68;
constexpr std::size_t Saved = 69;
constexpr std::size_t LanguageSel = 120;
constexpr std::size_t Music = 11;
constexpr std::size_t Sfx = 12;
constexpr std::size_t SaveGame = 38;
constexpr std::size_t BackMain = 127;
constexpr std::size_t Paused = 84;
constexpr std::size_t SkillClimber = 74;  // … +7 = Digger
constexpr std::size_t DiffEasy = 32;
}  // namespace Ltx

struct PlayRequest {
  std::string levelId;
  int group = 0;
  int slot = 0;
};

struct ResultsInfo {
  bool won = false;
  int saved = 0;
  int saveTarget = 0;
  int out = 0;
  int timeLeftSec = 0;
};

class Shell {
public:
  void setStrings(StringTable strings) { strings_ = std::move(strings); }
  void setLevelTable(LevelTable table);
  void setLevelInfos(std::vector<LevelInfo> infos) { infos_ = std::move(infos); }
  void setProgress(Progress progress);
  void setLanguages(std::vector<std::string> langs) { languages_ = std::move(langs); }
  void setProgressPath(std::filesystem::path path) { progressPath_ = std::move(path); }
  const std::filesystem::path& progressPath() const { return progressPath_; }

  const Progress& progress() const { return progress_; }
  Progress& progress() { return progress_; }
  const StringTable& strings() const { return strings_; }
  const LevelTable& table() const { return table_; }
  Screen screen() const { return screen_; }
  bool quitRequested() const { return quit_; }

  std::string_view text(std::size_t index) const { return strings_[index]; }

  // Keyboard: Up/Down/Return/Esc. Returns a play request when entering a level.
  std::optional<PlayRequest> handleKey(int sdlKey);

  void showResults(ResultsInfo info);
  void returnToMap();
  void returnToMain();

  void draw(gfx::Renderer& r, const gfx::Font& font, unsigned fontTex, int vw, int vh) const;

  // After Options changes language, caller reloads .LTX into setStrings.
  bool languageDirty() const { return languageDirty_; }
  void clearLanguageDirty() { languageDirty_ = false; }
  const std::string& language() const { return progress_.language; }

  void applyVolumes(audio::Mixer& mixer) const;

private:
  void activateMain();
  void activateConfirm(bool yes);
  void activateOptions();
  void activateMap();
  std::optional<PlayRequest> activateMapSelect();
  void persist() const;

  const LevelInfo* infoForId(std::string_view id) const;

  StringTable strings_;
  LevelTable table_;
  std::vector<LevelInfo> infos_;
  Progress progress_;
  std::filesystem::path progressPath_ = defaultProgressPath();
  std::vector<std::string> languages_;

  Screen screen_ = Screen::Main;
  int highlight_ = 0;
  int mapGroup_ = 0;
  int mapSlot_ = 0;
  int optionsRow_ = 0;  // 0 language, 1 music, 2 sfx, 3 save, 4 back
  bool quit_ = false;
  bool languageDirty_ = false;
  ResultsInfo results_{};
  bool hasSave_ = false;
};

}  // namespace webo::shell
