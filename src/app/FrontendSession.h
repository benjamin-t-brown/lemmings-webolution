#pragma once

#include "app/MapScene.h"
#include "app/LevelOutcome.h"
#include "app/ResultsScene.h"
#include "shell/Frontend.h"

#include <array>
#include <expected>
#include <optional>
#include <string>

namespace webo::app {

// Retail assets plus the retained B2 screen sequence. SDL is only an input
// adapter: the model does not claim that an SDL click proves native DirectInput
// binding. New-game progress starts empty in memory; no installed save is read
// or reset. Results Continue/retry/Quit preserve this owned progress; New Game Yes
// clears it. Other menu actions and keyboard traversal remain unimplemented.
class FrontendSession {
 public:
  static std::expected<FrontendSession, std::string> create(
      const AssetLibrary& library);

  const Image& image() const;
  shell::Screen screen() const { return state_.screen(); }
  int constructionCounter() const { return state_.counter(); }
  shell::Outcome pressReturn();
  bool click(int x, int y);
  bool clickMap(int x, int y);
  const std::optional<std::string>& selectedLevel() const { return selectedLevel_; }
  bool levelIntroEnabled() const { return levelIntroEnabled_; }
  bool retryLevel();
  bool abandonLevelToMap();
  const std::array<std::int32_t, 102>& completionFlags() const { return completionFlags_; }
  const std::array<std::int32_t, 102>& mapStates() const { return map_.states(); }
  const std::optional<LevelOutcome>& outcome() const { return outcome_; }
  std::expected<void, std::string> completeLevel(const AssetLibrary& library,
                                               const LevelOutcome& outcome);

 private:
  FrontendSession() = default;
  shell::Outcome confirmChoice(bool yes);
  Image renderMapImage(const MapScene& map) const;
  shell::Frontend state_;
  Image menu_, confirm_, mapImage_;
  Image font_, initialMapImage_;
  std::string mapFooterLabel_;
  int firstMenuLeft_ = 0, firstMenuRight_ = 0;
  std::array<std::array<int, 4>, 2> confirmBoxes_{};
  std::array<int, 4> mapFooterBox_{};
  MapScene map_;
  MapScene initialMap_;
  std::array<std::int32_t, 102> completionFlags_{};
  std::optional<std::size_t> selectedNode_;
  std::optional<std::string> selectedLevel_;
  std::optional<LevelOutcome> outcome_;
  std::optional<ResultsScene> results_;
  // READ4E3584: next-load intro preference, not a routing or reload flag.
  bool levelIntroEnabled_ = true;
};

}  // namespace webo::app
