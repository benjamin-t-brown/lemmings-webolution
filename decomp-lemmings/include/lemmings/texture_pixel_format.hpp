#pragma once
#include <array>
#include <cstdint>

namespace lemmings {
// DDPIXELFORMAT's eight original words. Union alternatives remain explicit;
// these names describe only the interpretation used by routine 473B1E.
struct TexturePixelFormat {
    std::uint32_t size_00;
    std::uint32_t flags_04;
    std::uint32_t fourcc_08;
    std::uint32_t bits_0c;
    std::uint32_t red_mask_10;
    std::uint32_t green_mask_14;
    std::uint32_t blue_mask_18;
    std::uint32_t alpha_mask_1c;
    bool operator==(const TexturePixelFormat&) const = default;
};
struct TexturePixelFormatSelection {
    std::uint32_t bits_00;
    std::uint8_t require_multibit_alpha_04;
    std::uint8_t alpha_flag_05;
    std::uint8_t palette_06;
    std::uint8_t fourcc_07;
    std::uint8_t found_08;
    std::array<std::uint8_t, 3> unknown_09;
    std::uint32_t alpha_bits_0c;
    TexturePixelFormat* selected_10;
};
// 473B1E. Returns original enumeration status: 0 stops after an accepted
// palette format; 1 continues, including rejected or accepted RGB formats.
// Null arguments are accepted. selected_10 must be valid when accessed.
// Candidate may equal selected_10; other overlapping object storage is excluded.
std::uint32_t select_texture_pixel_format(const TexturePixelFormat*,
                                         TexturePixelFormatSelection*) noexcept;
} // namespace lemmings
