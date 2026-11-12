#pragma once
#include "lemmings/resource_cache.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
namespace lemmings {
// Known 20-byte x86 loaded-resource list node. The previously unknown word
// in ResourceCacheNode at +04 is the resource ID compared by 0x4306F0.
struct ResourceCacheLookupNode {
    ResourceCacheNode value;
    ResourceCacheLookupNode* next_0c=nullptr;
    ResourceCacheLookupNode* previous_10=nullptr;
};
struct ResourceCacheLookupList {
    ResourceCacheLookupNode begin,end;
    // Portable empty-list setup; this is not a claim about the original
    // list constructor's initialization of otherwise unknown fields.
    ResourceCacheLookupList();
    ResourceCacheLookupList(const ResourceCacheLookupList&)=delete;
    ResourceCacheLookupList& operator=(const ResourceCacheLookupList&)=delete;
};
struct ResourceCacheLookupManager {
    std::array<std::byte,4> unknown_00{};
    ResourceCacheLookupList list_04;
};
static_assert(std::is_standard_layout_v<ResourceCacheLookupNode>);
static_assert(offsetof(ResourceCacheLookupNode,value)==0);
#if INTPTR_MAX == INT32_MAX
static_assert(offsetof(ResourceCacheNode,unknown_04)==4);
static_assert(offsetof(ResourceCacheNode,references_08)==8);
static_assert(sizeof(ResourceCacheLookupNode)==0x14);
static_assert(offsetof(ResourceCacheLookupNode,next_0c)==0x0c);
static_assert(offsetof(ResourceCacheLookupNode,previous_10)==0x10);
static_assert(offsetof(ResourceCacheLookupManager,list_04)==4);
static_assert(offsetof(ResourceCacheLookupList,end)==0x14);
static_assert(sizeof(ResourceCacheLookupManager)==0x2c);
#endif
std::int32_t resource_cache_lookup_id(const ResourceCacheNode&) noexcept;
// Concrete helpers 0x430760 and 0x430780; no list/node contents are changed.
ResourceCacheLookupNode* resource_cache_first(const ResourceCacheLookupList&) noexcept;
bool resource_cache_is_end(const ResourceCacheLookupList&,const ResourceCacheLookupNode*) noexcept;
// 0x4306F0 returns the first matching node in next-link order, or null at end.
// The caller provides live nodes and a chain reaching the inline end sentinel.
// Null links or a cycle without a match retain the original invalid/nonterminating
// behavior. There are no array-index bounds, name comparisons or count updates.
ResourceCacheLookupNode* find_cached_resource_node(const ResourceCacheLookupList&,std::int32_t id) noexcept;
ResourceCacheNode* find_cached_resource(const ResourceCacheLookupManager&,std::int32_t id) noexcept;
}
