#include "lemmings/actor_walking.hpp"
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
#include <vector>
using namespace lemmings;
namespace {
template<class T> T get(const void* object, unsigned offset) {
    T value; std::memcpy(&value, static_cast<const char*>(object) + offset, sizeof(value)); return value;
}
template<class T> void put(void* object, unsigned offset, T value) {
    std::memcpy(static_cast<char*>(object) + offset, &value, sizeof(value));
}
struct Event {
    unsigned kind, gadget;
    std::array<std::uint64_t,3> point;
    bool operator==(const Event&) const = default;
};
std::vector<Event> trace;
std::array<void*,4> gadgets{reinterpret_cast<void*>(1), reinterpret_cast<void*>(2),
                            reinterpret_cast<void*>(3), reinterpret_cast<void*>(4)};
unsigned script, calls;
bool valid;
int count() {
    trace.push_back({1,0,{}});
    return static_cast<int>(script % 5);
}
void* at(int index) {
    trace.push_back({2,static_cast<unsigned>(index),{}});
    return gadgets[index];
}
std::uint8_t contains(void* gadget, const std::array<double,3>& point) {
    trace.push_back({3,static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(gadget)),
                    {std::bit_cast<std::uint64_t>(point[0]), std::bit_cast<std::uint64_t>(point[1]),
                     std::bit_cast<std::uint64_t>(point[2])}});
    ++calls;
    return calls == script % 11 ? 0x80 : 0;
}
int __attribute__((thiscall)) native_count(void* list) {
    valid &= list == reinterpret_cast<void*>(0x4C8978); return count();
}
void** __attribute__((thiscall)) native_at(void* list, int index) {
    valid &= list == reinterpret_cast<void*>(0x4C8978); at(index); return &gadgets[index];
}
std::uint32_t __attribute__((thiscall)) native_contains(void* gadget, const std::array<double,3>* point) {
    return 0xFFFFFF00u | contains(gadget, *point);
}
void jump(std::uintptr_t address, std::uintptr_t target) {
    put(reinterpret_cast<void*>(address), 0, std::uint8_t{0xE9});
    put(reinterpret_cast<void*>(address), 1, static_cast<std::uint32_t>(target - address - 5));
}
ActorWalkingSurfaceHooks hooks() {
    ActorWalkingSurfaceHooks value{};
    value.gadget_count = [](void*) { return count(); };
    value.gadget_at = [](void*, int index) { return at(index); };
    value.gadget_contains = [](void*, void* gadget, const std::array<double,3>& point) { return contains(gadget,point); };
    return value;
}
}
int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    auto* image = VirtualAlloc(reinterpret_cast<void*>(0x400000), 0xD0000,
                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    std::uint32_t blocks = 0;
    std::cin.read(reinterpret_cast<char*>(&blocks), 4);
    if (!std::cin || blocks > 64) return 2;
    for (unsigned i = 0; i < blocks; ++i) {
        std::uint32_t address, length;
        std::cin.read(reinterpret_cast<char*>(&address), 4);
        std::cin.read(reinterpret_cast<char*>(&length), 4);
        if (address < 0x400000 || length > 0xD0000 || address - 0x400000 > 0xD0000 - length) return 2;
        std::cin.read(reinterpret_cast<char*>(address), length);
        if (!std::cin) return 2;
    }
    jump(0x41CB90, reinterpret_cast<std::uintptr_t>(&native_count));
    jump(0x41CB70, reinterpret_cast<std::uintptr_t>(&native_at));
    jump(0x430BA1, reinterpret_cast<std::uintptr_t>(&native_contains));
    DWORD previous;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &previous)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    using Surface = void (__attribute__((thiscall)) *)(void*, void*);
    using Depth = void (__attribute__((thiscall)) *)(void*);
    const auto surface = reinterpret_cast<Surface>(0x41C875);
    const auto depth = reinterpret_cast<Depth>(0x42F977);
    std::array<std::int32_t, 8*256> cells{};
    CollisionGridHead grid{}; grid.height = 8; grid.cells.data = cells.data();
    std::memcpy(reinterpret_cast<void*>(0x4C6A10), &grid, sizeof(grid));
    unsigned cases = 0;
    auto compare_surface = [&](WalkingActor actor) {
        std::array<std::uint8_t, 0x3A8> original{};
        for (unsigned i=0; i<3; ++i) put(original.data(), 8+8*i, actor.geometry.position[i]);
        for (unsigned i=0; i<2; ++i) put(original.data(), 0x360+8*i, actor.geometry.pair_360[i]);
        put(original.data(), 0x40, actor.geometry.direction_40); put(original.data(), 0x44, actor.geometry.direction_44);
        put(original.data(), 0x4A, actor.special_4a); put(original.data(), 0x54, actor.gadget_54); put(original.data(), 0x58, actor.gadget_58);
        const auto before = original;
        trace.clear(); valid = true; calls = 0;
        surface(nullptr, original.data());
        const auto native_trace = trace; const bool native_valid = valid;
        trace.clear(); valid = true; calls = 0;
        track_actor_walking_surface(actor, grid, hooks());
        ++cases;
        if (!native_valid || native_trace != trace || get<std::uint8_t>(original.data(),0x4A) != actor.special_4a ||
            get<void*>(original.data(),0x54) != actor.gadget_54 || get<void*>(original.data(),0x58) != actor.gadget_58) {
            std::cerr << "surface mismatch case=" << cases << " script=" << script << '\n'; return false;
        }
        put(original.data(),0x4A,get<std::uint8_t>(before.data(),0x4A));
        put(original.data(),0x54,get<void*>(before.data(),0x54)); put(original.data(),0x58,get<void*>(before.data(),0x58));
        return original == before;
    };
    WalkingActor actor{}; actor.geometry.position={2.5,3.5,0.9}; actor.special_4a=0x80;
    actor.gadget_58=reinterpret_cast<void*>(99);
    for (script=0; script<220; ++script)
        for (unsigned first : {0u,1u,0x200u,0x2000u,0x4000u,0x8000u})
            for (unsigned second : {0u,0x200u,0x2000u,0x4000u}) {
                cells.fill(0); cells[2*256+3]=first; cells[3*256+3]=second;
                actor.gadget_54 = (script & 1) ? reinterpret_cast<void*>(3) : nullptr;
                if (!compare_surface(actor)) return 1;
            }
    std::mt19937_64 random(0x41C875);
    for (unsigned i=0; i<10000; ++i) {
        script=random()%220;
        for (auto& cell:cells) cell=static_cast<int>(random()&0xFFFF);
        actor.geometry.position={static_cast<double>(static_cast<int>(random()%1600)-300)/4.0,
                                 static_cast<double>(static_cast<int>(random()%80)-20)/4.0,0.9};
        actor.geometry.pair_360={static_cast<double>(static_cast<int>(random()%16)-8)/8.0,
                                 static_cast<double>(static_cast<int>(random()%16)-8)/8.0};
        actor.geometry.direction_40=static_cast<int>(random()%5)-2;
        actor.geometry.direction_44=static_cast<int>(random()%5)-2;
        actor.gadget_54=(random()&1) ? reinterpret_cast<void*>(3) : nullptr;
        if (!compare_surface(actor)) return 1;
    }
    unsigned depth_cases=0;
    auto compare_depth = [&](double z,double target1,double target2,int direction,std::uint8_t enabled) {
        std::array<std::uint8_t,0x80> original{};
        put(original.data(),0x18,z); put(original.data(),0x40,direction); put(original.data(),0x5E,enabled);
        const auto before=original;
        put(reinterpret_cast<void*>(0x4C5B30),0,target1); put(reinterpret_cast<void*>(0x4C5B20),0,target2);
        depth(original.data());
        WalkingActor actor{}; actor.geometry.position[2]=z; actor.geometry.direction_40=direction; actor.depth_adjust_5e=enabled;
        approach_actor_walking_depth(actor,target1,target2); ++depth_cases;
        if (std::bit_cast<std::uint64_t>(get<double>(original.data(),0x18)) != std::bit_cast<std::uint64_t>(actor.geometry.position[2])) {
            std::cerr << "depth mismatch " << depth_cases << " z=" << z << " target=" << target1 << '/' << target2
                      << " direction=" << direction << " native=" << std::hex << std::bit_cast<std::uint64_t>(get<double>(original.data(),0x18))
                      << " source=" << std::bit_cast<std::uint64_t>(actor.geometry.position[2]) << '\n'; return false;
        }
        put(original.data(),0x18,z); return original==before;
    };
    const double nan=std::numeric_limits<double>::quiet_NaN(), inf=std::numeric_limits<double>::infinity();
    for (double z : {-inf,-1.0,-0.02,-0.01,-0.0,0.0,0.01,0.02,1.0,inf,nan})
        for (double target : {-inf,-1.0,-0.0,0.0,1.0,inf,nan})
            for (int direction : {-1,0,1,2})
                for (std::uint8_t enabled : {std::uint8_t{0},std::uint8_t{0x80}})
                    if (!compare_depth(z,target,-target,direction,enabled)) return 1;
    for (unsigned i=0;i<10000;++i)
        if (!compare_depth(static_cast<double>(static_cast<int>(random()%40000)-20000)/10000.0,
                           static_cast<double>(static_cast<int>(random()%40000)-20000)/10000.0,
                           static_cast<double>(static_cast<int>(random()%40000)-20000)/10000.0,
                           static_cast<int>(random()%5)-2,random()%256)) return 1;
    std::cout << "native surface passed " << cases << " trace cases; depth passed " << depth_cases << " bit comparisons\n";
    VirtualFree(image,0,MEM_RELEASE);
}
