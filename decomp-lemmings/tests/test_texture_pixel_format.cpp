#include "texture_pixel_format_fixture.hpp"
int main() {
    using namespace texture_pixel_format_test;
    TexturePixelFormat out{32,0,0,16,0,0,0,0};
    TexturePixelFormat candidate{32,1,0,16,0x7c00,0x3e0,0x1f,0xf000};
    TexturePixelFormatSelection selection{16,1,0,0,0,0,{2,3,4},77,&out};
    require(select_texture_pixel_format(&candidate,&selection)==1);
    require(out==candidate && selection.found_08==1 && selection.alpha_bits_0c==4);
    candidate.red_mask_10=0x80000000; candidate.green_mask_14=0; candidate.blue_mask_18=0;
    const auto old=out;
    require(select_texture_pixel_format(&candidate,&selection)==1 && out==old);
    selection.palette_06=255; candidate.flags_04=0x20; candidate.bits_0c=7;
    require(select_texture_pixel_format(&candidate,&selection)==0 && out==candidate);
    require(selection.alpha_bits_0c==4 && selection.unknown_09==std::array<std::uint8_t,3>{2,3,4});
    for(unsigned i=0;i<trials;++i){Fixture f(i);select_texture_pixel_format(f.input(),f.context());}
    std::cout << trials << " pixel-format cases and independent alpha/palette/signed-mask assertions pass\n";
}
