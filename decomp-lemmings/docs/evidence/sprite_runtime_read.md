# Complete runtime SPR reader, normal-return domain

## Authority

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

`sprite_runtime_read_metadata.json` retains REA Ghidra procedure metadata,
pseudocode, evidence IDs and limitations for 28 reader/constructor/helper
routines. `sprite_runtime_read.asm` records their original bytes, PE offsets,
span lengths and SHA-256. Composed helpers are pinned in the earlier
`sprite_piece_runtime` and `sprite_resource_destruction` evidence. The runner
verifies the entire executable and every loaded code span before native calls.
Installed assets are read directly from archive payloads; none are stored here.

The recovered API is `read_runtime_sprite_resource`, producing the existing
`SpriteFactoryResource` and nested destruction-compatible runtime views.
It composes the recovered texture-name registration and nested destruction
implementations. Actual texture decoding and graphics resource creation remain
outside this reader.

## Behavior established by bytes

`483EF0` reads clips through `484160`, then tagged triple arrays through
`4841E0`. Each array reader first clears its old contents, reads a signed count,
and calls its resize helper twice. The top-level path always supplies zero
extra capacity, so the two requested sizes are identical.

| Reader | Element | Original stride | Resize | Constructor |
| --- | --- | ---: | --- | --- |
| `484160` | clip | `98` hex | `484260` | `484420` |
| `485840` | frame | `10` hex | `4858C0` | `484C10` |
| `485AA0` | piece | `48` hex | `485B20` | `484E30` |
| `4841E0` | tagged triple array | `08` hex | `4845F0` | `47C5B0` |

If growth is one, capacity becomes the requested size. Otherwise capacity is
increased by growth until it is **strictly greater** than the request. This
includes an extra increment on exact multiples. Original allocation size is
`capacity * original_stride + 4`; its four-byte cookie stores capacity. Every
capacity element is constructed, including unused elements beyond logical
count. Count is assigned after allocation and construction. The second resize
does not allocate again.

Constructors preserve unspecified bytes:

- clip: initialize its frame array to `{0,0,1,null}`, clear only the first byte
  of the inline name, preserve speed and the rest of the name;
- frame: initialize its piece array to `{0,0,1,null}`;
- piece: no writes;
- tagged triple array: clear its data pointer, preserve its type word.

`485800` reads a clip's frames, its bounded 128-byte name, then its double speed.
`485A80` delegates to its frame's piece reader. Each piece is the concrete
`485CE0`/`474519` numeric-read and texture-registration chain recovered earlier.
The source stores pieces in the destruction module's opaque 72-byte view and
uses `memcpy` to a typed local to avoid type aliasing. Before texture heap
callbacks, that local is published into the real array, and reloaded afterwards.
Thus a callback observes the original ordering: numeric fields written, final
texture ID still pending. The native trace checks this boundary explicitly.

`47C643` reads a tag, stores it, and calls `47C6A8` on its data member. Existing
data is freed first. Tag 2 yields null. Tag 1 allocates 256 triples; every other
tag allocates 16. These allocations contain 24 bytes per triple and **no cookie**.
`4011A0` invokes the empty `401B80` constructor, leaving all triple bytes intact.
The read count differs from the allocation count: `47C810` returns 16 for tag 0,
256 for tag 1, and zero for every other tag. In particular, unknown tags allocate
16 triples and leave them unread/uninitialized. `480BB0` reads three doubles
per populated triple.

All runtime double loads quiet signaling NaNs as masked x87 load/store does,
retaining sign and payload. Integers retain their serialized bits. The existing
bounded string behavior is used without changing over-capacity or negative
length handling.

## Verification

**Static:** REA and assembly establish orchestration, member offsets, exact
constructor writes, both resize calls, growth termination comparisons, cookies,
the unknown-tag allocation/read mismatch and empty triple constructors. A
separate agent audited the source against this normal read path and found the
capacity domain needed to be explicit; the header now states it.

**Units:** x64 `-O2` and x86 `-O0` cover growth to capacity greater than a request
on exact multiples, spare capacity constructors, preserved speed/name bytes,
unknown triple tags, signaling-NaN quieting and destruction during repeat reads.

**Native:** x86 `-O0` and `-O2` each pass **384 synthetic full reads**, plus all
**19 installed SPR entries read twice**, with **6,110 allocation blocks compared**.
The original `483EF0` and complete nested reader/constructor/destructor/texture
registration chains run unchanged. Only stream access `47EEF7`, allocation
`49C1A8`, and release `49C13D` are adapters. Allocator call-site addresses identify
element kinds in the harness; the original requests, cookies and constructions
still execute. There are no early exits inserted into original routines.

The comparisons include exact cursor/EOF, final resource and texture-manager
headers, every allocation cookie and every payload byte after normalizing only
known pointer fields, allocator/release ordering, and root header snapshots at
each heap callback. A hash of the latest live piece allocation at callbacks
checks visibility of partially completed reads. Freed blocks remain physically
alive in the oracle so their final contents and release status can be compared.

Synthetic resources use zero through three clips, zero through four frames,
zero through three pieces per frame, growth increments one through seven,
three successive reads into each resource, texture slot growth and reuse,
128-byte clip-name declarations, NaN payloads, and triple tags 0, 1, 2, 3 and
`FFFFFFFF`. The installed corpus contains 132 clips, 1,526 frames, 7,354 pieces,
six type-0 arrays and 96 triples per complete corpus pass; each entry is read
again to exercise its original nested cleanup before reconstruction.

## Explicit domain and limits

The API recovers the full reader on well-formed runtime objects and complete
inputs, with successful allocation and callbacks preserving sprite object
fields and input bytes. Initial capacities must be nonnegative and positive
capacities must have backing storage. Growth must terminate without signed32
overflow, and original allocation-size arithmetic must fit uint32. Texture
callbacks may maintain their own registration state under that module's
contract. Registered names must fit its original 128-byte entry.

Because each reader clears before resizing, old-element copying in the four
**standalone** resize helpers is unreachable in this domain. Those general
resize/copy branches are not claimed as recovered public operations. Arbitrary
sprite-header mutation during allocation, failed allocation/exception unwinding,
negative counts and incomplete stream reads are also outside native equivalence.
The source rejects invalid counts, nonterminating growth, absent storage and
truncated input instead of reproducing corruption or uninitialized reads.

The reader consumes one resource and does not independently require EOF;
callers decide whether trailing bytes are permitted. The corpus oracle requires
exact EOF for each complete standalone SPR archive entry.

## Build integration

Add `src/sprite_runtime_read.cpp` to the library. Dependencies are
`sprite_piece_runtime`, `resource_registration`, `level_stream`,
`sprite_resource_destruction`, and `sprite_factory`. Unit target:
`tests/test_sprite_runtime_read.cpp`. Native target:
`tools/native_sprite_runtime_read_oracle.cpp`, linked at image base `0x10000000`.
Run `tools/run_native_sprite_runtime_read_oracle.py --exe <installed-exe>
--program <oracle>`. It reads `lemmings.box` and `patch.box` beside the executable
by default; `--archives` can explicitly select archive paths.

Host allocation callbacks receive kind, element count, and original byte count.
Return properly aligned host elements with writable cookie storage four bytes
before them for clips, frames, pieces and tagged triple arrays. Plain triples
have no cookie. Free callbacks receive the cookie address or plain triple data
pointer accordingly. The original x86 strides are not host `sizeof` values.
