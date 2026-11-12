#pragma once
#include "lemmings/terrain_blast.hpp"
#include <bit>
#include <cmath>
#include <vector>

namespace blast_test {
using namespace lemmings;
using Point = std::array<double,3>;
inline std::uint64_t bits(double value) { return std::isnan(value) ? 0x7ff8000000000000ULL : std::bit_cast<std::uint64_t>(value); }
struct Event {
    int kind, object;
    std::array<std::uint64_t,7> values;
    std::uint32_t grid_hash;
    bool operator==(const Event&) const = default;
};
struct Fixture : TerrainBlastCallbacks {
    struct Token { std::uintptr_t vtable=0; std::array<unsigned char,0xb8> rest{}; };
    std::array<Token,64> tokens;
    unsigned next=0, script=0, entities=0;
    bool null_entities=false,valid=true;
    void* last_effect=nullptr;
    Point position{8,8,0.5};
    std::array<std::int32_t,16*256> cells{};
    CollisionGridHead grid{};
    std::vector<Event> trace;
    Fixture(){grid.height=16;grid.cells.data=cells.data();}
    int id(void* p) { if(!p)return -1;for(unsigned i=0;i<tokens.size();++i)if(p==&tokens[i])return static_cast<int>(i);valid=false;return -2; }
    void* token(){return &tokens.at(next++);}
    void event(int kind,void* p=nullptr,Point value={},std::uint64_t extra=0){
        std::uint32_t hash=2166136261u;for(auto cell:cells)hash=(hash^static_cast<std::uint32_t>(cell))*16777619u;
        trace.push_back({kind,id(p),{bits(position[0]),bits(position[1]),bits(position[2]),bits(value[0]),bits(value[1]),bits(value[2]),extra},hash});
    }
    void* create_debris_40c960(const Point& p)override{auto* result=token();last_effect=result;event(1,result,p);if(script&1)position[0]+=0.25;return result;}
    void set_time_40c920(void* p,double value)override{event(2,p,{value,0,0});if(script&2)for(auto& cell:cells)cell^=1;}
    void reset_3c(void* p,std::int32_t value)override{event(3,p,{},value);}
    void set_debris_count_40dbf0(void* p,std::int32_t value)override{event(4,p,{},value);}
    Point world_position_43d56c(const Point& p)override{
        event(5,nullptr,p);if(script&4)position[1]+=0.125;
        // Deliberately nonidentity transform with binary64 stores.
        return {static_cast<double>(static_cast<long double>(p[0])*2+p[2]),
                static_cast<double>(static_cast<long double>(p[1])*3),
                static_cast<double>(static_cast<long double>(p[2])*4-p[0])};
    }
    void set_direction_54(void* p,const Point& value)override{event(6,p,value);}
    void* create_entity_432080(const Point& p,void* effect,std::uint8_t flag)override{
        event(7,effect,p,flag);++entities;if(script&8)position[2]+=0.5;
        return null_entities?nullptr:token();
    }
    void insert_entity_401890(void*& p)override{event(8,p);if(script&16){p=nullptr;cells[0]^=0x400;}}
    void notify_rectangle_43b0a1(std::uint32_t x,std::int32_t y,std::uint32_t ex,std::int32_t ey)override{
        event(9,nullptr,{double(x),double(y),double(ex)},static_cast<std::uint32_t>(ey));if(script&32)position[1]+=0.5;
    }
    void* create_particles_402870()override{auto* result=token();last_effect=result;event(10,result);if(script&64)position[0]-=0.5;return result;}
    void set_color_43fa30(void* p,const Point& color)override{event(11,p,color);}
    void set_particle_count_402ce0(void* p,std::int32_t count)override{event(12,p,{},count);}
    void set_position_08(void* p,const Point& value)override{event(13,p,value);}
    void set_pair_402d80(void* p,double a,double b)override{event(14,p,{a,b,0});}
    void set_flag_403e10(void* p,std::int32_t value)override{event(15,p,{},value);}
    void set_mode_402dc0(void* p,std::int32_t value)override{event(16,p,{},value);}
    void advance_2c(void* p,double dt)override{event(17,p,{dt,0,0});}
};
} // namespace blast_test
