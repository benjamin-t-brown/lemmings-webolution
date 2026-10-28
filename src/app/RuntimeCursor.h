#pragma once

#include "app/CursorPolicy.h"
#include "assets/Bitmap.h"

#include <array>

namespace webo::app {

// Bounded gameplay adapter: actor/no-actor, skill plates, and right-drag only.
// No inferred entrance/exit markers, facing filters or MouseDown lifecycle.
CursorState runtimeCursorState(bool focused, bool grabbed, bool actorHit,
                              bool paused, int panelIndex, int panelAvailability);

class RuntimeCursor {
 public:
  static std::expected<RuntimeCursor, std::string> load(const AssetLibrary& library);
  void draw(Image& target, CursorState state, int x, int y) const;

 private:
  // Whole 32x32 bitmap resources, not temporal frames.
  std::array<Image, 3> images_;
};

} // namespace webo::app
