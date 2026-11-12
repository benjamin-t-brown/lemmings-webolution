#include "actor_shadow_fixture.hpp"
#include <bit>
#include <cassert>

using namespace shadow_test;
int main() {
    std::array<std::int32_t, 32*256> cells{};
    CollisionGridHead grid{};grid.height=32;grid.cells.data=cells.data();
    std::array<WorldGeometryPrefix::TripleBlock,2> rows{};
    for(unsigned r=0;r<2;++r)for(unsigned x=0;x<64;++x) {
        rows[r][x]={std::bit_cast<std::uint64_t>(double(x%32)*8),
                    std::bit_cast<std::uint64_t>(double(r)*16),
                    std::bit_cast<std::uint64_t>(double(x/32))};
    }
    WorldCoordinateMesh mesh{32,rows};
    ShadowActor actor;actor.geometry.position={11,6,0.5};
    ActorShadow shadow;shadow.actor_3a8=&actor;shadow.mode_120=99;
    ActorShadowGlobals globals;Effects effects;
    actor.geometry.direction_44=-1;
    update_actor_shadow(shadow,globals,grid,mesh,effects);
    assert(shadow.visible_3ad==0 && shadow.position_3b0==actor.geometry.position);
    assert((effects.calls==std::vector<int>{1,11,2}));
    effects.calls.clear();actor.geometry.direction_44=1;effects.has=0x80;
    update_actor_shadow(shadow,globals,grid,mesh,effects);
    assert(shadow.visible_3ad==0 && effects.calls.back()==2 && shadow.mode_120==99);
    effects.has=0;cells[5*256+11]=1;
    update_actor_shadow(shadow,globals,grid,mesh,effects);
    assert(shadow.visible_3ad==1 && shadow.intensity_3c8==255 && globals.width_4c05c8==2.5);
    assert(shadow.mode_120==1 && effects.left==9.75 && effects.right==12.25);
    assert((shadow.uv_128[0][2]==std::array<double,2>{0.5,1}));
    effects.points.push_back({13,7,0});
    update_actor_shadow(shadow,globals,grid,mesh,effects);
    assert(shadow.mode_120==5 && shadow.uv_128[4][3][0]==1);
    cells.fill(0);cells[3*256+11]=1;
    update_actor_shadow(shadow,globals,grid,mesh,effects);
    assert(shadow.visible_3ad==1 && shadow.intensity_3c8==178 && shadow.position_3b0[1]==4);
    actor.geometry.position[1]=20;
    update_actor_shadow(shadow,globals,grid,mesh,effects);
    assert(shadow.visible_3ad==0 && shadow.mode_120==5 && shadow.intensity_3c8==178);
}
