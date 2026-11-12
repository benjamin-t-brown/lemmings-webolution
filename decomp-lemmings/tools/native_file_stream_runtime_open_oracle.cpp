#include "../tests/file_stream_runtime_open_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

namespace {
file_runtime_open_test::Fixture* active;
void globals_from_fixture() {
    *reinterpret_cast<const void**>(0x5d55fc)=active->buffer.next_5d55fc;
    *reinterpret_cast<const void**>(0x5d5600)=active->buffer.end_5d5600;
}
std::int32_t __cdecl tell_adapter(void* file) {auto result=active->tell(file);globals_from_fixture();return result;}
std::int32_t __cdecl seek_adapter(void* file,std::int32_t offset,std::int32_t origin) {auto result=active->seek(file,offset,origin);globals_from_fixture();return result;}
void jump(std::uintptr_t address,void* target){auto* code=reinterpret_cast<unsigned char*>(address);code[0]=0xe9;auto offset=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-address-5);std::memcpy(code+1,&offset,4);}
}
// Recreates only the enclosing routine's local frame. The selected original
// block reloads this from [ebp-128], uses [ebp-124] for saved position, and exits
// through the unchanged original `mov esp,ebp; pop ebp; ret 8` epilogue.
extern "C" std::uint8_t __attribute__((naked,stdcall)) execute_open_suffix(void*,std::uint32_t) {
    __asm__ volatile(
        "push %ebp\n\t"
        "mov %esp,%ebp\n\t"
        "sub $0x128,%esp\n\t"
        "mov 8(%ebp),%eax\n\t"
        "mov %eax,-0x128(%ebp)\n\t"
        "cmpl $0,12(%ebp)\n\t"
        "jne 1f\n\t"
        "mov $0x47e770,%eax\n\t"
        "jmp *%eax\n\t"
        "1: mov $0x47e6cc,%eax\n\t"
        "jmp *%eax\n\t");
}
int main() {
    using namespace file_runtime_open_test;
    static_assert(sizeof(void*)==4);_setmode(_fileno(stdin),_O_BINARY);
    void* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x200000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t blocks=0;std::cin.read(reinterpret_cast<char*>(&blocks),4);
    while(blocks--){std::uint32_t address=0,size=0;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0x200000||address>0x600000-size)return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);
    }
    if(!std::cin)return 2;
    jump(0x49db9d,reinterpret_cast<void*>(&tell_adapter));jump(0x49dcf5,reinterpret_cast<void*>(&seek_adapter));
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    for(unsigned i=0;i<24000;++i){
        Fixture original(i,true),source(i);active=&original;globals_from_fixture();
        const auto actual=execute_open_suffix(original.raw.data(),original.archive);
        const auto expected=source.run_open();
        if(actual!=expected||original.raw!=source.encode()||original.events!=source.events||*reinterpret_cast<const void**>(0x5d55fc)!=source.buffer.next_5d55fc||*reinterpret_cast<const void**>(0x5d5600)!=source.buffer.end_5d5600){std::cerr<<"open suffix mismatch "<<i<<'\n';return 1;}
    }
    std::cout<<"24000 original direct/archive open-finalization blocks match (tell/seek adapters)\n";
}
