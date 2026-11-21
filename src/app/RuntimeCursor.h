#pragma once

#include "app/CursorPolicy.h"
#include "assets/Bitmap.h"

#include <array>

namespace webo::app {

// Gameplay adapter for actor and hatch hits, skill plates, and right-drag.
CursorState runtimeCursorState(bool focused, bool grabbed, bool actorHit,
                               bool paused, int panelIndex, int panelAvailability,
                               bool hatchHit = false);

// READ453BF6..453D98: frontend flag selects1 normally or10 for either held
// button, then skips gameplay hit/Grab policy. Focus hiding and SDL button
// mapping are SOURCE adaptations, not evidence of native physical bindings.
CursorState frontendCursorState(bool focused, bool eitherButtonHeld);

class RuntimeCursor {
 public:
  static std::expected<RuntimeCursor, std::string> load(const AssetLibrary& library);
  void draw(Image& target, CursorState state, int x, int y) const;

 private:
  // Whole cursor images at the 32x32 draw extent.
  std::array<Image, 5> images_;
};

} // namespace webo::app
