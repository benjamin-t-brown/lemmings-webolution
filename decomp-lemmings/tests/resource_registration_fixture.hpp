#pragma once
#include "lemmings/resource_registration.hpp"
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace registration_test {
using namespace lemmings;
struct Block {
    std::uint32_t cookie=24;
    std::array<ResourceCacheEntry,24> entries;
};
struct Fixture {
    Block old_block,alternate_block,new_block;
    ResourceCache cache;
    ResourceRegistration state{cache};
    ResourceCacheEntry source;
    std::array<char,164> input{};
    alignas(void*) std::array<unsigned char,64> raw_manager{};
    std::vector<std::uint64_t> trace;
    unsigned trial,mode,operation,alignment;
    bool native;
    explicit Fixture(unsigned t,bool n=false):trial(t),mode((t/2)%8),operation(t%2),alignment((t/16)%4),native(n) {
        unsigned seed=t+1;
        auto random=[&]{seed=seed*1664525u+1013904223u;return seed;};
        for (auto* block:{&old_block,&alternate_block,&new_block}) {
            for (auto& entry:block->entries) {
                for (auto& ch:entry.name_00)ch=static_cast<char>(1+random()%255);
                unsigned length=random()%128;
                entry.name_00[length]=0;
                entry.cached_80=reinterpret_cast<ResourceCacheNode*>(std::uintptr_t(0x12340000u+(random()%32)*4));
                entry.unknown_84=(random()&0xffffff00u)|(1+random()%255);
            }
        }
        source=old_block.entries[15];
        const unsigned length=(t/64)%128;
        for (unsigned i=0;i<length;++i)input[alignment+i]=static_cast<char>(1+random()%255);
        input[alignment+length]=0;
        state.count_30=static_cast<std::int32_t>((t/2048)%7);
        state.capacity_34=state.count_30+static_cast<std::int32_t>((t/1024)%3);
        state.growth_38=1+static_cast<std::int32_t>((t/3072)%4);
        if(mode==3)state.growth_38=std::max(2,state.growth_38);
        const auto hole=(t/256)%8;
        if (hole<static_cast<unsigned>(state.count_30))old_block.entries[hole].unknown_84&=0xffffff00u;
        cache.entries_30=(state.count_30==0 && t%3==0)?nullptr:old_block.entries.data();
        if(!cache.entries_30)state.capacity_34=0;
        sync_to_raw();
    }
    void sync_to_raw() {
        if (!native)return;
        std::memcpy(raw_manager.data()+0x30,&state.count_30,4);
        std::memcpy(raw_manager.data()+0x34,&state.capacity_34,4);
        std::memcpy(raw_manager.data()+0x38,&state.growth_38,4);
        std::memcpy(raw_manager.data()+0x3c,&cache.entries_30,4);
    }
    void sync_from_raw() {
        if (!native)return;
        std::memcpy(&state.count_30,raw_manager.data()+0x30,4);
        std::memcpy(&state.capacity_34,raw_manager.data()+0x34,4);
        std::memcpy(&state.growth_38,raw_manager.data()+0x38,4);
        std::memcpy(&cache.entries_30,raw_manager.data()+0x3c,4);
    }
    unsigned storage_id(ResourceCacheEntry* p)const {
        if (!p)return 0;
        if(p==old_block.entries.data())return 1;
        if(p==alternate_block.entries.data())return 2;
        if(p==new_block.entries.data())return 3;
        std::abort();
    }
    void record(unsigned event) {
        trace.push_back(event);trace.push_back(static_cast<std::uint32_t>(state.count_30));
        trace.push_back(static_cast<std::uint32_t>(state.capacity_34));
        trace.push_back(static_cast<std::uint32_t>(state.growth_38));
        trace.push_back(storage_id(cache.entries_30));
    }
    ResourceEntryAllocation allocate(std::uint32_t bytes) {
        sync_from_raw();record(1);trace.push_back(bytes);
        if(mode==1||mode==4){state.count_30=std::max(0,state.count_30-1);state.growth_38=17;cache.entries_30=alternate_block.entries.data();}
        if(mode==2){cache.entries_30=alternate_block.entries.data();source.name_00[0]='!';source.name_00[1]=0;source.cached_80=nullptr;}
        if(mode==5)state.capacity_34=-4;
        if(mode==6){state.count_30=0;cache.entries_30=nullptr;}
        // Registration must have already copied the requested name into its temporary.
        input[alignment]='?';input[alignment+1]=0;
        sync_to_raw();return {new_block.entries.data(),&new_block.cookie};
    }
    void deallocate(ResourceCacheEntry* entries) {
        sync_from_raw();record(2);trace.push_back(storage_id(entries));trace.push_back(new_block.cookie);
        // Captures initialization and copying before release.
        for(unsigned i=0;i<new_block.cookie;++i){trace.push_back(new_block.entries[i].unknown_84);trace.push_back(static_cast<unsigned char>(new_block.entries[i].name_00[0]));}
        if(mode==3){++state.count_30;state.growth_38=19;state.capacity_34=21;cache.entries_30=alternate_block.entries.data();source.name_00[0]='#';source.name_00[1]=0;}
        if(mode==4){state.count_30=0;new_block.entries[0].unknown_84=0x87654300;new_block.entries[1].name_00[0]='@';}
        if(mode==7){state.count_30=0;cache.entries_30=alternate_block.entries.data();}
        sync_to_raw();
    }
    ResourceRegistrationHooks hooks() {
        return {this,[](void* p,std::uint32_t n){return static_cast<Fixture*>(p)->allocate(n);},
            [](void* p,ResourceCacheEntry* e){static_cast<Fixture*>(p)->deallocate(e);}};
    }
    int run() {
        auto h=hooks();
        if(operation==0)return register_resource_slot(state,input.data()+alignment,h);
        append_resource_entry(state,source,h);return -1;
    }
    bool equals(Fixture& other) {
        sync_from_raw();other.sync_from_raw();
        if(state.count_30!=other.state.count_30||state.capacity_34!=other.state.capacity_34||state.growth_38!=other.state.growth_38||storage_id(cache.entries_30)!=other.storage_id(other.cache.entries_30))return false;
        const Block* left[]={&old_block,&alternate_block,&new_block};
        const Block* right[]={&other.old_block,&other.alternate_block,&other.new_block};
        for(unsigned b=0;b<3;++b){
            if(left[b]->cookie!=right[b]->cookie)return false;
            for(unsigned i=0;i<24;++i){const auto& a=left[b]->entries[i];const auto& c=right[b]->entries[i];
                if(a.name_00!=c.name_00||a.cached_80!=c.cached_80||a.unknown_84!=c.unknown_84)return false;
            }
        }
        return true;
    }
};
}
