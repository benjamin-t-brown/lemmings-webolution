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
#include "app/ActorSprite.h"
#include "app/Camera.h"
#include "app/Raster.h"
#include "assets/CellMesh.h"
#include "assets/CollisionGrid.h"
#include "assets/Cylinder.h"
#include "assets/LightMap.h"
#include "assets/Sprite.h"
#include "sim/Terrain.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace webo {
class AssetLibrary;
struct Sprite;
}

namespace webo::app {

class LemmingAnimTracker;
class LemmingMotionTracker;

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
    // Walk SPR temporal strip index for this injected sample (default 0).
    int walkTemporalIndex = 0;
    // Diagnostic (A6d6): force chooser output; negative = use selectSpriteView.
    int forcedViewIndex = -1;
    int forcedMirror = -1;
    // Explicit native actor+28 input. Injected/source alive actors default on;
    // the full native lifecycle producer is not reconstructed by this adapter.
    bool renderEnabled = true;
    // actor-anim-001: the lemming SPR clip this walker draws ("walk", "climb", ...) and, in
    // `walkTemporalIndex`, the temporal entry inside that clip. A literal with static storage
    // (app/LemmingAnim.h names them); a clip the sheet lacks falls back to "walk".
    // Last member so positional initialisers of the fields above are unaffected.
    const char* clip = "walk";
    // Simulation gravity persists after leaving a flipper cell. Keep it with
    // the shared draw/pick sample rather than resampling the terrain at draw time.
    int verticalDir = 1;
    int sheetType = 0;  // actor+4C: ordinary / water / acid (42DF75..42DF89)
    double drownSeconds = -1.0;  // presentation only; negative outside fluid drowning
    int drownFluidKind = -1;  // -1 keeps static-cell drowning on its existing path
    double drownSurfaceY = 0.0, drownBrightness = 1.0;
  };
  std::vector<InjectedNativeWalker> injectedWalkers;
  // U2: opaque scenery depth with the fluid pass excluded, only for the
  // additive submerged figure/effects. Normal actors use the completed depth.
  std::span<const double> fluidDrownSceneDepth;
  bool composeActorsBeforeRoute = false;
  // Diagnostic (A6d9): composite overlay with legacy `a != 0` replace instead of A4y `> 0xEF`.
  bool overlayCompositeLegacyNonZero = false;
  bool composeSkipActorPose = false;
  // Diagnostic only: project/render even actors rejected by native +29 gate
  // or the provisional actor scene-depth test. Production drawing/picking leaves off.
  bool composeSkipActorVisibility = false;
  // Controller +0x1D8 rotation phase (A3 chooser input; spriteViewScores adds 0.0625).
  // A6d5 live read: all three matched-pause walkers hold 0.0 at controller+0x1D8.
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

  // Actor pass after all scenery/terrain/fluid passes. render() also calls this
  // for standalone scene consumers. Does not change scene depth.
  void drawInjectedWalkers(Raster& raster, const CameraBasis& camera,
                           const SceneOptions& options, const AssetLibrary& lib) const;

  // Redraw only the selected actor silhouette, using the same preparation and
  // scene-depth mask as drawing/picking. Never changes picking or scene depth.
  bool drawHoveredWalker(Image& target, std::size_t walker, const CameraBasis&,
                         const SceneOptions&, const AssetLibrary&, const Raster&) const;

  // Read-only screen box for each `options.injectedWalkers` entry, in that
  // vector's order (forward). Each placement is the one `drawInjectedWalkers`
  // would pass to `drawActorSprite` for that walker; nullopt when that walker
  // is skipped or fully depth-occluded. Sprites use the same scene-depth
  // mask as drawing. Without a rendered frame, a tube-only depth pass is used.
  //
  // `injectedWalkersFromWorld` emits one walker per alive lemming, in
  // `World::lemmings()` order. While every lemming is alive the returned
  // vector is index-aligned with `World::lemmings()`. A dead or exited
  // lemming has no walker, so it has no entry here.
  std::vector<std::optional<ActorSpritePlacement>> injectedWalkerScreenBoxes(
      const CameraBasis& camera, const SceneOptions& options, const AssetLibrary& lib) const;
  // Production passes the completed frame explicitly so picking uses exactly
  // the scene depth that clipped its sprites; the three-argument diagnostic
  // overload above constructs a fresh tube-only depth pass.
  std::vector<std::optional<ActorSpritePlacement>> injectedWalkerScreenBoxes(
      const CameraBasis& camera, const SceneOptions& options, const AssetLibrary& lib,
      const Raster& sceneRaster) const;

  // terrain-render-001: brings the drawn tube in line with the live simulation terrain, so
  // cells a skill removed stop being drawn and cells a Builder added are. The cheap path
  // (terrain version unchanged) is one integer compare. On a change it diffs the terrain
  // against the solidity the mesh was last built from (grid row y is terrain row
  // height - 1 - y), updates the collision words the mesh rule reads (a removed cell becomes
  // 0 like the original's clear; an added cell becomes kCellSolid), fills any key the stored
  // table lacks with the native 0x43A31D/0x43D2B8 vertex, and regenerates only the 8x16
  // blocks the changed cells touch, padded by one cell, with the per-cell rule
  // (rebuildCellMeshCells). The `--geom route` / `cellmesh` banks follow; the constant
  // lattice skin (`--geom wall`) is not edited.
  void syncTerrain(const sim::Terrain& terrain);
  std::size_t terrainCellsChanged() const { return terrainCellsChanged_; }
  std::size_t terrainBlocksRebuilt() const { return terrainBlocksRebuilt_; }
  std::size_t terrainVerticesAdded() const { return terrainVerticesAdded_; }
  std::size_t terrainUnresolved() const { return terrainUnresolved_; }
  const std::string& terrainError() const { return terrainError_; }
  const CellMeshes& cellMeshes() const { return cellMesh_; }
  const CollisionGrid& collisionGrid() const { return collision_; }

  // actor+4C selects ordinary / water / acid; nullptr if that sheet is absent.
  // Trackers read per-type strip lengths (special flip: 15, ordinary: 14).
  const SpriteSheet* lemmingSprites(int type = 0) const {
    return type >= 0 && type < 3 && haveLemmingSprites_[type] ? &lemmingSprites_[type] : nullptr;
  }

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
  CylinderMesh mesh_;
  CylinderMesh poseCylinderTemplate_;
  std::vector<std::uint32_t> poseGrid0_;
  std::vector<Vec3> poseSurface_;
  ActorPoseLevel poseLevel_{};
  bool havePoseLevel_ = false;
  ActorLightMap actorLight_;
  std::array<SpriteSheet, 3> lemmingSprites_;
  std::array<bool, 3> haveLemmingSprites_{};
  SpriteSheet bubbleSprites_;

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

  // syncTerrain state. `meshedSolid_` is the solidity (grid row order) the current `cellMesh_`
  // was built from; `editKeys_` is a copy of the stored key table that gains pool vertices
  // (appended to `mesh_.surface`) for the keys edited blocks need. Both are created on the
  // first change only, so a level that is never edited costs nothing.
  bool terrainSeen_ = false;
  bool terrainFailed_ = false;
  std::uint64_t terrainVersion_ = 0;
  std::vector<std::uint8_t> meshedSolid_;
  std::vector<std::uint32_t> editKeys_;
  std::size_t terrainCellsChanged_ = 0;
  std::size_t terrainBlocksRebuilt_ = 0;
  std::size_t terrainVerticesAdded_ = 0;
  std::size_t terrainUnresolved_ = 0;
  std::string terrainError_;

  // The stored `f64` cylinder extent (0.36), the walker's pass-bound fraction.
  double cylinderExtent_ = 0.36;
};

