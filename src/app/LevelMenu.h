#pragma once

#include "assets/Bitmap.h"

#include <array>
#include <expected>
#include <string>

namespace webo {
class AssetLibrary;
}

namespace webo::app {

enum class LevelMenuAction { None, Resume, Retry, MapQuit, SoundOptions, Examine };

// The four labelled UI2 rows followed by the clone's Examine action.
// The native empty selector-19 widget stays unsupported.
class LevelMenu {
 public:
  static std::expected<LevelMenu, std::string> load(const AssetLibrary& library);

  void open();
  void close();
  bool isOpen() const { return open_; }
  void hover(int x, int y);
  void moveFocus(int direction);
  LevelMenuAction actionAt(int x, int y) const;
  LevelMenuAction focusedAction() const;
  int focusedIndex() const { return focusedIndex_; }
  void draw(Image& frame) const;
  bool touchButtonAt(int x, int y) const;
  void drawTouchButton(Image& frame) const;

 private:
  Image font_;
  std::array<std::string, 5> labels_{};
  bool open_ = false;
  int focusedIndex_ = 0;
};

} // namespace webo::app
