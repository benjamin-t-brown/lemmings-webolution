# Phase 3 round 6 — when a builder lays a brick

Static read of `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`.
`tools/harness/read_steppers.py` checks SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`
before it prints, and it printed. Base `0x400000`. The game was not
started. `PLAN.md`, `src/sim/World.h`, `src/sim/World.cpp`, and
`tools/harness/read_steppers.py` were not edited.

One cell is one world unit. Nothing below is multiplied by 2.

## Answer

The collision stamp runs when a 0–100 animation percentage equals **81**,
once per pass of that counter. It sets the solid bit on **two** cells in
the facing direction, at offsets 1 and 2, on the lemming's current row.
The count stored on the state starts at **12** and is decremented by the
later callback, at percentage **90**. The last decrement leaves for the
shrug state.

There is no tick interval in this read. The `16.0` at `0x004B08F8` is
applied once, on enter, and is not the gap between bricks. Where the
conversion to 200 ms ticks stops is at the end.

Provisional classic values, for comparison only: `builderBricks = 12`
matches the immediate that was read; `brickLength = 3` and
`builderInterval = 8` do not appear as the loop or as a tick count.

## What calls it

Observation. The builder state is the vtable at `0x004B08DC`, written by
its constructor:

```
0040f93f  c7 02 dc 08 4b 00     mov dword ptr [edx], 0x4b08dc
```

That constructor is `0x0040F8D1`. Its only direct caller is `0x0045B116`,
inside `0x0045AFEB`. The switch there is

```
0045b03e  83 bd 70 ff ff ff 08  cmp dword ptr [ebp-0x90], 8
0045b051  ff 24 95 70 b5 45 00  jmp dword ptr [edx*4 + 0x45b570]
```

and table entry `[4]` at `0x0045B580` is `0x0045B0F6`, which does
`push 0x470` / `call 0x0049C1A8` / `call 0x0040F8D1`. Who stores 4 into
the switched field `[this+0x28]` was not followed.

The six dwords at `0x004B08DC`:

| offset | address | role here |
| --- | --- | --- |
| +0x00 | `0x00412730` | not read |
| +0x04 | `0x00410302` | step |
| +0x08 | `0x0040FAB6` | holds the `build` animation site |
| +0x0C | `0x0040FBBF` | not read |
| +0x10 | `0x004011D0` | the common stub |
| +0x14 | `0x00410486` | not `0x004011E0` |

`read_steppers.py --states` reports `0x0040FB83` as inside a stepper
because this table fails its shape test: slot +0x14 is `0x00410486`, not
the usual `0x004011E0`. The site is in `0x0040FAB6`, the function stored
at vtable +8. On the 46 tables that pass the shape test, +8 is the enter
hook. The brick is not laid there.

The step at `0x00410302` ends by calling the shared staggered-action
stepper:

```
0041047b  e8 f7 ab 00 00        call 0x41b077
```

`0x0041B077` turns the animation clock into an integer percentage and
walks it. The modulus is the dword at `0x004BF4C8`, value 100
(`64 00 00 00`). The debug string at `0x004BF564` is
`Staggered action new frame %d`. When the cursor equals the node's
`[+0x08]`, it calls the node's `[+0x0C]`:

```
0041b21b  8b 42 08              mov eax, dword ptr [edx+8]
0041b21e  3b 45 f8              cmp eax, dword ptr [ebp-8]
0041b248  ff 50 0c              call dword ptr [eax+0xc]
```

The constructor registers two nodes, through `0x0040C860` into the list
at `state+0x18`:

```
0040f9a8  c7 40 08 51 00 00 00  mov dword ptr [eax+8], 0x51
0040f9b2  c7 41 0c 1c 01 41 00  mov dword ptr [ecx+0xc], 0x41011c

