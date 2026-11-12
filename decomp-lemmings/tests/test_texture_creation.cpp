#include "texture_creation_fixture.hpp"
int main() {
    using namespace texture_creation_test;
    const std::uint32_t expected[] = {0,0,1,2,4,8,16,1,2,4,8,16,0,0,0,0,0,0};
    for (unsigned i = 0; i < std::size(Fixture::names); ++i)
        require(texture_name_flags(Fixture::names[i]) == expected[i]);
    require(texture_has_quality_variants("abcINN_xyz"));
    require(texture_has_quality_variants("OUT_"));
    require(!texture_has_quality_variants("inn_out_"));
    for (unsigned i = 0; i < 29160; ++i) { Fixture f(i); f.run(); }
    for (unsigned i = 0; i < 270; ++i) {
        Fixture f(i, true); bool terminated = false;
        try { f.run(); } catch (const Terminated&) { terminated = true; }
        require(terminated);
    }
    Fixture f(2); f.mutation_event = 0;
    f.put<std::int32_t>(18, 3); f.put<std::int32_t>(22, 2); f.put<std::uint16_t>(28, 24);
    f.run(); require(f.resource.width_14 == 3 && f.resource.height_18 == 2);
    require(std::equal(f.output.begin(), f.output.begin() + 24, f.bmp.begin() + 64));
    require(f.output[24] == std::byte{0});
    std::cout << "29160 texture creation paths, 270 fatal paths, classification and padded BMP copy pass\n";
}
