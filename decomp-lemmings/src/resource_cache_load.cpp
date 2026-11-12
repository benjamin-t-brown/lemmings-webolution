#include "lemmings/resource_cache_load.hpp"
#include <bit>

namespace lemmings {
void* create_resource_by_index(ResourceCache& cache, std::int32_t index,
    const ResourceCacheNameFactoryHooks& hooks) {
    const char* name = cache.entries_30[index].name_00.data();
    return hooks.create_14(hooks.context, name);
}
ResourceCacheNode* initialize_resource_cache_value(ResourceCacheNode& value) {
    value.resource_00 = nullptr;
    value.references_08 = 0;
    return &value;
}

ResourceCacheLookupNode* construct_resource_cache_node(ResourceCacheLookupNode& node,
    const ResourceCacheNode& value, ResourceCacheLookupNode* next,
    ResourceCacheLookupNode* previous) {
    initialize_resource_cache_value(node.value);
    node.value.resource_00 = value.resource_00;
    node.value.unknown_04 = value.unknown_04;
    node.value.references_08 = value.references_08;
    node.next_0c = next;
    node.previous_10 = previous;
    return &node;
}

void prepend_resource_cache_node(ResourceCacheLookupManager& manager,
    const ResourceCacheNode& value, const ResourceCacheLoadHooks& hooks) {
    auto* node = hooks.allocate_node(hooks.context);
    if (node) {
        construct_resource_cache_node(*node, value,
            manager.list_04.begin.next_0c, &manager.list_04.begin);
    }
    manager.list_04.begin.next_0c->previous_10 = node;
    manager.list_04.begin.next_0c = node;
}

void* load_cached_resource(ResourceCacheLookupManager& manager,
    std::uint32_t& loaded_count, std::int32_t index,
    const ResourceCacheLoadHooks& hooks) {
    auto* node = find_cached_resource(manager, index);
    if (!node) {
        ResourceCacheNode value;
        initialize_resource_cache_value(value);
        value.unknown_04 = std::bit_cast<std::uint32_t>(index);
        value.resource_00 = hooks.create(hooks.context, index);
        prepend_resource_cache_node(manager, value, hooks);
        node = &manager.list_04.begin.next_0c->value;
        ++loaded_count;
    }
    ++node->references_08;
    return node->resource_00;
}
} // namespace lemmings
