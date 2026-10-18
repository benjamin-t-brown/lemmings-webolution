# Phase 3 round 8 — what blocker bit `0x800` does to a walker

2026-10-15. Stock `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`,
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, driven through `tools/dinput-shim`.

Harness: `tools/harness/measure_blocker.py` (new). Every address is in its
docstring.

The brief's hypothesis was right and it is now measured: **a blocker does not
interact with other lemmings at all.** It stamps a field into the collision
grid, and a walker turns because its ordinary terrain probe reads that field.
`World::blockedBy`'s scan over the lemming list was the wrong shape, the way
`basherInterval` was in round 5, and it is gone.

---

## SUMMARY

| Thing | Status | Value |
| --- | --- | --- |
| **where the bit comes from** | **READ** | the block state's **enter hook `0x0040E20C`**, `or ah, 8` at `0x0040E312`. No `e8` caller in all of `.text`; reached only through vtable `0x004B07B8 + 0x08`. |
| **where it goes** | **READ** | the block state's **exit hook `0x0040E0DF`**, `and eax, 0xf7ff` at `0x0040E1D5`. Same loop, same cells, and it clears nothing but `0x800`. Only these two instructions in `.text` write the bit. |
| **the stamped footprint** | **MEASURED, 3 blockers, 12 cells** | **one column × four rows**: the blocker's own `trunc(X)`, rows `trunc(Y) .. trunc(Y)+3` upward in the Y-up live grid — the feet row and the three above it. Every cell predicted from the read before the click and matched. |
| **what it writes** | **MEASURED, 12 of 12** | `0x00000000 -> 0x00000800`. A read-modify-write of the whole dword; the cells are **air and stay air**. |
| **when** | **MEASURED** | inside **5 ms** of the button press, all four cells in one 1.5 ms sample. Not on a tick boundary — the enter hook runs in the click. |
| **the turn rule** | **MEASURED, 10 walkers, both facings, 2 levels** | a walker turns when the cell at its **probe column** — `cell + facing` — carries `0x800`, at its feet row. Right-facing turns at world X = C, left-facing at X = C + 1: the two edges of the blocker's own cell. |
| **where the test is** | **READ** | three sites **inside the walk probe `0x0041BFFC`**, interleaved with the step-up arm, each taken BEFORE that cell's own `and 1`: `0x0041C430` (feet), `0x0041C4C9` (feet+1), `0x0041C563` (feet+2). The probe has **one** caller in `.text`. |
| **what the turn is** | **READ** | `0x0042F66B`: `[+0x40] = -[+0x40]`, then `[+0x350]` and `[+0x360]` set to −1.0 facing left and 0.0 facing right. The probe then returns **0** and the walk's slice loop keeps going. |
| **the erase on removal** | **MEASURED, once** | bombing a blocker cleared exactly its four cells **2 ms after** its state became `explode`, a whole second before the crater. |
| **`0x800` is runtime state** | **MEASURED, offline, 102 of 102** | **0 cells of 2,506,752** in the shipped grids carry it. The first cell bit in this project shown to be written during play rather than authored. |
| **bash and mine also read it** | **READ, not measured** | `0x0040C110` refuses the bash bite; `0x00418AF9` makes the miner **turn** (`0x00418B06` calls `0x0042F66B`). The dig bite `0x00415A43` has no `0x800` test. |
| **the `[+0x4c] == 3` exemption** | **READ; never fires in play** | `[lemming+0x4c]` read **0** on every live lemming sampled this round, blockers included. What type 3 is was not read. |
| `blockerFieldHeight` | **MEASURED** | **4**. The stamp loop is `j = 0` while `j <= [0x004B10B8]`, and that dword is 3. |
| `blockedBy`'s `|dx| <= 3` | **WRONG, and now shown wrong** | it turned a walker at cells 127 and 133 either side of a blocker at 130; the original turns at 129 and 131. |

Re-run. Offline first; it needs no process:

```powershell
python tools/harness/measure_blocker.py --audit
python tools/harness/measure_blocker.py --predict MM_BLOCK_001_A
python tools/harness/read_steppers.py --states          # vtable 0x004B07B8
python tools/harness/read_steppers.py --at 0x40E20C --len 0x140   # the stamp
python tools/harness/read_steppers.py --at 0x40E0DF --len 0x130   # the erase
python tools/harness/read_steppers.py --at 0x41BFFC --len 0x600   # the probe
python tools/harness/read_steppers.py --at 0x42F66B --len 0xA0    # the turn
```

Live, one process per level (there is still no route from inside a level back
to the map). **`MM_Block_Bomb_A` is spelt that way in the live table and
`select_level` is case-sensitive**:

