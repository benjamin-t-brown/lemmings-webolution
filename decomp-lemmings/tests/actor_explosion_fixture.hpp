#pragma once
#include "lemmings/actor_explosion.hpp"
#include <bit>
#include <cmath>
#include <cstring>
#include <vector>
namespace explosion_test {
using Triple=std::array<double,3>;
struct Trace:lemmings::ActorExplosionCallbacks{
    alignas(8) std::array<unsigned char,0x128> effect{};
    alignas(8) std::array<unsigned char,0x48> entity{};
    std::array<std::array<unsigned char,0x200>,8> children{};
    void* position=nullptr;void* category=nullptr;double* remaining=nullptr;
    unsigned mutation=0;int child_total=6;bool null_entity=false;
    std::vector<std::uint64_t> events;
    void n(std::uint64_t v){events.push_back(v);}void d(double v){n(std::isnan(v)?0x7ff8000000000000ULL:std::bit_cast<std::uint64_t>(v));}
    void p(const Triple& v){for(auto x:v)d(x);}
    void tick(unsigned stage){n(stage);if(mutation==stage){Triple value{77.25,-3.5,2};std::memcpy(position,&value,24);int c=3;std::memcpy(category,&c,4);*remaining=-.125;}}
    void event_44535b(lemmings::TerminalActor&,std::int16_t id,std::int32_t e,std::uint8_t f)override{n(static_cast<std::uint32_t>(id));n(e);n(f);tick(1);}
    void nearby_effect_41dd86(Triple& v,double r)override{p(v);d(r);tick(2);v={90,91,92};}
    void world_notice_43b31e(std::int32_t n0,Triple& v)override{n(n0);p(v);tick(3);if(mutation==3)v[0]=14.25;}
    void terrain_blast_43f510(Triple& v,std::int32_t n0,double elapsed)override{p(v);n(n0);d(elapsed);tick(4);if(mutation==4)v[1]=-6.75;}
    void* create_effect(lemmings::TerminalEffectKind k)override{n(static_cast<unsigned>(k));tick(5);return effect.data();}
    void set_count(void*,std::int32_t count)override{n(count);tick(6);}
    void set_position_08(void*,const Triple& v)override{p(v);tick(7);}
    void set_pair(void*,double a,double b)override{d(a);d(b);tick(8);}
    void set_mode(void*,std::int32_t mode)override{n(mode);tick(9);}
    Triple world_position_43d56c(const Triple& v)override{p(v);Triple out{v[0]*2,v[1]*4,v[2]*8};tick(10);return out;}
    void set_direction_54(void*,const Triple& v)override{p(v);tick(11);}
    void advance_effect_2c(void*,double elapsed)override{d(elapsed);tick(12);}
    Triple sample_triple_403f40(std::uint32_t x,std::int32_t y)override{n(x);n(static_cast<std::uint32_t>(y));tick(13);return{13,19,23};}
    std::int32_t child_count_4018d0(void*)override{tick(14);return child_total;}
    void* child_4018b0(void*,std::int32_t i)override{n(i);tick(15);if(mutation==15)child_total=1;return children.at(i).data();}
    void set_child_triple_403820(void*,const Triple& v)override{p(v);tick(16);}
    void set_triple_4017d0(void*,const Triple&)override{n(0xbad);}
    void* create_entity_432080(const Triple& v,void*,std::uint8_t f)override{if(null_entity)return nullptr;p(v);n(f);tick(17);return entity.data();}
    void insert_entity_401890(void*& v)override{n(v?1:0);tick(18);v=nullptr;}
};
}
