#include "lemmings/gadget_bounds.hpp"
#include "lemmings/cylinder_coordinate.hpp"

#include <utility>

namespace lemmings {

bool gadget_contains_point(const GadgetBoundsHead& gadget,
                           const double (&point)[3]) {
    // The original compares each minimum first and returns immediately on
    // x87 less-than or unordered. All three maximum tests then accept x87
    // less-than or equal; an unordered maximum also passes that bit test.
    for (int axis = 0; axis != 3; ++axis) {
        if (!(point[axis] >= gadget.minimum[axis])) {
            return false;
        }
    }
    for (int axis = 0; axis != 3; ++axis) {
        if (point[axis] > gadget.maximum[axis]) {
            return false;
        }
    }
    return true;
}

void set_gadget_bounds_xyz(GadgetBoundsHead& gadget,
                           const CollisionGridHead& grid,
                           double x0, double y0, double z0,
                           double x1, double y1, double z1) {
    gadget.minimum[0] = static_cast<double>(wrap_cylinder_x(x0));
    gadget.minimum[1] = static_cast<double>(clamp_cylinder_y(grid, y0));
    gadget.minimum[2] = z0;
    gadget.maximum[0] = static_cast<double>(wrap_cylinder_x(x1));
    gadget.maximum[1] = static_cast<double>(clamp_cylinder_y(grid, y1));
    gadget.maximum[2] = z1;
    // The original x87 C0 test also swaps on an unordered comparison.
    if (!(gadget.maximum[1] >= gadget.minimum[1])) {
        std::swap(gadget.minimum[1], gadget.maximum[1]);
    }
}

void set_gadget_bounds_xy(GadgetBoundsHead& gadget,
                          const CollisionGridHead& grid,
                          double x0, double y0, double x1, double y1) {
    set_gadget_bounds_xyz(gadget, grid, x0, y0, 0.0, x1, y1, 1.0);
}

void set_gadget_bounds_for_mode(GadgetBoundsHead& gadget,
                                const CollisionGridHead& grid,
                                double x0, double y0, double x1, double y1,
                                int mode) {
    if (mode == 2) {
        set_gadget_bounds_xyz(gadget, grid,
                              x0, y0, 0.5, x1, y1, 1.0);
    } else {
        set_gadget_bounds_xy(gadget, grid, x0, y0, x1, y1);
    }
}

} // namespace lemmings
