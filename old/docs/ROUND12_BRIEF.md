# Round 12 brief — finish the exit

This file is a **self-contained prompt**. It assumes no prior conversation, so it can be
pasted into a fresh session — a different agent, a different service, a different machine —
and stand on its own. Copy everything below the rule.

Its evidence standard, environment traps and must-not-touch list match
`docs/ROUND9_BRIEF.md` and `docs/ROUND11_BRIEF.md`; they are repeated here so this file can
be pasted alone.

**Why this round and not Phase 2 close-out.** `PLAN.md` §3 listed three Phase 2 items for
months. On 2026-10-16 all three were checked and are done or down to residue — the chunk
walk is in `CollisionGrid.cpp` and lands on the grid in all 102 levels, every cell bit has a
reader instruction behind it, and `rasteriseSurface` is already out of the collision path
(its only callers are the `LevelStats` survey and `lrtool`). The list was stale and is now
corrected in place. There is no round's worth of work there. What there *is* a round's worth
of work in is the hole round 11 left — below.

Written 2026-10-16, tree green at 16/16, rounds 1–11 landed.

---

You are picking up Phase 3 of Lemmings Webolution at `C:\progs\lemmings-webolution`, a 1:1
reimplementation of Lemmings Revolution. **The stock 640x480 executable is the authority** —
not classic Lemmings, not plausibility, not a decompiler's guess.

## Read first, in this order

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | the evidence standard; non-negotiable |
| `.claude/rules/windows-msys2-build.md` | build + PATH traps |
| `docs/scratch/phase3-round11-exit.md` | **the round you are finishing** — read this closely |
| `docs/PHASE3_HANDOFF.md` | rounds table, `Rules` status, "how to read a constant" |
| `PLAN.md` section 1.6 "Where to go next" | every landed result, newest last |
| `docs/scratch/phase3-round9-antigrav.md` | the object-overlap machinery round 11 reused |

## What round 11 established, and what it did not

Round 11 found that a lemming is saved by the **general object-overlap AABB** at
`0x00430BA1`, not by the invented `8 x 8` / `12 x 14` rectangles that were in the code. The
predicate is **inclusive on every edge** — verified instruction by instruction: the mins at
`object+0x48/+0x50/+0x58` use `fcomp` / `test ah, 1` / `jne` (fail when the point is below),
and the maxes at `+0x60/+0x68/+0x70` use `test ah, 0x41` / `je` (pass on below **or**
equal). The exit's step `0x0042688D` (vtable `0x004B0F90`) walks list `0x004C88D0` and tests
the lemming's **raw** `(X, Y, Z)` at `+8`. Z is a fixed `[0, 1]` slab from setter
`0x00430FE1`. The box measures **2 x 3** in world units, inclusive, which is 3 columns by 4
rows — twelve cells, where the old guess covered 168.

That part is solid. Two things are not, and they are your round.

### Task 1 — the offsets are measured for one exit model out of four

Round 11 wrote the result as `[col±1] x [ty−9 .. ty−6]` relative to `Exit*.a3d`. The glob is
too generous. An offline census of the 102 retail levels finds **four** exit models:

| model | levels |
| --- | --- |
| `Exit.a3d` | 84 (78 of them exclusively) |
| `Exit_water.a3d` | 14 |
| `Exit_any.a3d` | 9 |
| `Exit_acid.a3d` | 1 |

Both levels round 11 dumped — `MM_CLIMB_002` and `MM_FLOAT` — use `Exit.a3d`. So the
offsets are **measured for 84 levels and assumed for 18**. If the AABB is derived from the
model's own geometry, a different model is a different box, and `exitTriggerFromPose`
currently applies the same `−9 / −6 / ±1` to all of them.

Note: `MM_DIG_000`, the third level round 11 failed to enter, is also `Exit.a3d` — it would
have added no coverage. **Dump a `Exit_water.a3d` level and an `Exit_any.a3d` level**, and
`Exit_acid.a3d` if you can reach the one level that has it. Re-run the census yourself to
pick targets; a scan of each `.LVL` for `[A-Za-z0-9_]+\.a3d` names containing `xit` is all
it takes, against `build/lvl_dump/levels/COMPILED` (102 files).

