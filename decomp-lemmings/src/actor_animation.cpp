#include "lemmings/actor_animation.hpp"

#include <stdexcept>

namespace lemmings {
bool contains_animation_clip(const AnimationResource& resource, const char* name) {
    return static_cast<std::size_t>(find_animation_clip(resource, name)) < resource.clips.size();
}
void select_actor_animation(std::int32_t category,
                            std::span<const std::int32_t> default_ids,
                            std::span<const ActorAnimationOverride> overrides,
                            const char* name, const ActorAnimationHooks& hooks) {
    if (category < 0 || static_cast<std::size_t>(category) >= default_ids.size())
        throw std::out_of_range("actor animation category outside default resources");
    if (!name || !hooks.bind || !hooks.select)
        throw std::invalid_argument("actor animation requires name and hooks");
    auto id = default_ids[category];
    for (const auto& entry : overrides) {
        if (!entry.resource) throw std::invalid_argument("null actor animation override resource");
        if (contains_animation_clip(*entry.resource, name)) id = entry.id;
    }
    hooks.bind(hooks.context, id);
    hooks.select(hooks.context, name);
}
} // namespace lemmings
