#include "SpriteView.h"

#include <cmath>

namespace webo::gfx {

std::array<double, 2> spriteViewScores(SpriteViewVector cameraToActor,
                                       SpriteViewVector basis, double phase) {
  const double length = std::sqrt(cameraToActor.x * cameraToActor.x +
                                  cameraToActor.y * cameraToActor.y +
                                  cameraToActor.z * cameraToActor.z);
  if (length != 0.0) {
    cameraToActor.x /= length;
    cameraToActor.y /= length;
    cameraToActor.z /= length;
  }

  constexpr double pi = 3.1415927410125732;
  const double firstAngle = phase + 0.0625;
  const std::array angles{firstAngle, firstAngle + 0.1875};
  std::array<double, 2> scores{};
  for (std::size_t i = 0; i < angles.size(); ++i) {
    const double angle = angles[i] * 2.0 * pi;
    const double sine = std::sin(angle);
    const double cosine = std::cos(angle);
    const SpriteViewVector candidate{
        cosine * basis.x - sine * basis.z,
        basis.y,
        sine * basis.x + cosine * basis.z,
    };
    double score = cameraToActor.x * candidate.x +
                   cameraToActor.y * candidate.y +
                   cameraToActor.z * candidate.z;
    if (score < -1.0)
      score += 2.0;
    else if (score > 1.0)
      score -= 2.0;
    scores[i] = score;
  }
  return scores;
}

SpriteViewSelection selectSpriteView(double x, double y, std::size_t count,
                                     bool priorMirror) {
  if (count < 2)
    return {0, priorMirror};

  if (count == 5) {
    bool mirror = false;
    std::size_t index;
    if (x <= 0.0) {
      if (y <= 0.0) {
        index = -x <= -y ? 2 : 3;
      } else if (-x <= y) {
        index = 3;
        mirror = true;
      } else {
        index = 4;
      }
    } else if (y <= 0.0) {
      index = x <= -y ? 1 : 0;
    } else if (x <= y) {
      index = 2;
      mirror = true;
    } else {
      index = 1;
      mirror = true;
    }
    return {index, mirror};
  }

  std::size_t index;
  if (x <= 0.0) {
    if (y <= 0.0)
      index = -x <= -y ? 2 : 3;
    else
      index = -x <= y ? 5 : 4;
  } else if (y <= 0.0) {
    index = x <= -y ? 1 : 0;
  } else {
    index = x <= y ? 6 : 7;
  }
  return {index, priorMirror};
}

}  // namespace webo::gfx
