#pragma once
#include "lemmings/resource_cache_cleanup.hpp"
#include <array>
#include <bit>
#include <cstring>
#include <vector>

namespace cleanup_test {
using namespace lemmings;
struct Manager {
    ResourceCacheLookupManager lookup;
    std::uint32_t loaded_count = 0;
    std::int32_t entry_count = 0;
    std::uint32_t capacity = 8, growth = 1;
    ResourceCache cache;
};
#if INTPTR_MAX == INT32_MAX
static_assert(offsetof(Manager,loaded_count)==0x2c);
static_assert(offsetof(Manager,entry_count)==0x30);
static_assert(offsetof(Manager,cache)==0x3c);
#endif
struct Event {
    unsigned kind;
    std::uint64_t argument;
    std::vector<std::uint64_t> state;
    bool operator==(const Event&) const = default;
};
struct Fixture {
    Manager manager;
    std::array<ResourceCacheLookupNode,8> nodes;
    std::array<ResourceCacheEntry,8> entries_a,entries_b;
    ResourceCacheCleanupHooks hooks;
    std::vector<Event> trace;
    unsigned script,operation,live_nodes,freed=0,callback_count=0,variant=0;
    bool mutated_destroy=false,mutated_create=false;
    void (*on_switch)(Fixture&)=nullptr;
    static void* create0(void* p,std::int32_t id){return static_cast<Fixture*>(p)->create(id,0);}
    static void* create1(void* p,std::int32_t id){return static_cast<Fixture*>(p)->create(id,1);}
    static void destroy0(void* p,void* value){static_cast<Fixture*>(p)->destroy(value,0);}
    static void destroy1(void* p,void* value){static_cast<Fixture*>(p)->destroy(value,1);}
    static void release(void* p,ResourceCacheLookupNode* node){static_cast<Fixture*>(p)->free_node(node);}
    Fixture(unsigned seed,unsigned script_value,unsigned operation_value)
        :script(script_value),operation(operation_value),live_nodes(seed%9) {
        if(operation==6 && live_nodes==0)live_nodes=1;
        hooks={this,&create0,&destroy0,&release};
        manager.loaded_count=seed%3==0?0:(seed%3==1?0xffffffffu:live_nodes);
        manager.entry_count=seed%7==0?-3:(seed%7==1?0:8);
        manager.capacity=0x87654321;manager.growth=0x13572468;
        manager.cache.entries_30=entries_a.data();
        auto& list=manager.lookup.list_04;
        list.begin.value={reinterpret_cast<void*>(0x111100),0x12345678,0x87654321};
        list.end.value={reinterpret_cast<void*>(0x222200),0x34567812,0x76543218};
        list.begin.next_0c=live_nodes?&nodes[0]:&list.end;
        list.end.previous_10=live_nodes?&nodes[live_nodes-1]:&list.begin;
        for(unsigned i=0;i<nodes.size();++i){
            auto& n=nodes[i];
            n.value.resource_00=(seed+i)%4==0?nullptr:reinterpret_cast<void*>(0x1000+i*0x10);
            n.value.unknown_04=(seed%5==0)?i%3:((seed%11==0)?0xffffffffu:i);
            n.value.references_08=(seed+i)%3==0?0:((seed+i)%3==1?1:0xffffffffu);
            n.next_0c=i+1<live_nodes?&nodes[i+1]:&list.end;
            n.previous_10=i?&nodes[i-1]:&list.begin;
        }
        for(unsigned i=0;i<8;++i){
            entries_a[i].name_00.fill(static_cast<char>(i+1));entries_b[i].name_00.fill(static_cast<char>(i+33));
            entries_a[i].cached_80=(seed+i)%4==0?nullptr:&nodes[i].value;
            entries_b[i].cached_80=&nodes[7-i].value;
            entries_a[i].unknown_84=0xa1b2c300u|((seed+i)%3==0?0u:0x80u);
            entries_b[i].unknown_84=0xb2c3d401u;
        }
    }
    int node_id(const ResourceCacheLookupNode* node)const {
        if(!node)return -3;
        if(node==&manager.lookup.list_04.begin)return -1;
        if(node==&manager.lookup.list_04.end)return -2;
        for(unsigned i=0;i<nodes.size();++i)if(node==&nodes[i])return static_cast<int>(i);
        return -4;
    }
    int value_id(const ResourceCacheNode* value)const {
        if(!value)return -3;
        for(unsigned i=0;i<nodes.size();++i)if(value==&nodes[i].value)return static_cast<int>(i);
        return -4;
    }
    std::vector<std::uint64_t> snapshot()const {
        std::vector<std::uint64_t> out{manager.loaded_count,std::bit_cast<std::uint32_t>(manager.entry_count),
            manager.capacity,manager.growth,manager.cache.entries_30==entries_b.data(),freed,variant};
        auto append=[&](const ResourceCacheLookupNode& node){
            out.push_back(reinterpret_cast<std::uintptr_t>(node.value.resource_00));
            out.push_back(node.value.unknown_04);out.push_back(node.value.references_08);
            out.push_back(static_cast<unsigned>(node_id(node.next_0c)));out.push_back(static_cast<unsigned>(node_id(node.previous_10)));
        };
        append(manager.lookup.list_04.begin);append(manager.lookup.list_04.end);
        for(const auto& node:nodes)append(node);
        for(const auto* array:{&entries_a,&entries_b})for(const auto& entry:*array){
            out.push_back(static_cast<unsigned>(value_id(entry.cached_80)));out.push_back(entry.unknown_84);
            std::uint32_t hash=0;for(unsigned char c:entry.name_00)hash=hash*31+c;out.push_back(hash);
        }
        return out;
    }
    void event(unsigned kind,std::uint64_t argument){trace.push_back({kind,argument,snapshot()});++callback_count;}
    void common_mutation(){
        if(script&1)manager.loaded_count+=17;
        if(script&8){manager.cache.entries_30=entries_b.data();manager.entry_count=5;}
        if(script&128){variant=1;hooks.create_0c=&create1;hooks.destroy_10=&destroy1;if(on_switch)on_switch(*this);}
    }
    void destroy(void* resource,unsigned dispatch){
        event(1+dispatch,reinterpret_cast<std::uintptr_t>(resource));common_mutation();
        if(!mutated_destroy){
            mutated_destroy=true;
            if((script&2)&&live_nodes>=3){nodes[0].next_0c=&nodes[2];nodes[2].previous_10=&nodes[0];}
            if(script&32)nodes[0].value.resource_00=reinterpret_cast<void*>(0x987600);
        }
    }
    void free_node(ResourceCacheLookupNode* node){
        const int id=node_id(node);event(3,static_cast<unsigned>(id));
        if(id>=0)freed|=1u<<id;
        if(script&4)nodes[1].value.references_08=0;
        if(script&16){node->next_0c=&manager.lookup.list_04.end;manager.loaded_count+=3;}
    }
    void* create(std::int32_t id,unsigned dispatch){
        event(4+dispatch,std::bit_cast<std::uint32_t>(id));common_mutation();
        if(!mutated_create){mutated_create=true;if(script&64)nodes[0].next_0c=&manager.lookup.list_04.end;}
        nodes[0].value.resource_00=reinterpret_cast<void*>(0xabc000);
        return (static_cast<std::uint32_t>(id)+script)%3==0?nullptr:
            reinterpret_cast<void*>(0x2000+(static_cast<std::uint32_t>(id)&255u)*16+dispatch);
    }
    void run(){
        auto& m=manager;
        switch(operation){
        case 0:evict_unused_resources(m.lookup,m.loaded_count,hooks);break;
        case 1:unload_cached_resources(m.lookup,m.loaded_count,hooks);break;
        case 2:recreate_cached_resources(m.lookup,m.loaded_count,hooks);break;
        case 3:invalidate_and_unload_resources(m.lookup,m.loaded_count,m.cache,m.entry_count,hooks);break;
        case 4:recreate_and_refresh_resources(m.lookup,m.loaded_count,m.cache,m.entry_count,hooks);break;
        case 5:clear_unused_resource_flags_and_evict(m.lookup,m.loaded_count,m.cache,m.entry_count,hooks);break;
        case 6:unlink_resource_cache_node(nodes[0],hooks);break;
        }
    }
};
} // namespace cleanup_test
