# Actor vertical range and terrain distance

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The complete original instruction bytes are recorded in
[actor_terrain_probe.asm](actor_terrain_probe.asm). No original binary or assets
are stored in this repository.

| Routine | VA | PE file offset | Bytes | SHA-256 |
| --- | --- | --- | ---: | --- |
| Vertical range | `0x0042F6EC` | `0x2F6EC` (194284) | 156 | `a47aaf13325a019ad3242e30894400456f1a5a5a5cc459c9e0a5820fe700caee` |
| Terrain distance | `0x0042F788` | `0x2F788` (194440) | 194 | `5af155c4d6fdc639bd74190564f17b2b78e6c1f3654fd96aea231286b96adc8d` |

REA static evidence:

- Range: `ev_6063573c8e716a09d128e2d067ce746239ab2b30f7871bffc7e807c452c902cb`.
- Scan: `ev_601f5c74065e391c69c078c4d9491595a829ffca2b7b1922e5f4d4e1555bc492`.

## Static recovery

`actor_sample_y_in_world` uses the existing `0x42F22E` sample, so the Y tested
is the binary64 sum of actor position Y and `pair_360[1]`. It accepts both zero
and the exact grid height. The two x87 `TEST AH,1` branches reject NaN and
out-of-range values. Only AL is a defined boolean return; upper EAX bits are
not represented in the public API. The original temporary vector constructor
and destructor have no externally visible effect in this slice.

`actor_distance_to_terrain` copies raw actor position Y, truncates it with
`__ftol` (`0x49C5AC`), and uses the signed low 32 bits as a row. It scans while
the row is nonnegative and less than grid height. Each cell query truncates
raw position X and lets `0x40DE60` mask X to eight bits. The actor's sample
offset pairs and category are not used.

A row stops the scan only when its cell is nonzero, bit `0x200` is clear, and
bit `0x80` is clear. Original short-circuit query order is retained. Other
bits, including `0x1000`, do not make a cell passable. Each continuing iteration
subtracts actor `direction_44` with wrapping 32-bit arithmetic. The result is
raw starting Y minus the final signed row, returned directly in x87 extended
precision. It is neither an absolute distance nor multiplied by direction.
An empty downward scan can end at row -1; an upward scan can end at row height
and yield a negative result. Starting outside the row interval returns the
fractional difference from the initial truncated row without accessing cells.

The integer projection preserves masked-invalid conversion behavior: an
unrepresentable signed64 conversion yields the x87 indefinite integer, whose
low 32 bits are zero. Finite values outside signed32 are not clamped to signed32.

## Unit and native verification

`tests/test_actor_terrain_probe.cpp` passes on x64 at O2 and x86 at O0. It checks
inclusive sample bounds, offsets, NaN rejection, mask priorities, skipped rows,
upward/downward signs, zero direction with an immediate hit, out-of-bounds
starts and low-32-bit conversion of a large finite Y.

`tools/native_actor_terrain_probe_oracle.cpp` executes both unchanged routines
and the complete original helper chain with **no adapters**. Its Python runner
verifies executable identity, each code range and constants before streaming
them into the owned x86 process. The chain includes sampling, vector temporary
helpers, grid height/cell access, integer-array indexing and `__ftol`.

O0 and O2 each pass **21,536 vertical-range comparisons** and **21,792 distance
comparisons**. Distance results compare all 80 meaningful extended-precision
bits, with NaNs compared by classification rather than payload. Actor storage
is unchanged for both queries; cell storage is unchanged by scans. Cases
include infinities/NaNs, signed zero, tiny values, integer limits, large values
projecting into signed32, boundaries, both standard directions, larger steps,
signed subtraction wrap and all relevant cell masks. Range tests also cover
nonpositive and maximal signed heights without accessing cell storage.

## Limits

Distance scans require valid storage for every accessed row, masked floating
point exceptions and inputs that terminate. Direction zero with a passable
starting cell repeats forever in the original; the source deliberately adds
no fallback or iteration guard. Arbitrary directions can also cycle because
row subtraction wraps. Tests use only terminating inputs. Floating-point
status flags, signaling-NaN payload propagation and concurrent mutations are
not compared. These are isolated original-code comparisons, not a full-game
runtime test.
