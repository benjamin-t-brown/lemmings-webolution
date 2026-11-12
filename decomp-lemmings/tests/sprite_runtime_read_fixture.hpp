#pragma once
#include "lemmings/sprite_runtime_read.hpp"
#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include <iostream>

namespace sprite_read_test {
using namespace lemmings;
inline std::size_t host_stride(unsigned kind) {
    switch(kind){case 0:return sizeof(SpriteDestructionClip);case 1:return sizeof(SpriteDestructionFrame);
    case 2:return sizeof(SpriteDestructionPiece);case 3:return sizeof(SpriteDestructionTripleArray);
    case 4:return 24;default:return sizeof(ResourceCacheEntry);}
}
inline std::uint32_t original_stride(unsigned kind) {
    constexpr std::uint32_t sizes[]={152,16,72,8,24,136};return sizes[kind];
}
struct Block {
    unsigned kind;std::uint32_t count,bytes;std::size_t payload;
    std::unique_ptr<std::byte[]> owner;std::byte* elements;bool freed=false;
};
struct Fixture {
    SpriteFactoryResource resource;
    ResourceCache textures{};
    ResourceRegistration registration{textures};
    std::vector<Block> blocks;
    std::vector<std::uint64_t> trace;
    std::size_t cursor=0;std::size_t* live_cursor=&cursor;
    void* global_manager=nullptr;
    explicit Fixture(unsigned growth=1) {
        initialize_sprite_resource(resource);resource.clips_00.growth_08=growth;
        resource.triple_arrays_10.growth_08=growth;registration.growth_38=3;
    }
    std::uint32_t pointer_id(const void* p)const {
        if(!p)return 0;
        for(unsigned i=0;i<blocks.size();++i) {
            if(p==blocks[i].elements)return 2+i*2;
            if(p==blocks[i].elements-4)return 3+i*2;
        }
        return 0xffffffff;
    }
    void sync_from_global() {
        if(!global_manager)return;
        auto* p=static_cast<std::byte*>(global_manager);
        std::memcpy(&registration.count_30,p+0x30,4);std::memcpy(&registration.capacity_34,p+0x34,4);
        std::memcpy(&registration.growth_38,p+0x38,4);std::memcpy(&textures.entries_30,p+0x3c,4);
    }
    void sync_to_global() {
        if(!global_manager)return;
        auto* p=static_cast<std::byte*>(global_manager);
        std::memset(p,0,64);
        std::memcpy(p+0x30,&registration.count_30,4);std::memcpy(p+0x34,&registration.capacity_34,4);
        std::memcpy(p+0x38,&registration.growth_38,4);std::memcpy(p+0x3c,&textures.entries_30,4);
    }
    void record(unsigned event,unsigned kind,std::uint32_t count,std::uint32_t bytes) {
        sync_from_global();trace.insert(trace.end(),{event,kind,count,bytes,*live_cursor,
            static_cast<std::uint32_t>(registration.count_30),static_cast<std::uint32_t>(registration.capacity_34)});
        for(const auto* a:{&resource.clips_00,&resource.triple_arrays_10})
            trace.insert(trace.end(),{a->count_00,a->capacity_04,a->growth_08,pointer_id(a->data_0c)});
        // Texture registration must observe numeric piece writes before the
        // final ID store. The latest live piece allocation contains that view.
        std::uint64_t hash=14695981039346656037ULL;
        for(auto i=blocks.rbegin();i!=blocks.rend();++i)if(i->kind==2&&!i->freed) {
            for(std::size_t j=0;j<i->payload;++j)hash=(hash^std::to_integer<unsigned char>(i->elements[j]))*1099511628211ULL;
            break;
        }
        trace.push_back(hash);
    }
    void* allocate(unsigned kind,std::uint32_t count,std::uint32_t bytes) {
        record(1,kind,count,bytes);
        const auto payload=host_stride(kind)*count;
        auto owner=std::make_unique<std::byte[]>(payload+32);
        auto* elements=reinterpret_cast<std::byte*>((reinterpret_cast<std::uintptr_t>(owner.get())+19)&~std::uintptr_t(15));
        std::memset(elements-4,0xa5,payload+4);
        blocks.push_back({kind,count,bytes,payload,std::move(owner),elements,false});return elements;
    }
    void release(void* allocation) {
        for(auto& b:blocks)if(allocation==(b.kind==4?b.elements:b.elements-4)) {
            if(b.freed)throw std::runtime_error("double free");
            record(2,b.kind,b.count,b.bytes);b.freed=true;return;
        }
        throw std::runtime_error("unknown free");
    }
    SpriteRuntimeReadHooks read_hooks() {
        return {this,[](void* p,SpriteReadAllocationKind k,std::uint32_t n,std::uint32_t bytes){return static_cast<Fixture*>(p)->allocate(static_cast<unsigned>(k),n,bytes);},
            [](void* p,void* a){static_cast<Fixture*>(p)->release(a);}};
    }
    ResourceRegistrationHooks texture_hooks() {
        return {this,[](void* p,std::uint32_t bytes){auto* elements=static_cast<ResourceCacheEntry*>(static_cast<Fixture*>(p)->allocate(5,(bytes-4)/136,bytes));
                return ResourceEntryAllocation{elements,reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::byte*>(elements)-4)};},
            [](void* p,ResourceCacheEntry* e){static_cast<Fixture*>(p)->release(reinterpret_cast<std::byte*>(e)-4);}};
    }
    void run(std::span<const std::uint8_t> bytes) {
        LevelByteStream stream{bytes};live_cursor=&stream.cursor;
        auto rh=read_hooks();auto th=texture_hooks();read_runtime_sprite_resource(resource,stream,registration,th,rh);
        cursor=stream.cursor;live_cursor=&cursor;
    }
    std::vector<std::byte> normalized(const Block& b)const {
        std::vector<std::byte> result(b.elements-4,b.elements+b.payload);
        unsigned offset=0;
        if(b.kind==0||b.kind==1)offset=offsetof(SpriteRuntimeArray,data_0c);
        else if(b.kind==3)offset=offsetof(SpriteDestructionTripleArray,data_04);
        else if(b.kind==5)offset=offsetof(ResourceCacheEntry,cached_80);
        else return result;
        for(unsigned i=0;i<b.count;++i) {
            void* pointer=nullptr;std::memcpy(&pointer,b.elements+i*host_stride(b.kind)+offset,sizeof(pointer));
            const auto id=pointer_id(pointer);
            if(id!=0xffffffff) {
                std::memset(result.data()+4+i*host_stride(b.kind)+offset,0,sizeof(pointer));
                std::memcpy(result.data()+4+i*host_stride(b.kind)+offset,&id,4);
            }
        }
        return result;
    }
    bool equals(Fixture& other) {
        sync_from_global();other.sync_from_global();
        const auto same_header=[&](const SpriteRuntimeArray& a,const SpriteRuntimeArray& b) {
            return a.count_00==b.count_00&&a.capacity_04==b.capacity_04&&a.growth_08==b.growth_08&&pointer_id(a.data_0c)==other.pointer_id(b.data_0c);
        };
        if(!same_header(resource.clips_00,other.resource.clips_00)||!same_header(resource.triple_arrays_10,other.resource.triple_arrays_10)||
           registration.count_30!=other.registration.count_30||registration.capacity_34!=other.registration.capacity_34||
           registration.growth_38!=other.registration.growth_38||pointer_id(textures.entries_30)!=other.pointer_id(other.textures.entries_30)) {
            std::cerr<<"final headers mismatch\n";return false;
        }
        if(cursor!=other.cursor||trace!=other.trace||blocks.size()!=other.blocks.size()) {
            std::cerr<<"cursor/trace/block mismatch "<<cursor<<'/'<<other.cursor<<" traces "<<trace.size()<<'/'<<other.trace.size()<<" blocks "<<blocks.size()<<'/'<<other.blocks.size()<<'\n';return false;
        }
        for(unsigned i=0;i<blocks.size();++i) {
            if(blocks[i].freed!=other.blocks[i].freed||normalized(blocks[i])!=other.normalized(other.blocks[i])) {
                auto a=normalized(blocks[i]),b=other.normalized(other.blocks[i]);
                auto at=std::mismatch(a.begin(),a.end(),b.begin()).first-a.begin();
                std::cerr<<"block "<<i<<" kind "<<blocks[i].kind<<" differs at "<<at<<'\n';return false;
            }
        }
        return true;
    }
};
template<class T> void append(std::vector<std::uint8_t>& data,T value) {
    auto old=data.size();data.resize(old+sizeof(value));std::memcpy(data.data()+old,&value,sizeof(value));
}
inline void append_name(std::vector<std::uint8_t>& data,const std::string& name) {
    append(data,static_cast<std::uint32_t>(name.size()));data.insert(data.end(),name.begin(),name.end());
}
inline std::vector<std::uint8_t> synthetic(unsigned seed) {
    std::vector<std::uint8_t> data;const unsigned clips=seed%4;append(data,clips);
    for(unsigned i=0;i<clips;++i) {
        const unsigned frames=(seed+i)%5;append(data,frames);
        for(unsigned j=0;j<frames;++j) {
            const unsigned pieces=(seed+j)%4;append(data,pieces);
            for(unsigned k=0;k<pieces;++k) {
                for(unsigned x=0;x<4;++x)append(data,std::uint32_t(seed*1542+i*20+j*4+k+x));
                for(unsigned x=0;x<4;++x)append(data,std::uint64_t(0x7ff0000000000001ULL+seed+x));
                for(unsigned x=0;x<4;++x)append(data,std::uint32_t(~(seed*10+k+x)));
                append_name(data,"tex"+std::to_string((seed+j+k)%7));
            }
        }
        append_name(data,i==1?std::string(128,'q'):"clip"+std::to_string(i));
        append(data,std::uint64_t(0xfff0000000000001ULL+seed));
    }
    append(data,std::uint32_t(seed%4));
    for(unsigned i=0;i<seed%4;++i) {
        const std::uint32_t tag=(seed+i)%5;
        const std::uint32_t type=tag==4?0xffffffffu:tag;append(data,type);
        const auto count=type==0?16:type==1?256:0;
        for(int j=0;j<count*3;++j)append(data,std::uint64_t(0x7ff0000000000001ULL+seed+j));
    }
    return data;
}
}
