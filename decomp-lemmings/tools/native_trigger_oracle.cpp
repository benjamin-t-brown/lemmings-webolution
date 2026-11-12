// Offline comparison of 0x0041DE6B against the recovered trigger predicate.
#include "lemmings/trigger_region.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>

namespace {
constexpr std::uintptr_t image_base = 0x00400000;
constexpr std::size_t image_size = 0xD0000;
using NativeTrigger = unsigned char (__attribute__((thiscall)) *)(
    const lemmings::TriggerRegionHead*, const double*);
using NativeArm = void (__attribute__((thiscall)) *)(
    lemmings::TriggerRegionHead*, double);
using NativeNearby = void (__cdecl *)(const double*, double);
using NativeUpdate = unsigned char (__attribute__((thiscall)) *)(void*, double);
struct NativeRegistry {
    std::int32_t count;
    std::int32_t capacity;
    std::int32_t growth;
    lemmings::TriggerRegionHead** data;
};
static_assert(offsetof(NativeRegistry, data) == 0xC);
struct NativeDevice {
    lemmings::GadgetSubboxHead boxes;
    std::uint8_t timer_active;
    std::array<std::byte, 7> padding;
    double timer_remaining;
};
static_assert(offsetof(NativeDevice, timer_active) == 0x98);
static_assert(offsetof(NativeDevice, timer_remaining) == 0xA0);
int native_effect_count = 0;
extern "C" void __cdecl adapter_effect(const double*, int, int, int) {
    ++native_effect_count;
}

void write_jump(std::uintptr_t address, std::uintptr_t target) {
    const std::uint8_t code[3] = {0xB8, 0xFF, 0xE0};
    const auto target32 = static_cast<std::uint32_t>(target);
    std::memcpy(reinterpret_cast<void*>(address), code, 1);
    std::memcpy(reinterpret_cast<void*>(address + 1), &target32, 4);
    std::memcpy(reinterpret_cast<void*>(address + 5), code + 1, 2);
}

template<std::size_t Size>
bool read_at(std::uintptr_t address) {
    std::array<char, Size> bytes{};
    std::cin.read(bytes.data(), bytes.size());
    if (std::cin.gcount() != static_cast<std::streamsize>(bytes.size())) return false;
    std::memcpy(reinterpret_cast<void*>(address), bytes.data(), bytes.size());
    return true;
}
} // namespace

