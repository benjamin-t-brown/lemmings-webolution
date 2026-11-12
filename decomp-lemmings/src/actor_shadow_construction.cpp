#include "lemmings/actor_shadow_construction.hpp"

namespace lemmings {

ActorShadow& initialize_actor_shadow(ActorShadow& shadow, ShadowActor* actor) {
    // The original array constructors at 401B80 and 4821D0 perform no writes.
    shadow.position_3b0 = {0, 0, 0};
    shadow.flag_3ac = 0;
    for (std::int32_t group = 0; group < 5; ++group) {
        auto& metadata = shadow.quad_metadata_128[group];
        metadata.vertex_indices_20 = {group * 2, group * 2 + 1,
                                     group * 2 + 3, group * 2 + 2};
        shadow.uv_128[group] = {{{0, 0}, {0, 1}, {1, 1}, {1, 0}}};
        metadata.color_00 = {255, 255, 255};
        metadata.alpha_18 = 255;
    }
    shadow.mode_120 = 0;
    shadow.actor_3a8 = actor;
    return shadow;
}

} // namespace lemmings
