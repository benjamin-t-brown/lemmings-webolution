# Phase 3 Handoff — The simulation

2026-10-14

Lemmings Webolution — a 1:1 reimplementation of Lemmings Revolution. This is the
dispatch plan for Phase 3. It assumes the reader has `PLAN.md` and
`docs/PHASE2_HANDOFF.md`. The evidence standard is
`.claude/rules/match-the-original.md`.

One agent does one round. The phase is nine rounds. A session that tries to
finish the phase will die in the middle, and its last line is not a result.

---

## The goal

A headless, deterministic simulation that matches the stock 640×480 executable
tick for tick. No rendering in the step.

Done when the same input, on a retail level, produces the same lemmings as
the running original: position, state, and the tick a skill acts. A green
`test_world` only shows the sim agrees with itself. Agreement with the
original is a live series through `tools/dinput-shim`.

The oracle in `PLAN.md` §2 is the acceptance test:

1. Drive the original to a known level.
2. Each 200 ms tick, read positions, states, counters, and terrain.
3. Step `webo::sim::World` with the same input and assert they match.

The harness is **`tools/harness/measure_steppers.py --compare walker|climber`**,
which reads the actor list at `0x004CBB18`. `compare_tick.py` and
`build/cmake/dev-debug/compare_tick.exe` are **SUPERSEDED** — the quad that
script fits is not a lemming, so its numbers (including "open-floor walk
matched 8 ticks") are withdrawn; see round 10. File grid is Y-up; Terrain
flips on load, by truncation, to `height - 1 - y` — **not** `floor(96 - y)`.
Current agreement: walker 24 ticks open floor, 12 through a right-facing wall
turn; climber 16 ticks left-facing, 13 right-facing. Later rounds are not
allowed to change a `Rules` field without a series from it.

---

## What already exists

Do not rebuild this, and do not remeasure the climb.

`src/sim/World` already steps a walker, faller, floater, climber, hoister,
blocker, builder, basher, miner, digger, and bomber. The shape is classic
Lemmings. Most of the numbers are still that game's.

| Field | Status | Value | Do this |
| --- | --- | --- | --- |
| `ticksPerSecond` | Measured | 5 | Leave it. 200 ms, not frame-locked. |
| `walkSpeed` | Measured | 1 column / tick | Leave it. |
| `climbSpeed` | Measured | 1 world unit / tick | Leave it. Actor-list compare (`measure_steppers.py --compare climber`); the old 34.190 / 35.190 / 36.190 billboard series is **withdrawn** (round 10). |
| `fallSpeed` | Read + live walk-off | 4 | Confirmed: 48.190→44→40→36 (−4.000). `phase3-round2.md`. |
| `floatSpeed` | Read + live series | 2 | Done. It is `fallSpeed / 2`: `0x00416038` divides 20.0 by the 2.0 at `0x004B03F8`. |
| `floatAfter` | Read + live series | 10 | Done. 10.0 at `0x004B03E8`, umbrella latch at accumulator 10.0. |
| `splatDistance` | Read + live probe | 30 | Done. 30.0 at `0x004B10A8`; accumulator held at 29 survives, 31 dies. |
| `maxStepUp` | Read + live series | 2 | Done. Probe `0x0041BFFC` is unrolled exactly twice (`0x0041C48E`, `0x0041C528`). |
| `lemmingHeight` | Read + live series | 3 | Done. `[+0x358]` is the double 3.0, `mov [ecx+0x35c], 0x40080000` at `0x0042E0FC`. |
| builder / basher / miner / digger / bomber fields | Classic; static notes exist | see `World.h` + round5–7 scratch | Animation-% bites and fuse 5.0 s are read. Intervals not yet tick counts. `diggerDepth` is 1 because it could not halve. |
| `climberTopOut` | Read + live series | 2 | Round 10. `0x00413B22` subtracts `[+0x44]` once where the ceiling arm subtracts it three times; five climbs on four walls. **Not** `lemmingHeight`. |
| `hoistCycleMs` | Read + live series | 840 | Round 10. Strip 8 `flip`, 14 frames x 0.06. Held 0.833-0.840 s, five hoists. At a 200 ms tick any value 800-999 behaves identically — the read is the evidence, not a test. |
| `hoistLift` | Read + live series | 2 | Round 10. The step's `Y += 3` and the exit hook `0x00417DD7`'s `Y -= 1` compose; X is left on the wall column. |
| `floatOpenMs` | Read + live series | 480 | Round 10. Strip 12 `umbopen`, 8 frames. The halving flag `[+0x4b]` flipped 0.479 s / 9.564 units after the umbrella opened. |

The fall read is vtable `0x004B0A20`, accessor `0x00416038`, constant 20.0 at
`0x004B0A18`, times the measured 0.2 s tick. An earlier read took 25.0 from
`0x004B0C60` and was the wrong state. Do not put `fallSpeed` back to 10, and
do not copy `0.467` from `LEMMINGSLO.SPR` — that is the animation clock.

The terrain under the sim is the stored 256×96 grid, one cell per world
unit. `rasteriseSurface` is not the mask.

`World::spawn` **is** the original's now (Round 3, done). `findHatchRecords`
parses the hatch's stored `(count, x, y)` words and `World::spawn` drops from
them; `Entrance::source` says which read filled a slot, and the
`Entrance*.a3d` translation is only the fallback for hand-built levels. All
102 retail levels have a hatch record (144 in all). Do not re-derive this.

Two things about it the next rounds must not undo. A world Y becomes a file
row by **truncation** (`0x0049C5AC` sets x87 rounding to chop), so the word
42 is file row 42 and terrain row `height - 1 - 42`; `worldToMaskPixel`'s
`floor(worldH - y)` is one cell off for an integer y and still feeds
`LevelStats`, so the two mappings coexist on purpose. And the y word being
the entrance world Y minus 2 holds on all 144 records but is an
**observation, not a rule** — nothing computes it, and the code reads the
stored word.

The corpus survey is 99 of 102 releasable on the stored grid after the Y flip
(81 was the pre-flip pin). It drops from the *translation*, not from the
spawn words, so it is no longer a statement about where lemmings start. Do
not tune anything to move that number.

---

## Units

One cell is one world unit. Height deltas are in world units.

`tools/harness/live_terrain_check.py` still has `PIXELS_PER_UNIT = 2`, and
`measure_motion.py`, `measure_fall.py`, and `check_mask.py` import it. That
2 is the old raster, two rows per world unit. A height change of 1.000 is
one cell, not two. Do not multiply a world-unit delta by 2 and call it a
mask row.

Live Y is a double. The climb held a fraction of `.190` while the integer
part stepped by 1. Compare integer world units to our cell. Record the
fraction. Our step has no floating point on purpose (`World.h`). Do not add
doubles to the sim because the fraction is nonzero. Stop and write it up if
a series shows the fraction changing which cell a probe hits.

---

## What this phase is not

- Parsing. Object-zone bits and the blocker bit are named in
  `CollisionGrid.h`. The blocker is Round 8 (sim acts on it). Anti-grav's
  `0x8000` is Round 9 (`Terrain::antiGrav`). The `0x200`/`0x2000`/`0x4000`
  zone stamps and landing-path splat exemptions are Round 29.
- Rendering. Sprite scale, `SurfaceMesh` fits, and PolyList tails stay put.
- The shell. Menus, saves, and music are Phase 5 (**done**; FMV still open).
- `PLAN.md`. Park prose in `docs/scratch/phase3-roundN.md`. One editor lands
  `PLAN.md` afterwards. Reconciled 2026-10-14 into §1.6 "Where to go next",
  §2.3 Next, Phase 3 Handoff, and risk 2.
- A rewrite of the steppers into a more classical shape. The steppers stay
  until a cited instruction or a live series disagrees with them.

---

## The rounds

Each round has one question. A clean negative is an answer. Do not start
the next round in the same session.

| Round | Question | Needs the game? | Done when |
| --- | --- | --- | --- |
| 1 | Can we compare one live lemming to `World` per tick? | Yes | **Done, in round 10** — through the actor list `0x004CBB18`, not billboards; round 1's own two numbers are retracted |
| 2 | Does a walk-off fall at 4 world units per tick, and which state is it? | Yes | **Done** — step 4, vtable `0x004B0A20` confirmed live, umbrella 10 and splat 30 measured in the float/splat round |
| 3 | Does spawning from the hatch int16s match the first live sample? | Yes, to check | **Done** — yes, 144 records across 102 levels, survey untouched |
| 4 | What are `maxStepUp` and `lemmingHeight`? | Static first | **Done** — 2 and 3, read and then bounded by a live series on two levels |
| 5 | What does the shared bash / dig / mine stepper remove, and when? | Static, then one live skill | **Done** — all three measured; the removal is a dword 0 and the interval fields are gone |
| 6 | When does a builder lay a brick? | Static, then live | **Done** — two cells at percent 81, 16-frame cycle, 12 bricks then a shrug |
| 7 | How long is the fuse, and when does Ohno play? | Static, then live | **Done** — 5000 ms, Ohno at the end, crater a further 1020 ms later from a stored 7x7 stencil |
| 8 | What does blocker bit `0x800` do to a walker? | Live | **Done** — a blocker stamps one column × four rows; the walk probe turns on it; `blockedBy` deleted |
| 9 | Gadgets, one mechanic per agent | Live | **Done (anti-grav + trampoline + water/acid drown)** — `0x8000` / `0x1000` / `0x40`. Teleport / other hazards still open |
| 10 | The four stepper differences round 4 left open | Live | **Done** — all four settled; four fields added, round 1's billboard numbers retracted, the actor list replaces `compare_tick.py` |
| 11 | The exit save box | Live | **Done** — object-overlap AABB 2×3 inclusive via `0x00430BA1`; not the guessed 8×8 / 12×14. Measured on `Exit.a3d`; other three models and the extent pin closed in round 12 |
| 12 | Finish the exit (other models + pin extent) | Live | **Done** — all four models share 2×3; column is `round` not `floor`; `testExitTriggerFromPose` pins the extent |
| 13 | Trampoline (gadget repeat) | Live | **Done** — cell bit `0x1000` → bounce `0x004B0810`; runtime-only; `pressure_pad.a3d` clean negative |
| 14 | Water hazard (gadget repeat) | Live | **Done** — cell bit `0x40` → drown `0x004B0570` ~2.0 s; not Entrance_water / zone `0x200` |
| 15 | Acid hazard (gadget repeat) | Live | **Done** — same `0x40` drown as water; type `0x2e` unused; `0x100` = `lem[+0x5f]` only |
| 16 | Teleport (gadget repeat) | Live | **Done** — object AABB `teleport.a3d` / `0x004B1250`; partner relocate; fx-only `teleport_fx`; no cell bit |
| 17 | SpeedPad (gadget repeat) | Live | **Done** — `speedup_two_way.a3d` / `0x004B1178` → FASTER 5.0 s / `+0x50=2` (~2×); no cell bit |
| 18 | Balloon (gadget repeat) | Static | **Done** — clean negative: cue registered/preloaded/unloaded, never played; no gadget |
| 19 | Fire (gadget repeat) | Static | **Done** — clean negative: cue unused; no fire a3d / no play refs |
| 20 | Witch (gadget repeat) | Static | **Done** — UI easter egg on e10_kf_1 pause; no lemming gadget |
| 21 | Lever (gadget repeat) | Live | **Done** — `lever.a3d` / `0x004B1288` facing-gated `+0xb8` toggle + msg fanout; no lem attach |
| 22 | ForceF (gadget repeat) | Static | **Done** — clean negative: cue unused; no force a3d |
| 23 | Axe (gadget repeat) | Static | **Done** — HUD sprite sheet only; no lemming gadget |
| 24 | Bubble (gadget repeat) | Static | **Done** — SFX on fluid type `0x23`; no Bubble gadget |
| 25 | Scrape (gadget repeat) | Static | **Done** — dig/bash steel-refuse SFX only; no gadget |
| 26 | Rumble (gadget repeat) | Static | **Done** — object state-commit SFX; no Rumble gadget |
| 27 | Changeop (gadget repeat) | Static | **Done** — UI skill/volume SFX; no Changeop gadget |
| 28 | Die (gadget repeat) | Static | **Done** — SFX on type-0x23 enter (subtype 1); no Die gadget |
| 29 | Splat-exemption zones | Live | **Done** — Fluid `0x200`/`0x4000` + sheet `[+0x4c]` skips splat |
| 30 | Inverted steppers | Live | **Done** — gravity-flipped step-up/climb/dig; refuse `0x20` |
| 31 | Cell-flag writers | Static | **Done** — `0x100` authored-only; `0x400` door/gate stamp |

Round 9 repeats. It is not one round with seven mechanics.

Round 9 has a written, self-contained brief at **`docs/ROUND9_BRIEF.md`**.
Per-gadget briefs: `ROUND13` … `ROUND28` Die; residue `ROUND29`–`ROUND31`.

**Rounds 11–31 are closed.** **Round-9 cue/gadget repeats are done.**
**Phase 3 residue list is empty** (splat-exemption, inverted steppers,
cell-flag writers all closed).

**Phase 3 polish (deferred, not residue):** bash `0x4` right-facing unread;
blocker fall-land; hatch facing isolation; teleport stream-id pairing;
lever message fanout; `"Cylinder"` still byte-scanned. See `PLAN.md` §1.6
“Where to go next”.

**Do not dispatch a "Phase 2 close-out" round.** That list in `PLAN.md` §3 was
stale and is corrected there as of 2026-10-16: the chunk walk is in
`CollisionGrid.cpp` and lands on the grid in all 102 levels, every cell bit has
a reader instruction behind it, and `rasteriseSurface` is already out of the
collision path (only `LevelStats`' survey and `lrtool` call it). Former
unread-writer residue (`kCellFlag0x100` / `kCellFlag0x400`) closed Round 31.
The `"Cylinder"` chunk is still found by a linear byte scan (not a Phase 3
residue target).

---

## Round 1 — status

**Closed by round 10, and both of its numbers are retracted.** Round 1
reported a walk that matched 8 ticks on empty floor and then diverged at
tick 8. Both came from a billboard quad that is **not a lemming** — at one
instant the actor list held eight lemmings at Y exactly 32.000 while the
locked quad sat at Y 32.190 and drifted at 3.8 world units a second, where a
walker moves 5.0. Do not quote either number. The replacement, through the
actor list at `0x004CBB18`: walker **24 ticks** on open floor and **12
through the right-facing wall turn**, climber **16 ticks** left-facing and
**13** right-facing — the climb compare this round owed from the start.
Scratch: `docs/scratch/phase3-round1.md` (historical),
`docs/scratch/phase3-round10-steppers.md` (current).

**The standing lesson.** Fitting a quad to a lemming is a fit, and it fitted
the wrong object for nine rounds without anyone noticing, because a plausible
series came out of it. Prefer the actor list for anything per-tick.

**Question (original).** Build the per-tick comparison, and prove it on
behaviour that is already measured. Do not learn new physics in this round.

**Why this is first.** Every later correction is otherwise another
self-test. The walk and the climb are the calibration set because their
numbers are already known. If the harness cannot reproduce them, it is
wrong, and it must not be used to "discover" the fall.

**Owns.** A new script under `tools/harness/`, named for what it does
(`compare_tick.py` or similar). A scratch note at
`docs/scratch/phase3-round1.md`.

**Must not touch.** `src/sim/World.h` `Rules` values, `World.cpp` steppers,
`SurfaceMesh.cpp`, `CollisionGrid.cpp`, `PLAN.md`, `test_corpus` expected
numbers, `test_collisiongrid`.

**Procedure.**

1. Reach `MM_CLIMB_002` with `tools/harness/goto_level.py`. Read
   `docs/PHASE2_HANDOFF.md` "Driving the original" before the first key.
   RETURN only when the menu highlight is exactly 0. Both capture paths
   return black. Changing level means restarting the process.
2. Reuse the billboard probe in `measure_motion.py`: four vertices, shared
   Y, centroid, column `atan2(-x, z) / 2π * 256`. Height is world Y. Do not
   import `PIXELS_PER_UNIT`.
3. Record one walker for a few ticks. Expected, already measured: column
   changes by 1.000 per 200 ms step, Y stays 32.190 while walking. The pen
   is about columns 128–168. Other walkers sit near columns 5–31.
4. Record one climber on the column-128 wall. Expected: column frozen, Y
   integer part +1 per tick. Do not click the skill panel. The level
   auto-selects the climber when the first skill stock is the climber.
   `restart()` in our sim does the same. A sandbox with unlimited counts
   stays on the digger — that is our bug to avoid, not a new measurement.
5. Step a `World` loaded from that level's stored grid, with the same
   starting cell, for the same number of ticks. Print live integer cell,
   our cell, and the live fraction, one line per tick.
6. Stop at the first disagreement. Write down the tick, both cells, and
   the fraction. Do not edit the stepper to remove it.

**A clean result** is either "walk and climb match for N ticks" or "they
diverge at tick T, here is the pair." Both are finished work. A result that
says the fall is 3 because classic Lemmings falls at 3 is a failed round.

**The running game.** This round owns the process. Say so. Static reads
belong to other people; they do not restart the game.

**Harness python.** Do not put `C:\progs\msys2\ucrt64\bin` on `PATH` for the
scripts. `capstone` is in the default Python. The test binaries need that
directory, and this round should not need to build.

---

## Round 2 — the fall

**Step done.** Walk-off from y = 48.190 measured **−4.000** world units per
step (three edges). Agrees with `fallSpeed = 4`; Rules unchanged. Vtable
not confirmed live (heap scan failed / was killed). Umbrella/splat not
exercised. Scratch: `docs/scratch/phase3-round2.md`.

**Question.** A walk-off the y = 48 ring: is the step 4.000 world units per
200 ms, and which of the six `fall` animation states does it enter?

The read to test is `PLAN.md` §2.3: vtable `0x004B0A20`, 20.0 at
`0x004B0A18`, umbrella at 10.0 (`0x004B03E8`), splat at 30.0
(`0x004B10A8`). The 25.0 sibling at `0x004B0C60` is a different descent.

The only descent sampled so far, 48.190 → 32.190 with no intermediate
heights inside 0.39 s, is not this fall. A 4-unit step would have taken
about 0.8 s and shown intermediates. Do not set `fallSpeed` from that snap,
and do not treat the snap as a refutation of 20.0.

**Done when** the series names the vtable entered and the world-unit step,
and `Rules` changes only if that series disagrees with the read. Update the
`test_world` fall checks in the same change, and say they moved because of
the series, not because the old expected value was inconvenient.

`read_steppers.py` does the static half with no process. It checks the exe
SHA-256 `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`
before it reports anything.

---

## Round 3 — spawn

**Done, 2026-10-14. The answer is yes.** 102 of 102 levels carry a hatch
record, 144 records; the live words and a fresh birth's `+0x08` / `+0x10`
are those numbers on the three levels read; zero records have a y word that
is not entrance-Y-minus-2, so there is no exception list. Independent check:
the count words sum to `LEVELMAP.DAT`'s total on 102 of 102. `Rules`
unchanged. Facilitator re-ran the corpus survey, the three file anchors, the
spawn-row geometry against the stored grids, and the suite (16 of 16).
Scratch: `docs/scratch/phase3-round3.md`. Still open from it: which column
direction `+0x40 = 1` is — three samples did not isolate it, and
`Facing::Right` was left rather than guessed.

**Question (original).** A released lemming's first position is the hatch
int16 pair, not the entrance object's translation.

Already observed, on three levels, in `PLAN.md` §1.6 close-out and
`tools/harness/measure_spawn.py`: file triples `(count, x, y)` and the live
record. On `MM_CLIMB_002`, `MM_FLOAT`, and `MM_DIG_000` the x word is the
entrance column and the y word is the entrance world Y minus 2. Nothing in
the constructor subtracts 2. It is not shown for every level.

**Done when** `World::spawn` writes those words, `measure_spawn.py --watch`
agrees on those three levels, and any level whose y word is not
entrance-Y-minus-2 is reported rather than folded into a formula. Leave
`test_corpus`'s stored-grid survey where it is: it pins 99 of 102 since the
Y flip (`kGridSurvivableLevels`). An 81 of 102 in an older note or checkout
is the pre-flip figure.

---

## The float/splat round — done

2026-10-14/15. `floatSpeed` 2, `floatAfter` 10, `splatDistance` 30 all hold;
no `Rules` value moved. Faller vtable `0x004B0A20` confirmed live, closing
Round 2's negative. Scratch: `docs/scratch/phase3-round3b-float-splat.md`.
Harness: `tools/harness/measure_float_splat.py` (`--grid-search` is offline
and finds the long drops: **6 of 144 hatch records fall further than 30**,
four of them on `E10_KF_1`). That harness tracks the **actor list**
`0x004CBB18` rather than billboards, so it has no tracker gaps — prefer it
over the billboard path for anything per-tick.

Three things it leaves for later rounds:

- **The float halving is delayed and we do not model it.** `[+0x49]` is set
  on assignment and exempts the splat; `[+0x4b]` is set when the `umbopen`
  animation *finishes*, ~9.6 world units later, and only it halves the speed
  (it is the byte `0x00416038` branches on). `World::stepFaller` halves
  immediately. Read the animation length before inventing a field.
- **A real panel assignment of the parasol was never shown live.** Every
  floater in that round was a `[+0x49]` memory write — a probe, labelled as
  one. The panel mapping was re-measured (index k at y ≈ 27 + 40k, 28 drawn
  rectangles in three strips) but `lemming_slots` found no lemming-sized
  sprite inside its screen box, so the clicks were no-ops before being sent.
  **Widen `is_lemming` first**; this is the same wall Round 1 hit, and it
  blocks every skill round.
- **The zone bits do something in the landing path.** Closed Round 29:
  Fluid stamps `0x200`/`0x2000`/`0x4000`; matching sheet `[+0x4c]` skips
  splat (`0x0041662B` / `0x00416641`). Lava has no arm.

Do not re-cite `0x004B0C60` as a speed constant: it is a vtable, and eight
bytes read there are 1.96e-307. The point that survives is that its state is
a different descent.

## Round 7 — done. Every `Rules` field is now read or measured.

2026-10-15. **A bomb is three stages, not two.** Click → 5000 ms fuse →
flapper installed and Ohno played (one function, `0x00414370`) → a further
**1020 ms** of `explode` animation (strip 6, 17 frames) → the crater. Total
6.018–6.021 s, measured six times on two levels. `bomberFuseMs = 5000`,
`exploderCycleMs = 1020`. Our `OhNo` cue was a second early by construction
and has moved to the end of the fuse. `tools/harness/measure_bomb.py`;
`docs/scratch/phase3-round7-live.md`.

**`explosionRadius` and `clearCircle` are deleted.** The footprint is a
**stored 7×7 bitmap at `0x004B1528`**, 37 of 49 bytes set, a rounded disc,
anchored `col0 = trunc(X − 3.5)`, `row0 = trunc(Y + 1.5 − 3.5)`, cells written
by `mov dword ptr [edx], 0` at `0x0043F7F5`. A radius-5 circle clears **81**
cells where the original clears **37**. Five of six runs matched the predicted
change set exactly. The lemming sits **one unit lower in the blast state than
in the flapper** (`0x00416B5B`), which is why the disc straddles the feet row.

**A warning about reading `.data`, which cost this round's static note a
claim.** It reported Ohno gated by `0x004CBB40` being non-zero, "`0xFF` in the
image". It is **`0x00`** and Ohno always plays: `.data` has vsize `0x1185B8`
but **rawsize `0x7000`**, so `0x004CBB40` (delta `0xCB40`) has **no byte in
the file** and the loader zero-fills it. Before citing any `.data` value from
the image, check it is inside the section's **rawsize** — beyond that you are
reading whatever follows in the file, not what the process sees. `.rdata`
constants are unaffected (its rawsize covers it), so every double cited in
this project so far stands.

Read, not measured: the `0x2` (indestructible) and `0x200` (zone) skips — no
reachable level puts either inside a bombable lemming's 7×7. The `0x800` skip
is consistent-with-the-read but not separable live inside one 1.5 ms sample.
The flapper's fall is implemented from `0x00416A8D` and labelled READ; all six
bombs were on solid ground.

## Round 6 — done. Read the divisor note before round 7.

2026-10-15. A brick is **two cells** at `(p+facing, row)` and
`(p+2·facing, row)` on the builder's **own** row, `p = trunc(X at the click)`
in both facings. Value written is plain **`0x1`** on 72 of 72 cells, so a
brick is **destructible** and the `or edx, 0x403` paint belongs to something
else. Stamp at percent **81**, advance at **90** (Y +1.000, X +1·facing, same
callback as the decrement), **12** bricks (`0x0040F948` stores `0xc` at
`state+0x3c`), then the **shrug** state `0x004B0C78` for 0.966 s and a walk.
`brickLength` 3 → **2**; `builderInterval` **deleted**; `builderCycleMs` 960
and `shrugCycleMs` 960. `tools/harness/measure_build.py`;
`docs/scratch/phase3-round6-live.md`.

**A duration is `frames × 0.06 / divisor`.** The shrug is the first non-1.0
divisor found (strip 10, 8 frames, 0.5 → 0.96 s, measured 0.966), which
settles that the divisor belongs in the **duration** even though it cancels in
the percentage. **It does not explain dig's `0.75`** — dig measured 1.2005 s
and `20 × 0.06` is 1.2, so dig's divisor is 1.0 and the 0.75 at
`controller+0x1D8` remains unread. Do not cite the shrug as having resolved
it.

The `16.0` at `0x004B08F8` is an **enter-time seek** of one sixteenth of a
cycle (6.25%), so the *first* brick lands at 75% rather than 81% — predicted
0.718 s after the click, measured 0.715 s.

Read but not modelled: a non-zero step-into cell **turns the builder out of
the build state without spending the brick** (first of its two exits measured
and predicted).

**Harness capability worth reusing: the arrow keys are the engine's own
direction filter.** Holding RIGHT sets `[screen+0x16B1]`, LEFT sets
`[screen+0x16B2]`. Pure input, no writes, and it is how a mirrored facing was
reached on a level with twenty lemmings in one pen. Use it whenever a round
needs a specific facing.

**Process note.** This round's agent died on a session rate limit at its last
step — "verify the new tests fail against the old behaviour" — with all edits
already on disk. The facilitator completed that check: reverting
`brickLength` to 3 and the cycle to 8 ticks fails **18** of `test_world`'s
204 checks, 16 of them behavioural (per-brick cell counts, the whole brick
tick schedule, column positions, the advance, shrug timing). If a round dies
mid-flight, **build and run the suite before trusting anything on disk** —
here it was green, which is what made the work safe to keep.

## Round 8 — done. The blocker is terrain.

2026-10-15. **A blocker never touches the lemming list.** `World::blockedBy`
is deleted; `Terrain` now carries a lazily-allocated blocker-field layer and
`stepWalker` tests it at each of the step-up probe's three cells, before that
cell's solidity. `Rules::blockerFieldHeight = 4` (MEASURED).
`tools/harness/measure_blocker.py`; `docs/scratch/phase3-round8-blocker.md`.

- **Stamp**: block state vtable `0x004B07B8`, enter hook `0x0040E20C`.
  One column × four rows — `trunc(X)`, rows `trunc(Y)..trunc(Y)+3` upward
  (Y-up live grid). `0x0040E312` is `or ah, 8`; the loop bound at
  `0x004B10B8` is the dword 3, inclusive from 0. Same column in both
  facings: the `sete`/`add` at `0x0040E29A`/`0x0040E2A4` cancels the facing
  bias. Measured on 12 cells, predicted before the click, within 5 ms.
- **Erase**: exit hook `0x0040E0DF`, `and eax, 0xf7ff` at `0x0040E1D5`, then
  the cell is stored back. It runs on **any** departure, not only death — a
  bombed blocker's cells cleared 2 ms after the state became `explode`.
- **Turn**: three sites in the walk probe (`0x0041C430`, `0x0041C4C9`,
  `0x0041C563`), each before that cell's own `and 1`, each falling to
  `0x0042F66B`. Right-facing turns at world X = C, left-facing at C + 1 —
  the two edges of the blocker's cell. The old `|dx| <= 3` turned two cells
  early on both sides.

**`0x800` is runtime state, not authoring** — 0 of 2,506,752 cells across the
102 shipped grids (`--audit`). It is the only cell bit of which that is true.
**So a live grid dump taken while a blocker is alive will not match the
`.LVL`.** That is a bad dump, not a parser bug; `test_collisiongrid`'s five
fixtures are untouched and must stay that way.

Closed by this round: one of round 4's five stepper differences — the walk
probe's X bias collapses to `cell + facing` both ways for in-cell
comparisons, so our `l.x + dir` was already right. **Four remain.**

Left open, **read but not modelled**: a faller landing in a stamped column
should stop four rows up, since `0x0041A99A` accepts any non-zero cell with
`0x80` clear and a stamped air cell reads `0x00000800`. Two live attempts
failed on `[screen+0x16D8]` naming a neighbour. Also read, not modelled: the
bash refusal `0x0040C110` and the miner's turn `0x00418AF9` (no level in
reach stocks those skills). `[lemming+0x4c]`, the probe's exemption, read
**0** on every live lemming including blockers, so it never fires in play.

Panel indices: **2 is the bomber** (measured via the explode state and the
crater), closing one of round 5's unknowns. Clean negative: **index 9 is not
a nuke reachable in two clicks** — two clicks with a blocker alive, 692 Hz
whole-grid watch, zero change. 8 and 9 remain unidentified.

## Round 5 — done. The timing model changed; read this before 6 and 7.

2026-10-15. Bash, dig and mine all measured live.
`tools/harness/measure_dig.py`; `docs/scratch/phase3-round5-live.md`.
`phase3-round5.md` (the static read) is untouched and still correct.

**There are no tick intervals in this engine.** A skill's cycle is
`frames × 0.06 s` — 0.06 is the double at `0x004B1940`, the frame count is on
the controller at `lemming+0x128`: bash 24, dig 20, mine 25 → 1.440 / 1.200 /
1.500 s. Only dig divides evenly into 200 ms ticks. `basherInterval`,
`minerInterval` and `diggerInterval` are **deleted**; `Rules` carries
`basherCycleMs` / `diggerCycleMs` / `minerCycleMs` and `World::bitesThisTick`
samples the real cycle, so a basher acts every 7 or 8 ticks and averages 7.2.
**Round 6 should expect the builder to work the same way** — look for its
strip's frame count before looking for a `builderInterval`.

**The removal is `mov dword ptr [edx], 0`** at `0x00415B84`, through the cell
pointer returned by `0x0040DE60`. Whole dword, not a mask. `FUN_0041B683`'s
`and 0xf77c` is **not** the bite — that is now settled by the instruction and
by 38 of 38 live cells going to zero, and it remains caller-less.

**Refusal is two tests, and both can stop a bite:** bit `0x2`
(indestructible), and a **directional** bit built on the stack —
`mov [ebp-0x10], 0x10`, or `0x20` when `[+0x44]` (vertical direction) is −1,
tested by `and edx, [ebp-0x10]`. That stack construction is why scans for an
`and <imm>` never found `0x4`/`0x8`/`0x10`/`0x20`. `kCellUnreadBits` is now 0.
The right-facing and inverted halves are **read, not measured** — don't cite
them as measured.

Footprints per cycle, relative to the lemming's cell: **dig** 3 cells
(`trunc(X)−1, 0, +1`) on the row below the feet, then Y −1 (bite at percent
25, step at 30). **bash** the leading column only, 4 rows from the feet up
(percents 12, 16, 60, 64 → rows +0, +1, +3, +2). **mine** columns +1, +2 ahead
× rows feet−1…+3 less one skipped cell, then X +1 / Y −1.

Panel indices now known: 0 climber, 1 parasol, **5 basher**, **6 miner**,
7 digger, 3 blocker, 4 builder. **2, 8 and 9 are still unidentified** — do not
fill them in from the classic order.

Left open by this round, with citations: dig's `0.75` at `controller+0x1D8` is
**not** the cycle divisor (its strip is 20 frames and 20 × 0.06 is the
measured 1.2 s) and what it does is unread; `0x00426DB0` is unread; and one
bash run gave up mid-second-cycle for reasons not established — recorded as an
observation, not guessed at.

## Skill assignment — solved. Read this before rounds 5–8.

2026-10-15. Assigning a skill through the panel works: **38 clicks, 38
assignments**, across digger / blocker / parasol / climber / builder on five
levels. Call `assign(h, inp, skill, choose=...)` in
`tools/harness/assign_skill.py`; every address is in its docstring. Scratch:
`docs/scratch/phase3-assign-skill.md`. Three rounds failed at this before and
each fell back to writing `[lemming+0x48]` / `[+0x49]` by hand. **Do not do
that any more** — a flag poke is a probe, not an assignment.

`0x004CBFB0` is the **level screen object** (`0x0045DA9E` reads it as
`[0x004C6A08 + 0x55A8]`), and it already holds what the harness needs:

- `screen+0x16A0` / `+0x16A8` — the cursor, doubles, render pixels. The drawn
  crosshair is written *from* them by truncation. They are the master.
- `screen+0x16D8` — the lemming under the cursor, 0 for none. The hover pass
  applies `0x0045A0E0`, the same applicability test the click uses, so
  non-zero means **a click will land**. Use it as aim feedback.
- `lemming+0x128` — that lemming's own screen rectangle (`+0x20` left,
  `+0x28` top, `+0xF0` right, `+0x90` bottom). The pick is an AABB against
  it. This is the stored screen pair earlier rounds looked for and reported
  absent.

