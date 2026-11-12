#include "lemmings/actor_geometry.hpp"
#include <cassert>
#include <limits>

int main() {
    lemmings::ActorGeometryState actor{};
    actor.position={10,5,9};
    actor.pair_350={7,8};actor.pair_360={-1,3};
    const auto point=lemmings::actor_sample_position_360(actor);
    assert((point==std::array<double,3>{9,8,9}));
    lemmings::reverse_actor_direction_40(actor);
    assert(actor.direction_40==-1&&actor.pair_350[0]==-1&&actor.pair_360[0]==-1);
    lemmings::reverse_actor_direction_44(actor);
    assert(actor.direction_44==-1&&actor.flag_5c==1);
    assert(actor.pair_350[1]==0&&actor.pair_360[1]==3);
    actor.direction_40=std::numeric_limits<std::int32_t>::min();
    lemmings::reverse_actor_direction_40(actor);
    assert(actor.direction_40==std::numeric_limits<std::int32_t>::min());
    assert(actor.pair_350[0]==0&&actor.pair_360[0]==0);
    assert(!lemmings::actor_accepts_terrain_value(0,0x200));
    assert(lemmings::actor_accepts_terrain_value(1,0x200));
    assert(lemmings::actor_accepts_terrain_value(2,0x4000));
    assert(!lemmings::actor_accepts_terrain_value(2,0x2000));
    assert(lemmings::actor_accepts_terrain_value(0,0x201));
    const double nan=std::numeric_limits<double>::quiet_NaN();
    assert(lemmings::actor_contains_xy_point(actor,{9,4,nan}));
    assert(lemmings::actor_contains_xy_point(actor,{11,6,nan}));
    assert(!lemmings::actor_contains_xy_point(actor,{11.001,6,0}));
    assert(!lemmings::actor_contains_xy_point(actor,{nan,5,0}));
    std::array<std::int32_t,10*256> cells{};
    lemmings::CollisionGridHead grid{};
    grid.height=10;grid.cells.data=cells.data();
    actor.position={10,5,0};actor.pair_360={-1,0};
    actor.direction_40=-1;actor.direction_44=1;actor.category_4c=0;
    cells[4*256+10]=1;
    assert(lemmings::actor_has_terrain_contact(actor,0,grid));
    assert(!lemmings::actor_has_terrain_contact(actor,4,grid));
    assert(!lemmings::actor_has_terrain_contact(actor,5,grid));
    cells[4*256+10]=0x81;
    assert(!lemmings::actor_has_terrain_contact(actor,0,grid));
    cells[4*256+10]=0x1001;
    assert(!lemmings::actor_has_terrain_contact(actor,0,grid));
    cells[4*256+10]=0x200;
    assert(!lemmings::actor_has_terrain_contact(actor,0,grid));
    actor.category_4c=1;
    assert(lemmings::actor_has_terrain_contact(actor,0,grid));
    actor.position={10,2,0};actor.pair_360={0,0};
    actor.direction_40=1;actor.direction_44=1;
    cells.fill(0);assert(!lemmings::actor_has_wall_contact(actor,grid));
    for(int y=2;y<6;++y)cells[y*256+11]=1;
    assert(lemmings::actor_has_wall_contact(actor,grid));
    cells[3*256+11]=0x80;
    assert(lemmings::actor_has_wall_contact(actor,grid));
    cells[4*256+11]=0;
    assert(!lemmings::actor_has_wall_contact(actor,grid));
    cells.fill(0x80);assert(lemmings::actor_has_wall_contact(actor,grid));
    actor.direction_44=0;cells[2*256+11]=0;
    assert(!lemmings::actor_has_wall_contact(actor,grid));
}
