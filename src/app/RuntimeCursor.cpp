#include "app/RuntimeCursor.h"
#include "app/SkillPanel.h"

#include <algorithm>

namespace webo::app {
namespace {

constexpr int cursorExtent = 32;

Image scaleEntranceCursor(const Image& source) {
  // 0x45595E..0x45596F gives the entrance the same 32x32 quad as the
  // other cursors. The stock entrance bitmap is 28x28. Sample at pixel
  // centers to fill the quad while keeping the black-key edges crisp.
  Image image{cursorExtent, cursorExtent, source.alpha, {}};
  image.rgba.resize(image.pixelCount() * 4);
  for (int y = 0; y < cursorExtent; ++y) {
    const int sy = (2 * y + 1) * source.height / (2 * cursorExtent);
    for (int x = 0; x < cursorExtent; ++x) {
      const int sx = (2 * x + 1) * source.width / (2 * cursorExtent);
      const auto src = (sy * source.width + sx) * 4;
      const auto dst = (y * cursorExtent + x) * 4;
      std::copy_n(source.rgba.data() + src, 4, image.rgba.data() + dst);
    }
  }
  return image;
}

} // namespace

CursorState runtimeCursorState(bool focused, bool grabbed, bool actorHit,
                              bool paused, int panelIndex, int panelAvailability,
                              bool hatchHit) {
  if (!focused) return CursorState::Hidden;
  // The caller resolves actor precedence before supplying a hatch hit. The
  // native picker sets both the object and entrance markers for a hatch hit
  // (0x45524D/0x45525C), then selects the entrance cursor at 0x455378.
  return selectCursorState({CursorState::NoLemming, 0, 2, paused ? 1 : 0,
      true, grabbed, {actorHit, hatchHit, hatchHit, false}, 0, 0,
      panelIndex, panelAvailability});
}

CursorState frontendCursorState(bool focused, bool eitherButtonHeld) {
  if (!focused) return CursorState::Hidden;
  return eitherButtonHeld ? CursorState::MouseDown : CursorState::NoLemming;
}

std::expected<RuntimeCursor, std::string> RuntimeCursor::load(const AssetLibrary& library) {
  RuntimeCursor cursor;
  constexpr CursorState states[]{CursorState::NoLemming, CursorState::Lemming,
                                 CursorState::Grab, CursorState::MouseDown,
                                 CursorState::Entrance};
  for (std::size_t i = 0; i < cursor.images_.size(); ++i) {
    auto image = loadTexture(library, "TEXTURES\\" + std::string(softwareCursorResource(states[i])) + ".BMP");
    if (!image) return std::unexpected(image.error());
    if (states[i] == CursorState::Entrance && image->width == 28 && image->height == 28)
      *image = scaleEntranceCursor(*image);
    if (image->width != cursorExtent || image->height != cursorExtent)
      return std::unexpected("unexpected runtime cursor bitmap dimensions");
    cursor.images_[i] = std::move(*image);
  }
  return cursor;
}

std::optional<PreparedIconRegion> RuntimeCursor::prepare(CursorState state, int x, int y) const {
  const Image* image = nullptr;
  switch (state) {
    case CursorState::NoLemming: image = &images_[0]; break;
    case CursorState::Lemming: image = &images_[1]; break;
    case CursorState::Grab: image = &images_[2]; break;
    case CursorState::MouseDown: image = &images_[3]; break;
    case CursorState::Entrance: image = &images_[4]; break;
    default: return std::nullopt; // Unsupported producers have no artwork.
  }
  // READ 455774/4577CD: full 32x32 quad centered on integer sprite position.
  // NoLem/Lem/Entrance use black-key binary alpha. Grab's mode1 selects SRCALPHA /
  // INVSRCALPHA (414D80). Existing compositor supplies that SOURCE integer
  // blend. Hardware sampling/bias/quantization remains unverified.
  constexpr int halfExtent = cursorExtent / 2;
  return PreparedIconRegion{image, {x - halfExtent, y - halfExtent, 0, 0, cursorExtent, cursorExtent}};
}

void RuntimeCursor::draw(Image& target, CursorState state, int x, int y) const {
  if (const auto icon = prepare(state, x, y))
    compositeIconRegion(target, *icon->texture, icon->placement);
}

} // namespace webo::app
