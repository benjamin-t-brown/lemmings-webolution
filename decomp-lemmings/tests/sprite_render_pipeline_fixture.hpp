#pragma once
#include "lemmings/sprite_render_submission.hpp"
#include "lemmings/sprite_render_clip.hpp"
#include "lemmings/sprite_render_draw.hpp"
#include <memory>
#include <vector>
#include <bit>
#include <cmath>
#include <cstring>
#include <cstdlib>
namespace sprite_pipeline_test {
using namespace lemmings;
struct Texture {void* device_texture_00;std::array<std::byte,32> unknown_04;std::uint32_t flags_24;};
static_assert(offsetof(Texture,device_texture_00)==0);
static_assert(sizeof(void*)!=4||offsetof(Texture,flags_24)==0x24);
struct Fixture {
    unsigned trial;std::uint64_t rng;
    SpriteRenderGeometry model{};Texture texture{};ResourceCacheNode node{};ResourceCacheEntry entries[2]{};ResourceCache cache{};
    SpriteSubmissionState submission{};SpriteClipState clipping{};SpriteDrawState drawing{};
    SpriteRenderVertex pool[64]{};
    std::vector<std::unique_ptr<SpriteRenderVertex*[]>> pointer_storage;
    std::vector<std::unique_ptr<SpriteDeviceVertex[]>> vertex_storage;
    std::vector<std::vector<std::uint64_t>> trace;
    void(*pull)(Fixture&)=nullptr;
    std::uint64_t random(){rng^=rng<<13;rng^=rng>>7;rng^=rng<<17;return rng;}
    static void put(SpriteRenderVertex& v,unsigned off,double d){std::memcpy(reinterpret_cast<std::byte*>(&v)+off,&d,8);}
    explicit Fixture(unsigned i):trial(i),rng(0xed289472c67f013bull+i){
        std::memset(&model,0xa5,sizeof(model));model.texture_id_1a8=1;int mode=static_cast<int>((i/32)%7)-1;std::memcpy(model.unknown_00.data(),&mode,4);
        for(auto& v:model.vertices_08){for(unsigned j=0;j<11;++j)put(v,j*8,static_cast<int>(random()%512)-128.25);put(v,0x10,(random()%100+1)/16.0);v.x_18=static_cast<int>(random()%65)-32;v.y_20=static_cast<int>(random()%65)-32;put(v,0x28,(random()%1024)/1024.0);put(v,0x30,(random()%1024)/1024.0);}
        if(i%8==1)for(auto& v:model.vertices_08){v.x_18/=8;v.y_20/=8;}
        if(i%8==2)for(auto& v:model.vertices_08)v.y_20=100;
        if(i%8==3)for(auto& v:model.vertices_08)v.y_20=-100;
        if(i%8==4)for(auto& v:model.vertices_08)v.x_18=100;
        if(i%8==5)for(auto& v:model.vertices_08)v.x_18=-100;
        if(i%8==6){model.vertices_08[0].x_18=NAN;model.vertices_08[2].y_20=NAN;}
        if(i%8==7)for(auto& v:model.vertices_08)put(v,0x10,0);
        texture.device_texture_00=reinterpret_cast<void*>(0x12345678);texture.flags_24=i%32;node={&texture,0,7};entries[1].cached_80=&node;cache.entries_30=entries;
        submission.device={reinterpret_cast<void*>(0x1100),(i/224)%2?&texture:nullptr,0x80000000,127,9};submission.initialized_flags_5d6160=0xcafebaa0;submission.vertices_5d6170={0,0,1,nullptr};
        clipping={-8,8,8,-8,{0,0,static_cast<int>((i/448)%4)+1,nullptr},{{1,2,3},pool,{0,0,1,nullptr},0}};
        drawing={{0,0,1,nullptr},0xfedcba98,0.0005,-0.00075};std::memset(pool,0xa5,sizeof(pool));
    }
    unsigned pointer(const void* p)const{if(!p)return 0;if(p==&texture)return 1;for(unsigned i=0;i<4;++i)if(p==&model.vertices_08[i])return 10+i;for(unsigned i=0;i<64;++i)if(p==pool+i)return 100+i;for(unsigned i=0;i<pointer_storage.size();++i)if(p==pointer_storage[i].get())return 1000+i;for(unsigned i=0;i<vertex_storage.size();++i)if(p==vertex_storage[i].get())return 2000+i;return static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(p));}
    SpriteRenderVertex** allocate_pointers(unsigned bytes){trace.push_back({1,bytes});auto p=std::make_unique<SpriteRenderVertex*[]>(bytes/4);auto* result=p.get();pointer_storage.push_back(std::move(p));return result;}
    SpriteDeviceVertex* allocate_vertices(unsigned bytes){trace.push_back({2,bytes});auto p=std::make_unique<SpriteDeviceVertex[]>(bytes/32);std::memset(p.get(),0x5a,bytes);auto* result=p.get();vertex_storage.push_back(std::move(p));return result;}
    void release(void* p){trace.push_back({3,pointer(p)});}
    void draw(void* device,unsigned primitive,unsigned format,SpriteDeviceVertex* vertices,unsigned count,unsigned flags){
        if(pull)pull(*this);std::vector<std::uint64_t> event={9,pointer(device),primitive,format,pointer(vertices),count,flags};
        for(unsigned i=0;i<count;++i){const auto& v=vertices[i];auto number=[](float f){return std::isnan(f)?0x7fc00000u:std::bit_cast<unsigned>(f);};for(auto n:{number(v.x_00),number(v.y_04),number(v.z_08),number(v.rhw_0c),v.color_10,v.unknown_14,number(v.u_18),number(v.v_1c)})event.push_back(n);}trace.push_back(std::move(event));
    }
    SpriteDrawHooks draw_hooks(){return {this,[](void* p,unsigned bytes){return static_cast<Fixture*>(p)->allocate_vertices(bytes);},[](void* p,SpriteDeviceVertex* v){static_cast<Fixture*>(p)->release(v);},[](void* p){static_cast<Fixture*>(p)->trace.push_back({4,0x46d605});},[](void* p,void* d,unsigned primitive,unsigned format,SpriteDeviceVertex* v,unsigned count,unsigned flags){static_cast<Fixture*>(p)->draw(d,primitive,format,v,count,flags);}};}
    SpriteClipHooks clip_hooks(){return {this,[](void* p,unsigned bytes){return static_cast<Fixture*>(p)->allocate_pointers(bytes);},[](void* p,SpriteRenderVertex** v){static_cast<Fixture*>(p)->release(v);},[](void* p,SpriteVertexPointerArray& input){auto& f=*static_cast<Fixture*>(p);draw_sprite_polygon(input,f.submission.device,f.drawing,f.draw_hooks());}};}
    SpriteSubmissionHooks submit_hooks(){SpriteSubmissionHooks h{};h.context=this;h.texture_view=[](void*,void* p){auto& t=*static_cast<Texture*>(p);return SpriteTextureBindingView{p,t.device_texture_00,t.flags_24};};
        h.device={this,[](void* p,void* d,unsigned stage,void* t){auto& f=*static_cast<Fixture*>(p);f.trace.push_back({5,f.pointer(d),stage,f.pointer(t)});},[](void* p,void* d,unsigned stage,unsigned type,unsigned value){auto& f=*static_cast<Fixture*>(p);f.trace.push_back({6,f.pointer(d),stage,type,value});},[](void* p,void* d,unsigned type,unsigned value){auto& f=*static_cast<Fixture*>(p);f.trace.push_back({7,f.pointer(d),type,value});}};
        h.allocate_pointers=[](void* p,unsigned bytes){return static_cast<Fixture*>(p)->allocate_pointers(bytes);};h.free_pointers=[](void* p,SpriteRenderVertex** v){static_cast<Fixture*>(p)->release(v);};h.register_cleanup_49cb29=[](void* p){static_cast<Fixture*>(p)->trace.push_back({4,0x4150a0});};
        h.clip_and_draw_46d3c1=[](void* p,SpriteVertexPointerArray& input){auto& f=*static_cast<Fixture*>(p);f.clipping.scratch_58d578.used_20=f.submission.scratch_used_58d598;f.clipping.scratch_58d578.available_10.count_00=static_cast<int>(f.submission.scratch_count_58d588);clip_and_draw_sprite_polygon(input,f.clipping,f.clip_hooks());f.submission.scratch_used_58d598=f.clipping.scratch_58d578.used_20;f.submission.scratch_count_58d588=static_cast<unsigned>(f.clipping.scratch_58d578.available_10.count_00);};return h;
    }
    void run(){ResourceCacheHooks h{this,[](void*,int)->ResourceCacheNode*{std::abort();},[](void*,int)->void*{std::abort();},nullptr};submit_sprite_geometry(model,submission,cache,h,submit_hooks());}
    std::vector<std::uint64_t> snapshot()const{auto& s=submission;auto& d=s.device;std::vector<std::uint64_t> r={pointer(d.active_texture_5cd5f4),d.texture_flags_4c24e4,static_cast<unsigned>(d.blend_mode_4c227c),s.scratch_count_58d588,s.scratch_used_58d598,s.initialized_flags_5d6160,drawing.initialized_flags_58d550,node.references_08};
        for(auto* a:{&s.vertices_5d6170,&clipping.temporary_58d558}){r.push_back(a->count_00);r.push_back(a->capacity_04);r.push_back(a->growth_08);r.push_back(pointer(a->data_0c));for(int i=0;i<a->count_00;++i)r.push_back(pointer(a->data_0c[i]));}
        auto& a=drawing.vertices_58d540;r.insert(r.end(),{static_cast<unsigned>(a.count_00),static_cast<unsigned>(a.capacity_04),static_cast<unsigned>(a.growth_08),pointer(a.data_0c)});return r;
    }
    bool same(const Fixture& f)const{return trace==f.trace&&snapshot()==f.snapshot()&&std::memcmp(&model,&f.model,sizeof(model))==0;}
};
}
