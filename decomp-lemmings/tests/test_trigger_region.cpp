#include "lemmings/trigger_region.hpp"

#include <limits>
#include <array>
#include <stdexcept>

namespace {
void check(bool value) {
    if (!value) throw std::runtime_error("trigger region mismatch");
}
}

int main() {
    lemmings::TriggerRegionHead region{};
    region.center_x = 10.0;
    region.center_y = 20.0;
    const double lower[3] = {4.0, 14.0, 0.0};
    const double upper[3] = {16.0, 26.0, 1.0};
    const double below_x[3] = {3.9, 20.0, 0.5};
    const double above_y[3] = {10.0, 26.1, 0.5};
    const double below_z[3] = {10.0, 20.0, -0.1};
    check(lemmings::trigger_region_contains_point(region, lower));
    check(lemmings::trigger_region_contains_point(region, upper));
    check(!lemmings::trigger_region_contains_point(region, below_x));
    check(!lemmings::trigger_region_contains_point(region, above_y));
    check(!lemmings::trigger_region_contains_point(region, below_z));
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double nan_z[3] = {10.0, 20.0, nan};
    const double nan_x[3] = {nan, 20.0, 0.5};
    check(!lemmings::trigger_region_contains_point(region, nan_z));
    check(!lemmings::trigger_region_contains_point(region, nan_x));

    lemmings::arm_trigger_timer_once(region, 0.6);
    check(region.timer_active == 1 && region.timer_remaining == 0.6);
    lemmings::arm_trigger_timer_once(region, 1.5);
    check(region.timer_active == 1 && region.timer_remaining == 0.6);
    region.timer_active = 0;
    lemmings::arm_trigger_timer_once(region, 2.0);
    check(region.timer_active == 1 && region.timer_remaining == 2.0);

    lemmings::TriggerRegionHead nearby{};
    lemmings::TriggerRegionHead distant{};
    lemmings::TriggerRegionHead disabled{};
    nearby.enabled = distant.enabled = 1;
    nearby.center_x = 2.0;
    distant.center_x = 3.0;
    disabled.center_x = 0.0;
    const double origin[3] = {0.0, 0.0, 0.0};
    std::array<lemmings::TriggerRegionHead*, 3> actors = {
        &nearby, &distant, &disabled
    };
    lemmings::arm_nearby_trigger_timers(origin, 3.0, actors);
    check(nearby.timer_active == 1 && nearby.timer_remaining == 0.1);
    check(distant.timer_active == 0 && disabled.timer_active == 0);
    lemmings::arm_nearby_trigger_timers(origin, 4.0, actors);
    check(nearby.timer_remaining == 0.1 && distant.timer_active == 1);

    lemmings::GadgetSubboxHead boxes{};
    lemmings::Vec3d low[1] = {{{-1.0, -1.0, -1.0}}};
    lemmings::Vec3d high[1] = {{{1.0, 1.0, 1.0}}};
    boxes.local_lower.count = boxes.local_upper.count = 1;
    boxes.local_lower.data = low;
    boxes.local_upper.data = high;
    lemmings::ActorHead retained_actor{};
    retained_actor.facing = 1;
    retained_actor.region_flag = 1;
    retained_actor.position[2] = 0.5;
    lemmings::ActorListHead retained{};
    lemmings::ActorListNode node{&retained_actor, &retained.end};
    retained.first = &node;
    lemmings::TriggerRegionHead device{};
    int effects = 0;
    auto effect = [](const double (&)[3], void* context) {
        ++*static_cast<int*>(context);
    };
    std::array<lemmings::TriggerRegionHead*, 1> targets = {&nearby};
    check(lemmings::update_trigger_region(device, boxes, retained, targets,
                                           0.0, effect, &effects));
    check(device.timer_active == 1 && device.timer_remaining == 0.6);
    check(lemmings::update_trigger_region(device, boxes, retained, targets,
                                           0.6, effect, &effects));
    check(effects == 0);
    check(!lemmings::update_trigger_region(device, boxes, retained, targets,
                                            0.1, effect, &effects));
    check(effects == 1 && retained_actor.region_flag == 0);
}
