#pragma once
#include "lemmings/resource_cache.hpp"

namespace lemmings {
// The original manager contains a vector header at +30. ResourceCache exposes
// its backing entries; these are the other three words of that vector header.
struct ResourceRegistration {
    ResourceCache& cache;
    std::int32_t count_30=0, capacity_34=0, growth_38=0;
};
struct ResourceEntryAllocation {
    ResourceCacheEntry* entries=nullptr;
    std::uint32_t* cookie=nullptr;
};
struct ResourceRegistrationHooks {
    void* context=nullptr;
    // Size is the original x86 byte count, not sizeof(ResourceCacheEntry).
    // Return live, uninitialized entry storage and its writable array cookie.
    ResourceEntryAllocation (*allocate_49c1a8)(void*,std::uint32_t);
    // Identifies entries whose original allocation header is entries - 4.
    // Entry destructors are empty; only the storage release remains external.
    void (*deallocate_49c13d)(void*,ResourceCacheEntry*);
};

// Partial initialization/assignment preserves name bytes after the NUL and
// the high three bytes of unknown_84. Default initialization also preserves
// cached_80. Names must fit 128 bytes including NUL, without partial overlap.
void initialize_resource_entry(ResourceCacheEntry&);             // 44BE60
void initialize_named_resource_entry(ResourceCacheEntry&,const char*); // 449170
void assign_resource_entry(ResourceCacheEntry&,const ResourceCacheEntry&); // 448F50

// Normal-return 449DB0. Counts, storage, cookie and any callback mutations must
// leave all accessed elements valid. Growth must provide enough storage and
// allocation must succeed. The original does not recover from failed growth.
void append_resource_entry(ResourceRegistration&,const ResourceCacheEntry&,
                           const ResourceRegistrationHooks&);
// 446040 reuses the first slot whose low byte at +84 is zero, else appends.
// No name deduplication. Returns the selected index before allocation hooks.
std::int32_t register_resource_slot(ResourceRegistration&,const char*,
                                    const ResourceRegistrationHooks&);
}