```powershell
# restart the process, then
python tools/harness/measure_blocker.py --enter MM_BLOCK_001_A --probe
python tools/harness/measure_blocker.py --stamp --seconds 3 --watch 20
# restart the process, then
python tools/harness/measure_blocker.py --enter MM_Block_Bomb_A --probe
python tools/harness/measure_blocker.py --tight --dir 1  --watch 30
python tools/harness/measure_blocker.py --tight --dir -1 --watch 30
python tools/harness/measure_blocker.py --bomb 2 --seconds 10
```

Build and suite:

```powershell
.\scripts\Invoke-Ucrt64.ps1 "make -C src test"
```

---

## The static read, in the order it was followed

### The stamp is the block state's enter hook

`read_steppers.py --states` names the block state: vtable **`0x004B07B8`**,
dtor `0x0040E390`, step `0x0040E003`, **enter `0x0040E20C`**, **exit
`0x0040E0DF`**, animation `block` (the string pointer `0x004BF148` pushed into
`0x0042FD25` at `0x0040E22A`). None of those three functions has an `e8`
caller anywhere in `.text`; each appears exactly once in the whole file as a
dword, at file offsets `0xB07BC` / `0xB07C0` / `0xB07C4`, which are
`0x004B07B8 + 4 / 8 / 0xC`. So entering the block state is the only thing that
can stamp the field and leaving it the only thing that can erase it.

The enter hook, `0x0040E20C` (observation, disassembly):

```
0040e22a  68 48 f1 4b 00     push 0x4bf148          ; "block"
0040e232  e8 ee 1a 02 00     call 0x42fd25          ;   the animation strip
0040e237  68 00 00 d0 3f     push 0x3fd00000        ; the rate 0.25 ...
0040e247  e8 9d a1 05 00     call 0x4683e9          ;   ... onto lemming+0x128
0040e253  e8 d6 0f 02 00     call 0x42f22e          ; pos = (X + [+0x360],
                                                    ;        Y + [+0x368], Z)
0040e26f  dd 45 b8           fld  [ebp-0x48]        ; that X
0040e272  e8 35 e3 08 00     call 0x49c5ac          ;   truncated
0040e288  e8 33 fc ff ff     call 0x40dec0          ;   wrapped into [0, 256)
0040e28d  e8 1a e3 08 00     call 0x49c5ac          ;   truncated again -> col
0040e29a  83 7a 40 ff        cmp  [edx+0x40], -1
0040e29e  0f 94 c0           sete al
0040e2a4  03 c8              add  ecx, eax          ;   +1 when facing LEFT
0040e2a9  dd 45 c0           fld  [ebp-0x40]        ; that Y
0040e2ac  e8 fb e2 08 00     call 0x49c5ac          ;   truncated     -> base
0040e2c9  3b 05 b8 10 4b 00  cmp  eax, [0x4b10b8]   ; the dword 3
0040e2cf  7f 4b              jg   0x40e31c          ;   so j = 0, 1, 2, 3
0040e2d7  0f af 51 44        imul edx, [ecx+0x44]   ; row = base + j * vdir
0040e2f1  e8 5a fc ff ff     call 0x40df50          ;   clamped to the grid
0040e305  e8 56 fb ff ff     call 0x40de60          ; the cell at (col, row)
0040e30a  89 45 b4           mov  [ebp-0x4c], eax
0040e310  8b 02              mov  eax, [edx]
0040e312  80 cc 08           or   ah, 8             ; SET 0x800
0040e318  89 01              mov  [ecx], eax        ;   stored back
0040e31a  eb a1              jmp  0x40e2bd          ; next row
```

Four cells, one column, and the column is the blocker's own `trunc(X)` in both
facings: `0x0042F22E` returns `X + [+0x360]`, `[+0x360]` is −1.0 facing left
and 0.0 facing right (`0x0042F66B`, below), and the `sete`/`add` pair puts the
1 back. The rows are `trunc(Y) + j * [+0x44]` — the feet row and the three
above it for an upright lemming.

Observation worth keeping: the stamp is an `or` on the loaded dword and the
store writes the whole dword back, so **every other bit of the cell survives**.
`0x0040E2F1`'s `0x0040DF50` is the Y clamp (round 5's note names it) and
`0x0040DE60` is the cell probe.

### The erase is its exit hook, and it is the same cells

`0x0040E0DF` is the same loop with one instruction changed:

```
0040e1d5  25 ff f7 00 00     and  eax, 0xf7ff       ; CLEAR 0x800, only 0x800
```

The only difference is which position helper it uses: the exit hook calls
`0x0042F183`, which truncates first and then adds the bias, where the enter
hook calls `0x0042F22E`, which adds the bias and then the hook truncates. For
a non-negative X and an integer bias those agree cell for cell, so the erase
footprint is the stamp footprint.

