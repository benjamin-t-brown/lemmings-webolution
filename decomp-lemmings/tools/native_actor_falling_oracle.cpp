// Original falling state bytes with explicit model, terrain-height and landing adapters.
#include "../tests/actor_falling_fixture.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <cstring>
#include <iostream>
#include <random>

namespace {
using namespace falling_test;
Fixture* active = nullptr;
std::array<std::uint8_t, 0x28> native_state{};
std::array<std::uint8_t, 0x80> native_actor{};
std::array<std::uintptr_t, 20> model_vtable{};
double native_remaining = 0;
constexpr std::uintptr_t base = 0x00400000;
template<class T> void store(void* address, std::size_t offset, T value) {
    std::memcpy(static_cast<std::uint8_t*>(address) + offset, &value, sizeof(value));
}
template<class T> T load(const void* address, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(address) + offset, sizeof(value));
    return value;
}
void read_native(Fixture& f) {
    std::memcpy(&f.state.distance_18, native_state.data() + 0x18, 8);
    f.state.umbrella_open_20 = native_state[0x20];
    f.state.last_cell_21 = native_state[0x21];
    std::memcpy(f.actor.geometry.position.data(), native_actor.data() + 8, 24);
    f.actor.geometry.direction_44 = load<std::int32_t>(native_actor.data(), 0x44);
    f.actor.umbrella_49 = native_actor[0x49];
    f.actor.floating_4b = native_actor[0x4B];
    std::memcpy(&f.remaining, &native_remaining, 8);
}
std::uint8_t __attribute__((thiscall)) query_adapter(void*, std::int32_t id) {
    return active->has_state_42ecc9(active->actor, id);
}
void __attribute__((thiscall)) animation_adapter(void*, const char* name) {
    active->select_animation_42fd25(active->actor, name);
}
long double __attribute__((thiscall)) duration_adapter(void* model) {
    return active->model_duration_4c(model);
}
long double __attribute__((thiscall)) elapsed_adapter(void* model) {
    return active->model_elapsed_48(model);
}
void __cdecl log_adapter(const char*, double value) { active->log_umbrella_time_40f860(value); }
long double __attribute__((thiscall)) height_adapter(void*, double* position, double y) {
    const auto result = active->height_43da38(active->actor.geometry.position, y);
    // Preserve the helper's ability to modify the supplied position pointer.
    std::memcpy(position, active->actor.geometry.position.data(), 24);
    return result;
}
void __attribute__((thiscall)) advance_adapter(void* model, double step) {
    active->advance_model_2c(model, step);
}
std::int32_t __attribute__((thiscall)) landing_adapter(void*, void*) {
    return active->landing_416433(active->state, active->actor);
}
void jump(std::uintptr_t address, std::uintptr_t target) {
    auto* destination = reinterpret_cast<std::uint8_t*>(address);
    destination[0] = 0xE9;
    store(destination, 1, static_cast<std::uint32_t>(target - address - 5));
}
template<std::size_t Size> bool read_at(std::uintptr_t address) {
    std::array<char, Size> bytes{};
    std::cin.read(bytes.data(), bytes.size());
    if (std::cin.gcount() != Size) return false;
    std::memcpy(reinterpret_cast<void*>(address), bytes.data(), Size);
    return true;
}
void write_native(const Fixture& f) {
    native_state.fill(0xCD);
    native_actor.fill(0xCD);
    store(native_state.data(), 0x18, f.state.distance_18);
    store(native_state.data(), 0x20, f.state.umbrella_open_20);
    store(native_state.data(), 0x21, f.state.last_cell_21);
    std::memcpy(native_actor.data() + 8, f.actor.geometry.position.data(), 24);
    store(native_actor.data(), 0x44, f.actor.geometry.direction_44);
    store(native_actor.data(), 0x49, f.actor.umbrella_49);
    store(native_actor.data(), 0x4B, f.actor.floating_4b);
    store(native_actor.data(), 0x78, model_vtable.data());
    store(reinterpret_cast<void*>(0x004C6A10), 0x418, f.grid.height);
    std::memcpy(&native_remaining, &f.remaining, 8);
}
void copy_initial(const Fixture& source, Fixture& target) {
    target.state = source.state;
    target.actor = source.actor;
    target.grid.height = source.grid.height;
    target.remaining = source.remaining;
    target.model_duration = source.model_duration;
    target.model_elapsed = source.model_elapsed;
    target.height_result = source.height_result;
    target.height_mutation = source.height_mutation;
    target.query_result = source.query_result;
    target.landing_result = source.landing_result;
    target.transition_on_cell = source.transition_on_cell;
}
bool equal_state(const Fixture& a, const Fixture& b) {
    return a.trace == b.trace && a.cells == b.cells && bits(a.remaining) == bits(b.remaining) &&
        bits(a.state.distance_18) == bits(b.state.distance_18) &&
        a.state.umbrella_open_20 == b.state.umbrella_open_20 &&
        a.state.last_cell_21 == b.state.last_cell_21 &&
        a.actor.floating_4b == b.actor.floating_4b &&
        bits(a.actor.geometry.position[0]) == bits(b.actor.geometry.position[0]) &&
        bits(a.actor.geometry.position[1]) == bits(b.actor.geometry.position[1]) &&
        bits(a.actor.geometry.position[2]) == bits(b.actor.geometry.position[2]);
}
std::array<std::uint8_t, 10> extended_bits(long double value) {
    std::array<std::uint8_t, 10> result;
    std::memcpy(result.data(), &value, 10);
    return result;
}
void print_trace(const Fixture& f) {
    for (const auto& item : f.trace) {
        for (auto value : item) std::cerr << std::hex << value << ' ';
        std::cerr << '\n';
    }
    std::cerr << "remaining " << std::hex << bits(f.remaining) << " y "
              << bits(f.actor.geometry.position[1]) << '\n';
}
} // namespace

