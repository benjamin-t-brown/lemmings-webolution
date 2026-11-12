# Runtime resource-name table retention and resizing

## Reference and evidence

Authority: repository-local `Lemmings Revolution/Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The path is resolved from the parent repository root. The old absolute paths in
the retained REA records describe their collection provenance, not a current
installation default or authorization to execute anything.

| Original routine | File offset | Span | Portable implementation |
| --- | ---: | ---: | --- |
| Texture table loader `0x00445C10` | 285712 | 347 bytes | `load_resource_name_table` |
| Sprite table loader `0x00445EB0` | 286384 | 347 bytes | `load_resource_name_table` |
| Texture vector resize `0x00449940` | 301376 | 445 bytes | `resize_resource_entries` |
| Sprite vector resize `0x00449BF0` | 302064 | 445 bytes | `resize_resource_entries` |

The [byte dossier](resource_table_load_bytes.json) retains exact bytes, offsets,
lengths and digests for these four spans and their dependency closure. The
[primary REA dossier](resource_table_load_rea.json) retains identities, assembly,
decompiler drafts, call relationships and collection limitations; the
[helper dossier](resource_table_load_helpers_rea.json) retains supporting list,
deletion, string and indexing routines. Assembly establishes branch conditions,
operand widths and call ordering below. Decompiler signatures and pseudocode
are drafts, especially the inferred `__fastcall` labels on no-stack-argument
member helpers. No floating-point or x87 conversion occurs in these four spans.

All four primary spans already have pins in
[`tools/verify_reference.py`](../../tools/verify_reference.py). No duplicate pins
are added. This does not discharge the unrelated native-001/native-002 verifier
digest debt recorded in [FUNCTIONS.md](../FUNCTIONS.md).

## One implementation for two specializations

[`resource_table_load.cpp`](../../src/resource_table_load.cpp) represents the
normal-return control flow of both original loader/vector pairs through the
existing portable registration, cleanup and stream interfaces. This is a typed
projection, not a replacement x86 manager layout on a 64-bit host.

The four primary routines receive `this` in ECX and one stack argument, returning
with `RET 4`: the loaders take the stream pointer; resize takes the signed
requested count. Resize receives `manager + 0x30` as its vector `this`. These
are original x86 `__thiscall` boundaries; the portable API uses ordinary C++
references. Heap entry wrappers `0x0049C1A8` and `0x0049C13D` are `__cdecl`.
Manager destruction dispatches through the virtual method at vtable `+0x10`.

| State | Original offset/layout |
| --- | --- |
| Manager virtual dispatch word | `+0x00` |
| Inline begin/end list sentinels | `+0x04`, `+0x18` |
| Live loaded-node count | `+0x2C`, 32-bit word |
| Vector logical count, capacity, growth | `+0x30`, `+0x34`, `+0x38`, signed comparisons |
| Vector entry pointer | `+0x3C` (vector-relative `+0x0C`) |
| Original entry stride | `0x88` |
| Entry name, cached node, active byte | `+0x00..0x7F`, `+0x80`, `+0x84` |
| Unknown high flag bytes | `+0x85..0x87`, preserved |
| Original array cookie | 32-bit capacity immediately before entry storage |
| Loaded list node | resource `+0`, ID `+4`, references `+8`, next `+0xC`, previous `+0x10`; stride `0x14` |

Host pointer sizes can enlarge the portable entry and node types. Allocation
requests still use the original stride, not `sizeof(ResourceCacheEntry)`.
`ResourceRegistration` refers to the cache and supplies the three vector words;
`loaded_count` separately references manager `+0x2C`. Neither count substitutes
for the other. The allocation hook returns typed storage and a separate writable
cookie pointer. The deallocation hook identifies the entries; its adapter owns
the translation to the original allocation base four bytes earlier.

Texture loader `445C10` calls flag cleanup `445E10`, name membership `449B30`
and resize `449940`; sprite loader `445EB0` calls `449FB0`, `449F20` and
`449BF0`. Their indexing helpers are respectively `414D00` and `4307A0`.
Texture resize uses constructor/assignment/deletion `44BDC0`/`44B9F0`/`44B970`;
sprite resize uses `44BE60`/`448F50`/`44BA30`. The retained instructions and
the [registration](resource_registration.md),
[cleanup](resource_cache_cleanup.md) and
[runtime sprite-piece](sprite_piece_runtime.md) dependency contracts establish
the same relevant partial writes and lookup predicates. Sharing their portable
implementation is a static reconstruction conclusion, not a runtime comparison.

## Ordering, counts and arithmetic

1. Before reading even the incoming count, cleanup scans the current logical
   entries. A nonzero active byte is cleared if its cached pointer is null or
   its cached node has exactly zero references. Names, cache pointers and high
   flag bytes are untouched. Referenced entries retain their original nonzero
   active value, including values other than one. Previously inactive slots
   remain inactive regardless of their stale names or pointers.
2. Cleanup then evicts zero-reference list nodes. It saves the next link,
   destroys the resource (including null), unlinks and releases the node, then
   decrements the live loaded count. Referenced list nodes survive. Entry cache
   pointers to removed nodes are not cleared by this operation. Tests retain
   fixture node storage to observe these stale pointer values safely.
3. The incoming signed 32-bit count is read via `402560`. Resize first requests
   `incoming + live loaded_count`, before any name is tested. Logical count now
   describes this provisional table, independently of old logical count.
4. Each of the declared positive-count records is read via `47D940`, with a
   destination capacity argument of `0xFF` and a 260-byte stack temporary.
   Name membership scans only active slots in the current logical range and
   uses case-sensitive `strcmp`. Retained names and names inserted earlier in
   this same load can both match. Inactive names do not match. Each duplicate
   increments a local subtraction count and still consumes its entire record.
5. A new name scans from the saved free index until the first inactive byte.
   Copy runs through the terminating NUL, then cached pointer is set null and
   only active byte is set to one. The free index stays at the selected slot;
   the next new name scans past it. There is no compaction or index remapping.
6. Final resize requests `incoming + live loaded_count - duplicates`. It rereads
   `+0x2C` here (`445D4D` / `445FED`), rather than reusing a pre-callback count.
   Incoming names do not increment loaded count: they are slots awaiting later
   acquisition. With unchanged capacity metadata, duplicate subtraction shrinks
   logical count only and keeps the earlier allocation/capacity.

ADD, SUB and size IMUL use 32-bit wrap. The portable source computes unsigned
32-bit results and bit-casts count results back to signed words. Resize compares
requested versus capacity as signed values. If requested is no greater than
capacity, only logical count changes, even for a negative requested count.
When growth is needed, growth exactly one sets capacity to requested; otherwise
the do/while adds the current growth word until capacity is **strictly greater**
than requested. Equality performs another step. Thus capacity 2, growth 3,
request 5 produces 8, not 5. No saturation, normalization or termination bound
is added. The post-growth nonpositive-capacity branch releases existing storage
and stores null; it is distinct from ordinary shrinking.

## Allocation, partial writes and callback effects

For positive grown capacity, resize captures it before calling allocation with
wrapped `capacity * 0x88 + 4`. It writes that captured value to the cookie and
default-initializes exactly that many entries. Initialization clears name byte
zero and active byte only: cached pointer, the other 127 name bytes and high
flag bytes retain their destination contents.

When live old storage is nonnull, assignment copies each old logical entry's
name through NUL, cached pointer and active byte. The destination name suffix
and high flag bytes remain those of the new allocation, not the source. The
copy loop reloads live logical count and backing pointer. Allocation callbacks
can change these fields, growth or capacity; captured cookie/initialization
length still use pre-callback capacity. This is an observable dependency
contract, not evidence that the real allocator changes resource metadata.

Only after copying does resize release the live old storage. Original vector
deleting destructors use cookie-driven reverse element destruction with flags
3; the element destructors have no observable entry writes. The portable
deallocator boundary omits that effect-free iteration, then installs the new
entry pointer after release returns. Callback changes to live capacity/growth
are not overwritten with captured capacity. The final logical-count store
uses the requested argument. A callback-mutated loaded count can affect the
loader's final request. Callbacks must preserve all subsequently accessed
storage and a terminating list; this is not support for arbitrary corruption.

## Source tests and comparison limits

[`test_resource_table_load.cpp`](../../tests/test_resource_table_load.cpp)
adds two independently specified expected-result scenarios, using unit-only
helpers; the shared oracle fixture is unchanged. Both start with eight logical
slots and four loaded nodes, of which two are referenced. Eight incoming names
are `Keep`, `fresh`, `fresh`, `keep`, empty, `repeat`, 127 `z` bytes and `Also`.
Referenced `Keep` and `Also` survive. There are exactly three duplicates; new
names occupy slots 1, 3, 4, 5, 6 in that order. The inactive old `repeat` does
not suppress insertion. Loaded/logical counts finish at 2/7. Case distinction,
empty/NUL-only copy and a full 127-byte name are directly asserted.

The spare-capacity case retains capacity 16 without allocation. The growth case
starts at capacity 8, growth 3, reaches 11 for provisional request 10, allocates
1500 original-layout bytes, writes cookie 11 and releases old storage before
publishing new storage. The expected callback trace specifies cleanup before
count read, null destruction, unlink effects, live counts/capacity/pointers,
allocation size, cookie and stream cursor. Allocation and release callbacks
compare every entry field with expected untouched, initialized and copied
storage. Final assertions compare all 24 entries in both fixture buffers,
including name suffixes, cached pointers, low and high flag bytes and entries
beyond allocated capacity. They compare fields, not host padding. Exactly 191
bytes are consumed; the trailing four sentinel bytes remain unread.

Existing exact-growth, strictly-greater stepped-growth, negative-count shrink,
callback-mutated-capacity and cleanup-before-input assertions remain. The
existing 4,096-case loop runs the recovered source twice and compares results:
it checks repeatability only. It supplies no independent expected answers and
is separate from the new explicit scenarios. No expected result is generated
by another invocation of the recovered implementation.

Worker checks for native-003 are retained in parent `build/native-003-worker/`:
the registered MinGW-w64 GCC 16.1.0 x64 Debug target builds and the targeted
`resource_table_load` CTest passes 1/1. Read-only hashing confirms the reference
SHA-256 and all 49 spans in the byte dossier. An initial trace-literal error
(expecting a nonnull resource where the fixture supplies null) was corrected;
the failed first test log is retained. No unchanged-source/assembly discrepancy
was found. Full `verify.py --profile all` is delegated to the supervising
process after worker exit; these targeted checks do not claim a new 70/70 or
84/84 full-suite result. Independent acceptance review remains pending.

The [oracle source](../../tools/native_resource_table_load_oracle.cpp) and
[runner](../../tools/run_native_resource_table_load_oracle.py) show an intended
offline original-byte comparison route with stream/heap/virtual-destroy
adapters. Their loop counts and success-message strings are not result
artifacts. This task identifies no artifact establishing a passed comparison
for this slice and reports no original-byte runtime result. Source tests
establish source consistency only, not agreement with the original game.
No original game or original-byte oracle was executed here. Default verification
keeps `LEMMINGS_REFERENCE_EXE` empty; `native_int_array` is the only registered
`native_*` test and is an ordinary recovered-source unit test.

## Preconditions and unresolved behavior

This recovery covers successful allocation and normal return with valid,
nonoverlapping source/destination storage. Names must be bounded to at most
127 bytes plus NUL despite the larger stream temporary. Counts, cookies,
initial spare capacity and callback-mutated storage must keep every entry
access valid; newly exposed spare slots must have appropriate inactive state.
Cached nodes examined during cleanup must be live. The list must reach its
sentinel. Dirty storage checks do not authorize dereferencing freed pointers.

`LevelByteStream` is an in-memory adapter. Its count/string bounds checks throw
`std::invalid_argument` for truncated/invalid input; this exception behavior is
not established original behavior. Neither original short-read/error semantics
nor recovery after a partially consumed or partially mutated load is claimed.
The original resize contains MSVC SEH construction bookkeeping and a null
allocation branch; the portable routine assumes a successful allocation and
does not recover allocation failure or exceptional unwind. Heap internals,
Windows CRT/stream implementation, resource destruction effects, malformed or
oversize names, invalid counts/storage, concurrency and exception recovery
remain boundaries. Zero/negative/wrapping growth can fail to terminate or
produce invalid storage; such executions are explicitly excluded from tests.
Preserving wrapped arithmetic is not a safety claim for those domains.

The earlier compiled-level/name-table parser recovers serialized records,
not runtime retention/allocation or resource resolution. Its historical installed
fixture checks remain distinct from this source-only runtime-table evidence.
The four rows added to the ledger neither recount the `372 / 6,186` baseline
nor give duplicate credit for the parser's references to the two loader addresses.
