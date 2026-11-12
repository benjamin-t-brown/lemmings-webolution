// Offline comparison of 0x0040DEC0 against the recovered finite X wrapper.
#include "lemmings/cylinder_coordinate.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>

namespace {
constexpr std::uintptr_t image_base = 0x00400000;
constexpr std::size_t image_size = 0xC0000;
using NativeX = long double (__attribute__((thiscall)) *)(const void*, double);

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
    if (!read_at<110>(0x0040DEC0) || !read_at<21>(0x0040DF30) ||
        !read_at<181>(0x0049C8F4) || !read_at<35>(0x0049FC56) ||
        !read_at<8>(0x004B0338) || !read_at<8>(0x004B0698) ||
        !read_at<8>(0x004B07A8)) {
        std::cerr << "expected verified code and constant bytes on stdin\n";
        return 2;
    }
    DWORD old_protection = 0;
    if (!VirtualProtect(image, image_size, PAGE_EXECUTE_READ, &old_protection)) {
        std::cerr << "VirtualProtect failed\n";
        return 2;
    }
    FlushInstructionCache(GetCurrentProcess(), image, image_size);
    auto native = reinterpret_cast<NativeX>(0x0040DEC0);

    int cases = 0;
    auto compare = [&](double x) {
        const long double original = native(nullptr, x);
        const long double recovered = lemmings::wrap_cylinder_x(x);
        ++cases;
        if (original == recovered &&
            (original != 0.0L || std::signbit(original) == std::signbit(recovered))) {
            return true;
        }
        std::cerr << "X mismatch x=" << x
                  << " original=" << static_cast<double>(original)
                  << " recovered=" << static_cast<double>(recovered) << '\n';
        return false;
    };

    constexpr double threshold = std::bit_cast<double>(
        std::uint64_t{0x3e45798ee0000000});
    const double special[] = {
        -1000.0, -512.0, -257.0, -256.0, -255.5, -threshold,
        -std::nextafter(threshold, 0.0), -0.0, 0.0,
        std::nextafter(threshold, 0.0), threshold,
        255.0, 255.999999, 256.0, 256.5, 512.0, 512.5, 1000.0
    };
    for (double x : special) {
        if (!compare(x)) return 1;
    }
    std::mt19937_64 random(0x40DEC0);
    std::uniform_real_distribution<double> values(-1000.0, 1000.0);
    for (int trial = 0; trial != 10000; ++trial) {
        if (!compare(values(random))) return 1;
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native finite X comparison passed " << cases << " cases\n";
}