The click path is `0x00455657`: bail on `+0x16D8 == 0`, `0x0045AFEB` builds
the state, `0x0042E696` writes `[lemming+0x340]`, `0x0045B594` spends stock.
Stock array `0x004C91D8`; its `+0x28` is the selected index `0x004C9200`.

**Stop writing `0x004C36A0`.** It has exactly two references in all of
`.text`, both its own accessors, and the readers are frontend and level-map.
Nothing in the level screen reads it. That is why Round 1 lost a session to
it, and the rules file has been corrected.

Four things to carry into the next round:

- **Proof of assignment is the stock count going down.** The engine
  pre-selects a level's only stocked skill, so `0x004C9200` agreeing with
  your intent proves nothing at all.
- **The dig state lives 271 ms.** Sample `[lemming+0x340]` at kHz; do not
  sleep and then read, or you will record a walker and conclude the click
  missed.
- **Panel indices confirmed**: 0 climber, 1 parasol, 3 blocker, 4 builder,
  7 digger. 2, 5, 6, 8, 9 are unidentified — `0x0045A0E0`'s ten-arm jump
  table at `0x0045A4DD` is unfollowed. Do not fill them in from the classic
  order.
- **Budget ~3 s per assignment**, plus one `find_menu_index` retry per level.
  `is_lemming` in `measure_dropoff.py` is still a **fit** to one camera (it
  demands `cy >= 220`, where real boxes sit at y 77–126 on MM_DIG_000) and is
  left that way so `assign_climber` remains the regression case. Prefer the
  new module.

