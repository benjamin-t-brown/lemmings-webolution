#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace lemmings {
struct ActorResourceSelectionHooks {
    void* context = nullptr;
    std::int32_t (*register_446040)(void*, std::string_view name) = nullptr;
};
// Loader block 0x4437F6..0x44389D, not the entire 0x4432C1 loader.
// Inputs are reread per category; callback may mutate their backing arrays and
// outputs. Enabled categories require quality 0 (Lo) or 1 (Hi) at the read.
// Disabled categories never inspect quality and store -1 without registration.
void select_actor_resources(const std::array<std::uint8_t,4>& enabled,
    const std::array<std::int32_t,4>& quality, std::array<std::int32_t,4>& ids,
    const ActorResourceSelectionHooks&);
} // namespace lemmings
