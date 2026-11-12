#pragma once
#include "lemmings/resource_cache_find.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <random>
#include <vector>
namespace cache_find_test {
using namespace lemmings;
struct Fixture {
    ResourceCacheLookupManager manager;
    std::array<ResourceCacheLookupNode,64> nodes;
    std::vector<unsigned> order;
    std::vector<std::int32_t> queries;
    explicit Fixture(unsigned trial){
        std::mt19937 random(0x4306f0u+trial);
        manager.unknown_00.fill(std::byte{0xa5});
        constexpr std::uint32_t edges[]={0,1,0xffffffffu,0x80000000u,0x7fffffffu,0x01000000u,0x80000001u};
        for(unsigned i=0;i<nodes.size();++i){
            auto& node=nodes[i];
            node.value.resource_00=i%3?reinterpret_cast<void*>(std::uintptr_t(0x12340000+i)):nullptr;
            node.value.unknown_04=trial%4==0?edges[random()%7]:trial%4==1?random():trial%4==2?edges[trial%7]:i%5;
            node.value.references_08=random();
            node.next_0c=nullptr;
            node.previous_10=reinterpret_cast<ResourceCacheLookupNode*>(std::uintptr_t(1));
            order.push_back(i);
        }
        std::shuffle(order.begin(),order.end(),random);
        order.resize(trial%65);
        auto& list=manager.list_04;
        list.begin.value={reinterpret_cast<void*>(std::uintptr_t(0x44444444)),edges[trial%7],0x88888888u};
        list.end.value={reinterpret_cast<void*>(std::uintptr_t(0x55555555)),edges[(trial+1)%7],0x99999999u};
        list.begin.previous_10=reinterpret_cast<ResourceCacheLookupNode*>(std::uintptr_t(1));
        list.end.next_0c=reinterpret_cast<ResourceCacheLookupNode*>(std::uintptr_t(1));
        list.end.previous_10=order.empty()?&list.begin:&nodes[order.back()];
        list.begin.next_0c=order.empty()?&list.end:&nodes[order.front()];
        for(std::size_t i=0;i<order.size();++i)nodes[order[i]].next_0c=i+1<order.size()?&nodes[order[i+1]]:&list.end;
        for(auto id:edges)queries.push_back(std::bit_cast<std::int32_t>(id));
        for(unsigned i=0;i<6;++i)queries.push_back(std::bit_cast<std::int32_t>(random()));
        if(!order.empty()){
            queries.push_back(resource_cache_lookup_id(nodes[order.front()].value));
            queries.push_back(resource_cache_lookup_id(nodes[order[order.size()/2]].value));
            queries.push_back(resource_cache_lookup_id(nodes[order.back()].value));
        }
    }
    ResourceCacheLookupNode* expected(std::int32_t id){
        for(auto index:order)if(nodes[index].value.unknown_04==std::bit_cast<std::uint32_t>(id))return &nodes[index];
        return nullptr;
    }
    std::vector<unsigned char> bytes() const {
        std::vector<unsigned char> result(sizeof(manager)+sizeof(nodes));
        std::memcpy(result.data(),&manager,sizeof(manager));
        std::memcpy(result.data()+sizeof(manager),nodes.data(),sizeof(nodes));
        return result;
    }
};
}