Two leads it found and correctly did not measure: a builder's world Y rises
**+1.000** per assignment, and a digger **gives up after one tick**. Those
are rounds 5 and 6.

## Round 4 — done, and what it left in the steppers

2026-10-15. `maxStepUp` 2 (was classic 3), `lemmingHeight` 3 (was classic 4),
each with a static read and a two-sided live series. Scratch:
`docs/scratch/phase3-round4-live.md`. Harness:
`tools/harness/measure_stepup.py`. `test_world` gained `testStepUpHeight`,
whose expectations are **written out, not derived from `Rules`** — verified
by the facilitator: flipping `maxStepUp` back to 3 fails the behavioural rows
(`highest walking y 76 (expected 79)`, `turned at it`), not merely the field
assert. Suite 16 of 16.

The live half is a **labelled probe**, not natural play: the corpus has no
reachable rise of exactly 3, so pillars of 1/2/3/4 cells were written into
the live grid in front of the original's own walkers and restored. If you
redo it, that is the technique.

**Four differences this round cited and deliberately did not change.**
**All four are now closed by round 10** — kept here for the citations, which
held up. They were stepper work, not constants, and taking them together was
the right call: the hoist and the float delay both turned out to be animation
lengths, as round 5's skills had been.


- The walk probe carries an **X bias from the facing**: `0x0042F66B` writes
  −1.0 to `[+0x350]`/`[+0x360]` facing left and 0.0 facing right, so the
  probe reads `trunc(X)` going right and `trunc(X) - 1` going left. Ours has
  no bias.
