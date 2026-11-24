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
  shell::drawHudGlyphs(frame, font, prepareBomberCountdowns(world, actors, frame.width), sceneDepth);
}

std::vector<shell::HudGlyph> prepareBomberCountdowns(const sim::World& world,
    std::span<const LevelActorView> actors, int viewportWidth) {
  std::vector<shell::HudGlyph> draws;
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
    const double z = actorPhysicalDepth(actor.placement, viewportWidth);
    shell::appendHudGlyphs(draws, std::span(&line, 1), cells, z);
  }
  return draws;
}

}  // namespace webo::app
