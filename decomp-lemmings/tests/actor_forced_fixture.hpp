#pragma once
#include "lemmings/actor_forced.hpp"
#include "lemmings/game_random.hpp"
#include <bit>
#include <cmath>
#include <cstring>
#include <vector>
namespace forced_test {
using Triple=std::array<double,3>;
struct Trace:lemmings::ActorForcedCallbacks {
    alignas(8) std::array<unsigned char,0x148> effect{};
    alignas(8) std::array<unsigned char,0x48> entity{};
    alignas(8) std::array<unsigned char,0x300> child{};
    void* position=nullptr; void* category=nullptr; double* remaining=nullptr;
    unsigned mutate=0, random_index=0; bool null_entity=false;
    bool concrete_random=false; lemmings::GameRandomState random_state;
    std::array<long double,2> randoms{0.25L,-0.75L};
    std::vector<std::uint64_t> events;
    void number(std::uint64_t n){events.push_back(n);}
    void scalar(double d){number(std::isnan(d)?0x7ff8000000000000ULL:std::bit_cast<std::uint64_t>(d));}
    void triple(const Triple& p){for(auto d:p)scalar(d);}
    void tick(unsigned n){
        number(n);
        if(mutate==n){Triple p{41.25,-2.75,9.5};std::memcpy(position,&p,24);
            std::int32_t c=73;std::memcpy(category,&c,4);*remaining=-0.125;}
    }
    void reset(){events.clear();random_index=0;effect.fill(0xa5);entity.fill(0xa5);child.fill(0xa5);}
    long double random_404080() override {
        if(concrete_random)return lemmings::game_random_signed(random_state);
        auto v=randoms[random_index++];scalar(static_cast<double>(v));tick(random_index);return v;
    }
    void* create_effect_416ed0() override {tick(3);return effect.data();}
    void set_category_d8(void* e,std::int32_t c) override {std::memcpy(static_cast<unsigned char*>(e)+0xd8,&c,4);}
    void set_count_417150(void* e,std::int32_t n) override {std::int32_t c;std::memcpy(&c,static_cast<unsigned char*>(e)+0xd8,4);number(c);number(n);tick(4);}
    Triple world_position_43d56c(const Triple& p) override {triple(p);Triple result{p[0]*2,p[1]*4,p[2]*8};tick(5);return result;}
    void set_direction_54(void*,const Triple& p) override {triple(p);tick(6);}
    void set_position_08(void*,const Triple& p) override {triple(p);tick(7);}
    void set_pair_4171f0(void*,double a,double b) override {scalar(a);scalar(b);tick(8);}
    void set_mode_417230(void*,std::int32_t n) override {number(n);tick(9);}
    void advance_effect_2c(void*,double d) override {scalar(d);tick(10);}
    Triple sample_triple_403f40(std::uint32_t x,std::int32_t y) override {number(x);number(static_cast<std::uint32_t>(y));tick(11);return {17,29,43};}
    void* child_4018b0(void*,std::int32_t i) override {number(i);tick(12);return child.data();}
    void set_child_triple_403820(void*,const Triple& p) override {triple(p);tick(13);}
    void* create_entity_432080(const Triple& p,void*,std::uint8_t f) override {
        if(null_entity)return nullptr;triple(p);number(f);tick(14);return entity.data();}
    void insert_entity_401890(void*& p) override {number(p?1:0);tick(15);p=nullptr;}
};
}
