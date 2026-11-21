#pragma once

#include <cstdint>

namespace webo {

// Native graphics controller fields used by `0x00468833` (controller = actor+0x128).
struct ActorAnimClockState {
  double accumulator = 0;  // +0x1C0
  int temporalIndex = 0;   // +0x1CC
  int playDirection = 1;     // +0x1D0 (1 forward, else reverse)
  bool wrapped = false;      // +0x200
  bool loopAtEnd = false;  // +0x201: set -> wrap index to 0; clear -> hold at count-1 (0x46891B..0x468959)
};

struct ActorAnimStrip {
  int temporalCount = 0;  // first dword of action strip (`0x0045B8F0`)
  double rate = 1.0;      // strip +0x10
};

inline constexpr double kActorAnimSecPerStep = 0.06;

// `0x00468AB8`: new strip by name — temporal index and accumulator reset.
inline void resetActorAnimOnActionChange(ActorAnimClockState& st) {
  st.temporalIndex = 0;
  st.accumulator = 0;
  st.wrapped = false;
}

// One call to `0x00468833` with `elapsedSeconds` as param_1.
ActorAnimClockState advanceActorAnimClock(ActorAnimClockState st, const ActorAnimStrip& strip,
                                          double elapsedSeconds);

}  // namespace webo
