#include "lemmings/game_random.hpp"
namespace lemmings {
std::uint32_t next_game_random(GameRandomState& s) {
    s.value=s.value*0x343fdU+0x269ec3U;
    return (s.value>>16)&0x7fffU;
}
long double game_random_unit(GameRandomState& s) {
    return static_cast<long double>(next_game_random(s))/32767.0L;
}
long double game_random_signed(GameRandomState& s) {
    const auto half=game_random_unit(s)-0.5L;
    return half+half;
}
}
