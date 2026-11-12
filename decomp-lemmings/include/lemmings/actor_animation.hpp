#pragma once

#include "lemmings/animation_playback.hpp"

namespace lemmings {

struct ActorAnimationOverride {
    const AnimationResource* resource = nullptr; // original record +0
    std::int32_t id = -1;                       // original record +4
};
struct ActorAnimationHooks {
    void* context = nullptr;
    void (*bind)(void*, std::int32_t) = nullptr; // 0x46836D on actor +0x128
    void (*select)(void*, const char*) = nullptr; // actor +0x78 virtual slot 0x30
};

bool contains_animation_clip(const AnimationResource& resource, const char* name); // 0x48408C
// 0x42FD25: category selects the default ID from global 0x4E335C. Every
// matching override in global 0x4C5B10 replaces it; the last match wins.
// Calls bind first, then select, even when the chosen ID equals the old ID.
void select_actor_animation(std::int32_t category,
                            std::span<const std::int32_t> default_ids,
                            std::span<const ActorAnimationOverride> overrides,
                            const char* name, const ActorAnimationHooks& hooks);

} // namespace lemmings
