#include "lemmings/sprite_factory.hpp"

namespace lemmings {

SpriteRuntimeArray* initialize_sprite_array(SpriteRuntimeArray& array) noexcept {
    array.count_00 = 0;
    array.capacity_04 = 0;
    array.growth_08 = 1;
    array.data_0c = nullptr;
    return &array;
}

SpriteFactoryResource* initialize_sprite_resource(SpriteFactoryResource& resource) noexcept {
    initialize_sprite_array(resource.clips_00);
    initialize_sprite_array(resource.triple_arrays_10);
    return &resource;
}

SpriteFactoryResource* create_sprite_resource(const char* name,
                                             const SpriteFactoryHooks& hooks) {
    auto* resource = hooks.allocate(hooks.context, 32);
    if (resource != nullptr) initialize_sprite_resource(*resource);
    char path[260];
    hooks.format(hooks.context, path, "Sprites\\%s.spr", name);
    SpriteFactoryFile file;
    hooks.construct_file(hooks.context, file, path);
    if ((hooks.open_file(hooks.context, file, 1, 0) & 0xffu) == 0) {
        hooks.report_open_failure(hooks.context, "D:\\lrpc\\Source\\Sprites\\SpriteSheets.cpp", 31, path);
        hooks.terminate_process(hooks.context, 0);
    }
    hooks.read_resource(hooks.context, resource, file);
    hooks.close_file(hooks.context, file);
    hooks.destroy_file(hooks.context, file);
    return resource;
}

void destroy_sprite_resource_contents(SpriteFactoryResource& resource,
                                     const SpriteFactoryDestroyHooks& hooks) {
    hooks.clear_triple_arrays(hooks.context, resource.triple_arrays_10);
    hooks.clear_clips(hooks.context, resource.clips_00);
}

SpriteFactoryResource* delete_sprite_resource(SpriteFactoryResource* resource,
                                             std::uint32_t flags,
                                             const SpriteFactoryDestroyHooks& hooks) {
    destroy_sprite_resource_contents(*resource, hooks);
    if ((flags & 1u) != 0) hooks.free_resource(hooks.context, resource);
    return resource;
}

void destroy_sprite_resource(SpriteFactoryResource* resource,
                             const SpriteFactoryDestroyHooks& hooks) {
    if (resource != nullptr) delete_sprite_resource(resource, 1, hooks);
}

} // namespace lemmings
