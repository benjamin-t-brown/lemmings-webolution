// Tube geometry from level and archive data. The wall uses the
// 7-ring, 32-step inner/outer lattice and the level's 8x3 tile names.
// Optional SurfaceMesh and PolyList 09 sources add stored geometry.
// The route derives per-cell banks from collision words and keys,
// corresponding to the native mesh arrays (0x4ECB00).

#pragma once

#include "app/ActorPose.h"
#include "app/ActorSprite.h"
#include "app/Camera.h"
#include "app/CrtRand.h"
#include "app/DeathParticles.h"
#include "app/HoverFeedback.h"
#include "app/Raster.h"
#include "app/SceneRaster.h"
#include "assets/CellMesh.h"
#include "assets/Bitmap.h"
#include "assets/CollisionGrid.h"
#include "assets/Cylinder.h"
#include "assets/LightMap.h"
#include "assets/Sprite.h"
#include "sim/Terrain.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace webo {
class AssetLibrary;
struct Sprite;
namespace sim { struct OccupancySensor; }
}

namespace webo::app {

class LemmingAnimTracker;
class LemmingMotionTracker;

struct SceneOptions {
  bool drawOuter = true;
  bool drawInner = true;
  bool drawSurface = false;   // stored SurfaceMesh lattice quads
  bool drawPolyList = false;  // stored PolyList 09 records
  // Runtime cell topology derived from the collision grid (src/assets/CellMesh).
  bool drawCellMesh = false;
  // 0x43A4EC submits the front arc in order: INN (c..b, entry+8, material +24),
  // OUT right (d-1..mid+1, mesh4 reversed), OUT left (a..mid-1, mesh0
  // reversed), then OUT centre (mid, mesh0 and mesh4 in forward order,
  // split at cameraColumn).
  bool drawCellRoute = false;
  // Diagnostic overrides for the route passes.
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
  // Draw full wall quads or restrict them to blocks containing solid collision cells.
  bool fullWall = true;
  // Unwrap the outer skin into a 2D 8x3 tile atlas.
  bool flat = false;
  // Actor samples use native +0x08 coordinates. They draw after the route
  // unless the diagnostic composeActorsBeforeRoute is set.
  struct InjectedNativeWalker {
    double plus8X = 0;
    double plus8Y = 0;
    double plus8Z = 0;
    int facing = 1;
    // Walk SPR temporal strip index for this injected sample (default 0).
    int walkTemporalIndex = 0;
    // Negative selects the directional image with selectSpriteView.
    int forcedViewIndex = -1;
    int forcedMirror = -1;
    // Explicit native actor+28 input. Injected/source alive actors default on;
    // the full native lifecycle producer is not reconstructed by this adapter.
    bool renderEnabled = true;
    // Static SPR clip name ("walk", "climb", etc.). walkTemporalIndex selects the entry.
    // Missing clips fall back to "walk".
    const char* clip = "walk";
    // Keep simulation gravity with the draw/pick sample, including after leaving a flipper.
    int verticalDir = 1;
    int sheetType = 0;  // actor+4C: ordinary / water / acid (42DF75..42DF89)
    double drownSeconds = -1.0;  // presentation only; negative outside fluid drowning
    int drownFluidKind = -1;  // -1 keeps static-cell drowning on its existing path
    double drownSurfaceY = 0.0, drownBrightness = 1.0;
    double rotationPhase = 0.0;  // exit enter 418027 sets controller phase to 0.25
    double antiGravTurn = -1.0;  // half-turn progress, negative outside a lift
    // Render-only carried brick. Negative outside Builder. Its Y-up target
    // uses the upcoming cells independently of interpolated body motion.
    double builderPhase = -1.0;
    int builderX = 0, builderY = 0, builderLength = 2;
    // Rat clocks use the selected installed clip's own length/rate.
    double clipSeconds = -1.0;
    bool clipLoops = true;
  };
  std::vector<InjectedNativeWalker> injectedWalkers;
  // Additive submerged actors test opaque scenery depth without the fluid pass.
  std::span<const double> fluidDrownSceneDepth;
  // Save accepted pipe depth before the fluid pass can replace it.
  // Test it with the same unbiased actor plane as other surfaces.
  std::span<const double> pipeSceneDepth;
  bool composeActorsBeforeRoute = false;
  // Diagnostic: replace on any nonzero alpha instead of testing > 0xEF.
  bool overlayCompositeLegacyNonZero = false;
  bool composeSkipActorPose = false;
  // Diagnostic: bypass native +29 and provisional scene-depth rejection.
  bool composeSkipActorVisibility = false;
  // Rotation phase at controller+0x1D8. spriteViewScores adds 0.0625.
  double composeActorPhase = 0.0;
};

// SPR atlas placement and colour are resolved before any silhouette is drawn.
struct PreparedActorSprite {
  SpriteFrame frame;
  Image ownedTexture;  // For diagnostic calls outside the leg's texture cache.
  const Image* texture = nullptr;
  const Image& sheet() const { return texture ? *texture : ownedTexture; }
  ActorSpritePlacement placement{};
  bool mirrored = false;
  bool inverted = false;
  std::array<std::uint8_t, 3> tint{255, 255, 255};
  std::array<double, 3> worldPosition{};
};

struct ActorFootprint {
  // The footprint uses the tube's rings, with no editable surface pool.
  CylinderMesh lattice;
  CollisionGrid collision;
  std::vector<double> gateThickness;
};

class TubeScene {
 public:
  // Same cylinder interpolation used by visible actors. Input is the event's
  // snapshotted terrain feet location; no actor lookup after death/teleport.
  std::array<double, 3> soundWorldPosition(const std::array<double, 3>& position,
                                           int terrainHeight) const;
  void drawHoveredSensor(Image&, const CameraBasis&, const Raster&, const sim::OccupancySensor&) const;
  void prepareHoveredSensor(std::vector<HoverFragment>&, const CameraBasis&, const Raster&,
                            const sim::OccupancySensor&) const;
  static std::expected<TubeScene, std::string> build(const AssetLibrary& lib,
                                                     const CylinderMesh& mesh,
                                                     std::span<const std::byte> levelBytes,
                                                     std::span<const std::uint32_t> grid0,
                                                     bool highDetailTiles = true);

