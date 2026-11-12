#include "lemmings/trigger_region.hpp"

namespace lemmings {

bool trigger_region_contains_point(const TriggerRegionHead& region,
                                   const double (&point)[3]) {
    constexpr long double half_width = 12.0L / 2.0L;
    const long double x = point[0];
    const long double y = point[1];
    if (static_cast<long double>(region.center_x) - half_width > x) {
        return false;
    }
    if (static_cast<long double>(region.center_y) - half_width > y) {
        return false;
    }
    if (!(point[2] >= 0.0)) {
        return false;
    }
    if (!(static_cast<long double>(region.center_x) + half_width >= x)) {
        return false;
    }
    if (!(static_cast<long double>(region.center_y) + half_width >= y)) {
        return false;
    }
    return !(point[2] > 1.0);
}

void arm_trigger_timer_once(TriggerRegionHead& region, double duration) {
    if (region.timer_active == 0) {
        region.timer_active = 1;
        region.timer_remaining = duration;
    }
}

void arm_nearby_trigger_timers(const double (&origin)[3], double radius,
                               std::span<TriggerRegionHead* const> actors) {
    const double radius_squared = radius * radius;
    for (auto* actor : actors) {
        if (actor->enabled == 0 || actor->timer_active != 0) {
            continue;
        }
        const double difference[3] = {
            origin[0] - actor->center_x,
            origin[1] - actor->center_y,
            origin[2] - actor->center_z,
        };
        const long double squared_extended =
            static_cast<long double>(difference[0]) * difference[0]
            + static_cast<long double>(difference[1]) * difference[1]
            + static_cast<long double>(difference[2]) * difference[2];
        const double squared = static_cast<double>(squared_extended);
        // The original x87 C0 test accepts less-than and unordered.
        if (!(squared >= radius_squared)) {
            arm_trigger_timer_once(*actor, 0.1);
        }
    }
}

bool update_trigger_region(TriggerRegionHead& region,
                           const GadgetSubboxHead& boxes,
                           const ActorListHead& retained,
                           std::span<TriggerRegionHead* const> all_actors,
                           double elapsed,
                           TerrainEffect terrain_effect,
                           void* effect_context) {
    if (region.timer_active == 0) {
        if (first_actor_in_subbox(boxes, retained) != nullptr) {
            arm_trigger_timer_once(region, 0.6);
        }
        return true;
    }
    region.timer_remaining -= elapsed;
    if (region.timer_remaining >= 0.0) {
        return true;
    }
    const double origin[3] = {
        region.center_x, region.center_y, region.center_z
    };
    if (terrain_effect != nullptr) {
        terrain_effect(origin, effect_context);
    }
    arm_nearby_trigger_timers(origin, 12.0, all_actors);
    auto* node = retained.first;
    while (node != &retained.end) {
        auto* next = node->next;
        auto* actor = node->actor;
        if (trigger_region_contains_point(region, actor->position)) {
            actor->region_flag = 0;
        }
        node = next;
    }
    return false;
}

} // namespace lemmings
