#pragma once
#include "lemmings/sprite_render_clip.hpp"
#include <vector>
#include <bit>
#include <cmath>
#include <cstring>
#include <cstdlib>
namespace sprite_clip_test {
using namespace lemmings;
inline std::uint64_t number(double d){return std::isnan(d)?0x7ff8000000000000ull:std::bit_cast<std::uint64_t>(d);}
inline double get(const SpriteRenderVertex& v,unsigned i){double d;std::memcpy(&d,reinterpret_cast<const std::byte*>(&v)+8*i,8);return d;}
inline void put(SpriteRenderVertex& v,unsigned i,double d){std::memcpy(reinterpret_cast<std::byte*>(&v)+8*i,&d,8);}
struct Fixture {
    unsigned trial,allocated=0,draws=0;std::uint64_t rng;
    SpriteRenderVertex vertices[12],pool[256];
    SpriteRenderVertex* input_buffer[128]{};SpriteRenderVertex* temporary_buffer[128]{};SpriteRenderVertex* free_buffer[8]{};
    SpriteRenderVertex* storage[128][128]{};
    SpriteVertexPointerArray input;SpriteClipState state;
    std::vector<std::vector<std::uint64_t>> trace;
    void(*before)(Fixture&)=nullptr;void(*after)(Fixture&)=nullptr;
    std::uint64_t random(){rng^=rng<<13;rng^=rng>>7;rng^=rng<<17;return rng;}
    explicit Fixture(unsigned i):trial(i),rng(0x3bd920193cde987full+i){
        std::memset(pool,0xa5,sizeof(pool));
        for(unsigned j=0;j<12;++j){for(unsigned k=0;k<13;++k)put(vertices[j],k,static_cast<std::int32_t>(random())/65536.0);put(vertices[j],2,(random()%100+1)/16.0);vertices[j].x_18=static_cast<int>(random()%65)-32;vertices[j].y_20=static_cast<int>(random()%65)-32;
            if(i%8==1){vertices[j].x_18/=8;vertices[j].y_20/=8;}
            if(i%8==2)vertices[j].y_20=100;
            if(i%8==3)vertices[j].y_20=-100;
            if(i%8==4)vertices[j].x_18=100;
            if(i%8==5)vertices[j].x_18=-100;
            if(i%8==6){if(j%3==0)vertices[j].x_18=NAN;if(j%4==0)vertices[j].y_20=NAN;}
            if(i%8==7){put(vertices[j],2,j%2?0:-0.0);if(j%4==0)vertices[j].y_20=INFINITY;}
            input_buffer[j]=&vertices[j];
        }
        for(unsigned j=0;j<8;++j)free_buffer[j]=pool+j;
        input={static_cast<int>(i%12)+1,static_cast<int>(i%12)+1,static_cast<int>((i/12)%4)+1,input_buffer};
        state={-8,8,8,-8,{0,static_cast<int>((i/48)%5),static_cast<int>((i/240)%4)+1,temporary_buffer},{{0xaaaa,0xbbbb,0xcccc},pool,{static_cast<int>((i/960)%9),8,1,free_buffer},8}};
    }
    unsigned pointer(const void* p)const{if(!p)return 0;for(unsigned j=0;j<12;++j)if(p==vertices+j)return j+1;for(unsigned j=0;j<256;++j)if(p==pool+j)return j+100;if(p==input_buffer)return 500;if(p==temporary_buffer)return 501;if(p==free_buffer)return 502;for(unsigned j=0;j<128;++j)if(p==storage[j])return 1000+j;std::abort();}
    std::vector<std::uint64_t> snapshot()const{
        const auto& s=state;std::vector<std::uint64_t> r={static_cast<std::uint32_t>(s.minimum_x_58d280),static_cast<std::uint32_t>(s.maximum_x_58d284),static_cast<std::uint32_t>(s.maximum_y_58d288),static_cast<std::uint32_t>(s.minimum_y_58d28c),s.scratch_58d578.used_20,static_cast<std::uint32_t>(s.scratch_58d578.available_10.count_00),allocated,draws};
        for(auto* a:{&input,&s.temporary_58d558}){r.push_back(static_cast<std::uint32_t>(a->count_00));r.push_back(static_cast<std::uint32_t>(a->capacity_04));r.push_back(static_cast<std::uint32_t>(a->growth_08));r.push_back(pointer(a->data_0c));for(int i=0;i<a->count_00;++i)r.push_back(pointer(a->data_0c[i]));}
        for(unsigned j=0;j<s.scratch_58d578.used_20;++j)for(unsigned k=0;k<11;++k)r.push_back(number(get(pool[j],k)));return r;
    }
    void event(unsigned kind,unsigned value=0){if(before)before(*this);auto r=snapshot();r.insert(r.begin(),{kind,value});trace.push_back(std::move(r));
        if(kind==1&&trial%17==0)state.minimum_y_58d28c=-4;
        if(kind==2&&trial%19==0)state.maximum_x_58d284=4;
        if(kind==3&&trial%23==0)state.minimum_x_58d280=-3;
        if(after)after(*this);
    }
    SpriteClipHooks hooks(){return {this,[](void* p,std::uint32_t bytes){auto& f=*static_cast<Fixture*>(p);f.event(1,bytes);if(bytes>512||f.allocated>=128)std::abort();return f.storage[f.allocated++];},[](void* p,SpriteRenderVertex** v){auto& f=*static_cast<Fixture*>(p);f.event(2,f.pointer(v));},[](void* p,SpriteVertexPointerArray& v){auto& f=*static_cast<Fixture*>(p);if(&v!=&f.input)std::abort();++f.draws;f.event(3);}};}
    void run(){clip_and_draw_sprite_polygon(input,state,hooks());}
    bool same(const Fixture& f)const{return trace==f.trace&&snapshot()==f.snapshot()&&std::memcmp(vertices,f.vertices,sizeof(vertices))==0;}
};
}
