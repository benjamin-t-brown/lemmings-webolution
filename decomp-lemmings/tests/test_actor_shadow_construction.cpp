#include "lemmings/actor_shadow_construction.hpp"
#include <bit>
#include <cassert>
#include <cstring>

using namespace lemmings;
int main() {
    ActorShadow shadow;
    ShadowActor actor;
    for (unsigned i = 0; i < shadow.vertices_000.size(); ++i)
        shadow.vertices_000[i] = {double(i), -double(i),
            std::bit_cast<double>(std::uint64_t{0x7ff8000000000042})};
    const auto vertices = shadow.vertices_000;
    shadow.visible_3ad = 0xa5;
    shadow.intensity_3c8 = -123;
    shadow.flag_3ac = 0xff;
    shadow.mode_120 = 99;
    shadow.position_3b0 = {-0.0, 100, -100};
    assert(&initialize_actor_shadow(shadow, &actor) == &shadow);
    assert(shadow.actor_3a8 == &actor && shadow.mode_120 == 0);
    assert(shadow.flag_3ac == 0 && shadow.visible_3ad == 0xa5);
    assert(shadow.intensity_3c8 == -123);
    assert(std::memcmp(vertices.data(), shadow.vertices_000.data(), sizeof(vertices)) == 0);
    for (double value : shadow.position_3b0)
        assert(std::bit_cast<std::uint64_t>(value) == 0);
    for (std::int32_t group = 0; group < 5; ++group) {
        const auto& metadata = shadow.quad_metadata_128[group];
        assert((metadata.vertex_indices_20 == std::array<std::int32_t, 4>{
            group * 2, group * 2 + 1, group * 2 + 3, group * 2 + 2}));
        assert((metadata.color_00 == std::array<double, 3>{255, 255, 255}));
        assert(metadata.alpha_18 == 255);
        assert((shadow.uv_128[group] == std::array<std::array<double, 2>, 4>{
            {{0, 0}, {0, 1}, {1, 1}, {1, 0}}}));
    }
    initialize_actor_shadow(shadow, nullptr);
    assert(shadow.actor_3a8 == nullptr && shadow.visible_3ad == 0xa5);
}
