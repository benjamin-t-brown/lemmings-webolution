#pragma once
#include "lemmings/resource_table_load.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <string>
#include <vector>

namespace table_load_test {
using namespace lemmings;
struct Block {std::uint32_t cookie=24;std::array<ResourceCacheEntry,24> entries;};
struct Manager {
    ResourceCacheLookupManager lookup;
    std::uint32_t loaded=4;
    std::int32_t count=6,capacity=6,growth=1;
    ResourceCacheEntry* entries=nullptr;
};
struct Fixture {
    Manager manager;
    Block old_block,new_block;
    std::array<ResourceCacheLookupNode,4> nodes;
    ResourceCache cache;
    ResourceRegistration registration{cache};
    std::vector<std::uint8_t> bytes;
    LevelByteStream stream;
    std::vector<std::uint64_t> trace;
    bool native;
    unsigned trial;
    explicit Fixture(unsigned t,bool n=false):native(n),trial(t) {
        unsigned random=t+1;
        auto next=[&]{random=random*1664525u+1013904223u;return random;};
        for(auto* block:{&old_block,&new_block})for(auto& e:block->entries) {
            for(auto& ch:e.name_00)ch=static_cast<char>(1+next()%255);
            e.name_00[next()%128]=0;
            e.cached_80=reinterpret_cast<ResourceCacheNode*>(std::uintptr_t(0x12340000+(next()%16)*4));
            e.unknown_84=next();
        }
        manager.growth=1+static_cast<int>((t/8)%4);
        manager.entries=old_block.entries.data();
        for(unsigned i=0;i<6;++i) {
            auto name="old"+std::to_string(i);
            auto& e=old_block.entries[i];std::strcpy(e.name_00.data(),name.c_str());
            e.cached_80=i<4?&nodes[i].value:nullptr;
            e.unknown_84=(e.unknown_84&0xffffff00u)|(1+next()%255);
        }
        auto& list=manager.lookup.list_04;
        list.begin.next_0c=&nodes[0];list.end.previous_10=&nodes[3];
        for(unsigned i=0;i<4;++i) {
            auto& node=nodes[i];node.value.resource_00=(t%5==0&&i==1)?nullptr:reinterpret_cast<void*>(std::uintptr_t(0x2000+i*4));
            node.value.unknown_04=i;node.value.references_08=(t>>i)&1u;
            node.previous_10=i?&nodes[i-1]:&list.begin;
            node.next_0c=i==3?&list.end:&nodes[i+1];
        }
        auto word=[&](std::uint32_t value){for(unsigned i=0;i<4;++i)bytes.push_back(static_cast<std::uint8_t>(value>>(8*i)));};
        unsigned incoming=(t/16)%11;word(incoming);
        for(unsigned i=0;i<incoming;++i) {
            std::string name;
            switch((t+i)%5) {
                case 0:name="old"+std::to_string(i%6);break;
                case 1:name="new"+std::to_string(i%3);break;
                case 2:name="";break;
                case 3:name=std::string(127,static_cast<char>('a'+i%3));break;
                default:name="unique"+std::to_string(i);break;
            }
            word(static_cast<std::uint32_t>(name.size()));bytes.insert(bytes.end(),name.begin(),name.end());
        }
        // Trailing input proves exactly the declared records are consumed.
        bytes.insert(bytes.end(),{0xde,0xad,0xbe,0xef});stream={bytes};
        read_manager();
    }
    void read_manager() {
        if(!native&&registration.cache.entries_30)return;
        registration.count_30=manager.count;registration.capacity_34=manager.capacity;
        registration.growth_38=manager.growth;cache.entries_30=manager.entries;
    }
    void pull() {if(native) {cache.entries_30=nullptr;read_manager();}}
    void push() {if(native) {manager.count=registration.count_30;manager.capacity=registration.capacity_34;
        manager.growth=registration.growth_38;manager.entries=cache.entries_30;}}
    unsigned storage_id(const ResourceCacheEntry* p)const {return !p?0:p==old_block.entries.data()?1:p==new_block.entries.data()?2:99;}
    unsigned node_id(const ResourceCacheLookupNode* p)const {
        if(!p)return 0;if(p==&manager.lookup.list_04.begin)return 1;if(p==&manager.lookup.list_04.end)return 2;
        for(unsigned i=0;i<4;++i)if(p==&nodes[i])return 3+i;return 99;
    }
    std::uint64_t cache_id(const ResourceCacheNode* p)const {
        for(unsigned i=0;i<4;++i)if(p==&nodes[i].value)return 3+i;
        return reinterpret_cast<std::uintptr_t>(p);
    }
    void record(unsigned kind) {
        pull();trace.push_back(kind);trace.push_back(manager.loaded);trace.push_back(static_cast<std::uint32_t>(registration.count_30));
        trace.push_back(static_cast<std::uint32_t>(registration.capacity_34));trace.push_back(storage_id(cache.entries_30));trace.push_back(stream.cursor);
    }
    ResourceEntryAllocation allocate(std::uint32_t size) {record(1);trace.push_back(size);return {new_block.entries.data(),&new_block.cookie};}
    void deallocate(ResourceCacheEntry* p) {record(2);trace.push_back(storage_id(p));trace.push_back(new_block.cookie);}
    void destroy(void* p) {record(3);trace.push_back(reinterpret_cast<std::uintptr_t>(p));}
    void release(ResourceCacheLookupNode* p) {record(4);trace.push_back(node_id(p));trace.push_back(node_id(p->previous_10->next_0c));trace.push_back(node_id(p->next_0c->previous_10));}
    ResourceRegistrationHooks allocation_hooks() {return {this,[](void* p,std::uint32_t n){return static_cast<Fixture*>(p)->allocate(n);},[](void* p,ResourceCacheEntry* e){static_cast<Fixture*>(p)->deallocate(e);}};}
    ResourceCacheCleanupHooks cleanup_hooks() {return {this,nullptr,[](void* p,void* r){static_cast<Fixture*>(p)->destroy(r);},[](void* p,ResourceCacheLookupNode* n){static_cast<Fixture*>(p)->release(n);}};}
    void run() {load_resource_name_table(manager.lookup,manager.loaded,registration,stream,allocation_hooks(),cleanup_hooks());}
    bool equals(Fixture& other) {
        pull();other.pull();
        if(manager.loaded!=other.manager.loaded||registration.count_30!=other.registration.count_30||registration.capacity_34!=other.registration.capacity_34||registration.growth_38!=other.registration.growth_38||storage_id(cache.entries_30)!=other.storage_id(other.cache.entries_30)||stream.cursor!=other.stream.cursor)return false;
        if(node_id(manager.lookup.list_04.begin.next_0c)!=other.node_id(other.manager.lookup.list_04.begin.next_0c)||node_id(manager.lookup.list_04.end.previous_10)!=other.node_id(other.manager.lookup.list_04.end.previous_10))return false;
        for(unsigned i=0;i<4;++i)if(node_id(nodes[i].next_0c)!=other.node_id(other.nodes[i].next_0c)||node_id(nodes[i].previous_10)!=other.node_id(other.nodes[i].previous_10)||nodes[i].value.references_08!=other.nodes[i].value.references_08)return false;
        const Block* left[]={&old_block,&new_block};const Block* right[]={&other.old_block,&other.new_block};
        for(unsigned b=0;b<2;++b) {if(left[b]->cookie!=right[b]->cookie)return false;
            for(unsigned i=0;i<24;++i) {const auto& a=left[b]->entries[i];const auto& c=right[b]->entries[i];
                if(a.name_00!=c.name_00||cache_id(a.cached_80)!=other.cache_id(c.cached_80)||a.unknown_84!=c.unknown_84)return false;}}
        return true;
    }
};
}
