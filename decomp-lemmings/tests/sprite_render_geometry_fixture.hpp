#pragma once
#include "lemmings/sprite_render_geometry.hpp"
#include <bit>
#include <cmath>
#include <cstring>
#include <vector>

namespace sprite_geometry_test {
using namespace lemmings;
struct Fixture {
    SpriteRenderGeometry model;
    SpriteFactoryResource resource;
    std::array<SpriteDestructionClip,3> clips;
    std::array<std::array<SpriteDestructionFrame,3>,3> frames;
    std::array<std::array<std::array<SpriteDestructionPiece,3>,3>,3> pieces;
    std::array<double,4> arguments;
    std::uint32_t mode,reference;
    unsigned trial,calls=0;
    std::array<std::uint64_t,17> submitted{};
    explicit Fixture(unsigned seed):trial(seed) {
        std::memset(&model,0xa5,sizeof(model));std::memset(&resource,0xa5,sizeof(resource));
        std::uint64_t random=seed+1;auto next=[&]{random=random*6364136223846793005ULL+1;return random;};
        resource.clips_00={3,3,1,clips.data()};
        for(unsigned i=0;i<3;++i) {
            clips[i].frames_00={3,3,1,frames[i].data()};
            for(unsigned j=0;j<3;++j) {
                frames[i][j].pieces_00={3,3,1,pieces[i][j].data()};
                for(auto& storage:pieces[i][j]) {
                    RuntimeSpritePiece p;
                    for(auto& x:p.values_00)x=static_cast<std::uint32_t>(next());
                    for(auto& x:p.double_bits_10)x=next();
                    for(auto& x:p.values_30)x=static_cast<std::uint32_t>(next());
                    p.texture_id_40=std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(next()));p.unknown_44=static_cast<std::uint32_t>(next());
                    std::memcpy(storage.opaque_00.data(),&p,sizeof(p));
                }
            }
        }
        model.resource_1f8=&resource;model.clip_index_1c8=seed%3;model.frame_index_1cc=(seed/3)%3;model.piece_index_1b8=(seed/9)%3;
        model.swap_endpoints_1b0=seed%4==0?0:std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(next())|1u);
        reference=static_cast<std::uint32_t>(next());
        const std::uint32_t differences[]={0,0xffffffffu,1,2,0x80000000u};mode=reference+differences[seed%5];
        for(auto& x:arguments) {
            const auto bits=(next()&0x800fffffffffffffULL)|(std::uint64_t(960+next()%128)<<52);
            x=std::bit_cast<double>(bits);
        }
        if(seed<256) {
            constexpr std::uint64_t special[]={0,0x8000000000000000ULL,1,0x8000000000000001ULL,
                0x7fefffffffffffffULL,0xffefffffffffffffULL,0x7ff0000000000000ULL,0xfff0000000000000ULL,
                0x7ff0000000000001ULL,0xfff8000000001234ULL,0x0010000000000000ULL,0x8010000000000000ULL};
            for(unsigned i=0;i<4;++i)arguments[i]=std::bit_cast<double>(special[(seed+i)%12]);
        }
    }
    void submit() {
        unsigned at=0;
        for(const auto& v:model.vertices_08) {
            submitted[at++]=std::bit_cast<std::uint64_t>(v.x_18);submitted[at++]=std::bit_cast<std::uint64_t>(v.y_20);
            submitted[at++]=v.coordinate_bits_28;submitted[at++]=v.coordinate_bits_30;
        }
        submitted[16]=static_cast<std::uint32_t>(model.texture_id_1a8);++calls;
        if(trial&8){model.vertices_08[0].x_18=19;model.texture_id_1a8=-123;model.unknown_1b4[2]=std::byte{0x42};}
    }
    void run() {
        const SpriteRenderGeometryHooks hooks{this,[](void* p,SpriteRenderGeometry&){static_cast<Fixture*>(p)->submit();}};
        build_sprite_rectangle(model,arguments[0],arguments[1],arguments[2],arguments[3],mode,reference,hooks);
    }
    bool same(const Fixture& other)const {
        auto left=model,normalized=other.model;normalized.resource_1f8=model.resource_1f8;
        auto first=submitted,second=other.submitted;
        for(unsigned i=0;i<4;++i) {
            for(unsigned j=0;j<2;++j)if(std::isnan(std::bit_cast<double>(first[i*4+j]))&&std::isnan(std::bit_cast<double>(second[i*4+j])))
                first[i*4+j]=second[i*4+j]=0x7ff8000000000000ULL;
            if(std::isnan(left.vertices_08[i].x_18)&&std::isnan(normalized.vertices_08[i].x_18))left.vertices_08[i].x_18=normalized.vertices_08[i].x_18=0;
            if(std::isnan(left.vertices_08[i].y_20)&&std::isnan(normalized.vertices_08[i].y_20))left.vertices_08[i].y_20=normalized.vertices_08[i].y_20=0;
        }
        return calls==other.calls&&first==second&&std::memcmp(&left,&normalized,sizeof(model))==0;
    }
};
}
