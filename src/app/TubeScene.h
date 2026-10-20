// The tube scene, built only from stored level and archive data.
//
// Geometry sources, all file-side:
//   * the constant tube lattice (CylinderMesh::vertices, 7 rings x 32 steps x
//     inner/outer) subdivided per cell, textured with the level's own 8x3 tile
//     names -- the outer and inner skin;
//   * optionally the SurfaceMesh lattice reconstruction from the stored
//     grid0 index array (src/assets/SurfaceMesh);
//   * optionally the stored PolyList 09 records (src/assets/PolyList).
//
// The A2 fixture read per-cell mesh arrays from a live arena (0x4ECB00). This
// scene never touches that data: A4c decoded the load-time rule and derives the
// same per-cell banks from the stored collision grid and key table
// (src/assets/CellMesh). The arena remains an offline oracle only. `--geom
// route` submits the derived banks in the original walker's pass order; the
// constant lattice is not part of that walker.

#pragma once

#include "app/ActorPose.h"
#include "app/Camera.h"
#include "app/Raster.h"
#include "assets/CellMesh.h"
#include "assets/CollisionGrid.h"
#include "assets/Cylinder.h"
#include "assets/LightMap.h"
#include "assets/Sprite.h"

#include <array>
#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace webo {
class AssetLibrary;
struct Sprite;
}

namespace webo::app {

struct SceneOptions {
  bool drawOuter = true;
  bool drawInner = true;
  bool drawSurface = false;   // stored SurfaceMesh lattice quads
  bool drawPolyList = false;  // stored PolyList 09 records
  // The per-cell mesh the engine derives from the collision grid at level load
  // (src/assets/CellMesh). This is the runtime tube topology, not the smooth
  // constant lattice the wall pass draws.
  bool drawCellMesh = false;
  // The original's actual tube submission. `0x43A4EC` draws, in order:
  // INN (buckets c..b, all bands, the entry+8 whole-block quad, material +24),
  // OUT right (buckets d-1..mid+1, mesh4 reversed), OUT left (buckets a..mid-1,
  // mesh0 reversed), OUT centre (bucket mid, mesh0 forward with coord <
  // cameraColumn then mesh4 forward with coord >= it). Only the 23-bucket-wide
  // front arc is drawn at all. Production uses this alone: the stored constant
  // lattice is not part of that walker.
  bool drawCellRoute = false;
  // Diagnostics for the isolated route-term mutations (never the default).
  bool routeSwapBanks = false;   // swap the mesh0/mesh4 roles in all four passes
  bool routeNoInner = false;     // drop the INN pass
  bool routeFlipCentre = false;  // invert both centre filters
  bool routeTexBankSwap = false; // use the other texture bank, same tile cell
  bool drawWall = true;       // the constant lattice skin (wall pass)
  bool tileRowsFlipped = true;
  bool innerFirst = true;
  double outerContext = 0.0;
  double innerContext = -0.0003;
  bool depthTest = true;
  // Diagnostic: draw every outer/inner cell's wall quad (true) rather than only
  // the cells whose collision column is solid (false). The solid-only mode is
  // the file-side rule measured in the round note: all 156 runtime outer-wall
  // cells are solid, none is empty.
  bool fullWall = true;
  // Debug: unwrap the outer skin into a 2D 8x3 tile atlas instead of the 3D
  // view. Never the default.
  bool flat = false;
  // Injected A6a native `+0x08` samples (not sim lemmings). Empty leaves the
  // tube frame unchanged. Drawn after the route passes unless
  // composeActorsBeforeRoute (diagnostic).
  struct InjectedNativeWalker {
    double plus8X = 0;
    double plus8Y = 0;
    double plus8Z = 0;
    int facing = 1;
  };
  std::vector<InjectedNativeWalker> injectedWalkers;
  bool composeActorsBeforeRoute = false;
  bool composeSkipActorPose = false;
  // Controller +0x1D8 rotation phase; unsettled — default 0 for this slice.
  double composeActorPhase = 0.0;
};

class TubeScene {
 public:
  static std::expected<TubeScene, std::string> build(const AssetLibrary& lib,
                                                     const CylinderMesh& mesh,
                                                     std::span<const std::byte> levelBytes,
                                                     std::span<const std::uint32_t> grid0,
                                                     bool highDetailTiles = true);

  void render(Raster& raster, const CameraBasis& camera, const SceneOptions& options,
              const AssetLibrary* composeAssets = nullptr) const;

  std::size_t outerTiles() const { return outerTileCount_; }
  std::size_t innerTiles() const { return innerTileCount_; }
  std::vector<std::string> missingTiles() const { return missing_; }
  std::size_t surfaceQuads() const { return surfaceQuads_; }
  std::size_t polyListRecords() const { return polyListRecords_; }
  std::size_t cellMeshQuads() const { return cellMesh_.mesh0Quads + cellMesh_.mesh4Quads; }
  std::size_t cellMesh0Quads() const { return cellMesh_.mesh0Quads; }
  std::size_t cellMesh4Quads() const { return cellMesh_.mesh4Quads; }
  std::size_t cellInnerQuads() const { return cellMesh_.innerQuads; }

 private:
  struct Tile {
    Texture texture;
    int column = 0;
    int row = 0;
  };

  const Tile* findTile(bool outer, int column, int row) const;
  void rewirePoseLevel() const;
  void drawInjectedWalkers(Raster& raster, const CameraBasis& camera,
                           const SceneOptions& options, const AssetLibrary& lib) const;

  CylinderMesh mesh_;
  CylinderMesh poseCylinderTemplate_;
  std::vector<std::uint32_t> poseGrid0_;
  std::vector<Vec3> poseSurface_;
  ActorPoseLevel poseLevel_{};
  bool havePoseLevel_ = false;
  ActorLightMap actorLight_;
  SpriteSheet lemmingSprites_;
  bool haveLemmingSprites_ = false;

  bool haveCollision_ = false;
  CollisionGrid collision_;
  std::vector<Tile> outer_;
  std::vector<Tile> inner_;
  std::size_t outerTileCount_ = 0;
  std::size_t innerTileCount_ = 0;
  std::vector<std::string> missing_;

  // Reconstructed cut-in geometry (optional draw sources).
  struct SurfaceQuad {
    Vec3 v[4];
    bool outerFacing = true;
  };
  std::vector<SurfaceQuad> surface_;
  std::size_t surfaceQuads_ = 0;
  struct PolyFace {
    Vec3 v[3];
    std::array<double, 3> modulate{1, 1, 1};
    bool outerFacing = true;
  };
  std::vector<PolyFace> poly_;
  std::size_t polyListRecords_ = 0;

  // The per-cell mesh derived from the collision grid (src/assets/CellMesh).
  CellMeshes cellMesh_;

  // The stored `f64` cylinder extent (0.36), the walker's pass-bound fraction.
  double cylinderExtent_ = 0.36;
};

// Settled A6a native `+0x08` samples for MM_CLIMB_002 injected compose (not sim).
std::vector<SceneOptions::InjectedNativeWalker> defaultMmClimb002InjectedWalkers();

}  // namespace webo::app