  void render(Raster& raster, const CameraBasis& camera, const SceneOptions& options,
              const AssetLibrary* composeAssets = nullptr) const;
  struct PreparedTube {
    std::vector<PreparedRasterTriangle> triangles;
    // Diagnostic actors can be inserted before the cell route.
    std::size_t beforeRoute = 0;
    bool depthTest = true;
  };
  PreparedTube prepareTube(const CameraBasis&, const SceneOptions&, bool trackHighlights) const;
  static void drawTube(Raster&, const PreparedTube&);

  // Draw after scenery, terrain and fluid. Reset the actor coverage
  // shared by bodies, particles and highlights.
  void drawInjectedWalkers(Raster& raster, const CameraBasis& camera,
                           const SceneOptions& options, const AssetLibrary& lib) const;
  struct PreparedActors {
    struct Bubble { PreparedActorSprite sprite; double strength; };
    struct Actor {
      std::size_t owner;
      PreparedActorSprite body;
      ActorSpritePlacement bounds;
      std::vector<PreparedRasterTriangle> attachment;
      std::vector<Bubble> bubbles;
      std::array<double, 3> drowningPosition{};
      double drowningBrightness = 1.0;
      bool hasAttachment = false;
      bool drowning = false;
    };
    std::vector<Actor> draws;
    std::vector<std::optional<ActorSpritePlacement>> boxes;
    std::shared_ptr<const ActorFootprint> footprint;
    bool enabled = false;
    bool physical = true;
    bool legacyNonZero = false;
  };
  PreparedActors prepareActors(const CameraBasis&, const SceneOptions&, const AssetLibrary&,
                               bool prepareBoxes = false) const;
  static void drawActors(Raster&, const CameraBasis&, const PreparedActors&,
                         std::span<const double> drowningDepth = {}, std::span<const double> pipeDepth = {});

  // Add the selected body sprite only where it owns the completed actor layer.
  bool drawHoveredWalker(Image& target, std::size_t walker, const CameraBasis& camera,
                         const SceneOptions& options, const AssetLibrary& lib,
                         const Raster& sceneRaster) const;
  bool prepareHoveredWalker(std::vector<HoverFragment>&, std::size_t walker, const CameraBasis&,
      const PreparedActors&, const Raster&, std::span<const double> drowningDepth,
      std::span<const double> pipeDepth) const;

