#include "lemmings/world_coordinates.hpp"
#include <bit>
#include <cmath>
#include <stdexcept>

namespace lemmings {
namespace {
using Point = std::array<double,3>;
double rounded(long double value) {
    volatile double result = static_cast<double>(value);
    return result;
}
std::int32_t low_integer(double value) {
    constexpr long double limit = 9223372036854775808.0L;
    if (!(value >= -limit && value < limit)) return 0;
    return std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
}
Point point_at(const WorldCoordinateMesh& mesh, std::int32_t row, unsigned index) {
    const auto& bits = mesh.rows[static_cast<std::size_t>(row)][index];
    return {std::bit_cast<double>(bits[0]),std::bit_cast<double>(bits[1]),
            std::bit_cast<double>(bits[2])};
}
Point interpolate(const Point& first, const Point& second, double fraction) {
    Point result{};
    for (unsigned i=0;i<3;++i) {
        // Each original vector helper constructs a stored double triple.
        const double delta = rounded(static_cast<long double>(second[i])-first[i]);
        const double scaled = rounded(static_cast<long double>(fraction)*delta);
        result[i] = rounded(static_cast<long double>(first[i])+scaled);
    }
    return result;
}
}
WorldCoordinateMesh world_coordinate_mesh(const WorldGeometryPrefix& world) {
    return {std::bit_cast<std::int32_t>(world.leading_value),world.first_blocks};
}
std::array<double,3> map_world_coordinates(
    const WorldCoordinateMesh& mesh, const std::array<double,3>& point) {
    const auto row_count = mesh.height / 16;
    if (row_count < 1 || mesh.rows.size() < static_cast<std::size_t>(row_count))
        throw std::invalid_argument("invalid world coordinate mesh");
    const auto sector = low_integer(point[0]) / 8; // signed division truncates toward zero
    const unsigned left = static_cast<std::uint32_t>(sector) & 31u;
    const unsigned right = (left + 1u) & 31u;
    auto lower = low_integer(point[1]) / 16;
    // FCOMP/TEST AH,0x41 also selects the final row for unordered Y.
    if (!(static_cast<double>(mesh.height) > point[1])) lower = row_count-1;
    if (lower < 0 || lower >= row_count)
        throw std::out_of_range("world coordinate row is outside the mesh");
    auto upper = lower + 1;
    if (upper > row_count-1) upper = row_count-1;
    const auto a = interpolate(point_at(mesh,lower,left),point_at(mesh,lower,left+32),point[2]);
    const auto b = interpolate(point_at(mesh,lower,right),point_at(mesh,lower,right+32),point[2]);
    const auto c = interpolate(point_at(mesh,upper,left),point_at(mesh,upper,left+32),point[2]);
    const auto d = interpolate(point_at(mesh,upper,right),point_at(mesh,upper,right+32),point[2]);
    const auto base_x = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(sector)<<3);
    const double fx = rounded((static_cast<long double>(point[0])-base_x)/8.0L);
    const auto low = interpolate(a,b,fx);
    const auto high = interpolate(c,d,fx);
    const auto base_y = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(lower)<<4);
    const double fy = rounded((static_cast<long double>(point[1])-base_y)/16.0L);
    return interpolate(low,high,fy);
}
double world_depth_at_y(const WorldCoordinateMesh& mesh,
                        const std::array<double,3>& point, double new_y) {
    auto sample = point;
    sample[2] = 0.0;
    const auto inner = map_world_coordinates(mesh,sample);
    sample[2] = 1.0;
    const auto outer = map_world_coordinates(mesh,sample);
    sample[1] = new_y;
    sample[2] = point[2];
    const auto moved = map_world_coordinates(mesh,sample);
    const double dx = std::fabs(rounded(static_cast<long double>(outer[0])-inner[0]));
    const double dz = std::fabs(rounded(static_cast<long double>(outer[2])-inner[2]));
    const unsigned component = dx > dz ? 0 : 2;
    return rounded((static_cast<long double>(moved[component])-inner[component]) /
                   (static_cast<long double>(outer[component])-inner[component]));
}
} // namespace lemmings