int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    void* image = VirtualAlloc(reinterpret_cast<void*>(image_base), image_size,
                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(image_base)) {
        std::cerr << "could not map original function addresses\n";
        return 3;
    }
    if (!read_at<60>(0x004017F0) || !read_at<94>(0x00401830) ||
        !read_at<17>(0x004090F0) || !read_at<29>(0x00409110) ||
        !read_at<51>(0x00418350) || !read_at<17>(0x0041CBA0) ||
        !read_at<59>(0x0041DD4B) || !read_at<229>(0x0041DD86) ||
        !read_at<198>(0x0041DE6B) || !read_at<263>(0x0041DF31) ||
        !read_at<23>(0x0041E090) ||
        !read_at<25>(0x0041E0B0) || !read_at<16>(0x0041E0D0) ||
        !read_at<16>(0x00428670) || !read_at<584>(0x00430C9C) ||
        !read_at<105>(0x00430EE4) || !read_at<27>(0x00431370) ||
        !read_at<8>(0x004B02D0) ||
        !read_at<8>(0x004B0338) || !read_at<8>(0x004B03F8) ||
        !read_at<8>(0x004B0DB0) || !read_at<8>(0x004B0DB8) ||
        !read_at<8>(0x004B0DC0)) {
        std::cerr << "expected verified code and constant bytes on stdin\n";
        return 2;
    }
    // The terrain/visual effect body at 0x0043F510 is not reconstructed.
    // The adapter records its invocation without changing game state.
    write_jump(0x0043F510, reinterpret_cast<std::uintptr_t>(&adapter_effect));
    DWORD old_protection = 0;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &old_protection)) {
        std::cerr << "VirtualProtect failed\n";
        return 2;
    }
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    auto native = reinterpret_cast<NativeTrigger>(0x0041DE6B);
    auto native_arm = reinterpret_cast<NativeArm>(0x0041DD4B);
    auto native_nearby = reinterpret_cast<NativeNearby>(0x0041DD86);
    auto native_update = reinterpret_cast<NativeUpdate>(0x0041DF31);
    lemmings::TriggerRegionHead region{};
    int cases = 0;
    auto compare = [&](const double (&point)[3]) {
        const bool original = native(&region, point) != 0;
        const bool recovered = lemmings::trigger_region_contains_point(region, point);
        ++cases;
        if (original == recovered) return true;
        std::cerr << "trigger mismatch at case " << cases
                  << " center=" << region.center_x << ',' << region.center_y
                  << " point=" << point[0] << ',' << point[1] << ',' << point[2]
                  << " original=" << original
                  << " recovered=" << recovered << '\n';
        return false;
    };

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    const double special[] = {-inf, -6.0, -0.0, 0.0, 1.0, 6.0, inf, nan};
    for (double center : special) {
        region.center_x = center;
        region.center_y = 0.0;
        for (int axis = 0; axis != 3; ++axis) {
            for (double value : special) {
                double point[3] = {0.0, 0.0, 0.5};
                point[axis] = value;
                if (!compare(point)) return 1;
            }
        }
    }
    region.center_x = 10.0;
    region.center_y = 20.0;
    for (double z : {-0.1, 0.0, 0.5, 1.0, 1.1}) {
        for (double x : {4.0, 10.0, 16.0}) {
            for (double y : {14.0, 20.0, 26.0}) {
                const double point[3] = {x, y, z};
                if (!compare(point)) return 1;
            }
        }
    }
    std::mt19937_64 random(0x41DE6B);
    std::uniform_real_distribution<double> position(-100.0, 100.0);
    std::uniform_real_distribution<double> offset(-7.0, 7.0);
    for (int trial = 0; trial != 10000; ++trial) {
        region.center_x = position(random);
        region.center_y = position(random);
        const double point[3] = {
            region.center_x + offset(random),
            region.center_y + offset(random),
            offset(random) / 7.0
        };
        if (!compare(point)) return 1;
    }
    std::mt19937_64 timer_random(0x41DD4B);
    for (int trial = 0; trial != 10000; ++trial) {
        lemmings::TriggerRegionHead original{};
        lemmings::TriggerRegionHead recovered{};
        original.timer_active = static_cast<std::uint8_t>(trial % 4 == 0 ? 1 : 0);
        recovered.timer_active = original.timer_active;
        original.timer_remaining = 12.5;
        recovered.timer_remaining = 12.5;
        const double duration = static_cast<double>(timer_random() % 10000) / 100.0;
        native_arm(&original, duration);
        lemmings::arm_trigger_timer_once(recovered, duration);
        if (original.timer_active != recovered.timer_active ||
            std::memcmp(&original.timer_remaining, &recovered.timer_remaining,
                        sizeof(double)) != 0) {
            std::cerr << "trigger timer mismatch at case " << trial << '\n';
            return 1;
        }
    }
    auto* registry = reinterpret_cast<NativeRegistry*>(0x004C89A8);
    std::array<lemmings::TriggerRegionHead, 12> original_actors{};
    std::array<lemmings::TriggerRegionHead, 12> recovered_actors{};
    std::array<lemmings::TriggerRegionHead*, 12> original_ptrs{};
    std::array<lemmings::TriggerRegionHead*, 12> recovered_ptrs{};
    registry->data = original_ptrs.data();
    std::uniform_real_distribution<double> nearby_offset(-15.0, 15.0);
    for (int trial = 0; trial != 5000; ++trial) {
        const int count = trial < 13 ? trial : static_cast<int>(random() % 13);
        registry->count = count;
        const double origin[3] = {position(random), position(random), position(random)};
        const double radius = trial % 17 == 0 ? 0.0 : 12.0;
        for (int i = 0; i < count; ++i) {
            original_actors[i] = {};
            original_actors[i].center_x = origin[0] + nearby_offset(random);
            original_actors[i].center_y = origin[1] + nearby_offset(random);
            original_actors[i].center_z = origin[2] + nearby_offset(random);
            original_actors[i].enabled = (trial + i) % 7 == 0 ? 0 : 1;
            original_actors[i].timer_active = (trial + i) % 11 == 0 ? 1 : 0;
            original_actors[i].timer_remaining = 7.5;
            recovered_actors[i] = original_actors[i];
            original_ptrs[i] = &original_actors[i];
            recovered_ptrs[i] = &recovered_actors[i];
        }
        native_nearby(origin, radius);
        lemmings::arm_nearby_trigger_timers(
            origin, radius,
            std::span<lemmings::TriggerRegionHead* const>(recovered_ptrs.data(), count));
        for (int i = 0; i < count; ++i) {
            if (original_actors[i].timer_active != recovered_actors[i].timer_active ||
                std::memcmp(&original_actors[i].timer_remaining,
                            &recovered_actors[i].timer_remaining,
                            sizeof(double)) != 0) {
                std::cerr << "nearby timer mismatch at trial " << trial
                          << " actor " << i << '\n';
                return 1;
            }
        }
    }
    auto* retained = reinterpret_cast<lemmings::ActorListHead*>(0x004C88D0);
    std::array<lemmings::ActorHead, 4> native_retained{};
    std::array<lemmings::ActorHead, 4> source_retained{};
    std::array<lemmings::ActorListNode, 4> native_nodes{};
    std::array<lemmings::ActorListNode, 4> source_nodes{};
    lemmings::ActorListHead source_list{};
    std::array<lemmings::TriggerRegionHead, 4> native_targets{};
    std::array<lemmings::TriggerRegionHead, 4> source_targets{};
    std::array<lemmings::TriggerRegionHead*, 4> native_target_ptrs{};
    std::array<lemmings::TriggerRegionHead*, 4> source_target_ptrs{};
    lemmings::Vec3d lower[1] = {{{-1.0, -1.0, -1.0}}};
    lemmings::Vec3d upper[1] = {{{1.0, 1.0, 1.0}}};
    registry->data = native_target_ptrs.data();
    auto source_effect = [](const double (&)[3], void* context) {
        ++*static_cast<int*>(context);
    };
    for (int trial = 0; trial != 5000; ++trial) {
        const int count = trial % 5;
        retained->first = count == 0 ? &retained->end : &native_nodes[0];
        source_list.first = count == 0 ? &source_list.end : &source_nodes[0];
        for (int i = 0; i < count; ++i) {
            native_retained[i] = {};
            for (double& axis : native_retained[i].position) {
                axis = offset(random) * 2.0;
            }
            if (trial % 7 == 0 && i == 0) {
                native_retained[i].position[0] = 0.0;
                native_retained[i].position[1] = 0.0;
                native_retained[i].position[2] = 0.5;
            }
            native_retained[i].facing = 1;
            native_retained[i].region_flag = 1;
            source_retained[i] = native_retained[i];
            native_nodes[i] = {&native_retained[i],
                i + 1 == count ? &retained->end : &native_nodes[i + 1]};
            source_nodes[i] = {&source_retained[i],
                i + 1 == count ? &source_list.end : &source_nodes[i + 1]};
        }
        registry->count = count;
        for (int i = 0; i < count; ++i) {
            native_targets[i] = {};
            native_targets[i].enabled = 1;
            native_targets[i].center_x = offset(random) * 3.0;
            native_targets[i].center_y = offset(random) * 3.0;
            native_targets[i].center_z = offset(random) * 3.0;
            source_targets[i] = native_targets[i];
            native_target_ptrs[i] = &native_targets[i];
            source_target_ptrs[i] = &source_targets[i];
        }
        NativeDevice native_device{};
        lemmings::GadgetSubboxHead boxes{};
        boxes.local_lower.count = boxes.local_upper.count = 1;
        boxes.local_lower.data = lower;
        boxes.local_upper.data = upper;
        native_device.boxes = boxes;
        const double center[3] = {0.0, 0.0, 0.0};
        std::memcpy(native_device.boxes.bounds.unknown_prefix.data() + 8,
                    center, sizeof(center));
        lemmings::TriggerRegionHead source_device{};
        native_device.timer_active = trial % 3 == 0 ? 0 : 1;
        source_device.timer_active = native_device.timer_active;
        native_device.timer_remaining = trial % 4 == 0 ? 0.2
            : (trial % 4 == 1 ? 0.1 : -0.1);
        source_device.timer_remaining = native_device.timer_remaining;
        native_effect_count = 0;
        int source_effect_count = 0;
        const bool original_result = native_update(&native_device, 0.1) != 0;
        const bool recovered_result = lemmings::update_trigger_region(
            source_device, boxes, source_list,
            std::span<lemmings::TriggerRegionHead* const>(source_target_ptrs.data(), count),
            0.1, source_effect, &source_effect_count);
        if (original_result != recovered_result ||
            native_device.timer_active != source_device.timer_active ||
            std::memcmp(&native_device.timer_remaining,
                        &source_device.timer_remaining, sizeof(double)) != 0 ||
            native_effect_count != source_effect_count) {
            std::cerr << "trigger update mismatch at trial " << trial << '\n';
            return 1;
        }
        for (int i = 0; i < count; ++i) {
            if (native_retained[i].region_flag != source_retained[i].region_flag ||
                native_targets[i].timer_active != source_targets[i].timer_active ||
                std::memcmp(&native_targets[i].timer_remaining,
                            &source_targets[i].timer_remaining,
                            sizeof(double)) != 0) {
                std::cerr << "trigger update actor mismatch at trial " << trial
                          << " actor " << i << '\n';
                return 1;
            }
        }
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native trigger region passed " << cases
              << " cases; timer passed 10000 cases; nearby effect passed 5000 lists;"
              << " update passed 5000 cases\n";
}
