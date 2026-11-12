#include "../tests/sprite_render_submission_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
namespace {
using namespace sprite_submission_test;
Fixture* active;
template<class T>T& global(std::uintptr_t a){return *reinterpret_cast<T*>(a);}
void pull(Fixture& f){auto& s=f.state;auto& d=s.device;
    d.device_58cdb8=reinterpret_cast<void*>(global<std::uintptr_t>(0x58cdb8)==0x580000?0x1100:0x2200);
    d.active_texture_5cd5f4=global<void*>(0x5cd5f4);d.texture_flags_4c24e4=global<std::uint32_t>(0x4c24e4);d.alpha_reference_4c24e0=global<std::uint32_t>(0x4c24e0);d.blend_mode_4c227c=global<int>(0x4c227c);
    s.scratch_count_58d588=global<std::uint32_t>(0x58d588);s.scratch_used_58d598=global<std::uint32_t>(0x58d598);s.initialized_flags_5d6160=global<std::uint32_t>(0x5d6160);s.vertices_5d6170=global<SpriteVertexPointerArray>(0x5d6170);f.cache.entries_30=global<ResourceCacheEntry*>(0x58cb84);
}
void push(Fixture& f){auto& s=f.state;auto& d=s.device;
    global<std::uintptr_t>(0x58cdb8)=d.device_58cdb8==reinterpret_cast<void*>(0x1100)?0x580000:0x580100;
    global<void*>(0x5cd5f4)=d.active_texture_5cd5f4;global<std::uint32_t>(0x4c24e4)=d.texture_flags_4c24e4;global<std::uint32_t>(0x4c24e0)=d.alpha_reference_4c24e0;global<int>(0x4c227c)=d.blend_mode_4c227c;
    global<std::uint32_t>(0x58d588)=s.scratch_count_58d588;global<std::uint32_t>(0x58d598)=s.scratch_used_58d598;global<std::uint32_t>(0x5d6160)=s.initialized_flags_5d6160;global<SpriteVertexPointerArray>(0x5d6170)=s.vertices_5d6170;global<ResourceCacheEntry*>(0x58cb84)=f.cache.entries_30;
}
std::uint32_t device_id(void* d){return d==reinterpret_cast<void*>(0x580000)?0x1100:0x2200;}
int __attribute__((stdcall)) texture(void* d,unsigned stage,void* t){active->event(1,device_id(d),stage,active->pointer(t));return -1;}
int __attribute__((stdcall)) stage(void* d,unsigned index,unsigned type,unsigned value){if(index)std::abort();active->event(2,device_id(d),type,value);return -1;}
int __attribute__((stdcall)) render(void* d,unsigned type,unsigned value){active->event(3,device_id(d),type,value);return -1;}
ResourceCacheNode* __attribute__((thiscall)) find(void*,int id){return active->find(id);}
void* __attribute__((thiscall)) load(void*,int id){return active->load(id);}
void* __attribute__((cdecl)) allocate(unsigned bytes){active->event(5,bytes);if(bytes>64)std::abort();return active->new_buffer;}
void __attribute__((cdecl)) release(void* p){active->event(6,active->pointer(p));}
int __attribute__((cdecl)) cleanup(void* p){if(p!=reinterpret_cast<void*>(0x4150a0))std::abort();active->event(4);return 0;}
void __attribute__((thiscall)) clip(SpriteVertexPointerArray* v){if(v!=reinterpret_cast<void*>(0x5d6170))std::abort();active->event(7,active->pointer(v->data_0c[0]),active->pointer(v->data_0c[1]),active->pointer(v->data_0c[2]));}
void jump(std::uintptr_t a,void* target){auto* p=reinterpret_cast<unsigned char*>(a);p[0]=0xe9;auto d=static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)-a-5);std::memcpy(p+1,&d,4);}
}
int main(){
    static_assert(sizeof(void*)==4&&offsetof(Texture,flags)==0x24&&sizeof(SpriteVertexPointerArray)==16);
    _setmode(_fileno(stdin),_O_BINARY);auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0x1e0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);if(image!=reinterpret_cast<void*>(0x400000))return 3;
    unsigned n=0;std::cin.read(reinterpret_cast<char*>(&n),4);while(n--){unsigned a=0,size=0;std::cin.read(reinterpret_cast<char*>(&a),4);std::cin.read(reinterpret_cast<char*>(&size),4);if(a<0x400000||size>0xc0000||a>0x4c0000-size)return 2;std::cin.read(reinterpret_cast<char*>(a),size);}if(!std::cin)return 2;
    jump(0x414c50,reinterpret_cast<void*>(&find));jump(0x414e60,reinterpret_cast<void*>(&load));jump(0x49c1a8,reinterpret_cast<void*>(&allocate));jump(0x49c13d,reinterpret_cast<void*>(&release));jump(0x49cb29,reinterpret_cast<void*>(&cleanup));jump(0x46d3c1,reinterpret_cast<void*>(&clip));
    global<std::uintptr_t>(0x580000)=0x580200;global<std::uintptr_t>(0x580100)=0x580200;
    global<void*>(0x580258)=reinterpret_cast<void*>(&render);global<void*>(0x580298)=reinterpret_cast<void*>(&texture);global<void*>(0x5802a0)=reinterpret_cast<void*>(&stage);
    DWORD old=0;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Submit=void(__attribute__((thiscall))*)(SpriteRenderGeometry*);using Resize=void(__attribute__((thiscall))*)(SpriteVertexPointerArray*,int);using Clear=void(__attribute__((cdecl))*)();
    for(unsigned i=0;i<129024;++i){Fixture original(i),source(i);active=&original;original.before=pull;original.after=push;push(original);reinterpret_cast<Submit>(0x414920)(&original.model);pull(original);source.run();
        if(!original.same(source)){std::cerr<<"submission mismatch trial "<<i<<" events "<<original.trace.size()<<'/'<<source.trace.size()<<'\n';for(unsigned j=0;j<original.trace.size()&&j<source.trace.size();++j)if(original.trace[j]!=source.trace[j]){for(unsigned k=0;k<14;++k)std::cerr<<original.trace[j][k]<<'/'<<source.trace[j][k]<<' ';std::cerr<<'\n';}return 1;}
    }
    for(unsigned i=0;i<256;++i){Fixture original(i),source(i);active=&original;original.before=pull;original.after=push;
        auto prepare=[&](Fixture& f){f.state.vertices_5d6170={static_cast<int>(i%3),static_cast<int>(i%6)-2,static_cast<int>((i/6)%3)+1,(i&1)?f.old_buffer:nullptr};if(!f.state.vertices_5d6170.data_0c)f.state.vertices_5d6170.count_00=0;};prepare(original);prepare(source);push(original);
        int requested=static_cast<int>(i%7)-1;reinterpret_cast<Resize>(0x414a60)(reinterpret_cast<SpriteVertexPointerArray*>(0x5d6170),requested);pull(original);resize_sprite_submission_vertices(source.state.vertices_5d6170,requested,source.hooks());if(!original.same(source)){std::cerr<<"resize mismatch "<<i<<'\n';return 1;}
        reinterpret_cast<Clear>(0x4150a0)();pull(original);clear_sprite_submission_vertices(source.state.vertices_5d6170,source.hooks());if(!original.same(source)){std::cerr<<"clear mismatch "<<i<<'\n';return 1;}
    }
    std::cout<<"129024 full submission and 256 resize/cleanup original comparisons passed\n";
}
