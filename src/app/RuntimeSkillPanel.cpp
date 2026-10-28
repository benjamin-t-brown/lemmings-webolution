#include "app/RuntimeSkillPanel.h"

#include <algorithm>

namespace webo::app {

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

bool RuntimeSkillSelection::consumePanelClick(
    int x, int y, const std::array<int, 9>& counts) {
  const int index = runtimeSkillPanelIndex(x, y);
  if (index < 0) return false;
  if (index >= 0 && index < 8) selectAvailableIndex(index, counts);
  // Physical indices 8/9 are commands, not actor skills. Consume them so the
  // click cannot fall through to an actor behind this UI.
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
                           const RuntimeSkillSelection& selection) {
  panel.draw(dest, runtimeSkillPanelCounts(world), selection.selectedIndex());
}

}  // namespace webo::app
