#include "app/CrtRand.h"

#include <cmath>

namespace webo::app {

void CrtRand::seedFromMs(double ms) {
  const double scaled = std::trunc(ms * 32767.0);
  std::int64_t wide = INT64_MIN;  // fistp integer-indefinite for NaN / out-of-range
  if (std::isfinite(scaled) && scaled >= -9223372036854775808.0 && scaled < 9223372036854775808.0)
    wide = static_cast<std::int64_t>(scaled);
  state_ = static_cast<std::uint32_t>(static_cast<std::uint64_t>(wide));
}

}  // namespace webo::app
