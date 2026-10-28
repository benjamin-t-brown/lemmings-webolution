#pragma once

#include <string>

namespace webo::app {

// Immutable end-of-run values carried across SDL window teardown. The outcome
// belongs to one selected level; it is not a native save-file representation.
struct LevelOutcome {
  std::string level;
  int saved = 0;
  int total = 0;
  int required = 0;
  int secondsRemaining = 0;
  bool success = false; // Trigger-time latch, not necessarily saved>=required.
};

} // namespace webo::app
