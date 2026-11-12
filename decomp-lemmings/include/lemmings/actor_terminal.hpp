#pragma once

#include "lemmings/actor_update.hpp"

#include <array>

namespace lemmings {

// Behavior type 0x1A, constructed by 0x40B3A0. Its exact gameplay name is
// not established; the falling resolver selects it for terminal transitions.
struct ActorTerminalState { double elapsed_08 = 0; };
struct TerminalActor {
    std::array<double, 3> position_08{};
    std::uint8_t flag_28 = 0;
    std::int16_t identifier_3a = 0;
    std::int32_t direction_44 = 1;
    std::int32_t category_4c = 0;
    void* model_78 = nullptr;
};
enum class TerminalEffectKind {
    effect_4018e0, // First effect when actor category != 3; allocation 0x128.
    effect_4072a0, // First effect when actor category == 3; allocation 0x128.
    effect_402870  // Second effect for every category; allocation 0x158.
};

struct ActorTerminalEntryCallbacks {
    virtual ~ActorTerminalEntryCallbacks() = default;
    virtual void event_44535b(TerminalActor&, std::int16_t id, std::int32_t value, std::uint8_t flag) = 0;
    // Allocation + selected constructor. Must return a valid effect, matching
    // the original's subsequent unconditional dereference.
    virtual void* create_effect(TerminalEffectKind kind) = 0;
    virtual void set_count(void* effect, std::int32_t count) = 0;
    virtual void set_position_08(void* effect, const std::array<double, 3>& position) = 0;
    virtual void set_pair(void* effect, double first, double second) = 0;
    virtual void set_mode(void* effect, std::int32_t mode) = 0;
    virtual std::array<double, 3> world_position_43d56c(const std::array<double, 3>& position) = 0;
    virtual void set_direction_54(void* effect, const std::array<double, 3>& direction) = 0;
    virtual std::array<double, 3> sample_triple_403f40(std::uint32_t x, std::int32_t y) = 0;
    virtual std::int32_t child_count_4018d0(void* effect) = 0;
    virtual void* child_4018b0(void* effect, std::int32_t index) = 0;
    virtual void set_child_triple_403820(void* child, const std::array<double, 3>& value) = 0;
    virtual void set_triple_4017d0(void* effect, const std::array<double, 3>& value) = 0;
    // Allocation + 0x432080; allocation failure can return null, which the
    // original still passes by reference to the insertion routine.
    virtual void* create_entity_432080(const std::array<double, 3>& position, void* effect, std::uint8_t flag) = 0;
    virtual void insert_entity_401890(void*& entity) = 0;
};
struct ActorTerminalUpdateCallbacks {
    virtual ~ActorTerminalUpdateCallbacks() = default;
    virtual void advance_model_2c(TerminalActor&, double elapsed) = 0;
    virtual std::uint8_t finished_model_44(TerminalActor&) = 0;
};

// 0x40B3A0 -> 0x4010D0 -> 0x418390; metadata only, no host vtable assignment.
void initialize_actor_terminal(ActorTerminalState& state, ActorBehavior& behavior);
// 0x40B3EC. Actor position is copied and clamped to [0,height] for effect
// generation; actor.position itself is untouched. Graphics/resource operations
// remain explicit callback boundaries, while their order/parameters are recovered.
void enter_actor_terminal(TerminalActor& actor, std::int32_t world_height,
                          std::int32_t event_value, ActorTerminalEntryCallbacks& callbacks);
// 0x40B9F2. Advance model, accumulate elapsed, zero remaining, then query
// completion; return 2 when the completion byte is nonzero, else 0.
std::uint32_t update_actor_terminal(ActorTerminalState& state, TerminalActor& actor,
                                   double& remaining, ActorTerminalUpdateCallbacks& callbacks);
void leave_actor_terminal(); // 0x40BA60: no effect.

} // namespace lemmings
