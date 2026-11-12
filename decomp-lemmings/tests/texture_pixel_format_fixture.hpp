#pragma once
#include "lemmings/texture_pixel_format.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
namespace texture_pixel_format_test {
using namespace lemmings;
inline void require(bool value) { if (!value) std::exit(1); }
struct Fixture {
    TexturePixelFormat candidate, selected;
    TexturePixelFormatSelection selection;
    unsigned alias;
    explicit Fixture(unsigned trial) {
        std::uint32_t seed = trial + 0x12345678;
        auto next = [&] { seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5; return seed; };
        std::array<std::uint32_t,8> fields;
        for (auto& value : fields) value=next();
        std::memcpy(&candidate,fields.data(),sizeof candidate);
        for (auto& value : fields) value=next();
        std::memcpy(&selected,fields.data(),sizeof selected);
        const std::uint8_t bytes[] = {0,1,2,255};
        selection = {16, bytes[(trial/16)%4], bytes[(trial/64)%4], bytes[(trial/256)%4],
            bytes[(trial/1024)%4], bytes[(trial/4096)%4], {7,8,9}, next(), &selected};
        candidate.flags_04 = ((trial % 4) == 0 ? 0x20000u : 0u) |
            ((trial % 4) == 1 ? 0x20u : 0u) | ((trial / 4) % 2);
        candidate.fourcc_08 = (trial/8)%2 ? next() : 0;
        candidate.bits_0c = trial % 7 ? 16 : 32;
        const std::uint32_t masks[] = {0,1,3,0x80000000u,0xc0000000u,0xffffffffu};
        candidate.alpha_mask_1c = masks[(trial/16384)%6];
        const std::uint32_t rgb[] = {0,1,0x7fffffffu,0x80000000u,0xffffffffu};
        candidate.red_mask_10 = rgb[(trial/98304)%5];
        candidate.green_mask_14 = candidate.blue_mask_18 = 0;
        selected.red_mask_10 = rgb[(trial/491520)%5];
        selected.green_mask_14 = selected.blue_mask_18 = 0;
        alias = trial%97 == 0 ? 1 : trial%101 == 0 ? 2 : trial%103 == 0 ? 3 : 0;
    }
    const TexturePixelFormat* input() { return alias==1 ? nullptr : alias==3 ? &selected : &candidate; }
    TexturePixelFormatSelection* context() { return alias==2 ? nullptr : &selection; }
    bool equivalent(const Fixture& b) const {
        return candidate==b.candidate && selected==b.selected && selection.bits_00==b.selection.bits_00 &&
            std::memcmp(&selection.require_multibit_alpha_04,&b.selection.require_multibit_alpha_04,12)==0;
    }
};
inline constexpr unsigned trials = 2457600;
} // namespace texture_pixel_format_test
