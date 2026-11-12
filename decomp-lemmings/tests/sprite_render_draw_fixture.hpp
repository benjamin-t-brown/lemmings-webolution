#pragma once
#include "lemmings/sprite_render_draw.hpp"
#include <vector>
#include <bit>
#include <cmath>
#include <cstring>
#include <cstdlib>
namespace sprite_draw_test {
using namespace lemmings;
inline std::uint32_t float_bits(float f){return std::isnan(f)?0x7fc00000:std::bit_cast<std::uint32_t>(f);}
struct Fixture {
    unsigned trial,script;std::uint64_t rng;
    SpriteRenderVertex vertices[12]{};SpriteRenderVertex* pointers[12]{};
    SpriteVertexPointerArray input{};SpriteDeviceState device{};SpriteDrawState state{};
    SpriteDeviceVertex old_buffer[16],new_buffer[16],alternate_buffer[16];
    std::vector<std::vector<std::uint64_t>> trace;
    void(*before)(Fixture&)=nullptr;void(*after)(Fixture&)=nullptr;
    std::uint64_t random(){rng^=rng<<13;rng^=rng>>7;rng^=rng<<17;return rng;}
    static void put(SpriteRenderVertex& v,unsigned off,double d){std::memcpy(reinterpret_cast<std::byte*>(&v)+off,&d,8);}
    explicit Fixture(unsigned i):trial(i),script((i/24)%8),rng(0xd0e89d2d311e2899ull+i){
        std::memset(old_buffer,0xa5,sizeof(old_buffer));std::memset(new_buffer,0x5a,sizeof(new_buffer));std::memset(alternate_buffer,0x3c,sizeof(alternate_buffer));
        for(unsigned j=0;j<12;++j){pointers[j]=&vertices[j];for(unsigned k=0;k<13;++k){auto bits=random();std::memcpy(reinterpret_cast<std::byte*>(&vertices[j])+k*8,&bits,8);}
            for(unsigned off:{0x10u,0x18u,0x20u,0x28u,0x30u}){double d=static_cast<std::int32_t>(random())/1024.0;if(i%4==1)d=std::bit_cast<double>(random());if(i%4==2){const double edges[]={0.0,-0.0,1e-320,1e300,-1e300,INFINITY,-INFINITY,NAN};d=edges[(j+off/8)%8];}put(vertices[j],off,d);}
            for(unsigned off:{0x38u,0x40u,0x48u,0x50u}){double d=(random()%1024)/2.0-128.75;if(i%4==3)d=std::bit_cast<double>(random());put(vertices[j],off,d);}
        }
        input={static_cast<int>(i%12),12,1,pointers};device.device_58cdb8=reinterpret_cast<void*>(0x1100);
        state={{2,2,static_cast<int>((i/192)%3)+1,old_buffer},(i/12)%2?0x123456a5u:0x123456a4u,static_cast<double>(static_cast<int>(i%17)-8)/4096,static_cast<double>(static_cast<int>(i%19)-9)/2048};
    }
    unsigned pointer(const void* p)const{if(!p)return 0;if(p==old_buffer)return 1;if(p==new_buffer)return 2;if(p==alternate_buffer)return 3;return static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(p));}
    std::vector<std::uint64_t> snapshot() const {
        auto& a=state.vertices_58d540;std::vector<std::uint64_t> r={pointer(device.device_58cdb8),state.initialized_flags_58d550,static_cast<std::uint32_t>(a.count_00),static_cast<std::uint32_t>(a.capacity_04),static_cast<std::uint32_t>(a.growth_08),pointer(a.data_0c),static_cast<std::uint32_t>(input.count_00),std::bit_cast<std::uint64_t>(state.u_offset_58d2a0),std::bit_cast<std::uint64_t>(state.v_offset_58d2a8)};
        for(auto* b:{old_buffer,new_buffer,alternate_buffer})for(unsigned j=0;j<16;++j){auto& v=b[j];for(auto n:{float_bits(v.x_00),float_bits(v.y_04),float_bits(v.z_08),float_bits(v.rhw_0c),v.color_10,v.unknown_14,float_bits(v.u_18),float_bits(v.v_1c)})r.push_back(n);}return r;
    }
    void event(unsigned kind,unsigned value=0){if(before)before(*this);auto r=snapshot();r.insert(r.begin(),{kind,value});trace.push_back(r);auto& a=state.vertices_58d540;
        if(kind==1&&script==1)a={3,14,1,alternate_buffer};
        if(kind==1&&script==2)input.count_00=7;
        if(kind==2&&script==3){a.count_00=1;a.data_0c=alternate_buffer;}
        if(kind==2&&script==4){input.count_00=2;state.u_offset_58d2a0=0.125;device.device_58cdb8=reinterpret_cast<void*>(0x2200);}
        if(kind==3&&script==5){a.count_00=0;a.capacity_04=15;a.data_0c=old_buffer;}
        if(kind==4&&script==6){a.count_00=11;state.initialized_flags_58d550=0xfedcba98;}
        if(kind==2&&script==7)state.v_offset_58d2a8=-0.25;
        if(after)after(*this);
    }
    SpriteDrawHooks hooks(){return {this,
        [](void* p,std::uint32_t bytes){auto& f=*static_cast<Fixture*>(p);f.event(2,bytes);if(bytes>512)std::abort();return f.new_buffer;},
        [](void* p,SpriteDeviceVertex* v){auto& f=*static_cast<Fixture*>(p);f.event(3,f.pointer(v));},
        [](void* p){static_cast<Fixture*>(p)->event(1);},
        [](void* p,void* d,std::uint32_t primitive,std::uint32_t format,SpriteDeviceVertex* v,std::uint32_t count,std::uint32_t flags){auto& f=*static_cast<Fixture*>(p);if(d!=f.device.device_58cdb8||primitive!=6||format!=0x1c4||flags||v!=f.state.vertices_58d540.data_0c)std::abort();f.event(4,count);}};}
    void run(){draw_sprite_polygon(input,device,state,hooks());}
    bool same(const Fixture& f)const{return trace==f.trace&&snapshot()==f.snapshot()&&std::memcmp(vertices,f.vertices,sizeof(vertices))==0;}
};
}
