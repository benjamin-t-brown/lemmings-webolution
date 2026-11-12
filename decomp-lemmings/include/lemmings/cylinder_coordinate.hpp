#pragma once

namespace lemmings {

struct CollisionGridHead;

// Recovered behavior at 0x0040DEC0 for finite cylinder X coordinates.
// The original returns the result on the x87 register stack. Positive
// infinity would keep its subtraction loop running forever and is excluded.
long double wrap_cylinder_x(double x);

// Recovered from 0x0040DF50. Clamps a double Y against the world height;
// the return value remains on the original x87 register stack.
long double clamp_cylinder_y(const CollisionGridHead& grid, double y);

} // namespace lemmings
