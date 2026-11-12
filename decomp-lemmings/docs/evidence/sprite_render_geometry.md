# Sprite rectangle geometry and coordinate endpoints

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Original bytes, mapped file offsets and hashes are in the accompanying `.asm`;
REA metadata, pseudocode and evidence IDs are in the accompanying `_metadata.json`.
The runner checks whole-executable identity and all executed spans/constants.

## Recovered scope

| Routine | Bytes | Recovered behavior |
| --- | ---: | --- |
| `47A190` | 981 | Select current piece, assign coordinate endpoints, build scaled rectangle, submit. |
| `440C50` | 169 | Copy two endpoint pairs to four vertices, without floating arithmetic. |
| `47AB96` | 36 | Set selected texture ID, then invoke renderer `414920`. |
| `440C30` | 25 | Store texture ID at model +1A8. |
| `468D00` | 30 | Index a clip by original stride 98 hex. |
| `468CE0` | 27 | Index a frame by original stride 10 hex. |
| `47ABC0` | 27 | Index a piece by original stride 48 hex. |

The source is `sprite_render_geometry.hpp/.cpp`. It uses the actual runtime
sprite arrays from the recovered reader, keeps unknown model/vertex fields
explicit, and treats renderer submission as a callback. It performs no graphics
API operation and is not a playable renderer.

`47ABC0` callers identify this path: `47A190`, `47A565`, `47A75D`, and the
resource-wide loops `483F1C` / `483FD7`. The latter loops operate on texture IDs;
they are not geometry builders. The chosen routine has no virtual calls or
allocation in its geometry core.

## Field meaning supported by instructions

The model selects clip +1C8, frame +1CC and piece +1B8 from resource +1F8.
Its four vertex records begin at +08 with stride 68 hex. Each vertex contains
the two written position doubles at relative +18/+20, and two coordinate
double-bit fields at relative +28/+30. Other fields are preserved.

| Piece offset | Use in this path |
| --- | --- |
| +00..+0C | Not read; meaning remains unknown. |
| +10, +18 | Endpoints for the second vertex coordinate component. |
| +20, +28 | Endpoints for the first vertex coordinate component. |
| +30 | Signed horizontal origin/offset term. |
| +34 | Signed vertical origin/offset term, subtracted from input Y. |
| +38, +3C | Signed native width/height terms used for centering corrections. |
| +40 | Texture registration ID copied to model +1A8 before submission. |
| +44 | Not read; remains unknown. |

The width/height interpretation is also supported by the adjacent anchored
rectangle routine `47A75D`: its two X edges differ by the scaled +38 value,
and its Y edges differ by the scaled +3C value. These contextual branches are
statically recorded but are not implemented here. No piece field is identified
as color by this recovery.

Let A be the piece +28 bits, B its +20 bits, C its +18 bits, and D its +10 bits.
With model +1B0 zero, vertices 0..3 receive coordinate pairs:
`(A,C), (A,D), (B,D), (B,C)`. Any nonzero +1B0 exchanges C and D.
`440C50` copies integer words, so signaling-NaN payloads and signed zeros in
these coordinate fields remain bit-identical. The source calls them coordinate
endpoints rather than assigning unverified asset-authoring axis names.

The arithmetic branch uses uint32 subtraction of globals `58D53C - 58CB38`:

- zero: origin offsets use scale 1, centering terms use one-half, output width
  and height use scale 1;
- `FFFFFFFF`: origin offsets use one-half, centering terms use one-quarter,
  output width and height use one-half;
- every other value: origin offsets are shifted left one bit **in 32 bits**
  before signed conversion, centering terms use scale 1, output size doubles.

In particular, the third branch is not a general power-of-two zoom operation.
Large signed origin offsets wrap during that 32-bit shift.

Input X/Y plus scaled origin offsets are stored to binary64 before computing
edges. Remaining edge expressions use x87 extended intermediates and store
binary64 at the original points. Vertex 0/1 share the left X edge, 2/3 share
the right edge; 0/3 share the first Y edge and 1/2 the second. Coordinates are
written before texture ID, which is written before the single renderer call.
Callback changes are preserved after submission.

## Verification

**Static:** REA pseudocode and instruction reading establish all seven recovered
routines, including integer `SHL`, x87 stores, raw endpoint copies and call order.
The constants at `4B03F8` (2.0) and `4B02C8` (4.0) are pinned by exact bytes.

**Units:** x64 `-O2` and x86 `-O0` check rectangle coordinates, endpoint mapping,
nonzero swap flags, half scale, texture binding order and signed wrap after the
double-scale offset shift.

**Native:** x86 `-O0` and `-O2` each pass **30,000 complete original-chain
comparisons**. Only final renderer `414920` is adapted. Original piece indexing,
endpoint setter and texture ID assignment execute unchanged. The test compares
all model bytes after normalizing the resource pointer, captures all geometry,
coordinate bits and texture ID at submission, and checks callback mutations.

Cases exercise all 27 clip/frame/piece index combinations, zero and arbitrary
nonzero swap words, all three scale branches including wrapping global
subtraction, full-range integer offsets, random coordinate bits and finite
inputs over a wide exponent range. Initial cases add positive/negative zero,
subnormals, minimum normals, maximum finite values, infinities and signaling or
quiet NaNs. Finite numeric results and raw coordinate bits compare bitwise.
Numeric position NaNs compare by classification; their sign/payload is not
claimed. Unknown model/vertex fields compare byte-for-byte.

## Limits and next candidates

The caller must supply valid resource indices and live storage. Masked x87
exceptions and an extended-precision `long double` implementation are required
for the demonstrated arithmetic agreement. Graphics backend behavior, texture
acquisition inside `414920`, clipping, color conversion and final draw ordering
are not claimed as recovered by this module.

Evidence-only next candidates:

- `47A565`, 504 bytes: two different horizontal extents form a trapezoid,
  with a small mode-dependent vertical adjustment; same endpoint/bind helpers.
- `47A75D`, 1,081 bytes: native-size anchored rectangle with a second orientation
  flag at model +1B4 and renderer state calls at `470F29`.
- `414920`, 231 bytes: acquires texture through `414A10`, binds via `47261F`,
  applies draw state, prepares vertex-pointer order **0,3,2,1**, and forwards to
  `46D3C1`. That rendering/backend chain has not been ported here.

All three contextual routines have byte/evidence records, clearly marked
unported in the assembly ledger.

## Integration

Add `src/sprite_render_geometry.cpp`; unit target
`tests/test_sprite_render_geometry.cpp`; native target
`tools/native_sprite_render_geometry_oracle.cpp` linked at image base
`0x10000000`; runner `tools/run_native_sprite_render_geometry_oracle.py`.
The source uses runtime sprite headers but has no extra link dependencies.
