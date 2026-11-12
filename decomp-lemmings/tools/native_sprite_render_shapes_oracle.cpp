#include "../tests/sprite_render_shapes_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
namespace{using namespace sprite_shapes_test;Fixture* active;
template<class T>T& global(std::uintptr_t a){return *reinterpret_cast<T*>(a);}
void pull(Fixture& f){f.state.mode_58d53c=global<unsigned>(0x58d53c);f.state.reference_58cb38=global<unsigned>(0x58cb38);f.state.filtering_supported_58d121=global<std::uint8_t>(0x58d121);f.device.device_58cdb8=reinterpret_cast<void*>(global<unsigned>(0x58cdb8)==0x580000?0x1100:0x2200);f.device.alpha_reference_4c24e0=global<unsigned>(0x4c24e0);}
void push(Fixture& f){global<unsigned>(0x58d53c)=f.state.mode_58d53c;global<unsigned>(0x58cb38)=f.state.reference_58cb38;global<std::uint8_t>(0x58d121)=f.state.filtering_supported_58d121;global<unsigned>(0x58cdb8)=f.device.device_58cdb8==reinterpret_cast<void*>(0x1100)?0x580000:0x580100;global<unsigned>(0x4c24e0)=f.device.alpha_reference_4c24e0;}
void __attribute__((thiscall)) submit(SpriteRenderGeometry* m){if(m!=&active->model)std::abort();pull(*active);active->submit_shape();}
int __attribute__((stdcall)) stage(void* dev,unsigned index,unsigned type,unsigned value){if(index)std::abort();active->event(1,type,value,reinterpret_cast<void*>(dev==reinterpret_cast<void*>(0x580000)?0x1100:0x2200));return -1;}
void jump(std::uintptr_t a,void* target){auto* p=reinterpret_cast<unsigned char*>(a);p[0]=0xe9;auto d=static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-a-5);std::memcpy(p+1,&d,4);}
}
int main(){static_assert(sizeof(void*)==4&&offsetof(SpriteRenderGeometry,resource_1f8)==0x1f8);_setmode(_fileno(stdin),_O_BINARY);auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x190000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned n=0;std::cin.read(reinterpret_cast<char*>(&n),4);while(n--){unsigned a=0,size=0;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&size),4);if(a<0x400000||size>0xc0000||a>0x4c0000-size)return 2;std::cin.read(reinterpret_cast<char*>(a),size);}if(!std::cin)return 2;
    jump(0x414920,reinterpret_cast<void*>(&submit));global<unsigned>(0x580000)=0x580200;global<unsigned>(0x580100)=0x580200;global<void*>(0x5802a0)=reinterpret_cast<void*>(&stage);DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Trap=void(__attribute__((thiscall))*)(SpriteRenderGeometry*,double,double,double,double,double);using Anchored=void(__attribute__((thiscall))*)(SpriteRenderGeometry*,double,double);
    for(unsigned i=0;i<30000;++i)for(unsigned anchored=0;anchored<2;++anchored){Fixture original(i),source(i);active=&original;original.before=pull;original.after=push;push(original);auto& a=original.arguments;
        if(anchored)reinterpret_cast<Anchored>(0x47a75d)(&original.model,a[0],a[1]);else reinterpret_cast<Trap>(0x47a565)(&original.model,a[0],a[1],a[2],a[3],a[2]/2);pull(original);source.run(anchored);
        if(!original.same(source)){std::cerr<<"shape mismatch trial "<<i<<" anchored "<<anchored<<'\n';for(unsigned j=0;j<17;++j)if(original.submitted[j]!=source.submitted[j])std::cerr<<j<<' '<<std::hex<<original.submitted[j]<<'/'<<source.submitted[j]<<'\n';return 1;}}
    std::cout<<"60000 original sprite trapezoid/anchored/filtering comparisons passed\n";
}