**When it runs.** `0x0042E696` is the state swap (round 5's assign note names
it as the click path's third step). Read in full:

```
0042e6a9  ff 52 08           call [edx+8]           ; the NEW state's enter hook
0042e6af  e8 9c 23 00 00     call 0x430a50          ; queued instead?
0042e6d4  83 b9 40 03 ...    cmp  [ecx+0x340], 0
0042e6f5  ff 52 0c           call [edx+0xc]         ; the OLD state's EXIT hook
0042e6fe  89 88 40 03 00 00  mov  [eax+0x340], ecx  ; then store the new state
```

so *any* departure from the block state erases the field — a bomb, another
skill, or the block step's own route out. That step, `0x0040E003`, advances the
animation, zeroes the remaining slice, and calls `0x0042E913`; on a 0 it builds
the faller at `0x00415FA0` and swaps it in, which goes through `0x0042E696`.

Hypothesis, **not** measured and worth someone's time: the block state's
destructor `0x0040E390` calls only the base dtor `0x0040DFE7` and does **not**
clear the field, so a blocker destroyed without a state swap would leak its
four cells. Every death seen this round went through a state change.

### The test is three sites inside the walk probe

`0x0041BFFC` has exactly **one** `e8` caller in `.text`, `0x0041BEE6` inside
the walk stepper `0x0041BDD4`, so the turn is walk-only. The probe, in the
order it tests (observation, disassembly):

```
0041c069  call 0x42f22e      ; the biased position
0041c099..0041c0c2           ; col = trunc(wrap(trunc(X + [+0x360])))
0041c0c5..0041c104           ; cell A = (col, trunc(Y) - vdir), the floor
0041c154  and edx, 0x8000    ;   only 0x8000 is read off the floor cell
0041c128..0041c147           ; cell B = (col, trunc(Y)), the FEET row
0041c430  and eax, 0x800     ; B carries the field?
0041c43c  cmp [ecx+0x4c], 3  ;   ... and I am not type 3
0041c445  call 0x42f66b      ;   -> TURN; the probe then returns 0
0041c47d  and edx, 1         ; B solid? no -> the step-DOWN arm at 0x41c6ef
0041c48e  add ecx, [eax+0x44]; cell C = (col, trunc(Y) + vdir)
0041c4c9  and edx, 0x800     ; 0041c4d6 cmp [eax+0x4c], 3 -> the same turn
0041c517  and ecx, 1         ; C solid? no -> adopt rise 1
0041c528  add eax, [edx+0x44]; cell D = (col, trunc(Y) + 2*vdir)
0041c563  and ecx, 0x800     ; 0041c570 cmp [edx+0x4c], 3 -> the same turn
0041c5b1  and eax, 1         ; D solid? no -> adopt rise 2
0041c5c1  mov dl, [ecx+0x48] ;   D solid too: the climber flag, else 0x41c676
```

Three things follow, all observation.

1. **The field test comes before the solidity test at every level.** So the
   bit works on air cells, which is what the blocker stamps.
2. **It short-circuits with the step-up arm.** `feet+1` is only tested if
   `feet` was solid and `feet+2` only if `feet+1` was. There is no fourth
   test: this is the same unrolled-twice shape round 4 read `maxStepUp = 2`
   off, and the field tests sit inside it rather than beside it.
3. **The turn returns 0, and 0 does not end the walk.** `0x0041BDD4`'s slice
   loop is `while remaining > 0 && [ebp-4] == 0` (`0x0041BE13`), so the
   original reverses and spends the rest of the frame walking back.

`0x0042F66B`, read in full, is the turn and nothing else:

```
0042f678  6b c9 ff           imul ecx, ecx, -1      ; [+0x40] = -[+0x40]
0042f684  83 78 40 ff        cmp  [eax+0x40], -1
0042f68d/97  [+0x350] = 0xbff00000'00000000 = -1.0  ; facing left
0042f6a4/ae  [+0x360] = -1.0
0042f6bd..d1 [+0x350] = [+0x360] = 0.0              ; facing right
```

which is also why the probe column comes out `cell + facing` in both
directions — see "the asymmetry cancels" below.

### Everything in `.text` that touches `0x800`

Observation, re-runnable. Scanning every decoded instruction in `.text` for an
immediate of `0x800`, `0xf7ff`/`0xfffff7ff`, or an `and`/`or`/`xor` on
`ah`/`ch`/`dh`/`bh` whose mask lands on bits 8..15, and excluding the CRT above
`0x0049C000`:

| site | instruction | function | what |
| --- | --- | --- | --- |
| `0x0040E312` | `or ah, 8` | `0x0040E20C` | **the only setter** — block enter |
| `0x0040E1D5` | `and eax, 0xf7ff` | `0x0040E0DF` | **the only clearer** — block exit |
| `0x0040C110` | `and eax, 0x800` | `0x0040C0AF` | the **bash** bite's third cell test |
| `0x00418AF9` | `and edx, 0x800` | `0x004188F6` | the **mine** stepper |
| `0x0041C430` | `and eax, 0x800` | `0x0041BFFC` | the walk probe, feet |
| `0x0041C4C9` | `and edx, 0x800` | `0x0041BFFC` | the walk probe, feet+1 |
| `0x0041C563` | `and ecx, 0x800` | `0x0041BFFC` | the walk probe, feet+2 |
| `0x0043F63E` | `and eax, 0x800` | `0x0043F510` | unread; see below |

The dig bite `0x00415A43` has **no** `0x800` test — it tests `0x2` and the
stack-built directional mask and nothing else, which round 5 read.

`0x0040C0AF` is the bash bite's cell gate, and the `0x800` test is the third
of three on the same cell pointer (`[ebp+8]`):

```
0040c0f8  83 e2 02           and edx, 2             ; indestructible
0040c104  23 4d ec           and ecx, [ebp-0x14]    ; the directional 4 or 8
0040c110  25 00 08 00 00     and eax, 0x800         ; a blocker's field
0040c117  74 3b              je  0x40c154           ;   all clear -> remove
```

so **you cannot bash a cell a blocker is standing in.** `0x004188F6`'s is
stronger: a set bit makes the miner call `0x0042F66B` at `0x00418B06` and leave
the bite, i.e. **turn round**. Both are READ. Neither was reproduced live —
MM_BLOCK_001_A and MM_Block_Bomb_A stock no basher or miner, so it would take a
level that stocks both, and that is a run someone else can make.

`0x0043F510` is called from `0x004082D7` and `0x0041DF89` and tests `0x2`,
`0x200`, `0x800` and `0x1` on one cell pointer. Not read; recorded so the next
person does not have to find it again.

---

## Live — the stamp

MM_BLOCK_001_A ships `[0,0,0,10,0,0,0,0,0,1]`: **ten of panel index 3 and
nothing else**, which is the same pre-selection argument that identified 5 and
6 in round 5, and it agrees with `phase3-assign-skill.md`'s blocker = 3.

Run 1, `--stamp --seconds 3 --watch 20`, pid 50028. The script prints what the
read predicts for every live lemming *before* it clicks anything, so the series
is a test and not a fishing trip:

```
  0x02770760 x   78.515 y  71.000 dir -1 vdir +1 +0x4c 0 bias (-1.0,+0.0)  walk
       would stamp [(78, 71), (78, 72), (78, 73), (78, 74)]
  cells carrying 0x800 BEFORE the click: 0
  attempt 0: 0x02770760 state walk -> block, stock[3] 10 -> 9
  blocker at (76.645, 71.000) dir -1 vdir +1 +0x4c 0:
       the read predicts [(76, 71), (76, 72), (76, 73), (76, 74)]
  whole-grid before/after diff:
    (col  76, row  71)  0x00000000 -> 0x00000800   d(col)=+0 d(row)=+0
    (col  76, row  72)  0x00000000 -> 0x00000800   d(col)=+0 d(row)=+1
    (col  76, row  73)  0x00000000 -> 0x00000800   d(col)=+0 d(row)=+2
    (col  76, row  74)  0x00000000 -> 0x00000800   d(col)=+0 d(row)=+3
  cells that GAINED 0x800: [(76, 71), (76, 72), (76, 73), (76, 74)]
  MATCH: True
```

Two more on MM_Block_Bomb_A (`[0,0,15,15,0,0,0,0,0,1]`), pid 19396, through
`--tight`, which presses the button from inside the diff loop:

```
  aiming at 0x027b5260 x 170.066 y 78.000 dir +1;
       the read predicts [(170, 78), (170, 79), (170, 80), (170, 81)]
  tight stamp: 802 whole-grid samples in 1.20s (668 Hz), 4 cell changes
    t +0.0050s  (col 170, row  78)  0x00000000 -> 0x00000800
    t +0.0050s  (col 170, row  79)  0x00000000 -> 0x00000800
    t +0.0050s  (col 170, row  80)  0x00000000 -> 0x00000800
    t +0.0050s  (col 170, row  81)  0x00000000 -> 0x00000800
  stock[3] 15 -> 14  spent=True  state now block

  aiming at 0x027bb150 x 105.898 y 78.000 dir -1;
       the read predicts [(105, 78), (105, 79), (105, 80), (105, 81)]
    t +0.0050s  (col 105, rows 78, 79, 80, 81)  0x00000000 -> 0x00000800  x4
  stock[3] 14 -> 13  spent=True  state now block
```

Observations.

- **Three blockers, twelve cells, every one predicted and every one matched.**
  One column, four rows, feet row upward, at `trunc(X)` in both facings
  (76.645 and 105.898 facing left, 170.066 facing right).
- **All twelve went `0x00000000 -> 0x00000800`.** The cells a blocker marks
  are air and stay air: the bit is independent of solidity, as the `or`/`and`
  pair says.
- **The stamp is immediate, not tick-aligned.** All four cells changed in the
  same 1.5 ms sample, 5 ms after the press — about one 7 ms frame, which is
  the input poll. Round 5's dig bite was 300 ms in; this is not that kind of
  event, because it is in the enter hook rather than in a record.
- **Nothing else in the 256 × 96 grid ever moved.** Every run diffed the whole
  grid before and after and that check never fired.
- **Proof of assignment is the stock going down**, 10→9, 15→14, 14→13, with
  the state read as `block` in the same window.

---

## Live — the turn

Ten walkers, both facings, two levels. `--watch N` follows every live lemming
at 19–45 kHz and prints every change of `[+0x40]`.

MM_BLOCK_001_A, field column **76**, four left-facing walkers:

```
  t +5.552s FLIP 0x027714c0 walk dir -1 -> +1  x 77.011 -> 77.024  y 71.000
  t+10.803s FLIP 0x027724c0 walk dir -1 -> +1  x 77.007 -> 77.033  y 71.000
  t+16.048s FLIP 0x02773990 walk dir -1 -> +1  x 77.029 -> 77.006  y 71.000
  t+21.299s FLIP 0x02774a10 walk dir -1 -> +1  x 77.025 -> 77.010  y 71.000
```

MM_Block_Bomb_A, field column **170**, five right-facing walkers (four in the
first window, one more in the second):

```
  t +5.276s FLIP 0x027b60e0 walk dir +1 -> -1  x 169.997 -> 169.968  y 78.000
  t+12.368s FLIP 0x027b70e0 walk dir +1 -> -1  x 169.998 -> 169.962  y 78.000
  t+19.453s FLIP 0x0273a610 walk dir +1 -> -1  x 169.974 -> 169.991  y 78.000
  t+26.545s FLIP 0x027b6490 walk dir +1 -> -1  x 169.975 -> 169.990  y 78.000
  t+18.063s FLIP 0x027bdc50 walk dir +1 -> -1  x 169.971 -> 169.989  y 78.000
```

and the cleanest single result of the round — with blockers at **105** and
**170** on the same floor, one lemming bounced between them and gave both
facings:

```
  field columns [105, 170] rows [78, 79, 80, 81]
  t +5.268s FLIP 0x027bdc50 walk dir -1 -> +1  x 106.001 -> 106.034
            snapped x 106  probe col 105  ON the field
  t+18.063s FLIP 0x027bdc50 walk dir +1 -> -1  x 169.971 -> 169.989
            snapped x 170  probe col 170  ON the field
```

### Reading the X: the snap, and why the asymmetry cancels

The X a kHz sample sees just before a flip is **not** the X the probe tested.
`0x0041BDD4` walks in slices, and when a slice lands exactly on a column it
snaps X to that integer (`0x0041BECE`) and only then calls the probe. So the
probe's X is `ceil` of the last sample going right and `floor` of it going
left. With that correction:

| field column C | facing | last sample X | snapped X | probe col = `trunc(X + [+0x360])` |
| --- | --- | --- | --- | --- |
| 76 | left | 77.007–77.029 | 77 | `trunc(77 − 1) = 76` ✓ |
| 105 | left | 106.001 | 106 | `trunc(106 − 1) = 105` ✓ |
| 170 | right | 169.971–169.998 | 170 | `trunc(170 + 0) = 170` ✓ |

Ten of ten. So, as world X:

- walking **right**, a walker turns at **X = C** — it reaches the near edge of
  the blocker's own cell;
- walking **left**, it turns at **X = C + 1** — the far edge of the same cell.

Those are the two boundaries of the blocker's cell, so the blocker is
symmetric in space even though the probe expression is not. And **in cells the
expression collapses**: a walker in cell `c` going right snaps to `c+1` with a
0.0 bias, giving probe column `c+1`; going left it snaps to `c` with a −1.0
bias, giving `c-1`. Both are `c + facing`. That settles one of round 4's five
open stepper differences in our favour — *"the probe column is asymmetric in
the facing; `World::stepWalker` probes `l.x + dir` either way"* — and the
answer is that `l.x + dir` is what the original computes too, once the snap is
taken into account. Recorded here; the other four remain open and untouched.

### Which of the three sites fired

All ten walkers had their feet on the field's bottom row (y 71.000 against a
field at 71–74; y 78.000 against 78–81), so it is the **first** test,
`0x0041C430`, that fired every time. `0x0041C4C9` and `0x0041C563` are read and
not measured: making them fire needs the blocker's feet one or two rows above
the walker's with solid cells in between, which the corpus does not hand you
and nothing was carved this round.

