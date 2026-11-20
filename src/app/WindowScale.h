#pragma once

#include <algorithm>

namespace webo::app {

inline constexpr int kGameWidth = 640;
inline constexpr int kGameHeight = 480;
inline constexpr int kMaxFixedWindowScale = 3;

// Clone addition. Zero selects Auto on desktop and Fill in the browser.
constexpr int windowScaleOverride(int commandLine, bool automation) {
  if (commandLine) return commandLine;
  return automation ? 1 : 0;
}

constexpr int windowScaleChoice(int saved, int runOverride) {
  return runOverride ? runOverride : saved;
}

constexpr int largestFittingWindowScale(int clientWidth, int clientHeight) {
  return std::max(1, std::min(clientWidth / kGameWidth, clientHeight / kGameHeight));
}

constexpr int desktopWindowScale(int choice, int largestFit) {
  // Without usable desktop bounds, retain the previous ordinary 2x default.
  if (!largestFit) return choice ? choice : 2;
  return choice ? std::min(choice, largestFit) : largestFit;
}

struct WindowScaleMenu {
  int runOverride = 0;
  int largestDesktopFit = 0;
  int fillWidth = 0, fillHeight = 0;
};

}  // namespace webo::app
