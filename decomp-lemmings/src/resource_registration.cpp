#include "lemmings/resource_registration.hpp"
#include <bit>

namespace lemmings {
namespace {
void copy_name(char* destination,const char* source) {
    do { *destination=*source; ++destination; } while (*source++!='\0');
}
void set_active(ResourceCacheEntry& entry,std::uint8_t active) {
    entry.unknown_84=(entry.unknown_84&0xffffff00u)|active;
}
std::int32_t add32(std::int32_t a,std::int32_t b) {
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(a)+static_cast<std::uint32_t>(b));
}
}
void initialize_resource_entry(ResourceCacheEntry& entry) {
    entry.name_00[0]='\0';
    set_active(entry,0);
}
void initialize_named_resource_entry(ResourceCacheEntry& entry,const char* name) {
    entry.name_00[0]='\0';
    copy_name(entry.name_00.data(),name);
    entry.cached_80=nullptr;
    set_active(entry,1);
}
void assign_resource_entry(ResourceCacheEntry& entry,const ResourceCacheEntry& source) {
    copy_name(entry.name_00.data(),source.name_00.data());
    entry.cached_80=source.cached_80;
    set_active(entry,static_cast<std::uint8_t>(source.unknown_84));
}
void append_resource_entry(ResourceRegistration& state,const ResourceCacheEntry& source,
                           const ResourceRegistrationHooks& hooks) {
    if (state.count_30>=state.capacity_34) {
        state.capacity_34=add32(state.capacity_34,state.growth_38);
        const auto capacity=state.capacity_34;
        const auto allocation=hooks.allocate_49c1a8(hooks.context,
            static_cast<std::uint32_t>(capacity)*0x88u+4u);
        *allocation.cookie=static_cast<std::uint32_t>(capacity);
        for (std::int32_t i=0;i<capacity;++i)
            initialize_resource_entry(allocation.entries[i]);
        for (std::int32_t i=0;i<state.count_30;++i)
            assign_resource_entry(allocation.entries[i],state.cache.entries_30[i]);
        if (state.cache.entries_30)
            hooks.deallocate_49c13d(hooks.context,state.cache.entries_30);
        state.cache.entries_30=allocation.entries;
    }
    assign_resource_entry(state.cache.entries_30[state.count_30],source);
    state.count_30=add32(state.count_30,1);
}
std::int32_t register_resource_slot(ResourceRegistration& state,const char* name,
                                    const ResourceRegistrationHooks& hooks) {
    std::int32_t index=0;
    while (index<state.count_30 && (state.cache.entries_30[index].unknown_84&0xffu)!=0)
        ++index;
    ResourceCacheEntry temporary;
    initialize_named_resource_entry(temporary,name);
    if (index==state.count_30) append_resource_entry(state,temporary,hooks);
    else assign_resource_entry(state.cache.entries_30[index],temporary);
    return index;
}
}
