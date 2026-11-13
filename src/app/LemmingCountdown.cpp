#include "app/LemmingAnim.h"

#include "app/AppEntry.h"
#include "shell/HudText.h"
#include "shell/MenuRender.h"

#include <string>

namespace webo::app {

void drawBomberCountdowns(Image& frame, const Image& font, const sim::World& world,
                          std::span<const LevelActorView> actors) {
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
    shell::drawHud(frame, font, std::span<const shell::HudLine>(&line, 1), cells);
  }
}

}  // namespace webo::app
