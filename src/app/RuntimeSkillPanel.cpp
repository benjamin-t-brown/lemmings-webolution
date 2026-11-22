#include "app/RuntimeSkillPanel.h"
#include "assets/Box.h"
#include "assets/LevelObjects.h"
#include "sim/RetailLevel.h"

#include <algorithm>
#include <cmath>

namespace webo::app {

std::expected<RuntimeReleaseRateControl, std::string> RuntimeReleaseRateControl::create(
    const AssetLibrary& library, std::string_view levelId) {
  auto report = sim::loadRetailLevelReport(library, levelId);
  auto rates = loadReleaseRateTable(library);
  if (!report || !rates) return std::unexpected(!report ? report.error() : rates.error());
  const auto& hatches = report->hatches;
  if (hatches.empty()) return std::unexpected("release control: no hatch record");
  RuntimeReleaseRateControl control;
  for (const auto& hatch : hatches)
    control.limits_.push_back({hatch.rateMin, hatch.rateMax});
  control.rates_ = std::move(*rates);
  return control;
}

void RuntimeReleaseRateControl::change(std::size_t hatch, int delta, sim::World& world) {
  const int current = world.setup().entrances.at(hatch).releaseRate;
  const int rate = std::clamp(current + delta, minimum(hatch), maximum(hatch));
  // 453A42..453AAF passes the selected hatch to 425DE3, which leaves +C0 alone.
  if (rate != current)
    world.setHatchReleaseRate(hatch, rate, rates_.secondsPerLemming(rate),
                             rates_.intervalTicks(rate, world.rules().ticksPerSecond));
}

std::optional<std::size_t> RuntimeReleaseRateControl::pressTarget(
    std::span<const HatchHoverTarget> targets, int x, int y, int button, bool actorCanReceiveSkill) {
  // Provisional clone precedence: a valid left-click assignment takes priority.
  // The right button can adjust a hatch even when an actor is eligible.
  if (button != 3 && (button != 1 || actorCanReceiveSkill)) return std::nullopt;
  return hoveredHatch(targets, x, y);
}

std::optional<std::size_t> RuntimeReleaseRateControl::heldTarget(
    std::span<const HatchHoverTarget> targets, int x, int y) const {
  if (!heldHatch_) return std::nullopt;
  return hoveredHatch(targets, x, y, heldHatch_);
}

bool RuntimeReleaseRateControl::press(std::optional<std::size_t> hatch, int button, sim::World& world) {
  if (!hatch || *hatch >= hatchCount() || (button != 1 && button != 3)) return false;
  heldButton_ = button;
  heldHatch_ = hatch;
  repeatSeconds_ = 0;
  // READ 453A42..453AAF / 455DB0 / 455DF0: button 0 lowers, button 1
  // raises, queried as held state. Conventional SDL left/right binding.
  change(*hatch, button == 1 ? -1 : 1, world);
  return true;
}

bool RuntimeReleaseRateControl::advance(double seconds, std::span<const HatchHoverTarget> targets,
                                        int x, int y, sim::World& world) {
  if (!heldButton_) return false;
  if (!heldTarget(targets, x, y)) { reset(); return false; }
  if (!std::isfinite(seconds) || seconds <= 0) return false;
  const auto& hatch = world.setup().entrances.at(*heldHatch_);
  const int previous = hatch.releaseRate;
  // Provisional clone repeat steps immediately, waits 0.4 s, then repeats
  // every 0.1 s in real time. Native repeats on input updates.
  repeatSeconds_ += std::min(10.0, seconds);
  while (repeatSeconds_ + 1e-9 >= 0.4) {
    change(*heldHatch_, heldButton_ == 1 ? -1 : 1, world);
    repeatSeconds_ -= 0.1;
  }
  return hatch.releaseRate != previous;
}


void RuntimeNukeInput::press(int x, int y) {
  pressed_ = runtimeSkillPanelIndex(x, y) == 9;
  if (!pressed_) armed_ = false;
}

void RuntimeNukeInput::release(int x, int y, unsigned int timestampMs, sim::World& world) {
  const bool plate = pressed_ && runtimeSkillPanelIndex(x, y) == 9;
  pressed_ = false;
  if (!plate || world.nukeRequested() || world.setup().nukeStock <= 0) {
    armed_ = false;
    return;
  }
  if (armed_ && timestampMs - lastReleaseMs_ < 500) {
    world.nuke();
    armed_ = false;
  } else {
    lastReleaseMs_ = timestampMs;
    armed_ = true;
  }
}


sim::Skill RuntimeSkillSelection::selectedSkill() const {
  // Keep this mapping explicit: physical plate order is a separate contract.
  constexpr sim::Skill skills[] = {
      sim::Skill::Climber, sim::Skill::Floater, sim::Skill::Bomber,
      sim::Skill::Blocker, sim::Skill::Builder, sim::Skill::Basher,
      sim::Skill::Miner, sim::Skill::Digger};
  return skills[selectedIndex_];
}

bool RuntimeSkillSelection::selectIndex(int index) {
  if (index < 0 || index > 7) return false;
  selectedIndex_ = index;
  return true;
}

bool RuntimeSkillSelection::selectAvailableIndex(
    int index, const std::array<int, 9>& counts) {
  if (index < 0 || index > 7 || counts[static_cast<std::size_t>(index)] <= 0)
    return false;
  selectedIndex_ = index;
  return true;
}

int runtimeSkillPanelIndex(int x, int y) {
  // Clone hit geometry uses ten 72x32 plates.
  if (x < 18 || x >= 90) return -1;
  if (y < 12 || y >= 404) return -1;
  const int offset = y - 12;
  if (offset % 40 >= 32) return -1;
  return offset / 40;
}

RuntimePanelCommand runtimeSkillPanelCommand(int x, int y) {
  switch (runtimeSkillPanelIndex(x, y)) {
    case 8: return RuntimePanelCommand::FastForward;
    case 9: return RuntimePanelCommand::Armageddon;
    default: return RuntimePanelCommand::None;
  }
}

bool RuntimeSkillSelection::consumePanelClick(
    int x, int y, const std::array<int, 9>& counts) {
  const int index = runtimeSkillPanelIndex(x, y);
  if (index < 0) return false;
  if (index >= 0 && index < 8) selectAvailableIndex(index, counts);
  // Consume command plates before actor picking. Plate 8 bypasses stock
  // and selection (0x455AFF). The caller controls fast-forward.
  return true;
}

std::array<int, 9> runtimeSkillPanelCounts(const sim::World& world) {
  std::array<int, 9> result{};
  const auto& skills = world.setup().skills;
  for (std::size_t i = 0; i < skills.size(); ++i)
    result[i] = std::clamp(skills[i], 0, 99);
  result[8] = 0;
  return result;
}

void drawRuntimeSkillPanel(Image& dest, const RetailSkillPanel& panel,
                           const sim::World& world,
                           const RuntimeSkillSelection& selection, bool fastForward) {
  panel.draw(dest, runtimeSkillPanelCounts(world), selection.selectedIndex(), fastForward);
}

void drawRuntimeSkillPanel(Image& dest, const RetailSkillPanel& panel,
                           const sim::World& world,
                           const RuntimeSkillSelection& selection, bool fastForward,
                           const SkillPanelIconAnimation* animation) {
  panel.draw(dest, runtimeSkillPanelCounts(world), selection.selectedIndex(), fastForward, animation);
}

}  // namespace webo::app