### The post-turn X, and one thing it implies

Observation, recorded rather than acted on. After a turn the X moves back by
only ~0.03 in the same instant (169.997 → 169.968, 106.001 → 106.034), not by
the ~0.95 a full 200 ms tick of the reversed walk would give. 0.03 world units
at the walk's 5.0 units/second is about 6 ms. That is consistent with the walk
stepper being called **once per rendered frame with that frame's dt** rather
than once per 200 ms tick — the 200 ms tick being the animation and record
clock round 5 measured, and the 1.000 column per tick being 5 u/s integrated
over it. This round did not chase it and nothing here depends on it. It is the
natural companion to the step-down and walk-bias work that the combined
stepper round owns.

---

## Live — the erase

`--bomb 2` on MM_Block_Bomb_A. The level ships exactly two skills, index 3
(the measured blocker) and index **2**, and the level is named
`MM_Block_Bomb_A`; the assignment then produced the `explode` state and a
crater, so **panel index 2 is the bomber**. That closes one of the three
indices round 5 left unidentified (8 and 9 are still open; see the negative
below).

```
  field before: [(105, 78..81), (170, 78..81)]
  clicked panel 2 on blocker 0x027b5260 at t 0
  t  +0.026s state -> block
  t  +4.997s state -> explode
  t  +4.999s explode (col 170, row  78)  0x00000800 -> 0x00000000   -0x800
  t  +4.999s explode (col 170, row  79)  0x00000800 -> 0x00000000   -0x800
  t  +4.999s explode (col 170, row  80)  0x00000800 -> 0x00000000   -0x800
  t  +4.999s explode (col 170, row  81)  0x00000800 -> 0x00000000   -0x800
  t  +6.019s state -> 0x004b0550  x 170.066 y 77.000
  t  +6.020s  14 cells of 0x00000001 -> 0x00000000 at rows 75..77   (the crater)
  t  +6.025s state -> none
  stock[2] 15 -> 14  spent=True
  field after: [(105, 78), (105, 79), (105, 80), (105, 81)]
```

