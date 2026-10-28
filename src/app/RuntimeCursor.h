#pragma once

#include "app/CursorPolicy.h"
#include "assets/Bitmap.h"

#include <array>

namespace webo::app {

// Bounded gameplay adapter: actor/no-actor, skill plates, and right-drag only.
// No inferred entrance/exit markers, facing filters or gameplay MouseDown lifecycle.
CursorState runtimeCursorState(bool focused, bool grabbed, bool actorHit,
                               bool paused, int panelIndex, int panelAvailability);

// READ453BF6..453D98: frontend flag selects1 normally or10 for either held
// button, then skips gameplay hit/Grab policy. Focus hiding and SDL button
// mapping are SOURCE adaptations, not evidence of native physical bindings.
CursorState frontendCursorState(bool focused, bool eitherButtonHeld);

class RuntimeCursor {
 public:
  static std::expected<RuntimeCursor, std::string> load(const AssetLibrary& library);
  void draw(Image& target, CursorState state, int x, int y) const;

 private:
  // Whole 32x32 bitmap resources, not temporal frames.
  std::array<Image, 4> images_;
};

} // namespace webo::app
