#include "../tests/world_entity_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
using namespace world_test;
namespace {
using NativeUpdate=void (__attribute__((thiscall)) *)(void*,double);
std::uint8_t __attribute__((thiscall)) update(Entity* e,double dt){return active->update(*e,dt);}
void __attribute__((thiscall)) deactivate(Entity* e){active->deactivate(*e);}
void __attribute__((thiscall)) enqueue(void*,Entity* e){active->enqueue(*e);}
void __attribute__((thiscall)) unlink0(Node* node){active->unlink(0,static_cast<int>(node-active->raw_nodes[0].data()));}
void __attribute__((thiscall)) unlink1(Node* node){active->unlink(1,static_cast<int>(node-active->raw_nodes[1].data()));}
void __cdecl notice(const char*,int id,int){active->notice(id);}
void __attribute__((thiscall)) erase0(RawArray*,int index){active->erase(0,index);}
void __attribute__((thiscall)) erase2(RawArray* array,Entity** entry){active->erase(2,static_cast<int>(entry-array->data));}
void __attribute__((thiscall)) registration(void*,Entity* e){active->registration(*e);}
void jump(std::uintptr_t address,std::uintptr_t target){auto* p=reinterpret_cast<unsigned char*>(address);p[0]=0xe9;std::uint32_t delta=static_cast<std::uint32_t>(target-address-5);std::memcpy(p+1,&delta,4);}
bool load(){std::uint32_t n;std::cin.read(reinterpret_cast<char*>(&n),4);if(!std::cin||n>100)return false;while(n--){std::uint32_t a,s;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&s),4);if(a<0x400000||a+s>0x500000)return false;std::cin.read(reinterpret_cast<char*>(a),s);if(!std::cin)return false;}return true;}
}
int main(){
    static_assert(sizeof(void*)==4);static_assert(offsetof(RawArray,data)==12);
    _setmode(_fileno(stdin),_O_BINARY);
    void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x100000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(image!=reinterpret_cast<void*>(0x400000))return 3;if(!load())return 2;
    // Only these two jumps bypass unclaimed work. Every instruction in the
    // claimed [442B00,443160) collection block remains unchanged.
    jump(0x4423f6,0x442b00);jump(0x443160,0x4431b3);
    jump(0x4090b0,reinterpret_cast<std::uintptr_t>(&enqueue));jump(0x445950,reinterpret_cast<std::uintptr_t>(&unlink0));jump(0x408fc0,reinterpret_cast<std::uintptr_t>(&unlink1));jump(0x40f860,reinterpret_cast<std::uintptr_t>(&notice));jump(0x4465d0,reinterpret_cast<std::uintptr_t>(&erase0));jump(0x446c70,reinterpret_cast<std::uintptr_t>(&erase2));jump(0x43b536,reinterpret_cast<std::uintptr_t>(&registration));
    DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    void* vtable[6]{};vtable[2]=reinterpret_cast<void*>(&deactivate);vtable[5]=reinterpret_cast<void*>(&update);
    auto original=reinterpret_cast<NativeUpdate>(0x4423d2);
    for(int trial=0;trial<10000;++trial){Fixture native(trial,true,vtable),port(trial,false);active=&native;original(native.raw_world.data(),native.elapsed);port.run_port();if(!native.elapsed_matches||!port.elapsed_matches||native.trace!=port.trace||native.snapshot()!=port.snapshot()){
        std::cerr<<"collection mismatch trial "<<trial<<" native="<<native.trace.size()<<" port="<<port.trace.size()<<'\n';for(std::size_t i=0;i<native.trace.size()&&i<port.trace.size();++i)if(native.trace[i]!=port.trace[i]){std::cerr<<"event "<<i<<" "<<native.trace[i]<<" "<<port.trace[i]<<'\n';break;}return 1;}}
    VirtualFree(image,0,MEM_RELEASE);std::cout<<"native world collection block passed 10000 trace/state comparisons\n";
}
