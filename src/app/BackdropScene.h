#pragma once

#include "app/Camera.h"
#include "app/CrtRand.h"
#include "app/SceneBillboardSource.h"
#include "assets/Sprite.h"
#include "app/SceneRaster.h"
#include "app/SceneDispatch.h"
#include "app/HatchAnimation.h"
#include "app/HoverFeedback.h"
#include "app/Pick.h"
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
namespace sim { struct Crab; struct NamedHazard; struct LevelSetup; struct OccupancySensor; }
namespace app {
class Raster;
// Application-owned animation state for placed scenery.
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
// Serialized tagged IDs are shared with sim::Lever.
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
// App-owned UV tracks for placed type-3 gravity gadgets.
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
// Gadget clips use the level clock. Contacts notify presentation, which never feeds simulation.
struct GadgetPresentation {
  struct Spin { std::size_t node; unsigned axis; double turnsPerSecond; };
  struct Clip {
    HatchAnimation transforms;
    std::vector<PressurePadPresentation::UvTrack> uv;
    std::vector<Spin> spins;
  };
  struct Binding {
    std::size_t root, simIndex;
    bool teleport;
    int fxSide = -1; // -1 = base, 0/1 = the two authored FX owners
    SceneNode original;
    Clip clip;
    std::array<std::string,2> samples;
    double triggeredAt = -std::numeric_limits<double>::infinity();
    double activeUntil = -std::numeric_limits<double>::infinity();
  };
  std::vector<Binding> bindings;
};
struct BackdropStats {
  std::size_t roots=0, meshRecords=0, stageMeshRecords=0, meshTriangles=0,
      nullMaterials=0, nullMaterialTriangles=0, billboards=0, stageBillboards=0,
      predicateRejected=0, missingTextures=0, rejected=0, pixels=0;
  std::size_t rasterCalls=0;
  std::size_t projectionRejected=0, rasterRejected=0;
  // Accepted raster stores inside the observer mask, including repeated stores.
  std::size_t acceptedColorWrites=0, acceptedDepthWrites=0;
  std::vector<std::string> diagnostics;
  // Type-2 branch-1 sprite counts are also included in the general raster totals.
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
  // One base Fluid entry per submission, in entities_42acac stream order.
  // acceptedColorWrites counts raster stores within the observer mask.
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
  // FluidGate draws base Fluid only with live entities_42acac receiver state.
  // Frame order at 0x441845 is tube, +0x160/+0x164, base Fluid, +0x15C, secondary.
  enum class Stage { Primary, LegacyUnattributed, Secondary, FluidGate, FluidPipe, GadgetEffect };
  // Provisional clone fluid depth defaults to LessWriteOn. Original ZENABLE,
  // ZWRITEENABLE, ZFUNC and device ZBIAS are unresolved.
  enum class FluidDepthPolicy { LessWriteOn, LessWriteOff, DepthDisabled };
  static constexpr FluidDepthPolicy kProvisionalFluidDepthPolicy = FluidDepthPolicy::LessWriteOn;
  // Live per-receiver render state, indexed by entities_42acac stream index (from sim FluidGate).
  struct FluidGateState {
    std::uint32_t lane = 0;   // u32 [+0x16C]
    double height = 0.0;      // [+0x1A8]
    double wavePhase = 0.0;   // [+0x148], wrapped at this receiver kind's period
    bool animated = false;    // True once an update has animated the deserialized mesh.
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
  // Apply 0x0042B24D and 0x0042C5C9 to a copy of the base model, updating
  // vertices 2..5, the six block[1] top colours and the normal sign.
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
                  SceneRasterObserver* observer = nullptr,
                  std::vector<double>* depthOutput = nullptr) const;
  // Dispatches the parsed trees with the supplied inputs.
  void visitBillboards(const BillboardSourceInputs& cameraChain,
                       const std::function<void(const SceneBillboardSource&)>& visitor) const;
  // Reports the kind-5 predicate result and source record offset to the visitor.
   void visitKind5Predicates(const CameraBasis& camera,
       const std::function<void(std::size_t, bool)>& visitor) const;
  void visitSecondaryDispatch(const CameraBasis& camera,
       const std::function<void(const SecondaryDispatchItem&)>& visitor) const;
  void visitRoots(const std::function<void(const RootSource&, const SceneNode&)>& visitor) const;
  // Door progress, departure and bob are sampled at absolute times so repeated
  // draws during pause agree. The caller supplies release, end and level clocks.
  std::expected<EntranceExitPresentation,std::string> entranceExitPresentation(std::span<const std::byte> level) const;
  void setEntranceExitPresentation(const EntranceExitPresentation&, double openingProgress,
                                  double departureSeconds, double levelSeconds = 0.0,
                                  double swayScale = 1.0);
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
  std::expected<GadgetPresentation,std::string> gadgetPresentation(std::span<const std::byte> level) const;
  // Returns only the selected serialized sample (departure/arrival).
  std::string triggerGadget(GadgetPresentation&, bool teleport, std::size_t simIndex,
                            bool arrival, int facing, double levelSeconds);
  void animateGadgets(const GadgetPresentation&, double levelSeconds);
  std::vector<HatchHoverTarget> hatchHoverTargets(const CameraBasis&, const Raster&) const;
  void drawSwitchHover(Image&, const CameraBasis&, const Raster&, const sim::LevelSetup&,
                       const LeverPresentation*, const PressurePadPresentation*,
                       int pointerX, int pointerY,
                       const std::function<void(const sim::OccupancySensor&)>& drawSensor) const;
  // Overrides Secondary visibility byte +0x110, which 0x45DA0D toggles on
  // "cutme" type-1 nodes. A hidden node suppresses its subtree.
  void setSecondaryHidden(std::function<bool(const SceneNode&)> hidden) { secondaryHidden_ = std::move(hidden); }
  const std::vector<std::string>& diagnostics() const { return diagnostics_; }
  bool empty() const { return roots_.empty(); }

