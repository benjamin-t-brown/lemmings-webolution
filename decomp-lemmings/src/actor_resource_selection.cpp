#include "lemmings/actor_resource_selection.hpp"
#include <string>

namespace lemmings {
void select_actor_resources(const std::array<std::uint8_t,4>& enabled,
    const std::array<std::int32_t,4>& quality, std::array<std::int32_t,4>& ids,
    const ActorResourceSelectionHooks& hooks) {
    constexpr std::array<std::string_view,4> prefixes{"Lemmings","LemmingsWater","LemmingsAcid","EvilRat"};
    constexpr std::array<std::string_view,2> suffixes{"Lo","Hi"};
    for(std::size_t index=0;index<4;++index) {
        if(enabled[index]) {
            std::string name(prefixes[index]);
            name+=suffixes[quality[index]];
            const auto id=hooks.register_446040(hooks.context,name);
            ids[index]=id;
        } else ids[index]=-1;
    }
}
} // namespace lemmings
