# Sprite vertex packing and draw dispatch

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Original bytes, file offsets and routine hashes are recorded in the `.asm`.
REA evidence IDs, information and pseudocode are in `.metadata.json`.
The runner verifies the executable, all executed spans and the 0.25 constant
at `4B19D8` before running the original code.

## Recovered routines

| Address | Bytes | Behavior |
| --- | ---: | --- |
| `46D460` | 421 | Pack each input vertex and issue one device draw call. |
| `46E030` | 274 | Resize plain 32-byte vertex array. |
| `402C00` | 53 | Initialize array to count/capacity/growth/data 0/0/1/null. |
| `46E000` | 27 | Index device vertex by stride32. |
| `46E020` | 16 | Return device vertex count. |
| `46DEC0` | 16 | Return input pointer count. |
| `46DE70` | 22 | Index three-component double color data. |
| `46DE90` | 22 | Index two-component double coordinates. |
| `46DEB0` | 11 | Empty post-draw operation; no counter increment. |
| `46D605` | 15 | Global array cleanup entry. |
| `402C40` | 19 | Cleanup wrapper. |
| `403890` | 71 | Free data and clear count. |
| `403EC0` | 20 | Set count zero. |

The native chain additionally executes unchanged pointer index helper `414A40`
and CRT float-to-integer helper `49C5AC`. Source is
`sprite_render_draw.hpp/.cpp`. It composes with submission's vertex and device
views without modifying their layout or claiming previously unknown fields.

## Packing established by instructions

Each output vertex has stride32. The source input has stride104.

| Output | Input or expression |
| --- | --- |
| float +00/+04 | Input double +18/+20, converted separately. |
| float +08 | Input double +10, converted before reciprocal calculation. |
| float +0C | `0.25 / output_z`, with the stored float Z reloaded. |
| uint32 +10 | `(trunc(alpha)<<24) \| (trunc(red)<<16) \| (trunc(green)<<8) \| trunc(blue)`. |
| uint32 +14 | Preserved; routine never assigns it. |
| float +18/+1C | Globals `58D2A0/58D2A8` plus input double +28/+30. |

Alpha is input +50; RGB are +38/+40/+48. Truncation uses signed 64-bit
`__ftol`, retaining the low 32 bits. Out-of-range and nonfinite conversions
yield low word zero under the original masked x87 behavior. Components are
not clamped or masked to eight bits; e.g. blue -1 makes the entire packed word
`FFFFFFFF`. The source models that wrapping integer behavior explicitly.

Float arithmetic follows the original stores and uses extended intermediates
for division and coordinate addition. Numeric NaN payloads are not claimed.
The original makes no calls between input conversions, so their order creates
no callback boundaries.

The array is lazily initialized when bit zero of `58D550` is clear. The routine
sets the bit, constructs array `58D540`, registers cleanup `46D605`, resizes to
the input count, and iterates the live input count. It then always calls device
vtable +70, including for zero count, with arguments `(device,6,1C4,data,count,0)`.
These encode the observed triangle-fan primitive and transformed/lit vertex
format. Return status is ignored. No physical GPU driver is exercised here.

Resize allocates plain uninitialized storage, without an array cookie or vertex
constructor. Growth1 uses exact requested capacity; other growth values add
with 32-bit wrapping until capacity exceeds count. It rereads old data/count
after allocation, copies all32bytes per old vertex, frees old data, publishes
the captured new pointer, then assigns count last. Cleanup sets capacity zero
before freeing nonnull data and count zero after; null data preserves capacity.

## Verification

**Static:** REA and disassembly establish field offsets, the intermediate float
Z store/reload, raw integer color store, untouched output +14, lazy flags,
callback rereads, and empty `46DEB0`. All relevant bytes are pinned.

**Units:** x64 `-O2` and x86 `-O0` pass position/reciprocal packing, ARGB
truncation, absence of color clamping, UV offsets, preserved output word, zero
count dispatch and cleanup checks.

**Native:** x86 `-O0` and `-O2` each pass **30,000 complete draw and cleanup
comparisons** using the original 15-routine chain. Only allocation, free, CRT
cleanup registration and the device COM method are adapted. No vertex
conversion, vector helper or integer conversion is replaced in the original.

The fixture compares all source vertex bytes, every callback trace, normalized
array headers and all old/new/alternate output buffers. Finite output floats,
signed zeros, infinities, colors and the untouched word compare bit-for-bit;
NaN floats compare by classification. Cases cover counts0..11, both lazy states,
three growth values, finite/random-bit double attributes, subnormals, zero,
infinities, NaNs, huge colors and eight mutation scripts. Scripts exercise
registration-time input/header changes, allocator-time data/count changes,
device replacement, UV offset changes, free-time header changes and draw-time
state mutations. Upper initialization flag bytes are nonzero throughout.

Normal-return, successful allocation, live storage and terminating growth are
preconditions. Callbacks must leave subsequent array accesses valid. Allocation
failure, exceptions, x87 status flags and unmasked floating exceptions are not
claimed. Clipping remains a separate caller; this slice reaches the final
graphics interface but does not implement the driver or a visible game.
