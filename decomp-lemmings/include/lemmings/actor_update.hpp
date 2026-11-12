#pragma once

#include <cstdint>

namespace lemmings {

struct ActorBehavior;
struct ActorBehaviorUpdate;

// Explicit equivalents of behavior vtable slots +0,+4,+8,+0xC. State callbacks
// may replace actor.primary and change remaining_time; this scheduler does
// not impose an iteration cap or invent progress for them.
struct ActorBehaviorOperations {
    void (*destroy)(ActorBehavior&, std::uint32_t delete_flags);
    std::uint32_t (*update)(ActorBehavior&, ActorBehaviorUpdate&, double& remaining_time);
    void (*enter)(ActorBehavior&, ActorBehaviorUpdate&);
    void (*leave)(ActorBehavior&, ActorBehaviorUpdate&);
};
struct ActorBehavior {
    const ActorBehaviorOperations* operations = nullptr;
    void* context = nullptr;
    std::uint8_t phase_10 = 0; // Exactly 1: every pass; exactly 0: final pass.
    std::uint8_t extra_11 = 0; // Nonzero attaches at the front of the extra list.
    std::int32_t type_14 = 0;
};
struct ActorBehaviorNode {
    ActorBehavior* behavior = nullptr;
    ActorBehaviorNode* next = nullptr;
    ActorBehaviorNode* previous = nullptr;
};
struct ActorBehaviorList {
    ActorBehaviorNode begin;
    ActorBehaviorNode end;
    ActorBehaviorList();
    ActorBehaviorList(const ActorBehaviorList&) = delete;
    ActorBehaviorList& operator=(const ActorBehaviorList&) = delete;
};
struct ActorBehaviorUpdate {
    void* context = nullptr;
    std::int32_t mode_50 = 0;
    std::uint8_t forced_transition_5d = 0;
    std::int16_t identifier_3a = 0;
    ActorBehavior* primary = nullptr; // Original actor +0x340.
    ActorBehaviorList extra;          // Original actor +0x374.
};
struct ActorBehaviorHooks {
    void* context = nullptr;
    std::int32_t forced_event_value = 0; // Original global 0x004E32CC.
    void (*forced_event)(void*, std::int16_t identifier, std::int32_t value,
                         std::uint8_t flag) = nullptr;
    // Allocation + constructor 0x00416BF0, and attachment 0x0042E696.
    ActorBehavior* (*create_forced)(void*, ActorBehaviorUpdate&) = nullptr;
    void (*attach)(void*, ActorBehaviorUpdate&, ActorBehavior*) = nullptr;
    // Called after unlinking an extra-state node; behavior already destroyed.
    void (*release_node)(void*, ActorBehaviorNode&) = nullptr;
    // Boundary at 0x0042E58C: callers can project their runtime fields into
    // finalize_actor_update (actor_finalization.hpp).
    void (*finalize)(void*, ActorBehaviorUpdate&) = nullptr;
};

// Scheduler slice of 0x0042E2BF through 0x0042E58B. Returns false immediately
// for behavior result 2. Result 1 destroys the cached primary state, or leaves,
// destroys and unlinks an extra state. Result 3 ends primary time consumption.
// Other primary results retry; other extra results outside 0..3 retry the same
// node. Callbacks must satisfy the original lifetime/progress preconditions.
bool update_actor_behavior_schedule(ActorBehaviorUpdate& actor, double elapsed,
                                    const ActorBehaviorHooks& hooks);

struct ActorBehaviorAttachmentHooks {
    void* context = nullptr;
    ActorBehaviorNode* (*allocate_node)(void*) = nullptr;
};

// 0x0042E696: enter first, then inspect extra_11. Extra states are prepended;
// primary states call the current primary's leave callback before replacing it.
// This function never destroys the previous primary. New state and allocated
// node must be nonnull, matching the original routine's preconditions.
void attach_actor_behavior(ActorBehaviorUpdate& actor, ActorBehavior& state,
                           const ActorBehaviorAttachmentHooks& hooks);

// 0x0042ECC9: checks the primary and then the extra list for an exact type ID.
bool has_actor_behavior(const ActorBehaviorUpdate& actor, std::int32_t type);

// 0x0042ED4D: return primary first, then the first matching extra behavior.
// Unlike has_actor_behavior, this original routine requires nonnull primary.
ActorBehavior* find_actor_behavior(const ActorBehaviorUpdate& actor, std::int32_t type);

struct ActorBehaviorClearHooks {
    void* context = nullptr;
    // Boundary for 0x0042FF50(node,1): release the node after its behavior's
    // deleting destructor. The clear path leaves nodes linked until the final
    // sentinel reset; remove_actor_behavior unlinks its node before this hook.
    void (*release_node)(void*, ActorBehaviorNode&) = nullptr;
};

// 0x0042E78A + 0x004302B0: first leave every extra behavior in list order;
// then destroy/release every node in a second forward pass and reset sentinels.
// Neither pass changes primary. The next pointer is read after leave/destroy,
// so callbacks must preserve valid traversal and node lifetimes until release.
void clear_actor_extra_behaviors(ActorBehaviorUpdate& actor,
                                 const ActorBehaviorClearHooks& hooks);

// 0x0042E7F3 + 0x004301B0/0x0042FEF0: leave the first matching behavior,
// recheck its identity against the current primary, then destroy and clear
// primary or find it again in the current extra list, destroy, unlink, release.
// Requires nonnull primary initially. Leave must keep the selected behavior
// reachable as current primary or a valid extra node. Destroy must preserve
// that node through unlinking. Release uses the same hook boundary as clear.
bool remove_actor_behavior(ActorBehaviorUpdate& actor, std::int32_t type,
                           const ActorBehaviorClearHooks& hooks);

// 0x0042E725: leave the initial primary, destroy the current primary if still
// nonnull, then clear it unconditionally. Leave may replace the primary.
// The initial primary must be nonnull, as in the original.
void clear_actor_primary_behavior(ActorBehaviorUpdate& actor);

} // namespace lemmings
