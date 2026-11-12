#include "sprite_factory_fixture.hpp"
#include <iostream>

int main() {
    using namespace sprite_factory_test;
    // Independent expected results, including original callback order.
    Fixture ctor(1);
    require(ctor.run() == 1);
    require(ctor.snapshot() == std::vector<std::uint64_t>({0,0,1,0,0,0,1,0}));
    for (unsigned trial = 0; trial < 25200; ++trial) {
        Fixture fixture(trial);
        const auto result = fixture.run();
        if (fixture.operation == 0) {
            require(result == !fixture.allocation_null);
            require(fixture.trace[0] == 10 && fixture.trace[9] == 20);
            require(fixture.trace[18] == 30 && fixture.trace[19] == 40 && fixture.trace[20] == 50);
            require(fixture.trace[30] == 60 && fixture.trace[39] == 70);
            require(fixture.resource.clips_00.capacity_04 == 0xbeef);
            require(fixture.formatted_path.size() <= 259);
            if (fixture.name_alias_resource) require(fixture.formatted_path ==
                (fixture.allocation_null ? "Sprites\\INI.spr" : "Sprites\\.spr"));
            else if (fixture.mutate_name) require(fixture.formatted_path == "Sprites\\changed%by_allocator.spr");
        } else if (fixture.operation == 5) {
            require(result == 1 && fixture.trace[0] == 100 && fixture.trace[5] == 110);
            require(fixture.trace.size() == (fixture.flags & 1u ? 19u : 10u));
        } else if (fixture.operation == 6 && fixture.allocation_null) {
            require(fixture.trace.empty());
        }
    }
    for (unsigned trial = 0; trial < 504; ++trial) {
        Fixture fixture(trial, true);
        try { fixture.run(); require(false); } catch (const Terminated&) {}
        require(fixture.trace[18] == 30 && fixture.trace[19] == 40);
        require(fixture.trace[20] == 80 && fixture.trace[29] == 90 && fixture.trace.size() == 30);
        require(fixture.source_path == "D:\\lrpc\\Source\\Sprites\\SpriteSheets.cpp");
        auto expected = fixture.formatted_path;
        if (trial % 2) expected[0] = 's';
        require(fixture.reported_path == expected);
    }
    std::cout << "25200 sprite factory/constructor/destructor cases and 504 fatal-open cases pass\n";
}
