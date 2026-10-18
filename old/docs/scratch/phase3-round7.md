# Phase 3 round 7 — bomber fuse and Ohno

Static read of `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`.
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Base `0x400000`. The game was not started. `PLAN.md`, `src/sim/World.h`,
`src/sim/World.cpp`, and `tools/harness/read_steppers.py` were not edited.
`Rules` was not changed. Explosion radius was not in these functions and
was not read.

One tick below is the measured 0.2 s step (5 Hz). Nothing is multiplied
by the old two-rows-per-cell factor.

## Answer

The skill-bomb fuse is **5.0 seconds**. The integer `5` is pushed by the
bomb factory and `fild`'d into the countdown object's double. The stepper
subtracts the seconds time-slice and finishes at `<= 0.0`. There is no
stored tick count of 25.

Ohno is not cued when one second remains. Enter schedules an event at
**5000 ms** after the countdown object is entered (`trunc(5.0) * 1000`
added to the event clock). The scheduler, on the frame whose millisecond
delta covers that key, steps physics by `remaining_ms / 1000.0` and then
runs the event. The event plays the registered `Ohno` sample and installs
the flapper, whose enter starts the `explode` animation.

At a steady 200 ms step, 5000 / 200 = **25 ticks** after enter, on the
tick the 5.0 s interval elapses. The 25 is that division, not an
immediate in the exe. Our sim cues OhNo when `fuse == ticksPerSecond`
(one second left). That does not match this read.

The play itself is skipped while the byte at `0x004CBB40` is nonzero.
That byte is `0xFF` in the image, and no instruction in `.text` stores
that address. If it is still nonzero when the event fires, this site
does not play Ohno. The schedule and the flapper install still run.

## What calls it

Observation. Skill names at `0x004C0694` are climb, float, bomb, block,
build, bash, mine, dig. Bomb is index 2. `0x0045AFEB` switches on
`[object+0x28]` through the table at `0x0045B570`. Entry `[2]` is
`0x0045B197`:

```
0045b1b1  6a 00                 push 0
0045b1b3  6a 05                 push 5
0045b1b5  8b 4d c8              mov ecx, dword ptr [ebp - 0x38]
0045b1b8  e8 83 8b fb ff        call 0x413d40
```

`0x00413D40` writes vtable `0x004B0990`. The six dwords:

| offset | address | role here |
| --- | --- | --- |
| +0x00 | `0x00414310` | destructor |
| +0x04 | `0x00413F15` | step: subtracts the seconds slice |
| +0x08 | `0x00413DF9` | enter: schedules the 5000 ms event |
| +0x0C | `0x00413EC8` | exit, not read |
| +0x10 | `0x004011D0` | the common stub |
| +0x14 | `0x004140BF` | query for type `0x1F`, not the common stub |

`read_steppers.py --states` will not treat +8 as enter on this table,
because +0x14 is `0x004140BF` rather than `0x004011E0`. The schedule is
in `0x00413DF9` anyway.

The event helper's vtable is `0x004B09A8` (only two code slots; the next
qword is a double). Slot +4 is `0x00414370`. The scheduler calls
`0x0043F008` at `0x0045DC1D`, after the physics substep, and that walks
due events into the helper.

The flapper vtable is `0x004B0A58`. Enter `0x0041695C` is the site that
pushes `explode`.

## Fuse length

Observation. The constructor stores the second argument at `+0x18` and
the first at `+0x1C`, then loads the first as a double at `+8`:

```
00413d5e  89 51 18              mov dword ptr [ecx + 0x18], edx
00413d67  89 48 1c              mov dword ptr [eax + 0x1c], ecx
00413d74  db 45 08              fild dword ptr [ebp + 8]
00413d7a  dd 58 08              fstp qword ptr [eax + 8]
00413d80  c7 41 14 1f 00 00 00  mov dword ptr [ecx + 0x14], 0x1f
00413d8a  c6 42 11 01           mov byte ptr [edx + 0x11], 1
```

For the bomb factory the double is 5.0, `+0x18` is 0, and the type tag
is `0x1F`. The action-name string at `0x004C08D8` is `COUNTDOWN`.

