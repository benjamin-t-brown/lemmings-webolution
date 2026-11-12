#pragma once
#include "lemmings/resource_cache.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace lemmings {
// Original x86 offsets; host pointers expand on 64-bit builds. Unknown bytes
// are deliberately retained. No ownership or default initialization is implied.
struct TextureCreationResource {
    void* device_texture_00;
    void* bitmap_04;
    void* unknown_08;
    void* alpha_0c;
    std::uint8_t unknown_10;
    std::array<std::uint8_t, 3> unknown_11;
    std::int32_t width_14;
    std::int32_t height_18;
    std::uint32_t triple_type_1c;
    void* triple_data_20;
    std::uint32_t flags_24;
};
struct TextureCreationFile { alignas(void*) std::array<std::byte, 308> opaque; };
struct TextureBitmapBuffer { std::uint32_t size_00; void* data_04; };

struct TextureBitmapHooks {
    void* context;
    // 45015A file/packed decoder boundary. Must assign size and data. The buffer
    // stays live until free_data returns; callbacks may retain and mutate it.
    void (*load)(void*, TextureBitmapBuffer&, const char* path);
    void (*free_data)(void*, void*);
    void* (*create_compatible_dc)(void*, void* source_dc);
    void* (*create_dib_section)(void*, void* dc, const void* bitmap_info,
                               std::uint32_t usage, void** pixels,
                               void* section, std::uint32_t offset);
    void (*delete_dc)(void*, void* dc);
    // Explicit replacement for shared original scratch at 58D5B8. Must hold
    // the complete decoded BMP and remain stable during all callbacks.
    std::byte* scratch;
};
// 472490 with 450120/450138: valid BMP bytes, readable copied pixel range,
// writable GDI destination, and nonoverlapping memcpy ranges are required.
// width/height may alias one another or resource fields; writes remain ordered.
void load_texture_bitmap(TextureCreationResource&, const char* path,
                         std::int32_t& width, std::int32_t& height,
                         const TextureBitmapHooks&);

struct TextureCreationHooks {
    void* context;
    // Allocation boundaries receive ORIGINAL byte sizes (40 for resource).
    TextureCreationResource* (*allocate_resource)(void*, std::uint32_t);
    void* (*allocate_alpha)(void*, std::uint32_t);
    void (*construct_file)(void*, TextureCreationFile&, const char* path);
    std::uint32_t (*open_file)(void*, TextureCreationFile&, std::uint32_t, std::uint32_t);
    void (*read_file)(void*, TextureCreationFile&, void*, std::uint32_t);
    void (*close_file)(void*, TextureCreationFile&);
    void (*destroy_file)(void*, TextureCreationFile&);
    void (*report_open_failure)(void*, const char*, std::uint32_t, const char*);
    // Must not return; exceptions are permitted only to test this termination.
    void (*terminate_process)(void*, int);
    // 472F99 remains an explicit graphics/device boundary in this module.
    void (*initialize_graphics)(void*, TextureCreationResource&, TextureCreationResource&);
    TextureBitmapHooks bitmap;
    const std::uint8_t* high_quality_5cd5f2;
};

// 47244B. Case-sensitive substring match anywhere in the name.
bool texture_has_quality_variants(const char* name) noexcept;
// 4743DD prefix classification: only backslash separates basename components.
std::uint32_t texture_name_flags(const char* name) noexcept;
// 47218A. Names and all derived paths must fit the original 260-byte arrays;
// callbacks must preserve their lifetimes. No exception unwinding is modeled.
TextureCreationResource* construct_texture_resource(TextureCreationResource&,
    const char* name, std::uint32_t flags, const TextureCreationHooks&);
TextureCreationResource* create_texture_resource(const char* name,
    const TextureCreationHooks&);
// 471720 with 414D00/406500. Index must select a live, terminated cache name.
TextureCreationResource* create_texture_resource_by_index(ResourceCache&,
    std::int32_t index, const TextureCreationHooks&);
} // namespace lemmings
