#pragma once

#include "app/LevelOutcome.h"
#include "assets/Bitmap.h"

#include <array>
#include <string>
#include <vector>

namespace webo::app {

enum class ResultAction { None, Continue, Retry, Quit };

// READ 485E2B/486335: retail background, titles, count lines and visible rows.
// No AVI/fade, keyboard-focus repair or native pixel-acceptance claim.
class ResultsScene {
 public:
  static std::expected<ResultsScene, std::string> build(
      const AssetLibrary& library, const LevelOutcome& outcome);
  const Image& image(int highlight = -1) const;
  int rowAt(int x, int y) const;
  ResultAction actionAt(int x, int y) const;

 private:
  Image backdrop_, font_;
  std::vector<std::string> labels_;
  mutable Image image_;
  mutable int imageHighlight_ = -1;
  std::array<std::array<int, 4>, 3> boxes_{};
  std::array<ResultAction, 3> actions_{};
};

} // namespace webo::app
