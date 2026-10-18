# Round 11 brief — the exit box

This file is a **self-contained prompt**. It assumes no prior conversation, so it can be
pasted into a fresh session — a different agent, a different service, a different machine —
and stand on its own. Copy everything below the rule.

Its evidence standard, environment traps and must-not-touch list are the same as
`docs/ROUND9_BRIEF.md`; they are repeated here rather than referenced so this file can be
pasted alone. `docs/ROUND9_BRIEF.md` remains the brief for the gadget repeats.

Written 2026-10-16, with the tree green at 16/16 and rounds 1–10 landed.

---

You are picking up Phase 3 of Lemmings Webolution at `C:\progs\lemmings-webolution`, a 1:1
reimplementation of Lemmings Revolution. **The stock 640x480 executable is the authority** —
not classic Lemmings, not plausibility, not a decompiler's guess.

## Read first, in this order

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | the evidence standard; non-negotiable |
| `.claude/rules/windows-msys2-build.md` | build + PATH traps |
| `docs/PHASE3_HANDOFF.md` | the dispatch doc: rounds table, `Rules` status, "how to read a constant" |
| `PLAN.md` section 1.6 "Where to go next" | every landed result, newest last |
| `docs/scratch/phase3-round9-antigrav.md` | the object-overlap machinery you will reuse |
| `docs/scratch/phase3-round8-blocker.md` | the live-write-and-restore technique |

## Your round: measure the exit box

**This is the last structural guess in the simulation, and it decides whether the game can
be won.**

`src/sim/World.h` declares:

```cpp
struct Exit { int x = 0; int y = 0; int width = 8; int height = 8; };
```

`World::atExit` (`src/sim/World.cpp:165`) tests a lemming against that rectangle, and
`World::step` uses it to set `State::Exited`, increment `saved_`, and fire `SoundCue::Exit`.
`won()` is `saved_ >= setup_.saveTarget`. So the rectangle is the win condition.

**Every value in it is invented.** `main.cpp` hand-picks `12 x 14` at offset `(x - 6,
y - 12)` in all five places it builds one — lines 118, 139, 173, 204 and 214 — and the
struct's own default is `8 x 8`. Nothing measures any of it: `PLAN.md`,
`docs/PHASE3_HANDOFF.md` and all 21 scratch notes contain **zero** hits for an exit width,
height, box or radius. The asset layer gives you no fallback either —
`LevelObjects::exits` is a bare `WorldPose` point with no extent (`src/assets/LevelObjects.h`).

This is the same shape as the errors this project has already caught. `explosionRadius` was
a plausible halved classic value until the original turned out to store a 7x7 stencil
clearing 37 cells where the guess cleared 81. Assume the exit test is *not* a rectangle you
can guess.

### Three leads, all measured — start from these rather than searching

These were read out of the executable before this brief was written. Re-verify them; do not
take them on faith.

**1. There is a general object-overlap predicate, and it has 19 call sites.**
`0x00430BA1` takes an object as `this` and a lemming-derived argument, and returns a byte.
Round 9 proved it gates anti-grav. A `.text` scan for `E8` call sites resolving to it finds
**19**, the first fourteen being:

```
0x0041C1E3  0x0041C9F6  0x0041CA6A  0x0041CA9A  0x0041CABE  0x0041CF40  0x0041CFB1
0x0041D442  0x0041D45B  0x00426B5B  0x0042C7E9  0x0042C82D  0x00430C7B  0x00431AD3
```

`0x0041C1E3` is the anti-grav one. The exit test is very likely among the others. Identify
which, and read what bounds it compares — Round 9 explicitly left the body of `0x00430BA1`
unread, so you are closing that debt too.

**2. Do not assume `0x004C8958` is "the objects".** It is the anti-grav list specifically.
An imm32 scan of `.text` finds it at exactly **two** sites, `0x0041C192` and `0x0041C1A5`,
both inside the anti-grav arm of the walk probe. The container accessors it uses
(`0x0041CB60` count, `0x0041CB40` element-at) are generic and are called from elsewhere, so
expect the exit to walk a *different* list through the same accessors.

