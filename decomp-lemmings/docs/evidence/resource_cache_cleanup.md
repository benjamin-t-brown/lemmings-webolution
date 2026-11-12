# Resource cache eviction, unloading and recreation

## Identity and scope

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The executable remains outside the repository. [The byte ledger](resource_cache_cleanup.asm)
contains each original routine span, PE offset, instruction bytes and SHA-256.
[REA metadata](resource_cache_cleanup_metadata.json) records the complete routine
descriptions/pseudocode and evidence IDs; the [flag wrapper metadata](resource_cache_cleanup_flags_metadata.json)
records `0x449FB0` separately.

| Entry | Bytes | PE offset | Recovered behavior |
|---|---:|---:|---|
| `0x44A050` | 128 | 303184 | Evict exactly zero-reference nodes |
| `0x485380` | 119 | 545664 | Evict, then unload nonnull survivors |
| `0x485400` | 108 | 545792 | Evict, then recreate every survivor |
| `0x4854D0` | 82 | 546000 | Invalidate entry caches, then unload |
| `0x485530` | 94 | 546096 | Recreate, then refresh entry caches |
| `0x449FB0` | 157 | 303024 | Clear unused entry flag bytes, then evict |
| `0x44BE00` | 83 | 310784 | Unlink and release one list node |

Native comparison also includes scalar deletion `0x44C060` (46 bytes), its
no-op destructor `0x44C090` (11 bytes), and concrete lookup/array helpers
`0x430760`, `0x430780`, `0x4306F0`, `0x4307A0`, `0x406640`.

Discovery followed the additional caller of `0x4306F0` at `0x48556B` to
`0x485530`, then its recreation/eviction callees. REA reports direct callers
of `0x44A050` as `0x449FB0`, `0x485300`, `0x485380`, `0x485400`
(`ev_32db1a996c0901d4c1af69b9cfc0ea2fdfa54c5b4a992c19a894880dd688e1b9`).
The manager destructor `0x485300` is inspected context, not recovered here.

## Views and callback boundaries

The implementation reuses `ResourceCacheLookupManager` and its concrete
20-byte x86 nodes from `resource_cache_find.hpp`. A node contains resource
pointer `+0`, ID word `+4`, reference count `+8`, next `+0xC` and previous
`+0x10`. Manager list sentinels begin at `+4` and `+0x18`; the live count
word is `+0x2C`. The entry array has signed count at `+0x30`, data pointer
at `+0x3C`, stride `0x88`, cached node at entry `+0x80`, and a flag byte
at `+0x84`. The upper three bytes of `unknown_84` remain explicit.

The public functions take separate references to the existing views and
count fields, preserving the conventions used by the recovered loader.
Host structure sizes are not asserted as original layout except in the
x86 native harness. Unknown fields and name bytes are preserved.

`ResourceCacheCleanupHooks` exposes manager virtual create at `+0xC`,
virtual destroy at `+0x10`, and storage release at `0x49C13D`. The unlink
operation, scalar-deletion decision and no-op node destructor are concrete.
Callers provide appropriate live virtual bindings; changing the hook entries
between calls models the original's repeated vtable lookup.

## Exact ordering and conditions

### Eviction `0x44A050`

Walk from the first node to the inline end sentinel. Only reference count
**exactly zero** selects a node; `0xFFFFFFFF` remains referenced. For each
selected node:

1. Save its current next pointer.
2. Invoke virtual destruction on its resource, **including null**.
3. Unlink using the node's **current** previous/next pointers. Write previous
   node's next first, then next node's previous.
4. Release node storage via scalar deletion with flags 1.
5. Decrement the **current** manager count with 32-bit wrapping.
6. Continue at the pointer saved before destruction.

The algorithm does not clear entry cache pointers or node payload fields.
Storage release callbacks can overwrite the removed node's next field without
changing the saved continuation. Nodes that are retained advance through
their current next field.

