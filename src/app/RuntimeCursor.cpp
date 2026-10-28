#include "app/RuntimeCursor.h"
#include "app/SkillPanel.h"

namespace webo::app {

CursorState runtimeCursorState(bool focused, bool grabbed, bool actorHit,
                              bool paused, int panelIndex, int panelAvailability) {
  if (!focused) return CursorState::Hidden;
  // SOURCE ordinary gameplay adapter to the recovered ordered policy. Modes
  // here are explicit, not a claim that SDL reconstructs every native mode or
  // input producer. The adapter has no directional filters or object hit test.
  return selectCursorState({CursorState::NoLemming, 0, 2, paused ? 1 : 0,
      true, grabbed, {actorHit, false, false, false}, 0, 0,
      panelIndex, panelAvailability});
}

std::expected<RuntimeCursor, std::string> RuntimeCursor::load(const AssetLibrary& library) {
  RuntimeCursor cursor;
  constexpr CursorState states[]{CursorState::NoLemming, CursorState::Lemming, CursorState::Grab};
  for (std::size_t i = 0; i < cursor.images_.size(); ++i) {
    auto image = loadTexture(library, "TEXTURES\\" + std::string(softwareCursorResource(states[i])) + ".BMP");
    if (!image) return std::unexpected(image.error());
    if (image->width != 32 || image->height != 32)
      return std::unexpected("unexpected runtime cursor bitmap dimensions");
    cursor.images_[i] = std::move(*image);
  }
  return cursor;
}

void RuntimeCursor::draw(Image& target, CursorState state, int x, int y) const {
  const Image* image = nullptr;
  switch (state) {
    case CursorState::NoLemming: image = &images_[0]; break;
    case CursorState::Lemming: image = &images_[1]; break;
    case CursorState::Grab: image = &images_[2]; break;
    default: return; // Unsupported producers are not silently given artwork.
  }
  // READ 455774/4577CD: full 32x32 quad centered on integer sprite position.
  // NoLem/Lem use black-key binary alpha; Grab's mode1 selects SRCALPHA /
  // INVSRCALPHA (414D80). Existing compositor supplies that SOURCE integer
  // blend. Hardware sampling/bias/quantization remains unverified.
  compositeIconRegion(target, *image, {x - 16, y - 16, 0, 0, 32, 32});
}

} // namespace webo::app
