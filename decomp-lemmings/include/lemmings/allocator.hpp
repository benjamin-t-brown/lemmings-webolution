#pragma once

#include <cstddef>

namespace lemmings {

// Where recovered code gets its memory.
//
// The original allocates through the executable's own CRT: `operator new` at
// 0x0049C1A8 and `free` at 0x0049C13D, both __cdecl. That matters as soon as
// recovered code runs inside the game, because a block allocated on one heap
// and released on the other is heap corruption rather than a wrong answer.
//
// Offline builds (unit tests, the oracles in tools/) keep the host operators,
// which is why the offline comparisons never exposed the mismatch. The hook
// harness installs the game's pair at startup instead.
struct Allocator {
    void* (*allocate)(std::size_t bytes);
    void (*release)(void* block);
};

// Defaults to the host `::operator new` / `::operator delete` pair.
const Allocator& active_allocator();

// Installs a replacement. Both members must be non-null; a partially filled
// Allocator is rejected and the previous one is kept. Returns the Allocator
// that was in effect beforehand, so a caller can restore it.
Allocator set_active_allocator(Allocator allocator);

// The host pair, for restoring after a scoped override.
Allocator host_allocator();

} // namespace lemmings