### Unload and recreate

Both begin by running eviction, then obtain the list's current first node.
Unload destroys each **nonnull** surviving resource, decrements the current
count, clears that captured node's resource pointer, then reads its current
next link. Null survivors cause no callback or decrement. Reference counts
and IDs remain unchanged.

Recreate calls the current create callback with each surviving node's ID,
stores its return value into that captured node, increments the current
manager count, then reads the current next link. Existing nonnull resources
are overwritten without a destroy call in this pass. A null create result
still increments the count. Consequently these routines are not interchangeable
with a conventional “erase all” operation or an idempotent reload.

### Wrappers

- `0x4854D0`: clear all current entry cache pointers, then run unload. The
  invalidation occurs before any destruction callback; a callback can replace
  the entry array afterwards, and the replacement is not retroactively cleared.
- `0x485530`: run recreate, then for every current entry index find the first
  node with that ID and store it, or null, in the current entry array. This
  uses the separately recovered concrete lookup.
- `0x449FB0`: for each entry whose `+0x84` byte is nonzero, clear **only that
  byte** if the cached pointer is null or its node has zero references.
  Then evict. It does not clear `cached_80`, so released-node cache pointers
  can remain as in the original. Other flag bytes and referenced entries stay
  unchanged.

Array loops compare signed counts; zero or negative counts skip their array
work but still run the associated list operation. Counts are not repaired or
recomputed. The public API deliberately preserves these observed rules.

## Verification

- **Static:** REA function bodies/pseudocode checked against the original x86
  instructions, including saved-next timing, post-callback field reloads,
  null-resource distinctions, cache invalidation order and byte-only writes.
  The runner verifies the whole executable and all fourteen executed spans.
- **Unit:** `test_resource_cache_cleanup` passed x64 `-O2` and x86 `-O0`.
  It checks empty lists, null-resource eviction, wrapped count decrement,
  linked survivors, unload clearing, upper flag-byte preservation, concrete
  cache refresh, and unlink preserving the removed node's own fields.
- **Native comparison:** `native_resource_cache_cleanup_oracle` passed
  **21,504 cases and 65,408 ordered callback snapshots at each of x86 `-O0`
  and `-O2`**. All seven public entrypoints execute unchanged original
  routines. Their internal list, lookup, unlink, scalar-deletion and no-op
  destructor chains are also unchanged. Only manager virtual create/destroy
  methods and heap release are adapted.

The matrix covers 12 initial configurations × 256 mutation scripts × 7
entrypoints: empty through eight-node lists; null/nonzero resources; zero,
one and `0xFFFFFFFF` references; zero/max/nonzero manager counts; duplicate
and negative IDs; signed negative/zero/positive entry counts; multiple flag
bytes; and null/nonzero factory results.

Mutation scripts change the live manager count, next/previous links, future
reference counts, removed-node next links, resource pointers, entry-array
storage and count, and virtual dispatch. The native harness actually replaces
the manager vtable during callbacks; source hooks switch to corresponding
alternate functions. Full normalized list/node/entry/count snapshots are
compared at every callback and after completion, including entry names and
upper flag bytes. Native node release records the call and retains fixture
storage so subsequent mutation-sensitive snapshots remain readable.

## Limits

This is the manager's eviction/unload/recreate control flow, not the concrete
resource factory/destructor implementations or the heap allocator. Resource
construction/destruction effects, allocation ownership, exception unwinding,
concurrent access and invalid/stale-pointer use are not claimed. Callbacks must
leave all subsequently accessed storage live, and list traversal must terminate.
The manager destructor/assertion path at `0x485300` remains separate. The
resource-name dispatcher `0x485590` is documented in metadata only; its
virtual factory implementations are outside this recovery.

```powershell
python tools/run_native_resource_cache_cleanup_oracle.py `
  --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' `
  --program build/x86/native_resource_cache_cleanup_oracle.exe
```
