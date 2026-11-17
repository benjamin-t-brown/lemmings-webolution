#pragma once

#include <algorithm>
#include <cmath>

namespace webo::app {

// READ 0x4268CA..0x426901, 0x426AC0..0x426AE6, 0x426D90:
// phase += dt*0.5; logical Y = baseY + sin(phase*2*pi)*0.5.
// The stored pi is float-rounded. Period is approximately two level seconds,
// amplitude half a logical height unit. The entire model/basket moves together.
// PROVISIONAL: phase starts at zero for every clone exit, rather than consuming
// the original load-time random phase (0x426D31). Use the existing level clock:
// pause freezes it and fast-forward advances it, without changing simulation.
inline double exitBalloonBob(double levelSeconds) {
  return 0.5 * std::sin(std::max(0.0, levelSeconds) * 3.1415927410125732);
}

}  // namespace webo::app
