#pragma once
#include "lemmings/resource_cache_load.hpp"
#include <array>
#include <bit>
#include <cstdlib>
#include <vector>

namespace cache_load_test {
using namespace lemmings;
struct State {
    ResourceCacheLookupManager manager;
    std::uint32_t loaded_count = 0;
    std::uint32_t entry_count=0,entry_capacity=0,entry_growth=0;
    ResourceCacheEntry* entry_data=nullptr;
};
struct Fixture {
    State state;
    std::array<ResourceCacheLookupNode,8> nodes;
    ResourceCacheNode source;
    std::vector<std::uint32_t> trace;
    unsigned trial, operation;
    std::int32_t index;
    unsigned length;
    bool alias;
    Fixture(unsigned t):trial(t),operation(t%5),index(std::bit_cast<std::int32_t>(t*2654435761u)),length((t/5)%6),alias((t/30)%2) {
        const std::uint32_t counts[]{0,1,0xffffffffu,0x7fffffffu,0x80000000u};
        state.loaded_count=counts[(t/60)%5];
        for(unsigned i=0;i<nodes.size();++i) {
            nodes[i].value={reinterpret_cast<void*>(std::uintptr_t(0x12340000u+i)),t+i+1,counts[(t/300+i)%5]};
            nodes[i].next_0c=&state.manager.list_04.end;
            nodes[i].previous_10=&state.manager.list_04.begin;
        }
        link(0);
        // Some loads find a preexisting node; repeated IDs select the first.
        if(length && (t/1500)%3) {
            nodes[length-1].value.unknown_04=std::bit_cast<std::uint32_t>(index);
            if((t/4500)%2)nodes[0].value.unknown_04=std::bit_cast<std::uint32_t>(index);
        }
        source={reinterpret_cast<void*>(std::uintptr_t(0x23456789)),std::bit_cast<std::uint32_t>(index),counts[(t/150+i_salt)%5]};
    }
    static constexpr unsigned i_salt=3;
    void link(unsigned skip) {
        auto& list=state.manager.list_04;
        list.begin.next_0c=length>skip?&nodes[skip]:&list.end;
        for(unsigned i=skip;i<length;++i) {
            nodes[i].next_0c=i+1<length?&nodes[i+1]:&list.end;
            nodes[i].previous_10=i==skip?&list.begin:&nodes[i-1];
        }
        list.end.previous_10=length>skip?&nodes[length-1]:&list.begin;
    }
    void* create(std::int32_t id) {
        trace.push_back(10);trace.push_back(std::bit_cast<std::uint32_t>(id));
        if(trial&1)link(1);
        if(trial&2)state.loaded_count^=0xfedcba98;
        if(trial&4)nodes[0].value.references_08+=7;
        return (trial&8)?nullptr:reinterpret_cast<void*>(std::uintptr_t(0x34567890));
    }
    ResourceCacheLookupNode* allocate() {
        trace.push_back(20);
        if(trial&16)link(0);
        if(trial&32)state.loaded_count=0xffffffff;
        return operation==2 && (trial&64)?nullptr:&nodes[7];
    }
    ResourceCacheLoadHooks hooks() {
        return {this,[](void* p,std::int32_t id){return static_cast<Fixture*>(p)->create(id);},
            [](void* p){return static_cast<Fixture*>(p)->allocate();}};
    }
    void* run() {
        auto h=hooks();
        if(operation==0||operation==1)return load_cached_resource(state.manager,state.loaded_count,index,h);
        if(operation==2) {prepend_resource_cache_node(state.manager,source,h);return nullptr;}
        if(operation==3) {construct_resource_cache_node(nodes[7],alias?nodes[7].value:source,&nodes[2],&nodes[3]);return &nodes[7];}
        return initialize_resource_cache_value(nodes[7].value);
    }
    unsigned node_id(const ResourceCacheLookupNode* p) const {
        if(!p)return 0;
        if(p==&state.manager.list_04.begin)return 1;
        if(p==&state.manager.list_04.end)return 2;
        for(unsigned i=0;i<nodes.size();++i)if(p==&nodes[i])return i+3;
        std::abort();
    }
    std::uintptr_t result_id(void* p) const {
        if(p==&nodes[7]||p==&nodes[7].value)return 7;
        return reinterpret_cast<std::uintptr_t>(p);
    }
    std::vector<std::uintptr_t> snapshot() const {
        std::vector<std::uintptr_t> result{state.loaded_count};
        auto add=[&](const ResourceCacheLookupNode& n){
            result.push_back(reinterpret_cast<std::uintptr_t>(n.value.resource_00));
            result.push_back(n.value.unknown_04);result.push_back(n.value.references_08);
            result.push_back(node_id(n.next_0c));result.push_back(node_id(n.previous_10));
        };
        add(state.manager.list_04.begin);add(state.manager.list_04.end);
        for(const auto& n:nodes)add(n);
        result.push_back(reinterpret_cast<std::uintptr_t>(source.resource_00));
        result.push_back(source.unknown_04);result.push_back(source.references_08);
        return result;
    }
};
struct ComposedFixture:Fixture {
    std::array<ResourceCacheEntry,6> entries;
    ResourceCache cache;
    unsigned combined_operation;
    ComposedFixture(unsigned t):Fixture(t*5),combined_operation(t%3) {
        index=t%6;
        state.entry_data=entries.data();state.entry_count=state.entry_capacity=6;
        cache.entries_30=entries.data();
        for(unsigned i=0;i<6;++i) {
            entries[i].name_00.fill(static_cast<char>('A'+t%26));
            entries[i].name_00[0]=static_cast<char>('a'+i);
            entries[i].name_00[(t/6)%128]='\0';
            entries[i].unknown_84=0x87654321;
        }
        for(unsigned i=0;i<nodes.size();++i)nodes[i].value.unknown_04=std::bit_cast<std::uint32_t>(index+10);
        if(length && t%3)nodes[length-1].value.unknown_04=std::bit_cast<std::uint32_t>(index);
        if(t%4==0 || combined_operation==2)entries[index].cached_80=&nodes[0].value;
    }
    void notify(const char* name) {trace.push_back(100+static_cast<unsigned char>(*name));}
    void* named_create(const char* name) {
        if(name!=entries[index].name_00.data())std::abort();
        trace.push_back(400+static_cast<unsigned char>(*name));
        return create(*name-'a');
    }
    void* run_combined() {
        ResourceCacheHooks callbacks{this,
            [](void* p,std::int32_t id){auto& f=*static_cast<ComposedFixture*>(p);return find_cached_resource(f.state.manager,id);},
            [](void* p,std::int32_t id){
                auto& f=*static_cast<ComposedFixture*>(p);auto h=f.hooks();
                if(f.trial&128)h.create=+[](void* q,std::int32_t i){
                    auto& fixture=*static_cast<ComposedFixture*>(q);
                    return create_resource_by_index(fixture.cache,i,{&fixture,
                        [](void* context,const char* name){return static_cast<ComposedFixture*>(context)->named_create(name);}});
                };
                return load_cached_resource(f.state.manager,f.state.loaded_count,id,h);
            },
            +[](void* p,const char* name){static_cast<ComposedFixture*>(p)->notify(name);}};
        if(combined_operation==0)return acquire_resource(cache,index,callbacks);
        if(combined_operation==1)return get_resource_balanced(cache,index,callbacks);
        release_resource(cache,index);return nullptr;
    }
    std::vector<std::uintptr_t> combined_snapshot() const {
        auto result=snapshot();
        for(const auto& entry:entries) {
            result.push_back(node_id(reinterpret_cast<const ResourceCacheLookupNode*>(entry.cached_80)));
            result.push_back(entry.name_00[0]);result.push_back(entry.unknown_84);
        }
        return result;
    }
};
} // namespace cache_load_test
