#include "app/LemmingAnim.h"

#include "app/AppEntry.h"
#include "shell/HudText.h"
#include "shell/MenuRender.h"

#include <algorithm>
#include <string>

namespace webo::app {

void drawBomberCountdowns(Image& frame, const Image& font, const sim::World& world,
                          std::span<const LevelActorView> actors) {
  drawBomberCountdowns(frame, font, world, actors, {});
}

void drawBomberCountdowns(Image& frame, const Image& font, const sim::World& world,
                          std::span<const LevelActorView> actors, std::span<const double> sceneDepth) {
  const auto& cells = shell::fontCells();
  const auto& lemmings = world.lemmings();
  for (const LevelActorView& actor : actors) {
    if (actor.index >= lemmings.size() || !lemmings[actor.index].alive())
      continue;
    const int digit = bomberCountdownDigit(lemmings[actor.index]);
    if (digit <= 0)
      continue;
    shell::HudLine line;
    line.text = std::to_string(digit);
    const int width = shell::textWidth(line.text, cells);
    line.x = static_cast<int>((actor.placement.left + actor.placement.right) / 2.0) - width / 2;
    line.y = static_cast<int>(actor.placement.top) - shell::kLineHeight;
    line.colour = {255, 230, 80};
    if (sceneDepth.size() != std::size_t(frame.width) * frame.height) {
      shell::drawHud(frame, font, std::span<const shell::HudLine>(&line, 1), cells);
      continue;
    }
    // Draw into a copy so HUD glyph blending stays unchanged; restore only
    // pixels behind the scene. The label shares its actor's billboard plane.
    const auto before = frame.rgba;
    shell::drawHud(frame, font, std::span<const shell::HudLine>(&line, 1), cells);
    const double z = actorPhysicalDepth(actor.placement, frame.width);
    for (int y = std::max(0, line.y); y < std::min(frame.height, line.y + shell::kLineHeight); ++y)
      for (int x = std::max(0, line.x); x < std::min(frame.width, line.x + width); ++x) {
        const auto at = std::size_t(y) * frame.width + x;
        if (!(z < sceneDepth[at]))
          std::copy_n(before.begin() + at * 4, 4, frame.rgba.begin() + at * 4);
      }
  }
}

}  // namespace webo::app
