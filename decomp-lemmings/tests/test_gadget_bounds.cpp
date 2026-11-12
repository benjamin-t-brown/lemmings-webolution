#include "lemmings/gadget_bounds.hpp"
#include "lemmings/collision_grid.hpp"

#include <cmath>
#include <initializer_list>
#include <limits>
#include <stdexcept>

namespace {
void check(bool condition) {
    if (!condition) {
        throw std::runtime_error("gadget containment mismatch");
    }
}
}

int main() {
    lemmings::GadgetBoundsHead gadget{};
    gadget.minimum[0] = -1.0;
    gadget.minimum[1] = 2.0;
    gadget.minimum[2] = 0.0;
    gadget.maximum[0] = 1.0;
    gadget.maximum[1] = 5.0;
    gadget.maximum[2] = 1.0;

    const double inside[3] = {0.0, 3.0, 0.5};
    check(lemmings::gadget_contains_point(gadget, inside));

    // Every face is inclusive, and stepping outside any face is rejected.
    for (int axis = 0; axis != 3; ++axis) {
        for (double edge : {gadget.minimum[axis], gadget.maximum[axis]}) {
            double point[3] = {inside[0], inside[1], inside[2]};
            point[axis] = edge;
            check(lemmings::gadget_contains_point(gadget, point));
        }
        double below[3] = {inside[0], inside[1], inside[2]};
        below[axis] = std::nextafter(gadget.minimum[axis],
                                     -std::numeric_limits<double>::infinity());
        check(!lemmings::gadget_contains_point(gadget, below));

        double above[3] = {inside[0], inside[1], inside[2]};
        above[axis] = std::nextafter(gadget.maximum[axis],
                                     std::numeric_limits<double>::infinity());
        check(!lemmings::gadget_contains_point(gadget, above));
    }

    // These cases follow the original FNSTSW AX / TEST AH instructions.
    gadget.minimum[0] = std::numeric_limits<double>::quiet_NaN();
    check(!lemmings::gadget_contains_point(gadget, inside));
    gadget.minimum[0] = -1.0;
    gadget.maximum[0] = std::numeric_limits<double>::quiet_NaN();
    check(lemmings::gadget_contains_point(gadget, inside));

    const double nan_point[3] = {
        std::numeric_limits<double>::quiet_NaN(), 3.0, 0.5
    };
    check(!lemmings::gadget_contains_point(gadget, nan_point));

    gadget.maximum[0] = 1.0;
    gadget.minimum[0] = 2.0;
    check(!lemmings::gadget_contains_point(gadget, inside));

    lemmings::CollisionGridHead grid{};
    grid.height = 100;
    lemmings::set_gadget_bounds_xy(gadget, grid, -1.0, 30.0, 257.0, 10.0);
    check(gadget.minimum[0] == 255.0 && gadget.maximum[0] == 1.0);
    check(gadget.minimum[1] == 10.0 && gadget.maximum[1] == 30.0);
    check(gadget.minimum[2] == 0.0 && gadget.maximum[2] == 1.0);

    lemmings::set_gadget_bounds_xyz(gadget, grid,
                                    4.0, -5.0, -2.0,
                                    7.0, 120.0, 3.0);
    check(gadget.minimum[0] == 4.0 && gadget.maximum[0] == 7.0);
    check(gadget.minimum[1] == 0.0 && gadget.maximum[1] == 99.0);
    check(gadget.minimum[2] == -2.0 && gadget.maximum[2] == 3.0);

    lemmings::set_gadget_bounds_for_mode(gadget, grid,
                                         2.0, 3.0, 4.0, 5.0, 2);
    check(gadget.minimum[2] == 0.5 && gadget.maximum[2] == 1.0);
    lemmings::set_gadget_bounds_for_mode(gadget, grid,
                                         2.0, 3.0, 4.0, 5.0, 0);
    check(gadget.minimum[2] == 0.0 && gadget.maximum[2] == 1.0);
    lemmings::set_gadget_bounds_for_mode(gadget, grid,
                                         2.0, 3.0, 4.0, 5.0, 17);
    check(gadget.minimum[2] == 0.0 && gadget.maximum[2] == 1.0);
}
