#include "app/ActorVisibility.h"

#include <cstdio>
#include <cstdlib>

namespace {
void check(bool ok, const char* what) {
  if (!ok) { std::fprintf(stderr, "FAIL: %s\n", what); std::exit(1); }
}
}

int main() {
  using namespace webo::app;
  const auto front = tubePassBounds(0.625, 0.36);
  check(front.centre == 4 && front.a == 25 && front.b == 28 && front.mid == 36 &&
        front.c == 44 && front.d == 47, "native first-view bounds including truncation/shift");
  check(actorSubmissionVisible(32, .75, true, front), "first-level actor is submitted in initial view");
  const auto back = tubePassBounds(1.125, 0.36);
  check(back.a == 8 && back.d == 31, "opposite-view outer interval");
  check(!actorSubmissionVisible(32, .75, true, back), "same actor hidden from opposite view");
  check(actorSubmissionVisible(32, 1, true, back), "exact shellZ1 bypasses angular culling");
  check(!actorSubmissionVisible(32, .999999, true, back), "shellZ just below1 remains angularly culled");
  check(!actorSubmissionVisible(32, 1, false, back), "actor+28 off still blocks outer-shell path");
  check(!actorSubmissionVisible(32, .75, false, front), "actor+28 off blocks inner-shell path");
  check(actorSubmissionVisible(200, .5, true, front), "inclusive first bucket a25");
  check(!actorSubmissionVisible(199.999, .5, true, front), "bucket before lower endpoint hidden");
  check(actorSubmissionVisible(127.999, .5, true, front), "inclusive last bucket d47 after seam adjustment");
  check(!actorSubmissionVisible(128, .5, true, front), "bucket after upper endpoint hidden");
  const auto equal32 = tubePassBounds(1.75, .25);
  check(equal32.a == 32 && equal32.d == 48, "a32 is retained, not shifted to zero");
  const TubePassBounds seam{0, 31, 31, 32, 32, 32};
  check(actorSubmissionVisible(0, .5, true, seam), "d exactly32 enables seam adjustment");
  const TubePassBounds zero{0, 0, 0, 0, 0, 0};
  check(actorSubmissionVisible(256, .5, true, zero), "actorX256 wraps to0 (unlike target yaw256)");
  check(actorSubmissionVisible(512, .5, true, zero), "positive actorX repeatedly wraps");
  check(actorSubmissionVisible(-1e-10, .5, true, zero), "tiny negative X snaps to0");
  check(!actorSubmissionVisible(-1e-6, .5, true, zero), "larger negative X adds one turn");
  const TubePassBounds last{0, 31, 31, 31, 31, 31};
  check(!actorSubmissionVisible(-264, .5, true, last), "native negative X adds only one turn");
  std::puts("ActorVisibility: all checks passed (submission gate, not pixel occlusion)");
}
