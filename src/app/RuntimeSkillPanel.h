#pragma once

#include "app/SkillPanel.h"
#include "assets/GameData.h"
#include "sim/World.h"

#include <array>
#include <optional>
#include <vector>

namespace webo::app {

// Provisional clone input uses the clicked hatch's limits on the shared rate.
class RuntimeReleaseRateControl {
 public:
  static std::expected<RuntimeReleaseRateControl, std::string> create(
      const AssetLibrary& library, std::string_view levelId);
  // SDL button numbers: conventional button 0/1 native state maps to left/right.
  bool press(std::optional<std::size_t> hatch, int button, sim::World& world);
  void release(int button) { if (button == heldButton_) reset(); }
  void reset() { heldButton_ = 0; heldHatch_.reset(); repeatSeconds_ = 0; }
  void advance(double seconds, std::optional<std::size_t> hatch, sim::World& world);
  bool holding() const { return heldButton_ != 0; }
  int current() const { return current_; }
  std::size_t rateHatch() const { return rateHatch_; }
  std::size_t hatchCount() const { return limits_.size(); }
  int minimum(std::size_t hatch = 0) const { return limits_.at(hatch)[0]; }
  int maximum(std::size_t hatch = 0) const { return limits_.at(hatch)[1]; }
 private:
  void change(std::size_t hatch, int delta, sim::World& world);
  ReleaseRates rates_;
  std::vector<std::array<int, 2>> limits_;
  std::optional<std::size_t> heldHatch_;
  std::size_t rateHatch_ = 0;
  int current_ = 1, heldButton_ = 0;
  double repeatSeconds_ = 0;
};

// Same SOURCE plate geometry for hover and click; -1 means outside all plates.
int runtimeSkillPanelIndex(int x, int y);

// Plate 8 is fast-forward on hover with button 0 held (0x455AFF).
// Plate 9 is Armageddon.
enum class RuntimePanelCommand { None, FastForward, Armageddon };

// Uses runtimeSkillPanelIndex to share command and click hit regions.
RuntimePanelCommand runtimeSkillPanelCommand(int x, int y);

// For the release pair at 0x483B5D/0x454725, provisional clone input
// requires two complete clicks on the plate less than 500 ms apart.
class RuntimeNukeInput {
 public:
  void press(int x, int y);
  void release(int x, int y, unsigned int timestampMs, sim::World& world);
  void reset() { pressed_ = armed_ = false; }
  bool armed() const { return armed_; }
 private:
  bool pressed_ = false, armed_ = false;
  unsigned int lastReleaseMs_ = 0;
};
void drawRuntimeNukeFeedback(Image& image, const Image& font,
                             const sim::World& world, bool armed);

// A skill needs positive stock for plate selection. Keyboard input and
// click geometry use clone mappings.
class RuntimeSkillSelection {
 public:
  int selectedIndex() const { return selectedIndex_; }
  sim::Skill selectedSkill() const;
  bool selectIndex(int index);
  bool selectAvailableIndex(int index, const std::array<int, 9>& counts);
  bool consumePanelClick(int x, int y, const std::array<int, 9>& counts);

 private:
  int selectedIndex_ = 0;
};

// Provisional clone display policy. Clamp eight live stock values to [0,99],
// display unlimited (-1) as zero, and set the ninth slot to zero.
std::array<int, 9> runtimeSkillPanelCounts(const sim::World& world);
void drawRuntimeSkillPanel(Image& dest, const RetailSkillPanel& panel,
                           const sim::World& world,
                           const RuntimeSkillSelection& selection,
                           bool fastForward = false);
void drawRuntimeSkillPanel(Image& dest, const RetailSkillPanel& panel,
                           const sim::World& world,
                           const RuntimeSkillSelection& selection, bool fastForward,
                           const SkillPanelIconAnimation* animation);

}  // namespace webo::app
