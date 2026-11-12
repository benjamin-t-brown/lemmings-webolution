// Compare original actor-ID collection with host allocation adapters.
#include "lemmings/gadget_subboxes.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <random>

namespace {
constexpr std::uintptr_t image_base = 0x00400000;
constexpr std::size_t image_size = 0xD0000;
using NativeCollect = void (__attribute__((thiscall)) *)(
    const lemmings::GadgetSubboxHead*, int, lemmings::NativeIntArray*);

extern "C" void* __cdecl adapter_alloc(std::uint32_t size, std::uint32_t) {
    return std::malloc(size);
}
extern "C" void __cdecl adapter_free(void* pointer) {
    std::free(pointer);
}

template<std::size_t Size>
bool read_at(std::uintptr_t address) {
    std::array<char, Size> bytes{};
    std::cin.read(bytes.data(), bytes.size());
    if (std::cin.gcount() != static_cast<std::streamsize>(bytes.size())) return false;
    std::memcpy(reinterpret_cast<void*>(address), bytes.data(), bytes.size());
    return true;
}

void write_jump(std::uintptr_t address, std::uintptr_t target) {
    const std::uint8_t mov_eax = 0xB8;
    const std::uint8_t jmp_eax[2] = {0xFF, 0xE0};
    std::memcpy(reinterpret_cast<void*>(address), &mov_eax, 1);
    const auto target32 = static_cast<std::uint32_t>(target);
    std::memcpy(reinterpret_cast<void*>(address + 1), &target32, 4);
    std::memcpy(reinterpret_cast<void*>(address + 5), jmp_eax, 2);
}

bool same_array(const lemmings::NativeIntArray& original,
                const lemmings::NativeIntArray& recovered) {
    if (original.count != recovered.count ||
        original.capacity != recovered.capacity ||
        original.growth != recovered.growth) return false;
    for (int i = 0; i < original.count; ++i) {
        if (original.data[i] != recovered.data[i]) return false;
    }
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
    if (!read_at<71>(0x00406650) || !read_at<18>(0x00406A00) ||
        !read_at<20>(0x00406AE0) || !read_at<17>(0x004090F0) ||
        !read_at<29>(0x00409110) || !read_at<188>(0x0041BC10) ||
        !read_at<16>(0x00428670) || !read_at<584>(0x00430C9C) ||
        !read_at<148>(0x00430F4D) || !read_at<27>(0x00431370) ||
        !read_at<11>(0x0049C13D) || !read_at<14>(0x0049C1A8)) {
        std::cerr << "expected verified function bytes on stdin\n";
        return 2;
    }
    // The original allocator's OS/CRT chain is not reconstructed. Its two
    // entry points jump to this helper process's malloc/free pair.
    write_jump(0x0049E88A, reinterpret_cast<std::uintptr_t>(&adapter_alloc));
    write_jump(0x0049E340, reinterpret_cast<std::uintptr_t>(&adapter_free));
    DWORD old_protection = 0;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &old_protection)) {
        std::cerr << "VirtualProtect failed\n";
        return 2;
    }
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    auto native = reinterpret_cast<NativeCollect>(0x00430F4D);
    auto* actors = reinterpret_cast<lemmings::ActorListHead*>(0x004C88D0);

    lemmings::GadgetSubboxHead gadget{};
    lemmings::Vec3d lower[2]{};
    lemmings::Vec3d upper[2]{};
    gadget.local_lower.data = lower;
    gadget.local_upper.data = upper;
    gadget.local_lower.count = 2;
    gadget.local_upper.count = 2;
    for (int box = 0; box != 2; ++box) {
        for (int axis = 0; axis != 3; ++axis) {
            lower[box].value[axis] = box == 0 ? -1.0 : -3.0;
            upper[box].value[axis] = box == 0 ? 1.0 : 3.0;
        }
    }
    std::array<lemmings::ActorHead, 12> actor_storage{};
    std::array<lemmings::ActorListNode, 12> node_storage{};
    lemmings::NativeIntArray original{};
    lemmings::NativeIntArray recovered{};
    original.growth = 2;
    recovered.growth = 2;

    std::mt19937_64 random(0x430F4D);
    std::uniform_real_distribution<double> position(-5.0, 5.0);
    int cases = 0;
    for (int trial = 0; trial != 10000; ++trial) {
        const int count = trial < 13 ? trial : static_cast<int>(random() % 13);
        actors->first = count == 0 ? &actors->end : &node_storage[0];
        for (int i = 0; i < count; ++i) {
            node_storage[i].actor = &actor_storage[i];
            node_storage[i].next = i + 1 == count
                ? &actors->end : &node_storage[i + 1];
            for (int axis = 0; axis != 3; ++axis) {
                actor_storage[i].position[axis] = position(random);
            }
            if ((trial + i) % 3 == 0) {
                for (double& axis : actor_storage[i].position) axis = 0.0;
            }
            actor_storage[i].facing = (trial + i) % 5 == 0 ? 0
                : ((trial + i) % 2 == 0 ? -1 : 1);
            actor_storage[i].identifier = static_cast<std::int16_t>(random());
        }
        for (int all_matches : {0, 1}) {
            native(&gadget, all_matches, &original);
            lemmings::collect_subbox_actor_ids(gadget, *actors,
                                                all_matches != 0, recovered);
            ++cases;
            if (!same_array(original, recovered)) {
                std::cerr << "actor ID mismatch at case " << cases
                          << " count=" << count
                          << " all=" << all_matches << '\n';
                return 1;
            }
        }
    }
    std::free(original.data);
    lemmings::clear_native_int_array(recovered);
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native actor ID collection passed " << cases
              << " comparisons\n";
}
