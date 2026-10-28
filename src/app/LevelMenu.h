#pragma once

#include "assets/Bitmap.h"

#include <array>
#include <expected>
#include <string>

namespace webo {
class AssetLibrary;
}

namespace webo::app {

enum class LevelMenuAction { None, Resume, Retry, MapQuit, UnsupportedOptions };

// The four labelled UI2 rows. The native fifth, empty selector-19 widget is
// intentionally unsupported: its draw/raster behaviour is not established.
class LevelMenu {
 public:
  static std::expected<LevelMenu, std::string> load(const AssetLibrary& library);

  void open();
  void close();
  bool isOpen() const { return open_; }
  void hover(int x, int y);
  LevelMenuAction actionAt(int x, int y) const;
  LevelMenuAction focusedAction() const;
  int focusedIndex() const { return focusedIndex_; }
  void draw(Image& frame) const;

 private:
  Image font_;
  std::array<std::string, 4> labels_{};
  bool open_ = false;
  int focusedIndex_ = 0;
};

} // namespace webo::app
