#include "app/SkillPanel.h"

#include <algorithm>

namespace webo::app {

std::size_t compositeIconRegion(Image& dest, const Image& source, const SkillIconPlacement& placement) {
  if (dest.empty() || source.empty() || placement.width <= 0 || placement.height <= 0)
    return 0;

  // Clip the source rectangle to the source image, and the destination
  // placement to the destination image, keeping both offsets in step so a
  // partially off-screen or partially off-sheet placement still copies the
  // overlapping region correctly rather than reading/writing out of bounds.
  int srcX0 = placement.srcX;
  int srcY0 = placement.srcY;
  int destX0 = placement.destX;
  int destY0 = placement.destY;
  int width = placement.width;
  int height = placement.height;

  if (srcX0 < 0) {
    width += srcX0;
    destX0 -= srcX0;
    srcX0 = 0;
  }
  if (srcY0 < 0) {
    height += srcY0;
    destY0 -= srcY0;
    srcY0 = 0;
  }
  if (destX0 < 0) {
    width += destX0;
    srcX0 -= destX0;
    destX0 = 0;
  }
  if (destY0 < 0) {
    height += destY0;
    srcY0 -= destY0;
    destY0 = 0;
  }
  width = std::min({width, source.width - srcX0, dest.width - destX0});
  height = std::min({height, source.height - srcY0, dest.height - destY0});
  if (width <= 0 || height <= 0)
    return 0;

  std::size_t written = 0;
  for (int row = 0; row < height; ++row) {
    const std::size_t srcRowBase = (static_cast<std::size_t>(srcY0 + row) * source.width + srcX0) * 4;
    const std::size_t destRowBase = (static_cast<std::size_t>(destY0 + row) * dest.width + destX0) * 4;
    for (int col = 0; col < width; ++col) {
      const std::uint8_t* s = source.rgba.data() + srcRowBase + static_cast<std::size_t>(col) * 4;
      std::uint8_t* d = dest.rgba.data() + destRowBase + static_cast<std::size_t>(col) * 4;
      const std::uint8_t a = s[3];
      if (a == 0) {
        // Fully transparent: dest keeps whatever it already had.
      } else if (a == 255) {
        d[0] = s[0];
        d[1] = s[1];
        d[2] = s[2];
        d[3] = 255;
      } else {
        // Linear blend, straight (non-premultiplied) alpha: the convention
        // decodeBmp/applyAlfAlpha already use (colour channels are the
        // decoded BMP's own values regardless of alpha).
        for (int c = 0; c < 3; ++c) {
          const int blended = (static_cast<int>(s[c]) * a + static_cast<int>(d[c]) * (255 - a)) / 255;
          d[c] = static_cast<std::uint8_t>(blended);
        }
        d[3] = static_cast<std::uint8_t>(std::max<int>(d[3], a));
      }
      ++written;
    }
  }
  return written;
}

}  // namespace webo::app
