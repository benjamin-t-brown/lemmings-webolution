# Phase 2 task 1 — the seven unread cell bits

Re-run: `python tools/harness/read_steppers.py --cell-flags`

Image: `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, base
`0x400000`. `.text` is the only executable section (va `0x401000`, file-backed
`0xaf000`). The sweep decoded `0xaeff9` bytes and skipped 7. Every previously
cited flag site (the `and`/`or`/`push` list in PLAN.md 1.6) was found at its
address, and the raw E8 scan of the probe, stamp and unstamp matched the sweep
with nothing left over.

The grid is 256 x 96, one cell per world unit. Nothing below is in mask rows.

## Table

| bit | address or untested | caller | observation vs hypothesis |
| --- | --- | --- | --- |
| `0x4` | untested on a cell | — | Observation. No `test`/`and`/`or`/`xor`/`bt` of this bit on a cell word. See the search below. |
| `0x8` | untested on a cell | — | Observation. `or ah, 8` at `0x0040E312` is the blocker bit `0x800`, not this one. |
| `0x20` | untested on a cell | — | Observation. Present in the grid data; no instruction tests it. |
| `0x40` | `0x00415386` `and eax, 0x40` (`83 e0 40`). Set at `0x0042092E` `or edx, 0x40` (`83 ca 40`) and `0x00420A0F` `or al, 0x40` (`0c 40`), both stored back | Step of vtable `0x004B09C0` (type id `0x2f`). Setter `0x0042081B`: caller not found | Observation: the step branches on the bit and attaches type `0x2e` or `0x22`. Not named beyond that. |
| `0x100` | `0x00415552` `and edx, 0x100` (`81 e2 00 01 00 00`) | Same step, the branch taken when `0x40` is clear | Observation. No instruction was found that sets or clears it on a cell. |
| `0x400` | `0x004275C1` `or edx, 0x403` (`81 ca 03 04 00 00`); clear `0x00427641` `and edx, 0xfbfc`; test `0x00461704` `and eax, 0x400` (`25 00 04 00 00`) | Paint callers `0x004271D5`, `0x00427509`, `0x0043B1F8`. Scan `0x004615E9` called from `0x0046226E` | Observation: painted and erased together with solid and indestructible. Not named as a terrain kind. |
| `0x8000` | `0x0041C154` `and edx, 0x8000` (`81 e2 00 80 00 00`) | Walk step `0x0041BDD4` calls `0x0041BFFC` at `0x0041BEE6` | Observation: the walk tests the cell and, when the bit is set, pushes `0x2b` and calls `0x0042ECC9`. No setter found. |

`0x4`, `0x8` and `0x20` stay in `kCellUnreadBits`. The other four are
`kCellFlag0x40`, `kCellFlag0x100`, `kCellFlag0x400` and `kCellFlag0x8000`.

## What was searched

`.text` only, linear capstone sweep resumed after the 7 undecodable bytes,
plus an independent backward decode of every imm32 pattern of each bit, of
`~bit`, and of `(~bit) & 0xFFFF`.

Instruction forms: `test`, `and`, `or`, `xor`, `push`, `bt`/`bts`/`btr`/`btc`
with an immediate; `and` clear-masks (high half `0xFFFF`, or a 16-bit value
that clears 1..6 bits and is not a `2^n-1` width); `mov reg, imm` followed
within 8 instructions by `test`/`and`/`or`/`xor` of that register; `shr reg,
<bit index>` followed within 5 by `test`/`and` of 1.

Also every direct call of the stamp `0x004311D0` (`or eax, [ebp+8]` at
`0x0043126C`) and the unstamp `0x0043127E` (`not` then `and` at `0x004312F6`).
Ten stamp calls and five unstamp calls. The pushed immediates are only `0x80`,
`0x200` and `0x2000`. `0x4000` is pushed at `0x00428D9A` in the same shape; the
self-check finds that push. None of the seven bits is among them.

Not searched, so a later pass could still find something: `test reg, reg` and
`cmp reg, 0` (a whole-word nonzero test matches every bit; the landing helper
at `0x0041A991` does this on the cell); `bt reg, reg`; shifts other than the
shr-then-test-1 pair; indirect calls. Width masks (`and reg, 0xff` and the
other `2^n-1` values) and `and esp/ebp` alignment were counted and not cited.

`or ah, imm8` is bits 8..15. `or ah, 8` at `0x0040E312` (`80 cc 08`), in the
blocker vtable `0x004B07B8` slot 2, stores the cell back. That sets `0x800`,
the bit `0x0040E1D5` clears. It is not bit `0x8`.

## The four cited bits

### `0x40` and `0x100`

Constructor `0x004151F0` writes vtable `0x004B09C0` and type id `0x2f` at
`+0x14`. It is called from `0x0042E02C` (`push 0x28`, allocate, construct,
then `0x0042E696` attaches the object). Slot 1 of that vtable is `0x0041527B`.

That step probes the cell (`call 0x0040DE60` at `0x00415379`) and:

```
00415386  83 e0 40           and eax, 0x40
00415389  85 c0              test eax, eax
0041538b  0f 84 be 01 00 00  je 0x41554f
```

When the bit is set, one branch attaches type `0x2e` (constructor `0x00406C20`,
vtable `0x004B04D0`, `+0x14 = 0x2e`) and the other attaches type `0x22`
(constructor `0x00408700`, vtable `0x004B0570`). Both go through `0x0042ECC9`
(already attached?) and `0x0042E696`. The `0x2e` / `0x22` split depends on a
loop over `0x00420A52`; that loop was not read further.

When `0x40` is clear:

```
00415552  81 e2 00 01 00 00  and edx, 0x100
00415558  85 d2              test edx, edx
0041555a  0f 84 cb 00 00 00  je 0x41562b
```

If `0x00415800` then returns nonzero, the branch constructs the object at
`0x00415710` (vtable `0x004B09D8`).

The cell is also tested by two predicates that return the bit in `al`:
`0x0042F46B` `and ecx, 0x40` and `0x0042F5B4` `and ecx, 0x100`, each after
`call 0x0040DE60` and a load of `[eax]`. No E8, E9, `0F 8x` conditional, or
file dword refers to `0x0042F3F4` or `0x0042F53D`.

The setter is in `0x0042081B`. After a probe and `and eax, 1` (the cell is
solid), a nonzero argument byte does `or edx, 0x40` / `or al, 0x40` and stores
the word back (`0x00420934`, `0x00420A14`); a zero argument byte stores 0
(`0x00420950`, `0x00420A30`). The same search found no reference to
`0x0042081B`.

No `or` or clear-mask of `0x100` on a cell was found. The loader at
`0x0043B640` copies the grid as a byte blob (`shl edx, 2` at `0x0043B680`,
then `call 0x0043B5B0`), so a bit can be present in the file without an
instruction that sets it.

Hypothesis, not used: nothing here says what terrain `0x40` or `0x100` is.

### `0x400`

`0x00427554` loops a rectangle and does:

```
004275c1  81 ca 03 04 00 00  or edx, 0x403
004275ca  89 10              mov [eax], edx
```

`0x403` is `0x1|0x2|0x400`. The mirror `0x004275D4` does `and edx, 0xfbfc`,
which clears exactly those three bits, and stores back. Direct callers of the
paint are `0x004271D5`, `0x00427509`, `0x0043B1F8`; of the erase,
`0x00427172`, `0x00427541`, `0x0043B2D4`. At `0x0043B1F8` the call happens
when the object's byte `+0x58` is nonzero; the object came from
`call 0x004131F0` on the global `0x004C8988`. The function that contains
`0x00427509` does the opposite with the same byte: zero paints, nonzero
erases. Those two gates were not reconciled.

`0x004615E9`, called from `0x0046226E` (function `0x00462264`, itself called
from `0x0043A4BD`, `0x0043B24C` and `0x00460A41`), scans a run of cells:

```
00461704  25 00 04 00 00  and eax, 0x400
00461709  85 c0           test eax, eax
0046170b  75 0e           jne 0x46171b     ; stop when 0x400 is set
```

The neighbouring loop at `0x00461843` does `and eax, 0x401` / `cmp eax, 0x401`
and continues only while solid and `0x400` are both set. `0x00461E1D` is the
same test, called from `0x00461618` inside `0x004615E9`.

This is why the corpus word `0x403` exists as an instruction, not only as a
count. The 29,654 figure was not re-counted here.

Hypothesis, not used: that the bit is an edge, a one-way, or steel. The scan
treats it differently from bit `0x1`, and that is the whole of the observation.

### `0x8000`

The walk step is `0x0041BDD4` (vtable `0x004B0D28` slot 1). At `0x0041BEE6` it
calls `0x0041BFFC`. That function probes, saves the cell, and tests it:

```
0041c104  e8 57 1d ff ff   call 0x40de60
0041c109  8b 08            mov ecx, [eax]
0041c10b  89 4d e8         mov [ebp-0x18], ecx
0041c147  e8 14 1d ff ff   call 0x40de60
0041c14c  8b 08            mov ecx, [eax]
0041c14e  89 4d f0         mov [ebp-0x10], ecx
0041c151  8b 55 e8         mov edx, [ebp-0x18]
0041c154  81 e2 00 80 00 00  and edx, 0x8000
0041c15a  85 d2            test edx, edx
0041c15c  0f 84 bf 01 00 00  je 0x41c321
0041c162  6a 2b            push 0x2b
0041c167  e8 5d 2b 01 00   call 0x42ecc9
```

The `and` uses `[ebp-0x18]`, the cell from the probe at `0x0041C104`. The probe
at `0x0041C147` writes `[ebp-0x10]` and is not the word tested.

`0x0041BEE6` is the only relative call, jump, or conditional into `0x0041BFFC`.
No `or` of `0x8000` onto a cell was found. `and reg, 0x7fff` (three sites, all
outside probe functions) is a 15-bit width mask, not a clear of this flag.

## The three left unread

`0x4`. Twelve `and reg, 4`, eight `or reg, 4`, twelve `test reg, 4`, 108
`push 4`, none of them in a function that calls `0x0040DE60` or called from
one. Two that look like flag tests are static bytes, not the grid:
`0x0041036F` `and ecx, 4` on `[0x004C55C8]`, and `0x00461512` `and edx, 4` on
`[0x004ECA78]`. A backward decode reported `test dword ptr [ebx], 4` at
`0x004288C4`; the sweep's instruction there is `mov dword ptr [0x004BF720], 4`.
The `test` is a misaligned decode of that immediate.

`0x8`. After shifting AH/BH/CH/DH immediates left by 8, no cell instruction
mentions bit `0x8`. `and ecx, 0xfffffff7` at `0x00413F68` is the arithmetic
sequence `neg` / `sbb` / `and` / `add 0xa`, not a cell mask.

`0x20`. Cell-adjacent hits were `push` of `0x20`, `0x28`, `0x30` and similar
sizes. The stamp does not receive it. `and reg, 0x20` at `0x00473110` and
`0x00473B62`, and `and ..., 0xffffffdf` at `0x0049F70E`, are outside probe
functions. The counts in the brief (absent on MM_CLIMB_002, 876 on MM_DIG_000,
3,024 on MM_FLOAT) were not re-measured; they are why the bit stays listed.

## Refused

`0x82` is still `0x80|0x02`. Three immediates equal to `0x82` sit at
`0x004A009A`, `0x004A1AC8`, `0x004A1AF3`, not on a cell. The "exactly 40 times"
pattern was not re-surveyed and is still not a new bit.

`0x21` as an immediate is three `push 0x21` (`0x004183FB`, `0x004501B0`,
`0x0045DF11`), not a stamp argument.

`push 0x18`, `push 0x28`, `push 0x48`, `push 0x748` and the rest of the small
integers inside probe functions are allocation sizes and effect-id checks.
They contain unread bits only as arithmetic. They were not promoted.

No name was taken from classic Lemmings, and none of `0x40`, `0x100`, `0x400`
or `0x8000` was given a terrain-kind name. The constants are the bit, and the
comment is the instruction.
