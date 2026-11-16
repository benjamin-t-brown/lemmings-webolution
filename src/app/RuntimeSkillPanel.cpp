#include "app/RuntimeSkillPanel.h"
#include "assets/Box.h"
#include "assets/LevelObjects.h"

#include <algorithm>
#include <cmath>

namespace webo::app {

std::expected<RuntimeReleaseRateControl, std::string> RuntimeReleaseRateControl::create(
    const AssetLibrary& library, std::string_view levelId) {
  auto bytes = library.read("COMPILED\\" + std::string(levelId) + ".LVL");
  auto rates = loadReleaseRateTable(library);
  if (!bytes || !rates) return std::unexpected(!bytes ? bytes.error() : rates.error());
  const auto hatches = readHatchRecords(*bytes);
  if (hatches.empty()) return std::unexpected("release control: no hatch record");
  const auto& hatch = hatches.front();
  if (hatch.rateMin < 1 || hatch.rateMax > static_cast<int>(rates->size()) || hatch.rateMin > hatch.rateMax)
    return std::unexpected("release control: invalid stored rate limits");
  RuntimeReleaseRateControl control;
  control.rates_ = std::move(*rates);
  control.minimum_ = hatch.rateMin;
  control.maximum_ = hatch.rateMax;
  control.current_ = std::clamp(hatch.rate, hatch.rateMin, hatch.rateMax);
  // Loading is deliberately read-only: no player input must leave the accepted
  // RetailLevel timing path bit-for-bit unchanged, including first release.
  return control;
}

bool RuntimeReleaseRateControl::contains(int x, int y) {
  // PROVISIONAL panel layout: a separate box below Nuke. Native 453A31 uses
  // the hovered world hatch (+1894); no native panel-box geometry is claimed.
  return x >= 18 && x < 90 && y >= 412 && y < 468;
}

void RuntimeReleaseRateControl::change(int delta, sim::World& world) {
  const int rate = std::clamp(current_ + delta, minimum_, maximum_);
  if (rate == current_) return;
  current_ = rate;
  // READ 425DE3: change +A0 and table interval +B8, never countdown +C0.
  world.setReleaseInterval(rates_.secondsPerLemming(rate),
                           rates_.intervalTicks(rate, world.rules().ticksPerSecond));
}

bool RuntimeReleaseRateControl::press(int x, int y, int button, sim::World& world) {
  if (!contains(x, y) || (button != 1 && button != 3)) return false;
  heldButton_ = button;
  repeatSeconds_ = 0;
  // READ 453A42..453AAF / 455DB0 / 455DF0: button 0 lowers, button 1
  // raises, queried as held state. Conventional SDL left/right binding.
  change(button == 1 ? -1 : 1, world);
  return true;
}

void RuntimeReleaseRateControl::advance(double seconds, int x, int y, sim::World& world) {
  if (!heldButton_) return;
  if (!contains(x, y)) { reset(); return; }
  if (!std::isfinite(seconds) || seconds <= 0) return;
  // PROVISIONAL player-friendly repeat: immediate unit step, then 0.4 s delay
  // and 0.1 s repeats, independent of simulation pause/fast-forward. Original
  // repeats per input update; authorized original observation would settle feel.
  repeatSeconds_ += std::min(10.0, seconds);
  while (repeatSeconds_ + 1e-9 >= 0.4) {
    change(heldButton_ == 1 ? -1 : 1, world);
    repeatSeconds_ -= 0.1;
  }
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
  // Source-policy geometry: ten visible 72x32 plates at this fixed position.
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
  // Physical indices 8/9 are commands, not actor skills. Consume them so the
  // click cannot fall through to an actor behind this UI. Plate 8 never reads
  // stock or changes selection (0x455AFF); the fast-forward mode itself is
  // changed by the caller via runtimeSkillPanelCommand and RuntimeSimState.
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
