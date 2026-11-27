#include "app/RuntimeHud.h"
#include "app/LevelLayout.h"
#include "app/RuntimeSkillPanel.h"

#include "shell/HudText.h"
#include "shell/MenuRender.h"
#include "app/HoverFeedback.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace webo::app {

int hatchRemaining(int total, int spawned, std::size_t count, std::size_t index) {
  if (!count || index >= count) return 0;
  // Count pending releases for this hatch using World's round-robin entrance order.
  const auto n = static_cast<long long>(count), i = static_cast<long long>(index);
  const auto quota = [=](int released) { return std::max(0LL, (released + n - 1 - i) / n); };
  return static_cast<int>(std::max(0LL, quota(std::max(0, total)) - quota(std::clamp(spawned, 0, std::max(0, total)))));
}

namespace {
shell::HudLine centredLine(const Image& image, std::span<const shell::GlyphCell> cells,
                           std::string text, int left, int right, int y, shell::Rgb colour) {
  shell::HudLine line{std::move(text), 0, 0, colour};
  const int width = shell::textWidth(line.text, cells);
  line.x = std::clamp((left+right-width)/2, 0, std::max(0, image.width-width));
  line.y = std::clamp(y, 0, std::max(0, image.height-shell::kLineHeight));
  return line;
}

shell::HudLine hoverCountLine(const Image& image, std::span<const shell::GlyphCell> cells,
                              int count, int left, int right, int y) {
  return centredLine(image, cells, shell::counterText(count), left, right, y, {255,255,255});
}
}  // namespace

