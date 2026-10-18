# Phase 3, round 10 — the five stepper differences

2026-10-15. Question: five places where our stepper is known to differ from
the original's, each already cited and none modelled. They are all in or next
to the walk probe `0x0041BFFC` and the walk stepper `0x0041BDD4`, which is why
they are one round.

Gate: round 1's open disagreement — `compare_tick.py --only walker` matched
live for ticks 0–7 on `MM_CLIMB_002` and diverged at tick 8.

Static reads with `tools/harness/read_steppers.py --at`, stock exe
`C:\progs\Lemmings Revolution\Lemmings Revolution.exe`, SHA-256
`32675bdb…3737a2f` (the digest that script checks). Addresses are image-based
(`0x00400000`).

---

## Part 1 — what the binary says (static, before any live run)

### The walk loop advances X, snaps it, and only then probes

`0x0041BDD4`, the walk stepper, slot +4 of vtable `0x004B0D28`. `[ebp+8]` is
the lemming, `[ebp+0xc]` points at the seconds left in the frame.

```
0041bdfd  (loop head) while seconds > 0 and [ebp-4] == 0:
0041be24    call 0x41bf61            ; time to the next column boundary
0041be3e    fld [ebp-0x10] / fcomp [edx]
0041be4b    je 0x41be5f              ; boundary further than the slice ->
                                     ;   use the whole slice, flag 0
0041be4d    mov byte [ebp-8], 1      ; else clamp the slice to the boundary
0041be70    fld  qword [0x4b10b0]    ; 5.0
0041be76    fmul qword [ebp-0x18]    ;  * the slice
0041be7c    fimul dword [eax+0x40]   ;  * facing
0041be7f    fadd qword [edx+8]       ;  + X
0041be8d    call 0x40dec0            ; wrap round the 256-column tube
0041be95    fstp qword [ecx+8]       ; X := that            <-- X MOVES HERE
0041beac    call dword [eax+0x2c]    ; advance the animation by the slice
0041beb2    fld [ecx] / fsub [ebp-0x18] / fstp [edx]   ; seconds -= slice
0041bebc    mov eax,[ebp-8] / test / je 0x41bef7        ; boundary reached?
0041bece      call 0x49c5ac          ; trunc(X)
0041bed6      fild [ebp-0x20] / fstp [edx+8]   ; X := trunc(X)   <-- SNAP
0041bee6      call 0x41bffc          ; THE PROBE, with X already moved
0041beeb      mov [ebp-4], eax       ; non-zero -> leave the loop
0041bf35    jmp 0x41bdfd
```

`0x0041BF61` is the boundary helper, and it is asymmetric:

```
0041bf7c  cmp dword [eax+0x40], 1    ; facing
0041bf80  jne 0x41bfa7
0041bf82    fld [ecx+8] / call 0x49c5ac / add eax,1   ; right: trunc(X) + 1
0041bf93    fild / fsub [edx+8] / fdiv [0x4b10b0]     ;   (that - X) / 5.0
0041bfa7  else
0041bfad    fld [eax+8] / call 0x49c5ac               ; left: trunc(X)
0041bfb8    fild / fsubr [esi+8] / fdiv [0x4b10b0]    ;   (X - that) / 5.0
0041bfd3  if [ecx+0x18] and the time came out 0.0:
0041bfe3    fld [0x4b02d0] (1.0) / fdiv [0x4b10b0] (5.0)   ; = 0.2 s
```

So **X is an exact integer at every probe**: `trunc(X)+1` going right,
`trunc(X)` going left. `[ecx+0x18]` is the flag `0x0041BEF1` / `0x0041BEFA`
sets, and the 1.0/5.0 arm is what stops a probe-on-the-boundary looping — a
walker that has just probed gets a whole column of time, 0.2 s.

