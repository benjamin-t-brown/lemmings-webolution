#pragma once

#include "lemmings/sprite_factory.hpp"

namespace lemmings {

// Runtime layouts, distinct from serialized SpritePiece/SpriteClip objects.
// Offset names and strides describe the original x86 ABI; pointers use host size.
struct SpriteDestructionPiece {
    std::array<std::byte, 72> opaque_00;
};
struct SpriteDestructionFrame {
    SpriteRuntimeArray pieces_00;
};
struct SpriteDestructionClip {
    SpriteRuntimeArray frames_00;
    std::array<std::byte, 8> speed_bits_10;
    std::array<char, 128> name_18;
};
struct SpriteDestructionTripleArray {
    std::uint32_t type_00;
    void* data_04;
};

struct SpriteResourceDestructionHooks {
    void* context;
    void (*free_memory)(void*, void* allocation); // 0x49C13D
};

// Array deletion reads a uint32 cookie immediately four bytes before elements.
// It must be in [0, INT32_MAX] and no greater than the number of constructed
// elements. Host elements must be aligned and use the host layout above.
// The callback receives that cookie address, not the first element address.
// This allocation contract is NOT the compiler's native C++ new[] contract.
// The callback/context must remain valid and must not throw. It may mutate live
// headers, but must not invalidate objects that original cleanup still uses.
void clear_sprite_clips(SpriteRuntimeArray&, const SpriteResourceDestructionHooks&); // 484580
void clear_sprite_triple_arrays(SpriteRuntimeArray&, const SpriteResourceDestructionHooks&); // 4847A0
void clear_sprite_frames(SpriteRuntimeArray&, const SpriteResourceDestructionHooks&); // 484520
void clear_sprite_pieces(SpriteRuntimeArray&, const SpriteResourceDestructionHooks&); // 484930

void destroy_sprite_clip(SpriteDestructionClip&, const SpriteResourceDestructionHooks&); // 484470
void destroy_sprite_frame(SpriteDestructionFrame&, const SpriteResourceDestructionHooks&); // 4848F0/484910
void destroy_sprite_piece(SpriteDestructionPiece&) noexcept; // 484870: no effects
void destroy_sprite_triple_array(SpriteDestructionTripleArray&,
                                 const SpriteResourceDestructionHooks&); // 47C5D3

// Vector deleting destructors: bit 1 selects array versus scalar; bit 0 frees.
// Array destruction walks the cookie count backwards, independent of a header's
// logical count/capacity. Return is cookie address for arrays, object for scalar.
// Arguments must be nonnull; returned pointers may have been freed.
void* delete_sprite_clips(SpriteDestructionClip*, std::uint32_t flags,
                          const SpriteResourceDestructionHooks&); // 484990
void* delete_sprite_frames(SpriteDestructionFrame*, std::uint32_t flags,
                           const SpriteResourceDestructionHooks&); // 484880
void* delete_sprite_pieces(SpriteDestructionPiece*, std::uint32_t flags,
                           const SpriteResourceDestructionHooks&); // 484800
void* delete_sprite_triple_arrays(SpriteDestructionTripleArray*, std::uint32_t flags,
                                  const SpriteResourceDestructionHooks&); // 484A50

} // namespace lemmings
