#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace lemmings {

// Offsets refer to the original 32-bit ABI. These views use native pointers.
struct SpriteRuntimeArray {
    std::uint32_t count_00;
    std::uint32_t capacity_04;
    std::uint32_t growth_08;
    void* data_0c;
};

struct SpriteFactoryResource {
    SpriteRuntimeArray clips_00;
    SpriteRuntimeArray triple_arrays_10;
};

// The file implementation is an explicit boundary, not a recovered file class.
struct SpriteFactoryFile {
    alignas(void*) std::array<std::byte, 308> opaque;
};

struct SpriteFactoryHooks {
    void* context;
    // The argument is the ORIGINAL allocation size (32), not the host size.
    // Return storage for one host SpriteFactoryResource, or null.
    SpriteFactoryResource* (*allocate)(void*, std::size_t original_bytes);
    void (*format)(void*, char* destination, const char* format, const char* name);
    void (*construct_file)(void*, SpriteFactoryFile&, const char* path);
    std::uint32_t (*open_file)(void*, SpriteFactoryFile&, std::uint32_t, std::uint32_t);
    void (*read_resource)(void*, SpriteFactoryResource*, SpriteFactoryFile&);
    void (*close_file)(void*, SpriteFactoryFile&);
    void (*destroy_file)(void*, SpriteFactoryFile&);
    void (*report_open_failure)(void*, const char* source_file, std::uint32_t line,
                                const char* path);
    // Must not return. Tests may throw here before entering original code.
    void (*terminate_process)(void*, int status);
};

// 0x485210 / 0x485270 and 0x485050.
SpriteRuntimeArray* initialize_sprite_array(SpriteRuntimeArray&) noexcept;
SpriteFactoryResource* initialize_sprite_resource(SpriteFactoryResource&) noexcept;

// 0x484EB0. All hooks are required. Name must remain alive across allocation;
// formatting must produce a terminated path within the original 260-byte buffer.
// No exceptions may cross this boundary except a nonreturning termination hook.
SpriteFactoryResource* create_sprite_resource(const char* name,
                                             const SpriteFactoryHooks&);

struct SpriteFactoryDestroyHooks {
    void* context;
    void (*clear_clips)(void*, SpriteRuntimeArray&);       // 0x484580 boundary
    void (*clear_triple_arrays)(void*, SpriteRuntimeArray&); // 0x4847A0 boundary
    void (*free_resource)(void*, SpriteFactoryResource*);
};

// 0x4850D0 with 0x4852B0 / 0x485250. Resource must be nonnull.
void destroy_sprite_resource_contents(SpriteFactoryResource&,
                                     const SpriteFactoryDestroyHooks&);
// 0x4850A0. Returns the original pointer even when freed; never dereference it
// after a freeing callback. Only bit zero of flags controls deallocation.
SpriteFactoryResource* delete_sprite_resource(SpriteFactoryResource*, std::uint32_t flags,
                                             const SpriteFactoryDestroyHooks&);
// 0x484FE7: null is accepted; otherwise calls the deleting destructor with 1.
void destroy_sprite_resource(SpriteFactoryResource*, const SpriteFactoryDestroyHooks&);

} // namespace lemmings
