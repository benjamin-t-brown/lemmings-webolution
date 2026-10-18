// Player progress for the Phase 5 shell.
//
// Not the stock SavedGame.dat layout — that file is unread. Behaviour matches
// the original's intent: slots unlock as levels are cleared, Load/Save keep
// language and volumes, and a new game resets the unlock chain.

#pragma once

#include "assets/GameData.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace webo::shell {

enum class SlotState : std::uint8_t { Locked = 0, Unlocked = 1, Completed = 2 };

struct Progress {
  std::string language = "UK";
  float musicVolume = 0.6f;
  float sfxVolume = 1.0f;

  // Flat 5*25 entries matching LevelTable::groups order.
  std::vector<SlotState> slots;

  static constexpr int kGroups = 5;
  static constexpr int kSlotsPerGroup = 25;

  void resizeFor(const LevelTable& table);
  void newGame(const LevelTable& table);
  // Mark (group, slot) completed and unlock the next playable slot.
  void complete(const LevelTable& table, int group, int slot);
  bool canPlay(int group, int slot) const;
  SlotState state(int group, int slot) const;

  std::size_t index(int group, int slot) const {
    return static_cast<std::size_t>(group) * kSlotsPerGroup + static_cast<std::size_t>(slot);
  }
};

std::filesystem::path defaultProgressPath();

std::expected<void, std::string> saveProgress(const Progress&, const std::filesystem::path&);
std::expected<Progress, std::string> loadProgress(const std::filesystem::path&);

// First non-blank slot index in a group, or -1.
int firstPlayableSlot(const LevelTable& table, int group);
// Next non-blank after (group,slot), wrapping to the next group. -1 if none.
bool nextPlayable(const LevelTable& table, int group, int slot, int& outGroup, int& outSlot);

}  // namespace webo::shell
