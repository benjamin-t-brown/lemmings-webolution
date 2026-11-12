#pragma once
#include "lemmings/level_stream.hpp"
#include "lemmings/resource_registration.hpp"
#include <array>
#include <cstdint>

namespace lemmings {
// Native 0x48-byte piece. +40 is a registration index, not a texture pointer.
// Keep raw doubles to retain NaN payloads. +44 has no writes in these routines.
struct RuntimeSpritePiece {
    std::array<std::uint32_t,4> values_00;
    std::array<std::uint64_t,4> double_bits_10;
    std::array<std::uint32_t,4> values_30;
    std::int32_t texture_id_40;
    std::uint32_t unknown_44;
};
RuntimeSpritePiece& construct_runtime_sprite_piece(RuntimeSpritePiece&); // 484E30: no writes

// 4607D0 returns count on miss; 449B30 tests whether the index differs from
// count. Active means only the low byte of entry +84. Comparison is exact,
// case-sensitive C-string equality. Counts must be nonnegative and names live.
std::int32_t find_sprite_texture_name(const ResourceRegistration&,const char*);
bool contains_sprite_texture_name(const ResourceRegistration&,const char*);

// 45FE10 is the texture specialization of 446040; its field operations and
// allocation ordering match register_resource_slot. Registration does not load
// or decode a texture. It requires the registration module's allocation domain.
std::int32_t register_sprite_texture_name(ResourceRegistration&,const char*,
                                         const ResourceRegistrationHooks&);
// 474519 performs contains, then another lookup or registration, then writes ID.
void resolve_sprite_texture_name(ResourceRegistration&,const char*,std::int32_t&,
                                 const ResourceRegistrationHooks&);

// 485CE0, with complete memory input replacing 47EEF7. Preserves original field
// write order and x87 masked signaling-NaN quieting. Invalid/truncated input is
// rejected instead of reproducing uninitialized reads. New names must fit the
// manager's 128-byte entry; the serialized name buffer itself is 260 bytes.
void read_runtime_sprite_piece(RuntimeSpritePiece&,LevelByteStream&,
                               ResourceRegistration&,const ResourceRegistrationHooks&);
}
