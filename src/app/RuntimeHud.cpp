#include "app/RuntimeHud.h"
#include "app/RuntimeSkillPanel.h"

#include "shell/HudText.h"
#include "app/HoverFeedback.h"

#include <algorithm>
#include <cmath>

namespace webo::app {

int hatchRemaining(int total, int spawned, std::size_t count, std::size_t index) {
  if (!count || index >= count) return 0;
  // Count pending releases for this hatch using World's round-robin entrance order.
  const auto n = static_cast<long long>(count), i = static_cast<long long>(index);
  const auto quota = [=](int released) { return std::max(0LL, (released + n - 1 - i) / n); };
  return static_cast<int>(std::max(0LL, quota(std::max(0, total)) - quota(std::clamp(spawned, 0, std::max(0, total)))));
}

std::optional<std::size_t> hoveredHatch(std::span<const HatchHoverTarget> targets, int x, int y) {
  for (std::size_t i = 0; i < targets.size(); ++i) {
    const auto& h = targets[i];
    if (pointerOverSceneBounds(x, y, h.left, h.top, h.right, h.bottom)) return i;
  }
  return std::nullopt;
}

void drawHatchHover(Image& image, const Image& font, const HatchHoverTarget& h,
                    std::optional<int> adjustingRate) {
  const auto cells = shell::fontCells();
  // The clone displays remaining population. Native 0x454B95 passes release rate.
  shell::HudLine line{shell::counterText(h.remaining), 0, 0, {255,255,255}};
  const int width = shell::textWidth(line.text, cells);
  // 454C34..454C64 centres text on projected hatch Y - (44 << mode).
  // Provisional 640x480 display uses ^Font, mesh centre and viewport clamping.
  line.x = std::clamp((h.left+h.right-width)/2, 0, std::max(0, image.width-width));
  line.y = std::clamp((h.top+h.bottom)/2 - 44, 0, std::max(0, image.height-shell::kLineHeight));
  shell::drawHud(image, font, std::span<const shell::HudLine>(&line, 1), cells);
  // Keep the rate visible while the player holds a hatch button to adjust it.
  if (adjustingRate) {
    line.text = "RATE " + std::to_string(*adjustingRate);
    const int rateWidth = shell::textWidth(line.text, cells);
    line.x = std::clamp((h.left+h.right-rateWidth)/2, 0, std::max(0, image.width-rateWidth));
    // Put the second label below the count if the upper edge has no room.
    line.y = line.y >= shell::kLineHeight ? line.y - shell::kLineHeight :
        std::min(line.y + shell::kLineHeight, std::max(0, image.height-shell::kLineHeight));
    line.colour = {255,232,96};
    shell::drawHud(image, font, std::span<const shell::HudLine>(&line, 1), cells);
  }
}

void drawRuntimeNukeFeedback(Image& image, const Image& font, const sim::World& world, bool armed) {
  if (!armed && !world.nukeRequested()) return;
  // PROVISIONAL control feedback: gold outline and confirmation/status hint.
  // The large countdown is composed with the level's Let's Go text in main.
  for (int y = 372; y < std::min(404, image.height); ++y)
    for (int x = 18; x < std::min(90, image.width); ++x)
      if (x < 20 || x >= 88 || y < 374 || y >= 402) {
        const auto p = (std::size_t(y) * image.width + x) * 4;
        image.rgba[p] = 255; image.rgba[p+1] = 220; image.rgba[p+2] = 64;
      }
  const std::string text = !world.nukeRequested() ? "CLICK AGAIN" : "NUKE";
  const shell::HudLine line{text, 100, 380, {255,220,64}};
  shell::drawHud(image, font, std::span(&line, 1), shell::fontCells());
}

shell::HudValues runtimeHudValues(const sim::World& world) {
  shell::HudValues values;
  values.out = world.spawned() - world.saved() - world.dead();
  values.saved = world.saved();
  values.saveTarget = world.setup().saveTarget;
  values.secondsRemaining = static_cast<int>(
      world.timeRemainingTicks() / world.rules().ticksPerSecond);
  return values;
}

void drawRuntimeHud(webo::Image& frame, const webo::Image& font,
                    const sim::World& world, bool paused) {
  const auto values = runtimeHudValues(world);
  const auto cells = shell::fontCells();
  const auto lines = shell::layoutHud(values, {}, cells);
  shell::drawHud(frame, font, lines, cells);
  if (paused) shell::drawCaption(frame, font, cells);
}

void RuntimeSimState::togglePause() {
  paused_ = !paused_;
  clock_ = SimClock{};
  carryMs_ = 0.0;
}

void RuntimeSimState::resume() {
  if (paused_) {
    paused_ = false;
    clock_ = SimClock{};
    carryMs_ = 0.0;
  }
}

bool RuntimeSimState::advanceElapsed(sim::World& world, double elapsedSeconds, bool trackCompletion,
                                     const std::function<void()>& afterTick, int stallCapMs) {
  return advanceElapsed(world, elapsedSeconds, trackCompletion, afterTick, stallCapMs, true);
}

bool RuntimeSimState::advanceElapsed(sim::World& world, double elapsedSeconds, bool trackCompletion,
                                     const std::function<void()>& afterTick, int stallCapMs, bool releaseEnabled) {
  if (paused_) return false;
  if (trackCompletion && end_.phase() == LevelEndPhase::Results) return false;
  // Integer milliseconds with the sub-ms fraction carried (the epsilon absorbs the binary
  // representation of values such as 0.016 s).
  const double exactMs = std::max(0.0, elapsedSeconds) * 1000.0 + carryMs_;
  long long wholeMs = static_cast<long long>(std::floor(exactMs + 1e-6));
  carryMs_ = std::max(0.0, exactMs - static_cast<double>(wholeMs));
  if (stallCapMs > 0 && wholeMs > stallCapMs) {
    // Stall: only the capped part is simulated; the rest is dropped, not carried.
    discardedStallMs_ += wholeMs - stallCapMs;
    wholeMs = stallCapMs;
    carryMs_ = 0.0;
  }
  const int dtMs = static_cast<int>(std::min<long long>(wholeMs, 1LL << 30));
  lastPassMs_ = dtMs;

  // READ 0x45DB0E-0x45DD5C: every fast-forward pass reuses the same integer dt.
  const int passes = fastForward_ ? kFastForwardPasses : 1;
  bool updated = false;
  for (int pass = 0; pass < passes; ++pass) {
    if (pass > 0 && trackCompletion && end_.phase() == LevelEndPhase::Results) break;
    world.advanceElapsedMsWithReleaseGate(dtMs, trackCompletion, releaseEnabled, [&](double segmentSeconds) {
      updated = true;
      if (afterTick) afterTick();
      if (trackCompletion) {
        int retainedNonRat = 0;
        for (const auto& lemming : world.lemmings())
          if (lemming.alive() && (world.nukeRequested() || lemming.sheetType != 3)) ++retainedNonRat;
        // Pass the slice's own dt to the end transition. Provisional Nuke policy
        // delays the result until the replacement and animation sequence finishes,
        // including at timeout and with only rats.
        const bool nukeDraining = world.nukeRequested() && retainedNonRat > 0;
        end_.afterSimulationSlice(nukeDraining ? 0.0 : segmentSeconds,
            {world.timedOut() && !nukeDraining, retainedNonRat, world.remainingToRelease(),
             world.saved(), world.setup().saveTarget});
        // A completed result stops all remaining updates and passes.
        if (end_.phase() == LevelEndPhase::Results) return false;
      }
      return true;
    });
  }
  return updated;
}

bool RuntimeSimState::advance(sim::World& world, double elapsedSeconds, bool trackCompletion,
                              const std::function<void()>& afterTick) {
  if (paused_) return false;
  // 0x45DB0E-0x45DD5C selects 1 or the pass count at 0x4B15F8 (7).
  // Each pass gives the full frame delta to SimClock, with its own tick cap.
  const int passes = fastForward_ ? kFastForwardPasses : 1;
  bool ticked = false;
  for (int pass = 0; pass < passes; ++pass) {
    // A completed result stops all remaining passes.
    if (pass > 0 && trackCompletion && end_.phase() == LevelEndPhase::Results) break;
    clock_.advance(elapsedSeconds);
    while ((!trackCompletion || end_.phase() != LevelEndPhase::Results) && clock_.takeStep()) {
      // Connected play keeps hatches active while timeout is pending (425A95).
      world.tick(trackCompletion);
      ticked = true;
      if (afterTick) afterTick();
      if (trackCompletion) {
        int retainedNonRat = 0;
        for (const auto& lemming : world.lemmings())
          if (lemming.alive() && (world.nukeRequested() || lemming.sheetType != 3)) ++retainedNonRat;
        // Native5098 counts unreleased actors of the selected type. The clone uses
        // the ordinary population, clearing it on Nuke without changing actor counters.
        const bool nukeDraining = world.nukeRequested() && retainedNonRat > 0;
        end_.afterSimulationSlice(nukeDraining ? 0.0 : 1.0 / world.rules().ticksPerSecond,
            {world.timedOut() && !nukeDraining, retainedNonRat, world.remainingToRelease(),
             world.saved(), world.setup().saveTarget});
      }
    }
  }
  return ticked;
}

}  // namespace webo::app
