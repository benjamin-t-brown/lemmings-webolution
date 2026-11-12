#include "../tests/sprite_render_clip_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <memory>
namespace{using namespace sprite_clip_test;Fixture* active;
template<class T>T& global(std::uintptr_t a){return *reinterpret_cast<T*>(a);}
void pull(Fixture& f){auto& s=f.state;s.minimum_x_58d280=global<int>(0x58d280);s.maximum_x_58d284=global<int>(0x58d284);s.maximum_y_58d288=global<int>(0x58d288);s.minimum_y_58d28c=global<int>(0x58d28c);s.temporary_58d558=global<SpriteVertexPointerArray>(0x58d558);s.scratch_58d578=global<SpriteClipScratch>(0x58d578);}
void push(Fixture& f){auto& s=f.state;global<int>(0x58d280)=s.minimum_x_58d280;global<int>(0x58d284)=s.maximum_x_58d284;global<int>(0x58d288)=s.maximum_y_58d288;global<int>(0x58d28c)=s.minimum_y_58d28c;global<SpriteVertexPointerArray>(0x58d558)=s.temporary_58d558;global<SpriteClipScratch>(0x58d578)=s.scratch_58d578;}
void* __attribute__((cdecl)) allocate(unsigned bytes){active->event(1,bytes);if(bytes>512||active->allocated>=128)std::abort();return active->storage[active->allocated++];}
void __attribute__((cdecl)) release(void* p){active->event(2,active->pointer(p));}
void __attribute__((thiscall)) draw(SpriteVertexPointerArray* v){if(v!=&active->input)std::abort();++active->draws;active->event(3);}
void jump(std::uintptr_t a,void* target){auto* p=reinterpret_cast<unsigned char*>(a);p[0]=0xe9;auto d=static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(target)-a-5);std::memcpy(p+1,&d,4);}
}
int main(){static_assert(sizeof(void*)==4&&sizeof(SpriteClipScratch)==36);_setmode(_fileno(stdin),_O_BINARY);auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x190000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned n=0;std::cin.read(reinterpret_cast<char*>(&n),4);while(n--){unsigned a=0,size=0;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&size),4);if(a<0x400000||size>0xc0000||a>0x4c0000-size)return 2;std::cin.read(reinterpret_cast<char*>(a),size);}if(!std::cin)return 2;
    jump(0x49c1a8,reinterpret_cast<void*>(&allocate));jump(0x49c13d,reinterpret_cast<void*>(&release));jump(0x46d460,reinterpret_cast<void*>(&draw));DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Clip=void(__attribute__((thiscall))*)(SpriteVertexPointerArray*);using Interpolate=SpriteRenderVertex*(__attribute__((cdecl))*)(SpriteRenderVertex*,double,const SpriteRenderVertex*,const SpriteRenderVertex*);
    for(unsigned i=0;i<12000;++i){auto original=std::make_unique<Fixture>(i),source=std::make_unique<Fixture>(i);active=original.get();original->before=pull;original->after=push;push(*original);reinterpret_cast<Clip>(0x46d3c1)(&original->input);pull(*original);source->run();
        if(!original->same(*source)){std::cerr<<"clip mismatch trial "<<i<<" traces "<<original->trace.size()<<'/'<<source->trace.size()<<'\n';auto a=original->snapshot(),b=source->snapshot();for(unsigned k=0;k<a.size()&&k<b.size();++k)if(a[k]!=b[k])std::cerr<<k<<' '<<std::hex<<a[k]<<'/'<<b[k]<<'\n';return 1;}
    }
    std::uint64_t rng=0x34cb3e724abd8491ull;auto random=[&](){rng^=rng<<13;rng^=rng>>7;rng^=rng<<17;return rng;};
    for(unsigned i=0;i<40000;++i){SpriteRenderVertex a{},b{},original{},source{};for(unsigned k=0;k<13;++k){double x=static_cast<std::int32_t>(random())/32768.0,y=static_cast<std::int32_t>(random())/32768.0;if(i%3==1){x=std::bit_cast<double>(random());y=std::bit_cast<double>(random());}if(i%3==2){const double edge[]={0,-0.0,INFINITY,-INFINITY,NAN,1e-320,1e300,-1e300};x=edge[(i+k)%8];y=edge[(i/8+k)%8];}put(a,k,x);put(b,k,y);}
        const double plane=static_cast<std::int32_t>(random())/65536.0;bool x_axis=i%2;reinterpret_cast<Interpolate>(x_axis?0x46db80:0x46d910)(&original,plane,&a,&b);interpolate_sprite_clip_vertex(source,plane,a,b,x_axis);
        for(unsigned k=0;k<11;++k)if(number(get(original,k))!=number(get(source,k))){std::cerr<<"interpolation mismatch trial "<<i<<" component "<<k<<' '<<std::hex<<number(get(original,k))<<'/'<<number(get(source,k))<<'\n';return 1;}
    }
    std::cout<<"12000 complete clipping and 40000 interpolation original comparisons passed\n";
}
