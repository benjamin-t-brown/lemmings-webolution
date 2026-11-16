#pragma once

#include "app/Camera.h"
#include "app/CrtRand.h"
#include "app/SceneBillboardSource.h"
#include "assets/Sprite.h"
#include "app/SceneRaster.h"
#include "app/SceneDispatch.h"
#include "app/HatchAnimation.h"
#include "app/HoverFeedback.h"
#include "assets/SceneNode.h"
#include "lemmings/object_base_prefix.hpp"
#include "lemmings/world_coordinates.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <vector>
#include <functional>
#include <utility>

namespace webo {
class AssetLibrary;
namespace sim { struct Crab; struct NamedHazard; }
namespace app {
class Raster;
class TubeScene;
// App-owned animation data keeps BackdropScene's layout compatible with the
// retained demonstrations that compile older headers against current support.
struct EntranceExitPresentation {
  struct Entrance { std::size_t root; HatchAnimation animation; };
  struct Exit {
    std::size_t root;
    std::array<double,12> matrix;
    std::array<double,3> cylinderPosition{};
  };
  std::vector<Entrance> entrances;
  std::vector<Exit> exits;
  // Own the installed coordinate lattice: WorldCoordinateMesh itself is a view.
  std::int32_t exitCoordinateHeight = 0;
  std::vector<lemmings::WorldGeometryPrefix::TripleBlock> exitCoordinateRows;
};
// Application-owned binding: serialized tagged IDs are shared with sim::Lever.
// Keeping this outside BackdropScene preserves the retained renderers' layout.
struct LeverPresentation {
  struct Binding {
    std::size_t root;
    int id, initialFace;
    bool reversed;
    HatchAnimation animation;
  };
  std::vector<Binding> bindings;
};
struct PressurePadPresentation {
  struct UvKey { double end, u, v, du, dv; };
  struct Corner { std::size_t node, mesh, record, corner; double u, v; };
  struct UvTrack { std::vector<UvKey> keys; std::vector<Corner> corners; };
  struct Binding {
    std::size_t root;
    int id, initialState;
    SceneNode original;
    int displayedState = 0;
    HatchAnimation animation;
    std::vector<UvTrack> uvTracks;
    double triggeredAt = -std::numeric_limits<double>::infinity();
  };
  std::vector<Binding> bindings;
};
struct CrabPresentation {
  struct Binding {
    std::size_t root, entityIndex;
    HatchAnimation rest, attack;
  };
  std::vector<Binding> bindings;
};
struct NamedHazardPresentation {
  struct Spin { std::size_t node; unsigned axis; double turnsPerSecond; };
  struct Clip { HatchAnimation transforms; std::vector<Spin> spins; };
  struct Binding {
    std::size_t root, entityIndex;
    Clip rest, attack;
    std::vector<std::array<double,12>> originalMatrices;
  };
  std::vector<Binding> bindings;
  std::int32_t coordinateHeight = 0;
  std::vector<lemmings::WorldGeometryPrefix::TripleBlock> coordinateRows;
};
// App-owned UV tracks for the placed type-3 gravity gadgets. No scene layout
// change: historical render demonstrations keep their own compatible headers.
struct GravityPresentation {
  struct Key { double end, u, v, du, dv; };
  struct Corner {
    std::size_t node, mesh, record, corner;
    double u, v;
  };
  struct Track { std::vector<Key> keys; std::vector<Corner> corners; };
  struct Binding { std::size_t root; double duration; std::vector<Track> tracks; };
  std::vector<Binding> bindings;
};
struct BackdropStats {
  std::size_t roots=0, meshRecords=0, stageMeshRecords=0, meshTriangles=0,
      nullMaterials=0, nullMaterialTriangles=0, billboards=0, stageBillboards=0,
      predicateRejected=0, missingTextures=0, rejected=0, pixels=0;
  std::size_t rasterCalls=0;
  std::size_t projectionRejected=0, rasterRejected=0;
  // Actual accepted stores within the supplied observer mask (zero without an
  // observer). These count events, including repeated stores to one pixel.
  std::size_t acceptedColorWrites=0, acceptedDepthWrites=0;
  std::vector<std::string> diagnostics;
  // Type-2 branch-1 (sprite) scenery, scene-branch-impl-001. These are also included in the
  // general rasterCalls / meshTriangles / accepted-store totals above; the sprite counters let a
  // caller separate them from mesh submissions.
  std::size_t stageSprites=0, sprites=0, spriteRasterCalls=0, spriteTriangles=0,
      spriteAcceptedColorWrites=0, spriteAcceptedDepthWrites=0;
  struct SpriteSubmission {
    std::size_t sourceOffset=0;
    std::uint32_t temporal=0;
    bool mirror=false;
    std::size_t rasterCalls=0, culled=0, rejected=0;
    std::string reason;  // empty when both triangles were rasterized
  };
  std::vector<SpriteSubmission> spriteSubmissions;
  // fluid-surface-impl-001: one entry per drawn base `Fluid` model (Stage::FluidGate), in source
  // order, labelled by the entities_42acac stream index. `acceptedColorWrites` counts actual
  // raster stores and is nonzero only when the caller supplied an observer mask.
  struct FluidSubmission {
    std::size_t entityIndex = 0;
    double height = 0.0;
    bool raw = false;  // true: stored (pre-first-update) mesh, false: animated geometry
    std::size_t records = 0, predicateRejected = 0, rasterCalls = 0, triangles = 0,
        acceptedColorWrites = 0, acceptedDepthWrites = 0;
  };
  std::vector<FluidSubmission> fluidSubmissions;
  std::size_t fluidPixelsChanged = 0;  // framebuffer pixels whose RGB changed during the fluid stage
};
class BackdropScene {
 public:
  // FluidGate (fluid-surface-impl-001): the post-tube base `Fluid` pass of the entities_42acac
  // receivers (frame order 0x441845: tube, +0x160/+0x164, base Fluid, +0x15C, secondary). It draws
  // only while the caller supplies live gate state (setFluidState); without it nothing is drawn.
  enum class Stage { Primary, LegacyUnattributed, Secondary, FluidGate, FluidPipe };
  // PROVISIONAL clone depth policy for the alpha-blended fluid draw (user-approved milestone
  // exception). The inherited original ZENABLE/ZWRITEENABLE/ZFUNC/device ZBIAS are UNRESOLVED
  // (fluid research section 7.1); LessWriteOn is the clone's existing mesh default, not original
  // evidence. The alternatives exist for focused matched-time diagnostics only.
  enum class FluidDepthPolicy { LessWriteOn, LessWriteOff, DepthDisabled };
  static constexpr FluidDepthPolicy kProvisionalFluidDepthPolicy = FluidDepthPolicy::LessWriteOn;
  // Live per-receiver render state, indexed by entities_42acac stream index (from sim FluidGate).
  struct FluidGateState {
    std::uint32_t lane = 0;   // u32 [+0x16C]
    double height = 0.0;      // [+0x1A8]
    double wavePhase = 0.0;   // [+0x148], wrapped at this receiver kind's period
    bool animated = false;    // first update has run; false => stored deserialized mesh
    double alphaPhase = 0.0;  // [+0x150], kind 2 only
  };
  // Serialized per-receiver parameters of the animated base model (Entity42acac).
  struct FluidRecord {
    std::size_t entityIndex = 0;
    std::array<std::array<double, 3>, 4> posOff{}, amp{};  // +0x1B8.. / +0x218.. per slot k
    std::array<double, 3> baseColour{};                     // vec3 [+0x170]; c = v + ((255,255,255) - v) * col
    double projectionBias = 0.0;                            // model+0xF8 (third stored double)
    std::uint32_t kind = 0;  // [+0x128], table 0x4BF6E8 + kind*0x48
  };
  // Applies one animation update to a copy of the parsed base model (verified geometry node):
  // vertices 2..5, the six block[1] top colours and the normal sign (0x0042B24D, 0x0042C5C9).
  // Public for value checks; production reaches it only through Stage::FluidGate.
  static void applyFluidState(SceneNode& geometry, const FluidRecord& record, const FluidGateState& state);
  struct RootSource {
    Stage stage;
    std::string owner; // paired primary/secondary or typed entity collection
    std::size_t entityIndex;
    lemmings::ObjectSourceExtent extent;
  };
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
  void visitRoots(const std::function<void(const RootSource&, const SceneNode&)>& visitor) const;
  // P9-P11 presentation only. The caller derives progress from the existing
  // release arm clock and departure time from the existing pending end timer.
  // Absolute samples make pause, retry and repeated draws deterministic.
  // Build presentation data once from the same bytes passed to build(); it
  // belongs to this scene and is discarded when the application reloads it.
  std::expected<EntranceExitPresentation,std::string> entranceExitPresentation(std::span<const std::byte> level) const;
  void setEntranceExitPresentation(const EntranceExitPresentation&, double openingProgress, double departureSeconds);
  std::expected<LeverPresentation,std::string> leverPresentation(std::span<const std::byte> level) const;
  void setLeverFaces(const LeverPresentation&, std::span<const std::pair<int,int>> idFaces);
  std::expected<PressurePadPresentation,std::string> pressurePadPresentation(std::span<const std::byte> level) const;
  void setPressurePadStates(PressurePadPresentation&, std::span<const std::pair<int,int>> idStates);
  std::expected<CrabPresentation,std::string> crabPresentation(std::span<const std::byte> level) const;
  void setCrabStates(const CrabPresentation&, std::span<const sim::Crab>);
  std::expected<NamedHazardPresentation,std::string> namedHazardPresentation(std::span<const std::byte> level) const;
  void setNamedHazardStates(const NamedHazardPresentation&, std::span<const sim::NamedHazard>);
  std::expected<GravityPresentation,std::string> gravityPresentation(std::span<const std::byte> level) const;
  void animateGravity(const GravityPresentation&);
  std::vector<HatchHoverTarget> hatchHoverTargets(const CameraBasis&, const Raster&) const;
  // Runtime override of the per-node visibility byte +0x110 for Secondary nodes (the original toggles
  // it only for "cutme" type-1 nodes, 0x45DA0D). A hidden node and its subtree are not dispatched.
  // Production never installs one; it is the seam used by mutation checks.
  void setSecondaryHidden(std::function<bool(const SceneNode&)> hidden) { secondaryHidden_ = std::move(hidden); }
  const std::vector<std::string>& diagnostics() const { return diagnostics_; }
  bool empty() const { return roots_.empty(); }

