#include "../tests/texture_pixel_format_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
int main() {
    using namespace texture_pixel_format_test;
    static_assert(sizeof(TexturePixelFormat)==32 && sizeof(TexturePixelFormatSelection)==20);
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x100000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t count;std::cin.read(reinterpret_cast<char*>(&count),4);
    while(count--){std::uint32_t address,size;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(address<0x400000||size>0x100000||address>0x500000-size)return 2;
        std::cin.read(reinterpret_cast<char*>(address),size);}
    if(!std::cin)return 2;
    auto* patch=reinterpret_cast<std::uint8_t*>(0x49d010);patch[0]=0xe9;
    const auto delta=std::uint32_t(reinterpret_cast<std::uintptr_t>(&std::memcpy)-0x49d015);
    std::memcpy(patch+1,&delta,4);
    DWORD old;if(!VirtualProtect(image,0x100000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0x100000);
    using Original=std::uint32_t(__stdcall*)(const TexturePixelFormat*,TexturePixelFormatSelection*);
    for(unsigned i=0;i<trials;++i){Fixture a(i),b(i);
        const auto expected=select_texture_pixel_format(a.input(),a.context());
        const auto actual=reinterpret_cast<Original>(0x473b1e)(b.input(),b.context());
        if(expected!=actual||!a.equivalent(b)){std::cerr<<"pixel format mismatch "<<i<<'\n';return 1;}}
    std::cout<<trials<<" pixel-format cases match unchanged original473B1E\n";
}
