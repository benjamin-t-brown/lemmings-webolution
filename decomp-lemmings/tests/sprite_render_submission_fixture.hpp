#pragma once
#include "lemmings/sprite_render_submission.hpp"
#include <array>
#include <vector>
#include <cstring>
#include <cstdlib>
namespace sprite_submission_test {
using namespace lemmings;
struct Texture {void* device;std::array<std::byte,32> unknown;std::uint32_t flags;};
struct Fixture {
    unsigned trial,script,events=0,finds=0;bool loaded=false;
    SpriteRenderGeometry model{};Texture texture{};
    SpriteSubmissionState state{};ResourceCacheEntry entries[2]{},alternate[2]{};
    ResourceCacheNode node{},other{};ResourceCache cache{};
    SpriteRenderVertex* old_buffer[16]{};
    SpriteRenderVertex* new_buffer[16]{};
    SpriteRenderVertex* alternate_buffer[16]{};
    std::vector<std::array<std::uint64_t,14>> trace;
    void (*before)(Fixture&)=nullptr;void (*after)(Fixture&)=nullptr;
    explicit Fixture(unsigned i):trial(i),script((i/32)%16) {
        std::memset(&model,0xa5,sizeof(model));model.texture_id_1a8=1;
        int mode=static_cast<int>((i/512)%7)-1;std::memcpy(model.unknown_00.data(),&mode,4);
        texture.device=reinterpret_cast<void*>(0x12345678);texture.flags=i%32;
        state.device={reinterpret_cast<void*>(0x1100),nullptr,0x80000000u,127,9};
        if(script==1)state.device.active_texture_5cd5f4=&texture;
        if(script==2)state.device.texture_flags_4c24e4=texture.flags;
        state.scratch_count_58d588=123;state.scratch_used_58d598=456;
        state.initialized_flags_5d6160=(i/3584)%2?0xa5:0xa4;
        state.vertices_5d6170={2,2,static_cast<int>((i/7168)%3)+1,old_buffer};
        for(unsigned j=0;j<16;++j){old_buffer[j]=&model.vertices_08[j%4];new_buffer[j]=nullptr;alternate_buffer[j]=&model.vertices_08[(j+1)%4];}
        node={&texture,0x123,((i/21504)%2)?0xffffffffu:7};other={&texture,0x456,11};
        cache.entries_30=entries;alternate[1].cached_80=&other;
        if((i/43008)%3==0)entries[1].cached_80=&node;
    }
    std::uint64_t pointer(const void* p) const {
        if(!p)return 0;if(p==&texture)return 1;if(p==old_buffer)return 2;if(p==new_buffer)return 3;if(p==alternate_buffer)return 4;
        if(p==&node)return 5;if(p==&other)return 6;if(p==entries)return 7;if(p==alternate)return 8;
        for(unsigned j=0;j<4;++j)if(p==&model.vertices_08[j])return 10+j;
        return reinterpret_cast<std::uintptr_t>(p);
    }
    void event(unsigned kind,std::uint64_t a=0,std::uint64_t b=0,std::uint64_t c=0) {
        if(before)before(*this);
        auto& d=state.device;auto& v=state.vertices_5d6170;
        trace.push_back({kind,a,b,c,pointer(d.active_texture_5cd5f4),d.texture_flags_4c24e4,static_cast<std::uint32_t>(d.blend_mode_4c227c),static_cast<std::uint32_t>(v.count_00),static_cast<std::uint32_t>(v.capacity_04),pointer(v.data_0c),state.scratch_count_58d588,state.scratch_used_58d598,state.initialized_flags_5d6160,node.references_08});
        ++events;
        if(kind==1&&script==3)texture.flags=4;
        if(kind==1&&script==4){int mode=3;std::memcpy(model.unknown_00.data(),&mode,4);d.device_58cdb8=reinterpret_cast<void*>(0x2200);}
        if(kind==2&&b==3&&script==5)texture.flags=1;
        if(kind==3&&b==15&&script==6)texture.flags=4;
        if(kind==2&&b==5&&script==7)d.alpha_reference_4c24e0=201;
        if(kind==3&&b==27&&script==8)d.blend_mode_4c227c=77;
        if(kind==4&&script==9)v={3,6,1,alternate_buffer};
        if(kind==5&&script==10){v.count_00=1;v.data_0c=alternate_buffer;}
        if(kind==6&&script==11){v.count_00=0;v.data_0c=old_buffer;v.capacity_04=9;}
        if(kind==7&&script==12){state.scratch_count_58d588=91;v.data_0c[1]=&model.vertices_08[0];}
        if(kind==9&&script==13)cache.entries_30=alternate;
        if(kind<=3&&script==14)d.device_58cdb8=reinterpret_cast<void*>(events%2?0x2200:0x1100);
        if(kind==1&&script==15){texture.flags=0;d.texture_flags_4c24e4=0;}
        if(after)after(*this);
    }
    ResourceCacheNode* find(int id){event(8,id);++finds;return loaded||(trial/43008)%3==1?&node:nullptr;}
    void* load(int id){event(9,id);loaded=true;return &texture;}
    SpriteSubmissionHooks hooks() {
        SpriteSubmissionHooks h{};h.context=this;
        h.texture_view=[](void*,void* r){auto& t=*static_cast<Texture*>(r);return SpriteTextureBindingView{r,t.device,t.flags};};
        h.device={this,
            [](void* p,void* d,std::uint32_t stage,void* t){auto& f=*static_cast<Fixture*>(p);f.event(1,f.pointer(d),stage,f.pointer(t));},
            [](void* p,void* d,std::uint32_t stage,std::uint32_t type,std::uint32_t value){if(stage)std::abort();auto& f=*static_cast<Fixture*>(p);f.event(2,f.pointer(d),type,value);},
            [](void* p,void* d,std::uint32_t type,std::uint32_t value){auto& f=*static_cast<Fixture*>(p);f.event(3,f.pointer(d),type,value);}};
        h.register_cleanup_49cb29=[](void* p){static_cast<Fixture*>(p)->event(4);};
        h.allocate_pointers=[](void* p,std::uint32_t bytes){auto& f=*static_cast<Fixture*>(p);f.event(5,bytes);if(bytes>64)std::abort();return f.new_buffer;};
        h.free_pointers=[](void* p,SpriteRenderVertex** v){auto& f=*static_cast<Fixture*>(p);f.event(6,f.pointer(v));};
        h.clip_and_draw_46d3c1=[](void* p,SpriteVertexPointerArray& v){auto& f=*static_cast<Fixture*>(p);f.event(7,f.pointer(v.data_0c[0]),f.pointer(v.data_0c[1]),f.pointer(v.data_0c[2]));};return h;
    }
    ResourceCacheHooks cache_hooks(){return {this,[](void* p,int id){return static_cast<Fixture*>(p)->find(id);},[](void* p,int id){return static_cast<Fixture*>(p)->load(id);},nullptr};}
    void run(){submit_sprite_geometry(model,state,cache,cache_hooks(),hooks());}
    std::vector<std::uint64_t> snapshot() const {
        auto& d=state.device;auto& v=state.vertices_5d6170;
        std::vector<std::uint64_t> r={pointer(d.device_58cdb8),pointer(d.active_texture_5cd5f4),d.texture_flags_4c24e4,d.alpha_reference_4c24e0,static_cast<std::uint32_t>(d.blend_mode_4c227c),state.scratch_count_58d588,state.scratch_used_58d598,state.initialized_flags_5d6160,static_cast<std::uint32_t>(v.count_00),static_cast<std::uint32_t>(v.capacity_04),static_cast<std::uint32_t>(v.growth_08),pointer(v.data_0c),texture.flags,pointer(cache.entries_30),pointer(entries[1].cached_80),node.references_08,other.references_08,finds,loaded};
        for(auto p:old_buffer)r.push_back(pointer(p));for(auto p:new_buffer)r.push_back(pointer(p));for(auto p:alternate_buffer)r.push_back(pointer(p));
        return r;
    }
    bool same(const Fixture& f)const{return trace==f.trace&&snapshot()==f.snapshot()&&std::memcmp(&model,&f.model,sizeof(model))==0;}
};
}
