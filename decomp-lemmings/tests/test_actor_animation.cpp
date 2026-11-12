#include "lemmings/actor_animation.hpp"

#include <array>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
struct Context {
    std::vector<int> events;
    std::int32_t bound = 0;
    std::string selected;
};
void bind(void* pointer, std::int32_t id) {
    auto& context = *static_cast<Context*>(pointer);
    context.events.push_back(1); context.bound = id;
}
void select(void* pointer, const char* name) {
    auto& context = *static_cast<Context*>(pointer);
    context.events.push_back(2); context.selected = name;
}
}
int main() {
    try {
        const std::array<lemmings::AnimationClip, 3> first{{{1, 1, "walk"}, {2, 1, ""}, {3, 1, "fall"}}};
        const std::array<lemmings::AnimationClip, 2> second{{{1, 1, "walk"}, {2, 1, "walk"}}};
        const lemmings::AnimationResource a{first}, b{second}, empty{};
        check(lemmings::contains_animation_clip(a, "walk"), "exact match");
        check(lemmings::contains_animation_clip(a, ""), "empty name match");
        check(!lemmings::contains_animation_clip(a, "Walk"), "case sensitive name");
        check(!lemmings::contains_animation_clip(empty, ""), "empty resource");
        const std::array<std::int32_t, 3> defaults{12, -1, 99};
        const std::array<lemmings::ActorAnimationOverride, 3> overrides{{{&a, 21}, {&empty, 88}, {&b, -99}}};
        for (const auto* name : {"walk", "fall", "Walk", ""}) {
            Context context;
            lemmings::select_actor_animation(2, defaults, overrides, name, {&context, bind, select});
            const auto expected = std::string(name) == "walk" ? -99 : std::string(name) == "Walk" ? 99 : 21;
            check(context.bound == expected, "last matching resource/default");
            check(context.events == std::vector<int>{1, 2} && context.selected == name, "callback order");
        }
        Context context;
        lemmings::select_actor_animation(1, defaults, {}, "missing", {&context, bind, select});
        check(context.bound == -1 && context.selected == "missing", "no matching resource still selects");
        std::cout << "actor animation override precedence and bind/select order passed\n";
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
