#include "../tests/resource_registration_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

namespace {
registration_test::Fixture* active;
void* __cdecl allocate(std::uint32_t size) {
    const auto storage=active->allocate(size);return storage.cookie;
}
void __cdecl deallocate(void* pointer) {
    active->deallocate(reinterpret_cast<lemmings::ResourceCacheEntry*>(static_cast<unsigned char*>(pointer)+4));
}
void jump(std::uintptr_t address,void* target) {
    auto* code=reinterpret_cast<unsigned char*>(address);code[0]=0xe9;
    auto displacement=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-address-5);
    std::memcpy(code+1,&displacement,4);
}
}
int main() {
    using namespace registration_test;
    static_assert(sizeof(void*)==4);static_assert(sizeof(ResourceCacheEntry)==0x88);
    static_assert(offsetof(Block,entries)==4);
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xc0000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);
    while(blocks--){std::uint32_t address=0,size=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0xc0000||address>0x4c0000-size)return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);
    }
    if(!std::cin)return 2;
    jump(0x49c1a8,reinterpret_cast<void*>(&allocate));jump(0x49c13d,reinterpret_cast<void*>(&deallocate));
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Register=int(__attribute__((thiscall))*)(void*,const char*);
    using Append=void(__attribute__((thiscall))*)(void*,const ResourceCacheEntry*);
    for(unsigned i=0;i<32768;++i){
        Fixture original(i,true),source(i);active=&original;int actual=-1;
        if(original.operation==0)actual=reinterpret_cast<Register>(0x446040)(original.raw_manager.data(),original.input.data()+original.alignment);
        else reinterpret_cast<Append>(0x449db0)(original.raw_manager.data()+0x30,&original.source);
        const int expected=source.run();
        if(actual!=expected||original.trace!=source.trace||!original.equals(source)){
            std::cerr<<"registration mismatch trial "<<i<<" mode "<<original.mode<<" operation "<<original.operation<<" return "<<actual<<'/'<<expected<<'\n';return 1;
        }
    }
    // Direct constructors and assignments exercise all name lengths/alignment,
    // including fields in empty/default records that registration never reads.
    using Default=ResourceCacheEntry*(__attribute__((thiscall))*)(ResourceCacheEntry*);
    using Named=ResourceCacheEntry*(__attribute__((thiscall))*)(ResourceCacheEntry*,const char*);
    using Assign=ResourceCacheEntry*(__attribute__((thiscall))*)(ResourceCacheEntry*,const ResourceCacheEntry*);
    for(unsigned i=0;i<8192;++i){
        Fixture original(i,true),source(i);auto& a=original.old_block.entries[0];auto& b=source.old_block.entries[0];
        if(i%3==0){if(reinterpret_cast<Default>(0x44be60)(&a)!=&a)return 1;initialize_resource_entry(b);}
        else if(i%3==1){if(reinterpret_cast<Named>(0x449170)(&a,original.input.data()+original.alignment)!=&a)return 1;initialize_named_resource_entry(b,source.input.data()+source.alignment);}
        else{if(reinterpret_cast<Assign>(0x448f50)(&a,&original.source)!=&a)return 1;assign_resource_entry(b,source.source);}
        if(!original.equals(source)){std::cerr<<"entry mismatch trial "<<i<<'\n';return 1;}
    }
    std::cout<<"32768 resource registration/append traces and 8192 entry operations match original\n";
}
