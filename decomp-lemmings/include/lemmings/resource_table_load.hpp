#pragma once
#include "lemmings/level_stream.hpp"
#include "lemmings/resource_cache_cleanup.hpp"
#include "lemmings/resource_registration.hpp"

namespace lemmings {
// Normal-return texture/generic vector specializations 449940 / 449BF0.
// Growth 1 selects the requested capacity; other positive growth values step
// until capacity is strictly greater than the request. Shrinking changes only
// the logical count. Allocation must succeed and all live callback-mutated
// counts/pointers must leave the copied entries valid. Array cookies and
// untouched entry bytes retain the original partial-initialization behavior.
void resize_resource_entries(ResourceRegistration&,std::int32_t requested,
                             const ResourceRegistrationHooks&);

// Complete in-memory input form of 445C10 (textures) and 445EB0 (sprites).
// Clears unreferenced slots and evicts zero-reference loaded nodes before
// reading the incoming name count. Retained active names are deduplicated,
// and new names fill the first inactive slots. loaded_count is the live +2C
// manager word, independent of registration.count_30. Names must fit 128 bytes
// including NUL; input and storage must satisfy the original access domain.
void load_resource_name_table(ResourceCacheLookupManager&,std::uint32_t& loaded_count,
    ResourceRegistration&,LevelByteStream&,const ResourceRegistrationHooks&,
    const ResourceCacheCleanupHooks&);
}