- The original's walk **steps down one cell before falling**
  (`0x0041C6FF`, `0x0041C741`, `0x0041C77D`). Ours has no step-down arm —
  the likeliest cause of the remaining open-floor edge disagreements.
- The climber's **top-out is one cell lower** than its ceiling check
  (`0x00413B22`), where `World::stepClimber` uses `lemmingHeight` for both.
- The **hoist holds Y and X for ~4 ticks**, against our 2, then places the
  feet at +2.

**Round 8 is already located**: the blocker's `0x800` turn is inside this same
walk probe (`0x0041C430`, `0x0041C4C9`/`0x0041C4D6`,
`0x0041C563`/`0x0041C570`), not a separate stepper. Do not go looking for a
blocker state.

**Harness trap worth knowing**: `lrstate.attach` returns `(handle, pid)`, not
`(pid, handle)`. Getting it backwards costs a run and presents as a
level-table failure.

## Handing over the facilitator role

If you are taking over the **facilitator** role — dispatching rounds, verifying claims
before they are promoted, and owning this file, `PLAN.md` §1.6 and `.claude/rules/` — read
**`docs/FACILITATOR_HANDOFF.md`**. It carries the verification loop, the failure catalogue
(agent-side and facilitator-side), the dispatch-brief structure, and what to do when an
agent dies mid-round.

