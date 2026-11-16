#include "app/RuntimeHud.h"
#include "app/RuntimeSkillPanel.h"
#include "app/ReleaseRateDisplay.h"

#include "shell/HudText.h"
#include "app/HoverFeedback.h"

#include <algorithm>
#include <cmath>

namespace webo::app {

void RuntimeReleaseRateControl::draw(Image& image, const Image& font) const {
  drawReleaseRateDisplay(image, font, current_);
}

void drawReleaseRateDisplay(Image& image, const Image& font, int rate) {
  // PROVISIONAL presentation, using the existing retail font. The label is a
  // rate (higher is faster), not seconds or remaining hatch population.
  for (int y = 412; y < std::min(468, image.height); ++y)
    for (int x = 18; x < std::min(90, image.width); ++x) {
      const auto p = (std::size_t(y) * image.width + x) * 4;
      const bool edge = x == 18 || x == 89 || y == 412 || y == 467;
      for (int c = 0; c < 3; ++c)
        image.rgba[p+c] = edge ? 200 : std::uint8_t(image.rgba[p+c] / 3);
    }
  const auto cells = shell::fontCells();
  std::array<shell::HudLine, 2> lines{{{"RATE", 0, 414, {255,255,255}},
      {std::to_string(rate), 0, 440, {255,232,96}}}};
  for (auto& line : lines) line.x = 54 - shell::textWidth(line.text, cells) / 2;
  shell::drawHud(image, font, lines, cells);
}


int hatchRemaining(int total, int spawned, std::size_t count, std::size_t index) {
  if (!count || index >= count) return 0;
  // SOURCE: World::spawn cycles the entrance vector once per successful
  // release. Count future releases assigned to this hatch under that existing
  // policy; do not substitute the original's unresolved per-hatch scheduling.
  const auto n = static_cast<long long>(count), i = static_cast<long long>(index);
  const auto quota = [=](int released) { return std::max(0LL, (released + n - 1 - i) / n); };
  return static_cast<int>(std::max(0LL, quota(std::max(0, total)) - quota(std::clamp(spawned, 0, std::max(0, total)))));
}

std::optional<std::size_t> hoveredHatch(std::span<const HatchHoverTarget> targets, int x, int y) {
  for (std::size_t i = 0; i < targets.size(); ++i) {
    const auto& h = targets[i];
    if (x >= h.left && x < h.right && y >= h.top && y < h.bottom) return i;
  }
  return std::nullopt;
}

void drawHatchHover(Image& image, const Image& font, const HatchHoverTarget& h) {
  // PROVISIONAL hatch highlight: four gold corner brackets. Static 454A85
  // evidences integer text (rate field +A0), not a remaining-count display or
  // the final original hatch highlight appearance. Remaining is the requested
  // playable-v1 choice; an original hover observation would settle the mismatch.
  const auto pixel = [&](int x, int y) {
    if (x < 0 || y < 0 || x >= image.width || y >= image.height) return;
    const auto at = (std::size_t(y) * image.width + x) * 4;
    image.rgba[at] = 255; image.rgba[at+1] = 232; image.rgba[at+2] = 96;
  };
  for (int d = 0; d < 6; ++d) for (int edge = 0; edge < 2; ++edge) {
    const int x = edge ? h.right + 2 : h.left - 3;
    const int sign = edge ? -1 : 1;
    pixel(x + sign*d, h.top-3); pixel(x + sign*d, h.bottom+2);
    pixel(x, h.top-3+d); pixel(x, h.bottom+2-d);
  }
  const auto cells = shell::fontCells();
  shell::HudLine line{shell::counterText(h.remaining), 0, 0, {255,255,255}};
  const int width = shell::textWidth(line.text, cells);
  // READ 454C34..454C64: centered on projected hatch, y - (44 << mode).
  // PROVISIONAL normal ^Font at 640x480: mode-0's other font is unresolved;
  // use mesh centre as anchor and clamp labels to keep them on the viewport.
  line.x = std::clamp((h.left+h.right-width)/2, 0, std::max(0, image.width-width));
  line.y = std::clamp((h.top+h.bottom)/2 - 44, 0, std::max(0, image.height-shell::kLineHeight));
  shell::drawHud(image, font, std::span<const shell::HudLine>(&line, 1), cells);
}

void drawRuntimeNukeFeedback(Image& image, const Image& font, const sim::World& world, bool armed) {
  if (!armed && !world.nukeRequested()) return;
  // PROVISIONAL control feedback: gold outline, confirmation hint and the
  // shared level-clock countdown. Native confirmation/overlay pixels unread.
  for (int y = 372; y < std::min(404, image.height); ++y)
    for (int x = 18; x < std::min(90, image.width); ++x)
      if (x < 20 || x >= 88 || y < 374 || y >= 402) {
        const auto p = (std::size_t(y) * image.width + x) * 4;
        image.rgba[p] = 255; image.rgba[p+1] = 220; image.rgba[p+2] = 64;
      }
  const std::string text = !world.nukeRequested() ? "CLICK AGAIN" :
      world.nukeCountdownMs() > 0 ? "NUKE " + std::to_string((world.nukeCountdownMs()+999)/1000) : "NUKE";
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
        // The completion slice receives this update's own dt, not a fixed 1/ticksPerSecond.
        // PROVISIONAL Nuke completion policy: finish its replacement/animation
        // sequence before the result delay can expire, even at timeout or on
        // a rat-only population. Ordinary completion/Bomber paths are unchanged.
        const bool nukeDraining = world.nukeRequested() && retainedNonRat > 0;
        end_.afterSimulationSlice(nukeDraining ? 0.0 : segmentSeconds,
            {world.timedOut() && !nukeDraining, retainedNonRat, world.remainingToRelease(),
             world.saved(), world.setup().saveTarget});
        // The existing completion stop also ends the remaining updates and passes.
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
  // READ 0x45DB0E-0x45DD5C: the latch selects the 0x4B15F8 pass count (7)
  // instead of 1, and every pass reuses the full frame delta. Only this pass
  // loop repeats; input and camera clocks run once per frame in the caller.
  // Each pass goes through the unchanged SimClock path, so the base tick rate
  // and the per-advance tick cap apply to every pass.
  const int passes = fastForward_ ? kFastForwardPasses : 1;
  bool ticked = false;
  for (int pass = 0; pass < passes; ++pass) {
    // The existing completion stop also ends the remaining passes.
    if (pass > 0 && trackCompletion && end_.phase() == LevelEndPhase::Results) break;
    clock_.advance(elapsedSeconds);
    while ((!trackCompletion || end_.phase() != LevelEndPhase::Results) && clock_.takeStep()) {
      // Native hatches continue while timeout-pending (425A95). Default World
      // fixtures keep their old timeout-release policy; connected play opts in.
      world.tick(trackCompletion);
      ticked = true;
      if (afterTick) afterTick();
      if (trackCompletion) {
        int retainedNonRat = 0;
        for (const auto& lemming : world.lemmings())
          if (lemming.alive() && (world.nukeRequested() || lemming.sheetType != 3)) ++retainedNonRat;
        // DERIVED ordinary-population adapter. Native5098 is selected-type
        // unreleased hatch population, not an active-actor count. Nuke cancels
        // it without falsifying spawned/dead/saved counters.
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
