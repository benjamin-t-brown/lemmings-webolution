#include "lemmings/actor_gadget_contact.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
using namespace lemmings;
namespace {
template<class T> T get(const void* object, unsigned offset) {
    T value; std::memcpy(&value, static_cast<const char*>(object)+offset,sizeof(value)); return value;
}
template<class T> void put(void* object, unsigned offset, T value) {
    std::memcpy(static_cast<char*>(object)+offset,&value,sizeof(value));
}
std::uint64_t bits(double value) { return std::bit_cast<std::uint64_t>(value); }
}
int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xD0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if (image!=reinterpret_cast<void*>(0x400000)) return 3;
    std::uint32_t blocks=0;
    std::cin.read(reinterpret_cast<char*>(&blocks),4);
    if (!std::cin || blocks>64) return 2;
    for (unsigned i=0;i<blocks;++i) {
        std::uint32_t address,length;
        std::cin.read(reinterpret_cast<char*>(&address),4); std::cin.read(reinterpret_cast<char*>(&length),4);
        if (address<0x400000 || length>0xD0000 || address-0x400000>0xD0000-length) return 2;
        std::cin.read(reinterpret_cast<char*>(address),length); if (!std::cin) return 2;
    }
    DWORD previous;
    if (!VirtualProtect(image,0xC0000,PAGE_EXECUTE_READ,&previous)) return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xC0000);
    using Contact=std::uint8_t (__attribute__((thiscall)) *)(void*,void*);
    using Height=long double (__attribute__((thiscall)) *)(void*,double);
    const auto contact=reinterpret_cast<Contact>(0x42C78F);
    const auto height=reinterpret_cast<Height>(0x42C714);
    CollisionGridHead grid{}; grid.height=32;
    std::memcpy(reinterpret_cast<void*>(0x4C6A10),&grid,sizeof(grid));
    unsigned cases=0;
    auto compare = [&](ActorGeometryState actor,const GadgetBoundsHead& bounds,const GadgetBoundsHead& contact_bounds,
                       const std::array<std::array<double,3>,6>& points) {
        std::array<std::uint8_t,0x160> receiver{};
        std::memcpy(receiver.data(),&bounds,sizeof(bounds));
        std::array<std::uint8_t,0x130> model{};
        put(model.data(),0x124,points.data()); put(receiver.data(),0x158,model.data());
        std::array<std::uint8_t,0x3A8> original{};
        for (unsigned i=0;i<3;++i) put(original.data(),8+8*i,actor.position[i]);
        for (unsigned i=0;i<2;++i) put(original.data(),0x360+8*i,actor.pair_360[i]);
        put(original.data(),0x44,actor.direction_44); put(original.data(),0x54,&contact_bounds);
        const auto before=original; const auto receiver_before=receiver; const auto model_before=model;
        const auto bounds_before=std::bit_cast<std::array<std::byte,sizeof(GadgetBoundsHead)>>(contact_bounds);
        const auto points_before=std::bit_cast<std::array<std::byte,sizeof(points)>>(points);
        const GadgetSurfaceView view{&bounds,points};
        const auto native_height=height(receiver.data(),actor.position[0]);
        const auto source_height=gadget_surface_y(view,actor.position[0]);
        // Compare the 80 meaningful x87 bits, excluding ABI padding.
        if (std::memcmp(&native_height,&source_height,10)!=0) {
            std::cerr << "slope mismatch " << cases << '\n'; return false;
        }
        const auto native_result=contact(receiver.data(),original.data());
        const auto source_result=adjust_actor_gadget_contact(view,contact_bounds,actor,grid);
        ++cases;
        if (native_result!=source_result || bits(get<double>(original.data(),0x10))!=bits(actor.position[1])) {
            std::cerr << "contact mismatch " << cases << " native=" << static_cast<int>(native_result)
                      << " source=" << source_result << '\n'; return false;
        }
        put(original.data(),0x10,get<double>(before.data(),0x10));
        return original==before && receiver==receiver_before && model==model_before &&
            std::bit_cast<std::array<std::byte,sizeof(GadgetBoundsHead)>>(contact_bounds)==bounds_before &&
            std::bit_cast<std::array<std::byte,sizeof(points)>>(points)==points_before;
    };
    const double nan=std::numeric_limits<double>::quiet_NaN(), inf=std::numeric_limits<double>::infinity();
    ActorGeometryState actor{};
    GadgetBoundsHead bounds{},contact_bounds{};
    std::array<std::array<double,3>,6> points{}; points[4][1]=8; points[5][1]=2;
    bounds.minimum[0]=2; bounds.maximum[0]=6;
    contact_bounds.minimum[0]=0;contact_bounds.maximum[0]=8;
    contact_bounds.minimum[1]=3;contact_bounds.maximum[1]=4;
    contact_bounds.minimum[2]=0;contact_bounds.maximum[2]=1;
    for (double x : {-inf,-1.0,0.0,2.0,3.0,6.0,8.0,9.0,inf,nan})
        for (double y : {-inf,2.0,3.0,4.0,5.0,inf,nan})
            for (int direction : {-1,0,1,2}) {
                actor.position={x,y,0.5};actor.direction_44=direction;
                if (!compare(actor,bounds,contact_bounds,points)) return 1;
            }
    actor.position={3,3.5,0.5}; actor.direction_44=1;
    for (double first : {-inf,-1.0,0.0,1.0,inf,nan})
        for (double second : {-inf,-1.0,0.0,1.0,inf,nan}) {
            bounds.minimum[0]=first;bounds.maximum[0]=second;
            if (!compare(actor,bounds,contact_bounds,points)) return 1;
            points[4][1]=first;points[5][1]=second;
            if (!compare(actor,bounds,contact_bounds,points)) return 1;
        }
    std::mt19937_64 random(0x42C78F);
    const auto value=[&] { return static_cast<double>(static_cast<int>(random()%10000)-2500)/100.0; };
    for (unsigned i=0;i<20000;++i) {
        for (unsigned axis=0;axis<3;++axis) {
            bounds.minimum[axis]=value();bounds.maximum[axis]=value();
            contact_bounds.minimum[axis]=value();contact_bounds.maximum[axis]=value();
            actor.position[axis]=value();
            if (i&1) { contact_bounds.minimum[axis]=actor.position[axis]-3;contact_bounds.maximum[axis]=actor.position[axis]+3; }
        }
        for (auto& point:points) for (auto& coordinate:point) coordinate=value();
        actor.pair_360={value()/100,value()/100};actor.direction_44=static_cast<int>(random()%5)-2;
        if (!compare(actor,bounds,contact_bounds,points)) return 1;
    }
    std::cout << "native gadget contact passed " << cases << " full-chain and extended-slope comparisons\n";
    VirtualFree(image,0,MEM_RELEASE);
}
