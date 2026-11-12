#include "../tests/resource_cache_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
namespace {
cache_test::Fixture* active;
void* __attribute__((thiscall)) find(void* p,int i){if(p!=active->raw_manager.data())std::abort();return active->find(i);}
void* __attribute__((thiscall)) load_resource(void* p,int i){if(p!=active->raw_manager.data())std::abort();return active->load(i);}
void __cdecl notify(const char* name){active->notify(name);}
void jump(std::uintptr_t a,void* f){auto* p=reinterpret_cast<unsigned char*>(a);p[0]=0xe9;auto d=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(f)-a-5);std::memcpy(p+1,&d,4);}
}
int main(){
    static_assert(sizeof(void*)==4);static_assert(sizeof(lemmings::ResourceCacheEntry)==0x88);static_assert(offsetof(lemmings::ResourceCacheNode,references_08)==8);
    _setmode(_fileno(stdin),_O_BINARY);void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xc0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t count;std::cin.read(reinterpret_cast<char*>(&count),4);while(count--){std::uint32_t a,n;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);if(a<0x400000||a+n>0x4c0000)return 2;std::cin.read(reinterpret_cast<char*>(a),n);}if(!std::cin)return 2;
    jump(0x4306f0,reinterpret_cast<void*>(&find));jump(0x430660,reinterpret_cast<void*>(&load_resource));DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Acquire=void*(__attribute__((thiscall))*)(void*,int,void(__cdecl*)(const char*));using Release=void(__attribute__((thiscall))*)(void*,int);using Balanced=void*(__attribute__((thiscall))*)(void*,int);
    for(unsigned i=0;i<20000;++i){cache_test::Fixture native(i,true),port(i,false);active=&native;void* actual=nullptr;
        if(native.operation==0)actual=reinterpret_cast<Acquire>(0x42ff90)(native.raw_manager.data(),native.index,i%2?&notify:nullptr);else if(native.operation==1)reinterpret_cast<Release>(0x430050)(native.raw_manager.data(),native.index);else actual=reinterpret_cast<Balanced>(0x446010)(native.raw_manager.data(),native.index);
        void* expected=port.run();if(actual!=expected||native.trace!=port.trace||native.snapshot()!=port.snapshot()){std::cerr<<"cache mismatch "<<i<<'\n';return 1;}}
    std::cout<<"20000 resource cache acquire/release/balanced traces match original\n";
}
