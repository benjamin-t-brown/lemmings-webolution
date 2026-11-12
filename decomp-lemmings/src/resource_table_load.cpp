#include "lemmings/resource_table_load.hpp"
#include "lemmings/sprite_piece_runtime.hpp"
#include <bit>
#include <cstring>
#include <stdexcept>

namespace lemmings {
namespace {
std::int32_t add32(std::int32_t a,std::uint32_t b) {
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(a)+b);
}
std::int32_t read_count(LevelByteStream& stream) {
    if(stream.cursor>stream.bytes.size() || stream.bytes.size()-stream.cursor<4)
        throw std::invalid_argument("truncated resource table count");
    std::uint32_t value=0;
    for(unsigned i=0;i<4;++i)value|=std::uint32_t(stream.bytes[stream.cursor++])<<(8*i);
    return std::bit_cast<std::int32_t>(value);
}
}
void resize_resource_entries(ResourceRegistration& state,std::int32_t requested,
                             const ResourceRegistrationHooks& hooks) {
    if(state.capacity_34<requested) {
        if(state.growth_38==1)state.capacity_34=requested;
        else do {
            state.capacity_34=add32(state.capacity_34,static_cast<std::uint32_t>(state.growth_38));
        } while(state.capacity_34<=requested);
        if(state.capacity_34<1) {
            if(state.cache.entries_30)hooks.deallocate_49c13d(hooks.context,state.cache.entries_30);
            state.cache.entries_30=nullptr;
        } else {
            const auto capacity=state.capacity_34;
            const auto allocation=hooks.allocate_49c1a8(hooks.context,
                static_cast<std::uint32_t>(capacity)*0x88u+4u);
            *allocation.cookie=static_cast<std::uint32_t>(capacity);
            for(std::int32_t i=0;i<capacity;++i)initialize_resource_entry(allocation.entries[i]);
            if(state.cache.entries_30) {
                for(std::int32_t i=0;i<state.count_30;++i)
                    assign_resource_entry(allocation.entries[i],state.cache.entries_30[i]);
                if(state.cache.entries_30)hooks.deallocate_49c13d(hooks.context,state.cache.entries_30);
            }
            state.cache.entries_30=allocation.entries;
        }
    }
    state.count_30=requested;
}
void load_resource_name_table(ResourceCacheLookupManager& manager,std::uint32_t& loaded_count,
    ResourceRegistration& state,LevelByteStream& stream,const ResourceRegistrationHooks& allocation,
    const ResourceCacheCleanupHooks& cleanup) {
    clear_unused_resource_flags_and_evict(manager,loaded_count,state.cache,state.count_30,cleanup);
    const auto incoming=read_count(stream);
    resize_resource_entries(state,add32(incoming,loaded_count),allocation);
    std::uint32_t duplicates=0;
    std::int32_t free_index=0;
    for(std::int32_t i=0;i<incoming;++i) {
        char name[260];
        read_level_string(stream,name,255);
        if(contains_sprite_texture_name(state,name))++duplicates;
        else {
            while((state.cache.entries_30[free_index].unknown_84&0xffu)!=0)++free_index;
            auto& entry=state.cache.entries_30[free_index];
            std::strcpy(entry.name_00.data(),name);
            entry.cached_80=nullptr;
            entry.unknown_84=(entry.unknown_84&0xffffff00u)|1u;
        }
    }
    resize_resource_entries(state,add32(incoming,loaded_count-duplicates),allocation);
}
}