## Rounds 4–8 — how to read a constant

The method is the one that fixed the fall. Do it in this order.

1. Find the stepper. `read_steppers.py --states` prints the 24 animation
   sites and the 46 vtables. `bash`, `dig`, and `mine` share `0x0041B077`.
   Their enter hooks differ by the animation and the rate written through
   `0x004683E9`. Dig's hook passes 0.75.
2. Cite the address and the bytes of the constant. A decompiler line is not
   a measurement.
3. Ask what calls it. The removal at `FUN_0041B683` is slot 3 of the
   trampoline vtable `0x004B0D08` and has no direct caller in `.text`. It is
   not the skills' bite. Do not store it in `diggerDepth` or `basherReach`.
4. One live series on a level that actually uses the skill. `goto_level.py`
   can enter it. Assigning a skill needs the panel, which Round 1 does not
   set up: the drawn crosshair is the doubles at `0x004CBFB0` +0x170 and
   +0x178, and the hit-test reads `0x004C36A0` / `0x004C36A4`. Write the
   lemming's screen position there immediately before the shim click.
5. Then change `Rules`, with the field comment saying measured or read, and
   the address.

`maxStepUp` was not a double in the walk stepper. The walk loads 2.0, 0.0
and 5.0; the step-up was in the mask probe. **Round 4 followed that probe and
settled it — 2, from the two unrolled arms of `0x0041BFFC`.** Kept here as
the worked example of this method: the constant was not in the stepper that
appeared to own it, and the answer came from the shape of the code (two
`add`/`and` pairs and no third) rather than from a value read anywhere.

