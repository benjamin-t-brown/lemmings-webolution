# Resource cache acquisition and release

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

| Routine | Address | Original bytes |
| --- | --- | ---: |
| Acquire | `42FF90` | 178 |
| Release | `430050` | 55 |
| Acquire then release | `446010` | 47 |
| Inline entry getter | `4307A0` | 30 |
| Name getter | `406500` | 14 |

Exact bytes, PE offsets and SHA-256 digests are in
[resource_cache.asm](resource_cache.asm) and the native runner. REA procedure
metadata, pseudocode and evidence IDs are retained in
[resource_cache_metadata.json](resource_cache_metadata.json).

## Static behavior

The manager's inline array header starts at `+30`; its data pointer is at
header `+0C`. Each original entry is `0x88` bytes: a 128-byte name, cached
node pointer at `+80`, and an uninterpreted final word at `+84`. A cache node
holds the returned resource pointer at `+00`, unknown word `+04`, and a
32-bit reference count at `+08`. Host structures preserve known fields; their
pointer-sized fields naturally differ in layout on x64.

Acquire captures the selected entry before callbacks. If its cache pointer
is null, call `4306F0(manager,index)` and assign the returned node to the
captured entry. If still null, call `430660(manager,index)`, optionally notify
with the captured entry's live name, refind through `4306F0`, and assign that
result to the captured entry. Return the loader's result, which may differ
from the resource in the refound node or be null. This path does **not**
increment the refound node's count. Otherwise increment the cached node's
count with 32-bit wrapping and return its resource pointer.

Release obtains the entry from the manager's **current** array and decrements
its cached node's count with 32-bit wrapping. It has **no null check** and does
not dispose anything when the count reaches zero. A valid index, nonnull
cached node and live storage are preconditions.

`446010` acquires with notification disabled, then releases through the live
manager array, and returns the captured acquire result. Thus callbacks that
move the manager array may cause the release to affect a different entry
from acquisition. An old entry captured by acquisition must remain alive
through all callbacks. The loader must leave a nonnull current cache for
balanced access; no unsupported fallback is invented.

## Verification

Unit tests pass on x64 and x86, covering cached acquisition, wrapping release,
no increment after loading, and notification suppression by balanced access.
The x86 native oracle executes all five original bodies unchanged and passes
**20,000** return-value, callback-trace, count, cache-pointer and unknown-word comparisons
at both O0 and O2. Fixtures cover already cached nodes, successful first find,
loading with null/non-null result, null second find on direct acquisition,
optional notifications, live name mutation by the loader, manager array
movement during find/load/notify, indices across six entries, and counts
`0`, `FFFFFFFF`, `7FFFFFFF`, `80000000`, and `42`.

Only find, load and the optional notification are adapted. Their internal
loading/search behavior is not claimed recovered here. The tests preserve old
entry lifetime across array movement. They do not execute invalid-index or
null-cache release faults, free resources, or establish live game behavior.
Original game data are read in place by the runner and are not stored here.

Build `tools/native_resource_cache_oracle.cpp` and `src/resource_cache.cpp`
for x86 with image base `0x10000000`, then run:

```powershell
python tools/run_native_resource_cache_oracle.py --exe 'C:/progs/Lemmings Revolution/Lemmings Revolution.exe' --program build/native_resource_cache_oracle.exe
```
