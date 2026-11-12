#include "../tests/sprite_render_geometry_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
namespace {
sprite_geometry_test::Fixture* active;
void __attribute__((thiscall)) submit(lemmings::SpriteRenderGeometry* m){if(m!=&active->model)std::abort();active->submit();}
void jump(std::uintptr_t a,void* target){auto* p=reinterpret_cast<unsigned char*>(a);p[0]=0xe9;auto d=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-a-5);std::memcpy(p+1,&d,4);}
}
int main() {
    using namespace sprite_geometry_test;
    static_assert(sizeof(void*)==4&&sizeof(SpriteRenderVertex)==0x68&&offsetof(SpriteRenderGeometry,resource_1f8)==0x1f8);
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x190000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t n=0;std::cin.read(reinterpret_cast<char*>(&n),4);
    while(n--){std::uint32_t a=0,size=0;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&size),4);
        if(a<0x400000||size>0xc0000||a>0x4c0000-size)return 2;
        std::cin.read(reinterpret_cast<char*>(a),size);}
    if(!std::cin)return 2;
    jump(0x414920,reinterpret_cast<void*>(&submit));DWORD old=0;
    if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;
    FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Build=void(__attribute__((thiscall))*)(void*,double,double,double,double);
    for(unsigned i=0;i<30000;++i) {
        Fixture original(i),source(i);active=&original;
        *reinterpret_cast<std::uint32_t*>(0x58d53c)=original.mode;*reinterpret_cast<std::uint32_t*>(0x58cb38)=original.reference;
        reinterpret_cast<Build>(0x47a190)(&original.model,original.arguments[0],original.arguments[1],original.arguments[2],original.arguments[3]);
        source.run();
        if(!original.same(source)){std::cerr<<"geometry mismatch trial "<<i<<'\n';for(unsigned j=0;j<17;++j)if(original.submitted[j]!=source.submitted[j])std::cerr<<j<<' '<<std::hex<<original.submitted[j]<<'/'<<source.submitted[j]<<'\n';return 1;}
    }
    std::cout<<"30000 complete sprite rectangle geometry/UV/binding and submit traces match original\n";
}
