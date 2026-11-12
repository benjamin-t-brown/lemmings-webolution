#pragma once
#include <cstdint>
namespace lemmings {
// Original global 0x4C4570. Explicit state allows deterministic replay without
// tying callers to process-global addresses. No implicit reseeding.
struct GameRandomState { std::uint32_t value = 1; };
std::uint32_t next_game_random(GameRandomState&); // 0x49C84A, integer 0..32767
long double game_random_unit(GameRandomState&);  // 0x4040A0, inclusive 0..1
long double game_random_signed(GameRandomState&); // 0x404080, inclusive -1..1
}
