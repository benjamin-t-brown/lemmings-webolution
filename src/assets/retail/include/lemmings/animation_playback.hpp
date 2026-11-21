#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace lemmings {

// Fields read from the original 0x98-byte clip record; frame payloads omitted.
struct AnimationClip {
    std::int32_t frame_count = 0; // +0
    double speed = 1.0;          // +0x10
    std::string name;            // inline C string at +0x18
};
struct AnimationResource {
    std::span<const AnimationClip> clips;
};
struct AnimationPlayback {
    double fraction_1c0 = 0.0;
    std::int32_t clip_1c8 = 0;
    std::int32_t frame_1cc = 0;
    std::int32_t direction_1d0 = 1;
    const AnimationResource* resource_1f8 = nullptr;
    std::int32_t resource_id_1fc = -1;
    std::uint8_t finished_200 = 0;
    std::uint8_t looping_201 = 1;
};
struct AnimationResourceHooks {
    void* context = nullptr;
    const AnimationResource* (*acquire)(void*,std::int32_t) = nullptr; // 0x42FF90
    void (*release)(void*,std::int32_t) = nullptr;                    // 0x430050
};

// 0x468833 (model wrapper 0x43CF30). Positive infinite or excessively large
// positive accumulated time can fail to terminate, as in the original.
void advance_animation(AnimationPlayback& state, double elapsed,
                       const AnimationResourceHooks& hooks = {});
// 0x46836D: release old ID (unless -1), store ID, then acquire resource.
void bind_animation_resource(AnimationPlayback& state, std::int32_t id,
                             const AnimationResourceHooks& hooks);
// 0x4840F3: first exact C-string match, or clip count if absent.
std::int32_t find_animation_clip(const AnimationResource& resource,const char* name);
// 0x468AB8/0x43CF53; selecting a missing name retains the one-past-end ID.
void select_animation(AnimationPlayback& state,const char* name);
// 0x43CF72 and 0x43D030; these operations do not clear the finished flag.
void select_animation_index(AnimationPlayback& state,std::int32_t index);
void set_animation_fraction(AnimationPlayback& state,double fraction);
// 0x468B04/0x468B5D return an x87 quotient without a final double store.
long double animation_duration(const AnimationPlayback& state);
long double animation_time(const AnimationPlayback& state);
std::uint8_t animation_finished(const AnimationPlayback& state); // 0x468BA3
void set_animation_looping(AnimationPlayback& state,std::uint8_t looping); // 0x468BD9

} // namespace lemmings
