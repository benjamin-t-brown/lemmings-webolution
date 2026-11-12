#pragma once

#include "lemmings/actor_list.hpp"
#include "lemmings/native_int_array.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace lemmings {

struct Vec3d {
    double value[3];
};
static_assert(sizeof(Vec3d) == 0x18);

// A recovered 16-byte x86 array header: count at +0, data at +0xC.
// Pointer-width-dependent host layout is used for the compiled library.
struct Vec3ArrayHead {
    std::int32_t count;
    std::array<std::byte, 8> unknown;
    const Vec3d* data;
};

// Known object prefix through the pair of local lower/upper box arrays.
struct GadgetSubboxHead {
    GadgetBoundsHead bounds;
    Vec3ArrayHead local_lower;
    Vec3ArrayHead local_upper;
};

#if defined(INTPTR_MAX) && defined(INT32_MAX) && INTPTR_MAX == INT32_MAX
static_assert(offsetof(Vec3ArrayHead, data) == 0xC);
static_assert(sizeof(Vec3ArrayHead) == 0x10);
static_assert(offsetof(GadgetSubboxHead, local_lower) == 0x78);
static_assert(offsetof(GadgetSubboxHead, local_upper) == 0x88);
#endif

// Recovered from 0x00430C9C. The original returns the first local box index,
// or -1 if facing is not +/-1 or no box contains the actor's point.
int containing_subbox_index(const GadgetSubboxHead& gadget,
                            const ActorHead& actor);

// Recovered from 0x00430EE4. Searches retained actors in list order.
ActorHead* first_actor_in_subbox(const GadgetSubboxHead& gadget,
                                 const ActorListHead& actors);

// Recovered from 0x00430F4D. Clears output, then appends signed actor IDs
// for every matching actor or only the first, according to all_matches.
void collect_subbox_actor_ids(const GadgetSubboxHead& gadget,
                              const ActorListHead& actors,
                              bool all_matches,
                              NativeIntArray& output);

} // namespace lemmings
