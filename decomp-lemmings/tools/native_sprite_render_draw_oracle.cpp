#include "../tests/sprite_render_draw_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
namespace {using namespace sprite_draw_test;Fixture* active;
template<class T>T& global(std::uintptr_t a){return *reinterpret_cast<T*>(a);}
void pull(Fixture& f){f.device.device_58cdb8=reinterpret_cast<void*>(global<std::uintptr_t>(0x58cdb8)==0x580000?0x1100:0x2200);f.state.vertices_58d540=global<SpriteDeviceVertexArray>(0x58d540);f.state.initialized_flags_58d550=global<unsigned>(0x58d550);f.state.u_offset_58d2a0=global<double>(0x58d2a0);f.state.v_offset_58d2a8=global<double>(0x58d2a8);}
void push(Fixture& f){global<std::uintptr_t>(0x58cdb8)=f.device.device_58cdb8==reinterpret_cast<void*>(0x1100)?0x580000:0x580100;global<SpriteDeviceVertexArray>(0x58d540)=f.state.vertices_58d540;global<unsigned>(0x58d550)=f.state.initialized_flags_58d550;global<double>(0x58d2a0)=f.state.u_offset_58d2a0;global<double>(0x58d2a8)=f.state.v_offset_58d2a8;}
void* __attribute__((cdecl)) allocate(unsigned bytes){active->event(2,bytes);if(bytes>512)std::abort();return active->new_buffer;}
void __attribute__((cdecl)) release(void* p){active->event(3,active->pointer(p));}
int __attribute__((cdecl)) cleanup(void* p){if(p!=reinterpret_cast<void*>(0x46d605))std::abort();active->event(1);return 0;}
int __attribute__((stdcall)) draw(void* d,unsigned primitive,unsigned format,SpriteDeviceVertex* v,unsigned count,unsigned flags){pull(*active);auto& f=*active;auto id=reinterpret_cast<void*>(d==reinterpret_cast<void*>(0x580000)?0x1100:0x2200);if(id!=f.device.device_58cdb8||primitive!=6||format!=0x1c4||flags||v!=f.state.vertices_58d540.data_0c)std::abort();active->event(4,count);return -1;}
void jump(std::uintptr_t a,void* target){auto* p=reinterpret_cast<unsigned char*>(a);p[0]=0xe9;auto d=static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-a-5);std::memcpy(p+1,&d,4);}
}
int main(){static_assert(sizeof(void*)==4&&sizeof(SpriteDeviceVertexArray)==16);_setmode(_fileno(stdin),_O_BINARY);auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x190000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned n=0;std::cin.read(reinterpret_cast<char*>(&n),4);while(n--){unsigned a=0,size=0;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&size),4);if(a<0x400000||size>0xc0000||a>0x4c0000-size)return 2;std::cin.read(reinterpret_cast<char*>(a),size);}if(!std::cin)return 2;
    jump(0x49c1a8,reinterpret_cast<void*>(&allocate));jump(0x49c13d,reinterpret_cast<void*>(&release));jump(0x49cb29,reinterpret_cast<void*>(&cleanup));global<std::uintptr_t>(0x580000)=0x580200;global<std::uintptr_t>(0x580100)=0x580200;global<void*>(0x580270)=reinterpret_cast<void*>(&draw);
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Draw=void(__attribute__((thiscall))*)(SpriteVertexPointerArray*);using Clear=void(__attribute__((cdecl))*)();
    for(unsigned i=0;i<30000;++i){Fixture original(i),source(i);active=&original;original.before=pull;original.after=push;push(original);reinterpret_cast<Draw>(0x46d460)(&original.input);pull(original);source.run();if(!original.same(source)){std::cerr<<"draw mismatch trial "<<i<<'\n';auto a=original.snapshot(),b=source.snapshot();for(unsigned k=0;k<a.size();++k)if(a[k]!=b[k])std::cerr<<k<<' '<<std::hex<<a[k]<<'/'<<b[k]<<'\n';return 1;}
        reinterpret_cast<Clear>(0x46d605)();pull(original);clear_sprite_device_vertices(source.state.vertices_58d540,source.hooks());if(!original.same(source)){std::cerr<<"cleanup mismatch trial "<<i<<'\n';return 1;}
    }
    std::cout<<"30000 packed sprite draw/cleanup original comparisons passed\n";
}
