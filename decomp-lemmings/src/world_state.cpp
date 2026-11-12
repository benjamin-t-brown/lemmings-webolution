#include "lemmings/world_state.hpp"
#include <bit>
#include <limits>
#include <stdexcept>

namespace lemmings {
CollisionGridHead CollisionGridStorage::view() {
    CollisionGridHead result{};
    result.height=height;
    result.cells.data=cells.data();
    result.cells.count=static_cast<std::int32_t>(cells.size());
    result.cells.capacity=result.cells.count;
    return result;
}
CollisionGridStorage collision_grid_from_level(const WorldGeometryPrefix& world) {
    CollisionGridStorage result;
    result.height=std::bit_cast<std::int32_t>(world.following_value);
    if(result.height<=0 || world.second_integer_array.size()>
       static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()) ||
       world.second_integer_array.size()<static_cast<std::size_t>(result.height)*256)
        throw std::invalid_argument("compiled collision grid has invalid dimensions");
    result.cells.reserve(world.second_integer_array.size());
    for(const auto value:world.second_integer_array)
        result.cells.push_back(std::bit_cast<std::int32_t>(value));
    return result;
}
} // namespace lemmings
