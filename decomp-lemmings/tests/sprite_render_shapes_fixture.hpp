#pragma once
#include "sprite_render_geometry_fixture.hpp"
#include "lemmings/sprite_render_shapes.hpp"
#include <cstdlib>
namespace sprite_shapes_test {
using namespace lemmings;
struct Fixture: sprite_geometry_test::Fixture {
    SpriteShapeState state;SpriteDeviceState device{};std::vector<std::array<std::uint64_t,7>> events;
    void(*before)(Fixture&)=nullptr;void(*after)(Fixture&)=nullptr;
    explicit Fixture(unsigned i):sprite_geometry_test::Fixture(i){
        state={i%3==0?0u:mode,reference,static_cast<std::uint8_t>(i%3?0xff:0)};
        model.unknown_1b4[0]=static_cast<std::byte>(i%3==0?0:i%3==1?0x80:1);device.device_58cdb8=reinterpret_cast<void*>(0x1100);device.alpha_reference_4c24e0=77;
    }
    void event(unsigned kind,unsigned type,unsigned value,void* dev){
        if(before)before(*this);events.push_back({kind,type,value,reinterpret_cast<std::uintptr_t>(dev),device.alpha_reference_4c24e0,state.mode_58d53c,state.reference_58cb38});
        if(kind==1&&type==16&&trial%8==0){state.mode_58d53c=state.reference_58cb38;device.device_58cdb8=reinterpret_cast<void*>(0x2200);model.piece_index_1b8=2;}
        if(kind==1&&type==16&&trial%8==1){device.alpha_reference_4c24e0=99;state.filtering_supported_58d121=0;}
        if(kind==2&&trial%8==2){state.filtering_supported_58d121=0xff;device.device_58cdb8=reinterpret_cast<void*>(0x2200);}
        if(kind==2&&trial%8==3)state.filtering_supported_58d121=0;
        if(after)after(*this);
    }
    void submit_shape(){sprite_geometry_test::Fixture::submit();event(2,0,0,device.device_58cdb8);}
    SpriteShapeHooks hooks(){return {{this,[](void* p,SpriteRenderGeometry&){static_cast<Fixture*>(p)->submit_shape();}},{this,nullptr,[](void* p,void* dev,unsigned stage,unsigned type,unsigned value){if(stage)std::abort();static_cast<Fixture*>(p)->event(1,type,value,dev);},nullptr}};}
    void run(bool anchored){if(anchored)build_sprite_anchored_rectangle(model,arguments[0],arguments[1],state,device,hooks());else build_sprite_trapezoid(model,arguments[0],arguments[1],arguments[2],arguments[3],arguments[2]/2,state.mode_58d53c,hooks().geometry);}
    bool same(const Fixture& other)const{return sprite_geometry_test::Fixture::same(other)&&events==other.events&&state.mode_58d53c==other.state.mode_58d53c&&state.reference_58cb38==other.state.reference_58cb38&&state.filtering_supported_58d121==other.state.filtering_supported_58d121&&device.device_58cdb8==other.device.device_58cdb8&&device.alpha_reference_4c24e0==other.device.alpha_reference_4c24e0;}
};
}
