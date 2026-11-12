#include "lemmings/sprite_piece_runtime.hpp"
#include <cstring>
#include <stdexcept>

namespace lemmings {
namespace {
std::uint64_t read_integer(LevelByteStream& stream,unsigned count) {
    if(stream.cursor>stream.bytes.size() || count>stream.bytes.size()-stream.cursor)
        throw std::invalid_argument("truncated runtime sprite piece");
    std::uint64_t result=0;
    for(unsigned i=0;i<count;++i)result|=std::uint64_t(stream.bytes[stream.cursor++])<<(8*i);
    return result;
}
std::uint64_t x87_loaded_double(std::uint64_t bits) {
    if((bits&0x7ff0000000000000ULL)==0x7ff0000000000000ULL && (bits&0xfffffffffffffULL))
        bits|=0x8000000000000ULL;
    return bits;
}
}
RuntimeSpritePiece& construct_runtime_sprite_piece(RuntimeSpritePiece& piece) {return piece;}
std::int32_t find_sprite_texture_name(const ResourceRegistration& state,const char* name) {
    std::int32_t index=0;
    while(index!=state.count_30) {
        const auto& entry=state.cache.entries_30[index];
        if((entry.unknown_84&0xffu)!=0 && std::strcmp(name,entry.name_00.data())==0)break;
        ++index;
    }
    return index;
}
bool contains_sprite_texture_name(const ResourceRegistration& state,const char* name) {
    return find_sprite_texture_name(state,name)!=state.count_30;
}
std::int32_t register_sprite_texture_name(ResourceRegistration& state,const char* name,
                                         const ResourceRegistrationHooks& hooks) {
    return register_resource_slot(state,name,hooks);
}
void resolve_sprite_texture_name(ResourceRegistration& state,const char* name,std::int32_t& id,
                                 const ResourceRegistrationHooks& hooks) {
    const auto result=contains_sprite_texture_name(state,name)
        ? find_sprite_texture_name(state,name):register_sprite_texture_name(state,name,hooks);
    id=result;
}
void read_runtime_sprite_piece(RuntimeSpritePiece& piece,LevelByteStream& stream,
                               ResourceRegistration& state,const ResourceRegistrationHooks& hooks) {
    for(auto& value:piece.values_00)value=static_cast<std::uint32_t>(read_integer(stream,4));
    for(auto& value:piece.double_bits_10)value=x87_loaded_double(read_integer(stream,8));
    for(auto& value:piece.values_30)value=static_cast<std::uint32_t>(read_integer(stream,4));
    char name[260];
    read_level_string(stream,name,260);
    resolve_sprite_texture_name(state,name,piece.texture_id_40,hooks);
}
}