The probe `0x0041BFFC` does not write X at all (checked over the whole
function to its `ret 4` at `0x0041C872`: the only `fstp [reg+8]` is absent;
X is written only by `0x0042F66B`'s callers and the hoist).

### The probe's column and its two cells

`0x0041C062` and `0x0041C1BE` call `0x0042F22E`, which returns a biased
position:

```
0042f263  fld  qword [ecx+0x360] / fadd qword [eax+8]    ; X + [+0x360]
0042f275  fld  qword [eax+0x368] / fadd qword [edx+0x10] ; Y + [+0x368]
```

and the probe turns that into one column and two cells:

```
0041c099  fld [ebp-0x4c]  (X + [+0x360]) -> trunc -> wrap 0x40dec0 -> trunc
0041c0c2  mov [ebp-0x14], eax                 ; THE PROBE COLUMN
0041c0c5  fild [ecx+0x44] / fsubr [ebp-0x44]  ; (Y + [+0x368]) - vdir
0041c0f4  mov [ebp-0x1c], eax                 ; row F-1
0041c104  call 0x40de60 / mov [ebp-0x18],[eax] ; CELL BELOW THE FEET
0041c10e  fld [ebp-0x44]  (Y + [+0x368])      -> trunc -> clamp -> trunc
0041c137  mov [ebp-0x1c], eax                 ; row F  (the feet)
0041c147  call 0x40de60 / mov [ebp-0x10],[eax] ; CELL AT THE FEET
```

`[+0x360]` is 0.0 facing right and −1.0 facing left (`0x0042F66B`:
`0x0042F6A4`/`0x0042F6AE` write `0xbff00000`, `0x0042F6BD`/`0x0042F6C7` write
0.0); `[+0x368]` is 0.0 upright (`0x0042E109`, `0x0042E120`).

**Therefore the probe column is `B` facing right and `B − 1` facing left,
where `B = trunc(X)` is the integer X already sits on.** Both are the cell
ahead of the cell the walker was in before the boundary; what differs is
whether the walker's own `trunc(X)` is that cell (right) or the one behind it
(left).

### The arms, and what each returns

```
0041c42d  and [ebp-0x10], 0x800    ; feet cell, blocker field (round 8)
0041c47a  and [ebp-0x10], 1        ; feet cell solid?
0041c482    je 0x41c6ef            ;   no  -> the STEP-DOWN arm
0041c48e  add ecx,[eax+0x44]       ;   yes -> row F+1
0041c517  and ecx, 1 / je 0x41c6ae ;        clear -> ADOPT, rise 1
0041c528  add eax,[edx+0x44]       ;        row F+2
0041c5b1  and eax, 1 / je 0x41c6ae ;        clear -> ADOPT, rise 2
0041c5c1  mov dl,[ecx+0x48]        ;        three solid: the climber byte
0041c5c6    je 0x41c676            ;          0 -> 0x0042F66B, turn, RETURN 0
0041c5cc  push 0x20 / call 0x413750 ;         else the climb state
0041c625  mov [edx+0x340], eax
0041c646  mov [ebp-0xbc], 1        ;          RETURN 1 -> the slice loop stops
```

Adopt (`0x0041C6AE`) stores the row through `0x0042EE74` and **returns 0**, so
the walker keeps walking inside the same tick. The turn returns 0 too, which
is why a turned walker reverses and walks back within one tick.

### The step-down arm — exactly one cell, then a fall

```
0041c6ef  cmp [ebp-0x18], 0        ; the cell below the feet, at the probe col
0041c6f3    jne 0x41c838            ;   solid -> RETURN 0, Y UNTOUCHED
0041c6ff  sub edx,[ecx+0x44]       ; row := F-1
0041c732  cmp dword [eax], 0       ; the WHOLE dword, not a mask
0041c735    jne 0x41c809
0041c741  sub eax,[edx+0x44]       ; row := F-2
0041c774  cmp dword [eax], 0
0041c777    jne 0x41c809
0041c77d  push 0x28 / call 0x49c1a8 ; the FALL state
0041c7d7  call 0x42e696             ;  installed
0041c7dc  mov [ebp-0xd0], 1         ;  RETURN 1
0041c809  mov cl,[eax+0x4a] / test / jne 0x41c838   ; the zone exemption
0041c81b  add eax,[edx+0x44]       ; row := F-1
0041c833  call 0x42ee74            ; Y := F-1       -- ONE CELL DOWN
```

The first of the two dword tests is redundant with `[ebp-0x18]` — both read
cell(col, F−1) — so in practice the shape is: **F−1 solid ⇒ Y unchanged;
F−1 empty and F−2 solid ⇒ Y := F−1; F−1 and F−2 both empty ⇒ the fall
state.** `[+0x4a]` is zeroed at probe entry (`0x0041C078`) and set only in the
`[+0x54]`/`[+0x4c]` zone branch (`0x0041C429`), so it is 0 in ordinary play.

`0x0042EE74` stores an integer row, so an adopt or a step-down **snaps Y to a
whole number**; a walker crossing level ground keeps whatever fraction it had
(which is why live Y sits at 32.190 for columns on end).

### The climb's two probes

`0x004137CA`, slot +4 of vtable `0x004B0960`. The head test `0x004139BF` is
called only from `0x004138E8`, takes its position from `0x0042F35F`
(`X + [+0x350]`, `Y + [+0x358]`, and `[+0x358]` is the double 3.0), and makes
two samples:

```
004139f3  fild [ecx+0x40] / fsubr [ebp-0x24]   ; column = (X+[+0x350]) - facing
00413a58  cmp dword [eax], 0 / jne 0x413a70    ; non-zero -> "Climber hit head"
00413b0c  fld [ebp-0x24]                       ; column = X+[+0x350], NO facing
00413b1a  call 0x49c5ac
00413b22  sub eax, [edx+0x44]                  ; row = trunc(Y+[+0x358]) - 1
00413b50  call 0x40de60
00413b55  cmp dword [eax], 0
00413b58  jne 0x413bfd                         ; non-zero -> keep climbing
00413b5e  push 0x004bf310 ("Climber topping out")
00413b8e  push 0x18 / call 0x417c80            ; the flip state
```

So the ceiling samples **Y + 3** one column behind, and the top-out samples
**Y + 2** on the wall column itself. `World::stepClimber` uses
`rules_.lemmingHeight` (3) for both.

### The hoist is the `flip` animation, and its net move is X 0 / Y +2 — new

The top-out state's constructor is `0x00417C80`:

```
00417c92  mov dword [eax], 0x4b0af8   ; vtable 0x004B0AF8
00417c9b  mov [ecx+0x14], 0x1e        ; state id 30
00417ca5  mov byte [edx+0x11], 0
```

`read_steppers.py --states` names `0x004B0AF8`: enter `0x00417CCC`, step
`0x00417CE6`, exit `0x00417DD7`, animation **`flip`** (`0x004BF41C`, pushed at
`0x00417CD3`). The enter hook does nothing but set that animation.

```
00417d05  call dword [edx+0x2c]   ; advance the animation by the slice
00417d1b  call dword [eax+0x44]   ; HAS IT FINISHED?
00417d25    je 0x417dbf           ;   no -> zero the slice, return 0. Nothing
                                  ;         else happens: X and Y are untouched
00417d2e  fld [eax+8] / call 0x49c5ac
00417d39  add eax, [ecx+0x40]     ; X := trunc(X) + facing
00417d4b  call 0x40dec0 / 0x49c5ac / fstp [edx+8]
00417d64  fld [eax+0x10] / call 0x49c5ac
00417d6f  mov edx,[ecx+0x44] / imul edx,edx,3
00417d75  add eax, edx            ; Y := trunc(Y) + 3 * vdir
00417d99  fstp [eax+0x10]
00417da8  call dword [eax+0x48]   ; the time left over, handed back
00417db3  call 0x42f8b9           ; -> 0x0041BD60, a fresh WALK state, via
                                  ;    0x0042E696 (which runs the old state's
                                  ;    exit hook)
```

and the exit hook takes one unit back off each:

```
00417de4  fild [ecx+0x44] / fsubr [eax+0x10] / fstp [edx+0x10]  ; Y -= vdir
00417df6  fild [ecx+0x40] / fsubr [eax+8]    / fstp [edx+8]     ; X -= facing
```

Net: **X unchanged, Y + 2.** That is exactly round 4's live pair — 0.834 s
held at (61.000, 232.000), then feet at 63.000 with X still 232. And the hold
is the animation, not a tick count: `0x00417D1B` is the same "strip finished"
slot `+0x44` that the flapper uses at `0x00416AC6`, which round 7 measured as
`frames × 0.06 / divisor`.

`World::stepHoister` waits `l.frame >= 2`, moves X by the facing, and lifts
`lemmingHeight` (3). All three differ.

---

## Part 2 — the predictions, written before the live run

Geometry, from `build/climb.lvl` through `read_lvl_grid.find_grid` (offset
`0x3a987`, 256×96), file rows Y-up:

* floor solid everywhere at rows ≤ 31;
* a block at columns 100–159 solid rows 32–46 (top solid row **46**);
* a taller part, columns 112–143, solid to row 56;
* the climb ledge is columns **160–171**, feet row 32 on floor row 31;
* a pillar at columns **172–175**, solid rows 32–56 (top solid row **56**);
* another at 188–191, rows 32–47.

So the ledge is bounded by a 15-cell wall at column 159 on the left and a
25-cell pillar at column 172 on the right. A walker on it meets the 159 wall
facing left and the 172 pillar facing right.

**P1a** A right-facing climber's X is the wall column exactly: **172.000** on
the 172 pillar. A left-facing climber's X is the wall column **+ 1**:
**160.000** on the 159 wall. Falsified by a climber at 171.000 or 159.000.

**P1b** A left-facing **non-climber** walker on the ledge turns at X exactly
**160.000** and is never seen with 159.000 < X < 160.000. A right-facing one
turns at X exactly **172.000**. Falsified by any walker X strictly inside
(159, 160).

**P1c** Round 1's tick-8 sample — live column 159.163, walking — is therefore
**not a lemming X**. It is the billboard column, and the gap is the
billboard's own offset. Falsified if the actor list shows a walker X of
159.163 on that ledge.

**P2** A walker meeting a **1-cell** drop steps to Y = F−1 (Y snapped to a
whole number by `0x0042EE74`) and stays a walker; a **2-cell** and a **3-cell**
drop both put it in the fall state. Falsified by a walker that falls at a
1-cell drop, or steps down 2.

**P3** The last climbing Y is **55.000** on the 172 pillar (top solid 56) and
**45.000** on the 159 wall (top solid 46). Ours tops out one cell earlier, at
54 and 44. Falsified by a last climbing Y of 54 or 56 on the pillar.

> **Corrected after the live grid dump, and the correction is mine, not the
> engine's.** The rule under P3 — the last climbing Y is `topSolid − 1` — is
> what was predicted and it is what held. The arithmetic in the line above
> used the wrong top row for the 172 pillar because the map I printed before
> the run **stopped at row 56**; the pillar is solid to **59**, so the
> prediction should read 58.000. The 159 wall's 46 is right. The pen, not the
> ledge, is where the lemmings turned out to be, so the walls actually
> measured were 64, 80, 96 and 112 — see run 5.

**P4a** The hoist hold is the `flip` strip's length, `frames × 0.06 /
divisor`, not a whole number of 200 ms ticks. Round 4 measured 0.834 s, so the
strip should read **14 frames, divisor 1.0 → 0.840 s**. Falsified by a hold of
0.400 or 0.800 s, or by a `flip` strip whose `frames × 0.06 / div` is not
within a sample of 0.834.

**P4b** The hoist's net move is **X unchanged, Y + 2** — and so a left-facing
climber up the 159 wall tops out at Y 45, is placed at Y 47 with X still
160.000, and then **falls 15 cells back onto the ledge**, because column 160
is empty at row 47. That is a quirk, not a bug to smooth: the hoist only lands
a climber on the wall it climbed when it was facing right. Falsified by a
left-facing hoist that ends at X 159.

**P5** The float halving's delay is the `umbopen` strip's length. Round 3b
measured ≈0.48 s (9.6–9.7 world units at 20/s), so the strip should read
**8 frames, divisor 1.0 → 0.480 s**. Falsified by a strip whose
`frames × 0.06 / div` is not ≈0.48.

---

## Part 3 — the live runs

(filled in after the runs)
### Run 1 — `MM_CLIMB_002`, pid 24112, the geometry and the turns

`tools/harness/measure_steppers.py` (new). Stock exe, entered with the
`select_level` record swap (slot [0][0] restored: True), `ui_state` 0, clock
293 s, `loaded 'Compiled\MM_CLIMB_002.lvl'`.

**First correction, and it matters for every earlier round-1 number.** The
lemmings are not where round 1 put them. Off the actor list the whole pen is
**X 24.000 … 64.000 at Y 32.000** — not "columns 128–168 at Y 32.190". The
live grid (read through `read_grid`'s addressing, `0x004C6A10`) is:

```
col  23  solid rows 28..95   (the pen's left wall, to the grid top)
col  24  solid rows 28..31   (the pen floor; feet row 32)
...
col  63  solid rows 28..31
col  64  solid rows 28..47   (the pen's right wall, top solid row 47)
col  67  solid rows 28..47
col  68  solid rows 28..31
col 159  solid rows 28..46   (the ledge's left wall, top solid row 46)
col 160  solid rows 28..31   (the climb ledge, columns 160..171)
col 172  solid rows 28..59   (the ledge's right pillar, top solid row 59)
col 188  solid rows 28..47
```

The `.LVL` agrees cell for cell on every column checked (`build/climb.lvl`,
`find_grid` offset `0x3a987`) — **except that my pre-run map print stopped at
row 56 and so read the 172 pillar's top as 56 when it is 59.** The rule under
prediction P3 is unaffected; the arithmetic in it is corrected below.

So round 1's `159.163` was never a lemming X. **P1c holds**: the billboard
column it recorded is not `lemming+0x08`, and the tick-8 "divergence" was a
tracker artefact, not a stepper difference. Round 1's eight matched ticks were
matched against the same artefact.

**Turns, 31 of them in 25 s, 10 walkers, no input and no writes:**

| facing | turned at X | wall column | probe column = `trunc(X + [+0x360])` |
| --- | --- | --- | --- |
| +1 (right) | **64.000** | 64 | `trunc(64.000 + 0.0)` = 64 |
| −1 (left) | **24.000** | 23 | `trunc(24.000 − 1.0)` = 23 |

Every one of the 31 turns had X inside 0.045 of those integers, and over
346 293 samples **no walker's X ever left [24.000, 64.000]**: the maxima
observed were 63.998, 63.997, 63.993, 63.992, 63.990, 63.985, 63.980 and
exactly **64.000**; the minima 24.000, 24.005, 24.007, 24.010, 24.012, 24.013.
Nothing inside (23, 24) or (64, 65). **P1b holds.**

Raw, one turn, at the engine's own frame rate (X moves ~0.03 per frame, which
is ~6 ms, not per 200 ms tick):

```
  2.0892  x 63.9323  f +1  walk   strip 13 frame 11
  2.0966  x 63.9673  f +1  walk   strip 13 frame 11
  2.1033  x 63.9973  f +1  walk   strip 13 frame  0
  2.1034  x 63.9977  f -1  walk   strip 13 frame  0   <- turned
  2.1105  x 63.9577  f -1  walk   strip 13 frame  0
```

The `63.9977` is a mid-frame read: our sampler runs concurrently with the
engine's own frame, so it catches X between `0x0041BE95`'s store and the turn.
The committed values either side are 63.9973 and 63.9577, and the envelope
above is what pins the boundary at 64.000.

### Run 2 — one panel-assigned climber, the whole sequence

`--climber 70`. `assign_skill.assign(h, inp, 0, choose=...)` on a pen walker.
Proof of assignment is the stock: **`0x004C91D8[0]` went 10 → 9** and
`[lemming+0x48]` went 0 → 1. Nothing was written to the lemming.

Lemming `0x027164C0`, one 0x344-byte read per pass, 8958 object samples a
second:

| state | t (s) | held | X | Y |
| --- | --- | --- | --- | --- |
| walk | 0.000 – 6.433 | 6.433 | 31.812 → 63.967 | 32.000 |
| **climb** | 6.433 – 9.233 | **2.799** | **64.000 → 64.000** | 32.007 → 45.967 |
| **flip** | 9.233 – 10.073 | **0.841** | **64.000** | **46.000** |
| walk | 10.073 – 10.879 | 0.806 | 64.002 → 67.997 | **48.000** |
| fall | 10.879 – 11.678 | 0.799 | 68.000 | 47.872 → 32.032 |
| walk | 11.678 – 12.116 | 0.439 | 68.027 → 70.212 | 32.000 |

and the transitions, to the sample:

```
 6.433  walk  -> climb   x 63.967 -> 64.000   y 32.000 -> 32.007   f +1  climb 1
 9.233  climb -> flip    x 64.000 -> 64.000   y 45.967 -> 46.000
10.073  flip  -> walk    x 64.000 -> 64.002   y 46.000 -> 48.000
10.879  walk  -> fall    x 67.997 -> 68.000   y 48.000 -> 47.872
11.678  fall  -> walk    x 68.000 -> 68.000   y 32.032 -> 32.000
```

**P1a holds.** A right-facing climber's X is **64.000 — the wall column
itself**, held to every one of 145 254 samples. It got there because the walk
stepper moved X onto the boundary (`0x0041BE95`) and snapped it
(`0x0041BECE`) *before* calling the probe, and the probe never writes X.

**P3 holds.** Column 64's top solid row is **47** and the last climbing Y is
**46.000** — `topSolid − 1`, which is exactly what a probe at `Y + 2` gives
(`trunc(46 + 3) − 1 = 48`, the first empty row). Our `stepClimber` probes
`Y + 3` and would have topped out at 45.

**P4a holds, and names the strip.** The hoist held **0.841 s**. Strip 8 of the
live controller table is **14 frames, divisor 1.0 → 0.840 s**. It is not a
whole number of 200 ms ticks (4.205 ticks), and our `l.frame >= 2` is 0.400 s.

**P4b holds exactly.** Across the flip, **X 64.000 → 64.000 and Y 46.000 →
48.000**: net `dx 0`, `dy +2`. Not `+lemmingHeight`, and not `x += facing`.
The step's `X := trunc(X) + facing` / `Y := trunc(Y) + 3*vdir`
(`0x00417D39` / `0x00417D72`) and the exit hook's `X -= facing` /
`Y -= vdir` (`0x00417DF6` / `0x00417DE4`) both ran, and they compose to this.

Three things measured in passing, all agreeing with values already in `Rules`,
which is the sanity check on the sampler: the climb is 13.993 units in
2.799 s = **5.00 units/s** (`climbSpeed` 1 cell a tick); the walk on the
pillar top is 3.995 columns in 0.806 s = **4.96 columns/s** (`walkSpeed` 1);
the fall is 15.840 units in 0.799 s = **19.82 units/s**, the 20.0 at
`0x004B0A18` (`fallSpeed` 4).

And one step-down observation for item 2, free: at X = 68.000 the probe found
column 68's feet row 48, 47 and 46 all empty and took the **fall** transition
(`0x0041C77D`), falling down column 68 to feet 32 — a 16-cell drop falls, as
the read says.

### The live strip table

`--strips`, off the controller at `lemming+0x128`
(`[[+0x1F8]+0x0C] + ix*0x98`, frames at `+0x00`, divisor at `+0x10`):

| ix | frames | div | `frames*0.06/div` | known as |
| --- | --- | --- | --- | --- |
| 0 | 24 | 1 | 1.440 s | bash (round 5) |
| 1 | 16 | 1 | 0.960 s | |
| 2 | 16 | 1 | 0.960 s | build (round 6) |
| 3 | 18 | 1 | 1.080 s | |
| 4 | 20 | 1 | 1.200 s | dig (round 5) |
| 5 | 17 | 1 | 1.020 s | |
| 6 | 17 | 1 | 1.020 s | explode (round 7) |
| 7 | 15 | 1 | 0.900 s | |
| **8** | **14** | **1** | **0.840 s** | **flip — the hoist, this round** |
| 9 | 25 | 1 | 1.500 s | mine (round 5) |
| 10 | 8 | 0.5 | 0.960 s | shrug (round 6) |
| 11 | 15 | 1 | 0.900 s | |
| **12** | **8** | **1** | **0.480 s** | candidate for `umbopen` |
| 13 | 12 | 1 | 0.720 s | walk (observed on every pen walker) |
| 14 | 16 | 2 | 0.480 s | |

Strip 13 is `walk`: every walker in run 1 was on it. A strip record carries no
name pointer (`strip_name` searched all 0x98 bytes of 15 records and found no
ASCII), so index → name has to come from `[controller+0x1C8]` while the
animation plays. Strip 8 is the only entry whose cycle is 0.840 s, against a
0.841 s hoist. Strip 12 and strip 14 both give 0.480 s, so the `umbopen`
index is **not** settled by the table alone.

### Run 3 — trenches, `MM_CLIMB_002` pid 8044 (item 2)

Prediction, written before the write: with the pen floor's row 31 zeroed at
column 34 (one cell), at 42 (two cells, rows 31 and 30) and at 50 (three
cells, rows 31–29), a walker meeting the one-cell hole **steps down to Y 31
and stays a walker**, and the two- and three-cell holes both put it in the
**fall** state. Falsified by a fall at the one-cell hole or a step down of two.

Six cells zeroed with the whole-dword write the step-down arm's own test
implies (`cmp dword [eax], 0`), all six restored afterwards. 20 s, 10 walkers,
51 864 object reads/s, no input.

**One cell, column 34 — steps down, no state change, both facings:**

```
 2.223  0x02844c50  col 34  dy -1.000  x 33.987 -> 34.022  y 32.000 -> 31.000  f +1
 2.425  0x02844c50  col 35  dy +1.000  x 34.992 -> 35.027  y 31.000 -> 32.000  f +1
 0.910  0x028444c0  col 34  dy -1.000  x 35.010 -> 34.975  y 32.000 -> 31.000  f -1
 1.111  0x028444c0  col 33  dy +1.000  x 34.000 -> 33.965  y 31.000 -> 32.000  f -1
```

Seven lemmings crossed it, every one with `dy −1.000` and the state still
`walk`, then `dy +1.000` on the far side (the step-up arm putting it back).
The right-facing step happens at X = **34.000** (probe column 34) and the
left-facing one at X = **35.000** (probe column 34) — item 1's boundary again,
now on the step-down arm.

**Two cells, column 42 — falls, lands on row 29, then steps up 2:**

```
 0.938  0x02843650  col 42  walk -> fall  x 41.995 -> 42.000  y 32.000 -> 31.881
 1.035  0x02843650  col 42  fall -> walk  x 42.000 -> 42.000  y 30.061 -> 30.000
 1.236  0x02843650  col 43  dy +2.000     x 42.990 -> 43.025  y 30.000 -> 32.000
```

Eight lemmings, every one the same. The descent is 2 units in 0.097 s =
20.6 u/s, the 20.0 at `0x004B0A18`.

**Three cells, column 50 — falls, lands on row 28, and is then stuck:**

```
 2.632  0x02843650  col 50  walk -> fall  x 49.975 -> 50.000  y 32.000 -> 31.981
 2.785  0x02843650  col 50  fall -> walk  x 50.000 -> 50.000  y 29.061 -> 29.000
 0.597  0x02845c50  col 51  walk -> fall  x 51.003 -> 51.000  y 32.000 -> 31.870  f -1
 0.743  0x02845c50  col 50  fall -> walk  x 51.000 -> 50.993  y 29.090 -> 29.000  f -1
```

A three-cell rise is a wall (`maxStepUp` 2), so the hole fills up with
walkers oscillating between X 50.000 and 51.000. Reached from the left at
X = 50.000 and from the right at X = 51.000 — the two edges of column 50.

**P2 holds, unqualified.** `World::stepWalker` has no step-down arm and
becomes a faller at a drop of 1.

### Run 4 — a left-facing climber, pid 8044 (items 1, 3, 4)

To reach a left-facing top-out at all, column 23's wall (solid to the grid
top, row 95) was cut down: `--carve-clear 23:51 --trench-row 95` zeroed rows
45–95 of that one column, making its top solid row **44**. Restored after.
Prediction, before the write: the last climbing Y is **43.000** (`topSolid − 1`
again) and the hoist leaves X at **24.000**, the wall column **+ 1**.

Lemming `0x028474C0`, assigned with `--facing -1` so the engine's own pick
chose a lemming already walking left. Stock 10 → 9.

```
 4.830  walk  -> climb   x 24.027 -> 24.000   y 32.000 -> 32.013   f -1
 7.033  climb -> flip    x 24.000 -> 24.000   y 42.983 -> 43.000
 7.872  flip  -> walk    x 24.000 -> 23.977   y 43.000 -> 45.000
 8.073  walk  -> climb   x 23.007 -> 23.000   y 45.000 -> 45.028   f -1
17.874  climb -> walk    x 23.000 -> 24.034   y 93.999            TURN -1 -> +1
18.270  climb -> fall    x 25.000 -> 24.000   y 94.979 -> 94.962  TURN +1 -> -1
20.772  fall  -> splat   x 24.000             y 45.000
```

| state | held | X | Y | strip |
| --- | --- | --- | --- | --- |
| climb | 2.202 | **24.000** | 32.013 → 42.983 | 3 (18 frames) |
| flip | **0.839** | **24.000** | **43.000** | **8**, frame 0 → 13 |
| walk | 0.201 | 24.000 → 23.007 | **45.000** | 13 |
| climb | 9.801 | **23.000** | 45.028 → 93.999 | 3 |

**P1a/P1b hold for the left facing too.** The climber's X is **24.000**, the
wall column (23) **+ 1**, held over 112 634 samples, and the second climb up
column 22 held X = **23.000**. **P3 holds**: last climbing Y 43.000 against a
carved top solid row of 44. **P4a**: 0.839 s, strip **8**, animation frames 0
through 13 — fourteen of them. **P4b**: `dx 0.000`, `dy +2.000`.

**P4b's prediction of a fall-back was WRONG, and the real behaviour is worth
keeping.** I predicted that a left-facing hoist, landing at X 24.000 feet 45
over an empty column 24, would fall 13 cells back to the ledge. It does not.
The fresh walk state's very first probe fires immediately — `0x0041BF61`
returns 0.0 for a left-facing walker already on an integer X, which is inside
the slice, so `0x0041BECE` snaps and `0x0041BEE6` probes at once — and it
probes the column **ahead**, 23, whose feet row 45 is empty with row 44 solid
under it. That is the step-down arm's "Y unchanged, return 0" case, so the
walker is told to walk on and steps onto the wall it just climbed. It is
never asked whether its own column holds it up. The lemming spends one
instant unsupported and then walks left onto solid ground.

Two bonus confirmations from the same lemming, neither aimed for:

* it climbed column 22 to the grid ceiling, hit head at Y 93.999, and
  `0x00413A79`'s `X = X − facing` put it at **24.034** before the turn —
  the ceiling arm, read in round 4, seen firing;
* it then fell from Y 94.979 to **45.000** — i.e. down column
  `trunc(24.000 − 1.0) = 23`, whose carved top was 44 — so the **faller uses
  the same `[+0x350]` facing bias** as the walk probe. 49.96 units in
  2.502 s = 19.97 world units/s, and 50 > `splatDistance` 30, so it
  **splatted** (`0x004B0668`).

### Run 5 — five climbs and the umbrella, pid 16464 (items 1, 3, 4, 5)

`--climber 35 --facing 1 --poke-float --hz 2000`. Stock 10 → 9. The sample
rate was capped at 2 kHz this time: an uncapped single-lemming loop drove
~300 000 `ReadProcessMemory` a second and the process died at the end of both
long runs before this one.

`[lemming+0x49] = 1` was **written** — a PROBE, labelled as one, in round 3b's
shape and for its reason: `0x004C91D8` on this level reads
`[10,0,0,0,0,0,0,0,0,1]`, so there is no parasol to click. Nothing else was
written to any lemming in this round.

One climber walked right across the level and climbed **five** walls:

| wall column | top solid row | last climbing Y | flip held | after the flip |
| --- | --- | --- | --- | --- |
| 64 | 47 | **46.000** | **0.840 s** | x **64.000**, y **48.000** |
| 80 | 59 | **58.000** | **0.840 s** | x **80.000**, y **60.000** |
| 96 | 46 | **45.000** | **0.833 s** | x **96.000**, y **47.000** |
| 112 | 62 | **61.000** | **0.840 s** | x **112.000**, y **63.000** |
| 120 | — | still climbing at the cutoff | | |

(the top solid row is read back as `post-flip Y − 1`, and agrees with the grid
dump on column 64.)

Five climbs, five times X exactly the wall column; four top-outs, four times
`topSolid − 1`; four flips, every one 0.833–0.840 s on strip 8 with the
animation running frames 0 → 13; four placements, every one `dx 0.000`,
`dy +2.000`. **P1a, P3, P4a and P4b are settled.**

**Item 5 — `umbopen` is strip 12, 8 frames, 0.480 s.** The same lemming fell
twice, and the strip index is what names the animation:

```
  t  8.3850  y 47.990  fallen  0.000  strip  7  [+0x49] 1  [+0x4b] 0
  t  8.8920  y 37.850  fallen 10.140  strip 12  [+0x49] 1  [+0x4b] 0
      (16-unit drop; it landed at y 32.010 before umbopen could finish)
  t 18.4280  y 59.929  fallen  0.000  strip  7  [+0x49] 1  [+0x4b] 0
  t 18.9770  y 48.949  fallen 10.980  strip 12  [+0x49] 1  [+0x4b] 0
  t 19.4560  y 39.385  fallen 20.544  strip 11  [+0x49] 1  [+0x4b] 1
```

* strip **7** (15 frames, 0.900 s) is `fall`; strip **12** (8 frames, 1.0,
  **0.480 s**) is `umbopen`; strip **11** (15 frames, 0.900 s) is `float`.
* the switch to strip 12 happens as the distance passes **10** — the 10.0 at
  `0x004B03E8`, `floatAfter`, unchanged;
* `umbopen` ran **18.9770 → 19.4560 = 0.479 s** against the strip's
  `8 × 0.06 = 0.480`, and `[+0x4b]` went 0 → 1 on the same sample the strip
  became `float`, which is `0x004162C2` firing at `0x00416186`'s end-of-
  animation;
* the distance covered while it played was **9.564 world units**, inside
  round 3b's 9.6–9.7;
* the descent rate after the flag was **9.972 world units/s** — the 20.0 at
  `0x004B0A18` over the 2.0 at `0x004B03F8`, so `floatSpeed` 2 is untouched;
* and the **16-unit drop never halved at all**: 15.980 units in 0.805 s =
  19.85 u/s for the whole descent, because `umbopen` needs 0.480 s after the
  0.5 s it takes to fall ten units and the drop was over first. Our
  `stepFaller` halves the moment the distance passes 10 and would have taken
  five ticks over that drop where the original took four.

**P5 holds: it is a fixed animation length, 480 ms, not distance-dependent.**
The distance equivalence (9.6 units) is a consequence of the speed being
unchanged while it plays.

The `descent rate before the flag: 0.729 world units / s` line the harness
printed is meaningless — the "before" group spans both falls and the climb
between them. Recorded so nobody quotes it.

### Index → name for the animation strips, as far as this round can say

| ix | frames / div | cycle | named by |
| --- | --- | --- | --- |
| 0 | 24 / 1 | 1.440 s | bash, round 5 |
| 2 | 16 / 1 | 0.960 s | build, round 6 |
| **3** | 18 / 1 | 1.080 s | **climb, this round** (every climb state) |
| 4 | 20 / 1 | 1.200 s | dig, round 5 |
| 6 | 17 / 1 | 1.020 s | explode, round 7 |
| **7** | 15 / 1 | 0.900 s | **fall, this round** |
| **8** | 14 / 1 | **0.840 s** | **flip — the hoist, this round** |
| 9 | 25 / 1 | 1.500 s | mine, round 5 |
| 10 | 8 / 0.5 | 0.960 s | shrug, round 6 |
| **11** | 15 / 1 | 0.900 s | **float, this round** |
| **12** | 8 / 1 | **0.480 s** | **umbopen, this round** |
| **13** | 12 / 1 | 0.720 s | **walk, this round** |

Still unnamed: 1 (16/1), 5 (17/1), 14 (16/2). Not guessed at.

---

## Part 4 — round 1's gate

### The walk and the climb now match, off the actor list

`tools/harness/measure_steppers.py --compare walker|climber` is round 1's
comparison with the actor list where it used a billboard. It drives the same
printer, `build/cmake/dev-debug/compare_tick.exe`, which is unchanged.

How the ticks are found matters, so it is written out. The engine has no tick
in the walk: X moves continuously at 5.0 world units a second and the probe
fires when X reaches an integer, which is every 200 ms. **So the probe
instants are the ticks.** Sampling on a fixed 0.200 s grid off our own clock
drifts against them — the first attempt landed on 49.975 and 53.965 where the
engine's instants were 50.000 and 54.000, and reported five spurious
divergences — so `tick_series` groups the samples by the integer the moving
coordinate is nearest and takes the closest sample in each group. The largest
residual it used is printed with every run; the typical one is 0.014 world
units, which is half an engine frame.

| run | result |
| --- | --- |
| walker, right-facing, open pen floor, cells 39 to 63 | **MATCH 24 ticks** |
| walker, left-facing, cells 57 to 37, then a left turn at the wall | **MATCH 27 ticks, diverge at 28** (see below) |
| walker, right-facing **through the wall turn** at column 64 | **MATCH 12 ticks** |
| climber, left-facing, column 23 wall, X held 24.000 | **MATCH 16 ticks** |
| climber, right-facing, column 64 wall, X held 64.000 | **MATCH 13 ticks** |

The right-facing turn run is the differential proof of item 1:

```
  | tick | live cell | ours | live x |
  |  5 | 63,63 | 63,63 | 63.001 |
  |  6 | 64,63 | 64,63 | 63.994 |   <- ONTO the wall column, both sides
  |  7 | 63,63 | 63,63 | 63.019 |
```

**The climb compare round 1 has owed since the start is done**, both facings,
and it needed a panel assignment (stock 10 to 9) rather than a flag poke.

### `compare_tick.py --only walker` re-run, and why its number is worthless

Run verbatim as asked, same process, same level:

```
walker locked 0x02846108  col 168.042  y 32.190  dcol +0.999  pen False
walk: matched for 8 ticks
SUMMARY walk match 8  climb skipped None  pid 37044  ui_state 0  path 'Compiled\MM_CLIMB_002.lvl'
```

So it no longer diverges at tick 8 — but **the thing it locks is not a
lemming.** Measured, same process, same instant:

* the actor list held **eight** lemmings, at X **28.264, 36.102, 42.696,
  48.601, 50.534, 56.439, 57.129, 63.033**, all at Y exactly **32.000**;
* `classify` returned about 2100 quads, hundreds of them sitting at exactly
  224.000 — terrain mesh;
* nothing in the actor list is anywhere near column 168, and nothing can
  reach the 160-171 ledge without climbing (no climber was alive);
* tracking the locked quad and all eight lemmings together for 1.06 s, the
  quad's column went 168.212 to 164.162 — **minus 3.8 units/s**, not the 5.0 a
  walker moves — and **no lemming's X shadowed it with a constant offset**:
  the eight differences all drifted by 1 to 9 units over that second.

Its Y of 32.190 is not a lemming Y either: every lemming in that pen reads
32.000 to three decimals. **So round 1's "8 ticks matched" and its "diverge at
tick 8" are both artefacts of a quad that is not a lemming**, and the
pre-round-10 claim that our walk agreed with the original for eight ticks had
no evidence behind it. The handoff's standing advice — prefer the actor list
to billboards for anything per-tick (round 3b) — is confirmed, and
`compare_tick.py`'s billboard path should not be used again.

---

## Part 5 — what changed in the sim

`src/sim/World.h` gained four fields, each with its addresses and its series
in the field comment:

| field | value | status |
| --- | --- | --- |
| `climberTopOut` | **2** | READ `0x00413B22`; MEASURED, five climbs on four walls |
| `hoistCycleMs` | **840** (14 x 60) | READ strip 8; MEASURED 0.833-0.841 s, four hoists |
| `hoistLift` | **2** | READ `0x00417D72` + `0x00417DE4`; MEASURED, five placements |
| `floatOpenMs` | **480** (8 x 60) | READ strip 12; MEASURED 0.479 s / 9.564 units |

`lemmingHeight` is now the ceiling bias only, and its comment says so. No
measured field moved: `ticksPerSecond` 5, `walkSpeed` 1, `climbSpeed` 1,
`fallSpeed` 4, `floatSpeed` 2, `floatAfter` 10, `splatDistance` 30,
`maxStepUp` 2, `lemmingHeight` 3, `brickLength` 2, `blockerFieldHeight` 4,
`bomberFuseMs` 5000, `exploderCycleMs` 1020 and the skill cycles are all
untouched.

`src/sim/World.cpp`:

* **`stepWalker`** advances `l.x` onto the probe column when facing right, on
  the wall arm and on the blocker arm both (`settleOnProbeColumn`), and has a
  real **step-down arm**: level ground leaves Y alone, one empty cell with
  something under it steps down exactly one, two empty cells fall. The adopt
  arm no longer runs a fall test, which is the original's shape.
* **`stepClimber`** tests the ceiling first (the original's order), on the
  open column behind at `lemmingHeight`, and the top-out on the **wall
  column** at `climberTopOut`; the ceiling arm now writes `X -= facing`
  (`0x00413A79`) before turning.
* **`stepHoister`** holds for `hoistCycleMs`, lifts `hoistLift`, and **does
  not move X**.
* **`stepFaller`** delays the halving by `floatOpenCells()` — `floatOpenMs` at
  `fallSpeed`, which comes to 10 cells, so the halving begins at
  `floatAfter + 10` against the original's 19.6.

### Tests

`test_world` gained three, all with their numbers written out rather than
derived from the field under test:

* `testWalkerStepsDownOneCell` — one-, two- and three-cell holes;
* `testClimbWallColumnTopOutAndHoist` — the climber's column, its last
  climbing row, the four-tick hold, and where it ends up;
* `testFloatHalvingIsDelayed` — the per-tick Y series of a 16-cell drop that
  never halves and a 28-cell drop that does.

`testBlockerTurnsWalkers`'s right-hand case moved from `turnedAt == 129` /
`furthest == 129` to **130**. Round 8's probe column is unchanged — it is the
field's own cell either way — but the stored column is now the original's, and
round 8's own live number ("field column 170 -> five right-facing walkers
turned at X = 170.000") already said so.

**Verified against the old behaviour** by reverting all five stepper shapes in
place (keeping the new fields so it still compiled) and re-running: **12
failures, every one behavioural** — a state transition, a position, or a tick
count, not a field assertion:

```
  FAIL  and never became a faller                                   (step-down)
  FAIL  the climber stands ON the wall column 120 (got 119)
  FAIL  its last climbing row is 41, one inside the wall (got 42)
  FAIL  the hoist holds for four ticks (got 2)
  FAIL  and holds X on the wall column (got 119)
  FAIL  tick 4: y 79 (got 77)                                       (16-cell drop)
  FAIL  and it is walking on the fifth tick
  FAIL  tick 4: y 67 (got 65)                                       (28-cell drop)
  FAIL  tick 5: y 71 (got 67)
  FAIL  tick 6: y 73 (got 69)
  FAIL  turned in cell 130 (got 129)
  FAIL  never went past the blocker's column: furthest 129
```

Restored, rebuilt, suite **16 of 16**, `test_world` 258 checks 0 failures.

---

## Part 6 — left open, with citations

* **A left-facing turn costs us a tick and costs the original nothing.** New
  this round, not one of the five, and measured: `--compare walker` matched 27
  ticks and then diverged at the left-facing turn at X = 24.000 against the
  column-23 wall — live went 24, 25, 26, 27; ours went 24, **24**, 25, 26. The
  cause is read: the turn arm sets the probe's return to 0
  (`0x0041C67E`, after `0x0041C679` calls `0x0042F66B`), so `0x0041BDD4`'s
  slice loop keeps going and the walker reverses inside the same frame; at the
  clean integer phase there is no time left in the frame, so the next 200 ms
  moves it a whole column back. Our stepper spends the tick turning. Going
  right the same thing is invisible, because the X advance onto the wall
  column substitutes for the lost tick — which is why that case now matches.
  Fixing it means letting `stepWalker` take its step after a turn, which also
  perturbs round 8's left-hand turn expectation, so it was left alone rather
  than changed at the end of a round.
* **The ceiling arm's `X -= facing` is READ and was seen twice; which state it
  installs is read but not isolated live.** `0x00413AA7`'s `push 0x28` builds
  the fall state through `0x00415FA0` — the same constructor the step-down
  arm's fall uses — so `stepClimber` keeps `State::Faller`. Live, a climber
  that reached the grid ceiling showed one `climb -> walk` sample and one
  `climb -> fall` sample; the first is a single-sample read of `[+0x340]`
  mid-swap and is not evidence against the instruction.
* **The flip state's `[+0x14] = 0x1e`** (`0x00417C9B`) is a state id, like the
  death state's `0x1a` (round 3b). Not read further.
* **`[+0x4a]`, the step-down arm's Y-write suppressor**, is zeroed at probe
  entry `0x0041C078` and set only in the `[+0x54]` / `[+0x4c]` zone branch
  `0x0041C426`. Round 8 read `[+0x4c]` as 0 on every live lemming, so it never
  fires in play. Not modelled.
* **The `0x8000` cell bit has a whole arm in the walk probe**,
  `0x0041C151`-`0x0041C321`: it queries `0x0042ECC9(0x2b)`, walks a collection
  at `0x004C8958`, and on a hit allocates a `0x20`-byte state through
  `0x0041AD90` and a `0x28`-byte one through `0x00415FA0`, installing both
  with `0x0042E696`. First sighting of that bit doing anything. A gadget
  round's, not this one's.
* **Strips 1 (16/1), 5 (17/1) and 14 (16/2) are still unnamed.** A strip
  record carries no name pointer — `strip_name` searched all 0x98 bytes of
  fifteen records for an ASCII string and found none — so naming one needs
  `[controller+0x1C8]` sampled while it plays.
* **The hoist's four-tick hold is 0.800 s against the 0.840 measured**, one
  tick early, because `hoistCycleMs` is 4.2 ticks and we act on the tick that
  covers the cycle's end (rounds 5-7's convention). Likewise
  `floatOpenCells()` rounds 9.6 cells to 10, so our halving starts at
  `fallDistance` 20 against the original's 19.6. Both stated, neither tuned.
* **`compare_tick.py` is left exactly as round 1 wrote it**, billboard path
  intact, because it is that round's regression case. Part 4 says why its
  number must not be quoted.

---

## The game process at the end of the round

Four processes were used, each killed and relaunched by
`dump_grids.restart_game` when the previous one died: **24112** (runs 1 and
2), **8044** (runs 3 and 4), **16464** (run 5), **37044** (the comparisons).
Nothing died in a way this round did not cause.

**Two of them died at the end of a long single-lemming sampling run.** An
uncapped loop over one lemming drove about **300 000 `ReadProcessMemory` a
second** (21.9 million passes in 70 s) and the process was gone when the next
command attached. Capping the rate with `--hz 2000` — ten samples per engine
frame, which is more than enough — was used for every run after that and no
process died again. Anyone sampling one object should cap it.

Every live write was undone: six trench cells restored, 51 carved wall cells
restored, and `select_level`'s slot [0][0] put back on all four entries
(`slot [0][0] restored: True` each time). The grid is reloaded from the `.LVL`
on every level load in any case, which is round 4's finding and still holds.

The only writes to a lemming this round were **one** `[+0x49] = 1` in run 5, a
labelled probe, because `MM_CLIMB_002` stocks no parasol. Every climber came
from the panel, with the stock count going down as the proof.
