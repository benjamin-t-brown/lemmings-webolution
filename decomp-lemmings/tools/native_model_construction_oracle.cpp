#include "lemmings/model_construction.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cstring>
#include <iostream>
#include <random>

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
    // No entry points or call sites are patched. The CRT array-constructor
    // helper, all four empty corner constructors, and success cleanup run.
    DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Constructor=void*(__attribute__((thiscall))*)(void*);
    const std::array<unsigned,5> addresses{0x440a10,0x4409e0,0x4680f0,0x465840,0x43ce00};
    std::mt19937_64 random(0x43ce00);
    for(unsigned trial=0;trial<20000;++trial) {
        const unsigned mode=trial%5;
        std::array<std::uint8_t,0x2c8> raw;
        for(auto& byte:raw)byte=static_cast<std::uint8_t>(random());
        // Deliberately place signaling NaNs in preserved model position and
        // untouched pair members; projection copies their representations.
        if(trial<5) {
            const std::uint64_t snan=0x7ff0000000000001ULL;
            std::memcpy(raw.data()+0x90,&snan,8);std::memcpy(raw.data()+0xb0+8+0x18,&snan,8);
        }
        auto expected=raw;ModelConstructionState source;project(raw.data(),source,false);
        void* argument=raw.data()+(mode<3?0xb0:0);
        auto* returned=reinterpret_cast<Constructor>(addresses[mode])(argument);
        switch(mode) {
            case 0:initialize_model_quad(source.animation_b0.quad);break;
            case 1:initialize_animation_base(source.animation_b0);break;
            case 2:initialize_animation_model(source.animation_b0);break;
            case 3:initialize_model_base(source);break;
            case 4:initialize_actor_model(source);break;
        }
        project(expected.data(),source,true);
        if(returned!=argument || raw!=expected) {
            std::cerr<<"model construction mismatch "<<trial<<" mode="<<mode<<'\n';
            for(unsigned i=0;i<raw.size();++i)if(raw[i]!=expected[i]){std::cerr<<"offset "<<std::hex<<i<<'\n';break;}
            return 1;
        }
    }
    std::cout<<"native model construction passed 20000 complete-buffer comparisons across five entry points, without adapters\n";
    VirtualFree(image,0,MEM_RELEASE);
}