The step at `0x00413F15` subtracts the passed time-slice double:

```
00413f50  dd 40 08              fld qword ptr [eax + 8]
00413f53  dc 21                 fsub qword ptr [ecx]
00413f58  dd 5a 08              fstp qword ptr [edx + 8]
```

and finishes when that double is `<= 0.0`. The compared constant at
`0x004B0338` is 0.0. `test ah, 0x41` / `je` returns 0 while the double
is still greater than 0, and 1 otherwise:

```
0041407d  dd 41 08              fld qword ptr [ecx + 8]
00414080  dc 1d 38 03 4b 00     fcomp qword ptr [0x4b0338]
00414086  df e0                 fnstsw ax
00414088  f6 c4 41              test ah, 0x41
0041408b  74 07                 je 0x414094
0041408d  b8 01 00 00 00        mov eax, 1
```

The slice those milliseconds become is seconds. On the frame that
covers the event, `0x0045DBD0` does `fild` of the remaining milliseconds
and divides by the double 1000.0 at `0x004B1758`, then calls
`0x004423D2`:

```
0045dbd0  db 45 b8              fild dword ptr [ebp - 0x48]
0045dbd3  dc 35 58 17 4b 00     fdiv qword ptr [0x4b1758]
0045dbdc  dd 1c 24              fstp qword ptr [esp]
0045dbe5  e8 e8 47 fe ff        call 0x4423d2
```

Derived, not a stored immediate: 5.0 s at the measured 0.2 s tick is
25 ticks. The exe holds the 5 and the 1000, not a 25.

The on-screen digit is `trunc(remaining + 1.0)` via the 1.0 at
`0x004B02D0` and `0x0049C5AC`. A displayed 6, which is exactly 5.0, is
wiped (`cmp dword ptr [ebp-0x10], 6` / `mov byte ptr [ebp-0x18], 0`
at `0x0041401A`). The player then sees 5, 4, 3, 2, 1 while the double
sits in `[4,5)`, `[3,4)`, `[2,3)`, `[1,2)`, `[0,1)`. Ohno is not tied
to the digit becoming 1. The format string is `%d` at `0x004BF34C`;
the calls after it (`0x004151D0`, `0x004404FA`) are the text, not a
sound.

Four other direct callers of `0x00413D40` also push 5 as the first
argument (`0x0041A5A6`, `0x0041ACF0`, `0x0041CCE5`, `0x004426C1`) and
a different second argument (1, 4, 2, and 5). Same 5.0 double, different
`+0x18`, so a different expiry case than the skill bomb. Not followed
further.

## When Ohno plays

Observation. Enter truncates the 5.0 double and multiplies by the
dword stored in the timer object at `[0x004CBF80]`. That constructor,
`0x00465790`, writes 1000:

```
004657b2  c7 02 e8 03 00 00     mov dword ptr [edx], 0x3e8
```

`0x3E8` is 1000. Enter then adds the event clock at `[0x004CBF88]`,
which is the game object `0x004C6A08` plus `0x5580`:

```
00413e4b  dd 42 08              fld qword ptr [edx + 8]
00413e4e  e8 59 87 08 00        call 0x49c5ac
00413e53  8b 0d 80 bf 4c 00     mov ecx, dword ptr [0x4cbf80]
00413e59  8b 11                 mov edx, dword ptr [ecx]
00413e5b  0f af d0              imul edx, eax
00413e5e  a1 88 bf 4c 00        mov eax, dword ptr [0x4cbf88]
00413e63  03 c2                 add eax, edx
```

Key = clock + 5 * 1000 = clock + 5000. The helper is inserted on the
list at `0x004CBF8C` by `0x0043F0C0`. Enter also copies `+0x18` to the
helper's `+0x10`. For a skill bomb that copy is 0.

The scheduler compares the frame's millisecond delta with
`key - clock` (`cmp` at `0x0045DB93`, `jl` to skip at `0x0045DB96`).
When the delta covers the key it advances the clock by the remainder,
steps physics by `remainder / 1000` seconds, then calls `0x0043F008`.
The helper step is `0x00414370`. Case `+0x10 == 0` constructs the
flapper with `0x00416910` and, only if the guard byte is 0, plays
the sample:

