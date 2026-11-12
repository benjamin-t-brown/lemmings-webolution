#pragma once
#include "lemmings/resource_cache_find.hpp"
#include <cstdint>

namespace lemmings {
struct ResourceCacheNameFactoryHooks {
    void* context = nullptr;
    void* (*create_14)(void*, const char*);
};
// 485590: capture the current indexed entry's name and pass it directly to
// virtual factory slot +14. The index and NUL-terminated name must be valid.
void* create_resource_by_index(ResourceCache&, std::int32_t,
    const ResourceCacheNameFactoryHooks&);
struct ResourceCacheLoadHooks {
    void* context = nullptr;
    // Original virtual method at manager vtable +0C. May return null.
    void* (*create)(void*, std::int32_t);
    // Original operator new boundary. Storage must remain live and must not
    // overlap existing nodes. Normal load completion requires a nonnull node.
    ResourceCacheLookupNode* (*allocate_node)(void*);
};

// 430920 clears only resource_00 and references_08; it preserves the ID word.
ResourceCacheNode* initialize_resource_cache_value(ResourceCacheNode&);
// 430A10 initializes before copying, including when source aliases node.value.
ResourceCacheLookupNode* construct_resource_cache_node(ResourceCacheLookupNode&,
    const ResourceCacheNode&, ResourceCacheLookupNode* next,
    ResourceCacheLookupNode* previous);
// 430950 -> 430970. The first-node pointer is read after allocation, then the
// old first's previous pointer and the current first pointer are replaced.
void prepend_resource_cache_node(ResourceCacheLookupManager&,
    const ResourceCacheNode&, const ResourceCacheLoadHooks&);
// 430660. loaded_count is the live manager word at +2C, represented separately
// from the lookup view. Create/allocation can mutate both through context.
// Requires a valid terminating list and successful allocation on a miss.
void* load_cached_resource(ResourceCacheLookupManager&, std::uint32_t& loaded_count,
    std::int32_t index, const ResourceCacheLoadHooks&);
} // namespace lemmings
