#pragma once

#include "app/FrontendWindow.h"
#include "app/ActorSprite.h"
#include "app/CursorPolicy.h"
#include "app/LevelEndTransition.h"
#include "app/EntryStartCue.h"

#include <span>
#include <optional>

namespace webo::sim { class World; }

namespace webo::app {

struct LevelActorView {
  std::size_t index;
  ActorSpritePlacement placement;
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
};
using LevelFrameObserver = std::function<void(const LevelFrameView&)>;

struct ApplicationRunOptions {
  // Owned-process test clock only: positive values replace elapsed wall time
  // for bounded (--frames) level runs. Zero preserves the production clock.
  // This supplies time, not World mutation or a fabricated completion event.
  double frameSeconds = 0.0;
};

// The CLI and the owned-process integration test use the same application path.
// The optional observer is the existing frontend frame hook (empty in normal
// runs); tests use it to queue SDL events in their own dummy-driver process.
// It is not a hook into the original executable or a native fidelity oracle.
int runApplication(int argc, char** argv, FrontendWindowObserver observer = {},
                   LevelFrameObserver levelObserver = {}, ApplicationRunOptions options = {});

}  // namespace webo::app