void drawHatchHover(Image& image, const Image& font, const HatchHoverTarget& h,
                    std::optional<int> adjustingRate, std::optional<int> noticeRate) {
  const auto cells = shell::fontCells();
  // The clone displays remaining population. Native 0x454B95 passes release rate.
  // 454C34..454C64 centres text on projected hatch Y - (44 << mode).
  // Provisional 640x480 display uses ^Font, mesh centre and viewport clamping.
  const int countY = (h.top+h.bottom)/2 - 44;
  const bool showingNotice = noticeRate && !adjustingRate;
  auto line = showingNotice
      ? centredLine(image, cells, "RATE " + std::to_string(*noticeRate), h.left, h.right, countY,
                    {255,232,96})
      : hoverCountLine(image, cells, h.remaining, h.left, h.right, countY);
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

namespace {

constexpr std::string_view kRateDecrease = "-", kRateIncrease = "+";
constexpr shell::Rgb kRateOrange{0xE0, 0x80, 0}, kRateHoverRed{255, 0, 0};

struct RateButtonGeometry {
  int x = 0, y = 0;
  std::array<int, 4> bounds{};
};

RateButtonGeometry rateButtonGeometry(const HatchHoverTarget& h, int direction,
                                      int frameWidth, int frameHeight) {
  const auto& cells = shell::fontCells();
  const auto label = direction > 0 ? kRateIncrease : kRateDecrease;
  const int width = shell::textWidth(label, cells);
  const int y = std::clamp((h.top + h.bottom) / 2 - shell::kLineHeight / 2,
      0, std::max(0, frameHeight - shell::kLineHeight));
  constexpr int gap = 10, padX = 4, padY = 2;
  const int wantedX = direction > 0 ? h.right + gap : h.left - gap - width;
  const int x = std::clamp(wantedX, 0, std::max(0, frameWidth - width));
  const auto bounds = std::array<int, 4>{
      std::max(0, x - padX), std::max(0, y - padY),
      std::min(frameWidth, x + width + padX), std::min(frameHeight, y + shell::kLineHeight + padY)};
  return {x, y, bounds};
}

}  // namespace

std::optional<HatchRateButton> hatchRateButtonAt(std::span<const HatchHoverTarget> targets,
                                                 int x, int y, int width, int height) {
  for (const auto& h : targets)
    for (const int direction : {-1, 1}) {
      const auto [textX, textY, bounds] = rateButtonGeometry(h, direction, width, height);
      (void)textX;
      (void)textY;
      if (pointerOverSceneBounds(x, y, bounds[0], bounds[1], bounds[2], bounds[3]))
        return HatchRateButton{h.index, direction};
    }
  return std::nullopt;
}

void drawHatchRateButtons(Image& image, const Image& font,
                          std::span<const HatchHoverTarget> targets,
                          std::optional<std::pair<int, int>> hover) {
  const auto& cells = shell::fontCells();
  for (const auto& h : targets)
    for (const int direction : {-1, 1}) {
      const auto [x, y, bounds] = rateButtonGeometry(h, direction, image.width, image.height);
      const bool hovered = hover && pointerOverSceneBounds(
          hover->first, hover->second, bounds[0], bounds[1], bounds[2], bounds[3]);
      const shell::HudLine line{direction > 0 ? std::string(kRateIncrease) : std::string(kRateDecrease),
                                x, y, hovered ? kRateHoverRed : kRateOrange};
      shell::drawHud(image, font, std::span(&line, 1), cells);
    }
}

void drawExitHover(Image& image, const Image& font, const PickBox& box, int saveTarget) {
  const auto cells = shell::fontCells();
  // Clone addition. Put the level's save target just above the visible model.
  const int above = box.top - shell::kLineHeight - 4;
  const auto line = hoverCountLine(image, cells, saveTarget, box.left, box.right,
                                   above >= 0 ? above : box.bottom + 4);
  shell::drawHud(image, font, std::span<const shell::HudLine>(&line, 1), cells);
}

void drawRuntimeNukeFeedback(Image& image, const Image& font, const sim::World& world,
                             bool armed, bool portrait) {
  if (!armed && !world.nukeRequested()) return;
  // PROVISIONAL control feedback: gold outline and confirmation/status hint.
  // The large countdown is composed with the level's Let's Go text in main.
  const auto [left, top, width, height] = skillPlate(9, portrait);
  const int right = left + width, bottom = top + height;
  for (int y = top; y < std::min(bottom, image.height); ++y)
    for (int x = left; x < std::min(right, image.width); ++x)
      if (x < left + 2 || x >= right - 2 || y < top + 2 || y >= bottom - 2) {
        const auto p = (std::size_t(y) * image.width + x) * 4;
        image.rgba[p] = 255; image.rgba[p+1] = 220; image.rgba[p+2] = 64;
      }
  const auto& cells = shell::fontCells();
  const std::string text = world.nukeRequested() ? "NUKE" : (portrait ? "TAP AGAIN" : "CLICK AGAIN");
  const auto line = portrait ? centredLine(image, cells, text, left, right, bottom, {255,220,64})
                             : shell::HudLine{text, 100, 380, {255,220,64}};
  shell::drawHud(image, font, std::span(&line, 1), cells);
}

shell::HudValues runtimeHudValues(const sim::World& world) {
  shell::HudValues values;
  values.out = world.spawned() - world.saved() - world.dead();
  values.saved = world.saved();
  values.saveTarget = world.setup().saveTarget;
  if (world.explicitUpdateCount() > 0) {
    // Live play follows the repeating level-second event (0x4458B0).
    const int limitSeconds = std::max(0, world.setup().timeLimitTicks / world.rules().ticksPerSecond);
    const auto secondsElapsed = std::min<std::uint64_t>(world.levelSecondEvents(), limitSeconds);
    values.secondsRemaining = limitSeconds - static_cast<int>(secondsElapsed);
  } else {
    values.secondsRemaining = static_cast<int>(
        world.timeRemainingTicks() / world.rules().ticksPerSecond);
  }
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
  setPause(paused() ? Pause::Playing : Pause::Standard);
}

void RuntimeSimState::toggleExaminePause() {
  setPause(examining() ? Pause::Playing : Pause::Examine);
}

void RuntimeSimState::setPause(Pause pause) {
  if (pause_ == pause) return;
  pause_ = pause;
  clock_ = SimClock{};
  carryMs_ = 0.0;
}

void RuntimeSimState::resume() {
  setPause(Pause::Playing);
}

bool RuntimeSimState::advanceElapsed(sim::World& world, double elapsedSeconds, bool trackCompletion,
                                     const std::function<void()>& afterTick, int stallCapMs) {
  return advanceElapsed(world, elapsedSeconds, trackCompletion, afterTick, stallCapMs, true);
}

bool RuntimeSimState::advanceElapsed(sim::World& world, double elapsedSeconds, bool trackCompletion,
                                     const std::function<void()>& afterTick, int stallCapMs, bool releaseEnabled,
                                     std::optional<double> releaseOpensAfter) {
  if (paused()) return false;
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
    const auto afterUpdate = [&](double segmentSeconds) {
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
    };
    world.advanceElapsedMsWithReleaseGate(dtMs, trackCompletion, releaseEnabled, afterUpdate,
                                          pass == 0 ? releaseOpensAfter : std::nullopt);
  }
  return updated;
}

bool RuntimeSimState::advance(sim::World& world, double elapsedSeconds, bool trackCompletion,
                              const std::function<void()>& afterTick) {
  if (paused()) return false;
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
