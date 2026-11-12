#include "lemmings/resource_cache_cleanup.hpp"

namespace lemmings {

void unlink_resource_cache_node(ResourceCacheLookupNode& node,
                               const ResourceCacheCleanupHooks& hooks) {
    node.previous_10->next_0c = node.next_0c;
    node.next_0c->previous_10 = node.previous_10;
    hooks.release_node(hooks.context, &node);
}

void evict_unused_resources(ResourceCacheLookupManager& manager, std::uint32_t& count,
                            const ResourceCacheCleanupHooks& hooks) {
    auto* node = resource_cache_first(manager.list_04);
    while (!resource_cache_is_end(manager.list_04, node)) {
        if (node->value.references_08 == 0) {
            auto* removed = node;
            node = node->next_0c;
            hooks.destroy_10(hooks.context, removed->value.resource_00);
            unlink_resource_cache_node(*removed, hooks);
            --count;
        } else {
            node = node->next_0c;
        }
    }
}

void unload_cached_resources(ResourceCacheLookupManager& manager, std::uint32_t& count,
                             const ResourceCacheCleanupHooks& hooks) {
    evict_unused_resources(manager, count, hooks);
    auto* node = resource_cache_first(manager.list_04);
    while (!resource_cache_is_end(manager.list_04, node)) {
        if (node->value.resource_00) {
            hooks.destroy_10(hooks.context, node->value.resource_00);
            --count;
            node->value.resource_00 = nullptr;
        }
        node = node->next_0c;
    }
}

void recreate_cached_resources(ResourceCacheLookupManager& manager, std::uint32_t& count,
                               const ResourceCacheCleanupHooks& hooks) {
    evict_unused_resources(manager, count, hooks);
    auto* node = resource_cache_first(manager.list_04);
    while (!resource_cache_is_end(manager.list_04, node)) {
        node->value.resource_00 = hooks.create_0c(hooks.context, resource_cache_lookup_id(node->value));
        ++count;
        node = node->next_0c;
    }
}

void invalidate_and_unload_resources(ResourceCacheLookupManager& manager, std::uint32_t& count,
    ResourceCache& cache, std::int32_t& entry_count, const ResourceCacheCleanupHooks& hooks) {
    for (std::int32_t index = 0; index < entry_count; ++index)
        cache.entries_30[index].cached_80 = nullptr;
    unload_cached_resources(manager, count, hooks);
}

void recreate_and_refresh_resources(ResourceCacheLookupManager& manager, std::uint32_t& count,
    ResourceCache& cache, std::int32_t& entry_count, const ResourceCacheCleanupHooks& hooks) {
    recreate_cached_resources(manager, count, hooks);
    for (std::int32_t index = 0; index < entry_count; ++index)
        cache.entries_30[index].cached_80 = find_cached_resource(manager, index);
}

void clear_unused_resource_flags_and_evict(ResourceCacheLookupManager& manager,
    std::uint32_t& count, ResourceCache& cache, std::int32_t& entry_count,
    const ResourceCacheCleanupHooks& hooks) {
    for (std::int32_t index = 0; index < entry_count; ++index) {
        auto& entry = cache.entries_30[index];
        if ((entry.unknown_84 & 0xffu) != 0 &&
            (!entry.cached_80 || entry.cached_80->references_08 == 0))
            entry.unknown_84 &= 0xffffff00u;
    }
    evict_unused_resources(manager, count, hooks);
}

} // namespace lemmings
