#include "lemmings/actor_finalization.hpp"
#include <bit>
#include <cstring>
#include <stdexcept>

namespace lemmings {
namespace {
std::int32_t integer_low(double value) {
    const long double wide = value;
    if (!(wide >= -9223372036854775808.0L && wide < 9223372036854775808.0L)) return 0;
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::int64_t>(wide)));
}
}
WorldSampleTable world_sample_table(const WorldGeometryPrefix& world) {
    return {world.byte_array, world.fixed_bytes};
}
void position_actor_model(ActorFinalizationState& actor, const WorldCoordinateMesh& mesh) {
    actor.model_position_108 = map_world_coordinates(mesh, actor.geometry.position);
}
SampleTriple world_sample_triple(const WorldSampleTable& table, std::uint32_t x, std::int32_t y) {
    const auto shifted = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(y) << 8);
    const auto row = shifted / 2; // Signed division, after 32-bit shift/wrap.
    const auto index = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(row) + (x & 255u) / 2u);
    if (index < 0 || static_cast<std::size_t>(index) >= table.indices_664.size())
        throw std::out_of_range("world sample index outside backing storage");
    const auto offset = static_cast<std::size_t>(table.indices_664[index]) * 24;
    SampleTriple value{};
    for(std::size_t i=0;i<24;++i)
        value[i/8] |= static_cast<std::uint64_t>(table.triples_678[offset+i]) << ((i%8)*8);
    return value;
}
void finalize_actor_update(ActorFinalizationState& actor, const WorldSampleTable& table,
                           const ActorFinalizationHooks& hooks) {
    if (actor.geometry.direction_44 == 1 && hooks.has_behavior(hooks.context, actor, 0x2B) == 0)
        actor.orientation_2d8 = 0;
    if (actor.geometry.direction_44 == -1 && hooks.has_behavior(hooks.context, actor, 0x2B) == 0)
        actor.orientation_2d8 = 1;
    hooks.model_position_08(hooks.context, actor);
    const auto& clip = actor.animation.resource_1f8->clips[actor.animation.clip_1c8];
    if (std::strcmp(clip.name.c_str(), "drown") != 0) {
        const auto y = integer_low(actor.geometry.position[1]);
        const auto x = static_cast<std::uint32_t>(integer_low(actor.geometry.position[0]));
        actor.model_triples[3] = world_sample_triple(table,x,y);
        actor.model_triples[2] = actor.model_triples[3];
        actor.model_triples[1] = actor.model_triples[2];
        actor.model_triples[0] = actor.model_triples[1];
    }
    hooks.effect_458186(hooks.context, actor);
}
} // namespace lemmings