  // Replaces the live receiver snapshot used by Stage::FluidGate (entities_42acac stream order).
  void setFluidState(std::vector<FluidGateState> gates) { fluidState_ = std::move(gates); hasFluidState_ = true; }
  void clearFluidState() { fluidState_.clear(); hasFluidState_ = false; }
  void setFluidDepthPolicy(FluidDepthPolicy policy) { fluidPolicy_ = policy; }
  FluidDepthPolicy fluidDepthPolicy() const { return fluidPolicy_; }
  // Stream indices of receivers with animated models.
  std::vector<std::size_t> fluidEntityIndices() const;

  // Generator for first-draw frames (0x004040A0). The supplied instance must
  // outlive the scene. Otherwise, use a scene-local generator seeded with 1.
  void setRandom(CrtRand* random) { random_ = random; }
  // Advance scenery by min(frameMs,100)/1000 seconds, or zero when paused or
  // outside play (0x0045CFF9 -> 0x004423D2). Advance sprite controllers
  // (0x00468833) and loop secondary clips at their authored duration (0x00437BDE).
  void advanceScenery(double frameMs, bool inLevel, bool paused);
  std::size_t spriteCount() const { return sprites_.size(); }
  // Current temporal state of the sprite node at `sourceOffset` (nullptr if none).
  const SpriteTemporalState* spriteState(std::size_t sourceOffset) const;
  // Generator state just before the sprite's first draw.
  std::optional<std::uint32_t> firstSpriteDrawRandomState() const { return firstDrawState_; }
  // Process-wide counts of temporal advances and first-draw latches.
  struct SceneryTelemetry {
    std::size_t advanceCalls=0, advancesWithTime=0, temporalSteps=0, firstDrawLatches=0;
  };
  static SceneryTelemetry& telemetry();
 private:
  struct OwnedRoot { RootSource source; SceneNode node; double projectionBias = 0.0; bool visible = true; };
  struct SpriteNode {
    std::string sheetName;  // registry name -> SPRITES\<name>.SPR
    std::size_t clip = 0;
    bool mirror = false;
    mutable SpriteTemporalState state;
  };
  std::vector<OwnedRoot> roots_;
  struct WaterUvTrack {
    std::vector<GravityPresentation::Key> keys;
    std::size_t materialSlot;
    std::vector<std::size_t> nodes;  // Preorder indices in the secondary tree.
  };
  struct SecondaryAnimation {
    struct Spin { std::size_t node; std::uint32_t axis; double turnsPerSecond; };
    std::size_t root;
    double duration;
    double time = 0.0;
    std::vector<WaterUvTrack> waterTracks;
    std::vector<Spin> spins;
  };
  std::optional<SecondaryAnimation> secondaryAnimation_;
  void bindSecondaryAnimation(const lemmings::ObjectBasePrefix& objects);
  void advanceSecondaryAnimation(double dt);
  // Base Fluid models of entities_42acac receivers, kept outside the other root collections.
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
      const SpriteVisitor& sprite = {}, std::optional<Stage> stage = std::nullopt) const;
};
}
}
