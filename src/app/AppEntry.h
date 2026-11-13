#pragma once

#include "app/FrontendWindow.h"
#include "audio/CuePlayback.h"
#include "app/ActorSprite.h"
#include "app/CursorPolicy.h"
#include "app/LevelEndTransition.h"
#include "app/EntryStartCue.h"
#include "app/Camera.h"
#include "save/SaveStore.h"

#include <cstddef>
#include <memory>
#include <span>
#include <optional>

namespace webo::sim { class World; }

namespace webo::app {

struct LevelActorView {
  std::size_t index;
  ActorSpritePlacement placement;
  // actor-anim-001: the SPR clip and temporal entry this actor was drawn with this frame.
  const char* clip = "walk";
  int temporalIndex = 0;
  // P7: actual fractional terrain sample sent to both drawing and picking.
  std::array<double, 3> drawnSample{};
};

// Read-only, callback-lifetime view after composing a simulated level frame.
// Rectangles are our projected actors, not native hover/visibility evidence.
// Used to drive owned-process SDL integration tests without mutating the World.
struct LevelFrameView {
  int frame;
  const sim::World& world;
  bool paused;
  int selectedSkillIndex;
  std::span<const LevelActorView> actors;
  const Image& image;
  // SOURCE pointer/selection snapshot used to compose this image. Not a native
  // physical-input observation or a complete native applicability predicate.
  std::optional<std::size_t> hoveredActor;
  int cursorX = 320, cursorY = 240;
  CursorState cursorState = CursorState::Hidden;
  LevelEndPhase endPhase = LevelEndPhase::Running;
  double endTimerSeconds = 7.0;
  bool cameraIntroActive = false;
  double entryCueSeconds = 0.0;
  std::optional<EntryCueFrame> entryCueFrame;
  bool levelMenuOpen = false;
  std::string_view levelName;
  // Present only on the connected frontend/game route. Standalone level runs
  // do not own the frontend construction allocator.
  std::optional<int> frontendConstructionCounter;
  // Audio adapter state at this frame. Cue routing only, no timing claim.
  bool audioDeviceReady = false;
  std::size_t audioCuesDrained = 0;  // every sim cue taken, routed or not
  std::size_t audioCuesRouted = 0;   // routed cues requested
  // Read-only source pose used for this image, before the intro clock advances,
  // and the controller's target/pick seed. No native runtime observation claim.
  CameraParams displayedPose;
  std::array<double, 3> cameraPickReference{};
  // terrain-render-001: running totals of the grid cells whose solidity the drawn tube has been
  // brought in line with (TubeScene::syncTerrain) and of the 8x16 mesh blocks regenerated for
  // them. Read-only counters; zero on a level whose terrain was never edited.
  std::size_t drawnTerrainCellsChanged = 0;
  std::size_t drawnTerrainBlocksRebuilt = 0;
};
using LevelFrameObserver = std::function<void(const LevelFrameView&)>;

struct ApplicationRunOptions {
  // Owned-process test clock only: positive values replace elapsed wall time
  // for bounded (--frames) level runs. Zero preserves the production clock.
  // This supplies time, not World mutation or a fabricated completion event.
  double frameSeconds = 0.0;
  // Save persistence for the connected frontend. Null (the default, which every
  // test that does not opt in uses) means nothing is read or written. The CLI
  // entry point main() installs the clone's per-user store; tests inject a store
  // over a temporary directory. The installed save is never involved either way.
  std::shared_ptr<save::SaveStore> saveStore = nullptr;
  // Set by main(): resolve the default clone store from the game directory when
  // saveStore is empty. Tests leave it false.
  bool useDefaultSaveStore = false;
  // Gameplay sound cues. Off by default so every test that does not opt in
  // needs no audio device. main() turns it on; --no-audio turns it back off.
  // A failed open, a missing WAV or no audio backend degrade to a stderr
  // diagnostic and a silent run.
  bool enableAudio = false;
  // Optional recording sink: receives the literal retail cue name of each routed
  // cue, whether or not a device exists (unless --no-audio). Tests only.
  audio::CueObserver cueObserver;
};

// The CLI and the owned-process integration test use the same application path.
// The optional observer is the existing frontend frame hook (empty in normal
// runs); tests use it to queue SDL events in their own dummy-driver process.
// It is not a hook into the original executable or a native fidelity oracle.
int runApplication(int argc, char** argv, FrontendWindowObserver observer = {},
                   LevelFrameObserver levelObserver = {}, ApplicationRunOptions options = {});

}  // namespace webo::app