int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    void* image = VirtualAlloc(reinterpret_cast<void*>(base), 0xD0000,
                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(base)) return 3;
    if (!read_at<52>(0x00416004) || !read_at<74>(0x00416038) ||
        !read_at<682>(0x004160BD) || !read_at<204>(0x00416367) ||
        !read_at<39>(0x0049C5AC) || !read_at<85>(0x0040DF50) ||
        !read_at<20>(0x0040BAA0) || !read_at<24>(0x004B0A08) ||
        !read_at<8>(0x004B0338) || !read_at<8>(0x004B03E8) ||
        !read_at<8>(0x004B03F8) || !read_at<8>(0x004B02D0) ||
        !read_at<56>(0x004BF354)) {
        std::cerr << "expected verified falling bytes, constants and strings\n";
        return 2;
    }
    jump(0x0042ECC9, reinterpret_cast<std::uintptr_t>(&query_adapter));
    jump(0x0042FD25, reinterpret_cast<std::uintptr_t>(&animation_adapter));
    jump(0x0040F860, reinterpret_cast<std::uintptr_t>(&log_adapter));
    jump(0x0043DA38, reinterpret_cast<std::uintptr_t>(&height_adapter));
    jump(0x00416433, reinterpret_cast<std::uintptr_t>(&landing_adapter));
    model_vtable[0x4C / 4] = reinterpret_cast<std::uintptr_t>(&duration_adapter);
    model_vtable[0x48 / 4] = reinterpret_cast<std::uintptr_t>(&elapsed_adapter);
    model_vtable[0x2C / 4] = reinterpret_cast<std::uintptr_t>(&advance_adapter);
    DWORD previous = 0;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &previous)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    using NativeEnter = void (__attribute__((thiscall)) *)(void*, void*);
    using NativeDistance = long double (__cdecl *)(void*, double);
    using NativeCellTime = long double (__attribute__((thiscall)) *)(void*, void*);
    using NativeUpdate = std::int32_t (__attribute__((thiscall)) *)(void*, void*, double*);
    auto native_enter = reinterpret_cast<NativeEnter>(0x00416004);
    auto native_distance = reinterpret_cast<NativeDistance>(0x00416038);
    auto native_cell_time = reinterpret_cast<NativeCellTime>(0x00416367);
    auto native_update = reinterpret_cast<NativeUpdate>(0x004160BD);
    std::mt19937_64 random(0x4160BD);
    constexpr std::uint64_t special[] = {
        0, 0x8000000000000000ULL, 1, 0x8000000000000001ULL,
        0x3FE0000000000000ULL, 0x3FD0000000000000ULL, 0xBFF0000000000000ULL,
        0x7FEFFFFFFFFFFFFFULL, 0xFFEFFFFFFFFFFFFFULL, 0x7FF0000000000000ULL,
        0xFFF0000000000000ULL, 0x7FF8000000001234ULL, 0xFFF8000000004321ULL,
        0x7FF0000000000001ULL, 0xFFF0000000001234ULL};
    std::size_t substeps = 0;
    for (int trial = 0; trial != 20000; ++trial) {
        Fixture source, original;
        const auto special_value = [&](int offset) {
            return std::bit_cast<double>(special[(trial + offset) % 15]);
        };
        const auto random_flag = [&] { return std::array<std::uint8_t, 3>{0, 1, 255}[random() % 3]; };
        auto& actor = source.actor;
        actor.geometry.position[1] = trial < 225 ? special_value(0) :
            trial % 4 == 0 ? std::bit_cast<double>(random()) : static_cast<double>(random() % 9500) / 100.0;
        actor.geometry.direction_44 = trial % 5 == 0 ? static_cast<std::int32_t>(random()) :
            (random() % 2 == 0 ? 1 : -1);
        actor.umbrella_49 = random_flag();
        actor.floating_4b = random_flag();
        source.state.distance_18 = trial % 5 == 0 ? special_value(3) : static_cast<double>(random() % 15);
        source.state.umbrella_open_20 = random_flag();
        source.state.last_cell_21 = random_flag();
        source.remaining = trial < 225 ? special_value(2) : static_cast<double>(random() % 1000) / 1000.0;
        source.model_duration = trial % 3 == 0 ? static_cast<long double>(special_value(4)) : 0.15L;
        source.model_elapsed = trial % 3 == 0 ? static_cast<long double>(special_value(6)) : 0.075L;
        source.height_result = trial % 3 == 0 ? static_cast<long double>(special_value(7)) : 2.125L;
        source.height_mutation = trial % 7 == 0 ? 0.125 : 0;
        source.query_result = random_flag();
        source.landing_result = (random() & 1) ? 1 : -3;
        source.transition_on_cell = 1 + random() % 3;
        if (trial == 3) {
            actor.geometry.position[1] = 20;
            actor.geometry.direction_44 = 1;
            actor.umbrella_49 = source.state.umbrella_open_20 = 1;
            actor.floating_4b = source.state.last_cell_21 = 0;
            source.model_duration = 0;
            source.model_elapsed = std::bit_cast<double>(std::uint64_t{0x3E45798EE0000000});
            source.remaining = 0.1;
            source.transition_on_cell = 1;
        }
        copy_initial(source, original);
        write_native(original);

        const auto native_d = native_distance(native_actor.data(), source.remaining);
        const auto source_d = lemmings::actor_fall_distance(source.actor, source.remaining);
        const auto native_t = native_cell_time(native_state.data(), native_actor.data());
        const auto source_t = lemmings::actor_fall_time_to_cell(source.state, source.actor);
        if (extended_bits(native_d) != extended_bits(source_d) ||
            extended_bits(native_t) != extended_bits(source_t)) {
            std::cerr << "falling helper mismatch trial " << trial << '\n';
            return 1;
        }

        active = &original;
        original.sync = [&] { read_native(original); };
        if (trial % 4 == 0) {
            native_enter(native_state.data(), native_actor.data());
            read_native(original);
            lemmings::enter_actor_falling(source.actor, source);
            if (!equal_state(source, original)) {
                std::cerr << "falling enter mismatch trial " << trial << '\n';
                return 1;
            }
        }
        const auto original_result = native_update(native_state.data(), native_actor.data(), &native_remaining);
        read_native(original);
        const auto source_result = source.run();
        if (source_result != original_result || !equal_state(source, original)) {
            std::cerr << "falling update mismatch trial " << trial << "\nsource:\n";
            print_trace(source);
            std::cerr << "native:\n";
            print_trace(original);
            return 1;
        }
        for (const auto& item : source.trace) if (item[0] == height) ++substeps;
    }
    std::cout << "20000 falling updates, 40000 helper calls and 5000 entries matched; "
              << substeps << " substeps; original arithmetic and callback traces compared\n";
    VirtualFree(image, 0, MEM_RELEASE);
}