Observations.

- **The erase is the state change, not the explosion.** The four cells cleared
  **2 ms** after the state became `explode` and a full second before the
  crater appeared. That is `0x0042E696` calling the exit hook, exactly as read.
- **Exactly the four cells, and only that blocker's.** The other blocker's
  field at column 105 was untouched.
- `0x00000800 -> 0x00000000` restores the word the cell had before the stamp,
  because `and 0xf7ff` clears only `0x800` and the cells were air.
- Incidental, and **round 7's not mine**: the fuse ran **4.971 s** from the
  assignment (0.026 → 4.997). `bomberFuse` was not touched.

---

## Clean negatives, and what was searched for

- **No shipped grid carries `0x800`.** All 102 `.LVL` collision arrays,
  2,506,752 cells, **zero**. `measure_blocker.py --audit`, offline. This is
  the contrast `CollisionGrid.h` wanted: `0x4`/`0x8`/`0x10`/`0x20` arrive in
  the file and have no writer, `0x800` appears in no file and has exactly one
  writer and one clearer.
- **No instruction other than `0x0040E312` and `0x0040E1D5` writes the bit.**
  The scan covered every decoded `.text` instruction, including the `ah`-mask
  encoding that hides a `0x800` behind an immediate of 8 — the encoding the
  round-5 scan saw and parked.