**3. The engine names its UI elements and its sound cues as strings, and two of them point
straight at the save path.** Packed null-terminated strings in `.data`, read from the image
(this region is inside `.data`'s rawsize — see trap 4 below):

```
0x004C0C7C  Letsgo     0x004C0C84  Yippee     0x004C0C8C  Chink
0x004C0C94  Explode    0x004C0C9C  Bubble     0x004C0CA4  Scrape
0x004C0CAC  Ohno       0x004C0CB4  Changeop   0x004C0CC0  Splat
0x004C0CC8  ForceF     0x004C0CD0  Die        0x004C0CD4  FallSplat
0x004C0CE0  Lever      0x004C0CE8  Mine       0x004C0CF0  MineMetal
0x004C0CFC  Rumble     0x004C0D04  AntiGrav   0x004C0D10  SpeedPad
0x004C0D1C  Action     0x004C0D24  Balloon    0x004C0D2C  Fire
0x004C0D34  Witch      0x004C0D6C  Axe        0x004C0D70  Smoke
...
0x004C0DC8  _curEnt    0x004C0DD0  _curExit   0x004C0DDC  ^curGrab
0x004C0E18  ^out       0x004C0E20  ^saved     0x004C0E28  ^time
```

Two of these are your route in:

- **`^saved` at `0x004C0E20`** is the on-screen saved counter. Find what writes it and you
  have the save path, and the exit test is immediately upstream of it.
- **`_curExit` at `0x004C0DD0`** is a dedicated hover cursor for an exit, so the engine
  already has an exit hit-test for the mouse. It may or may not be the same box the lemming
  is tested against — if it is a *second* box, that is a finding, and the kind of quirk this
  project preserves rather than unifies.

The `Yippee` string at `0x004C0C84` is the cue our sim already fires, so it also anchors you
in the right code.

### What done looks like

The exit's real test, read and then measured live: which bounds, relative to what origin,
on which axes. If it turns out to be an object predicate rather than a rectangle, say so and
model it as one — do not keep the rectangle because the struct already has one.

A live series is a walker walking into an exit while you watch the saved counter increment,
and a binary search on the column and row where it fires. That is the technique that pinned
the blocker field's edges to `X = C` going right and `X = C + 1` going left, and it is the
right one here.

Then fix all five `main.cpp` construction sites and the struct default, and give
`LevelObjects` whatever extent the measurement says an exit has.

## Evidence standard — the project has been burned by each of these

1. **A claim needs an address and the bytes read there, or a re-runnable script.** A
   decompiler line is NOT a measurement. Disassemble with:
   ```
   python tools/harness/read_steppers.py --at 0x00430BA1 --len 0x80
   python tools/harness/read_steppers.py --states        # animation sites + vtables
   python tools/harness/read_steppers.py --cell-flags
   ```
2. **Write the prediction down BEFORE the live run**, including what would falsify it. The
   strongest results in this project (the step-up pillars, the blast stencil, the anti-grav
   clean negative) all did this.
3. **Read lemmings from the actor list at `0x004CBB18`** — never from billboard quads.
   Fitting a quad to a lemming is a fit, and it fitted a *non-lemming* for nine rounds while
   producing a plausible series; two separate "measurements" had to be retracted.
   `tools/harness/compare_tick.py` is **superseded** — do not use it or quote its numbers.
   Use `tools/harness/measure_steppers.py --compare walker|climber` instead.
4. **Before citing any `.data` address from the executable image, check it falls inside that
   section's rawsize**, not just its virtual size. `.data` has va `0x004BF000`, vsize
   `0x1185B8` but **rawsize only `0x7000`** — so bytes exist in the file only up to
   `0x004C6000`. A round once asserted a gate byte of `0xFF` that was a read past the raw
   data and is actually zero-filled by the loader. The string table above is at
   `0x004C0C7C`–`0x004C0E28`, comfortably inside. `.rdata` is fine throughout.
5. **One cell is one world unit.** Never multiply a height or distance by
   `PIXELS_PER_UNIT` — the factor-of-2 lattice raster is retired, and doubling a world-unit
   delta is the single most repeated mistake in this project's history. A stale "climbSpeed
   is 2, which is two mask rows" line survived in `PLAN.md` until 2026-10-16.
6. **Do not invent rules from classic Lemmings, and do not halve classic values.** Halving a
   guess leaves a guess: `maxStepUp` and `lemmingHeight` came out 2 and 3, not the halved 3
   and 4.
7. **Expect provisional things to be the wrong *shape*, not the wrong number.** Three
   "intervals in ticks" were animation cycles (`frames x 0.06 s`, the double at
   `0x004B1940`); a blocker "list scan" was a terrain field; an "explosion radius" was a
   bitmap; a "wait of 2 ticks" was a 14-frame animation; anti-grav was a cell bit plus an
   object-overlap test. **The exit box is the most likely candidate yet for being the wrong
   shape.**
8. **A clean negative is a publishable result.** So is "the cursor box and the save box are
   different".
9. **Preserve quirks.** If the original does something that looks wrong, ours does it too.

## What is already settled — do not redo any of it

Rounds 1–10 are closed and every field in `Rules` (`src/sim/World.h`) is a read or a live
measurement. Do not relitigate: `ticksPerSecond` 5, `walkSpeed` 1, `climbSpeed` 1,
`fallSpeed` 4, `floatSpeed` 2, `floatAfter` 10, `splatDistance` 30, `maxStepUp` 2,
`lemmingHeight` 3 (ceiling bias only), `climberTopOut` 2, `hoistCycleMs` 840, `hoistLift` 2,
`floatOpenMs` 480, `brickLength` 2, `builderCycleMs` 960, `shrugCycleMs` 960, `basherReach`
1, `basherHeight` 4, `diggerWidth` 3, `minerReach` 2, the bash/dig/mine millisecond cycles,
`blockerFieldHeight` 4, `bomberFuseMs` 5000, `exploderCycleMs`, the 7x7 blast stencil at
`0x004B1528`, and anti-grav as cell bit `0x8000` (18 of 18 levels, audited twice).

Phase 2 terrain research is finished — do not restart it. The 102-level corpus survey (99
releasable) is settled; do not retune it.

## Tests

Every behavioural change needs a test that **fails against the old behaviour**. Verify that
by actually reverting the change (or flipping the new field **alone** — not bundled with
others) and watching the test fail on **behaviour**: a tick-by-tick position series, a state
transition, a saved count. A test whose expectation is derived from the field it is testing
passes at any value; that has happened here and was caught twice. Then restore and get the
whole suite green (16 tests).

**There is a known test gap you may as well close while you are here**: nothing covers
`Terrain::fromCollisionGrid` extracting the `0x8000` anti-grav bit — every existing test
builds pads by hand with `setAntiGrav`, so the extraction line could be deleted and the
suite would stay green. One test on a hand-built grid word fixes it.

## Build and test

Windows, MSYS2 UCRT64 — you are in PowerShell and do not inherit it.

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cmake --build --preset dev-debug"
.\scripts\Invoke-Ucrt64.ps1 "ctest --preset dev-debug"
```

Test binaries live at `build/cmake/dev-debug/*.exe` and need UCRT64 `bin` on `PATH` or they
fail to load silently — run them through `Invoke-Ucrt64.ps1`.

## Environment traps, all learned the hard way

- **Do not put UCRT64 `bin` on `PATH` for `tools/harness/*.py`** — it shadows the default
  Python and `capstone` disappears. Run harness scripts on the normal PowerShell/Python
  `PATH`.
- `tools/dinput-shim` is 32-bit mingw32, not UCRT64; use its own `build.sh`.
- `lrstate.attach` returns `(handle, pid)`, **not** `(pid, handle)`.
- `read_lvl_grid.find_grid` returns `(offset, cells)` — two values.
- Menus are German. **RETURN only works when the highlight is exactly 0.** A short ESC fails
  to leave pause. These are real behaviours; do not route around them by poking memory where
  a keypress is what you need.
- **Skill assignment must go through the panel.** Proof of assignment is the stock count at
  `0x004C91D8` **decrementing** — not a sprite change. `tools/harness/assign_skill.py`
  works. Budget ~3 s per assignment.
- **Short-lived states need kHz sampling, not sleep-then-read.** But **cap the rate** — an
  uncapped single-lemming run at ~300k `ReadProcessMemory`/s killed two game processes;
  `--hz 2000` is safe.
- If you write to the live collision grid (a legitimate, proven technique — see round 8),
  **log every cell and restore all of them**.
- `ui_state`: 0 playing, 2 pause, 3 frontend, 5 results. Frontend screen id `0x004C416C` is
  3 menu, 4 confirm, 5 map.
- Re-runnable scripts: `measure_steppers.py`, `measure_antigrav.py`, `measure_stepup.py`,
  `measure_blocker.py`, `measure_dig.py`, `measure_build.py`, `measure_bomb.py`,
  `measure_float_splat.py`, `read_lvl_grid.py`, `read_grid.py`, `assign_skill.py`,
  `goto_level.py`.
- If you cannot launch the game, a **static-only round is still a legitimate result**: read
  the predicate, name the bounds, write the predictions, and say clearly that nothing was
  measured live.

## Must not touch

`src/__test__/test_collisiongrid.cpp`, `src/__test__/test_corpus.cpp` and its expected
numbers, and any settled `Rules` field listed above. If a finding seems to require moving a
settled field, **stop and report that instead of moving it**. You *will* be changing
`src/sim/World.{h,cpp}`, `src/main.cpp` and probably `src/assets/LevelObjects.{h,cpp}` —
that is in scope for this round.

## Documentation — you own these

`PLAN.md` section 1.6, `docs/PHASE3_HANDOFF.md`, and `.claude/rules/match-the-original.md`.

**Append** your result; do not rewrite or compress what is there, and **add your bullet as
its own bullet** — the round-9 agent inserted one into the middle of an existing bullet and
orphaned its tail, which then read as current and revived three claims a later round had
already closed. Those files contain explicit "Correction." notes and retractions that were
expensive to establish, notably that `compare_tick.py`'s numbers are withdrawn and that
billboard quads are not lemmings. Do not undo them. Park working prose in
`docs/scratch/phase3-round11-exit.md`.

## Deliverables

1. `docs/scratch/phase3-round11-exit.md`: the addresses and bytes, the prediction written
   before the run, the live series, the verdict, and what you left open **with citations**.
2. The sim changes: `Exit` (or whatever replaces it), `World::atExit`, the five `main.cpp`
   sites, and any extent the asset layer should now carry.
3. New tests, verified to fail against the old behaviour, plus all 16 green.
4. `PLAN.md` + handoff updated by appending.
5. The state of the game process when you finish, and confirmation that every live write was
   restored.

**Report honestly.** Do not round, smooth, or over-claim; headline numbers here get
independently re-verified, and several rounds have had a claim narrowed or retracted on
re-check. Say plainly what you measured, what you only read, and what you could not settle.

## After this round

**Close Phase 2**, which is three bounded items and blocks nothing else: parse the collision
grid chunk through the level's chunk order instead of the fingerprint scan, read its seven
unread flag bits, and take `rasteriseSurface` out of the collision path entirely.

**Then the gadget repeats** — `docs/ROUND9_BRIEF.md`, one mechanic per round. The cue-string
table above is an inventory of what is still unmodelled and worth mining before picking one:
`SpeedPad`, `Balloon`, `Fire`, `Witch`, `Lever`, `ForceF`, `Axe`, `Bubble`, `Scrape`,
`Rumble`, `Changeop`, and a `Die` cue distinct from both `Splat` and `FallSplat`.

Also still open, smallest first: the test gap above; a left-facing turn costs our sim a tick
and the original nothing (`0x0041C67E`, entangled with round 8's left-hand expectation);
animation strips 1, 5 and 14 unnamed; panel indices 2, 5, 6, 8, 9 (jump table at
`0x0045A4DD`); dig's `0.75` at `controller+0x1D8`; `[+0x4a]`; the `0x2` and `0x200` blast
skips (read, never measured — no reachable level puts either inside a bombable lemming's
footprint); inverted step-up / climb / dig after an anti-grav flip; and the object-overlap
predicate's bounds, if this round does not close them.