---

## Round 9 — one gadget

**Anti-grav is closed.** Scratch: `docs/scratch/phase3-round9-antigrav.md`.
Harness: `tools/harness/measure_antigrav.py`.

Cell bit **`0x8000`** is the anti-grav pad: 18/18 retail levels with
`anti_grav.a3d` carry it and no other level does. The walk probe tests it on
the ground under the feet (`0x0041C154`), and when an anti-grav object also
overlaps (`0x004C8958` list) it attaches cartwheel `0x004B0CA8` (type `0x2b`).
Cartwheel enter calls `0x0042F5E3`, which negates `[lemming+0x44]`. Live on
`m06_kf_1`: 27 flips, every one at pad column 68 with `+1 → -1` and world Y
increasing after. Painting the bit onto MM_CLIMB_002 without an anti-grav
object flipped nothing — a clean negative. Side finding: hatch type word 1
(`entrance_acid`) becomes live `[+0x4c] = 1`.

Sim: `Terrain::antiGrav` loaded from `0x8000`, `Lemming::verticalDir`, walker
flips and falls along it. Test `testAntiGravFlipsGravity` fails when the flip
is disabled alone. Suite 16/16.

Round-9 cue/gadget repeats closed through Die (28). Residue rounds
29–31 closed (splat-exemption, inverted steppers, cell-flag writers).
Trampoline / water / acid / teleport / SpeedPad / Lever are done
(13–17, 21); Balloon/Fire/Witch/ForceF/Axe/Bubble/Scrape/Rumble/
Changeop/Die are non-gadget cues (18–20, 22–28).

---

## Round 11 — the exit box

**Closed.** Scratch: `docs/scratch/phase3-round11-exit.md`. Harness:
`tools/harness/measure_exit.py`.

