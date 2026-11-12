#include "lemmings/animation_playback.hpp"
#include <array>
#include <cmath>
#include <limits>
#include <iostream>

int main() {
    const std::array<lemmings::AnimationClip,3> clips{{{3,1,"walk"},{2,2,"fall"},{1,1,"walk"}}};
    const lemmings::AnimationResource resource{clips};
    lemmings::AnimationPlayback state;
    state.resource_1f8=&resource;
    lemmings::advance_animation(state,0.06);
    if(state.frame_1cc!=0 || state.fraction_1c0!=0.06) return 1;
    lemmings::advance_animation(state,0.001);
    if(state.frame_1cc!=1) return 1;
    lemmings::select_animation_index(state,0);
    state.looping_201=0;
    lemmings::advance_animation(state,0.5);
    if(state.frame_1cc!=2 || !state.finished_200) return 1;
    lemmings::advance_animation(state,0);
    if(state.finished_200) return 1; // finished is an event flag, not a permanent status
    state.direction_1d0=0;state.frame_1cc=0;state.fraction_1c0=0;
    lemmings::advance_animation(state,0.061);
    if(state.frame_1cc!=0 || !state.finished_200) return 1;
    state.looping_201=7;state.fraction_1c0=0;
    lemmings::advance_animation(state,0.061);
    if(state.frame_1cc!=2) return 1;
    if(lemmings::find_animation_clip(resource,"walk")!=0 ||
       lemmings::find_animation_clip(resource,"WALK")!=3) return 1;
    state.finished_200=9;
    lemmings::select_animation(state,"fall");
    if(state.clip_1c8!=1 || state.frame_1cc!=0 || state.fraction_1c0!=0 || state.finished_200!=9) return 1;
    lemmings::advance_animation(state,std::numeric_limits<double>::quiet_NaN());
    if(!std::isnan(state.fraction_1c0) || state.frame_1cc!=0 || state.finished_200!=0) return 1;
    std::cout<<"animation boundary checks passed\n";
}
