#pragma once

#include "assets/SceneNode.h"
#include <functional>
#include <span>

namespace webo::app {

struct SecondaryDispatchItem {
  std::size_t nodeSourceOffset = 0;
  std::uint32_t type = 0;
  double key = 0;
};

// The retained slot-8 child stream. Keys use the child's +108 head double and
// The transformed child translation is dispatched through a common render/test seam.
void dispatchSecondaryChildren(const SceneNode& root,
    const std::array<double,12>& parentCamera,
    const std::array<double,12>& sourceRotation,
    const std::function<void(const SecondaryDispatchItem&)>& visitor);

}