// Settled A6a native `+0x08` samples for MM_CLIMB_002 injected compose (not sim).
std::vector<SceneOptions::InjectedNativeWalker> defaultMmClimb002InjectedWalkers();

// D3b: one `InjectedNativeWalker` per live `sim::Lemming`, through the same
// `terrainSampleFromLemming` adapter the A6a fixture samples stand in for
// (ActorPose.h), so `drawInjectedWalkers` stays the only actor draw path --
// a ticking `World`'s lemmings reuse it rather than growing a second one.
// `terrainHeight` is the loaded level's own grid height (matches
// `sim::World::terrain().height()` when both are read from the same `.LVL`
// collision grid, which `webolution`'s `main.cpp` guarantees by construction).
//
// Only lemmings with `Lemming::alive()` (not yet Exited or Dead) are
// included -- Dead/Exited lemmings stay in `World::lemmings()` forever (see
// World.h) and are not part of what is "on the tube" any more.
//
// Without `anim` every entry draws "walk" at temporal index 0 (the D3b behaviour, kept for
// the fixtures that build walkers without a clock). With `anim` (actor-anim-001) each entry
// takes the clip and temporal index the tracker holds for its lemming (app/LemmingAnim.h).
// With `motion` (P7), confirmed terrain coordinates are interpolated in the
// presentation tracker. The fractional sample feeds drawing AND screen boxes;
// a null tracker preserves the integer-cell diagnostic/fixture behavior.
std::vector<SceneOptions::InjectedNativeWalker> injectedWalkersFromWorld(
    const sim::World& world, int terrainHeight, const LemmingAnimTracker* anim = nullptr);
std::vector<SceneOptions::InjectedNativeWalker> injectedWalkersFromWorld(
    const sim::World& world, int terrainHeight, const LemmingAnimTracker* anim,
    const LemmingMotionTracker* motion);

}  // namespace webo::app
