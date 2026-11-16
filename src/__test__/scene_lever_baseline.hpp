// Record actual depth-tested lever/flipper stores for rendering diagnostics.
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

} // namespace leverbaseline
