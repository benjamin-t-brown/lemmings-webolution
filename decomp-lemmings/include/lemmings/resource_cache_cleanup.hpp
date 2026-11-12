#pragma once

#include "lemmings/resource_cache_find.hpp"

namespace lemmings {

struct ResourceCacheCleanupHooks {
    void* context = nullptr;
    void* (*create_0c)(void*, std::int32_t id) = nullptr;
    // The original eviction path calls this even for a null resource.
    void (*destroy_10)(void*, void* resource) = nullptr;
    // Original scalar node deletion 44C060(flags=1), after links are changed.
    void (*release_node)(void*, ResourceCacheLookupNode*) = nullptr;
};

// 44BE00. Valid live previous/next links are required. The node's own fields
// are preserved before release; this does not alter a manager count.
void unlink_resource_cache_node(ResourceCacheLookupNode&, const ResourceCacheCleanupHooks&);

// 44A050. Evicts exactly zero-reference nodes, including null resources.
// Caches next before destroy, unlinks using current links, releases storage,
// then decrements the live manager word +2C with unsigned32 wrapping.
void evict_unused_resources(ResourceCacheLookupManager&, std::uint32_t& loaded_count,
                            const ResourceCacheCleanupHooks&);
// 485380. Evicts unused nodes first, then destroys each nonnull survivor,
// decrements the count and clears its resource pointer. Nodes remain linked.
void unload_cached_resources(ResourceCacheLookupManager&, std::uint32_t& loaded_count,
                             const ResourceCacheCleanupHooks&);
// 485400. Evicts unused nodes, then recreates every survivor, even when its
// current resource is nonnull. Each creation increments count, including null.
void recreate_cached_resources(ResourceCacheLookupManager&, std::uint32_t& loaded_count,
                               const ResourceCacheCleanupHooks&);

// 4854D0 clears current entry caches before unloading; 485530 recreates first
// and then refreshes each cache from the concrete list lookup. entry_count is
// the live signed manager word +30; loaded_count is +2C. Views must refer to
// the same logical manager. Callbacks may mutate these fields and list links,
// provided all subsequently accessed storage stays live and traversal ends.
void invalidate_and_unload_resources(ResourceCacheLookupManager&, std::uint32_t& loaded_count,
    ResourceCache&, std::int32_t& entry_count, const ResourceCacheCleanupHooks&);
void recreate_and_refresh_resources(ResourceCacheLookupManager&, std::uint32_t& loaded_count,
    ResourceCache&, std::int32_t& entry_count, const ResourceCacheCleanupHooks&);
// 449FB0 clears only the low byte of entry.unknown_84 when its cached node is
// null or has zero references, then evicts. cached_80 itself is not cleared.
void clear_unused_resource_flags_and_evict(ResourceCacheLookupManager&,
    std::uint32_t& loaded_count, ResourceCache&, std::int32_t& entry_count,
    const ResourceCacheCleanupHooks&);

} // namespace lemmings