  void emitDeathParticles(const sim::Lemming&, bool explosion,
      std::span<const std::array<int, 2>> removedCells, std::uint64_t clockMs,
      int terrainHeight, const AssetLibrary&, bool nukeActive = false);
  // Skill chunks share death debris's clock, lifetime storage and depth pass.
  void emitTerrainDebris(const sim::Lemming&, std::span<const std::array<int, 2>> removedCells,
                        std::uint64_t clockMs, int terrainHeight);
  void emitMinerAxe(const sim::Lemming&, std::uint64_t clockMs, int terrainHeight);
  void emitForcedDeathFlight(const sim::Lemming&, ActorTerrainSample, std::uint64_t clockMs);
  void advanceDeathParticles(std::uint64_t clockMs);
  void drawDeathParticles(Raster&, const CameraBasis&, const AssetLibrary&,
                          std::span<const double> pipeDepth = {}) const;
  struct PreparedParticles {
    struct Sprite {
      PreparedActorSprite sprite;
      std::size_t owner;
      enum class Depth { Origin, AxePlane, FlightPlane } depth;
    };
    using Draw = std::variant<PreparedRasterTriangle, Sprite>;
    std::vector<Draw> draws;
    std::shared_ptr<const ActorFootprint> footprint;
    bool active = false;
  };
  PreparedParticles prepareDeathParticles(const CameraBasis&, const AssetLibrary&) const;
  static void drawParticles(Raster&, const CameraBasis&, const PreparedParticles&,
                            std::span<const double> pipeDepth = {});
  std::size_t deathParticleCount() const { return deathParticles_.size(); }
  // Draw moving fluid after scenery and before actors, on level time.
  void drawExplosionDecals(Raster&, const CameraBasis&) const;
  std::vector<PreparedSceneTriangle> prepareExplosionDecals(const CameraBasis&) const;
  static void drawSceneTriangles(Raster&, std::span<const PreparedSceneTriangle>);
  std::size_t explosionDecalCount() const { return explosionDecals_.size(); }

  // Project one box per injected walker, in vector order. nullopt indicates
  // failed submission, asset or projection checks. A fully occluded actor can
  // keep its box. World walkers have a live prefix in actor order followed by
  // noninteractive exit tails and rats.
  std::vector<std::optional<ActorSpritePlacement>> injectedWalkerScreenBoxes(
      const CameraBasis& camera, const SceneOptions& options, const AssetLibrary& lib) const;
  // The completed frame must match the camera dimensions. Picking uses its boxes.
  std::vector<std::optional<ActorSpritePlacement>> injectedWalkerScreenBoxes(
      const CameraBasis& camera, const SceneOptions& options, const AssetLibrary& lib,
      const Raster& sceneRaster) const;

  // On a terrain version change, copy edited solidity into the collision
  // words (grid Y = height - 1 - terrain Y). Clear removed cells and set
  // new cells to kCellSolid. Synthesize missing keys with 0x43A31D/0x43D2B8.
  // Rebuild the 8x16 cells touched by edits plus a one-cell margin. The
  // route and cellmesh banks use the result. The constant wall is separate.
  void syncTerrain(const sim::Terrain& terrain, std::span<const sim::CounterGate> gates = {});
  void syncCounterGates(std::span<const sim::CounterGate> gates);
  void renderCounterGates(Raster&, const CameraBasis&, const AssetLibrary&,
                         std::span<const sim::CounterGate> gates);
  std::vector<PreparedSceneTriangle> prepareCounterGates(
      const CameraBasis&, const AssetLibrary&, std::span<const sim::CounterGate>);
  std::uint64_t counterGateRevision() const { return counterGateRevision_; }
  std::size_t terrainCellsChanged() const { return terrainCellsChanged_; }
  std::size_t terrainBlocksRebuilt() const { return terrainBlocksRebuilt_; }
  std::size_t terrainVerticesAdded() const { return terrainVerticesAdded_; }
  std::size_t terrainUnresolved() const { return terrainUnresolved_; }
  const std::string& terrainError() const { return terrainError_; }
  const CellMeshes& cellMeshes() const { return cellMesh_; }
  const CylinderMesh& terrainMesh() const { return mesh_; }
  const CollisionGrid& collisionGrid() const { return collision_; }

  // actor+4C selects ordinary / water / acid / evil rat; nullptr if absent.
  // Trackers read per-type strip lengths (special flip: 15, ordinary: 14).
  const SpriteSheet* lemmingSprites(int type = 0) const {
    return type >= 0 && type < 4 && haveLemmingSprites_[type] ? &lemmingSprites_[type] : nullptr;
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
    SceneAddress routeAddress = SceneAddress::Wrap;
  };

