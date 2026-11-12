#pragma once
#include "lemmings/world_entity_pass.hpp"
#include <array>
#include <cstddef>
namespace lemmings {
struct WorldAnimationObject {alignas(8) std::array<std::byte,0x28> unknown;};
struct WorldTimedRecord {
    std::array<std::byte,0x168> unknown_prefix;
    std::uint8_t paused_168;
    std::array<std::byte,7> unknown_169;
    double remaining_170;
    std::array<std::byte,8> unknown_178;
};
static_assert(sizeof(WorldTimedRecord)==0x180);
static_assert(offsetof(WorldTimedRecord,paused_168)==0x168);
static_assert(offsetof(WorldTimedRecord,remaining_170)==0x170);
struct WorldAuxiliaryRecord {std::array<std::byte,0x178> unknown;};
template<class T> struct WorldInlineArray {std::int32_t count=0;T* data=nullptr;};
struct WorldUpdateState {
    std::int32_t status_00=0;
    void* optional_50b8=nullptr;void* optional_50c0=nullptr;
    void* optional_50b4=nullptr;void* optional_50bc=nullptr;void* optional_50c8=nullptr;
    std::uint8_t sequence_done_50d3=0;
    std::uint16_t value_5098=0;
    WorldInlineArray<WorldTimedRecord> records_5100;
    double elapsed_5120=0,sequence_timer_5128=0;
    std::uint8_t sequence_enabled_5138=0,enabled_5139=0;
    WorldEntityNode* sequence_cursor_5304=nullptr;
    std::uint16_t sequence_count_5308=0;
    WorldInlineArray<WorldAnimationObject> animations_538c;
    void* sequence_539c=nullptr;
    std::uint8_t sequence_ready_53a0=0;
    WorldInlineArray<WorldAuxiliaryRecord> auxiliary_1fd0;
};
struct WorldUpdateHooks {
    void* context=nullptr;
    void (*update_437bde)(void*,double);
    void (*update_optional)(void*,std::uint32_t routine,void* object,double);
    std::uint32_t (*first_resource_412e90)(void*);
    void* (*lookup_payload_414a10)(void*,std::uint32_t); // returns resource+1C
    void (*initialize_animation_447390)(void*,WorldAnimationObject&);
    void (*configure_animation_47abe0)(void*,WorldAnimationObject&,void* payload,int first,int last,double);
    void (*append_animation_448050)(void*,WorldAnimationObject&);
    void (*destroy_animation_47ac05)(void*,WorldAnimationObject&);
    void (*update_animation_47ac1b)(void*,WorldAnimationObject&,double);
    void* (*create_sequence_413d40)(void*,int,int); // allocation plus constructor
    int (*sequence_update_04)(void*,void* sequence,double& remaining); // actor argument is null
    void (*sequence_position_414132)(void*,void*,int,int,std::uint8_t);
    void (*sequence_leave_0c)(void*,void*); // actor argument is null
    void (*destroy_00)(void*,void*,unsigned flags);
    int (*query_445436)(void*);
    void (*action_444a34)(void*);
    void* (*create_behavior_416910)(void*); // allocation plus constructor; may return null
    void (*set_primary_42e696)(void*,WorldUpdateEntity&,void*);
    void (*before_collections_43b419)(void*);
    void (*update_auxiliary_41f92e)(void*,WorldAuxiliaryRecord&,double);
    void (*tail_45af0e)(void*,double);
    void (*tail_44d22c)(void*,double);
    void (*tail_43b150)(void*);
    void (*tail_445498)(void*,double);
};
// Complete normal control flow at 0x4423D2, composing the recovered collection
// pass. External method bodies, allocation and exception unwinding are hooks.
// Valid original object/array lifetimes and successful sequence creation are
// required; an allocation failure before its unconditional virtual call faults
// in the original. Callback-visible reads follow original instruction order.
void update_world(WorldUpdateState&,WorldEntityPass&,double,const WorldUpdateHooks&,const WorldEntityPassHooks&);
}
