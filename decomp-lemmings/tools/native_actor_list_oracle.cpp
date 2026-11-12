// Offline comparison of the original global actor-list search.
#include "lemmings/actor_list.hpp"
#include "lemmings/gadget_subboxes.hpp"

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
using NativeFind = lemmings::ActorHead* (__attribute__((thiscall)) *)(
    const lemmings::GadgetBoundsHead*);

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
    if (!read_at<17>(0x004090F0) || !read_at<29>(0x00409110) ||
        !read_at<16>(0x00428670) || !read_at<146>(0x00430BA1) ||
        !read_at<105>(0x00430C33) || !read_at<584>(0x00430C9C) ||
        !read_at<105>(0x00430EE4) || !read_at<27>(0x00431370)) {
        std::cerr << "expected verified function bytes on stdin\n";
        return 2;
    }
    DWORD old_protection = 0;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &old_protection)) {
        std::cerr << "VirtualProtect failed\n";
        return 2;
    }
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    auto native = reinterpret_cast<NativeFind>(0x00430C33);
    auto native_subbox = reinterpret_cast<NativeFind>(0x00430EE4);
    auto* actors = reinterpret_cast<lemmings::ActorListHead*>(0x004C88D0);

    lemmings::GadgetBoundsHead gadget{};
    lemmings::GadgetSubboxHead subbox_gadget{};
    lemmings::Vec3d lower[2]{};
    lemmings::Vec3d upper[2]{};
    subbox_gadget.local_lower.data = lower;
    subbox_gadget.local_upper.data = upper;
    subbox_gadget.local_lower.count = 2;
    subbox_gadget.local_upper.count = 2;
    for (int box = 0; box != 2; ++box) {
        for (int axis = 0; axis != 3; ++axis) {
            lower[box].value[axis] = box == 0 ? -1.0 : -3.0;
            upper[box].value[axis] = box == 0 ? 1.0 : 3.0;
        }
    }
    std::array<lemmings::ActorHead, 12> actor_storage{};
    std::array<lemmings::ActorListNode, 12> node_storage{};
    std::mt19937_64 random(0x430C33);
    std::uniform_real_distribution<double> position(-10.0, 10.0);
    int cases = 0;
    for (int trial = 0; trial != 10000; ++trial) {
        for (int axis = 0; axis != 3; ++axis) {
            gadget.minimum[axis] = -1.0;
            gadget.maximum[axis] = 1.0;
            subbox_gadget.bounds.minimum[axis] = 0.0;
        }
        if (trial % 23 == 0) {
            gadget.maximum[1] = std::numeric_limits<double>::quiet_NaN();
        }
        const int count = trial < 13 ? trial : static_cast<int>(random() % 13);
        actors->first = count == 0 ? &actors->end : &node_storage[0];
        for (int i = 0; i < count; ++i) {
            node_storage[i].actor = &actor_storage[i];
            node_storage[i].next = i + 1 == count
                ? &actors->end : &node_storage[i + 1];
            for (int axis = 0; axis != 3; ++axis) {
                actor_storage[i].position[axis] = position(random);
            }
            actor_storage[i].facing = (trial + i) % 5 == 0 ? 0
                : ((trial + i) % 2 == 0 ? -1 : 1);
            if ((trial + i) % 7 == 0) {
                actor_storage[i].position[0] = 0.0;
                actor_storage[i].position[1] = 0.0;
                actor_storage[i].position[2] = 0.0;
            }
        }
        const auto* original = native(&gadget);
        const auto* recovered = lemmings::first_actor_inside(gadget, *actors);
        ++cases;
        if (original != recovered) {
            std::cerr << "actor list mismatch at case " << cases
                      << " count=" << count << '\n';
            return 1;
        }
        const auto* original_subbox = native_subbox(&subbox_gadget.bounds);
        const auto* recovered_subbox = lemmings::first_actor_in_subbox(
            subbox_gadget, *actors);
        if (original_subbox != recovered_subbox) {
            std::cerr << "actor subbox search mismatch at case " << cases
                      << " count=" << count << '\n';
            return 1;
        }
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native actor-list searches passed " << cases
              << " cases each\n";
}
