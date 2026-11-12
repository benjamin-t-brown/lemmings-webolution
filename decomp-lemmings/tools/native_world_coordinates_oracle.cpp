#include "lemmings/world_coordinates.hpp"
#include "lemmings/compiled_level.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <bit>
#include <cmath>
#include <cstring>
#include <iostream>
#include <random>

namespace {
using Point=std::array<double,3>;
using Map=double*(__attribute__((thiscall))*)(void*,double*,const double*);
using Depth=double(__attribute__((thiscall))*)(void*,const double*,double);
auto original_map=reinterpret_cast<Map>(0x43d56c);
auto original_depth=reinterpret_cast<Depth>(0x43da38);
bool same(double a,double b) {
    return (std::isnan(a)&&std::isnan(b)) ||
        std::bit_cast<std::uint64_t>(a)==std::bit_cast<std::uint64_t>(b);
}
bool check(const lemmings::WorldGeometryPrefix& world,const Point& point,double new_y,
           int trial) {
    const auto mesh=lemmings::world_coordinate_mesh(world);
    std::array<std::uint8_t,0x30> object{};
    auto* rows=world.first_blocks.data();
    std::memcpy(object.data()+0x24,&rows,4);
    std::memcpy(object.data()+0x28,&mesh.height,4);
    Point native{};
    if(original_map(object.data(),native.data(),point.data())!=native.data()) return false;
    const auto source=lemmings::map_world_coordinates(mesh,point);
    for(int j=0;j<3;++j) if(!same(native[j],source[j])) {
        std::cerr<<"map mismatch trial "<<trial<<" component "<<j<<" "<<std::hexfloat
            <<native[j]<<" != "<<source[j]<<"\n";return false;
    }
    const double nd=original_depth(object.data(),point.data(),new_y);
    const double sd=lemmings::world_depth_at_y(mesh,point,new_y);
    if(!same(nd,sd)) {
        std::cerr<<"depth mismatch trial "<<trial<<" "<<std::hexfloat
            <<nd<<" != "<<sd<<"\n";return false;
    }
    return true;
}
}
int main(int argc,char** argv) {
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=static_cast<std::uint8_t*>(VirtualAlloc(reinterpret_cast<void*>(0x400000),
        0xC0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));
    if(image!=reinterpret_cast<void*>(0x400000)) return 3;
    std::uint32_t blocks=0;
    std::cin.read(reinterpret_cast<char*>(&blocks),4);
    if(!std::cin||blocks>64) return 2;
    for(std::uint32_t i=0;i<blocks;++i) {
        std::uint32_t address=0,size=0;
        std::cin.read(reinterpret_cast<char*>(&address),4);
        std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0xC0000||address-0x400000>0xC0000-size) return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);
        if(!std::cin) return 2;
    }
    DWORD old=0;
    if(!VirtualProtect(image,0xC0000,PAGE_EXECUTE_READ,&old)) return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xC0000);
    std::mt19937_64 rng(0x43d56c);
    lemmings::WorldGeometryPrefix world;
    world.leading_value=128;world.first_blocks.resize(8);
    for(int trial=0;trial<20000;++trial) {
        if(trial%100==0) for(auto& row:world.first_blocks) for(auto& point:row)
            for(auto& bits:point) bits=std::bit_cast<std::uint64_t>(
                static_cast<double>(static_cast<int>(rng()%200000)-100000)/317.0);
        Point p{static_cast<double>(static_cast<int>(rng()%300000)-150000)/271.0,
                static_cast<double>(rng()%140000)/1000.0,
                static_cast<double>(static_cast<int>(rng()%5000)-2000)/1000.0};
        double y=static_cast<double>(rng()%140000)/1000.0;
        if(trial<512) {
            p[0]=static_cast<double>(trial)-128.0;
            p[1]=(trial%10)*16.0;
            p[2]=(trial%3)*0.5;
            y=(trial%9)*16.0;
        }
        if(!check(world,p,y,trial)) return 1;
    }
    int installed=0,levels=0;
    for(int arg=1;arg<argc;++arg) {
        const auto archive=lemmings::read_box_archive_index(argv[arg]);
        for(std::size_t i=0;i<archive.entries.size();++i) {
            if(!archive.entries[i].name.ends_with(".LVL")) continue;
            const auto level=lemmings::read_compiled_level(archive.read_entry(i));
            const auto height=static_cast<double>(level.world.leading_value);
            for(int sample=0;sample<100;++sample) {
                Point p{static_cast<double>(rng()%256000)/1000.0,
                    static_cast<double>(rng()%100001)/100000.0*height,
                    static_cast<double>(rng()%1001)/1000.0};
                const double y=static_cast<double>(rng()%100001)/100000.0*height;
                if(!check(level.world,p,y,installed)) {
                    std::cerr<<archive.entries[i].name<<"\n";return 1;
                }
                ++installed;
            }
            ++levels;
        }
    }
    std::cout<<"20000 synthetic coordinate/depth pairs; "<<installed
        <<" installed pairs across "<<levels<<" levels match unchanged originals\n";
}
