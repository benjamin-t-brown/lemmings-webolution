#include "app/ActorAnim.h"

#include <cmath>

namespace webo {

namespace {

constexpr double kDust = 9.99999993922529e-09;

void zeroDust(double& acc) {
  if (acc <= kDust) {
    acc = 0;
  }
}

void wrapForward(ActorAnimClockState& st, int temporalCount) {
  if (st.temporalIndex != temporalCount) {
    return;
  }
  if (!st.loopAtEnd) {
    st.temporalIndex = temporalCount - 1;
  } else {
    st.temporalIndex = 0;
  }
  st.wrapped = true;
}

void wrapReverse(ActorAnimClockState& st, int temporalCount) {
  if (st.temporalIndex >= 0) {
    return;
  }
  if (!st.loopAtEnd) {
    st.temporalIndex = 0;
  } else {
    st.temporalIndex = temporalCount - 1;
  }
  st.wrapped = true;
}

}  // namespace

ActorAnimClockState advanceActorAnimClock(ActorAnimClockState st, const ActorAnimStrip& strip,
                                          double elapsedSeconds) {
  if (strip.temporalCount <= 0 || strip.rate <= 0.0) {
    return st;
  }
  st.wrapped = false;
  st.accumulator += elapsedSeconds * strip.rate;

  if (st.playDirection == 1) {
    while (st.accumulator > kActorAnimSecPerStep) {
      ++st.temporalIndex;
      wrapForward(st, strip.temporalCount);
      st.accumulator -= kActorAnimSecPerStep;
      zeroDust(st.accumulator);
    }
  } else {
    while (st.accumulator > kActorAnimSecPerStep) {
      --st.temporalIndex;
      wrapReverse(st, strip.temporalCount);
      st.accumulator -= kActorAnimSecPerStep;
      if (st.accumulator < kDust) {
        st.accumulator = 0;
      }
    }
  }
  return st;
}

}  // namespace webo
