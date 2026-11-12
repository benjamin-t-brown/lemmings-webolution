#include "sprite_resource_destruction_fixture.hpp"
#include <iostream>

int main() {
    using namespace sprite_destruction_test;
    Fixture empty(0);
    empty.run();
    require(empty.freed == std::vector<unsigned>({empty.pointer(empty.allocations[0].cookie)}));
    require(empty.resource.clips_00.count_00 == 0 && empty.resource.clips_00.capacity_04 == 0);
    require(empty.resource.clips_00.growth_08 == 0x1200 && empty.resource.clips_00.data_0c == nullptr);

    Fixture full(39); // Three clips, three frames each, three pieces each.
    std::vector<unsigned> expected;
    for (int c = 2; c >= 0; --c) {
        for (int f = 2; f >= 0; --f)
            expected.push_back(full.pointer(reinterpret_cast<std::byte*>(full.pieces[c][f]) - 4));
        expected.push_back(full.pointer(reinterpret_cast<std::byte*>(full.frames[c]) - 4));
    }
    expected.push_back(full.pointer(reinterpret_cast<std::byte*>(full.clips) - 4));
    full.run();
    require(full.freed == expected);

    Fixture null_data(1560);
    const auto capacity = null_data.resource.clips_00.capacity_04;
    null_data.run();
    require(null_data.freed.empty());
    require(null_data.resource.clips_00.count_00 == 0 && null_data.resource.clips_00.capacity_04 == capacity);

    Fixture triple(9);
    auto* data = triple.triples[0].data_04;
    triple.run();
    require(triple.freed == std::vector<unsigned>({triple.pointer(data)}));
    require(triple.triples[0].data_04 == data);

    for (unsigned trial = 0; trial < 18720; ++trial) {
        Fixture fixture(trial);
        auto before = fixture.snapshot();
        const auto result = fixture.run();
        if (fixture.operation >= 4 && fixture.operation <= 7) {
            require(result % 100 == (fixture.flags & 2u ? 0u : 1u));
        } else if (fixture.operation == 11) {
            require(before == fixture.snapshot() && fixture.freed.empty());
        }
    }
    std::cout << "18720 nested sprite cleanup cases pass\n";
}
