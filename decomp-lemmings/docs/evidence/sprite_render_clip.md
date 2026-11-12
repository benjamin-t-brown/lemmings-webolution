# Sprite clipping, scratch vertices and interpolation

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The accompanying `.asm` records mapped offsets, original bytes and routine
SHA-256 hashes; `.metadata.json` records REA evidence IDs and pseudocode.
The native runner verifies executable identity, all33 routine spans and the
zero constant at `4B0338`.

## Scope

`sprite_render_clip.hpp/.cpp` recovers the following pipeline and its helpers.

| Address | Behavior |
| --- | --- |
| `46D3C1` | Four-plane clipping and nonempty final draw dispatch. |
| `46CCE2` / `46CE60` | Maximum/minimum Y plane passes. |
| `46CFDE` / `46D15C` | Maximum/minimum X plane passes. |
| `46D910` / `46DB80` | Y/X intersection interpolation. |
| `46DED0` | Append pointer, growing by one configured increment if necessary. |
| `46DF90` | Take scratch vertex from free-pointer stack or contiguous pool. |
| `46E150`, `46E170`, `46E190`, `46E1B0`, `46E1D0`, `46E1F0` | Scratch pointer stack helpers. |
| `420B80` | Contiguous vertex indexing by stride104. |
| `420D60`, `420DA0`, `401B80`, `4821D0` | Vertex/member construction and destruction, without field initialization. |
| `401830`, `402680`, `4027C0`, `4017F0` | Three-component subtraction, addition, scaling and raw construction. |
| `452770`, `4527D0`, `46D8B0`, `404D40`, `452830` | Two-component arithmetic, raw construction and access. |

The original chain also executes previously recovered count/index/resize
helpers `46DEC0`, `414A40`, `414A60`, and `414EF0`. Source composes operations
instead of retaining trivial wrappers. The apparent MFC class names emitted
by Ghidra for empty constructors are not used as semantic type evidence.

## Plane control and ordering

`58D280/284/288/28C` are **signed integers**, loaded with `FILD` and converted
to double immediately before each plane. The order is maximumY, minimumY,
maximumX, minimumX. Passes alternate between the caller's array and temporary
array `58D558`. Each pass clears only the output count, captures the input's
first/last pointer slots and initial count, then visits the closed polygon
starting with its last-to-first edge. Crossing intersections precede an inside
current vertex. Each empty result returns immediately; otherwise `46D460`
receives the final caller array.

Maximum planes classify `coordinate > plane` as outside, so unordered values
are inside. Minimum planes classify `!(coordinate >= plane)` as outside, so
unordered values are outside. These are instruction-derived x87 status tests,
not symmetric comparison substitutions. NaN coordinates can therefore create
NaN intersections at the minimum planes.

## Interpolation and its limits

Each interpolator orders its endpoints by the clipped axis. The first endpoint
is the high endpoint only when it is strictly greater; equality and unordered
select the second endpoint as high. It computes the other screen coordinate
using a linear ratio **stored to binary64 before multiplication**. The plane
coordinate itself is assigned by raw word copies.

For remaining attributes, with high/low axis values H/L and plane P, it first
stores `D = (L-P)*low_z + (P-H)*high_z` to binary64. If D is neither zero nor
NaN it replaces D with the binary64 ratio `(P-H)*high_z / D`. It then uses D
to interpolate components +00/+08/+10, +28/+30 and +38/+40/+48. Subtraction,
scaling and addition each have separate original binary64 stores. Alpha +50
uses one extended expression with a final binary64 store.

**Unknown tail:** the original stack temporary is104bytes, but its constructor
does not initialize fields and the interpolator assigns only the first88bytes.
The following16bytes at +58..+67 are copied from indeterminate stack storage.
They are not a stable recoverable value. The public source projection writes
the defined88bytes and preserves an explicitly unknown destination tail.
Native checks compare the defined prefix, and no original tail value is
claimed. The recovered final device packing never reads these16bytes.

This limitation is explicit rather than represented by uninitialized C++ reads.
It does not change the defined geometry/color/coordinate values used by the
recovered renderer.

## Storage

Scratch allocation first checks the available pointer count. If nonzero it
returns the last pointer and decrements that count. Otherwise it returns
`vertices[used]` and increments the unsigned used word. There is no capacity
check or heap allocation in this helper. Submission resets both scratch counts
before starting a polygon; callers composing the separate APIs must bridge
those two fields.

Pointer append grows when capacity is at most count, adding growth once with
32-bit wrapping. It allocates original capacity*4 bytes, copies the live count,
frees old data even when null, publishes the new pointer, dereferences the
supplied pointer slot after callbacks, and increments count. Existing input
pointer storage must remain alive through callbacks because each plane captures
addresses into that storage before iterating.

## Verification

**Static:** all33 routine bodies and their byte pins were read, including
integer clipping bounds, asymmetric unordered branches, perspective weighting,
binary64 store locations, empty constructors, scratch selection and pointer
append behavior. `46D3C1` is159bytes; each plane pass is382bytes; each
interpolator is615bytes. The byte ledger gives exact sizes for all helpers.

**Units:** x64 `-O2` and x86 `-O0` pass midpoint interpolation, inside-pointer
reuse, maximum-plane early exit, NaN classification on both plane directions,
and the explicit unknown-tail preservation contract.

**Native:** x86 `-O0` and `-O2` each pass **12,000 complete four-plane
comparisons** and **40,000 direct interpolation comparisons**. The33 original
routine bodies execute unchanged. Only heap allocation/free and final drawing
are adapted. Original interpolation, scratch selection, vector arithmetic,
growth and branch logic are not replaced.

Complete cases compare every allocation/free/draw trace, live pointer array
contents and headers, pool use/free counts, all original input bytes and the
defined prefix of each used scratch vertex. Cases cover counts1..12, all four
early-exit directions, fully inside and crossing polygons, reversed edges,
NaNs/infinities/zero depths, multiple growth sizes and free-list populations,
and callback changes to later clipping bounds. Direct interpolation adds
random finite and raw-bit attributes, signed zeros, subnormals, huge values,
infinities and NaNs in both axis variants. Non-NaN numeric values compare
bit-for-bit; numeric NaNs compare by classification. Indeterminate tails are
excluded from native value claims.

**Composition:** submission's clipping callback can call
`clip_and_draw_sprite_polygon`; its drawing callback can call the separately
verified `draw_sprite_polygon`. This reaches the actual device interface with
clipping and packing in recovered C++. The current native clip runner adapts
that final draw, so this ledger does not claim one uninterrupted native
submission-through-device run or visible rendering.

## Preconditions

Initial input count must be positive. Available-pointer count/capacity, scratch
pool size, and all live array storage must be valid. Scratch exhaustion,
allocation failure, exceptions, unterminating/invalid growth, x87 status flags,
and unmasked floating exceptions are outside the claim. Callback mutations must
leave subsequent accesses valid. Numeric comparison assumes original masked
round-to-nearest floating behavior.
