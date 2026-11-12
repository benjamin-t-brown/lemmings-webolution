#pragma once

#include "lemmings/gadget_bounds.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace lemmings {

// Known start of a retained actor. The routine at 0x00430C33 reads +8.
struct ActorHead {
    std::array<std::byte, 8> unknown_prefix;
    double position[3];
    std::array<std::byte, 0x1A> unknown_after_position;
    std::int16_t identifier;
    std::array<std::byte, 4> unknown_before_facing;
    std::int32_t facing;
    std::array<std::byte, 0x1A> unknown_after_facing;
    std::uint8_t region_flag;
};
static_assert(offsetof(ActorHead, position) == 8);
static_assert(offsetof(ActorHead, identifier) == 0x3A);
static_assert(offsetof(ActorHead, facing) == 0x40);
static_assert(offsetof(ActorHead, region_flag) == 0x5E);

struct ActorListNode {
    ActorHead* actor;
    ActorListNode* next;
};

// The original x86 list has first at +4 and an inline end node at +0xC.
// This typed representation also builds on a 64-bit host, where pointer
// fields necessarily have different offsets.
struct ActorListHead {
    std::array<std::byte, 4> unknown_prefix;
    ActorListNode* first;
    std::array<std::byte, 4> unknown_after_first;
    ActorListNode end;
};

#if defined(INTPTR_MAX) && defined(INT32_MAX) && INTPTR_MAX == INT32_MAX
static_assert(offsetof(ActorListHead, first) == 4);
static_assert(offsetof(ActorListHead, end) == 0xC);
static_assert(offsetof(ActorListNode, next) == 4);
#endif

// Recovered behavior at 0x00430C33. Returns the first matching actor in
// list order, or nullptr when no actor is inside the gadget bounds.
ActorHead* first_actor_inside(const GadgetBoundsHead& gadget,
                              const ActorListHead& actors);

} // namespace lemmings