Three outcomes, all publishable:
- The box is the same 2 x 3 at the same offsets for every model — then the generalisation
  was right and is now evidenced, and `exitTriggerFromPose` needs no change.
- The box differs per model — then the extent belongs to the model, not to a constant, and
  `LevelObjects` has to carry it per placement. **This is the likelier answer** given how
  often this project has found a per-object table where it expected a constant.
- A water or acid exit is not an exit at all in the save sense (it may be a hazard that
  consumes lemmings without saving them) — that is a finding, and it would change what
  `atExit` means.

### Task 2 — the headline number is in the code but not in the suite

`testExit` pins the *inclusivity*: make `World::atExit` half-open and it fails two checks
behaviourally. But **nothing exercises `exitTriggerFromPose`**. Widening it back to the old
guess —

```cpp
e.x = col - 6;  e.width = 12;   // instead of col - 1 and 2
```

— leaves **all 16 tests green**. So the actual measured result of round 11 is unprotected.

This is the second time the same gap has appeared: round 9 left the `0x8000` extraction
untested, and round 11 correctly closed it with `testAntiGravFromCollisionGrid` in
`test_terrain` (verified: disabling the extraction line fails 2 checks there). Do for your
own numbers what round 11 did for round 9's. A test that builds a `WorldPose` at a known
`(col, ty)`, calls `exitTriggerFromPose`, and asserts the exact cell rectangle — plus a
lemming one cell outside each of the four edges that is **not** saved — closes it.

Then verify the test discriminates by reverting the extent and watching it fail. A test
whose expectation is derived from the field it tests passes at any value; that has happened
in this project twice.

### Task 3 — if and only if tasks 1 and 2 are done and there is budget left

Round 11 left the natural walk-into-exit series unmeasured — the edges were pinned by
teleport-and-hold because the pen geometry on `MM_CLIMB_002` kept walkers away for ~50 s
watches. A lemming walking into an exit under its own power, with the saved counter at
`0x004CBAA4` rising, would turn the hold-probe result into an ordinary observation. Pick a
level whose exit is on the walking floor.

## Evidence standard — the project has been burned by each of these

1. **A claim needs an address and the bytes read there, or a re-runnable script.** A
   decompiler line is NOT a measurement. Disassemble with:
   ```
   python tools/harness/read_steppers.py --at 0x00430BA1 --len 0x90
   python tools/harness/read_steppers.py --states
   python tools/harness/read_steppers.py --cell-flags
   ```
2. **Write the prediction down BEFORE the live run**, including what would falsify it.
   Round 11's P1–P5 are the model to copy; P5 was the one that moved, and because it was
   written down first, that was a result rather than a muddle.
3. **Read lemmings from the actor list at `0x004CBB18`** — never from billboard quads.
   Fitting a quad to a lemming is a fit, and it fitted a *non-lemming* for nine rounds while
   producing a plausible series; two "measurements" had to be retracted.
   `tools/harness/compare_tick.py` is **superseded** — do not use it or quote its numbers.
   Use `tools/harness/measure_steppers.py --compare walker|climber`.
4. **Before citing any `.data` address from the image, check it falls inside that section's
   rawsize**, not just its virtual size. `.data` has va `0x004BF000`, vsize `0x1185B8`, but
   **rawsize only `0x7000`** — file bytes exist only up to `0x004C6000`. A round once
   asserted a gate byte of `0xFF` that was a read past the raw data and is zero-filled by
   the loader. `.rdata` is fine throughout.
5. **One cell is one world unit.** Never multiply a height or distance by
   `PIXELS_PER_UNIT` — the factor-of-2 lattice raster is retired, and doubling a world-unit
   delta is the single most repeated mistake in this project's history. A stale "climbSpeed
   is 2, which is two mask rows" line survived in `PLAN.md` until 2026-10-16.
