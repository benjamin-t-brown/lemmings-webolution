#include "app/ActorVisibility.h"

#include <cmath>

namespace webo::app {

TubePassBounds tubePassBounds(double selectorTurns, double extent) {
  double centre = (selectorTurns - 0.5) * 32.0;
  while (centre < 0.0) centre += 32.0;
  TubePassBounds bounds{centre, static_cast<int>(centre - extent * 32.0),
      static_cast<int>(centre - extent * 0.7 * 32.0), static_cast<int>(centre),
      static_cast<int>(centre + extent * 0.7 * 32.0),
      static_cast<int>(centre + extent * 32.0)};
  while (bounds.a < 0) {
    bounds.a += 32; bounds.b += 32; bounds.mid += 32; bounds.c += 32; bounds.d += 32;
  }
  while (bounds.a > 32) {
    bounds.a -= 32; bounds.b -= 32; bounds.mid -= 32; bounds.c -= 32; bounds.d -= 32;
  }
  return bounds;
}

bool actorSubmissionVisible(double x, double z, bool enabled, const TubePassBounds& bounds) {
  if (!enabled) return false;
  if (z >= 1.0) return true;
  // 40DEC0: tiny-magnitude snap; negative input adds one turn only, positive
  // input repeatedly subtracts256. Do not replace its quirks with fmod().
  if (std::abs(x) < 9.99999993922529e-9) x = 0.0;
  else if (x < 0.0) x += 256.0;
  else while (x >= 256.0) x -= 256.0;
  int bucket = static_cast<int>(x) / 8;
  if (bounds.d >= 32 && bucket < bounds.a) bucket += 32;
  return bucket >= bounds.a && bucket <= bounds.d;
}

}  // namespace webo::app
