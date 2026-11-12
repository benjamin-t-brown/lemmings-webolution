#include "../tests/resource_manager_construction_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

int main() {
    using namespace manager_construction_test;
    static_assert(sizeof(void*)==4);
    _setmode(_fileno(stdin),_O_BINARY);
    void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xc0000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t count=0;std::cin.read(reinterpret_cast<char*>(&count),4);
    while(count--){std::uint32_t address=0,size=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0xc0000||address>0x4c0000-size)return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);
    }
    if(!std::cin)return 2;
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    constexpr std::uintptr_t addresses[]={0x484e40,0x485120,0x4852d0,0x4855f0,0x485730,0x485470,0x430920};
    constexpr unsigned offsets[]={0,0,0,4,4,0x30,4};
    using Constructor=void*(__attribute__((thiscall))*)(void*);
    for(unsigned i=0;i<35000;++i){
        Fixture source(i);alignas(4) auto original=source.initial;
        auto* target=original.data()+offsets[source.operation];
        auto* actual=reinterpret_cast<Constructor>(addresses[source.operation])(target);
        auto offset=source.run();
        if(actual!=target||offset!=offsets[source.operation]||original!=source.serialize(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(original.data())))){
            std::cerr<<"manager constructor mismatch trial "<<i<<" entry "<<std::hex<<addresses[source.operation]<<'\n';return 1;
        }
    }
    std::cout<<"35000 complete resource manager constructor buffers match original (7 entrypoints, no adapters)\n";
}