6. **Do not invent rules from classic Lemmings, and do not halve classic values.**
   `maxStepUp` and `lemmingHeight` came out 2 and 3, not the halved 3 and 4.
7. **Expect things to be the wrong *shape*, not the wrong number.** Three "intervals in
   ticks" were animation cycles (`frames x 0.06 s`, the double at `0x004B1940`); a blocker
   "list scan" was a terrain field; an "explosion radius" was a stored 7x7 stencil; a "wait
   of 2 ticks" was a 14-frame animation; the exit "rectangle" was an object AABB. **A
   per-model exit extent is exactly this pattern again.**
8. **A clean negative is a publishable result.** So is "all four models share one box".
9. **Preserve quirks.** If the original does something that looks wrong, ours does it too.

## What is already settled — do not redo any of it

Rounds 1–11 are closed and every field in `Rules` (`src/sim/World.h`) is a read or a live
measurement: `ticksPerSecond` 5, `walkSpeed` 1, `climbSpeed` 1, `fallSpeed` 4, `floatSpeed`
2, `floatAfter` 10, `splatDistance` 30, `maxStepUp` 2, `lemmingHeight` 3 (ceiling bias
only), `climberTopOut` 2, `hoistCycleMs` 840, `hoistLift` 2, `floatOpenMs` 480,
`brickLength` 2, `builderCycleMs` 960, `shrugCycleMs` 960, `basherReach` 1, `basherHeight`
4, `diggerWidth` 3, `minerReach` 2, the bash/dig/mine millisecond cycles,
`blockerFieldHeight` 4, `bomberFuseMs` 5000, `exploderCycleMs`, the 7x7 blast stencil at
`0x004B1528`, anti-grav as cell bit `0x8000` (18 of 18 levels, audited twice), and the
exit AABB's inclusivity.

Phase 2 is closed (see the note at the top of this file). The 102-level corpus survey (99
releasable) is settled; do not retune it. Phase 2 terrain research is finished.

## Build and test

