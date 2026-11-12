#include "lemmings/resource_cache_find.hpp"
#include <bit>
namespace lemmings {
ResourceCacheLookupList::ResourceCacheLookupList(){begin.next_0c=&end;end.previous_10=&begin;}
std::int32_t resource_cache_lookup_id(const ResourceCacheNode& node) noexcept {
    return std::bit_cast<std::int32_t>(node.unknown_04);
}
ResourceCacheLookupNode* resource_cache_first(const ResourceCacheLookupList& list) noexcept {return list.begin.next_0c;}
bool resource_cache_is_end(const ResourceCacheLookupList& list,const ResourceCacheLookupNode* node) noexcept {return node==&list.end;}
ResourceCacheLookupNode* find_cached_resource_node(const ResourceCacheLookupList& list,std::int32_t id) noexcept {
    auto* node=resource_cache_first(list);
    while(!resource_cache_is_end(list,node)&&resource_cache_lookup_id(node->value)!=id)node=node->next_0c;
    return resource_cache_is_end(list,node)?nullptr:node;
}
ResourceCacheNode* find_cached_resource(const ResourceCacheLookupManager& manager,std::int32_t id) noexcept {
    auto* node=find_cached_resource_node(manager.list_04,id);
    return node?&node->value:nullptr;
}
}
