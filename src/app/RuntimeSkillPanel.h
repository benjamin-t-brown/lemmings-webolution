#pragma once

#include "app/SkillPanel.h"
#include "sim/World.h"

#include <array>

namespace webo::app {

// Same SOURCE plate geometry for hover and click; -1 means outside all plates.
int runtimeSkillPanelIndex(int x, int y);

// The two command plates after the eight skill plates. READ 0x455AFF: hover
// index 8 with mouse button 0 down is the fast-forward plate. Index 9 is
// Armageddon; its action is deliberately not implemented (parked decision).
enum class RuntimePanelCommand { None, FastForward, Armageddon };

// Reports which command plate, if any, covers the window point. Uses
// runtimeSkillPanelIndex for geometry, so hit testing is shared with clicks.
RuntimePanelCommand runtimeSkillPanelCommand(int x, int y);

// Runtime selection/input policy. READ evidence establishes a positive-stock
// gate for selecting skill plates. Keyboard selection remains a SOURCE adapter;
// the click rectangles are not a claim about native physical hit testing.
class RuntimeSkillSelection {
 public:
  int selectedIndex() const { return selectedIndex_; }
  sim::Skill selectedSkill() const;
  // Generic selection helper for source fixtures and rendering tests.
  bool selectIndex(int index);
  bool selectAvailableIndex(int index, const std::array<int, 9>& counts);
  bool consumePanelClick(int x, int y, const std::array<int, 9>& counts);

 private:
  int selectedIndex_ = 0;
};

// SOURCE display policy: eight live stocks clamped to [0,99], including hiding
// synthetic unlimited (-1). Ninth slot retains the C1 fixture's zero; native
// out-of-range/unlimited formatting is not established by this adapter.
std::array<int, 9> runtimeSkillPanelCounts(const sim::World& world);
void drawRuntimeSkillPanel(Image& dest, const RetailSkillPanel& panel,
                           const sim::World& world,
                           const RuntimeSkillSelection& selection);

}  // namespace webo::app
