#include "app/LevelEndTransition.h"

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {
int checks = 0;
int failures = 0;

void check(bool condition, std::string_view message) {
  ++checks;
  if (!condition) {
    ++failures;
    std::cerr << "FAIL: " << message << '\n';
  }
}

using webo::app::LevelEndInputs;
using webo::app::LevelEndPhase;
using webo::app::LevelEndTransition;

void advance(LevelEndTransition& transition, double seconds, bool timedOut,
             int retained, int unreleased, int saved, int needed) {
  transition.afterSimulationSlice(seconds,
      LevelEndInputs{timedOut, retained, unreleased, saved, needed});
}

void reachResults(LevelEndTransition& transition, bool timedOut,
                  int retained, int unreleased, int saved, int needed) {
  advance(transition, 99.0, timedOut, retained, unreleased, saved, needed);
  advance(transition, 4.0, false, retained, unreleased, saved, needed);
  advance(transition, 1.5, false, retained, unreleased, saved, needed);
  advance(transition, 0.75, false, retained, unreleased, saved, needed);
}
}

int main() {
  using namespace webo::app;

  LevelEndTransition initial;
  check(initial.phase() == LevelEndPhase::Running && initial.timerSeconds() == 7.0 &&
        !initial.effectActive() && !initial.snapshot(),
        "new transition starts running at seven seconds without effect or snapshot");

  // Reaching the save target alone is not an end condition.
  LevelEndTransition targetOnly;
  advance(targetOnly, 0.25, false, 4, 2, 10, 10);
  check(targetOnly.phase() == LevelEndPhase::Running &&
        targetOnly.timerSeconds() == 7.0 && !targetOnly.snapshot(),
        "meeting save target with population remaining does not trigger");

  // The exhaustion test is equality, not <=: signed selected remaining may
  // make the aggregate exactly zero even with retained population.
  LevelEndTransition exactZero;
  advance(exactZero, 50.0, false, 5, -5, 3, 4);
  check(exactZero.phase() == LevelEndPhase::Pending &&
        exactZero.timerSeconds() == 7.0 && !exactZero.snapshot(),
        "exact retained-plus-signed-unreleased zero triggers without subtracting trigger dt");

  LevelEndTransition negativeSum;
  advance(negativeSum, 0.0, false, 2, -3, 0, 1);
  check(negativeSum.phase() == LevelEndPhase::Running,
        "negative population sum is not treated as exhausted");

  LevelEndTransition timeoutPriority;
  advance(timeoutPriority, 1000.0, true, 500, 500, 0, 1);
  check(timeoutPriority.phase() == LevelEndPhase::Pending &&
        timeoutPriority.timerSeconds() == 7.0,
        "timeout triggers despite nonzero counts and takes priority without trigger subtraction");

  LevelEndTransition strictBoundaries;
  advance(strictBoundaries, 0.0, false, 0, 0, 1, 1);
  advance(strictBoundaries, 4.0, false, 7, 0, 1, 1);
  check(strictBoundaries.phase() == LevelEndPhase::Pending &&
        strictBoundaries.timerSeconds() == 3.0 && !strictBoundaries.effectActive(),
        "timer exactly three neither activates effect nor completes");
  advance(strictBoundaries, 0.5, false, 7, 0, 1, 1);
  check(strictBoundaries.phase() == LevelEndPhase::Pending &&
        strictBoundaries.timerSeconds() == 2.5 && strictBoundaries.effectActive(),
        "below three activates effect but remains pending");
  advance(strictBoundaries, 1.5, false, 7, 0, 1, 1);
  check(strictBoundaries.phase() == LevelEndPhase::Pending &&
        strictBoundaries.timerSeconds() == 1.0 && strictBoundaries.effectActive(),
        "timer exactly one remains pending with effect active");
  advance(strictBoundaries, 0.0, false, 7, 0, 1, 1);
  check(strictBoundaries.phase() == LevelEndPhase::Pending &&
        strictBoundaries.timerSeconds() == 1.0 && strictBoundaries.effectActive(),
        "zero dt does not advance pending timer at exact one boundary");
  advance(strictBoundaries, 0.5, false, 7, 0, 1, 1);
  check(strictBoundaries.phase() == LevelEndPhase::Results &&
        strictBoundaries.timerSeconds() == 7.0 && !strictBoundaries.effectActive() &&
        strictBoundaries.snapshot().has_value(),
        "below one enters results, resets timer, and clears effect");

  LevelEndTransition winningSnapshot;
  reachResults(winningSnapshot, false, 0, 0, 3, 3);
  check(winningSnapshot.phase() == LevelEndPhase::Results && winningSnapshot.snapshot() &&
        winningSnapshot.snapshot()->success && winningSnapshot.snapshot()->saved == 3 &&
        winningSnapshot.snapshot()->needed == 3,
        "results snapshot contains latched success and current saved/needed counts");
  const auto winSnapshot = winningSnapshot.snapshot();
  advance(winningSnapshot, 100.0, true, 0, 0, 0, 99);
  check(winningSnapshot.phase() == LevelEndPhase::Results &&
        winningSnapshot.snapshot() && winSnapshot &&
        winningSnapshot.snapshot()->success == winSnapshot->success &&
        winningSnapshot.snapshot()->saved == winSnapshot->saved &&
        winningSnapshot.snapshot()->needed == winSnapshot->needed,
        "results snapshot and outcome are immutable after later calls");

  LevelEndTransition lateRescueLoss;
  advance(lateRescueLoss, 0.0, true, 0, 4, 0, 1);
  advance(lateRescueLoss, 4.0, false, 0, 0, 2, 2);
  advance(lateRescueLoss, 1.5, false, 0, 0, 2, 2);
  advance(lateRescueLoss, 0.75, false, 0, 0, 2, 2);
  check(lateRescueLoss.snapshot() && !lateRescueLoss.snapshot()->success &&
        lateRescueLoss.snapshot()->saved == 2 && lateRescueLoss.snapshot()->needed == 2,
        "late rescue updates snapshot counts without upgrading latched timeout loss");

  LevelEndTransition lateWin;
  advance(lateWin, 0.0, false, 0, 0, 2, 2);
  advance(lateWin, 4.0, false, 0, 0, 0, 1);
  advance(lateWin, 1.5, false, 0, 0, 0, 1);
  advance(lateWin, 0.75, false, 0, 0, 0, 1);
  check(lateWin.snapshot() && lateWin.snapshot()->success &&
        lateWin.snapshot()->saved == 0 && lateWin.snapshot()->needed == 1,
        "saved/needed changes while pending do not recompute latched win");

  LevelEndTransition freshAfterPending;
  advance(freshAfterPending, 0.0, true, 0, 0, 0, 1);
  LevelEndTransition retry;
  check(retry.phase() == LevelEndPhase::Running && retry.timerSeconds() == 7.0 &&
        !retry.effectActive() && !retry.snapshot(),
        "a new instance after a pending instance has fully reset state");

  std::cout << checks << " checks, " << failures << " failures\n";
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
