# Phase 3 round 5 — what bash, dig, and mine remove

Static read of `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, image base
`0x400000`. Re-run with `python tools/harness/read_steppers.py --states` and
`--at` / `--consts` on the addresses below. No live process. Nothing here goes
into `Rules`.

The shared stepper does not clear a cell. It fires a function pointer when the
animation's integer percent equals a frame number stored on a record. Bash, dig,
and mine each register their own records in the constructor. The cell write, in
every bite that has one, is a store of dword 0 through the pointer from
`0x0040DE60`.

## Timing, shared by all three

Observation. Slot 1 of vtables `0x004B06AC` (bash), `0x004B09F0` (dig),
`0x004B0B68` (mine), and `0x004B0CC8` (unnamed) is `0x0041B077`. One direct
`E8` also reaches it, from `0x0041047B`, which is the tail of `0x00410302`.
That address is slot 1 of the table at `0x004B08DC`. That table is not one of
the 46 six-slot state vtables (`+0x14` is `0x00410486`, not `0x004011E0`). The
`build` animation site `0x0040FB83` sits in its enter hook `0x0040FAB6`. That
caller was not followed past the call.

`0x0041B077` samples the animation on the lemming's `+0x128`:

```
0041b0d1  e8 2e da 04 00        call   0x468b04          ; duration
0041b19b  e8 bd d9 04 00        call   0x468b5d          ; current
0041b1a3  dc 75 f0              fdiv   qword ptr [ebp-0x10]
0041b1a9  dd 05 e0 0c 4b 00     fld    qword ptr [0x4b0ce0]   ; 00 00 00 00 00 00 59 40 = 100.0
0041b1af  dc 4d cc              fmul   qword ptr [ebp-0x34]
0041b1b2  e8 f5 13 08 00        call   0x49c5ac          ; to int
```

`0x00468B04` returns `(int from 0x0045B8F0) * 0.06 / [anim+0x10]`. `0x00468B5D`
returns `(controller+0x1CC * 0.06 + controller+0x1C0) / [anim+0x10]`. The
`[anim+0x10]` term cancels in the ratio, so the integer is a percent of that
pair. The dword at `0x004BF4C8` is `64 00 00 00` (100). The stepper adds it
when the new percent is below the stored one (`0x0041B1D9`), then reduces both
with `idiv` at `0x0041B2A2` and `0x0041B2B2`.

The previous percent lives at state `+0x2C`, set to `-1` by `0x0041AFC3` (`c7
40 2c ff ff ff ff`) in the base constructor `0x0041AF70`, which all three
skill constructors call. Each tick walks every integer from that stored percent
through the new one. For each value it looks at the next record in the list at
state `+0x18`:

```
0041b21b  8b 42 08              mov    eax, dword ptr [edx+8]     ; record frame
0041b21e  3b 45 f8              cmp    eax, dword ptr [ebp-8]     ; cursor
0041b248  ff 50 0c              call   dword ptr [eax+0xc]        ; record function
```

The call is `record+0xC (state, lemming, record+0x10)`. A nonzero return stops
the rest of that tick (`0x0041B1F7`). Records are appended by `0x0040C860`,
which does not sort; the constructors below already append in increasing frame
order. The list index is state `+0x28`, also reset to 0 in `0x0041AF70`.

Hypothesis, not a tick count: how fast the percent advances is the animation
clock, not a constant in the stepper. `0x0046845C` does `fadd qword
[controller+0x1D8]` onto the `0.0625` at `0x004B1950` and passes the sum to
`0x00426DB0`. That callee was not read. Dig's enter is the only one of the
three that writes the rate: `0x004159CA` pushes `00 00 00 00` / `00 00 e8 3f`,
the double `0.75`, into `0x004683E9`, which stores it at controller `+0x1D8`.
Bash and mine never call `0x004683E9` from their enter hooks.

## What each skill registers

Frame numbers are the percent above. `shl eax, 2` / `shl ecx, 2` means the
constructor's loop index is multiplied by 4 before it is stored at record `+8`.
An immediate store is the percent itself.

### Bash — constructor `0x0040BB9E`, vtable written at `0x0040BBD0`

Loop index `i` runs `0 .. 0x17` (`0x0040BBFF` stores 0, `0x0040BC0B` adds 1,
`0x0040BC11` compares with `0x18`). Frame stored is `i * 4` (`0x0040BC6C` and
the same `shl` at every other store).

| frame | function | arg | what that call does |
| --- | --- | --- | --- |
| 4, 8 | `0x0040C7F6` | 0 | adds `lemming+0x40 * 1.0 / 13.0` to X |
| 12 | `0x0040C357` | 0 | the bite |
| 16 | `0x0040C357` | 1 | the bite |
| 32, 36, 40, 44, 48 | `0x0040C7F6` | 0 | the same X nudge |
| 60 | `0x0040C357` | 3 | the bite |
| 64 | `0x0040C357` | 2 | the bite |
| 72, 76, 80, 84, 88 | `0x0040C7F6` | 0 | the same X nudge |
| 92 | `0x0040C6E8` | 0 | commits X, may leave the skill |

Twelve nudges. `0x0040C806` multiplies by the `1.0` at `0x004B02D0` and
`0x0040C80C` divides by the `13.0` at `0x004B06D0`, then `0x004231DE` writes X.
`0x0040C6E8` adds `lemming+0x40` once to state `+0x48` (`0x0040C70F`), wraps it
through `0x0040DEC0`, and stores that integer back to `lemming+8`
(`0x0040C73C`). It then either calls `0x0042F8B9` and returns 1, or builds the
object from `0x00415FA0` and switches with `0x0042E696`, or increments state
`+0x44` while it is below 2 and clears state byte `+0x41`.

The bite is `0x0040C357`, which probes with `0x0040DE60` and calls `0x0040C0AF`.
The only store of a zero through a cell pointer in that helper is:

```
0040c16d  c7 01 00 00 00 00     mov    dword ptr [ecx], 0
```

`ecx` is the cell pointer argument. It is reached when the cell word is nonzero
and fails every refusal test below. The helper refuses the cell, and does not
write it, when any of these is set (`0x0040C0F8`, `0x0040C104`, `0x0040C110`):

- bit `0x2` (`kCellIndestructible`)
- bit `0x4`, or bit `0x8` when `lemming+0x40 == -1` (`0x0040C0DC` stores 4,
  `0x0040C0EC` stores 8)
- bit `0x800` (`kCellBlocker`)

A refused cell makes the helper return 1. A zeroed cell sets state byte `+0x41`
(`0x0040C166`).

Which cells. Observation of the loops, with the stride left as the lemming's
own fields:

- Y base is the position from `0x0042F22E`, plus `arg * lemming+0x44`, then
  `0x0040DF50` (`0x0040C3EB`).
- If state `+0x41` is clear and the current column differs from state `+0x4C`,
  it walks columns from the current X toward state `+0x4C`. The step is `+1`,
  or `-1` when `lemming+0x40 == -1` (`0x0040C44B`). For each column, `j` runs
  `0, 1, 2, 3` (`0x0040C4EC` `cmp dword ptr [ebp-0x48], 4`) and the probed Y is
  `base + j * lemming+0x44` (`0x0040C4FC`).
- After that walk, if no helper returned 1, it probes one more cell at the
  current column and the same Y base (`0x0040C5E2`).

State `+0x4C` is written only at enter, `0x0040BFE9`, from the lemming's X
through `0x0040DEC0`. The bash range `0x0040BB9E .. 0x0040C830` has no other
store to `+0x4C`. `0x0040DEC0` folds X into `[0, 256)` by the `256.0` at
`0x004B07A8`. `0x0040DE60` masks X with `0xFF` (`0x0040DE6A`) and clamps Y to
the height at the world object's `+0x418`. One added world unit is one cell.
`lemming+0x40` and `lemming+0x44` are not written by this constructor or this
enter hook. Their values at bash time were not read.

### Dig — constructor `0x0041584E`, vtable written at `0x00415880`

Two records, no loop. Frames are immediates.

| frame | bytes of the store | function | what it does |
| --- | --- | --- | --- |
| 25 | `0x004158E3` `c7 41 08 19 00 00 00` | `0x00415A43` | zeroes three cells |
| 30 | `0x00415941` `c7 40 08 1e 00 00 00` | `0x00415DD2` | steps Y, may leave the skill |

`0x00415A43` sets `i` to `-1` (`0x00415AB7` `c7 45 d4 ff ff ff ff`) and stops
when `i >= 2` (`0x00415AC9` `83 7d d4 02`). For `i` in `-1, 0, 1` it probes
`wrap(lemming+8 + i)` by `wrap(position - lemming+0x44)`. The position comes
from `0x0042F183`; the Y subtract is `0x00415AA3` `fsubr`. The only cell store
is:

```
00415b84  c7 02 00 00 00 00     mov    dword ptr [edx], 0
```

Three probes, so three cells in a row at one Y, the row one `lemming+0x44`
off the position object. It does not write the cell, and it calls `0x0042F8B9`
and returns 1, when the word has bit `0x2`, or bit `0x10`, or bit `0x20` when
`lemming+0x44 == -1` (`0x00415A86` stores `0x10`, `0x00415A96` stores `0x20`,
test at `0x00415B1B` and `0x00415B2A`).

`0x00415DD2` does not probe a cell. It writes `lemming+0x10 - lemming+0x44`,
after `0x0040DF50`, through `0x00423275`, which stores the double at `+0x10`.
If `0x0042E913` returns 0 it builds `0x00415FA0` and switches with `0x0042E696`,
and returns 1.

### Mine — constructor `0x0041846E`, vtable written at `0x004184A0`

Loop index `i` runs `0 .. 0x11` (`0x004184E1` compares with `0x12`). `i == 5`
registers nothing.

| frame | function | arg | what it does |
| --- | --- | --- | --- |
| 12 | `0x004188F6` | 3, then 2 | the bite, twice |
| 16 | `0x004188F6` | 1, then 0 | the bite, twice |
| 20 | `0x004188F6` | -1 | the bite |
| 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64 | `0x0041905F` | 0 | nudges X and Y |
| 68 | `0x004190F3` | 0 | commits the position, may leave the skill |

The 24..64 frames are `i * 4` for `i` in `6 .. 0x10` (`0x00418730` `shl ecx, 2`).
Frame 68 is the immediate `0x44` at `0x00418793`. `0x0041905F` adds
`lemming+0x40 * 1.0 / 12.0` to X and subtracts `lemming+0x44 * 1.0 / 12.0`
from Y. The `12.0` is at `0x004B0B80`. Eleven calls.

`0x004188F6` probes with `0x0040DE60`. The only cell store is:

```
00418b29  c7 02 00 00 00 00     mov    dword ptr [edx], 0
```

The dx loop starts at 1 (`0x004189F8`) and runs while `dx <=` the dword at
`0x004B10BC`. That dword is `02 00 00 00`, so dx is 1 and 2. X is
`base + dx * lemming+0x40`. Y is `base + arg * lemming+0x44`. When `arg == 2`
and `dx == 1` the iteration is skipped: `0x004B10B8` is dword 3, the code
subtracts 1 and compares with the arg (`0x00418A1E`), and subtracts 1 from the
dword 2 for the dx compare (`0x00418A2C`).

Per call, that is two cells (one, when arg is 2), at X offsets of one and two
times `lemming+0x40`, at a Y offset of `arg` times `lemming+0x44`. Five calls
per cycle, so nine probes before refusals: args 3, 2, 1, 0, -1 with the one
skipped probe. It does not write the cell when bit `0x2` is set, or bit `0x4`
(bit `0x8` if `lemming+0x40 == -1`) is set (`0x004189B0` / `0x004189C0`). Bit
`0x800` also skips the store and calls `0x0042F66B` (`0x00418AF9`). A stored
zero sets state byte `+0x30` (`0x00418B22`).

`0x004190F3` does not probe a cell. Its log strings are `0x004BF44C` "Mine
collision check\n" and `0x004BF464` "Lemming moved to %d, %d\n". It adds
`lemming+0x40` to state `+0x38` and subtracts `lemming+0x44` from state `+0x3C`,
writes those integers back to the lemming's X and Y, and may call `0x0042F8B9`
or switch to the object from `0x00415FA0`.

Same gap as bash: `lemming+0x40` and `lemming+0x44` are not written here. The
offsets above are multiples of those fields. They are not a new constant.

## Slot 3 is not the bite

These are the exit slots. No `E8` in `.text` targets any of them. The address
appears only as a vtable dword.

| skill | slot 3 | bytes | what it does | reference |
| --- | --- | --- | --- | --- |
| bash | `0x00401320` | `55 8b ec 51 89 4d fc 8b e5 5d c2 04 00` | saves `ecx`, returns | 23 vtables, first `0x004B0298`, bash's own at `0x004B06B8` |
| dig | `0x00415995` | `6a 00 / 6a 00` then `call 0x4683e9` | writes double `0.0` to the animation rate | only `0x004B09FC` |
| mine | `0x004188C3` | test of state bytes `+0x31`, `+0x30` | if `+0x31` is 0 and `+0x30` is nonzero, calls `0x004190F3` | only `0x004B0B74` |

Mine's exit can replay the frame-68 position commit. It still does not zero a
cell. Dig's exit undoes the `0.75` the enter hook wrote.

## `FUN_0041B683` has no caller in `.text`

Search, both on the file image:

- Little-endian dword `83 b6 41 00` in `.text`, `.rdata`, `.data`, `.rsrc`.
  One hit: `0x004B0D14`, which is vtable `0x004B0D08` plus `0x0C`. That vtable
  is the trampoline state (enter hook sets the string `trampoline`).
- Every `E8 rel32` in `.text`. Zero targets `0x0041B683`.

It is not on the bash, dig, or mine record lists. The bites above store a full
zero with `c7 0? 00 00 00 00`. `0x0041B783` `and 0xf77c` is a different
function and stays unassigned.

## Still unread

- The integers in `lemming+0x40` and `lemming+0x44` while a skill is running.
  Every X/Y offset in the bites is a multiple of those fields, and the enter
  hooks do not write them. Comparing `+0x40` to `-1` shows the code has a
  negative-facing case. It does not by itself prove the stored value is `±1`.
- `0x00426DB0`, the consumer of `0.0625 +` the rate at controller `+0x1D8`.
  Until that is read, `0.75` is the double dig stores, not a tick interval.
- The builder-side caller `0x00410302`, past the fact that it calls
  `0x0041B077`.
- Any live series. This session did not attach to the game.

`diggerDepth`, `basherReach`, and the three intervals stay as they are.
