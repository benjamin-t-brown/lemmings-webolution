#pragma once

#include "assets/SceneNode.h"
#include <functional>
#include <span>
#include <vector>

namespace webo::app {

struct SecondaryDispatchItem {
  std::size_t nodeSourceOffset = 0;
  std::uint32_t type = 0;
  double key = 0;
  // depth starts at 1 for direct children. parentOffset identifies the owning node.
  std::size_t depth = 0;
  std::size_t parentOffset = 0;
};

// Optional visibility gates. The default leaves nodes ungated.
struct SecondaryDispatchOptions {
  // true: a type-2 node whose serialized visibility byte (+0x110, 0x47B4C8) is 0 draws nothing, and
  // a type-1 node is skipped together with its children when z + headDoubles[0] <= 0
  // (0x46C5C4..0x46C5E4, z = camera-space depth of the node origin, the sort-key z before +0x108).
  bool originalGates = false;
  // Runtime override of the +0x110 byte (the original toggles it only for "cutme" type-1 nodes,
  // 0x45DA0D). Returning true hides the node and its whole subtree (0x46C56B). Empty = never.
  std::function<bool(const SceneNode&)> hidden;
};

// Descending binary insertion at 0x0046B8F4..0x0046BA72 (helper 0x0046C2A0).
// Equal keys follow the insertion search path. Keys must be ordered values.
std::vector<std::size_t> binaryInsertionOrder(std::span<const double> keys);

// Slot-8 sorts direct children by camera-space origin depth plus the
// child's +108 head double. Each is followed by its descendants in
// depth-first file order.
void dispatchSecondaryChildren(const SceneNode& root,
    const std::array<double,12>& parentCamera,
    const std::array<double,12>& sourceRotation,
    const std::function<void(const SecondaryDispatchItem&)>& visitor,
    const SecondaryDispatchOptions& options = {});

}