The save test is **not** a hand-sized rectangle. Exit objects (vtable
`0x004B0F90`, step `0x0042688D`) walk list `0x004C88D0` and call the general
overlap predicate `0x00430BA1` on the lemming's raw `(X,Y,Z)` at `+8` against
`object+0x48..+0x70`. Inclusive on every edge (`fcomp` / CF for mins,
`fcomp` / CF|ZF for maxes). Save state `0x004B0B28` / type `0x2a` increments
`0x004CBAA4`. Z is always `[0, 1]` (`0x00430FE1`). Live: MM_CLIMB_002
`X[220..222] Y[31..34]`, MM_FLOAT `X[176..178] Y[6..9]`; hold-probe hits the
exact max doubles. Column centre is **`round(ang×width)`**, not `floor`
(corrected in round 12; MM_FLOAT's own dump already centred on round).
`_curExit` is UI-only — not a second box.

Sim: `Exit` / `ExitTrigger` defaults **2×3**, `World::atExit` closed (`<=`),
`exitTriggerFromPose` (hatch-style Y flip), five `main.cpp` sites. Also closed
the round-9 gap: `testAntiGravFromCollisionGrid` pins `0x8000` extraction in
`fromCollisionGrid`. `testExit` fails if `atExit` is half-open alone. Suite
16/16. Live write `0x004CBAB0` (save target) was restored to 1.

---

## Round 12 — finish the exit

**Closed.** Scratch: `docs/scratch/phase3-round12-exit-models.md`. Harness:
`tools/harness/measure_exit.py` (now refuses to dump if the loaded path does
not contain the requested name).

**Same 2×3 box for all four retail exit models** — not per-model. Live dumps
(loaded path verified): `e14_kf_1` / `MM_W_012` (`Exit_water.a3d`),
`E16_KF_1` (`Exit_any.a3d`), `E23_JC_1` (`Exit_acid.a3d`); each one object
vtable `0x004B0F90`, size 2×3, Y `[ty−9 .. ty−6]`, Z `[0,1]`. Census 102/102:
`Exit.a3d` 84, water 14 (8 alone), any 9, acid 1.

**Column centre is `round`, not `floor`.** Offline: 46 of 111 placements have
`floor(ang×256) ≠ round(ang×256)`. Live centres on e14 (10.989→11) and E16
(210.995→211) match round; round 11's MM_FLOAT dump `X[176..178]` already
matched round (176.999→177) while the formula said floor.

`exitTriggerFromPose` uses `std::lround`. `testExitTriggerFromPose` in
`test_world` pins MM_CLIMB_002 and e14 rectangles, centre-saves, and the four
outside edges. Left open: natural walk-in; acid save-state attach (same class
and AABB measured; probe did not finish); `.a3d`→AABB writer chain.

---

## Round 13 — trampoline

**Closed.** Scratch: `docs/scratch/phase3-round13-trampoline.md`. Harness:
`tools/harness/measure_trampoline.py`. Brief: `docs/ROUND13_BRIEF.md`.

Cell bit **`0x1000`** on the landing cell attaches bounce (vtable
`0x004B0810`) at fall land `0x00416560`, before splat. Runtime-only (0/102
`.LVL`); trampoline state `0x004B0D08` paints it. `pressure_pad.a3d` clean
negative on H01_KF_1. `bounceRiseCells` / `bounceSpeed` are unpinned
stand-ins — see facilitator check in `PLAN.md` §1.6.

---

## Round 14 — water hazard

**Closed.** Scratch: `docs/scratch/phase3-round14-water.md`. Harness:
`tools/harness/measure_water.py`. Brief: `docs/ROUND14_BRIEF.md`.

Cell bit **`0x40`** on the ground under the feet → drown (`0x004B0570`) for
~2.0 s then death. Spawn side-effect `0x2f` probes at `0x00415386`. Not
`Exit_water`; not `Entrance_water` / zone `0x200`. Authored in 3/102 levels.
`drownMs=2000` pinned by mutate-alone in `test_world`.

---

## Round 15 — acid hazard

**Closed.** Scratch: `docs/scratch/phase3-round15-acid.md`. Harness:
`tools/harness/measure_acid.py`. Brief: `docs/ROUND15_BRIEF.md`.

**Same `0x40` drown as water** — no separate acid death. Type `0x2e`
unused live; `0x100` → `lem[+0x5f]` only. `Entrance_acid` / `LemmingsAcid`
/ `^acid_glow` are sheet / visual, not the hazard.

---

## Round 16 — teleport

**Closed.** Scratch: `docs/scratch/phase3-round16-teleport.md`. Harness:
`tools/harness/measure_teleport.py`. Brief: `docs/ROUND16_BRIEF.md`.

Object-only: `teleport.a3d` pads (vtable `0x004B1250`) overlap via
`0x00430BA1` at `0x00435467` / `0x004354CB`, attach tele-out (`0x28`) then
tele-snap (`0x2c`) with countdown **4**, relocate to partner (`+0x260` /
`0x0043503C`, Y+2.0). Live AABB 12×8; facing preserved. `teleport_fx.a3d`
FX-only; no exclusive cell bit (18/18). Pairing stand-in `i↔i+n/2` until
stream id is in the .LVL parse. `teleportDelayTicks=4` pinned by
mutate-alone.

---

## Round 17 — SpeedPad

**Closed.** Scratch: `docs/scratch/phase3-round17-speedpad.md`. Harness:
`tools/harness/measure_speedpad.py`. Brief: `docs/ROUND17_BRIEF.md`.

Object-only: `speedup_two_way.a3d` (vtable `0x004B1178`) overlap via
`FUN_00432742` / `0x00430BA1` → FASTER (`0x004B0A38`, type `0x10`) for
**5.0 s** with `+0x50=2` → main state twice per frame (~2× walk). Cue
`SpeedPad` at `0x004C0D10` plays on activate. Live AABB 2×6; no exclusive
cell bit (9/9). `pressure_pad.a3d` is not this object. `fasterMs=5000`
pinned by mutate-alone. SLOWER sibling (`+0x50=3`) unread this round.

---

## Round 18 — Balloon

**Closed (clean negative).** Scratch:
`docs/scratch/phase3-round18-balloon.md`. Harness:
`tools/harness/measure_balloon.py`. Brief: `docs/ROUND18_BRIEF.md`.

Cue `Balloon` at `0x004C0D24` registers to `+0x1c8f8` / `DAT_004E3300`,
is preloaded and unloaded with the SFX table, and is **never played**
(0 absolute refs; 0 of 23 play-factory callers). No `balloon.a3d`. Exit
`Models\exit\balloon_*` are decorations. `kBalloonGadgetInRetail=false`.

---

## Round 19 — Fire

**Closed (clean negative).** Scratch: `docs/scratch/phase3-round19-fire.md`.
Harness: `tools/harness/measure_fire.py`. Brief: `docs/ROUND19_BRIEF.md`.

Cue `Fire` at `0x004C0D2C` → `+0x1c8fc` / `DAT_004E3304`; register /
preload / unload only; **0** play refs. No fire a3d. `kFireGadgetInRetail=false`.

---

## Round 20 — Witch

**Closed (no lemming gadget).** Scratch:
`docs/scratch/phase3-round20-witch.md`. Harness:
`tools/harness/measure_witch.py`. Brief: `docs/ROUND20_BRIEF.md`.

Cue `Witch` at `0x004C0D34` → `+0x1c8a8` / `DAT_004E32B0`. Sole play is
pause easter egg on `compiled\e10_kf_1.lvl` (`0x0045D200` →
`FUN_0044521b`). No `witch.a3d`. `kWitchGadgetInRetail=false`.

---

## Round 21 — Lever

**Closed.** Scratch: `docs/scratch/phase3-round21-lever.md`. Harness:
`tools/harness/measure_lever.py`. Brief: `docs/ROUND21_BRIEF.md`.

Object `lever.a3d` (vtable `0x004B1288`): facing-gated AABB toggle of
`+0xb8`, cue play, message fanout type 4/5 to linked objects. No lemming
attach. Live E16 AABB 2×6. Sim models the toggle; fanout unread in sim.

---

## Round 22 — ForceF

**Closed (clean negative).** Scratch:
`docs/scratch/phase3-round22-forcef.md`. Harness:
`tools/harness/measure_forcef.py`. Brief: `docs/ROUND22_BRIEF.md`.

Cue `ForceF` at `0x004C0CC8` → `+0x1c8d0` / `DAT_004E32D8`; never
played. `kForceFGadgetInRetail=false`.

---

## Round 23 — Axe

**Closed (no lemming gadget).** Scratch: `docs/scratch/phase3-round23-axe.md`.
Harness: `tools/harness/measure_axe.py`. Brief: `docs/ROUND23_BRIEF.md`.

`Axe` is a HUD sprite sheet (`SPRITES\AXE.SPR`) via sprite registrar
`FUN_00446040`; sole abs use is UI bind `0x00496E40`. `kAxeGadgetInRetail=false`.

---

## Round 24 — Bubble

**Closed (no Bubble gadget).** Scratch:
`docs/scratch/phase3-round24-bubble.md`. Harness:
`tools/harness/measure_bubble.py`. Brief: `docs/ROUND24_BRIEF.md`.

Cue `Bubble` is SFX on fluid-mismatch state type `0x23` (vtable
`0x004B03C0`); play at `0x00404697` / `0x004046C4`. No `bubble.a3d`.
`kBubbleGadgetInRetail=false`.

---

## Round 25 — Scrape

**Closed (no Scrape gadget).** Scratch:
`docs/scratch/phase3-round25-scrape.md`. Harness:
`tools/harness/measure_scrape.py`. Brief: `docs/ROUND25_BRIEF.md`.

Cue `Scrape` is dig/bash steel-refuse SFX (`0x0040C139` / `0x00415B33`).
Mine refuse uses `MineMetal`. `kScrapeGadgetInRetail=false`.

---

## Round 26 — Rumble

**Closed (no Rumble gadget).** Scratch:
`docs/scratch/phase3-round26-rumble.md`. Harness:
`tools/harness/measure_rumble.py`. Brief: `docs/ROUND26_BRIEF.md`.

Cue `Rumble` is object state-commit SFX from step `FUN_00423749`
(vtable `0x004B0F00`) when prior committed state `+0xa4 ≠ 0`.
`kRumbleGadgetInRetail=false`.

---

## Round 27 — Changeop

**Closed (no Changeop gadget).** Scratch:
`docs/scratch/phase3-round27-changeop.md`. Harness:
`tools/harness/measure_changeop.py`. Brief: `docs/ROUND27_BRIEF.md`.

Cue `Changeop` is UI SFX: skill-panel select (`FUN_0045ac5c`) and
options SFX-volume slider (`FUN_0048e8d5`). `kChangeopGadgetInRetail=false`.

---

## Round 28 — Die

**Closed (no Die gadget).** Scratch:
`docs/scratch/phase3-round28-die.md`. Harness:
`tools/harness/measure_die.py`. Brief: `docs/ROUND28_BRIEF.md`.

Cue `Die` is SFX on type-`0x23` enter when fluid `+0x128==1` (Round 24
path). Distinct from FallSplat (type `0x1a`) and unused Splat.
`kDieGadgetInRetail=false`.

---

## Round 29 — splat-exemption zones

**Closed.** Scratch: `docs/scratch/phase3-round29-splat-exempt.md`.
Harness: `tools/harness/measure_splat_exempt.py`. Brief:
`docs/ROUND29_BRIEF.md`.

Fluid stamps `0x200`/`0x2000`/`0x4000` from `object[+0x128]`; hatch type
→ `[+0x4c]` (sheet table `0x004BFFC8`). Fall land skips splat for
matching `0x200`+type1 / `0x4000`+type2. Lava `0x2000` has no arm. Not
drown `0x40`.

---

## Round 30 — inverted steppers

**Closed.** Scratch: `docs/scratch/phase3-round30-inverted.md`. Harness:
`tools/harness/measure_inverted.py`. Brief: `docs/ROUND30_BRIEF.md`.

After anti-grav, step-up / climb / dig match upright with gravity
flipped. Dig refuse mask `0x20` when `[+0x44]==-1`. Bash `0x4`
right-facing still unread.

---

## Round 31 — cell-flag writers

**Closed.** Scratch: `docs/scratch/phase3-round31-cell-writers.md`.
Harness: `tools/harness/measure_cell_writers.py`. Brief:
`docs/ROUND31_BRIEF.md`.

`0x100` authored-only (3 levels; no runtime cell setter). `0x400` is
door/gate solid stamp (vtable `0x004B0F00`, `|0x403` / `&0xfbfc`).
Round 15 reader story unchanged. **Phase 3 residue list empty.**

---

## Checks that are not targets

- `test_collisiongrid` compares the parser to live dumps. A failure is a
  parser bug. Do not update the expected grid.
- `test_corpus` pins current behaviour, including 99 of 102 on the stored
  grid (post-Y-flip; 81 was the pre-flip pin) and the lattice side beside
  it. Those numbers are not quality scores.
- `test_world` encodes today's steppers. It moves when a round replaces a
  provisional constant with a read or a series. It does not move to make a
  round look finished.

---

## Environment

The traps are written out in `docs/PHASE2_HANDOFF.md` and they still apply.
The ones that will hit this phase:

- Test binaries need UCRT64 `bin` on `PATH`. Harness scripts must not have
  that export, or they lose `capstone`.
- `scripts/Invoke-Ucrt64.ps1` was silently useless until 2026-10-14: it went
  through `msys2_shell.cmd -no-start`, which detaches into its own console,
  so it printed **nothing** and exited **0 whatever happened** — including for
  a build that never ran. It now calls `usr/bin/bash.exe` directly and
  propagates stdout and the exit code. If you are on an older checkout, or it
  ever goes quiet again, do not read that silence as a green build.
- `find_menu_index` fails about one run in six and refuses to press RETURN.
  Retry. Do not press RETURN blind.
- ESC from a post-level screen can quit the process. A 25-frame ESC can
  fail to leave pause.
- There is no safe route from inside a level back to the map. Restart
  between levels.
- clangd errors on new files are stale until the file is in a build. The
  build is the authority. This phase's first rounds should not need one.

Before promoting a number into `Rules`:

- [ ] Address and bytes, or a script that re-runs the series?
- [ ] Observation or hypothesis, and does the note say which?
- [ ] If it is a negative, what was searched for?
- [ ] What calls this?
- [ ] Does it hold on a level other than the one it was found on?
- [ ] Is the number in world units, not the old two-rows-per-unit mask?
- [ ] Does it merely match classic Lemmings?