- **Panel index 9 is not a nuke you can reach in two clicks.** On
  MM_BLOCK_001_A with one blocker left alive, its box (18, 372)–(90, 404) was
  clicked twice 0.9 s apart while the whole grid was sampled at 692 Hz: **no
  cell changed, the blocker was still blocking, `ui_state` still 0.** Index 9
  is 1 on every level checked so far, so it is something, but it is not that.
- **Nothing outside the stamped cells ever changed.** Every live run diffed
  the whole 96-row grid before and after.
- **No lemming byte and no grid cell was written to the game.** The only write
  this session was `--enter`'s level-record swap, three times, each printing
  `slot [0][0] restored: True`; group 0 slot 0 read back as its own
  `MM_CLIMB_002` record at the end.
- **`[lemming+0x4c]` was 0 on every lemming sampled**, roughly two dozen
  across two levels and including all three blockers, so the probe's
  `cmp [..+0x4c], 3` exemption never takes its arm in ordinary play. `+0x4c`
  is the same field the splat's zone arms compare against 1 and 2
  (`phase3-round3b-float-splat.md`) and the same index `0x0042FD25` uses to
  pick an animation sheet. What value 3 means is **unread**.

### What could not be caught

**A lemming falling into a blocker's field.** The faller's landing probe
accepts any non-zero cell with `0x80` clear —

