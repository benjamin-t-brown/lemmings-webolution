# The collision grid's two neighbours

Static read of `Lemmings Revolution.exe` (SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, image base
`0x400000`) and of all 102 `COMPILED\*.LVL` entries in `lemmings.box` /
`patch.box`. Re-run with `python tools/harness/read_grid_siblings.py`. No game
process. Capstone on the image; the Ghidra bridge handshake failed, so there
is no decompiler transcript behind this.

The grid walk is the one in `CollisionGrid.cpp`, continued one chunk past the
grid. It lands on the known grid offsets (MM_CLIMB_002 at 240011, MM_FLOAT at
259037, MM_DIG_000 at 240600), so the chunk after it is the chunk after the
grid, not a second fingerprint.

## The loader reads four containers, in file order

`FUN_00439FD3` is a thiscall. After the height `u32` (`call 0x43aa63` at
`0x0043A048`) and the `f64` (`call 0x402580`, then `call 0x43a4ca`), the next
adds and calls are:

| address | field | callee | what the callee reads |
| --- | --- | --- | --- |
| `0x0043A07C` | `+0x42C` | `FUN_0043B640` | `u32` count, then `count * 4` bytes |
| `0x0043A0A0` | `+0x43C` | `FUN_0043B9C0` | `u32` count, then that many 12-byte records |
| `0x0043A0C4` | `+0x41C` | `FUN_0043B640` | the collision grid |
| `0x0043A0E8` | `+0x664` | `FUN_0043B700` | `u32` count, then `count` bytes |
| `0x0043A103` | `+0x678` | `FUN_0043B5B0` | a fixed `0x1800` bytes, no count |

`FUN_0043B640` does `shl edx, 2` at `0x0043B680` (`c1 e2 02`) before
`call 0x43b5b0`. `FUN_0043B700` does not: at `0x0043B73D` it is `mov edx,
[ebp-4]` / `push edx` / `call 0x43b5b0`. Same container shape (`data` at
`[this+0xc]`), different element size.

That order is the file order after the `f64` 0.36. `+0x42C` is the array
*before* the records and the grid. The 12288-count chunk is the array
*after* the grid, and it is loaded into `+0x664`, not into `+0x42C`.

`FUN_0043B700` has one direct caller, `0x0043A0EE`. There is no later
length-prefixed bulk read in this function. The float layer the file note
expected is not a fourth array the loader skipped.

## `+0x42C` is the tube index, 49858 `u32`s

**Observation.** In all 102 levels the length-prefixed array between the
`f64` and the 12-byte records has count 49858. The payloads differ (102
distinct). On MM_CLIMB_002, 48961 of the words are `0xffffffff` and the other
897 values each occur once. MM_FLOAT is 48300 sentinels and 1558 unique
others; MM_DIG_000 is 49168 and 690. Across the corpus the sentinel count
runs from 47086 to 49353.

**Observation.** `FUN_0043A31D` indexes it and treats `0xffffffff` as empty:

```
0043a346  add ecx, 0x42c
0043a34c  call 0x404d10          ; pointer to the u32
0043a351  mov ecx, [eax]
0043a356  cmp dword ptr [ebp-4], -1
```

On a miss it allocates an id from the container at `+0x148` and stores it
back through `FUN_0043C960` (`0x0043A37C`, the only caller of that getter).

**Observation.** The index is `FUN_0043C930`:

```
0043c93a  imul eax, eax, 0x101     ; arg2 * 257
0043c943  add ecx, eax             ; arg1 + arg2 * 257
0043c948  lea eax, [edx + ecx*2]   ; arg3 + 2 * (arg1 + 257 * arg2)
```

`2 * 257 * 97 = 49858`. The domain that fills the array exactly is arg1 in
`0..256`, arg2 in `0..96`, arg3 in `0..1`. One call site, `0x004635BD`,
pushes a constant `1`, then `movsx` of a word at record `+2`, then `movsx`
of a word at record `+4`, with `ecx = 0x4C6A10`.

**Hypothesis, not promoted.** Those bounds are the cell grid plus one
(`256 + 1`, `96 + 1`) and a pair of slots. That fits a vertex id map. The
two words are not shown here to be x and y; only the arithmetic and the
call's two `movsx`s are read.

### Who reads `+0x42C` after load

Search: every `.text` instruction whose immediate or memory displacement is
`0x42C` (capstone, from the section start), plus every raw dword
`0x0000042C` in `.text` and `.rdata`. `.rdata` has none.

| site | what it calls | role |
| --- | --- | --- |
| `0x00439DD1` | `0x404F20` | constructor, `FUN_00439D60` |
| `0x00439F5F` | `0x404CF0` | destructor, `FUN_00439EDF` |
| `0x0043A07C` | `FUN_0043B640` | the load |
| `0x0043A346` | `0x404D10` | the lookup inside `FUN_0043A31D` |
| `0x0043C97F` | `0x404D10` | the slot pointer inside `FUN_0043C960` |
| `0x004ABB01`, `0x004ABBA1` | `0x404CF0` then `ret` | the destructor again, in the unwind thunks |

The raw dword also occurs as a `call rel32` (`0x00414950`, `0x00485EA0`) and
a `jcc rel32` (`0x0045A622`). Those are branch displacements, not the field.

`FUN_0043A31D` has 28 direct calls, all with `ecx = 0x4C6A10`, all inside
`FUN_00463100` (4) and `FUN_00463410` (24). Each of those has one caller,
`FUN_00462264` at `0x00462276` and `0x0046227E`. `FUN_00462264` has three
callers, each walking a container and calling it on an element:
`0x0043A4BD` (function `0x0043A459`, the `+0x208` slots the loader fills),
`0x0043B24C` (function `0x0043B150`), `0x00460A41` (function `0x004608BB`).