0040fa06  c7 42 08 5a 00 00 00  mov dword ptr [edx+8], 0x5a
0040fa10  c7 40 0c cc fb 40 00  mov dword ptr [eax+0xc], 0x40fbcc
```

`0x51` is 81 and `0x5A` is 90. `0x0041011C` has no `E8` caller and no
other dword site. It runs when the percentage cursor hits 81.
`0x0040FBCC` runs when the cursor hits 90. The cursor walks upward, so
81 happens before 90 in the same pass. Both are once per 0–100 cycle.

## Which cells

Observation. `0x0041011C` is the solid-bit stamp. It asks `0x0042F22E`
for a position, truncates it (`0x0049C5AC`), wraps X through `0x0040DEC0`
and clamps Y through `0x0040DF50`, then loops:

```
00410199  c7 45 f0 01 00 00 00  mov dword ptr [ebp-0x10], 1
004101ab  83 7d f0 03           cmp dword ptr [ebp-0x10], 3
004101af  7d 51                 jge 0x410202
004101b7  0f af 42 40           imul eax, dword ptr [edx+0x40]
004101be  03 c8                 add ecx, eax
004101eb  e8 70 dc ff ff        call 0x40de60
004101f8  83 ca 01              or edx, 1
004101fe  89 10                 mov dword ptr [eax], edx
```

The index runs 1 and 2. That is two cells, not three. Column is
`truncX + i * facing`, row is `truncY`, and the cell dword is `or`ed
with 1. Facing is the lemming's `[+0x40]`. `0x0040DE60` is the cell
probe already named in `read_steppers.py`. The known solid bit is that
`or 1`.

`0x0042F22E` does not return the raw doubles. It copies out
`X + [+0x360]` and `Y + [+0x368]` (`fld` / `fadd` at `0x0042F263` and
`0x0042F275`, then `rep movsd` of 6 dwords at `0x0042F29B`). The init
beside spawn writes both offsets to 0.0:

```
0042e109  c7 82 60 03 00 00 00 00 00 00  mov dword ptr [edx+0x360], 0
0042e113  c7 82 64 03 00 00 00 00 00 00  mov dword ptr [edx+0x364], 0
0042e120  c7 80 68 03 00 00 00 00 00 00  mov dword ptr [eax+0x368], 0
0042e12a  c7 80 6c 03 00 00 00 00 00 00  mov dword ptr [eax+0x36c], 0
```

So until something else writes them, the two cells are
`(x+facing, y)` and `(x+2*facing, y)` in world units, with `x` and `y`
the truncated wrapped/clamped lemming position.

After the loop, `0x0041011C` calls `0x0043B016` with the same two columns
and the same Y. That calls `0x0043AE6C`. From `0x0043AE6C` to its `ret`
at `0x0043B013` there is no call to `0x0040DE60` and no `or ..., 1`. It
calls `0x0040BAA0` (the height function) and indexes the way the mesh
update does. It is not a second solid-bit write.

On enter, `0x0040FAB6` snaps the lemming onto integer coordinates before
any brick. If `[+0x40] == -1` it adds 1 to the truncated X
(`0x0040FB01` `cmp dword ptr [edx+0x40], -1`, `0x0040FB0B` `add ecx, eax`),
then writes X back through `0x0040DEC0` / `0x004231DE` and Y through
`0x0040DF50` / `0x00423275`. A left-facing builder's stored X is one
greater than the truncation of the X it entered with.

The percentage-90 function `0x0040FBCC` does not stamp cells. It moves
the lemming, which is where the next brick's pair sits. First, always:

```
0040fbf6  dd 40 10              fld qword ptr [eax+0x10]
0040fbfe  ...
0040fc01  03 41 44              add eax, dword ptr [ecx+0x44]
0040fc1b  e8 30 e3 ff ff        call 0x40df50
0040fc29  e8 47 36 01 00        call 0x423275
```

Y becomes `clamp(trunc(Y) + [+0x44])`. Spawn's write of `[+0x44] = 1` is
the one already recorded at `0x0042DFE7`; this round did not re-read
those bytes, and the builder functions read here do not write `[+0x44]`.

Then it probes. The continuing path (the byte flag at `[ebp-0x68]` is 0,
and the cell at `(truncX+facing, truncY)` is 0) adds `facing` to X
(`0x0040FF2A` `fild [ecx+0x40]`, `fadd` X, wrap, `0x004231DE`) and
decrements the counter. The flag is the OR of the byte results of
`0x00427654`. What that predicate tests was not read. If the flag is 0
and that cell is nonzero, it calls `0x0042F66B` and `0x0042F8B9` and
returns 1, and the counter is not decremented. The percentage-81 stamp
has already run in that pass.

`0x0042F66B` is the horizontal turn: `imul ecx, ecx, -1` on `[+0x40]`
at `0x0042F678`. When the new facing is -1 it writes -1.0 to `[+0x360]`
(`0x0042F6A4`, high dword `0xBFF00000`); otherwise it writes 0.0
(`0x0042F6D4`). That is the later writer of the X offset the stamp adds.
`0x0042F8B9` allocates `0x20` bytes; the state it builds was not read.

Hypothesis, not followed: a normal brick is a one-cell step up and a
one-cell step forward, so the next stamp's two cells start one column
over and one world unit up from this one. That depends on `[+0x44]`
still being 1, which this round did not re-read.

## The count

Observation. The constructor stores 12:

```
0040f948  c7 40 3c 0c 00 00 00  mov dword ptr [eax+0x3c], 0xc
```

The percentage-90 callback is the reader. On the paths that reach
`0x0040FF24`:

```
0040ff54  83 7a 3c 04           cmp dword ptr [edx+0x3c], 4
0040ff58  7d 1e                 jge 0x40ff78
              ; count is 3, 2, or 1: call 0x406a00, then 0x44535b
