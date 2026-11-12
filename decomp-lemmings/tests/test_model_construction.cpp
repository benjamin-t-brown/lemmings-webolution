#include "lemmings/model_construction.hpp"
#include <bit>
#include <cassert>
#include <iostream>

int main() {
    using namespace lemmings;
    ModelConstructionState actor;
    actor.buffer_08.fill(0xa5);actor.position_90={0x7ff0000000000001ULL,0x8000000000000000ULL,0xffffffffffffffffULL};
    actor.unknown_04=23;actor.unknown_8c=45;actor.unknown_ac=67;actor.unknown_2b2.fill(0xcd);actor.unknown_2b9.fill(0xef);
    auto& animation=actor.animation_b0;
    animation.unknown_1ac=1;animation.unknown_1b4=2;animation.unknown_1bc=3;animation.unknown_1d4=4;
    animation.playback={9,7,8,-7,reinterpret_cast<const AnimationResource*>(0x12340000),99,55,66};
    animation.field_1d8=-0.0;animation.field_1b0=10;animation.field_1b8=11;animation.triple_1e0={1,2,3};
    for(auto& corner:animation.quad.corners_08) {
        corner.triple_00={1,2,3};corner.pair_18={4,5};corner.pair_28={6,7};corner.unknown_58={8,9};
    }
    const auto initial=actor;
    initialize_actor_model(actor);
    assert(actor.original_vtable==0x4b14a8 && actor.mode_88==1 && actor.field_a8==1);
    assert(actor.buffer_08[0]==0 && actor.buffer_08[1]==0xa5 && actor.buffer_08[127]==0xa5);
    assert(actor.position_90==initial.position_90 && actor.unknown_04==23 && actor.unknown_8c==45 && actor.unknown_ac==67);
    assert(actor.unknown_2b2==initial.unknown_2b2 && actor.unknown_2b9==initial.unknown_2b9);
    assert(actor.flag_2b8==1 && !actor.field_2c0 && !actor.field_2c4);
    assert(animation.field_1a8==-1 && !animation.field_1b0 && !animation.field_1b8);
    assert(animation.unknown_1ac==1 && animation.unknown_1b4==2 && animation.unknown_1bc==3 && animation.unknown_1d4==4);
    assert(animation.playback.direction_1d0==1 && animation.playback.looping_201==1 && !animation.playback.finished_200);
    assert(!animation.playback.resource_1f8 && animation.playback.resource_id_1fc==-1);
    assert(animation.playback.clip_1c8==0 && animation.playback.frame_1cc==0);
    assert(std::bit_cast<std::uint64_t>(animation.playback.fraction_1c0)==0);
    assert(std::bit_cast<std::uint64_t>(animation.field_1d8)==0);
    assert((animation.triple_1e0==std::array<double,3>{0,0,-1}));
    for(auto& c:animation.quad.corners_08) {
        assert(c.triple_00[0]==1 && c.triple_00[1]==2 && c.triple_00[2]==0x3f6d7dbf487fcb92ULL);
        assert((c.pair_18==std::array<std::uint64_t,2>{4,5}) && (c.pair_28==std::array<std::uint64_t,2>{6,7}));
        assert((c.unknown_58==std::array<std::uint64_t,2>{8,9}));
        for(auto value:c.triple_38)assert(value==0x406fe00000000000ULL);
        assert(c.field_50==0x406fe00000000000ULL);
    }
    auto base=initial;initialize_model_base(base);
    assert(base.mode_88==3 && base.original_vtable==0x4b1858 && base.animation_b0.playback.resource_id_1fc==99);
    auto quad=initial.animation_b0;initialize_animation_base(quad);
    assert(quad.field_1a8==-1 && quad.playback.resource_id_1fc==99 && quad.field_1b0==10);
    std::cout<<"model construction units passed\n";
}
