#pragma once

#include "app/FrontendSession.h"

#include <expected>
#include <functional>
#include <optional>
#include <string>

namespace webo::app {

struct FrontendWindowOptions {
  int maxFrames = 0;
  // Capture only on reaching maxFrames. Selecting a level hands capture
  // ownership to the caller; closing early with a requested capture is an error.
  std::string finalFramePath;
};

using FrontendWindowObserver = std::function<void(const FrontendSession&, int)>;

// SDL is a SOURCE input adapter only; it does not establish native physical-input binding.
// A selected level is returned immediately and is left for the caller to render/capture.
std::expected<std::optional<std::string>, std::string> runFrontendWindow(
    FrontendSession& session, const FrontendWindowOptions& options,
    FrontendWindowObserver observer = {});

}  // namespace webo::app
