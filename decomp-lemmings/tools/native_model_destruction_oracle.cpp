#include "lemmings/model_destruction.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

namespace {
using namespace lemmings;
template<class T> void transfer(void* raw,unsigned offset,T& value,bool write) {
    if(write)std::memcpy(static_cast<char*>(raw)+offset,&value,sizeof(value));
    else std::memcpy(&value,static_cast<char*>(raw)+offset,sizeof(value));
}
void project(void* raw,ModelConstructionState& model,bool write) {
    transfer(raw,0,model.original_vtable,write);transfer(raw,4,model.unknown_04,write);
    transfer(raw,8,model.buffer_08,write);transfer(raw,0x88,model.mode_88,write);
    transfer(raw,0x8c,model.unknown_8c,write);transfer(raw,0x90,model.position_90,write);
    transfer(raw,0xa8,model.field_a8,write);transfer(raw,0xac,model.unknown_ac,write);
    auto* p=static_cast<char*>(raw)+0xb0;auto& a=model.animation_b0;
    transfer(p,0,a.quad.field_00,write);transfer(p,4,a.quad.unknown_04,write);
    for(unsigned i=0;i<4;++i) {
        auto* c=p+8+i*0x68;auto& corner=a.quad.corners_08[i];
        transfer(c,0,corner.triple_00,write);transfer(c,0x18,corner.pair_18,write);
        transfer(c,0x28,corner.pair_28,write);transfer(c,0x38,corner.triple_38,write);
        transfer(c,0x50,corner.field_50,write);transfer(c,0x58,corner.unknown_58,write);
    }
    transfer(p,0x1a8,a.field_1a8,write);transfer(p,0x1ac,a.unknown_1ac,write);
    transfer(p,0x1b0,a.field_1b0,write);transfer(p,0x1b4,a.unknown_1b4,write);
    transfer(p,0x1b8,a.field_1b8,write);transfer(p,0x1bc,a.unknown_1bc,write);
    transfer(p,0x1c0,a.playback.fraction_1c0,write);transfer(p,0x1c8,a.playback.clip_1c8,write);
    transfer(p,0x1cc,a.playback.frame_1cc,write);transfer(p,0x1d0,a.playback.direction_1d0,write);
    transfer(p,0x1d4,a.unknown_1d4,write);transfer(p,0x1d8,a.field_1d8,write);transfer(p,0x1e0,a.triple_1e0,write);
    transfer(p,0x1f8,a.playback.resource_1f8,write);transfer(p,0x1fc,a.playback.resource_id_1fc,write);
    transfer(p,0x200,a.playback.finished_200,write);transfer(p,0x201,a.playback.looping_201,write);
    transfer(raw,0x2b2,model.unknown_2b2,write);transfer(raw,0x2b8,model.flag_2b8,write);
    transfer(raw,0x2b9,model.unknown_2b9,write);transfer(raw,0x2c0,model.field_2c0,write);transfer(raw,0x2c4,model.field_2c4,write);
}
struct Event {
    std::int32_t id;std::uint32_t vtable;std::int32_t stored_id;
    std::uintptr_t resource;std::int32_t mode;
    bool operator==(const Event&)const=default;
};
struct Effects {
    ModelConstructionState* model=nullptr;unsigned script=0;std::vector<Event> events;
    static void release(void* context,std::int32_t id) {
        auto& self=*static_cast<Effects*>(context);auto& m=*self.model;auto& p=m.animation_b0.playback;
        self.events.push_back({id,m.original_vtable,p.resource_id_1fc,reinterpret_cast<std::uintptr_t>(p.resource_1f8),m.mode_88});
        if(self.script&1) {
            m.original_vtable=0x12345678;m.mode_88=99;m.buffer_08[17]=0x55;m.position_90[1]=0x7ff0000000000001ULL;
            p.resource_id_1fc=73;p.resource_1f8=reinterpret_cast<const AnimationResource*>(0x70000000);
        }
        if(self.script&2) {m.animation_b0.quad.corners_08[2].pair_18[0]=0x9876543210ULL;m.field_2c4=45;}
    }
};
void* native_model;
Effects* native_effects;
bool valid;
void __attribute__((thiscall)) release_adapter(void* manager,std::int32_t id) {
    valid &= manager==reinterpret_cast<void*>(0x58d23c);
    ModelConstructionState model;project(native_model,model,false);native_effects->model=&model;
    Effects::release(native_effects,id);project(native_model,model,true);
}
void jump(unsigned address,void* target) {
    const std::uint8_t opcode=0xe9;std::memcpy(reinterpret_cast<void*>(address),&opcode,1);
    const auto offset=static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-address-5);
    std::memcpy(reinterpret_cast<void*>(address+1),&offset,4);
}
}

int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xc0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);if(!std::cin || blocks>32)return 2;
    for(unsigned i=0;i<blocks;++i) {
        unsigned a=0,n=0;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);
        if(a<0x400000 || n>0xc0000 || a-0x400000>0xc0000-n)return 2;
        std::cin.read(reinterpret_cast<char*>(a),n);if(!std::cin)return 2;
    }
    jump(0x430050,reinterpret_cast<void*>(&release_adapter));
    DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Destructor=void(__attribute__((thiscall))*)(void*);
    const std::array<unsigned,4> addresses{0x468255,0x46820d,0x4658b7,0x43ce88};
    std::mt19937_64 random(0x43ce88);
    for(unsigned trial=0;trial<20000;++trial) {
        const auto mode=trial%4;
        std::array<std::uint8_t,0x2c8> raw;for(auto& b:raw)b=static_cast<std::uint8_t>(random());
        ModelConstructionState source;project(raw.data(),source,false);
        source.animation_b0.playback.resource_id_1fc=std::array<std::int32_t,5>{-1,0,-2147483647-1,2147483647,static_cast<int>(random())}[(trial/4)%5];
        source.animation_b0.playback.resource_1f8=(trial/20)%2?reinterpret_cast<const AnimationResource*>(0x98760000):nullptr;
        project(raw.data(),source,true);auto expected=raw;
        Effects native;native.script=(trial/40)%4;native_effects=&native;native_model=raw.data();valid=true;
        reinterpret_cast<Destructor>(addresses[mode])(raw.data()+(mode<2?0xb0:0));
        Effects effects;effects.model=&source;effects.script=native.script;
        const AnimationResourceHooks hooks{&effects,nullptr,Effects::release};
        switch(mode) {
            case 0:release_animation_model_resource(source.animation_b0.playback,hooks);break;
            case 1:destroy_animation_model(source.animation_b0,hooks);break;
            case 2:destroy_model_base(source);break;
            case 3:destroy_actor_model(source,hooks);break;
        }
        project(expected.data(),source,true);
        if(!valid || native.events!=effects.events || raw!=expected) {
            std::cerr<<"model destruction mismatch "<<trial<<" mode="<<mode<<'\n';
            for(unsigned i=0;i<raw.size();++i)if(raw[i]!=expected[i]){std::cerr<<"offset "<<std::hex<<i<<'\n';break;}
            return 1;
        }
    }
    std::cout<<"native model destruction passed 20000 release traces and complete buffers across four entry points\n";
    VirtualFree(image,0,MEM_RELEASE);
}