```
0041a991  83 7d f0 00        cmp dword ptr [ebp-0x10], 0
0041a995  74 6f              je  0x41aa06          ; empty -> keep falling
0041a99a  81 e2 80 00 00 00  and edx, 0x80
0041a9a2  75 62              jne 0x41aa06          ; not landable -> keep falling
```

— which is the test `CollisionGrid::solid` already carries, and a stamped air
cell reads `0x00000800`. So a lemming dropping down a blocker's column should
**land on the field**, four rows up, standing on the blocker's head, while a
*walker* in the same cells would not step up onto them (the walk's own
solidity test is `and 1`, which `0x800` fails). That is READ, not measured, and
it is **not modelled**.

Two 40 s and 150 s hunts for a walker at the hatch column (128 on both levels)
to block failed the same way: `--tight --col 128` found a candidate on nearly
every scan and never got one hoverable, because `[screen+0x16D8]` kept naming
a neighbour — the same wall round 5 recorded for `one_run`. The second hunt
also ran MM_BLOCK_001_A's 300 s clock out. The run that would settle it: enter
MM_BLOCK_001_A, block a lemming standing at `trunc(X) == 128` on the row-81
floor (the hatch drops at X = 128.000 from about Y 86), then watch a fresh
spawn's `fall -> walk` transition — `--watch` now prints every state change
with the cell it happened in. The prediction is feet at **row 85**, four rows
above the floor, instead of 81.

---

## What changed in the tree

| file | change |
| --- | --- |
| `tools/harness/measure_blocker.py` | **new.** `--audit` and `--predict` are offline; `--stamp`, `--tight`, `--bomb`, `--watch`, `--watch-only` are the live series above. Every address is in its docstring, with the three traps this round hit. |
| `src/sim/Terrain.h`, `src/sim/Terrain.cpp` | `blockerField` / `setBlockerField` / `blockerFieldCells`: a one-byte-per-cell flag layer beside the material, allocated lazily. The original keeps `0x800` in the same `u32` as the material and never disturbs the rest of the cell, so a flag is the faithful shape; a `Material` would have made a blocker's field destroy the terrain under it. |
| `src/sim/World.h` | `blockedBy` replaced by `stampBlockerField` / `clearBlockerField` / `leaveBlocker`. New `Rules::blockerFieldHeight = 4`, MEASURED, with the `0x004B10B8` read and the series. `step` lost its now-unused `index` parameter and `stepWalker` its `index`. |
| `src/sim/World.cpp` | the list scan is gone. The stamp and the erase are the two hooks; `leaveBlocker` is called from `assign`, from the Blocker→Faller arm of `step`, and from `explode`, which are the three routes out of the block state this sim has. `stepWalker`'s step-up loop now tests the field at each of its three cells before that cell's solidity, which is the probe's own order. |
| `src/assets/CollisionGrid.h` | `kCellBlocker` documented with both hooks, the footprint, the eight readers, the live series, and the 0-of-102 audit. The "what the search did see, and rejected" paragraph and the four-refusal-bits note now point at this round for the contrast between authored bits and runtime ones. |
| `src/__test__/test_world.cpp` | `testBlocker` (a loose two-lemming `||`) replaced by `testBlockerField`, `testBlockerTurnsWalkers` and `testBlockerFieldIsErased`. |

Not touched: `fallSpeed`, `floatSpeed`, `floatAfter`, `splatDistance`,
`walkSpeed`, `climbSpeed`, `ticksPerSecond`, `maxStepUp`, `lemmingHeight`,
round 5's cycle and footprint fields, `bomberFuse`, `World::spawn`,
`findHatchRecords`, `src/assets/LevelStats.cpp`, `worldToMaskPixel`,
`test_corpus`'s numbers, `test_collisiongrid`, `PLAN.md`,
`docs/PHASE3_HANDOFF.md`, and the five open stepper differences.

### A note on `test_collisiongrid`

Its five live dumps are fixtures and were not regenerated. They are safe:
`0x800` is written only while a blocker is standing, and
`tools/harness/dump_grids.py` dumps a level on entry before any skill is
assigned. **If a future dump is taken with a blocker alive it will contain
`0x800` and will not match the `.LVL`** — that would be a bad dump, not a
parser bug and not an expectation to update.

---

## Tests

`test_world` gained three cases, 153 checks in all, and the **expectations are
written out, not derived** from `Rules::blockerFieldHeight` and not read back
out of the stamping code. Mapped once from the engine's Y-up grid into our
downward y, and the mapping is stated in the test's own comment: the engine's
"feet row + 0..3" is our `y` down to `y - 3`, and its "a right-facing walker
turned at world X = C" is our "it never left cell C − 1".

- `testBlockerField` — four cells at `(130, 79/78/77/76)`, nothing at
  `(130, 75)`, `(130, 80)`, `(129, 79)` or `(131, 79)`, `solidCells()`
  unchanged, and the blocker's own cell still air.