**Negative, with that criterion.** None of those functions is the collision
probe `FUN_0040DE60` or the walk / climb / fall / bash-dig-mine steppers.
`FUN_0040DE60` adds `0x41C` at `0x0040DEAB`. The direct-call chain above is
the whole static chain; indirect calls are outside it. Whether
`FUN_00462264` runs during a tick was not established.

The loader itself is reached from one site, `0x00443A0A`, with
`ecx = [ebp-0x36c] + 8`. The constructor's one site, `0x00440F06`, is the
same `+ 8`. The readers pass the global `0x4C6A10`. Those are the same
class. They are not shown here to be the same instance: nothing at the
loader call is `mov ecx, 0x4C6A10`.

## The 12288 array is `+0x664`, one byte per two columns

**Observation.** The `u32` immediately after the grid is 12288 in all 102
levels. `FUN_0043B700` reads that many *bytes* into `+0x664`. The payloads
differ (102 distinct). MM_CLIMB_002's bytes are `0..12` (13 values, 2907
nonzero). MM_FLOAT is the same range, 4233 nonzero. MM_DIG_000 is the same
range, 3572 nonzero. Corpus max byte: 12 on 86 levels, 11 on 4, 10 on 2,
24 on 10.

**Observation, and the float claim fails.** Read as `f32`, those 12288 bytes
are 3072 values, 0 NaNs, max abs `1.079e-31`, and 0 values in `1e-3 .. 1e6`,
on MM_CLIMB_002, MM_FLOAT and MM_DIG_000. They are small integers. The `f64`s
sit in the *next* chunk: the fixed `0x1800` bytes at `+0x678`. Record 0 is
`(152.0, 152.0, 152.0)` on all three levels; byte 1 selects
`(147.0, 147.0, 147.0)` on all three. `0x1800 / 0x18 = 256` records of 24
bytes, three `f64`s each. The constructor places that table with count
`0x100` and stride `0x18` (`0x00439E16`).

**Observation.** `FUN_00403F40` is the reader:

```
00403f4a  and eax, 0xff
00403f52  sar eax, 1                 ; (x & 0xff) / 2
00403f68  call 0x403fc0              ; index = (y << 8) >> 1 + x/2
00403f71  add ecx, 0x664             ; = y * 128 + x/2
00403f77  call 0x403fa0              ; data + index, no scale
00403f7e  mov cl, [eax]              ; one byte
00403f80  imul ecx, ecx, 0x18
00403f86  lea eax, [edx + ecx + 0x678]
```

`128 * 96 = 12288`. Half the width, full height, one byte per cell. Not
`256 x 48` and not a float. The byte is an index into the 24-byte records
at `+0x678`. `FUN_00403FA0` adds the index to `[container+0xc]` with no
shift, which is the byte-element check a second time.

### Who reads it

Same search, for `0x664`. `.rdata` has none. Real `add ecx, 0x664` sites:

| site | callee | role |
| --- | --- | --- |
| `0x00403F71` | `FUN_00403FA0` | the byte read, inside `FUN_00403F40` |
| `0x00439E07` | `0x43B6A0` | constructor |
| `0x00439F29` | `0x43B6E0` | destructor |
| `0x0043A0E8` | `FUN_0043B700` | the load |
| `0x004ABB2E`, `0x004ABBCE` | `0x43B6E0` then `ret` | destructor, unwind thunks |

`0x0046B818` and `0x0048B4F8` are `call rel32` whose displacement happens to
be `0x664`.

`FUN_00403F40` has 33 direct calls, every one `mov ecx, 0x4C6A10`. The first
instruction after 31 of them is `mov esi, eax` / `mov ecx, 6` / `rep movsd`:
they copy the 24-byte record. `FUN_00403820`, reached from the stepper at
`0x0040843A`, copies those 6 dwords to `+0x178`, then to `+0x110`, then
toward `+0xa8`.

Eleven of the containing functions are slots in 6-slot vtables whose last
two slots are the usual stubs `0x004011D0` and `0x004011E0`. Two of those
slots are steppers (`0x00408227` in vtable `0x004B0550`, `0x00416C49` in
vtable `0x004B0A78`). None of the containing functions is
`0x0040DE60`, `0x004137CA`, `0x004160BD`, `0x0041A6D5`, `0x0041B077` or
`0x0041BDD4`. The collision probe and the movement steppers do not read
this array. Other state-vtable functions do, and what they keep is the
24-byte record.

**Hypothesis, not promoted.** The triples are equal numbers in `0..255` on
the three levels sampled, and callers smear the 24 bytes across object
fields. That looks like a tint. Nothing here is a color API, so it stays a
triple of `f64`s selected by the byte.

## Refused

- `+0x42C` is not the 12288 array. The loader reads it first, through
  `FUN_0043B640`, and the file count there is 49858.
- The 12288 count is not a float count and not half the height. It is a
  byte count. The index is `y * 128 + (x & 0xff) / 2`.
- The tube's `0xffffffff` tail is the empty sentinel `FUN_0043A31D` tests.
  The non-sentinel words being unique ids is an observation on three levels;
  naming the array a vertex buffer is the hypothesis above.
- `0x4C6A10` is the `this` every reader passes. It is not shown to be the
  object `FUN_00439FD3` is called on.
- Indirect calls are unsearched. The field search is the displacement and
  the raw dword, which does not depend on a call instruction.
