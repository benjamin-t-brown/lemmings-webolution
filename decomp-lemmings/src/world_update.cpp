#include "lemmings/world_update.hpp"
#include <bit>
namespace lemmings {
namespace {
double stored_double(long double value) {volatile double stored=static_cast<double>(value);return stored;}
constexpr double animation_interval=std::bit_cast<double>(std::uint64_t{0x3faeb851eb851eb8});
constexpr double sequence_interval=std::bit_cast<double>(std::uint64_t{0x3fa999999999999a});
}
void update_world(WorldUpdateState& w,WorldEntityPass& entities,double dt,const WorldUpdateHooks& h,const WorldEntityPassHooks& collections) {
    if(!w.enabled_5139){w.status_00=1;return;}
    w.elapsed_5120=dt;
    auto* c=h.context;
    h.update_437bde(c,dt);
    if(w.optional_50b8)h.update_optional(c,0x4911e9,w.optional_50b8,dt);
    if(w.optional_50c0)h.update_optional(c,0x4457e0,w.optional_50c0,dt);
    if(w.optional_50b4)h.update_optional(c,0x49185c,w.optional_50b4,dt);
    if(w.optional_50bc)h.update_optional(c,0x49259c,w.optional_50bc,dt);
    if(w.optional_50c8)h.update_optional(c,0x4457f0,w.optional_50c8,dt);
    if(w.animations_538c.count==0){
        const auto id=h.first_resource_412e90(c);
        void* payload=h.lookup_payload_414a10(c,id);
        WorldAnimationObject first,second;
        h.initialize_animation_447390(c,first);h.initialize_animation_447390(c,second);
        h.configure_animation_47abe0(c,first,payload,0xf0,0xf7,animation_interval);
        h.configure_animation_47abe0(c,second,payload,0xf8,0xff,animation_interval);
        h.append_animation_448050(c,first);h.append_animation_448050(c,second);
        h.destroy_animation_47ac05(c,second);h.destroy_animation_47ac05(c,first);
    }
    h.update_animation_47ac1b(c,w.animations_538c.data[0],dt);
    h.update_animation_47ac1b(c,w.animations_538c.data[1],dt);
    if(w.sequence_enabled_5138){
        if(w.sequence_count_5308==0){
            if(!w.sequence_539c&&!w.sequence_done_50d3){
                w.sequence_ready_53a0=0;
                w.sequence_539c=h.create_sequence_413d40(c,5,5);
                double zero=0;h.sequence_update_04(c,w.sequence_539c,zero);
            }
            w.sequence_cursor_5304=entities.list_1ec8.end.previous;
        }
        if(h.query_445436(c)!=0){h.action_444a34(c);w.value_5098=0;}
        double remaining=dt;
        if(w.sequence_539c){
            h.sequence_position_414132(c,w.sequence_539c,0x140,0xf0,1);
            if(h.sequence_update_04(c,w.sequence_539c,remaining)==1){
                h.sequence_leave_0c(c,w.sequence_539c);
                if(w.sequence_539c)h.destroy_00(c,w.sequence_539c,1);
                w.sequence_539c=nullptr;w.sequence_ready_53a0=1;w.sequence_done_50d3=1;
            }
        }
        // x87 C0 alone is tested: unordered (NaN) also enters this branch.
        if(w.sequence_ready_53a0&&w.sequence_cursor_5304!=&entities.list_1ec8.begin&&!(w.sequence_timer_5128>=0.0)){
            auto* actor=w.sequence_cursor_5304->entity;
            void* old=actor->primary_behavior_340;
            void* next=h.create_behavior_416910(c);
            h.set_primary_42e696(c,*actor,next);
            if(old!=actor->primary_behavior_340&&old)h.destroy_00(c,old,1);
            w.sequence_cursor_5304=w.sequence_cursor_5304->previous;
            ++w.sequence_count_5308;
            w.sequence_timer_5128=stored_double(static_cast<long double>(sequence_interval)+w.sequence_timer_5128);
        }
        w.sequence_timer_5128=stored_double(static_cast<long double>(w.sequence_timer_5128)-dt);
    }
    for(std::int32_t i=0;i<w.records_5100.count;++i){auto& r=w.records_5100.data[i];if(!r.paused_168)r.remaining_170=stored_double(static_cast<long double>(r.remaining_170)-dt);}
    h.before_collections_43b419(c);
    for(std::int32_t i=0;i<w.auxiliary_1fd0.count;++i)h.update_auxiliary_41f92e(c,w.auxiliary_1fd0.data[i],dt);
    update_world_entity_collections(entities,dt,collections);
    h.tail_45af0e(c,dt);h.tail_44d22c(c,dt);h.tail_43b150(c);h.tail_445498(c,dt);
}
}