  const Tile* findTile(bool outer, int column, int row) const;
  const Texture* builderBrickTexture(const SceneOptions::InjectedNativeWalker&) const;
  static bool prepareBodyHighlight(std::vector<HoverFragment>&, std::size_t walker, const CameraBasis&,
      const PreparedActorSprite&, const Raster&, std::span<const double> depth,
      std::span<const double> pipeDepth, const ActorFootprint*);
  std::shared_ptr<const ActorFootprint> actorFootprint() const;
  std::uint32_t terrainDebrisMaterial(const std::array<int, 2>& cell,
                                    int terrainHeight) const;
  void rewirePoseLevel() const;
  void emitExplosionDecal(const sim::Lemming&, std::uint64_t clockMs, int terrainHeight,
                          bool nukeActive);
  struct ExplosionDecal {
    struct Patch {
      std::array<Vec3, 4> corners;
      std::array<std::array<double, 2>, 4> uv;
    };
    std::uint64_t bornMs = 0;
    std::uint64_t lifetimeMs = 0;
    // An 8x8 rectangle crosses at most one 8-column and one 16-row seam.
    std::array<Patch, 4> patches{};
    std::size_t patchCount = 0;
  };
  SceneRgbaImage explosionTexture_;
  std::vector<ExplosionDecal> explosionDecals_;
  CylinderMesh mesh_;
  CylinderMesh poseCylinderTemplate_;
  std::vector<std::uint32_t> poseGrid0_;
  std::vector<Vec3> poseSurface_;
  ActorPoseLevel poseLevel_{};
  bool havePoseLevel_ = false;
  ActorLightMap actorLight_;
  std::array<SpriteSheet, 4> lemmingSprites_;
  std::array<bool, 4> haveLemmingSprites_{};
  SpriteSheet bubbleSprites_;
  SpriteSheet axeSprites_;
  std::vector<DeathParticle> deathParticles_;
  struct ForcedDeathFlight {
    ActorTerrainSample sample;
    ActorPoseResult launchPose;
    std::array<double, 3> velocity;
    std::uint64_t bornMs;
    int sheetType;
    int facing;
    double spinTurnsPerSecond;
  };
  std::vector<ForcedDeathFlight> forcedDeathFlights_;
  CrtRand forcedDeathSpinRandom_;
  SpriteSheet deathSprites_;
  bool deathSpritesLoaded_ = false;
  std::uint64_t deathClockMs_ = 0;

  bool haveCollision_ = false;
  CollisionGrid collision_;
  mutable std::shared_ptr<const ActorFootprint> footprint_;
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

  // Synthesized static terrain vertices share the editable key table and surface pool.
  bool ensureTerrainVertex(int x, int y, int layer);
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
  // World+0x43C triples map the gate's cylinder coordinates to mutable
  // surface-pool indices (0x43A1C6). Keep these distinct from grid0 keys.
  std::vector<std::array<std::int32_t, 3>> counterGateCoordinates_;
  std::vector<SceneHighlightId> counterGateVertexOwners_;
  std::vector<double> counterGateFractions_;
  std::vector<double> gateThickness_;  // per-cell actor footprint, 1 outside gate bounds
  std::uint64_t counterGateRevision_ = 0;
  std::size_t counterGateMeshedBlocks_ = static_cast<std::size_t>(-1);
  std::array<SceneRgbaImage, 11> counterTextures_;
  bool counterTexturesLoaded_ = false;

  // The stored `f64` cylinder extent (0.36), the walker's pass-bound fraction.
  double cylinderExtent_ = 0.36;
};

// Native +0x08 samples for the MM_CLIMB_002 diagnostic.
std::vector<SceneOptions::InjectedNativeWalker> defaultMmClimb002InjectedWalkers();

// Build render samples in World order using the loaded .LVL grid height.
// Keep live actors first, then noninteractive exit tails and placed rats.
// Without anim, use walk entry 0. Motion tracking supplies fractional
// terrain samples for drawing and picking. Without it, use cell positions.
std::vector<SceneOptions::InjectedNativeWalker> injectedWalkersFromWorld(
    const sim::World& world, int terrainHeight, const LemmingAnimTracker* anim = nullptr);
std::vector<SceneOptions::InjectedNativeWalker> injectedWalkersFromWorld(
    const sim::World& world, int terrainHeight, const LemmingAnimTracker* anim,
    const LemmingMotionTracker* motion);

}  // namespace webo::app
