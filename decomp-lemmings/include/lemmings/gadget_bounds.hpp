#pragma once

#include <array>
#include <cstddef>

namespace lemmings {

struct CollisionGridHead;

// Known prefix of the gadget object used by the routine at 0x00430BA1.
// The rest of the original object has not been recovered here.
struct GadgetBoundsHead {
    std::array<std::byte, 0x48> unknown_prefix;
    double minimum[3];
    double maximum[3];
};

static_assert(offsetof(GadgetBoundsHead, minimum) == 0x48);
static_assert(offsetof(GadgetBoundsHead, maximum) == 0x60);

// Recovered behavior of the original thiscall routine at 0x00430BA1.
// The input is a pointer to three consecutive doubles in X, Y, Z order.
bool gadget_contains_point(const GadgetBoundsHead& gadget,
                           const double (&point)[3]);

// Recovered from 0x00430FE1. X coordinates use the cylinder wrapper, Y
// coordinates use the world-height clamp, and Z defaults to [0, 1].
void set_gadget_bounds_xy(GadgetBoundsHead& gadget,
                          const CollisionGridHead& grid,
                          double x0, double y0, double x1, double y1);

// Recovered from 0x004310C0. The two Z coordinates are supplied by the caller.
void set_gadget_bounds_xyz(GadgetBoundsHead& gadget,
                           const CollisionGridHead& grid,
                           double x0, double y0, double z0,
                           double x1, double y1, double z1);

// Recovered from 0x004338C2. The original mode-zero branch first writes
// [0, 0.5], then immediately overwrites it with [0, 1].
void set_gadget_bounds_for_mode(GadgetBoundsHead& gadget,
                                const CollisionGridHead& grid,
                                double x0, double y0, double x1, double y1,
                                int mode);

} // namespace lemmings
