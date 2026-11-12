#include "lemmings/animation_playback.hpp"
#include <bit>
#include <cstring>
#include <stdexcept>

namespace lemmings {
namespace {
constexpr double frame_time=0.06;
constexpr double tiny=static_cast<double>(1.0e-8f);
double rounded(long double value) {
    volatile double result=static_cast<double>(value);
    return result;
}
std::int32_t add_wrapping(std::int32_t value,std::uint32_t increment) {
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(value)+increment);
}
const AnimationClip& clip(const AnimationPlayback& state) {
    if(!state.resource_1f8 || state.clip_1c8<0 ||
       static_cast<std::size_t>(state.clip_1c8)>=state.resource_1f8->clips.size())
        throw std::out_of_range("animation clip is outside its resource");
    return state.resource_1f8->clips[static_cast<std::size_t>(state.clip_1c8)];
}
}
void advance_animation(AnimationPlayback& state,double elapsed,
                       const AnimationResourceHooks& hooks) {
    if(!state.resource_1f8) {
        if(!hooks.acquire) throw std::invalid_argument("animation resource acquisition required");
        state.resource_1f8=hooks.acquire(hooks.context,state.resource_id_1fc);
    }
    const double speed=clip(state).speed;
    state.fraction_1c0=rounded(static_cast<long double>(elapsed)*speed+state.fraction_1c0);
    state.finished_200=0;
    if(state.direction_1d0==1) {
        while(state.fraction_1c0>frame_time) {
            state.frame_1cc=add_wrapping(state.frame_1cc,1);
            if(state.frame_1cc==clip(state).frame_count) {
                state.frame_1cc=state.looping_201 ? 0 : add_wrapping(clip(state).frame_count,UINT32_MAX);
                state.finished_200=1;
            }
            state.fraction_1c0=rounded(static_cast<long double>(state.fraction_1c0)-frame_time);
            if(!(state.fraction_1c0>tiny)) state.fraction_1c0=0.0;
        }
    } else {
        while(state.fraction_1c0>frame_time) {
            state.frame_1cc=add_wrapping(state.frame_1cc,UINT32_MAX);
            if(state.frame_1cc<0) {
                state.frame_1cc=state.looping_201 ? add_wrapping(clip(state).frame_count,UINT32_MAX) : 0;
                state.finished_200=1;
            }
            state.fraction_1c0=rounded(static_cast<long double>(state.fraction_1c0)-frame_time);
            // Reverse playback uses TEST AH,1; equality is retained.
            if(!(state.fraction_1c0>=tiny)) state.fraction_1c0=0.0;
        }
    }
}
void bind_animation_resource(AnimationPlayback& state,std::int32_t id,
                             const AnimationResourceHooks& hooks) {
    if(!hooks.acquire || (state.resource_id_1fc!=-1 && !hooks.release))
        throw std::invalid_argument("animation resource hooks required");
    if(state.resource_id_1fc!=-1) hooks.release(hooks.context,state.resource_id_1fc);
    state.resource_id_1fc=id;
    state.resource_1f8=hooks.acquire(hooks.context,state.resource_id_1fc);
}
std::int32_t find_animation_clip(const AnimationResource& resource,const char* name) {
    std::int32_t index=0;
    for(const auto& entry:resource.clips) {
        if(std::strcmp(name,entry.name.c_str())==0) break;
        ++index;
    }
    return index;
}
void select_animation(AnimationPlayback& state,const char* name) {
    if(!state.resource_1f8) throw std::invalid_argument("animation resource is null");
    select_animation_index(state,find_animation_clip(*state.resource_1f8,name));
}
void select_animation_index(AnimationPlayback& state,std::int32_t index) {
    state.clip_1c8=index;
    state.frame_1cc=0;
    state.fraction_1c0=0.0;
}
void set_animation_fraction(AnimationPlayback& state,double fraction) {
    state.fraction_1c0=fraction;
}
long double animation_duration(const AnimationPlayback& state) {
    const double scaled=rounded(static_cast<long double>(clip(state).frame_count)*frame_time);
    return static_cast<long double>(scaled)/clip(state).speed;
}
long double animation_time(const AnimationPlayback& state) {
    const double scaled=rounded(static_cast<long double>(state.frame_1cc)*frame_time+state.fraction_1c0);
    return static_cast<long double>(scaled)/clip(state).speed;
}
std::uint8_t animation_finished(const AnimationPlayback& state) {return state.finished_200;}
void set_animation_looping(AnimationPlayback& state,std::uint8_t looping) {state.looping_201=looping;}
} // namespace lemmings
