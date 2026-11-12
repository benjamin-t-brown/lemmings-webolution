#pragma once
#include "lemmings/sprite_resource_destruction.hpp"
#include "lemmings/sprite_piece_runtime.hpp"

namespace lemmings {
enum class SpriteReadAllocationKind {clips,frames,pieces,triple_arrays,triples};
struct SpriteRuntimeReadHooks {
    void* context;
    // Return aligned host elements. For all kinds except triples, reserve a
    // writable uint32 cookie at elements-4. original_bytes is the x86 request;
    // host structs may be larger. triples are 24 raw bytes each, without cookie.
    void* (*allocate)(void*,SpriteReadAllocationKind,std::uint32_t count,std::uint32_t original_bytes);
    // Receives cookie address for the four array kinds, data pointer for triples.
    void (*free_memory)(void*,void*);
};

// Complete 483EF0 read using the existing runtime/destruction projections.
// Clears old resource arrays first. Normal-return domain: complete input,
// well-formed initial arrays (nonnegative capacities, backing storage for
// positive capacities), nonnegative counts, successful allocations, positive
// terminating growth, counts/capacities fitting signed32 and byte arithmetic.
// Heap/texture callbacks must preserve sprite object fields and input bytes.
// They may maintain texture registration state under that module's contract.
// Standalone resizing/copying of nonempty arrays is not claimed by this API:
// every nested reader clears before its two resize calls (reserve extra=0).
// Unwritten fields retain allocator bytes. No texture decode is performed.
void read_runtime_sprite_resource(SpriteFactoryResource&,LevelByteStream&,
                                  ResourceRegistration& textures,
                                  const ResourceRegistrationHooks& texture_hooks,
                                  const SpriteRuntimeReadHooks&);
}
