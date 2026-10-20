#include "app/ActorAnim.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int g_failures = 0;

void check(bool cond, std::string_view what) {
  if (!cond) {
    ++g_failures;
    std::cerr << "  FAIL  " << what << "\n";
  }
}

struct ExpectedFrame {
  int temporalIndex;
  double accumulator;
};

// MM_CLIMB_002 Walker 0x2863630, action 13, temporalCount 12, rate 1.0, loopAtEnd true.
// Measured 2026-10-20 (first packet native_series.json).
constexpr int kElapsedMs[] = {
    6, 6, 8, 7, 8, 6, 8, 6, 6, 8, 6, 8, 6, 7, 8, 6, 8, 6, 7, 7, 7, 8, 6, 6, 8, 6, 9, 6, 8, 6, 6, 7, 7, 8, 6, 8, 6, 7, 7, 7, 7, 7, 8, 6
};

static_assert(sizeof(kElapsedMs) / sizeof(kElapsedMs[0]) == 44);

constexpr ExpectedFrame kExpected[] = {
    { 0, 0.04199488088488579 },
    { 0, 0.04799488186836243 },
    { 0, 0.053994882851839066 },
    { 1, 0.001994884107261896 },
    { 1, 0.00899488478899002 },
    { 1, 0.016994886100292206 },
    { 1, 0.022994887083768845 },
    { 1, 0.03099488839507103 },
    { 1, 0.03699488937854767 },
    { 1, 0.04299489036202431 },
    { 1, 0.05099489167332649 },
    { 1, 0.05699489265680313 },
    { 2, 0.004994893912225962 },
    { 2, 0.010994894430041313 },
    { 2, 0.017994895577430725 },
    { 2, 0.02599489688873291 },
    { 2, 0.03199489787220955 },
    { 2, 0.039994899183511734 },
    { 2, 0.04599490016698837 },
    { 2, 0.052994899451732635 },
    { 2, 0.0599948987364769 },
    { 3, 0.006994897965341806 },
    { 3, 0.014994898810982704 },
    { 3, 0.020994897931814194 },
    { 3, 0.026994898915290833 },
    { 3, 0.03499490022659302 },
    { 3, 0.040994901210069656 },
    { 3, 0.049994900822639465 },
    { 3, 0.055994901806116104 },
    { 4, 0.003994899336248636 },
    { 4, 0.009994899854063988 },
    { 4, 0.015994898974895477 },
    { 4, 0.02299489825963974 },
    { 4, 0.029994897544384003 },
    { 4, 0.03799489885568619 },
    { 4, 0.04399489983916283 },
    { 4, 0.05199490115046501 },
    { 4, 0.05799490213394165 },
    { 5, 0.004994901362806559 },
    { 5, 0.011994902044534683 },
    { 5, 0.018994901329278946 },
    { 5, 0.02599490061402321 },
    { 5, 0.03299489989876747 },
    { 5, 0.040994901210069656 },
    { 5, 0.046994902193546295 },
};

static_assert(sizeof(kExpected) / sizeof(kExpected[0]) == 45);

void forwardReplayNativeSeries() {
  webo::ActorAnimClockState st;
  st.temporalIndex = kExpected[0].temporalIndex;
  st.accumulator = kExpected[0].accumulator;
  st.playDirection = 1;
  st.loopAtEnd = true;
  webo::ActorAnimStrip strip{12, 1.0};
  check(st.temporalIndex == kExpected[0].temporalIndex, "frame 0 index");
  check(std::abs(st.accumulator - kExpected[0].accumulator) <= 1e-6, "frame 0 accumulator");
  for (size_t i = 0; i < sizeof(kElapsedMs) / sizeof(kElapsedMs[0]); ++i) {
    st = webo::advanceActorAnimClock(st, strip, static_cast<double>(kElapsedMs[i]) / 1000.0);
    const auto& want = kExpected[i + 1];
    check(st.temporalIndex == want.temporalIndex,
          "temporal index frame " + std::to_string(i + 1) + " expected " +
              std::to_string(want.temporalIndex) + " got " + std::to_string(st.temporalIndex));
    check(std::abs(st.accumulator - want.accumulator) <= 1e-6,
          "accumulator frame " + std::to_string(i + 1));
  }
}

void testRemainderCarry() {
  webo::ActorAnimClockState st;
  st.accumulator = 0.02;
  webo::ActorAnimStrip strip{12, 1.0};
  st = webo::advanceActorAnimClock(st, strip, 0.03);
  check(st.temporalIndex == 0, "sub-threshold step keeps index");
  check(std::abs(st.accumulator - 0.05) < 1e-12, "remainder accumulates without reset");
  st = webo::advanceActorAnimClock(st, strip, 0.02);
  check(st.temporalIndex == 1, "carried remainder crosses one 0.06 gate");
  check(std::abs(st.accumulator - 0.01) < 1e-12, "subtract 0.06 once remainder carried");
}

void testRateScalesElapsed() {
  webo::ActorAnimClockState st;
  webo::ActorAnimStrip strip{12, 2.0};
  st = webo::advanceActorAnimClock(st, strip, 0.031);
  check(st.temporalIndex == 1, "rate 2.0 scales elapsed before 0.06 gate");
}

void testLoopAtEndWrap() {
  webo::ActorAnimClockState st;
  st.temporalIndex = 11;
  st.accumulator = 0.055;
  st.loopAtEnd = true;
  webo::ActorAnimStrip strip{12, 1.0};
  st = webo::advanceActorAnimClock(st, strip, 0.01);
  check(st.temporalIndex == 0, "+0x201 set wraps to 0 at end");
  st.temporalIndex = 11;
  st.accumulator = 0.055;
  st.loopAtEnd = false;
  st = webo::advanceActorAnimClock(st, strip, 0.01);
  check(st.temporalIndex == 11, "+0x201 clear holds at count-1");
}

}  // namespace

int main() {
  std::cout << "test_actoranim\n";
  testRemainderCarry();
  testRateScalesElapsed();
  testLoopAtEndWrap();
  forwardReplayNativeSeries();
  if (g_failures) {
    std::cerr << g_failures << " failure(s)\n";
    return EXIT_FAILURE;
  }
  std::cout << "  ok\n";
  return EXIT_SUCCESS;
}
