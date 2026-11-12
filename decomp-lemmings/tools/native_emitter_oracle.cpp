// Run 0x00425A95 unchanged, replacing only its explicitly declared boundaries.
#include "../tests/emitter_update_fixture.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <iostream>
#include <random>

namespace {
using namespace emitter_test;
Fixture* active = nullptr;
std::array<std::uint8_t, 0xC8> native_emitter{};
std::array<std::uintptr_t, 19> model_vtable{};
std::array<std::uintptr_t, 6> actor_vtable{};
std::uintptr_t model = reinterpret_cast<std::uintptr_t>(model_vtable.data());
constexpr std::uintptr_t base = 0x00400000;

void __attribute__((thiscall)) effect_adapter(void*, std::uint32_t value, std::uint8_t flag) {
    active->effect_44521b(value, flag);
}
void __attribute__((thiscall)) model_update_adapter(void*, double elapsed) {
    active->update_model_432383(active->emitter, elapsed);
}
std::uint8_t __attribute__((thiscall)) model_finished_adapter(void* object) {
    return active->model_finished_44(object);
}
double __attribute__((thiscall)) model_remainder_adapter(void* object) {
    return active->model_remainder_48(object);
}
void* __cdecl allocate_adapter(std::uint32_t size) {
    return active->allocate_actor_49c1a8(size);
}
void* __attribute__((thiscall)) construct_adapter(void* storage, double x, double y,
                                                std::int32_t type) {
    return active->construct_actor_42de51(storage, x, y, type);
}
void __attribute__((thiscall)) vertical_adapter(void* actor) { active->flip_vertical_42f5e3(actor); }
void __attribute__((thiscall)) horizontal_adapter(void* actor) { active->flip_horizontal_42f66b(actor); }
std::uint8_t __attribute__((thiscall)) actor_update_adapter(void* actor, double elapsed) {
    return active->update_actor_14(actor, elapsed);
}
void __attribute__((thiscall)) true_insert_adapter(void*, void** actor) { active->insert_426180(*actor); }
void __attribute__((thiscall)) false_insert_adapter(void*, void* actor) { active->insert_4090b0(actor); }
void __attribute__((thiscall)) register_adapter(void*, void* actor) { active->register_43b536(actor); }
void __attribute__((thiscall)) refresh_adapter(void*) { active->refresh_444abc(); }

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
void write_native(Fixture& f) {
    const auto& e = f.emitter;
    auto* out = native_emitter.data();
    native_emitter.fill(0xCD);
    store(out, 0x44, &model);
    store(out, 0x98, e.remaining_98);
    store(out, 0x9A, e.coordinate_9a);
    store(out, 0x9C, e.coordinate_9c);
    store(out, 0xAC, e.opened_ac);
    store(out, 0xAD, e.flip_vertical_ad);
    store(out, 0xAE, e.effect_started_ae);
    store(out, 0xB0, e.actor_type_b0);
    store(out, 0xB4, e.direction_b4);
    store(out, 0xB6, e.alternate_b6);
    store(out, 0xB8, e.interval_b8);
    store(out, 0xC0, e.countdown_c0);
    store(reinterpret_cast<void*>(0x004E32A0), 0, f.world.suppress_effect_4e32a0);
    store(reinterpret_cast<void*>(0x004E32B4), 0, f.world.effect_value_4e32b4);
    std::memcpy(reinterpret_cast<void*>(0x004CBA8C), f.counts.data(), sizeof(f.counts));
    store(reinterpret_cast<void*>(0x004CBAA0), 0, f.world.remaining_total_4cbaa0);
    store(f.actor.data(), 0, actor_vtable.data());
    store(f.replacement.data(), 0, actor_vtable.data());
}
void read_native(Fixture& f) {
    auto& e = f.emitter;
    const auto* in = native_emitter.data();
    e.remaining_98 = load<std::int16_t>(in, 0x98);
    e.opened_ac = load<std::uint8_t>(in, 0xAC);
    e.effect_started_ae = load<std::uint8_t>(in, 0xAE);
    e.direction_b4 = load<std::int16_t>(in, 0xB4);
    // Copy floating point bytes without an x87 FLD/FSTP in the adapter.
    std::memcpy(&e.countdown_c0, in + 0xC0, sizeof(double));
    std::memcpy(f.counts.data(), reinterpret_cast<void*>(0x004CBA8C), sizeof(f.counts));
    f.world.remaining_total_4cbaa0 = load<std::uint16_t>(reinterpret_cast<void*>(0x004CBAA0), 0);
}
void copy_initial(const Fixture& source, Fixture& target) {
    target.emitter = source.emitter;
    target.counts = source.counts;
    target.world = source.world;
    target.world.remaining_by_type_4cba8c = target.counts;
    target.finished_result = source.finished_result;
    target.actor_result = source.actor_result;
    target.remainder = source.remainder;
    target.replace_on_insert = source.replace_on_insert;
    target.constructed_type = source.constructed_type;
    target.updated_type = source.updated_type;
}
bool equal_state(const Fixture& a, const Fixture& b) {
    return a.trace == b.trace && a.counts == b.counts &&
        a.world.remaining_total_4cbaa0 == b.world.remaining_total_4cbaa0 &&
        a.emitter.remaining_98 == b.emitter.remaining_98 &&
        a.emitter.opened_ac == b.emitter.opened_ac &&
        a.emitter.effect_started_ae == b.emitter.effect_started_ae &&
        a.emitter.direction_b4 == b.emitter.direction_b4 &&
        bits(a.emitter.countdown_c0) == bits(b.emitter.countdown_c0);
}
void print_trace(const Fixture& f) {
    for (const auto& event : f.trace) {
        for (auto value : event) std::cerr << std::hex << value << ' ';
        std::cerr << '\n';
    }
    std::cerr << "final timer " << std::hex << bits(f.emitter.countdown_c0) << '\n';
}
} // namespace

