#include "sprite_piece_runtime_fixture.hpp"
#include <iostream>
#include <stdexcept>

namespace {void require(bool value,const char* message){if(!value){std::cerr<<message<<'\n';std::exit(1);}}}
int main() {
    using namespace sprite_piece_test;
    static_assert(sizeof(RuntimeSpritePiece)==0x48);
    Fixture f(17);auto untouched=f.piece;
    require(&construct_runtime_sprite_piece(f.piece)==&f.piece && std::memcmp(&untouched,&f.piece,0x48)==0,"constructor writes fields");
    f.cache.entries_30=f.old_block.entries.data();f.state.count_30=3;f.state.capacity_34=6;
    initialize_named_resource_entry(f.old_block.entries[0],"Name");
    initialize_named_resource_entry(f.old_block.entries[1],"Name");
    initialize_named_resource_entry(f.old_block.entries[2],"name");
    f.old_block.entries[0].unknown_84=0x12345600;
    require(find_sprite_texture_name(f.state,"Name")==1,"inactive entry must be skipped");
    require(find_sprite_texture_name(f.state,"name")==2,"case sensitive search");
    require(!contains_sprite_texture_name(f.state,"absent")&&find_sprite_texture_name(f.state,"absent")==3,"miss returns count");
    auto h=f.runtime_hooks();std::int32_t id=-1;
    resolve_sprite_texture_name(f.state,"Name",id,h);
    require(id==1&&f.trace.empty()&&f.state.count_30==3,"existing name must not register again");
    resolve_sprite_texture_name(f.state,"new",id,h);
    require(id==0&&f.state.count_30==3&&f.old_block.entries[0].unknown_84==0x12345601,"reuse first inactive slot");
    Fixture nan(0);nan.mode=0;nan.run_piece();
    require(nan.piece.unknown_44==0xa5a5a5a5&&nan.cursor==nan.bytes.size(),"reader preserves unknown and consumes name");
    Fixture signaling(6);signaling.mode=0;signaling.run_piece();
    require(signaling.piece.double_bits_10[2]==0x7ff8000000000001ULL,"x87 signaling NaN quieting");
    Fixture mutated(16);mutated.state.count_30=0;mutated.state.capacity_34=0;mutated.mode=0;
    mutated.run_piece();
    require(mutated.piece.values_00[0]==0xabcdef12&&mutated.piece.unknown_44==0x12345678&&mutated.piece.texture_id_40==0,"lookup writes ID after allocator effects");
    Fixture short_input(0);short_input.bytes.resize(3);bool rejected=false;
    try{short_input.run_piece();}catch(const std::invalid_argument&){rejected=true;}
    require(rejected,"truncated scalar must be rejected");
    std::cout<<"sprite piece constructor, lookup, registration, NaN and callback ordering tests passed\n";
}
