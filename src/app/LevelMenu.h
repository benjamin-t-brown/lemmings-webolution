#pragma once

#include "shell/HudText.h"

#include <array>
#include <cstddef>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace webo {
class AssetLibrary;
}

namespace webo::app {

enum class LevelMenuAction { None, Resume, Retry, MapQuit, SoundOptions, Examine };

// The four labelled UI2 actions and the clone's Examine Level action.
// The native empty selector-19 widget stays unsupported.
class LevelMenu {
 public:
  static std::expected<LevelMenu, std::string> load(const AssetLibrary& library,
                                                   std::string_view levelId);

  void open();
  void close();
  bool isOpen() const { return open_; }
  void hover(int x, int y);
  void moveFocus(int direction);
  LevelMenuAction actionAt(int x, int y) const;
  LevelMenuAction focusedAction() const;
  int focusedIndex() const { return focusedIndex_; }
  void draw(Image& frame) const;
  bool touchButtonAt(int x, int y, bool portrait = false) const;
  void drawTouchButton(Image& frame, bool portrait = false) const;

 private:
  struct Row {
    LevelMenuAction action;
    std::optional<std::size_t> labelIndex;
    std::string_view cloneLabel;
  };

  static constexpr std::array kRows{
      Row{LevelMenuAction::Resume, 44, {}},
      Row{LevelMenuAction::Retry, 48, {}},
      Row{LevelMenuAction::SoundOptions, 3, {}},
      Row{LevelMenuAction::Examine, std::nullopt, "Examine Level"},
      Row{LevelMenuAction::MapQuit, 43, {}},
  };

  void layoutTitle(std::string_view title);

  Image font_;
  std::array<std::string, kRows.size()> labels_{};
  std::vector<shell::HudLine> titleLines_;
  int rowBaseY_ = 0;
  bool open_ = false;
  int focusedIndex_ = 0;
};

} // namespace webo::app
