// lever-render-001: old whole-frame baselines include the demonstrated invisible
// lever defect. Keep exact RGB/depth comparison everywhere except actual accepted
// stores from installed lever model extents. gravity-gadgets-001 also repairs
// invisible type-3 flippers: retain a separate mask of their actual depth-tested
// stores. No level, rectangle or whole frame is excluded. Frozen renderers remain
// unmodified; RGB/depth must still match everywhere outside these gadget stores.
#pragma once

#include "app/BackdropScene.h"
#include "app/Raster.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace leverbaseline {
struct Pixels {
  std::vector<lemmings::ObjectSourceExtent> extents;
  std::vector<lemmings::ObjectSourceExtent> gravityExtents;
  std::vector<std::uint8_t> mask = std::vector<std::uint8_t>(640 * 480, 0);
  std::vector<std::uint8_t> gravityMask = std::vector<std::uint8_t>(640 * 480, 0);

  void bind(const webo::app::BackdropScene& scene) {
    scene.visitRoots([&](const auto& source, const auto&) {
      if (source.owner == "tagged-lever") extents.push_back(source.extent);
      if (source.owner == "tagged-gravity") gravityExtents.push_back(source.extent);
    });
  }
  void accepted(const webo::app::SceneRasterWriteEvent& event) {
    for (const auto& extent : extents)
      if (event.source >= extent.typed_root_begin && event.source < extent.tree_end) {
        if (!event.depthAssigned || !(event.depthAfter < event.depthBefore))
          throw std::runtime_error("lever baseline requires an actual Less/write-on lever store");
        mask.at(event.pixel) = 1;
      }
    for (const auto& extent : gravityExtents)
      if (event.source >= extent.typed_root_begin && event.source < extent.tree_end) {
        if (!event.depthAssigned || !(event.depthAfter < event.depthBefore))
          throw std::runtime_error("gravity baseline requires an actual Less/write-on flipper store");
        gravityMask.at(event.pixel) = 1;
      }
  }
  void save(const std::filesystem::path& path) const {
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(mask.data()), mask.size());
    if (!out) throw std::runtime_error("write lever pixel mask: " + path.string());
    auto gravityPath = path;
    gravityPath.replace_extension(".gravity-mask");
    std::ofstream gravityOut(gravityPath, std::ios::binary);
    gravityOut.write(reinterpret_cast<const char*>(gravityMask.data()), gravityMask.size());
    if (!gravityOut) throw std::runtime_error("write gravity pixel mask: " + gravityPath.string());
  }
};

inline std::vector<char> read(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("read scene baseline: " + path.string());
  return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

inline bool unchangedOutsideGadgetStores(const std::filesystem::path& before,
    const std::filesystem::path& after, std::string key, std::ostream& log) {
  std::replace(key.begin(), key.end(), ':', '-');
  const auto mask = read(after / (key + ".lever-mask"));
  const auto gravityMask = read(after / (key + ".gravity-mask"));
  const auto oldRgb = read(before / (key + ".rgb")), rgb = read(after / (key + ".rgb"));
  const auto oldDepth = read(before / (key + ".depth")), depth = read(after / (key + ".depth"));
  if (mask.size() != 640 * 480 || gravityMask.size() != mask.size() ||
      rgb.size() != mask.size() * 3 || oldRgb.size() != rgb.size() ||
      depth.size() != mask.size() * sizeof(double) || oldDepth.size() != depth.size())
    throw std::runtime_error("scene baseline storage size: " + key);
  std::size_t rgbChanges = 0, depthChanges = 0, unexpected = 0;
  for (std::size_t p = 0; p < mask.size(); ++p) {
    const bool colorChanged = !std::equal(rgb.begin() + 3*p, rgb.begin() + 3*p + 3, oldRgb.begin() + 3*p);
    const bool depthChanged = !std::equal(depth.begin() + sizeof(double)*p,
        depth.begin() + sizeof(double)*(p+1), oldDepth.begin() + sizeof(double)*p);
    rgbChanges += colorChanged;
    depthChanges += depthChanged;
    unexpected += !mask[p] && !gravityMask[p] && (colorChanged || depthChanged);
  }
  log << "GADGET_BASELINE " << key << " accepted_lever_pixels="
      << std::count(mask.begin(), mask.end(), 1) << " accepted_gravity_pixels="
      << std::count(gravityMask.begin(), gravityMask.end(), 1) << " rgb_changes=" << rgbChanges
      << " depth_changes=" << depthChanges << " unexpected_pixels=" << unexpected << '\n';
  return unexpected == 0;
}
}  // namespace leverbaseline
