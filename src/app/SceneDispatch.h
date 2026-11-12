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
  // scene-nested-impl-001 (additive): 1 for the root's direct children, 2 for their children, ...
  // parentOffset is the source offset of the node that owns this one (the root for depth 1).
  std::size_t depth = 0;
  std::size_t parentOffset = 0;
};

// Optional per-node gates of the original draw routines (scene-nested-research-001 E4/E7/E8).
// A default-constructed value keeps the legacy leaf-only behaviour (no gates), which the
// pre-existing dispatch fixtures rely on; BackdropScene passes originalGates=true.
struct SecondaryDispatchOptions {
  // true: a type-2 node whose serialized visibility byte (+0x110, 0x47B4C8) is 0 draws nothing, and
  // a type-1 node is skipped together with its children when z + headDoubles[0] <= 0
  // (0x46C5C4..0x46C5E4, z = camera-space depth of the node origin, the sort-key z before +0x108).
  bool originalGates = false;
  // Runtime override of the +0x110 byte (the original toggles it only for "cutme" type-1 nodes,
  // 0x45DA0D). Returning true hides the node and its whole subtree (0x46C56B). Empty = never.
  std::function<bool(const SceneNode&)> hidden;
};

// The exact descending binary-insertion order of 0x0046B8F4..0x0046BA72 (insert helper
// 0x0046C2A0). Returns the indices of `keys` in draw order. Equal keys are placed by the search
// path, not by position, so this is neither stable nor reverse-stable and differs from
// std::sort(key >) wherever keys tie. Unordered (NaN) keys are outside the contract.
std::vector<std::size_t> binaryInsertionOrder(std::span<const double> keys);

// The retained slot-8 stream: the root's direct children are ordered by key
// (camera-space depth of the child origin + the child's +108 head double) with
// binaryInsertionOrder; each ordered child is followed immediately by its descendants,
// depth-first in file order (never reordered), before the next ordered direct child.
// The dispatch is a common render/test seam.
void dispatchSecondaryChildren(const SceneNode& root,
    const std::array<double,12>& parentCamera,
    const std::array<double,12>& sourceRotation,
    const std::function<void(const SecondaryDispatchItem&)>& visitor,
    const SecondaryDispatchOptions& options = {});

}
