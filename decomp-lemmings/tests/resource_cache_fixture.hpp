#pragma once
#include "lemmings/resource_cache.hpp"
#include <cstring>
#include <vector>
namespace cache_test {
using namespace lemmings;
struct Fixture {
    ResourceCache cache;
    std::array<ResourceCacheEntry,6> old_entries{},new_entries{};
    std::array<ResourceCacheNode,3> nodes{};
    alignas(void*) std::array<unsigned char,64> raw_manager{};
    std::vector<unsigned> trace;
    unsigned trial,operation,mode,finds=0;
    int index;
    bool native;
    Fixture(unsigned t,bool n):trial(t),operation(t%3),mode((t/3)%5),index((t/15)%6),native(n){
        unsigned counts[]={0,0xffffffffu,0x7fffffffu,0x80000000u,42};
        for(unsigned i=0;i<3;++i)nodes[i]={reinterpret_cast<void*>(std::uintptr_t(0x12340000+i)),0xa5a5a5a5,counts[(t/15+i)%5]};
        for(unsigned i=0;i<6;++i){old_entries[i].name_00[0]=static_cast<char>('a'+i);new_entries[i].name_00[0]=static_cast<char>('A'+i);new_entries[i].cached_80=&nodes[2];old_entries[i].cached_80=(mode==0||operation==1)?&nodes[0]:nullptr;old_entries[i].unknown_84=0xdeadbeef;}
        cache.entries_30=old_entries.data();sync();
    }
    void sync(){if(native)std::memcpy(raw_manager.data()+0x3c,&cache.entries_30,4);}
    void move(unsigned stage){if((trial/75)%4==stage){cache.entries_30=new_entries.data();sync();}}
    ResourceCacheNode* find(int i){trace.push_back(100+i);++finds;move(1);old_entries[index].cached_80=&nodes[2];if(finds==1)return mode==1?&nodes[0]:nullptr;return mode==4&&operation==0?nullptr:&nodes[1];}
    void* load(int i){trace.push_back(200+i);move(2);old_entries[index].name_00[0]='z';return mode==3?nullptr:reinterpret_cast<void*>(std::uintptr_t(0x12340007));}
    void notify(const char* name){trace.push_back(300+static_cast<unsigned char>(name[0]));move(3);}
    ResourceCacheHooks hooks(){return {this,[](void* p,int i){return static_cast<Fixture*>(p)->find(i);},[](void* p,int i){return static_cast<Fixture*>(p)->load(i);},trial%2?+[](void* p,const char* name){static_cast<Fixture*>(p)->notify(name);}:nullptr};}
    void* run(){auto h=hooks();if(operation==0)return acquire_resource(cache,index,h);if(operation==1){release_resource(cache,index);return nullptr;}return get_resource_balanced(cache,index,h);}
    std::vector<std::uintptr_t> snapshot(){std::vector<std::uintptr_t> out;for(auto& n:nodes){out.push_back(n.references_08);out.push_back(n.unknown_04);}for(auto* group:{old_entries.data(),new_entries.data()})for(int i=0;i<6;++i){out.push_back(group[i].cached_80?group[i].cached_80-nodes.data()+1:0);out.push_back(group[i].name_00[0]);out.push_back(group[i].unknown_84);}out.push_back(cache.entries_30==new_entries.data());return out;}
};
}
