// Original actor geometry routines execute unchanged with their original helpers.
#include "lemmings/actor_geometry.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>

namespace {
using Bytes=std::array<std::uint8_t,0x3a8>;
template<class T> void put(Bytes& bytes,std::size_t offset,T value) {
    std::memcpy(bytes.data()+offset,&value,sizeof(value));
}
template<class T> T get(const Bytes& bytes,std::size_t offset) {
    T value{};std::memcpy(&value,bytes.data()+offset,sizeof(value));return value;
}
Bytes mapped(const lemmings::ActorGeometryState& actor) {
    Bytes bytes{};
    for(int i=0;i<3;++i) put(bytes,8+8*i,actor.position[i]);
    put(bytes,0x40,actor.direction_40);put(bytes,0x44,actor.direction_44);
    put(bytes,0x4c,actor.category_4c);put(bytes,0x5c,actor.flag_5c);
    for(int i=0;i<2;++i) {
        put(bytes,0x350+8*i,actor.pair_350[i]);
        put(bytes,0x360+8*i,actor.pair_360[i]);
    }
    return bytes;
}
bool same_double(double a,double b) {
    return std::bit_cast<std::uint64_t>(a)==std::bit_cast<std::uint64_t>(b);
}
}
int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=static_cast<std::uint8_t*>(VirtualAlloc(reinterpret_cast<void*>(0x400000),
        0xD0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));
    if(image!=reinterpret_cast<void*>(0x400000)) return 3;
    std::uint32_t blocks=0;
    std::cin.read(reinterpret_cast<char*>(&blocks),4);
    if(!std::cin||blocks>64) return 2;
    for(std::uint32_t i=0;i<blocks;++i) {
        std::uint32_t address=0,size=0;
        std::cin.read(reinterpret_cast<char*>(&address),4);
        std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0xD0000||address-0x400000>0xD0000-size) return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);
        if(!std::cin) return 2;
    }
    DWORD old=0;
    if(!VirtualProtect(image,0xC0000,PAGE_EXECUTE_READ,&old)) return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xC0000);
    using Reverse=void(__attribute__((thiscall))*)(void*);
    using Sample=double*(__attribute__((thiscall))*)(void*,double*);
    using Accept=std::uint8_t(__attribute__((thiscall))*)(void*,std::uint32_t);
    using Contact=std::uint8_t(__attribute__((thiscall))*)(void*);
    using Contains=std::uint8_t(__attribute__((thiscall))*)(void*,double,double,double);
    auto reverse40=reinterpret_cast<Reverse>(0x42f66b);
    auto reverse44=reinterpret_cast<Reverse>(0x42f5e3);
    auto sample=reinterpret_cast<Sample>(0x42f22e);
    auto accept=reinterpret_cast<Accept>(0x42e8c7);
    auto contact=reinterpret_cast<Contact>(0x42e913);
    auto wall_contact=reinterpret_cast<Contact>(0x42eb48);
    auto contains=reinterpret_cast<Contains>(0x42fad7);
    std::array<std::int32_t,8*256> cells{};
    lemmings::CollisionGridHead grid{};
    grid.height=8;grid.cells.data=cells.data();
    std::memcpy(reinterpret_cast<void*>(0x4c6a10),&grid,sizeof(grid));
    std::mt19937_64 rng(0x42e913);
    const double nan=std::numeric_limits<double>::quiet_NaN();
    const double inf=std::numeric_limits<double>::infinity();
    const std::array<double,12> points{-inf,-2,-1,-0.0,0,1,2,256,1e16,inf,nan,1e-300};
    int reversals=0,samples=0,accepts=0,contacts=0,bounds=0,walls=0;
    for(int trial=0;trial<10000;++trial) {
        lemmings::ActorGeometryState actor{};
        for(auto& value:actor.position) value=static_cast<double>(static_cast<int>(rng()%200000)-100000)/100.0;
        for(auto& value:actor.pair_350) value=static_cast<double>(static_cast<int>(rng()%1000)-500)/100.0;
        for(auto& value:actor.pair_360) value=static_cast<double>(static_cast<int>(rng()%1000)-500)/100.0;
        const std::array<std::int32_t,7> dirs{-1,0,1,2,-2,INT32_MIN,INT32_MAX};
        actor.direction_40=dirs[trial%dirs.size()];
        actor.direction_44=dirs[(trial/dirs.size())%dirs.size()];
        actor.category_4c=trial%7-2;
        actor.flag_5c=static_cast<std::uint8_t>(rng());
        Bytes native=mapped(actor);
        auto source=actor;
        reverse40(native.data());lemmings::reverse_actor_direction_40(source);
        if(native!=mapped(source)) {std::cerr<<"direction40 mismatch\n";return 1;}
        reverse44(native.data());lemmings::reverse_actor_direction_44(source);
        if(native!=mapped(source)) {std::cerr<<"direction44 mismatch\n";return 1;}
        reversals+=2;
        native=mapped(actor);
        std::array<double,3> original{};
        if(sample(native.data(),original.data())!=original.data()) return 1;
        const auto recovered=lemmings::actor_sample_position_360(actor);
        for(int i=0;i<3;++i) if(!same_double(original[i],recovered[i])) {
            std::cerr<<"sample mismatch\n";return 1;
        }
        ++samples;
        const std::array<std::uint32_t,10> values{0,1,0x200,0x4000,0x2000,0x201,
            0x4001,0x80,0x1000,0xffffffff};
        for(auto value:values) {
            if((accept(native.data(),value)!=0)!=lemmings::actor_accepts_terrain_value(actor.category_4c,value)) {
                std::cerr<<"terrain acceptance mismatch\n";return 1;
            }
            ++accepts;
        }
        std::array<std::uint8_t,0x18> state{};
        const std::int32_t type=trial%8;
        std::memcpy(state.data()+0x14,&type,4);
        put(native,0x340,state.data());
        for(auto& cell:cells) cell=static_cast<std::int32_t>(values[rng()%values.size()]);
        if((contact(native.data())!=0)!=lemmings::actor_has_terrain_contact(actor,type,grid)) {
            std::cerr<<"terrain contact mismatch "<<trial<<"\n";return 1;
        }
        ++contacts;
        // Large direction_40 values make the original repeated-subtraction X
        // wrapper impractically slow, so use a finite terminating test range.
        auto wall_actor=actor;wall_actor.direction_40=trial%5-2;
        auto wall_native=mapped(wall_actor);
        const auto wall_before=wall_native;
        if ((wall_contact(wall_native.data())!=0)!=lemmings::actor_has_wall_contact(wall_actor,grid) || wall_native!=wall_before) {
            std::cerr<<"wall contact mismatch "<<trial<<"\n";return 1;
        }
        ++walls;
        std::array<double,3> point{actor.position[0]+static_cast<double>(static_cast<int>(rng()%300)-150)/100,
            actor.position[1]+static_cast<double>(static_cast<int>(rng()%300)-150)/100,nan};
        if(trial<static_cast<int>(points.size()*points.size())) {
            actor.position[0]=points[trial/points.size()];
            point[0]=points[trial%points.size()];
            actor.position[1]=0;point[1]=0;
            native=mapped(actor);
        }
        if((contains(native.data(),point[0],point[1],point[2])!=0)!=lemmings::actor_contains_xy_point(actor,point)) {
            std::cerr<<"actor XY mismatch "<<trial<<"\n";return 1;
        }
        ++bounds;
    }
    for(unsigned pattern=0;pattern<256;++pattern)for(int direction:{-1,1}) {
        lemmings::ActorGeometryState actor{};actor.position={255.5,3.25,0};
        actor.direction_40=1;actor.direction_44=direction;cells.fill(0);
        for(unsigned i=0;i<4;++i)cells[(direction==1?3+i:3-i)*256]=std::array<int,4>{0,1,0x80,0x81}[(pattern>>(2*i))&3];
        auto native=mapped(actor);const auto before=native;
        if((wall_contact(native.data())!=0)!=lemmings::actor_has_wall_contact(actor,grid)||native!=before)return 1;
        ++walls;
    }
    std::cout<<"native actor geometry passed "<<reversals<<" reversals, "<<samples
        <<" samples, "<<accepts<<" terrain filters, "<<contacts<<" contacts, "
        <<bounds<<" XY bounds comparisons and "<<walls<<" wall queries\n";
    VirtualFree(image,0,MEM_RELEASE);
}
