// Runs the original routines and their complete helper chain without adapters.
#include "lemmings/actor_terrain_probe.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>

namespace {
template<class T> void put(void* p, std::size_t offset, const T& value) {
    std::memcpy(static_cast<char*>(p) + offset, &value, sizeof(value));
}
std::array<std::uint8_t, 10> extended_bits(long double value) {
    std::array<std::uint8_t, 10> result{};
    std::memcpy(result.data(), &value, 10);
    return result;
}
bool same_extended(long double a, long double b) {
    return (std::isnan(a) && std::isnan(b)) || extended_bits(a) == extended_bits(b);
}
std::int32_t projected_x(double value) {
    constexpr long double limit = 9223372036854775808.0L;
    if (!(value >= -limit && value < limit)) return 0;
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
}
}

int main() {
    using namespace lemmings;
    _setmode(_fileno(stdin), _O_BINARY);
    auto* image = VirtualAlloc(reinterpret_cast<void*>(0x400000), 0xD0000,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(0x400000)) return 3;
    std::uint32_t count = 0;
    std::cin.read(reinterpret_cast<char*>(&count), 4);
    if (!std::cin || count > 64) return 2;
    for (std::uint32_t i = 0; i < count; ++i) {
        std::uint32_t address = 0, length = 0;
        std::cin.read(reinterpret_cast<char*>(&address), 4);
        std::cin.read(reinterpret_cast<char*>(&length), 4);
        if (address < 0x400000 || length > 0xD0000 || address - 0x400000 > 0xD0000 - length) return 2;
        std::cin.read(reinterpret_cast<char*>(address), length);
        if (!std::cin) return 2;
    }
    DWORD old = 0;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &old)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    using Range = std::uint8_t(__attribute__((thiscall))*)(void*);
    using Distance = long double(__attribute__((thiscall))*)(void*);
    const auto native_range = reinterpret_cast<Range>(0x42F6EC);
    const auto native_distance = reinterpret_cast<Distance>(0x42F788);
    std::array<std::uint8_t, 0x3A8> raw_actor{};
    std::array<std::int32_t, 32 * 256> cells{};
    CollisionGridHead grid{};
    grid.height = 16;
    grid.cells.data = cells.data();
    const auto prepare = [&](const ActorGeometryState& actor) {
        raw_actor.fill(0xA5);
        put(raw_actor.data(), 8, actor.position);
        put(raw_actor.data(), 0x40, actor.direction_40);
        put(raw_actor.data(), 0x44, actor.direction_44);
        put(raw_actor.data(), 0x350, actor.pair_350);
        put(raw_actor.data(), 0x360, actor.pair_360);
        put(reinterpret_cast<void*>(0x4C6A10), 0, grid);
    };
    unsigned ranges = 0, distances = 0;
    const auto compare_range = [&](const ActorGeometryState& actor) {
        prepare(actor);
        const auto before = raw_actor;
        const auto result = native_range(raw_actor.data());
        const bool expected = actor_sample_y_in_world(actor, grid);
        ++ranges;
        if (result != static_cast<unsigned>(expected) || raw_actor != before) {
            std::cerr << "vertical range mismatch " << ranges << " y=" << actor.position[1]
                      << " sample offset=" << actor.pair_360[1] << " height=" << grid.height << '\n';
            return false;
        }
        return true;
    };
    const auto compare_distance = [&](const ActorGeometryState& actor) {
        prepare(actor);
        const auto before = raw_actor;
        const auto cells_before = cells;
        const auto result = native_distance(raw_actor.data());
        const auto expected = actor_distance_to_terrain(actor, grid);
        ++distances;
        if (!same_extended(result, expected) || raw_actor != before || cells != cells_before) {
            std::cerr << "terrain distance mismatch " << distances << " x=" << actor.position[0]
                      << " y=" << actor.position[1] << " direction=" << actor.direction_44
                      << " result=" << result << '/' << expected << '\n';
            return false;
        }
        return true;
    };
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const std::array<double, 16> edges{
        -inf, -4294967296.5, -1.0, -0.0, 0, 1e-300, 0.5, 15.5,
        16, std::nextafter(16.0, inf), 2147483647.5, 4294967296.5,
        -9223372036854775808.0, 9223372036854775808.0, inf, nan};
    for (auto height : {-1, 0, 1, 16, 32, std::numeric_limits<int>::max()}) {
        grid.height = height;
        for (double y : edges) for (double offset : edges) {
            ActorGeometryState actor;
            actor.position = {nan, y, inf};
            actor.pair_360 = {nan, offset};
            if (!compare_range(actor)) return 1;
        }
    }
    grid.height = 16;
    // Complete mask cases at the first row, then either a solid cell two rows
    // away or the boundary. Zero direction is tested only for immediate stops.
    for (int mask : {0, 1, 0x80, 0x200, 0x280, 0x201, 0x81, 0x1000, 0x4000, -1})
        for (int direction : {-7, -2, -1, 1, 2, 7, std::numeric_limits<int>::min(), std::numeric_limits<int>::max()})
            for (double x : edges) {
                cells.fill(0);
                ActorGeometryState actor;
                actor.position = {x, 7.75, nan};
                actor.direction_44 = direction;
                actor.pair_350 = {1000, -1000}; actor.pair_360 = {-1000, 1000};
                const auto column = static_cast<unsigned>(projected_x(x)) & 255;
                cells[7 * 256 + column] = mask;
                cells[5 * 256 + column] = 1;
                cells[9 * 256 + column] = 1;
                if (!compare_distance(actor)) return 1;
            }
    for (double x : edges) for (double y : edges) for (int direction : {-1, 1}) {
        cells.fill(0);
        ActorGeometryState actor;
        actor.position = {x, y, 0}; actor.direction_44 = direction;
        if (!compare_distance(actor)) return 1;
    }
    std::mt19937_64 random(0x42F788);
    for (unsigned trial = 0; trial < 20000; ++trial) {
        grid.height = 1 + random() % 32;
        for (auto& cell : cells) cell = std::array<int, 10>{0, 0, 0, 1, 0x80, 0x200, 0x280, 0x1000, -1, 0x4000}[random() % 10];
        ActorGeometryState actor;
        actor.position = {double(int(random() % 4000) - 2000) / 8,
                          double(int(random() % 320) - 32) / 8, 0.5};
        actor.pair_360 = {double(int(random() % 65) - 32) / 8,
                          double(int(random() % 65) - 32) / 8};
        actor.direction_44 = std::array<int, 8>{-7, -2, -1, 0, 1, 2, 7, std::numeric_limits<int>::min()}[random() % 8];
        if (actor.direction_44 == 0) {
            const int row = static_cast<int>(actor.position[1]);
            if (row >= 0 && row < grid.height)
                cells[row * 256 + (static_cast<unsigned>(projected_x(actor.position[0])) & 255)] = 1;
        }
        if (!compare_range(actor) || !compare_distance(actor)) return 1;
    }
    std::cout << "native terrain probes passed " << ranges << " vertical ranges and "
              << distances << " extended-precision distances; no adapters\n";
    VirtualFree(image, 0, MEM_RELEASE);
}
