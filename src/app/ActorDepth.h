#pragma once

#include <array>
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
  static constexpr auto firstParticleOwner = noOwner / 2;
  void reset(std::size_t pixels) {
    indices_.assign(pixels / blockSize + (pixels % blockSize != 0), noOwner);
    blocks_.clear();
    pixels_ = pixels;
  }
  void clear() { reset(0); }
  bool visible(std::size_t pixel, double z, std::size_t owner) const {
    const auto* stored = lookup(pixel);
    return std::isfinite(z) && (!stored || z < stored->depth ||
        (z == stored->depth && owner <= stored->owner));
  }
  bool submit(std::size_t pixel, double z, std::size_t owner) {
    if (pixel >= size() || !visible(pixel, z, owner)) return false;
    auto& index = indices_[pixel / blockSize];
    if (index == noOwner) {
      blocks_.emplace_back();
      index = blocks_.size() - 1;
    }
    blocks_[index][pixel % blockSize] = {z, owner};
    return true;
  }
  bool owns(std::size_t pixel, std::size_t owner) const {
    const auto* stored = lookup(pixel);
    return !stored || stored->owner == noOwner || stored->owner == owner;
  }
  double depthAt(std::size_t pixel) const {
    const auto* stored = lookup(pixel);
    return stored ? stored->depth : std::numeric_limits<double>::infinity();
  }
  std::size_t ownerAt(std::size_t pixel) const {
    const auto* stored = lookup(pixel);
    return stored ? stored->owner : noOwner;
  }
  std::size_t size() const { return indices_.empty() ? 0 : pixels_; }

 private:
  // Bodies usually cover a small fraction of the screen. An absent block
  // represents 64 untouched pixels with infinite depth and no owner.
  static constexpr std::size_t blockSize = 64;
  struct Pixel {
    double depth = std::numeric_limits<double>::infinity();
    std::size_t owner = noOwner;
  };
  const Pixel* lookup(std::size_t pixel) const {
    if (pixel >= size()) return nullptr;
    const auto index = indices_[pixel / blockSize];
    return index == noOwner ? nullptr : &blocks_[index][pixel % blockSize];
  }
  std::size_t pixels_ = 0;
  std::vector<std::size_t> indices_;
  std::vector<std::array<Pixel, blockSize>> blocks_;
};

}  // namespace webo::app