  // ---- fluid surface (fluid-surface-impl-001) ----
  // Replaces the live receiver snapshot used by Stage::FluidGate (entities_42acac stream order).
  void setFluidState(std::vector<FluidGateState> gates) { fluidState_ = std::move(gates); hasFluidState_ = true; }
  void clearFluidState() { fluidState_.clear(); hasFluidState_ = false; }
  void setFluidDepthPolicy(FluidDepthPolicy policy) { fluidPolicy_ = policy; }
  FluidDepthPolicy fluidDepthPolicy() const { return fluidPolicy_; }
  // Admitted animated receivers (stream indices), for tests and run reports.
  std::vector<std::size_t> fluidEntityIndices() const;

  // ---- branch-1 sprite scenery state (scene-branch-impl-001) ----
  // The generator used for first-draw frames (0x004040A0). The application owns one per
  // process and seeds it from its own clock; without one a scene-local generator with the
  // CRT default state (1) is used, which exists for unit tests only. The pointee must
  // outlive the scene's draws.
  void setRandom(CrtRand* random) { random_ = random; }
  // One scenery update: dt = min(frameMs,100)/1000 seconds, zero outside the in-level state or
  // while paused (0x0045CFF9 -> 0x004423D2); then the scenery clock (0x00437BDE/0x004764C4,
  // no scene-duration wrap is parsed) and the 0x00468833 advance of every sprite node.
  void advanceScenery(double frameMs, bool inLevel, bool paused);
  std::size_t spriteCount() const { return sprites_.size(); }
  // Current temporal state of the sprite node at `sourceOffset` (nullptr if none).
  const SpriteTemporalState* spriteState(std::size_t sourceOffset) const;
  // Generator state immediately before the first sprite frame was drawn, for run diagnostics.
  std::optional<std::uint32_t> firstSpriteDrawRandomState() const { return firstDrawState_; }
  // Process-wide counters so a bounded public-route run can show that the temporal advance
  // and first-draw latch actually executed (diagnostic only; no behaviour depends on them).
  struct SceneryTelemetry {
    std::size_t advanceCalls=0, advancesWithTime=0, temporalSteps=0, firstDrawLatches=0;
  };
  static SceneryTelemetry& telemetry();
 private:
  struct OwnedRoot { RootSource source; SceneNode node; double projectionBias = 0.0; };
  struct SpriteNode {
    std::string sheetName;  // registry name -> SPRITES\<name>.SPR
    std::size_t clip = 0;
    bool mirror = false;
    mutable SpriteTemporalState state;
  };
  std::vector<OwnedRoot> roots_;
  // Base `Fluid` models of the entities_42acac receivers. Kept apart from roots_ so existing
  // root/record accounting and the fail-closed extraction of the other roles are unchanged.
  struct FluidRoot { RootSource source; SceneNode node; FluidRecord record; };
  std::vector<FluidRoot> fluidRoots_;
  std::vector<FluidGateState> fluidState_;
  bool hasFluidState_ = false;
  FluidDepthPolicy fluidPolicy_ = kProvisionalFluidDepthPolicy;
  void renderFluid(Raster&, const CameraBasis&, BackdropStats&, SceneRasterObserver* observer) const;
  std::string levelId_;
  std::vector<std::string> diagnostics_;
  std::map<std::string, SpriteSheet> sheets_;
  std::map<std::size_t, SpriteNode> sprites_;  // by node source offset
  CrtRand* random_ = nullptr;
  mutable CrtRand fallbackRandom_;
  mutable std::optional<std::uint32_t> firstDrawState_;
  std::function<bool(const SceneNode&)> secondaryHidden_;
  SecondaryDispatchOptions dispatchOptions() const;
  double sceneryClock_ = 0.0, sceneryPrevious_ = 0.0;
  struct PendingSprite { const SceneNode* node; BillboardSourceInputs inputs; };
  using SpriteVisitor = std::function<void(const RootSource&, const SceneNode&, const BillboardSourceInputs&)>;
  void collectBillboards(const BillboardSourceInputs&,
      const std::function<void(const RootSource&, const SceneBillboardSource&)>&,
      const std::function<void(const std::string&)>& error,
      const SpriteVisitor& sprite = {}) const;
};
}
}