int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    void* image = VirtualAlloc(reinterpret_cast<void*>(base), 0xF0000,
                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(base)) return 3;
    if (!read_at<728>(0x00425A95) || !read_at<8>(0x004B0338) || !read_at<8>(0x004B0F48)) {
        std::cerr << "expected verified emitter bytes and constants on stdin\n";
        return 2;
    }
    jump(0x0044521B, reinterpret_cast<std::uintptr_t>(&effect_adapter));
    jump(0x00432383, reinterpret_cast<std::uintptr_t>(&model_update_adapter));
    jump(0x0049C1A8, reinterpret_cast<std::uintptr_t>(&allocate_adapter));
    jump(0x0042DE51, reinterpret_cast<std::uintptr_t>(&construct_adapter));
    jump(0x0042F5E3, reinterpret_cast<std::uintptr_t>(&vertical_adapter));
    jump(0x0042F66B, reinterpret_cast<std::uintptr_t>(&horizontal_adapter));
    jump(0x00426180, reinterpret_cast<std::uintptr_t>(&true_insert_adapter));
    jump(0x004090B0, reinterpret_cast<std::uintptr_t>(&false_insert_adapter));
    jump(0x0043B536, reinterpret_cast<std::uintptr_t>(&register_adapter));
    jump(0x00444ABC, reinterpret_cast<std::uintptr_t>(&refresh_adapter));
    model_vtable[0x44 / 4] = reinterpret_cast<std::uintptr_t>(&model_finished_adapter);
    model_vtable[0x48 / 4] = reinterpret_cast<std::uintptr_t>(&model_remainder_adapter);
    actor_vtable[0x14 / 4] = reinterpret_cast<std::uintptr_t>(&actor_update_adapter);
    DWORD previous = 0;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &previous)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    using NativeUpdate = std::uint8_t (__attribute__((thiscall)) *)(void*, double);
    auto native_update = reinterpret_cast<NativeUpdate>(0x00425A95);
    std::mt19937_64 random(0x425A95);
    constexpr std::uint64_t special[] = {
        0, 0x8000000000000000ULL, 1, 0x8000000000000001ULL,
        0x3FF4CCCCCCCCCCCDULL, 0x3FD0000000000000ULL, 0xBFF0000000000000ULL,
        0x7FEFFFFFFFFFFFFFULL, 0xFFEFFFFFFFFFFFFFULL, 0x7FF0000000000000ULL,
        0xFFF0000000000000ULL, 0x7FF8000000001234ULL, 0xFFF8000000004321ULL,
        0x7FF0000000000001ULL, 0xFFF0000000001234ULL};
    std::size_t spawned = 0;
    for (int trial = 0; trial != 20000; ++trial) {
        Fixture source, original;
        const auto next_double = [&](int index) {
            return std::bit_cast<double>(trial < 225
                ? special[(trial / (index == 0 ? 1 : 15) + index) % 15] : random());
        };
        auto& e = source.emitter;
        e.remaining_98 = trial < 225 ? 4 : static_cast<std::int16_t>(random());
        e.coordinate_9a = static_cast<std::int16_t>(random());
        e.coordinate_9c = static_cast<std::int16_t>(random());
        e.opened_ac = trial < 225 ? 1 : (random() % 3 == 0 ? 0 : static_cast<std::uint8_t>(random()));
        e.effect_started_ae = random() % 3 == 0 ? 0 : static_cast<std::uint8_t>(random());
        e.flip_vertical_ad = random() % 3 == 0 ? 0 : static_cast<std::uint8_t>(random());
        e.actor_type_b0 = static_cast<std::int32_t>(random());
        e.direction_b4 = std::array<std::int16_t, 5>{-32768, -1, 0, 1, 32767}[random() % 5];
        e.alternate_b6 = std::array<std::uint8_t, 4>{0, 1, 2, 255}[random() % 4];
        e.countdown_c0 = next_double(0);
        e.interval_b8 = next_double(2);
        source.remainder = next_double(3);
        const double elapsed = next_double(1);
        source.world.suppress_effect_4e32a0 = random() % 3 == 0 ? 0 : static_cast<std::uint8_t>(random());
        source.world.effect_value_4e32b4 = static_cast<std::uint32_t>(random());
        source.world.remaining_total_4cbaa0 = static_cast<std::uint16_t>(random());
        for (auto& count : source.counts) count = static_cast<std::uint16_t>(random());
        source.finished_result = std::array<std::uint8_t, 3>{0, 1, 255}[random() % 3];
        source.actor_result = std::array<std::uint8_t, 3>{0, 1, 255}[random() % 3];
        source.replace_on_insert = (random() & 1) != 0;
        source.constructed_type = random() % 5;
        source.updated_type = random() % 5;
        copy_initial(source, original);
        write_native(original);
        active = &original;
        original.sync = [&] { read_native(original); };
        const auto original_result = native_update(native_emitter.data(), elapsed);
        read_native(original);
        const auto source_result = source.run(elapsed);
        if (original_result != 1 || !source_result || !equal_state(source, original)) {
            std::cerr << "emitter mismatch trial " << trial << " elapsed "
                      << std::hex << bits(elapsed) << "\nsource:\n";
            print_trace(source);
            std::cerr << "native:\n";
            print_trace(original);
            return 1;
        }
        for (const auto& event : source.trace) if (event[0] == allocation) ++spawned;
    }
    std::cout << "20000 emitter updates matched native bytes; " << spawned
              << " emissions; callback arguments/order/state and counters compared\n";
    VirtualFree(image, 0, MEM_RELEASE);
}
