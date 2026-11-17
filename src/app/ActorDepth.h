#pragma once

#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace webo::app {

// Separate from scenery Z: actors, carried bricks and particles share this
// opaque coverage layer without changing terrain/fluid depth or picking boxes.
// Exact ties belong to the lowest stable submission index, independent of sort.
class ActorDepth {
 public:
  static constexpr auto noOwner = std::numeric_limits<std::size_t>::max();
  void reset(std::size_t pixels) {
    depth_.assign(pixels, std::numeric_limits<double>::infinity());
    owner_.assign(pixels, noOwner);
  }
  void clear() { depth_.clear(); owner_.clear(); }
  bool visible(std::size_t pixel, double z, std::size_t owner) const {
    return std::isfinite(z) && (pixel >= depth_.size() || z < depth_[pixel] ||
        (z == depth_[pixel] && owner <= owner_[pixel]));
  }
  bool submit(std::size_t pixel, double z, std::size_t owner) {
    if (pixel >= depth_.size() || !visible(pixel, z, owner)) return false;
    depth_[pixel] = z;
    owner_[pixel] = owner;
    return true;
  }
  bool owns(std::size_t pixel, std::size_t owner) const {
    return pixel >= owner_.size() || owner_[pixel] == noOwner || owner_[pixel] == owner;
  }
  std::size_t size() const { return depth_.size(); }

 private:
  std::vector<double> depth_;
  std::vector<std::size_t> owner_;
};

}  // namespace webo::app