- `testBlockerTurnsWalkers` — right-facing turns in cell **129** and never
  reaches 130; left-facing turns in cell **131** and never reaches 130; and
  the control, a plain walker at 130 instead of a blocker, which the other
  walker walks straight past.
- `testBlockerFieldIsErased` — the bomb, the floor going away, a reassignment
  to the digger, and that a walker is no longer turned once the field is gone.

**Verified to fail against the old behaviour.** With the list scan restored
(`stepWalker` calling the old `|dx| <= 3` / `|dy| < lemmingHeight` scan and
the stamp and erase made no-ops, the rest of the file unchanged), `test_world`
fails **9 checks**, and four of them are behavioural rather than field or
footprint asserts:

```
  FAIL  four cells stamped (got 0)
  FAIL  (130, 79) carries the field          (and 78, 77, 76)
  FAIL  turned in cell 129 (got 127)
  FAIL  never entered the blocker's column: furthest 127
  FAIL  turned in cell 131 (got 133)
  FAIL  never entered the blocker's column: nearest 133
```

That is the size of the error the round removed: the old radius turned a
walker **two cells too early on both sides** — at 127 and 133 around a blocker
at 130, where the original turns at 129 and 131.

Restored, the suite is **16 of 16** and `test_world` is 153 checks,
0 failures.

```powershell
.\scripts\Invoke-Ucrt64.ps1 "make -C src test"
```

---

## Left open, with citations — not this round's to change

- **The bash refusal and the miner's turn on `0x800` are READ and not
  modelled.** `0x0040C110` refuses the bash bite; `0x00418AF9` →
  `0x00418B06` → `0x0042F66B` turns the miner. Neither level used this round
  stocks either skill. A level that stocks a basher and a blocker settles both
  in one run, and `measure_blocker.py --bomb <panel>` is already the shape of
  that run.
- **A faller should land on a blocker's field.** `0x0041A99A`, the prediction,
  and the exact run are above under "what could not be caught".
- **A digger can zero a blocker's field cell.** The dig bite has no `0x800`
  test and its removal is `mov dword ptr [edx], 0` (round 5), which would
  clear the bit with everything else; its footprint is the row *below* its own
  feet in its own column ±1, so it can reach a neighbouring blocker's column.
  Not modelled: `Terrain::clearBox` and `clearCircle` leave the flag layer
  alone, because what an *explosion* does to the bit is unread and guessing
  one of the two would be worse than documenting both.
- **`[lemming+0x4c] == 3`.** The probe's exemption. `+0x4c` is 0 on every
  ordinary lemming; 1 and 2 appear in the splat's zone arms; 3 is unread and
  nothing in this simulation has a lemming type to hold it.
- **`0x0043F510`** tests `0x800` at `0x0043F63E` alongside `0x2`, `0x200` and
  `0x1` on the same cell, and is called from `0x004082D7` and `0x0041DF89`.
  Unread.
- **The block state's destructor does not clear the field** (`0x0040E390`
  calls only `0x0040DFE7`). Whether a blocker can be destroyed without a state
  swap, and so leak four cells, is untested.
- **Panel indices 8 and 9 are still unidentified**, and index 9 did nothing
  when clicked twice. Index **2 is the bomber**, measured this round.
- **The walk stepper may run per frame, not per tick** — see "the post-turn X"
  above. Observation only, and it belongs to the combined stepper round.
- The other four of round 4's five open stepper differences are untouched. The
  walk-X-bias one is answered above: `l.x + dir` is right.

## Left the game

**Killed.** No process is running at the end of this session; `Get-Process`
for `*Lemming*` returns nothing. The shim is installed and inert
(`dinput.dll` in the game folder, `enable` left at 0 by `LRInput.release()`),
so a fresh launch comes up at `ui_state 3`, the main menu.

Nothing was running at dispatch either. Four processes were used:

| pid | level | what for |
| --- | --- | --- |
| 50028 | MM_BLOCK_001_A | the first stamp (blocker at column 76) and four left-facing turns; the panel-9 negative |
| 19396 | MM_Block_Bomb_A | both `--tight` stamps (170 and 105), ten turns in total, the bomb, and two failed column-128 hunts |
| 29104 | — | restarted, then found at `ui_state 5`; nothing was read from it |
| 22940 | MM_BLOCK_001_A | the third entry and the last column-128 hunt; the level clock ran out |

All three `--enter` calls printed `slot [0][0] restored: True`, and before the
kill the live table read group 0 slot 0 = `MM_CLIMB_002` with its own record,
which is the untouched state. The last level actually played was
`Compiled\MM_BLOCK_001_A.lvl`. No ESC was pressed from a results screen; the
two `ui_state 5` sightings were the level's own 300 s clock expiring during a
long hunt, and in both cases the process was killed rather than driven.
