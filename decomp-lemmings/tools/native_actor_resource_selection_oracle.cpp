#include "../tests/actor_resource_selection_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cstring>
#include <iostream>

namespace {
using resource_selection_fixture::Fixture;
Fixture* current;
char* frame;
bool valid;
template<class T> void put(void* p,unsigned n,const T& v){std::memcpy(static_cast<char*>(p)+n,&v,sizeof(v));}
template<class T> void copy_in(T& value,const void* p){std::memcpy(&value,p,sizeof(value));}
int formatter(char* target,const char* format,const char* prefix,const char* suffix) {
    valid &= std::strcmp(format,"%s%s")==0;
    std::strcpy(target,prefix);std::strcat(target,suffix);return static_cast<int>(std::strlen(target));
}
std::int32_t __attribute__((thiscall)) register_resource(void* manager,const char* name) {
    valid &= manager==reinterpret_cast<void*>(0x58d23c) && name==frame-0x2ac;
    copy_in(current->enabled,frame-0x1a0);copy_in(current->quality,frame-0x19c);
    copy_in(current->ids,reinterpret_cast<void*>(0x4e335c));
    auto result=Fixture::register_resource(current,name);
    std::memcpy(frame-0x1a0,current->enabled.data(),4);std::memcpy(frame-0x19c,current->quality.data(),16);
    put(reinterpret_cast<void*>(0x4e335c),0,current->ids);return result;
}
void jump(unsigned address,void* target) {
    put(reinterpret_cast<void*>(address),0,std::uint8_t{0xe9});
    put(reinterpret_cast<void*>(address),1,static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-address-5));
}
// The isolated original block uses EBP locals but has no prologue/epilogue.
// Supply a controlled frame and let its original exit branch reach a RET shim.
__attribute__((naked)) void run_block(void*) {
    __asm__ volatile("push %ebp\n\tpush %ebx\n\tpush %esi\n\tpush %edi\n\tmov 20(%esp),%ebp\n\tmov $0x4437f6,%eax\n\tcall *%eax\n\tpop %edi\n\tpop %esi\n\tpop %ebx\n\tpop %ebp\n\tret\n\t");
}
}
int main() {
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xf0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);if(!std::cin || blocks>32)return 2;
    for(unsigned i=0;i<blocks;++i) {
        unsigned a=0,n=0;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&n),4);
        if(a<0x400000 || n>0xf0000 || a-0x400000>0xf0000-n)return 2;
        std::cin.read(reinterpret_cast<char*>(a),n);if(!std::cin)return 2;
    }
    jump(0x49ca6a,reinterpret_cast<void*>(&formatter));jump(0x446040,reinterpret_cast<void*>(&register_resource));
    put(reinterpret_cast<void*>(0x44389e),0,std::uint8_t{0xc3});
    DWORD old;if(!VirtualProtect(image,0xc1000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc1000);
    unsigned cases=0;
    for(unsigned enabled=0;enabled<16;++enabled)for(unsigned quality=0;quality<16;++quality)for(unsigned script=0;script<8;++script) {
        Fixture native,source;
        for(unsigned i=0;i<4;++i){native.enabled[i]=source.enabled[i]=enabled&(1u<<i)?std::array<std::uint8_t,4>{1,0x80,255,2}[i]:0;native.quality[i]=source.quality[i]=(quality>>i)&1;}
        native.script=source.script=script;
        native.result=source.result=std::array<std::int32_t,4>{-1,0,-2147483647-1,2147483647}[(enabled+quality+script)%4];
        std::array<std::uint8_t,0x400> storage;storage.fill(0xa5);frame=reinterpret_cast<char*>(storage.data()+0x300);
        std::memcpy(frame-0x1a0,native.enabled.data(),4);std::memcpy(frame-0x19c,native.quality.data(),16);
        put(reinterpret_cast<void*>(0x4e335c),0,native.ids);current=&native;valid=true;
        run_block(frame);
        copy_in(native.enabled,frame-0x1a0);copy_in(native.quality,frame-0x19c);copy_in(native.ids,reinterpret_cast<void*>(0x4e335c));
        source.run();
        bool guards=true;
        for(unsigned i=0;i<storage.size();++i) {
            const bool mutable_byte=(i>=0x54 && i<0x54+16) || (i>=0x160 && i<0x174) || (i>=0x1a8 && i<0x1ac);
            if(!mutable_byte && storage[i]!=0xa5)guards=false;
        }
        if(!valid || !guards || native.events!=source.events || native.ids!=source.ids || native.enabled!=source.enabled || native.quality!=source.quality) {
            std::cerr<<"resource selection mismatch enabled="<<enabled<<" quality="<<quality<<" script="<<script<<'\n';return 1;
        }
        ++cases;
    }
    std::cout<<"native resource selection passed "<<cases<<" name/order/global/frame comparisons\n";
    VirtualFree(image,0,MEM_RELEASE);
}
