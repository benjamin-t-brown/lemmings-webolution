#include "lemmings/resource_manager_construction.hpp"
#include "lemmings/resource_cache_load.hpp"

namespace lemmings {
namespace {
void set_vtable(ResourceCacheLookupManager& manager,std::uint32_t token) {
    for (unsigned i=0;i<4;++i)
        manager.unknown_00[i]=static_cast<std::byte>((token>>(i*8))&0xffu);
}
}
ResourceCacheLookupNode* construct_empty_resource_cache_node(ResourceCacheLookupNode& node) {
    initialize_resource_cache_value(node.value);
    return &node;
}
ResourceCacheLookupList* construct_resource_cache_list(ResourceCacheLookupList& list) {
    construct_empty_resource_cache_node(list.begin);
    construct_empty_resource_cache_node(list.end);
    list.begin.next_0c=&list.end;
    list.begin.previous_10=nullptr;
    list.end.next_0c=nullptr;
    list.end.previous_10=&list.begin;
    return &list;
}
ResourceRegistration* construct_resource_registration(ResourceRegistration& registration) {
    registration.count_30=0;
    registration.capacity_34=0;
    registration.growth_38=1;
    registration.cache.entries_30=nullptr;
    return &registration;
}
ResourceCacheLookupManager* construct_base_resource_manager(
    ResourceCacheLookupManager& manager,std::uint32_t& loaded_count) {
    construct_resource_cache_list(manager.list_04);
    set_vtable(manager,0x4b1bd8);
    loaded_count=0;
    return &manager;
}
ResourceCacheLookupManager* construct_named_resource_manager(
    ResourceCacheLookupManager& manager,std::uint32_t& loaded_count,ResourceRegistration& registration) {
    construct_base_resource_manager(manager,loaded_count);
    construct_resource_registration(registration);
    set_vtable(manager,0x4b1bc0);
    return &manager;
}
ResourceCacheLookupManager* construct_sprite_resource_manager(
    ResourceCacheLookupManager& manager,std::uint32_t& loaded_count,ResourceRegistration& registration) {
    construct_named_resource_manager(manager,loaded_count,registration);
    set_vtable(manager,0x4b1ba8);
    return &manager;
}
}
