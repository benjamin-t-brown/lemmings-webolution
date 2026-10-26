#pragma once

#include "app/Camera.h"
#include "app/SceneBillboardSource.h"
#include "app/SceneRaster.h"
#include "app/SceneDispatch.h"
#include "assets/SceneNode.h"

#include <span>
#include <string>
#include <vector>
#include <functional>

namespace webo {
class AssetLibrary;
namespace app {
class Raster;
class TubeScene;
struct BackdropStats { std::size_t roots=0, meshRecords=0, stageMeshRecords=0, meshTriangles=0, nullMaterials=0, nullMaterialTriangles=0, billboards=0, stageBillboards=0, predicateRejected=0, missingTextures=0, rejected=0, pixels=0; };
class BackdropScene {
 public:
  enum class Stage { Primary, LegacyUnattributed, Secondary };
  static std::expected<BackdropScene,std::string> build(const AssetLibrary&, std::span<const std::byte> level, std::string_view levelId);
  void render(Raster&, const CameraBasis&, const AssetLibrary&, BackdropStats&) const;
  void renderStage(Raster&, const CameraBasis&, const AssetLibrary&, BackdropStats&, Stage,
                  SceneRasterObserver* observer = nullptr) const;
  // Runs the production parsed-tree traversal; explicit inputs are a fixture seam.
  void visitBillboards(const BillboardSourceInputs& cameraChain,
                       const std::function<void(const SceneBillboardSource&)>& visitor) const;
  // Replays the production kind-5 predicate context for value checks. The visitor
  // receives each source record offset and the branch result (true means pass).
   void visitKind5Predicates(const CameraBasis& camera,
       const std::function<void(std::size_t, bool)>& visitor) const;
  void visitSecondaryDispatch(const CameraBasis& camera,
       const std::function<void(const SecondaryDispatchItem&)>& visitor) const;
  bool empty() const { return roots_.empty(); }
 private:
  std::vector<SceneNode> roots_;
};
}
}
