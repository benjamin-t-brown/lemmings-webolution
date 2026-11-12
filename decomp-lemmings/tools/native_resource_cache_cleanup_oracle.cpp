#include "../tests/resource_cache_cleanup_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

namespace {
using namespace cleanup_test;
Fixture* active=nullptr;
bool valid=true;
std::array<std::uintptr_t,5> table0{},table1{};
void __attribute__((thiscall)) destroy0(void* manager,void* resource){valid &= manager==&active->manager.lookup;active->destroy(resource,0);}
void __attribute__((thiscall)) destroy1(void* manager,void* resource){valid &= manager==&active->manager.lookup;active->destroy(resource,1);}
void* __attribute__((thiscall)) create0(void* manager,int id){valid &= manager==&active->manager.lookup;return active->create(id,0);}
void* __attribute__((thiscall)) create1(void* manager,int id){valid &= manager==&active->manager.lookup;return active->create(id,1);}
void __cdecl release(void* node){active->free_node(static_cast<ResourceCacheLookupNode*>(node));}
void switch_dispatch(Fixture& fixture){auto* table=table1.data();std::memcpy(fixture.manager.lookup.unknown_00.data(),&table,4);}
void jump(unsigned address,void* target){auto* p=reinterpret_cast<unsigned char*>(address);p[0]=0xe9;auto displacement=static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-address-5);std::memcpy(p+1,&displacement,4);}
}
int main(){
    static_assert(sizeof(void*)==4);
    _setmode(_fileno(stdin),_O_BINARY);
    void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xc0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned count=0;std::cin.read(reinterpret_cast<char*>(&count),4);if(!std::cin||count>64)return 2;
    for(unsigned i=0;i<count;++i){unsigned address=0,size=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0xc0000||address-0x400000>0xc0000-size)return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);if(!std::cin)return 2;}
    jump(0x49c13d,reinterpret_cast<void*>(&release));
    table0[3]=reinterpret_cast<std::uintptr_t>(&create0);table0[4]=reinterpret_cast<std::uintptr_t>(&destroy0);
    table1[3]=reinterpret_cast<std::uintptr_t>(&create1);table1[4]=reinterpret_cast<std::uintptr_t>(&destroy1);
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Action=void(__attribute__((thiscall))*)(void*);
    constexpr std::array<unsigned,7> addresses{0x44a050,0x485380,0x485400,0x4854d0,0x485530,0x449fb0,0x44be00};
    unsigned cases=0,events=0;
    for(unsigned seed=0;seed<12;++seed)for(unsigned script=0;script<256;++script)for(unsigned operation=0;operation<7;++operation){
        Fixture native(seed,script,operation),port(seed,script,operation);
        auto* table=table0.data();std::memcpy(native.manager.lookup.unknown_00.data(),&table,4);
        native.on_switch=&switch_dispatch;active=&native;valid=true;
        reinterpret_cast<Action>(addresses[operation])(operation==6?static_cast<void*>(&native.nodes[0]):static_cast<void*>(&native.manager.lookup));
        port.run();++cases;events+=native.trace.size();
        if(!valid||native.trace!=port.trace||native.snapshot()!=port.snapshot()){
            std::cerr<<"resource cleanup mismatch seed="<<seed<<" script="<<script<<" operation="<<operation<<" trace sizes="<<native.trace.size()<<'/'<<port.trace.size()<<'\n';
            for(unsigned i=0;i<native.trace.size()&&i<port.trace.size();++i)if(native.trace[i]!=port.trace[i]){
                std::cerr<<"event="<<i<<" kind="<<native.trace[i].kind<<'/'<<port.trace[i].kind<<'\n';
                for(unsigned j=0;j<native.trace[i].state.size();++j)if(native.trace[i].state[j]!=port.trace[i].state[j]){std::cerr<<"field="<<j<<" value="<<native.trace[i].state[j]<<'/'<<port.trace[i].state[j]<<'\n';break;}
                break;
            }
            return 1;
        }
    }
    std::cout<<"resource cleanup: "<<cases<<" native cases and "<<events<<" callback snapshots passed\n";
}
