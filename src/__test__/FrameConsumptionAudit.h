#pragma once
#include "app/AppEntry.h"
#include "app/FrameConsumption.h"
#include "sim/World.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace webo::test {
// Explicit test opt-in; the override is read here, never by interactive play.
// It runs precisely the same driver with every frame drawn for paired evidence.
class FrameConsumptionAudit {
 public:
  explicit FrameConsumptionAudit(const std::filesystem::path& out)
      : scope_(!std::getenv("WEBOLUTION_TEST_RENDER_ALL")),
        gameplay_(out / "render-gameplay.txt"), releases_(out / "render-releases.txt"),
        frames_(out / "render-frame-state.txt"), actions_(out / "render-actions.txt"),
        outcomes_(out / "render-outcomes.txt"), work_(out / "render-work.txt"),
        out_(out), previous_(active_) {
    active_ = this;
    gameplay_ << std::setprecision(17);
    frames_ << std::setprecision(17);
  }
  ~FrameConsumptionAudit() {
    active_ = previous_;
    work_ << "frames=" << count_ << "\ndeferred=" << deferred_ << '\n';
  }
  static void inspected(const app::LevelFrameView& f) {
    if (!active_) return;
    const auto mask = app::levelFrameSpriteCoverage();
    std::ofstream out(active_->out_ / ("render-sprites-" + std::to_string(f.frame) + ".mask"), std::ios::binary);
    out.write(reinterpret_cast<const char*>(mask.data()), std::streamsize(mask.size()));
    // Each PNG's exact RGBA identifies its mask without changing existing PNG
    // naming, export sites or assertions in the historical drivers.
    std::uint32_t a = 1, b = 0;
    for (auto v : f.image.rgba) { a = (a + v) % 65521; b = (b + a) % 65521; }
    const std::uint32_t hash = (b << 16) | a;
    std::ofstream index(active_->out_ / "render-inspected.txt", std::ios::app);
    index << f.frame << ',' << hash << '\n';
  }
  void action(std::string_view value) { actions_ << lastFrame_ << ',' << value << '\n'; }
  void frame(const app::LevelFrameView& f) {
    lastFrame_ = f.frame;
    ++count_;
    deferred_ += f.image.rgba.empty();
    const auto& w = f.world;
    gameplay_ << f.frame << ',' << w.levelClockMs() << ',' << w.tickCount() << ','
              << w.explicitUpdateCount() << ',' << w.terrain().version();
    for (int n : w.setup().skills) gameplay_ << ',' << n;
    for (const auto& l : w.lemmings()) {
      gameplay_ << ';' << l.x << ',' << l.y << ',' << int(l.state) << ',' << int(l.facing)
                << ',' << l.fallDistance << ',' << l.frame << ',' << l.bricksLeft
                << ',' << l.bashCycles << ',' << l.fuseMs << ',' << l.fasterMs
                << ',' << l.speedPadLatched << ',' << l.canClimb << ',' << l.canFloat
                << ',' << l.verticalDir << ',' << l.sheetType << ',' << l.stepBudgetSeconds;
      for (auto n : l.teleportPadsLatched) gameplay_ << ':' << n;
    }
    for (const auto& l : w.setup().levers) gameplay_ << ";lever," << l.state << ',' << l.cooldownRemaining;
    for (const auto& g : w.setup().gates)
      gameplay_ << ";gate," << g.height << ',' << g.request << ',' << g.done << ',' << g.armed
                << ',' << g.wavePhase << ',' << g.animated;
    gameplay_ << '\n';
    releases_ << f.frame << ',' << w.levelClockMs() << ',' << w.spawned() << '\n';
    frames_ << f.frame << ',' << f.levelName << ',' << f.paused << ',' << f.selectedSkillIndex
            << ',' << f.cursorX << ',' << f.cursorY << ',' << f.cameraIntroActive
            << ',' << f.entryCueSeconds << ',' << bool(f.entryCueFrame) << ',' << f.levelMenuOpen
            << ',' << f.frontendConstructionCounter.value_or(-1) << ',' << f.audioDeviceReady
            << ',' << f.audioCuesDrained << ',' << f.audioCuesRouted << ',' << f.hatchOpeningProgress
            << ',' << f.drawnTerrainCellsChanged << ',' << f.drawnTerrainBlocksRebuilt
            << ',' << int(f.endPhase) << ',' << f.endTimerSeconds;
    const auto& p = f.displayedPose;
    frames_ << ',' << p.yawUnits << ',' << p.zoomShift << ',' << p.levelEyeHeight;
    for (auto n : f.cameraPickReference) frames_ << ',' << n;
    frames_ << '\n';
    outcomes_ << f.frame << ',' << w.spawned() << ',' << w.saved() << ',' << w.dead()
              << ',' << w.activeCount() << ',' << w.finished() << ',' << w.won()
              << ',' << w.timedOut() << ',' << w.timeRemainingTicks() << '\n';
  }
 private:
  app::ScopedFrameConsumption scope_;
  std::ofstream gameplay_, releases_, frames_, actions_, outcomes_, work_;
  std::filesystem::path out_;
  FrameConsumptionAudit* previous_;
  inline static thread_local FrameConsumptionAudit* active_ = nullptr;
  int count_ = 0, deferred_ = 0, lastFrame_ = -1;
};
inline const Image& inspectedImage(const app::LevelFrameView& f) {
  const auto& inspected = app::inspectLevelFrame(f);
  FrameConsumptionAudit::inspected(inspected);
  return inspected.image;
}
}  // namespace webo::test
