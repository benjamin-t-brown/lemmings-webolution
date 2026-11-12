#include "lemmings/game_random.hpp"
#include <cassert>
#include <initializer_list>
int main(){
    lemmings::GameRandomState s;
    for(auto expected:{41U,18467U,6334U,26500U,19169U})assert(lemmings::next_game_random(s)==expected);
    // For each possible output integer, invert the odd multiplier modulo2^32
    // to construct a seed producing that output on the next call.
    for(std::uint32_t n=0;n<32768;++n){
        const std::uint32_t seed=((n<<16)-0x269ec3U)*0xb9b33155U;
        s.value=seed;const auto unit=lemmings::game_random_unit(s);
        assert(unit==static_cast<long double>(n)/32767.0L);
        s.value=seed;const auto signed_value=lemmings::game_random_signed(s);
        assert(signed_value>=-1&&signed_value<=1);
        if(n==0)assert(signed_value==-1);if(n==32767)assert(signed_value==1);
    }
}