Windows, MSYS2 UCRT64 — you are in PowerShell and do not inherit it.

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
```

Test binaries live at `build/cmake/dev-debug/*.exe` and need UCRT64 `bin` on `PATH` or they
fail to load silently — run them through `Invoke-Ucrt64.ps1`. **Note which binary holds your
test**: `test_world`, `test_terrain` and `test_levelobjects` are separate executables, and
running the wrong one will make a good test look like it does not discriminate.

## Environment traps, all learned the hard way

- **Do not put UCRT64 `bin` on `PATH` for `tools/harness/*.py`** — it shadows the default
  Python and `capstone` disappears. Run harness scripts on the normal PowerShell/Python
  `PATH`.
- `tools/dinput-shim` is 32-bit mingw32, not UCRT64; use its own `build.sh`.
- `lrstate.attach` returns `(handle, pid)`, **not** `(pid, handle)`.
- `read_lvl_grid.find_grid` returns `(offset, cells)` — two values.
- Menus are German. **RETURN only works when the highlight is exactly 0.** A short ESC fails
  to leave pause. Real behaviours; do not route around them by poking memory where a
  keypress is what you need.
- **`goto_level` can silently fail**: round 11 tried to enter `MM_DIG_000`, the slot copy was
  restored but the loaded path stayed `Compiled\MM_FLOAT.lvl`, and the dump was of the wrong
  level. **Always read back the loaded path before trusting a dump.**
- If you raise the save target to keep a level alive while probing (round 11 used
  `0x004CBAB0`, raising it to 50/80), **restore it** — that round restored it to 1.
- **Cap live sampling**: an uncapped single-lemming run at ~300k `ReadProcessMemory`/s killed
  two game processes. `--hz 2000` is safe; `--hz 100` is plenty for a walk-in.
- If you write to the live collision grid, **log every cell and restore all of them**.
- `ui_state`: 0 playing, 2 pause, 3 frontend, 5 results. Frontend screen id `0x004C416C` is
  3 menu, 4 confirm, 5 map.
- Re-runnable scripts: `measure_exit.py` (round 11's, `--dump` / `--watch`),
  `measure_steppers.py`, `measure_antigrav.py`, `measure_stepup.py`, `measure_blocker.py`,
  `read_lvl_grid.py`, `read_grid.py`, `assign_skill.py`, `goto_level.py`.
- If you cannot launch the game, **task 2 is entirely offline** and is still a legitimate
  round on its own. Say clearly that task 1 was not measured live.

## Must not touch

`src/__test__/test_collisiongrid.cpp`, `src/__test__/test_corpus.cpp` and its expected
numbers, and any settled `Rules` field above. If a finding seems to require moving a settled
field, **stop and report that instead of moving it**. In scope: `src/sim/World.{h,cpp}`,
`src/assets/LevelObjects.{h,cpp}`, `src/main.cpp`, and the test files other than the two
named.

## Documentation — you own these

`PLAN.md` section 1.6, `docs/PHASE3_HANDOFF.md`, and `.claude/rules/match-the-original.md`.

**Append** your result, and **add your bullet as its own bullet** — the round-9 agent
inserted one into the middle of an existing bullet and orphaned its tail, which then read as
current and revived three claims a later round had already closed. Do not rewrite or
compress what is there: those files carry explicit "Correction." notes and retractions that
were expensive to establish, notably that `compare_tick.py`'s numbers are withdrawn, that
billboard quads are not lemmings, and that the exit offsets are `Exit.a3d`-only. Park
working prose in `docs/scratch/phase3-round12-exit-models.md`.

If task 1 shows the box is per-model, **update the rules file's exit bullet** — it currently
says the offsets are measured for `Exit.a3d` and assumed for the other 18 levels, which your
round either confirms or replaces.

## Deliverables

1. `docs/scratch/phase3-round12-exit-models.md`: the census you re-ran, the addresses and
   bytes, the predictions written before each live run, the dumps per model, the verdict,
   and what you left open **with citations**.
2. `exitTriggerFromPose` correct for every exit model — per-model extents if that is what
   the dumps say — and whatever `LevelObjects` needs to carry them.
3. The test from task 2, verified to fail when the extent is reverted, plus all 16 green.
4. `PLAN.md`, handoff and rules updated by appending.
5. The state of the game process when you finish, and confirmation that every live write was
   restored — grid cells, save target, and anything written to a lemming.

**Report honestly.** Do not round, smooth, or over-claim; headline numbers here get
independently re-verified, and several rounds have had a claim narrowed or retracted on
re-check. Round 11's own summary said its tests pinned the result; they pinned half of it.
Say plainly what you measured, what you only read, and what you could not settle.

## After this round

**Gadget repeats** — `docs/ROUND9_BRIEF.md`, one mechanic per round. The engine's own cue
strings in `.data` are the inventory of what is still unmodelled, and it is longer than the
asset-derived list: `SpeedPad` (`0x004C0D10`), `Balloon`, `Fire`, `Witch`, `Lever`,
`ForceF`, `Axe`, `Bubble`, `Scrape`, `Rumble`, `Changeop`, and a `Die` cue distinct from
both `Splat` and `FallSplat` — which suggests the original has more death paths than our one
splat.

Smaller open items, cheapest first: the three placeholder-named cell bits `kCellFlag0x40`,
`kCellFlag0x100`, `kCellFlag0x400` (all have reader instructions; what is unread is which
code *sets* them) and the unlisted `0x1000`; a left-facing turn costs our sim a tick and the
original nothing (`0x0041C67E`, entangled with round 8's left-hand expectation, so they must
be taken together); animation strips 1, 5 and 14 unnamed; panel indices 2, 5, 6, 8, 9 (jump
table at `0x0045A4DD`); dig's `0.75` at `controller+0x1D8`; `[+0x4a]`; the `0x2` and `0x200`
blast skips (read, never measured — no reachable level puts either inside a bombable
lemming's footprint); inverted step-up / climb / dig after an anti-grav flip; the remaining
17 call sites of `0x00430BA1`; and the `"Cylinder"` chunk still being found by a linear byte
scan rather than from a chunk table.
