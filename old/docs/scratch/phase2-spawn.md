# Phase 2 task 3 — spawn position

Static read of `Lemmings Revolution.exe`
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`, base `0x400000`.
Re-run the disassembly with `python tools/harness/read_steppers.py --at <addr> --len <n>`.
Live numbers, when present, come from `python tools/harness/measure_spawn.py`.

Units below are world units unless a field is explicitly an int16 from the file.
The engine's mask is 256 x 96, one cell per world unit. Nothing here is a classic Lemmings hatch.

## Observation: what the spawn writer stores

The release tick is vtable slot 5 of the hatch, `0x004B0F50` -> `0x00425A95`.
When the hatch's countdown double at `+0xC0` has reached 0 and the remaining-count
word at `+0x98` is still positive, `0x00425C30` calls the lemming constructor
`0x0042DE51`.

That constructor does not receive the entrance translation. The call site
sign-extends two int16 fields of the hatch and passes them as doubles, plus
the type dword at hatch `+0xB0`:

Address order. The two int16s are pushed as doubles right-to-left, so the
first argument the constructor sees is `+0x9A`:

```
00425bfa  8b 91 b0 00 00 00      mov   edx, [ecx+0xb0]
00425c00  52                     push  edx                      ; type, later [ebp+0x18]
00425c04  0f bf 88 9c 00 00 00   movsx ecx, word ptr [eax+0x9c]
00425c0e  db 45 e0               fild  dword ptr [ebp-0x20]
00425c14  dd 1c 24               fstp  qword ptr [esp]          ; double(int16 hatch+0x9C)
00425c1a  0f bf 82 9a 00 00 00   movsx eax, word ptr [edx+0x9a]
00425c24  db 45 dc               fild  dword ptr [ebp-0x24]
00425c2a  dd 1c 24               fstp  qword ptr [esp]          ; double(int16 hatch+0x9A)
00425c30  e8 1c 82 00 00         call  0x42de51
```

`0x0042DE51` builds a 3-double from those two values and the constant 0.5
(`push 0x3FE00000` / `push 0` at `0x0042DE6F`), copies it with `0x004017F0`
and `0x004066D0`, then calls `0x004230A2` (`0x0042DEA5`). The live reads
below are what assign the result: lemming `+0x08` is the `+0x9A` double,
`+0x10` is the `+0x9C` double, `+0x18` is 0.5. `0x004230A2` was not
re-disassembled as a `rep movsd` in the check of this note.

So at creation, before any stepper runs:

| lemming field | value written | role already read (PLAN.md 2.3) |
| --- | --- | --- |
| `+0x08` | `(double)` of signed int16 at hatch `+0x9A` | horizontal, the walk stepper's `fstp` at `0x0041BE95` |
| `+0x10` | `(double)` of signed int16 at hatch `+0x9C` | vertical, the fall stepper's `fsubr` at `0x00416219` |
| `+0x18` | `0.5` exactly | not the entrance Z; a constant in this constructor |

The release counter word at `0x004CBA78` (indexed by the type) is incremented
later, at `0x0042DFF1`, after that copy. The fall state is entered later
still (`0x0042E054` -> `0x0042E696`). A lemming pointer is published in the
vector at `0x004CBB18` (`+0x00` count, `+0x0C` pointer array) from inside
`0x004230A2`, which is also before the counter increment.

`0x0042DE51` then sets the lemming vtable to `0x004B10E8` (`0x0042DF1D`),
facing `+0x40 = 1` and vertical direction `+0x44 = 1` (`0x0042DFDD`,
`0x0042DFE7`).

A second constructor, `0x00426390`, tail-calls `0x0042DE51` with the same
three arguments and then overwrites the vtable with `0x004B0F6C`. It is not
the release-tick call. The release tick calls `0x0042DE51` directly.

## Observation: those int16s are not the entrance translation

The hatch loader is vtable slot 6, `0x00425E50`. It calls the base loader
`0x0043130E` -> `0x00432283`, which may construct the placed model
(`0x00465924`) and then virtually calls the model's slot `+0x08` with
`&hatch+0x08` (`0x00432302`). That is a different write from the spawn words.

After the base loader returns, `0x00425E50` reads the spawn words from the
level stream with `0x00423360` (read u16 via `0x0047DAC8`):

```
00425f0a  66 89 82 98 00 00 00   mov word ptr [edx+0x98], ax   ; remaining count
00425f1c  66 89 81 9a 00 00 00   mov word ptr [ecx+0x9a], ax   ; spawn X word
00425f2e  66 89 82 9c 00 00 00   mov word ptr [edx+0x9c], ax   ; spawn Y word
```

The only other write of those two words is the hatch constructor `0x004258C0`
zeroing them (`0x0042598A`, `0x00425996`) before the load. No instruction
copies an entrance double into `+0x9A` or `+0x9C`.

The parsed entrance (LevelObjects) is the `.a3d` Object 09 translation,
three doubles. The lemming is created from two int16s plus the constant 0.5.
That is already enough to say the writer does not store the entrance
object's `(tx, ty, tz)`.

Whether the two int16s are numerically equal to the unrolled entrance
(column, world Y) is a separate read. The live section below is that read.
The entrance-drop survey (81/102) is not used here and nothing was tuned.

## File side: parsed entrance translation

`python tools/harness/measure_spawn.py --entrances MM_CLIMB_002 MM_FLOAT MM_DIG_000`

Same rule as `LevelObjects.cpp` `poseAfterA3d`: Object 09 doubles [9],[10],[11].
Unrolled column is `atan2(-tx, tz) / 2pi * 256`, the same unroll as
`unrollCylinder`. World Y is the translation's Y, not a mask row.

| level | world (tx, ty, tz) | radius | unrolled column | world Y |
| --- | --- | --- | --- | --- |
| MM_CLIMB_002 | (-35.546876, 44.000000, 35.546874) | 50.271 | 32.0000 | 44 |
| MM_FLOAT | (-49.901756, 87.000000, -1.223663) | 49.917 | 64.9989 | 87 |
| MM_DIG_000 | (-43.909237, 81.000000, -23.483960) | 49.795 | 84.0101 | 81 |

MM_CLIMB_002's entrance Y is exactly 44. The walker billboard measured earlier
at y = 32.190 is 11.81 below that, which is the "12 cells above the floor"
note. These three Y values are integers already, so an int16 of 44, 87 or 81
would match the entrance Y even though the writer never loads the double.

## Live reads

`python tools/harness/goto_level.py MM_CLIMB_002` then
`python tools/harness/measure_spawn.py --watch`.
Loaded path `Compiled\MM_CLIMB_002.lvl`, pid 48160, ui_state 0.
Released counter was already 3 when the watch started, so this is not the
first lemming of the level. It is the birth the actor list at `0x004CBB18`
published while the watch was running (released went to 5 at the print; the
list insert is before the counter write, so the counter may already have
moved by the time Python prints).

Hatch `0x02755BD0`, vtable `0x004B0F50`:

| field | value | what it matches |
| --- | --- | --- |
| double `+0x08` | 32.0 | unrolled entrance column 32.0000 |
| double `+0x10` | 44.0 | entrance world Y 44 |
| double `+0x18` | 0.0 | not the entrance tz (35.55), not the spawn constant |
| word `+0x9A` | 32 | same column |
| word `+0x9C` | 42 | not the entrance Y. 44 − 42 = 2 |
| word `+0x98` | 6 | remaining count at that moment |
| dword `+0xB0` | 0 | type index |

Lemming `0x027854C0`, vtable `0x004B10E8`, first read of that birth:

| field | value |
| --- | --- |
| `+0x08` | 32.0 |
| `+0x10` | 41.93901062011719 |
| `+0x18` | 0.5 |

That first `+0x10` is not a pre-fall sample. It is 0.060989 below the int16
42, and the follow then falls smoothly at about 20 world units per second
(41.939 → 32.0 in 0.493 s) with `+0x08` held at 32 and `+0x18` held at 0.5.
At t = 0.503 s it is on the floor: `+0x10 = 32.0`, `+0x08 = 32.020237`,
`+0x18 = 0.520000`. The 32.0 landing is the sim position under the billboard
that was previously measured at y = 32.190 after release.

Observation, this level: the lemming's vertical field is the hatch word 42,
not the entrance world Y 44 and not the floor 32. The first sample had
already moved 0.061 off that word. Horizontal is the entrance column, 32,
exactly. `+0x18` is the constructor's 0.5, not an entrance coordinate.

One level does not make 42 = entrance − 2 a rule. The word is loaded from
the stream (`0x00425F2E`), not computed from the 44.

### MM_FLOAT

Same two commands, after a process restart. pid 40816.
Loaded `Compiled\MM_FLOAT.lvl`. Released was already 2; the watched birth
was the next one.

Hatch `0x026CD180`:

| field | value | what it matches |
| --- | --- | --- |
| double `+0x08` | 65.0 | unrolled column 64.9989, rounded |
| double `+0x10` | 87.0 | entrance world Y 87 |
| double `+0x18` | 0.0 | not tz |
| word `+0x9A` | 65 | same column as the double |
| word `+0x9C` | 85 | 87 − 2, not the entrance Y |
| word `+0x98` | 8 | remaining at that moment |
| dword `+0xB0` | 0 | type |

Lemming `0x02702C50`, vtable `0x004B10E8`, first read:

| field | value |
| --- | --- |
| `+0x08` | 65.0 |
| `+0x10` | 84.939453125 |
| `+0x18` | 0.5 |

0.060547 below the int16 85. Same shape as the climb sample (0.060989 below
42), so this is a few milliseconds of the 20 units/s fall, not a second
stored offset. `+0x18` is exactly 0.5 and `+0x08` is exactly the int16.
Landed at t = 0.207 s on `+0x10 = 81.0` (`+0x08` 65.020149, `+0x18` 0.520000).

### MM_DIG_000

Process restart, pid 32844. Loaded `Compiled\MM_DIG_000.lvl`.
Released was already 1; the watched birth is the next one.

Hatch `0x027563C0`:

| field | value | what it matches |
| --- | --- | --- |
| double `+0x08` | 84.0 | unrolled column 84.0101, rounded |
| double `+0x10` | 81.0 | entrance world Y 81 |
| double `+0x18` | 0.0 | not tz |
| word `+0x9A` | 84 | same column as the double |
| word `+0x9C` | 79 | 81 − 2, not the entrance Y |
| word `+0x98` | 18 | remaining at that moment. The file's initial count is 20 |
| dword `+0xB0` | 0 | type |

Lemming `0x027834C0`, vtable `0x004B10E8`, first read:

| field | value |
| --- | --- |
| `+0x08` | 84.0 |
| `+0x10` | 78.92462921142578 |
| `+0x18` | 0.4999997913837433 |

0.075371 below the int16 79. Larger than the 0.061 on the other two levels,
so it is not a stored fraction. `+0x08` is still the int16. Landed at
t = 0.208 s on `+0x10 = 75.0` (`+0x08` 83.976151, `+0x18` 0.47999978).

### The words are in the .LVL

Same three pairs, read back out of `lemmings.box` entry `COMPILED\<level>.LVL`.
Little-endian u16 count, u16 x, u16 y. The count is the level's total, not
the remaining count seen live.

| level | file offset of the count word | count, x, y |
| --- | --- | --- |
| MM_CLIMB_002 | `0xA0C5A` | 10, 32, 42 |
| MM_FLOAT | `0x13C197` | 10, 65, 85 |
| MM_DIG_000 | `0x130AE3` | 20, 84, 79 |

`0x00425F1C` / `0x00425F2E` store those x and y words. Nothing in the
constructor subtracts 2 from the entrance Y. The 2 is the difference between
two stored fields: the `.a3d` translation Y (which lands in the hatch double
`+0x10`) and this int16.

## Conclusion

A lemming does not start at the entrance object's position.

Observation. `0x0042DE51` writes three doubles onto the new lemming:
`(double)` of the signed int16 at hatch `+0x9A`, `(double)` of the signed
int16 at hatch `+0x9C`, and the constant `0.5`. On MM_CLIMB_002, MM_FLOAT
and MM_DIG_000 the horizontal word is the entrance column (32, 65, 84) and
the vertical word is the entrance world Y minus 2 (42, 85, 79). The entrance
world Y is a different field, the hatch double at `+0x10` (44, 87, 81),
copied from the placed model and not passed to the constructor.

The first live sample of each birth was already 0.061, 0.061 and 0.075 below
that int16. Hypothesis, not a second measurement of the store: that gap is
the 20 units/s fall integrator (`0x004B0A18`) running for a few milliseconds
before the read. It is not one value across the three births, and `+0x08`
was still the int16 and `+0x18` was still 0.5. The exact pre-integration
double was not read.

Not concluded. That every level's vertical word is the entrance Y minus 2 —
three levels and no subtract instruction. That the post-release billboard at
y = 32.190 is a spawn height — on MM_CLIMB_002 the sim field landed at 32.0.
That the constant 0.5 is an entrance coordinate. The 81/102 entrance-drop
survey was not used.
