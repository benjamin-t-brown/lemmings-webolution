#pragma once

#include <algorithm>
#include <cmath>

namespace webo::app {

inline constexpr double kExitJumpSeconds = 1.0;
inline constexpr double kExitBalloonSwayBlendSeconds = 0.25;

// Float-rounded pi from 0x4B0F88.
inline constexpr double kExitBalloonPi = 3.1415927410125732;

// Phase advances by 0.5 cycles per level second.
inline double exitBalloonPhaseRadians(double levelSeconds) {
  return std::max(0.0, levelSeconds) * kExitBalloonPi;
}

// 0x4268CA..0x426901, 0x426AC0..0x426AE6 and 0x426D90 bob the whole
// exit with a two-second period and amplitude 0.5 height units.
// Provisional clone phase starts at zero (native random phase at 0x426D31).
// Level time supplies pause and fast-forward.
inline double exitBalloonBob(double levelSeconds) {
  return 0.5 * std::sin(exitBalloonPhaseRadians(levelSeconds));
}

}  // namespace webo::app
