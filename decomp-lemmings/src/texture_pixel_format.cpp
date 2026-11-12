#include "lemmings/texture_pixel_format.hpp"
#include <bit>

namespace lemmings {
std::uint32_t select_texture_pixel_format(const TexturePixelFormat* candidate,
                                         TexturePixelFormatSelection* selection) noexcept {
    if (!selection || !candidate || (candidate->flags_04 & 0xe0000u)) return 1;
    if (selection->palette_06) {
        if (!(candidate->flags_04 & 0x20u)) return 1;
        *selection->selected_10 = *candidate;
        selection->found_08 = 1;
        return 0;
    }
    const auto fourcc = std::uint8_t(candidate->fourcc_08 != 0);
    const auto alpha = std::uint8_t(candidate->flags_04 & 1u);
    const auto alpha_bits = std::uint32_t(std::popcount(candidate->alpha_mask_1c));
    if (candidate->bits_0c != selection->bits_00 || selection->fourcc_07 != fourcc) return 1;
    if (selection->require_multibit_alpha_04) {
        if (!alpha || alpha_bits <= 1) return 1;
    } else if (selection->alpha_flag_05 != alpha) return 1;
    if (selection->found_08) {
        const auto& previous = *selection->selected_10;
        const auto old_mask = previous.red_mask_10 | previous.green_mask_14 | previous.blue_mask_18;
        const auto new_mask = candidate->red_mask_10 | candidate->green_mask_14 | candidate->blue_mask_18;
        if (std::bit_cast<std::int32_t>(new_mask) < std::bit_cast<std::int32_t>(old_mask)) return 1;
    }
    *selection->selected_10 = *candidate;
    selection->found_08 = 1;
    if (selection->require_multibit_alpha_04) selection->alpha_bits_0c = alpha_bits;
    return 1;
}
} // namespace lemmings
