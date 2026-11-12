#pragma once
#include <cstdint>
namespace lemmings {
// Array header used by global0x4CBB18 and world-coordinate object+0x1E88.
// Pointer width is native to the host; stored counters retain original32bits.
struct ActorRegistry {
    std::int32_t count=0,capacity=0,growth=0;
    void** data=nullptr;
};
struct ActorRegistryAllocation {
    void* context=nullptr;
    // byte_count is the original32bit allocation size (capacity<<2). Host
    // implementations allocate byte_count/4 pointer slots of native width.
    void** (*allocate)(void*,std::uint32_t byte_count)=nullptr;
    void (*release)(void*,void**)=nullptr;
};
// 0x4233A0. Input is read after allocation/copy/release, through its live
// reference. Callbacks may mutate header/input but must retain valid buffers.
// Positive capacity/growth and sufficient storage after callbacks required.
void append_actor_registry(ActorRegistry&,void*& value,const ActorRegistryAllocation&);
// 0x43B517 wraps append with a stack-local copy of the pointer argument.
void register_world_actor(ActorRegistry&,void* value,const ActorRegistryAllocation&);
// 0x43BB90: shifts later entries left, decrements count and retains stale tail.
// Index must refer to a live entry; original has no bounds check.
void erase_actor_registry(ActorRegistry&,std::int32_t index);
// 0x43B536: removes first exact pointer match, preserving later duplicates.
void unregister_world_actor(ActorRegistry&,void* value);
}
