#include "lemmings/sprite_resource_destruction.hpp"

#include <cstring>

namespace lemmings {
namespace {
template<class T, class Destroy>
void* delete_objects(T* elements, std::uint32_t flags,
                     const SpriteResourceDestructionHooks& hooks, Destroy destroy) {
    void* allocation = elements;
    if ((flags & 2u) != 0) {
        auto* cookie = reinterpret_cast<std::byte*>(elements) - 4;
        std::uint32_t count;
        std::memcpy(&count, cookie, sizeof count);
        while (count != 0) {
            --count;
            destroy(elements[count]);
        }
        allocation = cookie;
    } else {
        destroy(*elements);
    }
    if ((flags & 1u) != 0) hooks.free_memory(hooks.context, allocation);
    return allocation;
}

template<class T, class Delete>
void clear_array(SpriteRuntimeArray& array, Delete delete_elements) {
    if (array.data_0c != nullptr) {
        array.capacity_04 = 0;
        auto* elements = static_cast<T*>(array.data_0c);
        delete_elements(elements);
        array.data_0c = nullptr;
    }
    array.count_00 = 0;
}
} // namespace

void clear_sprite_clips(SpriteRuntimeArray& array, const SpriteResourceDestructionHooks& hooks) {
    clear_array<SpriteDestructionClip>(array, [&](auto* elements) { delete_sprite_clips(elements, 3, hooks); });
}
void clear_sprite_triple_arrays(SpriteRuntimeArray& array, const SpriteResourceDestructionHooks& hooks) {
    clear_array<SpriteDestructionTripleArray>(array, [&](auto* elements) { delete_sprite_triple_arrays(elements, 3, hooks); });
}
void clear_sprite_frames(SpriteRuntimeArray& array, const SpriteResourceDestructionHooks& hooks) {
    clear_array<SpriteDestructionFrame>(array, [&](auto* elements) { delete_sprite_frames(elements, 3, hooks); });
}
void clear_sprite_pieces(SpriteRuntimeArray& array, const SpriteResourceDestructionHooks& hooks) {
    clear_array<SpriteDestructionPiece>(array, [&](auto* elements) { delete_sprite_pieces(elements, 3, hooks); });
}

void destroy_sprite_clip(SpriteDestructionClip& clip, const SpriteResourceDestructionHooks& hooks) {
    // The inline 128-byte name destructor 0x41A4B0 has no effects.
    clear_sprite_frames(clip.frames_00, hooks);
}
void destroy_sprite_frame(SpriteDestructionFrame& frame, const SpriteResourceDestructionHooks& hooks) {
    clear_sprite_pieces(frame.pieces_00, hooks);
}
void destroy_sprite_piece(SpriteDestructionPiece&) noexcept {}
void destroy_sprite_triple_array(SpriteDestructionTripleArray& array,
                                 const SpriteResourceDestructionHooks& hooks) {
    if (array.data_04 != nullptr) hooks.free_memory(hooks.context, array.data_04);
    // 0x47C800 has no effects; the data pointer and type remain unchanged.
}

void* delete_sprite_clips(SpriteDestructionClip* elements, std::uint32_t flags,
                          const SpriteResourceDestructionHooks& hooks) {
    return delete_objects(elements, flags, hooks, [&](auto& value) { destroy_sprite_clip(value, hooks); });
}
void* delete_sprite_frames(SpriteDestructionFrame* elements, std::uint32_t flags,
                           const SpriteResourceDestructionHooks& hooks) {
    return delete_objects(elements, flags, hooks, [&](auto& value) { destroy_sprite_frame(value, hooks); });
}
void* delete_sprite_pieces(SpriteDestructionPiece* elements, std::uint32_t flags,
                           const SpriteResourceDestructionHooks& hooks) {
    return delete_objects(elements, flags, hooks, [&](auto& value) { destroy_sprite_piece(value); });
}
void* delete_sprite_triple_arrays(SpriteDestructionTripleArray* elements, std::uint32_t flags,
                                  const SpriteResourceDestructionHooks& hooks) {
    return delete_objects(elements, flags, hooks, [&](auto& value) { destroy_sprite_triple_array(value, hooks); });
}
} // namespace lemmings
