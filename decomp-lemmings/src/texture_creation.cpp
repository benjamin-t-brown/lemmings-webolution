#include "lemmings/texture_creation.hpp"
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace lemmings {
namespace {
template<class T> T word(const std::byte* p) {
    T value;
    std::memcpy(&value, p, sizeof value);
    return value;
}
std::uint32_t area(const TextureCreationResource& resource) {
    return std::uint32_t(resource.width_14) * std::uint32_t(resource.height_18);
}
}
bool texture_has_quality_variants(const char* name) noexcept {
    return std::strstr(name, "INN_") || std::strstr(name, "OUT_");
}
std::uint32_t texture_name_flags(const char* name) noexcept {
    const auto* slash = std::strrchr(name, '\\');
    switch (*(slash ? slash + 1 : name)) {
    case '_': return 1;
    case '$': return 2;
    case '~': return 4;
    case '^': return 8;
    case '+': return 16;
    default: return 0;
    }
}
void load_texture_bitmap(TextureCreationResource& resource, const char* path,
    std::int32_t& width, std::int32_t& height, const TextureBitmapHooks& hooks) {
    TextureBitmapBuffer buffer;
    buffer.data_04 = nullptr; // 450120 leaves the size word untouched.
    hooks.load(hooks.context, buffer, path);
    std::memcpy(hooks.scratch, buffer.data_04, buffer.size_00);
    const auto* pixels = hooks.scratch + word<std::uint32_t>(hooks.scratch + 10);
    width = word<std::int32_t>(hooks.scratch + 18);
    height = word<std::int32_t>(hooks.scratch + 22);
    const auto aligned_bits = (std::uint32_t(width) *
        word<std::uint16_t>(hooks.scratch + 28) + 31u) & ~31u;
    const auto stride = std::bit_cast<std::int32_t>(aligned_bits) >> 3;
    const auto bytes = std::uint32_t(stride) * std::uint32_t(height);
    auto* dc = hooks.create_compatible_dc(hooks.context, nullptr);
    void* output;
    resource.bitmap_04 = hooks.create_dib_section(hooks.context, dc,
        hooks.scratch + 14, 0, &output, nullptr, 0);
    hooks.delete_dc(hooks.context, dc);
    std::memcpy(output, pixels, bytes);
    hooks.free_data(hooks.context, buffer.data_04);
}
TextureCreationResource* construct_texture_resource(TextureCreationResource& resource,
    const char* name, std::uint32_t flags, const TextureCreationHooks& hooks) {
    resource.triple_data_20 = nullptr;
    resource.unknown_08 = nullptr;
    resource.device_texture_00 = nullptr;
    resource.alpha_0c = nullptr;
    resource.unknown_10 = 0;
    resource.flags_24 = flags;
    char selected_name[260];
    char bitmap_path[260];
    std::strcpy(selected_name, name);
    if (*hooks.high_quality_5cd5f2 == 0 || !texture_has_quality_variants(selected_name)) {
        if (*hooks.high_quality_5cd5f2 == 0) std::strcat(selected_name, "Lo");
    } else std::strcat(selected_name, "Hi");
    std::sprintf(bitmap_path, "Textures\\%s.bmp", selected_name);
    TextureCreationFile bitmap_file;
    hooks.construct_file(hooks.context, bitmap_file, bitmap_path);
    if ((hooks.open_file(hooks.context, bitmap_file, 1, 0) & 255u) == 0) {
        std::strcpy(selected_name, name);
        std::sprintf(bitmap_path, "Textures\\%s.bmp", selected_name);
    } else hooks.close_file(hooks.context, bitmap_file);
    load_texture_bitmap(resource, bitmap_path, resource.width_14, resource.height_18, hooks.bitmap);
    if (flags & 0x18u) {
        resource.alpha_0c = hooks.allocate_alpha(hooks.context, area(resource));
        char alpha_path[260];
        std::sprintf(alpha_path, "Textures\\%s.alf", selected_name);
        TextureCreationFile alpha_file;
        hooks.construct_file(hooks.context, alpha_file, alpha_path);
        if ((hooks.open_file(hooks.context, alpha_file, 1, 0) & 255u) == 0) {
            hooks.report_open_failure(hooks.context,
                "D:\\lrpc\\Source\\3DEngine\\TexturePC.cpp", 0x5a, alpha_path);
            hooks.terminate_process(hooks.context, 0);
            std::abort();
        }
        const auto bytes = area(resource);
        auto* alpha = resource.alpha_0c;
        hooks.read_file(hooks.context, alpha_file, alpha, bytes);
        hooks.close_file(hooks.context, alpha_file);
        hooks.destroy_file(hooks.context, alpha_file);
    }
    hooks.initialize_graphics(hooks.context, resource, resource);
    hooks.destroy_file(hooks.context, bitmap_file);
    return &resource;
}
TextureCreationResource* create_texture_resource(const char* name,
    const TextureCreationHooks& hooks) {
    const auto flags = texture_name_flags(name);
    auto* resource = hooks.allocate_resource(hooks.context, 40);
    return resource ? construct_texture_resource(*resource, name, flags, hooks) : nullptr;
}
TextureCreationResource* create_texture_resource_by_index(ResourceCache& cache,
    std::int32_t index, const TextureCreationHooks& hooks) {
    return create_texture_resource(cache.entries_30[index].name_00.data(), hooks);
}
} // namespace lemmings
