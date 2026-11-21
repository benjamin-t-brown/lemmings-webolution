#pragma once

#include "app/FrontendWindow.h"
#include "audio/CuePlayback.h"
#include "app/ActorSprite.h"
#include "app/CursorPolicy.h"
#include "app/LevelEndTransition.h"
#include "app/EntryStartCue.h"
#include "app/Camera.h"
#include "app/HoverFeedback.h"
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
  // SPR clip and temporal entry drawn for this actor.
  const char* clip = "walk";
  int temporalIndex = 0;
  // Fractional terrain sample shared by drawing and picking.
  std::array<double, 3> drawnSample{};
};

// Read-only post-composition frame view, valid for the observer callback.
struct LevelFrameView {
  int frame;
  const sim::World& world;
  bool paused;
  int selectedSkillIndex;
  std::span<const LevelActorView> actors;
  const Image& image;
  // The frame's pointer and selection state.
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
  // The connected frontend owns the construction allocator.
  std::optional<int> frontendConstructionCounter;
  // Audio cue routing state for this frame.
  bool audioDeviceReady = false;
  std::size_t audioCuesDrained = 0;  // every sim cue taken, routed or not
  std::size_t audioCuesRouted = 0;   // routed cues requested
  // Rendered intro pose before clock advance, plus the target used for picking.
  CameraParams displayedPose;
  std::array<double, 3> cameraPickReference{};
  // Cumulative cells synchronized with live terrain and 8x16 mesh blocks rebuilt.
  std::size_t drawnTerrainCellsChanged = 0;
  std::size_t drawnTerrainBlocksRebuilt = 0;
  // Door pose submitted for this frame.
  double hatchOpeningProgress = 0.0;
  // Feedback composed for this frame, when present.
  std::optional<std::size_t> markedActor = std::nullopt;
  std::optional<std::size_t> hoveredHatch = std::nullopt;
  std::span<const HatchHoverTarget> hatches{};
};
using LevelFrameObserver = std::function<void(const LevelFrameView&)>;

struct ApplicationRunOptions {
  // Positive values supply elapsed wall time to bounded (--frames) invocations.
  // Zero uses the application clock.
  double frameSeconds = 0.0;
  // Optional save store for the connected frontend. Without a store, skip disk I/O.
  std::shared_ptr<save::SaveStore> saveStore = nullptr;
  // main() enables resolution of the clone's per-user store when saveStore is empty.
  bool useDefaultSaveStore = false;
  // main() enables gameplay audio unless --no-audio is set. Missing samples,
  // devices or backends report to stderr and leave playback silent.
  bool enableAudio = false;
  // Optional sink for routed retail cue names, independent of device availability.
  audio::CueObserver cueObserver;
};

// The optional observer can inspect each frontend frame and enqueue SDL events.
int runApplication(int argc, char** argv, FrontendWindowObserver observer = {},
                   LevelFrameObserver levelObserver = {}, ApplicationRunOptions options = {});

}  // namespace webo::app
