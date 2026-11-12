#include "lemmings/world_coordinates.hpp"
#include <bit>
#include <cmath>
#include <iostream>
#include <stdexcept>

int main() {
    lemmings::WorldGeometryPrefix world;
    world.leading_value=32;
    world.first_blocks.resize(2);
    for(int row=0;row<2;++row) for(int layer=0;layer<2;++layer)
        for(int sector=0;sector<32;++sector) {
            const std::array<double,3> point{sector*8.0+layer*2.0,row*16.0,
                                           layer*4.0+row*2.0};
            for(int j=0;j<3;++j) world.first_blocks[row][layer*32+sector][j]=
                std::bit_cast<std::uint64_t>(point[j]);
        }
    const auto mesh=lemmings::world_coordinate_mesh(world);
    const auto p=lemmings::map_world_coordinates(mesh,{12,8,0.25});
    if(p!=std::array<double,3>{12.5,8,2}) return 1;
    const auto seam=lemmings::map_world_coordinates(mesh,{252,0,0});
    if(seam!=std::array<double,3>{124,0,0}) return 1;
    const auto top=lemmings::map_world_coordinates(mesh,{8,32,0.5});
    if(top!=std::array<double,3>{9,16,4}) return 1;
    // Largest radial component is Z: moving up one row adds two world units.
    if(lemmings::world_depth_at_y(mesh,{8,0,0.5},16)!=1.0) return 1;
    bool rejected=false;
    try {lemmings::map_world_coordinates(mesh,{0,-16,0});}
    catch(const std::out_of_range&) {rejected=true;}
    if(!rejected) return 1;
    std::cout<<"world coordinate boundary checks passed\n";
}
