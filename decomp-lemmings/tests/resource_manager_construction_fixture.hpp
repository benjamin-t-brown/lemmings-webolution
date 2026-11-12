#pragma once
#include "lemmings/resource_manager_construction.hpp"
#include "lemmings/resource_cache_load.hpp"
#include <bit>
#include <cstdlib>
#include <cstring>

namespace manager_construction_test {
using namespace lemmings;
struct Fixture {
    ResourceCacheLookupManager manager;
    ResourceCache cache;
    ResourceRegistration registration{cache};
    std::uint32_t loaded_count=0;
    std::array<unsigned char,64> initial{};
    unsigned operation;
    static std::uint32_t read(const unsigned char* p) {
        return std::uint32_t(p[0])|(std::uint32_t(p[1])<<8)|(std::uint32_t(p[2])<<16)|(std::uint32_t(p[3])<<24);
    }
    static void write(unsigned char* p,std::uint32_t value) {
        for(unsigned i=0;i<4;++i)p[i]=static_cast<unsigned char>(value>>(i*8));
    }
    static void seed_node(ResourceCacheLookupNode& node,const unsigned char* p) {
        node.value.resource_00=reinterpret_cast<void*>(std::uintptr_t(read(p)));
        node.value.unknown_04=read(p+4);node.value.references_08=read(p+8);
        node.next_0c=reinterpret_cast<ResourceCacheLookupNode*>(std::uintptr_t(read(p+12)));
        node.previous_10=reinterpret_cast<ResourceCacheLookupNode*>(std::uintptr_t(read(p+16)));
    }
    explicit Fixture(unsigned trial):operation(trial%7) {
        unsigned random=trial+1;
        for(auto& byte:initial){random=random*1664525u+1013904223u;byte=static_cast<unsigned char>(random>>24);}
        for(unsigned i=0;i<4;++i)manager.unknown_00[i]=static_cast<std::byte>(initial[i]);
        seed_node(manager.list_04.begin,initial.data()+4);seed_node(manager.list_04.end,initial.data()+0x18);
        loaded_count=read(initial.data()+0x2c);
        registration.count_30=std::bit_cast<std::int32_t>(read(initial.data()+0x30));
        registration.capacity_34=std::bit_cast<std::int32_t>(read(initial.data()+0x34));
        registration.growth_38=std::bit_cast<std::int32_t>(read(initial.data()+0x38));
        cache.entries_30=reinterpret_cast<ResourceCacheEntry*>(std::uintptr_t(read(initial.data()+0x3c)));
    }
    unsigned run() {
        switch(operation){
        case 0:if(construct_sprite_resource_manager(manager,loaded_count,registration)!=&manager)std::abort();return 0;
        case 1:if(construct_named_resource_manager(manager,loaded_count,registration)!=&manager)std::abort();return 0;
        case 2:if(construct_base_resource_manager(manager,loaded_count)!=&manager)std::abort();return 0;
        case 3:if(construct_resource_cache_list(manager.list_04)!=&manager.list_04)std::abort();return 4;
        case 4:if(construct_empty_resource_cache_node(manager.list_04.begin)!=&manager.list_04.begin)std::abort();return 4;
        case 5:if(construct_resource_registration(registration)!=&registration)std::abort();return 0x30;
        default:if(initialize_resource_cache_value(manager.list_04.begin.value)!=&manager.list_04.begin.value)std::abort();return 4;
        }
    }
    std::uint32_t node_pointer(const ResourceCacheLookupNode* node,std::uint32_t raw_base)const {
        if(node==&manager.list_04.begin)return raw_base+4;
        if(node==&manager.list_04.end)return raw_base+0x18;
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(node));
    }
    void serialize_node(unsigned char* p,const ResourceCacheLookupNode& node,std::uint32_t raw_base)const {
        write(p,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(node.value.resource_00)));
        write(p+4,node.value.unknown_04);write(p+8,node.value.references_08);
        write(p+12,node_pointer(node.next_0c,raw_base));write(p+16,node_pointer(node.previous_10,raw_base));
    }
    std::array<unsigned char,64> serialize(std::uint32_t raw_base)const {
        auto result=initial;
        for(unsigned i=0;i<4;++i)result[i]=std::to_integer<unsigned char>(manager.unknown_00[i]);
        serialize_node(result.data()+4,manager.list_04.begin,raw_base);serialize_node(result.data()+0x18,manager.list_04.end,raw_base);
        write(result.data()+0x2c,loaded_count);write(result.data()+0x30,static_cast<std::uint32_t>(registration.count_30));
        write(result.data()+0x34,static_cast<std::uint32_t>(registration.capacity_34));write(result.data()+0x38,static_cast<std::uint32_t>(registration.growth_38));
        write(result.data()+0x3c,static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(cache.entries_30)));
        return result;
    }
};
}
