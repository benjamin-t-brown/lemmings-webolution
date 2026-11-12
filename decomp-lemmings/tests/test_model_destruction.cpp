#include "lemmings/model_destruction.hpp"
#include <cassert>
#include <iostream>

namespace {
using namespace lemmings;
struct Effects {
    ModelConstructionState& model;
    unsigned calls=0;
    std::int32_t seen_id=0;
    static void release(void* context,std::int32_t id) {
        auto& self=*static_cast<Effects*>(context);++self.calls;self.seen_id=id;
        assert(self.model.original_vtable==0x4b14a8);
        self.model.original_vtable=0xdeadbeef;self.model.mode_88=32;
        self.model.animation_b0.playback.resource_id_1fc=55;
        self.model.animation_b0.playback.resource_1f8=reinterpret_cast<const AnimationResource*>(0x56780000);
    }
};
}
int main() {
    for(auto id:{-1,0,-2,2147483647})for(bool pointer:{false,true}) {
        ModelConstructionState model;model.mode_88=7;model.buffer_08.fill(0xa5);
        auto& playback=model.animation_b0.playback;
        playback.resource_id_1fc=id;playback.resource_1f8=pointer?reinterpret_cast<const AnimationResource*>(0x12340000):nullptr;
        Effects effects{model};const AnimationResourceHooks hooks{&effects,nullptr,Effects::release};
        destroy_actor_model(model,hooks);
        assert(model.original_vtable==0x4b1858 && model.buffer_08[0]==0xa5 && model.buffer_08[127]==0xa5);
        assert(effects.calls==(id!=-1 && pointer?1u:0u));
        if(effects.calls)assert(effects.seen_id==id && model.mode_88==32);
        if(id==-1)assert(playback.resource_1f8==(pointer?reinterpret_cast<const AnimationResource*>(0x12340000):nullptr));
        else assert(!playback.resource_1f8 && playback.resource_id_1fc==-1);
    }
    ModelConstructionState base;base.animation_b0.playback.resource_id_1fc=44;
    destroy_model_base(base);assert(base.original_vtable==0x4b1858 && base.animation_b0.playback.resource_id_1fc==44);
    std::cout<<"model destruction units passed\n";
}
