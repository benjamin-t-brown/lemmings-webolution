#pragma once

#include "app/FrontendSession.h"

#include <expected>
#include <functional>
#include <optional>
#include <string>

union SDL_Event;

namespace webo::app {

class SdlWindow;

struct FrontendWindowOptions {
  int maxFrames = 0;
  // Capture only on reaching maxFrames. Selecting a level hands capture
  // ownership to the caller; closing early with a requested capture is an error.
  std::string finalFramePath;
};

using FrontendWindowObserver = std::function<void(const FrontendSession&, int)>;

// Observe focus and button transitions before dispatching actions on either menu path.
void updateFrontendCursorInput(FrontendSession& session, const SDL_Event& event);

// Shared SDL input adapter while either caller owns sound page 9.
void handleSoundOptionsInput(FrontendSession& session, const SDL_Event& event,
                             const SdlWindow& presentation);

// Return immediately on level selection so the caller can present the level.
std::expected<std::optional<std::string>, std::string> runFrontendWindow(
    FrontendSession& session, const FrontendWindowOptions& options,
    FrontendWindowObserver observer = {}, SdlWindow* sharedWindow = nullptr);

}  // namespace webo::app