0040ff7e  8b 42 3c              mov eax, dword ptr [edx+0x3c]
0040ff8a  83 ea 01              sub edx, 1
0040ff93  89 51 3c              mov dword ptr [ecx+0x3c], edx
0040ff96  83 f8 01              cmp eax, 1
```

The value before the subtract is compared with 1. Starting from 12, the
twelfth time this path runs the old value is 1, the stored value becomes
0, and it constructs `0x0041AB30` and hands it to `0x0042E696` (the
state transition already named in PLAN.md). `0x0041AB30` writes the
vtable `0x004B0C78`:

```
0041ab42  c7 00 78 0c 4b 00     mov dword ptr [eax], 0x4b0c78
```

`--states` names that table `shrug` from its enter hook. The sound call
when the stored count is still below 4 is the three passes whose old
values are 3, 2, and 1.

The same constructor also writes `[state+0x14] = 4`
(`0x0040F952`, `c7 41 14 04 00 00 00`). Nothing read here consumes that
4 as a length. It is not the brick.

It also calls `0x004011A0` twice with count `0x0C` and stride `0x18`, on
`state+0x230` and `state+0x350` (`0x0040F912`, `0x0040F929`).
`0x004011A0` calls `[ebp+0x14]` (`0x00401B80`) once per element. The
element constructor was not read. The 12 that matters is the counter at
`[+0x3c]`, because that one is decremented and tested.

The paths that return before `0x0040FF7E` do not decrement. A pass can
stamp two cells at percentage 81 and then skip the decrement at 90.

## The interval, and the stop

Observation, as far as the clock was followed. The percentage is

```
0041b0d1  e8 2e da 04 00        call 0x468b04     ; -> [ebp-0x10]
0041b19b  e8 bd d9 04 00        call 0x468b5d
0041b1a3  dc 75 f0              fdiv qword ptr [ebp-0x10]
0041b1a9  dd 05 e0 0c 4b 00     fld qword ptr [0x4b0ce0]   ; 100.0
0041b1af  dc 4d cc              fmul qword ptr [ebp-0x34]
0041b1b2  e8 f5 13 08 00        call 0x49c5ac               ; truncate
```

`0x00468B04` returns `(frameCount * 0.06) / rate`. `0x00468B5D` returns
`(frameIndex * 0.06 + fraction) / rate`. `frameCount` is the first dword
of the animation record (`0x0045B8F0` returns `[ecx]`). `rate` is the
double at that record's `+0x10`. The record is `base + index * 0x98`
from `0x00468D00`. The rate cancels, so the percentage is the frame
index over the frame count, times 100. The `0.06` is the qword at
`0x004B1940`.

The frame index lives at `controller+0x1CC`. `0x00468833` adds
`argument * rate` to the accumulator at `controller+0x1C0`, and while
the accumulator is greater than `0.06` it increments the frame and
subtracts `0.06` (`0x004688C0`, `0x004688E0`, `0x00468972`).

That argument is the time double the state step was given. The builder
step passes it to `0x0041B077`, which passes it to
`[lemming+0x78]` slot `+0x2C` (`0x0041B18F`). That slot is `0x0043CF30`,
the dword at `0x004B14D4`. `0x0043CF30` adds `0xB0` to its `this` and
calls `0x00468833`. `lemming+0x78+0xB0` is `lemming+0x128`, the
controller.

The `16.0` is not on that path. The enter function loads it once:

```
0040fb99  e8 66 8f 05 00        call 0x468b04
0040fb9e  dc 35 f8 08 4b 00     fdiv qword ptr [0x4b08f8]
```

`0x004B08F8` is `00 00 00 00 00 00 30 40` = 16.0. The quotient is passed
to the same `[+0x2C]` method (`0x0040FBB6`). Substituting the formula
above, that one call advances the accumulator by
`frameCount * 0.06 / 16`, which is `frameCount/16` frames. It is an
enter-time seek. It is not a per-tick divisor and it is not a brick
count.

Stop. A tick count needs three things this round did not read:

1. The build strip's frame count and rate. They are the animation
   record's first dword and its `+0x10`, filled from the sprite sheet,
   not immediates in the builder.
2. The double the caller of vtable `+0x04` stores for one step. The
   builder consumes whatever it is given. It was not shown to be the
   measured 0.2 s rather than a sub-step.
3. Whether the enter seek of `frameCount/16` frames itself crosses 81
   and 90 before the first tick. That depends on (1).

The step function also keeps a window of `2.0 * 0.06` and `13.0 * 0.06`
(`0x004B03F8`, `0x004B06D0`, both multiplied by `0x004B1940` and stored
at `0x004C55C0` and `0x004C55B0`). Outside it, the step sets
`[state+0x228] = 1` and skips `0x004105B0`. Inside it, the first return
calls `0x00410E78` once. `0x004105B0` has no call to `0x0040DE60` and no
`or 1`; it is not the stamp. A third product, `7.0 * 0.06` from
`0x004B0900`, is stored at `0x004C55B8` (`0x00410363`) and that address
has no other immediate reference in `.text`.

`0x00410C0E`, called at the start of enter, loads a 16-byte-strided table
at `0x004B084C + i*16` for `i = 0..4` (`0x00410C9C`). Those dwords feed
`0x004135A0`. They are not the collision loop. The unreferenced dword
`0x0C` at `0x004B0848` was not used as the brick count; the count is the
immediate at `0x0040F948`.
