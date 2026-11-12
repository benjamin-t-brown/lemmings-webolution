#include "../tests/resource_cache_load_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cstring>
#include <iostream>
namespace {
cache_load_test::Fixture* active;
cache_load_test::ComposedFixture* composed;
void __cdecl notify(const char* name) {composed->notify(name);}
void* __attribute__((thiscall)) create_named(void* manager,const char* name) {
    if(manager!=&composed->state.manager)std::abort();
    return composed->named_create(name);
}
void* __attribute__((thiscall)) create(void* manager,int id) {
    if(manager!=&active->state.manager)std::abort();
    return active->create(id);
}
void* __cdecl allocate(std::uint32_t bytes) {
    if(bytes!=0x14)std::abort();
    return active->allocate();
}
void jump(std::uintptr_t address,void* target) {
    auto* p=reinterpret_cast<unsigned char*>(address);p[0]=0xe9;
    auto displacement=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-address-5);
    std::memcpy(p+1,&displacement,4);
}
}
int main() {
    static_assert(sizeof(void*)==4);
    static_assert(offsetof(cache_load_test::State,loaded_count)==0x2c);
    _setmode(_fileno(stdin),_O_BINARY);
    void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xc0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t count=0;std::cin.read(reinterpret_cast<char*>(&count),4);
    while(count--) {
        std::uint32_t address=0,size=0;
        std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||address+size>0x4c0000)return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);
    }
    if(!std::cin)return 2;
    jump(0x49c1a8,reinterpret_cast<void*>(&allocate));
    DWORD previous=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&previous))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Load=void*(__attribute__((thiscall))*)(void*,int);
    using Prepend=void(__attribute__((thiscall))*)(void*,const lemmings::ResourceCacheNode*);
    using Construct=void*(__attribute__((thiscall))*)(void*,const lemmings::ResourceCacheNode*,void*,void*);
    using Initialize=void*(__attribute__((thiscall))*)(void*);
    void* vtable[6]{nullptr,nullptr,nullptr,reinterpret_cast<void*>(&create),nullptr,reinterpret_cast<void*>(&create_named)};
    for(unsigned t=0;t<25000;++t) {
        cache_load_test::Fixture native(t),port(t);active=&native;
        auto* table=vtable;std::memcpy(native.state.manager.unknown_00.data(),&table,4);
        void* actual=nullptr;
        if(native.operation<2)actual=reinterpret_cast<Load>(0x430660)(&native.state.manager,native.index);
        else if(native.operation==2)reinterpret_cast<Prepend>(0x430950)(&native.state.manager.list_04,&native.source);
        else if(native.operation==3)actual=reinterpret_cast<Construct>(0x430a10)(&native.nodes[7],native.alias?&native.nodes[7].value:&native.source,&native.nodes[2],&native.nodes[3]);
        else actual=reinterpret_cast<Initialize>(0x430920)(&native.nodes[7].value);
        auto* expected=port.run();
        if(native.result_id(actual)!=port.result_id(expected)||native.trace!=port.trace||native.snapshot()!=port.snapshot()) {
            std::cerr<<"resource load mismatch "<<t<<'\n';return 1;
        }
    }
    using Acquire=void*(__attribute__((thiscall))*)(void*,int,void(__cdecl*)(const char*));
    using Release=void(__attribute__((thiscall))*)(void*,int);
    for(unsigned t=0;t<20000;++t) {
        cache_load_test::ComposedFixture native(t),port(t);active=&native;composed=&native;
        vtable[3]=(native.trial&128)?reinterpret_cast<void*>(0x485590):reinterpret_cast<void*>(&create);
        auto* table=vtable;std::memcpy(native.state.manager.unknown_00.data(),&table,4);
        void* actual=nullptr;
        if(native.combined_operation==0)actual=reinterpret_cast<Acquire>(0x42ff90)(&native.state.manager,native.index,&notify);
        else if(native.combined_operation==1)actual=reinterpret_cast<Load>(0x446010)(&native.state.manager,native.index);
        else reinterpret_cast<Release>(0x430050)(&native.state.manager,native.index);
        auto* expected=port.run_combined();
        if(actual!=expected||native.trace!=port.trace||native.combined_snapshot()!=port.combined_snapshot()) {
            std::cerr<<"composed cache mismatch "<<t<<'\n';return 1;
        }
    }
    for(unsigned t=0;t<10000;++t) {
        cache_load_test::ComposedFixture native(t),port(t);active=&native;composed=&native;
        auto* table=vtable;std::memcpy(native.state.manager.unknown_00.data(),&table,4);
        auto* actual=reinterpret_cast<Load>(0x485590)(&native.state.manager,native.index);
        auto* expected=lemmings::create_resource_by_index(port.cache,port.index,
            {&port,[](void* p,const char* name){return static_cast<cache_load_test::ComposedFixture*>(p)->named_create(name);}});
        if(actual!=expected || native.trace!=port.trace || native.combined_snapshot()!=port.combined_snapshot()) {
            std::cerr<<"resource name bridge mismatch "<<t<<'\n';return 1;
        }
    }
    std::cout<<"25000 load/list/node, 20000 composed cache and 10000 name bridge cases match\n";
}
