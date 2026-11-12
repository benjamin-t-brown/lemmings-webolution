#pragma once
#include <array>
#include <cstdint>
namespace lemmings {
struct ResourceCacheNode {void* resource_00=nullptr;std::uint32_t unknown_04=0,references_08=0;};
struct ResourceCacheEntry {std::array<char,128> name_00{};ResourceCacheNode* cached_80=nullptr;std::uint32_t unknown_84=0;};
struct ResourceCache {ResourceCacheEntry* entries_30=nullptr;};
struct ResourceCacheHooks {
    void* context=nullptr;
    ResourceCacheNode* (*find_4306f0)(void*,std::int32_t);
    void* (*load_430660)(void*,std::int32_t);
    void (*notify)(void*,const char*)=nullptr;
};
// Valid index and live entry storage are required. Acquire captures its entry
// before callbacks; old storage must remain alive even if entries_30 changes.
void* acquire_resource(ResourceCache&,std::int32_t,const ResourceCacheHooks&);
// Original 430050 unconditionally dereferences cached_80; it must be nonnull.
void release_resource(ResourceCache&,std::int32_t);
// Original 446010 suppresses notification, acquires, then rereads the current
// entry for release. This also requires a nonnull cache after acquisition.
void* get_resource_balanced(ResourceCache&,std::int32_t,const ResourceCacheHooks&);
}
