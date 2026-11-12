#include "lemmings/actor_shadow_segments.hpp"
#include "lemmings/cylinder_coordinate.hpp"

#include <bit>

namespace lemmings {
namespace {
std::int32_t low_integer(long double value) {
    constexpr long double limit = 9223372036854775808.0L;
    if (!(value >= -limit && value < limit)) return 0;
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
}
std::int32_t add(std::int32_t value, std::int32_t delta) {
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(value) + static_cast<std::uint32_t>(delta));
}
}

void build_actor_shadow_segments(const std::array<double, 3>& position_3b0,
    double left, double right, CollisionGridHead& grid, ActorShadowSegmentList& list) {
    const auto position = position_3b0;
    const auto row = low_integer(clamp_cylinder_y(grid, low_integer(position_3b0[1])));
    const auto first = add(low_integer(position[0]), -1);
    const auto last = add(low_integer(position[0]), 1);
    double selected_x = left;
    double selected_y = row;
    std::int32_t count = 0;
    const auto cell = [&](std::int32_t column, std::int32_t y) {
        return static_cast<std::uint32_t>(*collision_cell(grid, static_cast<std::uint32_t>(column), y));
    };
    const auto clamped_row = [&](std::int32_t delta) {
        return low_integer(clamp_cylinder_y(grid, add(row, delta)));
    };
    const auto append = [&](double x, double y) {
        list.resize_421260(add(count, 1));
        const std::array<double, 3> point{x, y, 0};
        const auto index = count;
        count = add(count, 1);
        list.point_at_421240(index) = point;
    };
    for (auto column = first; column <= last; column = add(column, 1)) {
        const auto wrapped = low_integer(wrap_cylinder_x(column));
        if (cell(wrapped, row) & 1) {
            if (!(cell(wrapped, clamped_row(1)) & 1)) {
                selected_x = !(static_cast<long double>(wrapped) >= position[0]) ? left : right;
                selected_y = add(row, 1);
            } else if (wrapped == last) {
                selected_x = right;
                selected_y = row;
            }
            // Otherwise both values deliberately carry across iterations.
        } else if (cell(wrapped, clamped_row(-1)) == 0) {
            if (cell(wrapped, clamped_row(-2)) == 0) {
                selected_x = add(wrapped, 1);
                selected_y = row;
            } else {
                selected_x = !(static_cast<long double>(wrapped) >= position[0]) ? left : right;
                selected_y = add(row, -1);
            }
        } else {
            selected_y = row;
            if (wrapped == first) selected_x = left;
            else if (wrapped == last) selected_x = right;
            else selected_x = wrapped;
        }
        append(selected_x, selected_y);
        // TEST AH,1 also takes this branch for unordered right endpoints.
        if (wrapped == low_integer(position[0]) && !(static_cast<long double>(last) >= right))
            append(static_cast<double>(static_cast<long double>(selected_x) + 1.0L), selected_y);
    }
}

} // namespace lemmings
