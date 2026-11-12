#include "lemmings/game_random.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <cstring>
namespace {
bool load(){std::uint32_t count;std::cin.read(reinterpret_cast<char*>(&count),4);for(unsigned i=0;i<count;++i){std::uint32_t a,n;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);if(a<0x400000||a>0x4f0000||n>0x4f0000-a)return false;std::cin.read(reinterpret_cast<char*>(a),n);}return bool(std::cin);}
}
int main(){
    _setmode(_fileno(stdin),_O_BINARY);auto image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xf0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;if(!load())return 2;
    DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    auto* seed=reinterpret_cast<std::uint32_t*>(0x4c4570);
    using Integer=std::uint32_t(__cdecl*)();using Real=long double(__cdecl*)();
    for(std::uint32_t i=0;i<32768;++i){
        auto input=((i<<16)-0x269ec3U)*0xb9b33155U;
        lemmings::GameRandomState s{input};*seed=input;
        if(lemmings::next_game_random(s)!=reinterpret_cast<Integer>(0x49c84a)()||s.value!=*seed)return 1;
        s.value=input;*seed=input;volatile long double unit=reinterpret_cast<Real>(0x4040a0)();
        if(lemmings::game_random_unit(s)!=unit||s.value!=*seed)return 1;
        s.value=input;*seed=input;volatile long double signed_value=reinterpret_cast<Real>(0x404080)();
        if(lemmings::game_random_signed(s)!=signed_value||s.value!=*seed)return 1;
    }
    // A continuous sequence catches accidental reseeding or double advancement.
    lemmings::GameRandomState s{0xffffffffU};*seed=s.value;
    for(unsigned i=0;i<100000;++i)if(lemmings::game_random_signed(s)!=reinterpret_cast<Real>(0x404080)()||s.value!=*seed)return 1;
    std::cout<<"98304 exhaustive-output integer/unit/signed calls and100000 sequence calls match original\n";
}
