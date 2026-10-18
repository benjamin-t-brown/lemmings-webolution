#include "LemmingSprites.h"

#include "Renderer.h"
#include "assets/Bitmap.h"
#include "assets/Box.h"

#include <format>

namespace webo::gfx {
const LemmingSprites::Sheet* LemmingSprites::sheetFor(const std::string& name) const {
  const auto it = sheets_.find(name);
  return it == sheets_.end() ? nullptr : &it->second;
}

std::expected<void, std::string> LemmingSprites::load(const AssetLibrary& lib, Renderer& renderer,
                                                      std::string_view sprFile) {
  auto parsed = loadSpriteSheet(lib, sprFile);
  if (!parsed)
    return std::unexpected(parsed.error());
  sheet_ = std::move(*parsed);

  // Collect every sheet the frames reference, then load each one once.
  for (const auto& sprite : sheet_.sprites)
    for (const auto& anim : sprite.animations)
      for (const auto& frame : anim.frames) {
        if (frame.sheet.empty() || sheets_.contains(frame.sheet))
          continue;
        auto image = loadTexture(lib, sheetAssetName(frame.sheet));
        if (!image) {
          missing_.push_back(frame.sheet);
          sheets_.emplace(frame.sheet, Sheet{});
          continue;
        }
        Sheet s;
        s.texture = renderer.uploadImage(*image);
        s.width = image->width;
        s.height = image->height;

        // The .SPR states the sheet size independently, via the normalised
        // rectangle. If the two disagree the frame rectangles are being read
        // against the wrong axes, and the lemmings come out sliced. Prefer
        // the image, but say so.
        const int declaredW = static_cast<int>(frame.sheetWidth());
        const int declaredH = static_cast<int>(frame.sheetHeight());
        if (declaredW && declaredH && (declaredW != s.width || declaredH != s.height))
          missing_.push_back(std::format("{} size disagrees: image {}x{}, .SPR {}x{}", frame.sheet,
                                         s.width, s.height, declaredW, declaredH));
        sheets_.emplace(frame.sheet, s);
      }

  ready_ = true;
  return {};
}

bool LemmingSprites::draw(Renderer& renderer, sim::State state, float x, float y, float height,
                          int tick, bool faceLeft) const {
  if (!ready_)
    return false;
  const std::string_view name = spriteNameForState(state);
  if (name.empty())
    return false;
  const Sprite* sprite = sheet_.find(name);
  if (!sprite || sprite->animations.empty())
    return false;

  // View 0: the flat view needs only one of the rotational views.
  const auto& anim = sprite->animations.front();
  if (anim.frames.empty())
    return false;
  const auto& frame = anim.frames[static_cast<std::size_t>(tick) % anim.frames.size()];

  const Sheet* sheet = sheetFor(frame.sheet);
  if (!sheet || !sheet->texture || frame.width == 0 || frame.height == 0)
    return false;

  // Scale the artwork to the simulation's lemming height and centre it
  // horizontally on the lemming's position, which stands on its feet.
  const float scale = height / static_cast<float>(frame.height);
  const float w = static_cast<float>(frame.width) * scale;
  const float h = height;

  renderer.drawSprite(sheet->texture, x - w * 0.5f, y - h, w, h, sheet->width, sheet->height,
                      static_cast<int>(frame.left), static_cast<int>(frame.top),
                      static_cast<int>(frame.width), static_cast<int>(frame.height), faceLeft,
                      Colour{1, 1, 1, 1});
  return true;
}

}  // namespace webo::gfx
