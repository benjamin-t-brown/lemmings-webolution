# Nested sprite resource destruction

## Reference and recovered scope

Installed Windows `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The original executable is read in place; no executable or assets are committed.

| Address | Bytes | Behavior |
| --- | ---: | --- |
| `0x484580` | 93 | Clear clip array |
| `0x4847A0` | 93 | Clear triple-record array |
| `0x484520` | 93 | Clear frame array |
| `0x484930` | 93 | Clear piece array |
| `0x484990` | 115 | Clip scalar/vector deleting destructor |
| `0x484880` | 112 | Frame scalar/vector deleting destructor |
| `0x484800` | 112 | Piece scalar/vector deleting destructor |
| `0x484A50` | 112 | Triple-record scalar/vector deleting destructor |
| `0x484470` | 75 | Clip destructor: empty inline-name destructor, then frame array destruction |
| `0x484500` | 19 | Forward to frame-array clear |
| `0x4848F0` | 19 | Frame destructor forwarding wrapper |
| `0x484910` | 19 | Forward to piece-array clear |
| `0x484870` | 11 | Piece destructor with no effects |
| `0x41A4B0` | 11 | Inline name destructor with no effects |
| `0x47C5D3` | 99 | Free nonnull triple data, then empty pointer-wrapper destructor |
| `0x47C800` | 11 | Pointer-wrapper destructor with no effects |
| `0x484AC0` | 20 | Set piece header count to zero |
| `0x484C70` | 20 | Set frame header count to zero |
| `0x484C90` | 20 | Set clip header count to zero |
| `0x484CB0` | 20 | Set triple header count to zero |
| `0x49C655` | 104 | CRT reverse element destruction, normal nonthrowing path |
| `0x49C6BD` | 24 | CRT normal-return cleanup helper; exceptional branch excluded |

The native comparison also composes the previously recovered resource destructor
`0x4850D0` (75 bytes) and forwarding wrappers `0x485250` / `0x4852B0`
(19 bytes each), for 25 byte-pinned routines in total.

Exact bytes: [disassembly](sprite_resource_destruction.asm).
PE offsets and per-body SHA-256 values:
[native runner](../../tools/run_native_sprite_resource_destruction_oracle.py).
REA analysis identity, inclusive extents, pseudocode, assembly, and Evidence IDs:
[metadata](sprite_resource_destruction_metadata.json).
The clip-clear dossier is
`ev_90ae517e32aeabad1d9aa8224ee4cc4bc0c7b23f1cad317e6afaf4e19eb3c03f`;
the reverse iterator dossier is
`ev_76fcd6da85c3c21cd5eaa0c2f08115527c12ce6641ae7c2554fc10ff57a97ec4`.

## Static observations

Every array header has logical count, capacity, growth, and data pointer. Clear
with a null data pointer only sets count to zero; capacity and growth survive.
Clear with nonnull data sets capacity to zero first, deletes the captured
array, sets data to null after deletion returns, and sets count to zero last.
Callbacks can therefore observe the old count and data during deletion. Writes
to capacity or growth made during a free callback are not subsequently reset.

The deleting destructors inspect only flag bits 0 and 1. Bit 1 selects an array;
bit 0 selects deallocation after destruction. Scalar deletion returns the
object address. Array deletion reads its element count from a four-byte cookie
immediately before the first element, destroys elements in reverse order, and
returns the cookie address. The free callback receives that cookie address,
including for an allocated array with zero elements.

The cookie is independent of the header count and capacity. Its count is
captured before invoking element destructors. Changing that cookie during an
element's cleanup does not change the already-running iteration. Nested arrays
read their own cookies when their individual deletion begins.

Original element strides are 152 bytes for a clip, 16 for a frame, 72 for a
piece, and 8 for a triple record. Clips own frame arrays; frames own piece
arrays. The inline clip name and each piece have empty destructors. In
particular, these routines perform **no texture release**. A triple record
frees its nonnull data pointer, independent of its type tag; it does not null
that pointer or reset its type afterward.

## Implementation and allocation contract

`sprite_resource_destruction.hpp` shares `SpriteRuntimeArray` with the factory
and adds host-pointer runtime views. The serialized vector-backed types in
`sprite_resource.hpp` remain separate. Unused piece bytes, clip speed bits, and
inline name bytes are explicitly retained rather than assigned extra behavior.
The only dependency is `free_memory(context, allocation)` for `0x49C13D`.

Array storage must contain aligned host-layout elements with a 32-bit count
cookie exactly four bytes before the first element. On x64, element strides
follow host structure sizes. The caller must arrange alignment and map the
cookie address to its allocation owner; ordinary compiler `new[]` does not
establish this custom cookie contract. The count must be in `[0, INT32_MAX]`
and fit the actually constructed storage. Invalid cookies, corrupt pointers,
overflowed address calculations, and negative cookie bit patterns are outside
the supported domain. Arbitrary logical header count/capacity bits are
preserved and do not determine the deletion iteration.

Free callbacks may mutate still-live headers or remaining objects, but must
keep required storage alive until the original code is finished with it.
Arguments to the inner destructors must be nonnull. Returned deletion pointers
may already have been freed. Callbacks must not throw. Original MSVC exception
handling and the exceptional branch from `0x49C6BD` to `0x49C6D5` are not
reconstructed or exercised. The portable reverse loop implements the normal
path over supported allocation counts.

## Verification

### Unit tests

`test_sprite_resource_destruction` passes on x64 and x86: 18,720 cases.
Independent checks establish full nested reverse free order, zero-element
allocation deallocation, null-storage capacity preservation, unchanged growth,
and triple-pointer retention. Additional cases cover every public operation,
empty piece destruction, scalar/array return addresses, and flag combinations
`0`, `1`, `2`, `3`, `0x80000000`, and `0xFFFFFFFF`.

### Native comparison

Both `-O0` and `-O2` builds pass 18,720 comparisons against original x86 code.
All 25 pinned bodies execute unchanged. Only `0x49C13D` receives a jump adapter;
the entire nested chain and original CRT reverse destructor iterator execute
directly. The free adapter logs calls and retains test allocation storage for
post-destruction inspection.

Fixtures use allocation counts zero through three, three nested levels, null
and nonnull arrays, arbitrary header count/capacity values, scalar and array
deletion, and composition through the complete resource destructor. Every
case compares return address identity, free-call order, selected live header
values at each free, all header fields, all cookies, every opaque piece byte,
clip speed/name bytes, and triple type/pointer values.

Three adversarial callback modes additionally mutate the live target header,
set allocation cookies to zero after the first free, or clear a remaining
clip's frame pointer and a remaining triple's data pointer. These verify the
captured iteration state and later field reads. No allocation is actually
released during the oracle so those deliberate mutations remain within live
test storage. This verifies destruction behavior and ownership order, not
the original heap allocator or exception unwinding.
