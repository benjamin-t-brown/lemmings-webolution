#pragma once

#include <string>

namespace webo::app {

// Immutable outcome for one level, carried across SDL teardown.
struct LevelOutcome {
  std::string level;
  int saved = 0;
  int total = 0;
  int required = 0;
  int secondsRemaining = 0;
  bool success = false; // Trigger-time latch, not necessarily saved>=required.
};

} // namespace webo::app
