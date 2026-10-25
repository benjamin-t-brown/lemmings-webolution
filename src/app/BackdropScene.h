#pragma once

#include "app/Camera.h"
#include "app/SceneRaster.h"
#include "assets/SceneNode.h"

#include <span>
#include <string>
#include <vector>

namespace webo {
class AssetLibrary;
namespace app {
class Raster;
class TubeScene;
struct BackdropStats { std::size_t roots=0, meshRecords=0, meshTriangles=0, nullMaterials=0, nullMaterialTriangles=0, billboards=0, predicateRejected=0, missingTextures=0, rejected=0, pixels=0; };
class BackdropScene {
 public:
  static std::expected<BackdropScene,std::string> build(const AssetLibrary&, std::span<const std::byte> level, std::string_view levelId);
  void render(Raster&, const CameraBasis&, const AssetLibrary&, BackdropStats&) const;
  bool empty() const { return roots_.empty(); }
 private:
  std::vector<SceneNode> roots_;
};
}
}
