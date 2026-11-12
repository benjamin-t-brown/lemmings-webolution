# Sprite resource factory and lifetime wrappers

## Identity and scope

Reference: installed Windows `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
No executable or asset is stored here. The runner checks the whole executable,
PE mapping, each routine's digest, and both referenced strings before execution.

| Original routine | Bytes | Recovered behavior |
| --- | ---: | --- |
| `0x484EB0` | 311 | Sprite manager factory: allocate, initialize, format path, construct/open file, read, close, destroy file, return resource |
| `0x485050` | 78 | Initialize both resource array headers |
| `0x485210` | 53 | Initialize first array header |
| `0x485270` | 53 | Initialize second array header |
| `0x484FE7` | 55 | Null-tolerant manager destruction callback |
| `0x4850A0` | 46 | Scalar deleting destructor; bit zero controls free |
| `0x4850D0` | 75 | Destroy second array, then first array |
| `0x485250` | 19 | First array destructor forwards to `0x484580` |
| `0x4852B0` | 19 | Second array destructor forwards to `0x4847A0` |

Exact original bytes are recorded in [the disassembly](sprite_factory.asm).
PE offsets and SHA-256 pins are in
[the runner](../../tools/run_native_sprite_factory_oracle.py).
REA procedure extents, pseudocode, assembly, analysis identity, and evidence IDs
are retained in [the metadata](sprite_factory_metadata.json).
The factory dossier is
`ev_1f713b56706b6add5c7aae3512ab84195cf6bb0da723f53afe0473323511d2a2`;
resource constructor dossier is
`ev_e0b36c75acc159736e558f6f736ee755c358081425a30a37490f41c4b91cde81`.

## Static observations and recovered semantics

The factory saves but does not use its manager `this`. Ghidra's recovered
signature omits the name argument. Original assembly loads `[ebp+8]`, pushes it
as the third formatter argument, and ends with `ret 4`; the public API therefore
accepts a name explicitly.

Allocation requests 32 original bytes. A nonnull result runs the concrete
constructor, whose two 16-byte headers become `{0,0,1,nullptr}`. Only then does
formatting read the name. The name can change during allocation; an alias into
the allocated resource also observes the constructor's writes. The two header
views use native pointers, so their host sizes differ on a 64-bit build.

Formatting uses `Sprites\%s.spr`, with a 260-byte local filename buffer.
The opaque file object occupies 308 bytes and is constructed with that buffer.
Open is called with `(1,0)`; **only the low result byte** determines success.
Failure reports `D:\lrpc\Source\Sprites\SpriteSheets.cpp`, line 31, and the
live filename buffer, then calls exit with status zero. This path does not read,
close, or destroy the file, or free the resource before exit.

Success unconditionally calls the resource reader, including when allocation
returned null. It then closes and destroys the file and returns the retained
resource pointer. This preserves the original call sequence; it does not claim
that the original reader safely handles an allocation failure.

Destruction calls the second-array clear dependency before the first-array
clear dependency. Mutations to the first header made by the second clear are
visible to the first clear. The scalar deleting destructor always destroys
contents, frees only for `flags & 1`, and returns the original pointer even after
the freeing callback. The manager destruction wrapper accepts null; the inner
destructors require a valid resource.

## Explicit boundaries and limits

`sprite_factory.hpp` supplies separate typed headers and explicit dependencies
for allocation, formatting, file construction/open/close/destruction, sprite
reading, failure reporting, process termination, nested array clearing, and
deallocation. It does not implement these dependencies or claim recovery of
their internal behavior. Existing `sprite_resource.hpp` decodes serialized
resources; that high-level vector representation is distinct from these
runtime array headers.

All callbacks and their context must remain valid for the call. Input names
must remain readable across allocation, and formatting must produce a
terminated path within 260 bytes. The original formatter is unbounded;
overlong paths are outside the supported memory-safe domain, with no invented
truncation. Callbacks must not throw through a normal original-code execution.
The original compiler's exception-unwind handlers are not reconstructed.
The termination callback must not return. Portable tests use a thrown marker
only at that nonreturning boundary, before any original-code invocation.
The deleting destructor's returned pointer must not be dereferenced after an
actual freeing callback.

## Verification

### Portable unit tests

`test_sprite_factory` passes on x64 and x86: 25,200 normal scenarios plus
504 failed-open scenarios. Independent assertions cover initial header values,
callback order, null manager destruction, delete flags, reported source/line,
the live mutated filename, and absence of post-exit callbacks. Cases include
empty names, percent characters, 247-character names (259-character paths),
allocation failure, allocation-time name mutation, and names aliasing resource
storage.

### Native comparisons

`native_sprite_factory_oracle` passes with both `-O0` and `-O2`, each comparing
25,200 cases against the original nine routines and 18 additional fatal-open
cases in separate processes. All nine claimed routine bodies execute unchanged.
Only these dependency entries receive jump adapters:

| Address | Explicit dependency |
| --- | --- |
| `0x49C1A8` | Allocation |
| `0x49CA6A` | Formatter |
| `0x47DB73` | File constructor |
| `0x47E40D` | File open |
| `0x483EF0` | Sprite resource reader |
| `0x47E844` | File close |
| `0x47DC28` | File destructor |
| `0x482A02` | Open failure report |
| `0x49D383` | Process exit |
| `0x484580` | First array clear |
| `0x4847A0` | Second array clear |
| `0x49C13D` | Free |

Each comparison checks callback traces, all resource header fields, name/path
contents, and return-pointer identity. The reader, close, destructor, and array
clear adapters deliberately mutate fields to check ordering and visibility.
The original constructor helpers run directly, including when called by the
original factory. Native file adapters verify the same file object and buffer
are passed through the sequence.

Failed-open native cases cover `0`, `0x100`, and `0xFFFFFF00`, with successful
and null allocation, aliasing, and name mutation. Their exit adapter compares
the complete trace to the portable result, flushes its result, and calls
`std::_Exit`. It never returns or unwinds through the original MSVC SEH frames.
This tests the original fatal branch without changing it into a normal return.

The native harness replaces dependency bodies; these comparisons establish
the recovered orchestration and constructors/destructor wrappers, not original
disk I/O, allocator, sprite parsing, CRT formatting, or nested-resource cleanup.
