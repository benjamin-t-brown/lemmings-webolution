#pragma once
#include <array>
#include <cstdint>

namespace lemmings {
// Known fields of base object 0x4230A2. Unnamed storage between these fields
// is intentionally absent; this portable projection is not an ABI layout.
struct ActorBaseState {
    std::uint32_t original_vtable = 0;
    std::array<double, 3> position{}; // +08, copied as bits.
    double field_20 = 0;
    std::uint8_t flag_28 = 0;
    double field_30 = 0;
    std::uint8_t flag_38 = 0;
    std::uint16_t id_3a = 0; // The stored 16 bits, independent of signed use.
};
struct ActorBaseRegistryHooks {
    void* context = nullptr;
    // Original this=0x4CBB18, argument points to a stack-local actor pointer.
    // Changing the pointer does not change the constructor's returned actor.
    void (*register_4233a0)(void*, ActorBaseState*&) = nullptr;
};

// 0x4230A2 normal-return construction. next_id represents global 0x4C55EC.
// position is a distinct array or exactly actor.position. In the latter case,
// the initial zero construction of actor.position is visible to the input.
// Registry callback must be present and return normally. SEH cleanup omitted.
ActorBaseState& construct_actor_base(ActorBaseState& actor,
    const std::array<double, 3>& position, std::uint16_t& next_id,
    const ActorBaseRegistryHooks&);
} // namespace lemmings