```
00414420  8a 15 40 bb 4c 00     mov dl, byte ptr [0x4cbb40]
00414426  85 d2                 test edx, edx
00414428  75 1e                 jne 0x414448
0041442c  a1 cc 32 4e 00        mov eax, dword ptr [0x4e32cc]
00414435  e8 c6 25 ff ff        call 0x406a00
00414443  e8 13 0f 03 00        call 0x44535b
```

`0x004E32CC` is `0x004C6A08 + 0x1C8C4`. Registration at `0x0045E5B8`
pushes the string `Ohno` at `0x004C0CAC` and stores the id at
`[ecx+0x1C8C4]` (`0x0045E5CA`). `0x004E32CC` itself is past the raw
file; the address identity is the evidence, not a file byte. The
play wrapper `0x0044535B` calls `0x00404CC0` with that id. After the
sound (or the skip) the same function installs the flapper through
`0x0042E696`.

Flapper enter starts `explode` unless `[lemming+0x4C] == 3`:

```
00416985  83 7a 4c 03           cmp dword ptr [edx + 0x4c], 3
00416989  74 0d                 je 0x416998
0041698b  68 e0 f3 4b 00        push 0x4bf3e0
00416993  e8 8d 93 01 00        call 0x42fd25
```

`0x004BF3E0` is the string `explode`. The `printf` just above it uses
`Added Flapper to Lemming %d\n at %d` at `0x004BF3BC` and is not a
sound. So the explode animation starts when the 5000 ms event runs,
not when the skill is assigned and not when the digit shows 1.

Derived: a steady 200 ms frame reaches a key 5000 ms ahead on the
25th such frame. The live delta comes from the timer (`0x004657D1`
calls through `[0x004B0220]`), so a hitch can make one frame cover
the key early. The substep passed to physics is still exactly the
remaining milliseconds divided by 1000.

## Guard byte

Observation. The image byte at `0x004CBB40` is `0xFF`. The two `.text`
references are reads: `0x00414420` (this play) and `0x0043B38D` (a
different function, which substitutes a double when the byte is
nonzero). No store of that address was found. Hypothesis: it is a
mute or disable flag. It was not traced to a writer. If it stays
`0xFF`, `0x00414370` still builds and installs the flapper, and it
does not play Ohno.

A second play of the same id is at `0x0042E349`, inside `0x0042E2BF`,
when `[lemming+0x5D]` is nonzero. `+0x5D` is set on other lemmings
from `0x004264F2` (animation `punch%d` at `0x004BF6C8`) and from
`0x00427266`. That is a nearby-lemming reaction, not the bomber's
own scheduled cue.

## Not closed

Hypothesis, not required for the 5.0 s or the 5000 ms. The countdown
constructor sets `[this+0x11] = 1`, and `0x0042E696` puts an object
with that byte set onto the lemming's list at `+0x374` rather than
replacing `+0x340`. The virtual step `0x00413F15` is called from
`0x0042E2BF`, whose only direct caller is `0x004265D1` inside
`0x00426414`. That function is not called directly; a pointer to it
sits at `0x004B0F80`. `0x0042E2BF` walks the `+0x374` list on the
branch where the passed time double is `<= 0`. The path from the
scheduler's `0x004423D2` down to `0x00413F15` was not traced, so this
read does not prove the double is decremented on every 0.2 s tick.
The Ohno event does not depend on that call. It is armed in enter
and fired from the millisecond list.

Vtable `0x004B0A38` is a different object. Its enter writes a double
5.0 at `+0x18` and its step subtracts dt until `<= 0`. Its type tag
is `0x10`, and the action-name id `0x10` is the string `FASTER` at
the name map, not `EXPLODE` (`EXPLODE` is id 2, string `0x004C0888`).
It is not the bomber fuse.

Another `explode` enter, `0x0040123C` on vtable `0x004B028C`, sets
the animation and does not count down. Its constructor `0x004011F0`
has no direct caller. Not the fuse.

## Radius

Not read. The countdown step, the Ohno helper, and the flapper enter
do not load an explosion radius. `Rules::explosionRadius` stays the
provisional 5.
