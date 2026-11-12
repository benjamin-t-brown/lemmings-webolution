#pragma once

#include "lemmings/actor_geometry.hpp"
#include "lemmings/world_coordinates.hpp"

namespace lemmings {

// Logical views of the known fields used by 0x458186. "Shadow" describes the
// caller's effect role; mode_120 is retained without assigning a render mode.
struct ShadowActor {
    ActorGeometryState geometry;
    void* gadget_54 = nullptr;
    std::int32_t primary_type_14 = 0;
};
struct ActorShadowQuadMetadata {
    std::array<double, 3> color_00{};
    double alpha_18 = 0;
    // Corner +0x20, stride 0x18. The following four bytes remain unknown.
    std::array<std::int32_t, 4> vertex_indices_20{};
};
struct ActorShadow {
    std::array<std::array<double, 3>, 12> vertices_000{};
    std::int32_t mode_120 = 0;
    // Original groups start at +0x128, stride 0x80; UV pairs at group+0x28,
    // corner stride 0x18. Metadata is projected separately below; each
    // corner's four bytes after its index remain unknown and untouched.
    std::array<std::array<std::array<double, 2>, 4>, 5> uv_128{};
    std::array<ActorShadowQuadMetadata, 5> quad_metadata_128{};
    ShadowActor* actor_3a8 = nullptr;
    std::uint8_t flag_3ac = 0;
    std::uint8_t visible_3ad = 0;
    std::array<double, 3> position_3b0{};
    std::int32_t intensity_3c8 = 0;
};
struct ActorShadowGlobals {
    std::int32_t resource_id_4c05c0 = 0;
    double width_4c05c8 = 0; // Zero-initialized global; contact overwrites with 2.5.
};
struct ActorShadowCallbacks {
    virtual ~ActorShadowCallbacks() = default;
    virtual void* create_point_list_4229f0() = 0;
    virtual void resize_point_list_421260(void* list, std::int32_t count) = 0;
    virtual void destroy_point_list_422a30(void* list) = 0;
    virtual std::uint8_t has_state_42ecc9(ShadowActor& actor, std::int32_t type) = 0;
    virtual void build_segments_459557(ActorShadow& shadow, void* list, double left, double right) = 0;
    virtual std::int32_t point_count_459990(void* list) = 0;
    virtual const std::array<double, 3>& point_at_421240(void* list, std::int32_t index) = 0;
    virtual void* lookup_resource_414a10(std::int32_t id) = 0;
    virtual void activate_resource_47261f(void* resource) = 0;
};

// 0x458186. Concrete terrain queries, visibility, intensity, vertex topology,
// world-coordinate mapping and UV writes; list/resource boundaries are explicit.
// Requires valid actor/grid/mesh, finite X/width/list-X, terminating scanning
// inputs, and a stable list length of 1..4 with valid mapped Y. Counts 3 and 4
// produce geometry;
// other supported counts preserve prior vertices/mode/UVs after resource use.
// No exception-unwind behavior is claimed for throwing callbacks.
void update_actor_shadow(ActorShadow& shadow, ActorShadowGlobals& globals,
    CollisionGridHead& grid, const WorldCoordinateMesh& mesh, ActorShadowCallbacks& callbacks);

} // namespace lemmings
