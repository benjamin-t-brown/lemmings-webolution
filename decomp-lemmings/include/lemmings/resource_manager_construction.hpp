#pragma once
#include "lemmings/resource_cache_find.hpp"
#include "lemmings/resource_registration.hpp"

namespace lemmings {
// 485730 clears only the payload's resource pointer and reference count.
// Its ID and both list links retain their previous bits.
ResourceCacheLookupNode* construct_empty_resource_cache_node(ResourceCacheLookupNode&);
// 4855F0 initializes both sentinel payloads, preserving their ID words, then
// connects begin/end and clears the two outer links.
ResourceCacheLookupList* construct_resource_cache_list(ResourceCacheLookupList&);
// 485470 initializes count=capacity=0, growth=1 and backing pointer=null.
ResourceRegistration* construct_resource_registration(ResourceRegistration&);

// Original manager word +00 is a vtable token, represented in unknown_00 as
// four little-endian bytes. These functions do not install host virtual calls.
// The separate counter represents original +2C. The supplied views must refer
// to distinct portions of the logical manager; existing resources are not freed.
ResourceCacheLookupManager* construct_base_resource_manager(
    ResourceCacheLookupManager&,std::uint32_t& loaded_count); // 4852D0
ResourceCacheLookupManager* construct_named_resource_manager(
    ResourceCacheLookupManager&,std::uint32_t& loaded_count,ResourceRegistration&); // 485120
ResourceCacheLookupManager* construct_sprite_resource_manager(
    ResourceCacheLookupManager&,std::uint32_t& loaded_count,ResourceRegistration&); // 484E40
}
