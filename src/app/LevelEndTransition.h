#pragma once

#include <optional>

namespace webo::app {

enum class LevelEndPhase { Running, Pending, Results };

struct LevelEndInputs {
  bool timedOut;
  int retainedNonRat;
  int selectedUnreleased;
  int saved;
  int needed;
};

struct LevelEndSnapshot {
  bool success;
  int saved;
  int needed;
};

// READ 444BC2 /445498. Called AFTER an admitted simulation slice. No wall
// timer, target-only finish, inferred fade, or results-phase re-evaluation.
class LevelEndTransition {
 public:
  void afterSimulationSlice(double seconds, const LevelEndInputs& inputs);
  LevelEndPhase phase() const { return phase_; }
  double timerSeconds() const { return timer_; }
  bool effectActive() const { return effectActive_; }
  const std::optional<LevelEndSnapshot>& snapshot() const { return snapshot_; }

 private:
  LevelEndPhase phase_ = LevelEndPhase::Running;
  double timer_ = 7.0;
  bool winLatched_ = false;
  bool effectActive_ = false;
  double effectElapsed_ = 0.0;
  std::optional<LevelEndSnapshot> snapshot_;
};

} // namespace webo::app
