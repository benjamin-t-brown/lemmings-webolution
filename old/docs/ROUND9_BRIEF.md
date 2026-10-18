# Round 9 brief — gadgets

This file is a **self-contained prompt**. It assumes no prior conversation, so it can be
pasted into a fresh session — a different agent, a different service, a different machine —
and stand on its own. Copy everything below the rule.

It is written for whoever picks up Phase 3 after Round 10. Rounds 1–8 and 10 are closed;
`docs/PHASE3_HANDOFF.md` is the standing dispatch doc and this brief points at it rather
than restating it.

Written 2026-10-16, with the tree green at 16/16 and Round 10 landed in `PLAN.md`,
`docs/PHASE3_HANDOFF.md` and `.claude/rules/match-the-original.md`.

---

You are picking up Phase 3 of Lemmings Webolution at `C:\progs\lemmings-webolution`, a 1:1
reimplementation of Lemmings Revolution. **The stock 640x480 executable is the authority** —
not classic Lemmings, not plausibility, not a decompiler's guess.

## Read first, in this order

| File | What it gives you |
| --- | --- |
| `.claude/rules/match-the-original.md` | the evidence standard; non-negotiable |
| `.claude/rules/windows-msys2-build.md` | build + PATH traps |
| `docs/PHASE3_HANDOFF.md` | the dispatch doc: rounds table, `Rules` status, "how to read a constant", per-round findings |
| `PLAN.md` section 1.6 "Where to go next" | every landed result, newest last |
| `docs/scratch/phase3-round10-steppers.md` | the most recent round |
| `docs/scratch/phase3-round8-blocker.md` | the live-write-and-restore technique |

## What is already done — do not redo any of it

Rounds 1–8 and 10 are closed. Every field in `Rules` (`src/sim/World.h`) is now a read from
the executable or a live measurement; there are no provisional fields left.

Settled and **not** to be relitigated: `ticksPerSecond` 5, `walkSpeed` 1, `climbSpeed` 1,
`fallSpeed` 4, `floatSpeed` 2, `floatAfter` 10, `splatDistance` 30, `maxStepUp` 2,
`lemmingHeight` 3 (ceiling bias only), `climberTopOut` 2, `hoistCycleMs` 840, `hoistLift` 2,
`floatOpenMs` 480, `brickLength` 2, `builderCycleMs` 960, `shrugCycleMs` 960, `basherReach`
1, `basherHeight` 4, `diggerWidth` 3, `minerReach` 2, the bash/dig/mine millisecond cycles,
`blockerFieldHeight` 4, `bomberFuseMs` 5000, `exploderCycleMs`, and the 7x7 blast stencil
read from `0x004B1528`.

Phase 2 terrain research is finished — do not restart it. The 102-level corpus survey (99
releasable) is settled; do not retune it.

## Your round: Round 9 — gadgets. One mechanic, not all of them.

The level objects carry zone bits that the engine's landing path tests and our sim ignores.
Pick **exactly one** gadget mechanic (anti-grav, trampoline, water, acid, teleport, or a
hazard), settle it, and stop. A round that half-answers three mechanics is worth less than
one that closes a single one.

The lead is already found. In the landing path:

```
0x0041662B   and 0x200    / cmp [eax+0x4c], 1   -> jumps to the safe landing
0x00416641   and 0x4000   / cmp [edx+0x4c], 2   -> jumps to the safe landing
0x0041C151   and 0x8000   (found in round 10, in the walk probe, unfollowed)
```

`[+0x4c]` is a lemming type/kind field that read 0 on every one of two dozen live lemmings
sampled so far, so find out what sets it to 1 and 2. `src/assets/CollisionGrid.h` names the
zone bits and deliberately ignores them; `src/assets/LevelObjects.{h,cpp}` parses object
records. Start by identifying which bit belongs to which gadget, then take the one you can
reach in a real level.

## Evidence standard — the project has been burned by each of these

1. **A claim needs an address and the bytes read there, or a re-runnable script.** A
   decompiler line is NOT a measurement. Disassemble with:
   ```
   python tools/harness/read_steppers.py --at 0x0041662B --len 0x80
   python tools/harness/read_steppers.py --states        # animation sites + vtables
   python tools/harness/read_steppers.py --cell-flags
   ```
2. **Write the prediction down BEFORE the live run**, including what would falsify it. The
   two strongest results in the project (the step-up pillars, the blast stencil) both did
   this.
3. **Read lemmings from the actor list at `0x004CBB18`** — never from billboard quads.
   Fitting a quad to a lemming is a fit, and it fitted a non-lemming for nine rounds while
   producing a plausible series; two separate "measurements" had to be retracted.
   `tools/harness/compare_tick.py` is **superseded** — do not use it or quote its numbers.
   Use `tools/harness/measure_steppers.py --compare walker|climber` instead.
4. **Before citing any `.data` address from the executable image, check it falls inside that
   section's rawsize**, not just its virtual size. `.data` has vsize `0x1185B8` but rawsize
   only `0x7000`; a round once asserted a gate byte of `0xFF` that was a read past the raw
   data and is actually zero-filled by the loader. `.rdata` is fine.
5. **One cell is one world unit.** Never multiply a height or distance by
   `PIXELS_PER_UNIT` — the factor-of-2 lattice raster is retired, and doubling a world-unit
   delta is the single most repeated mistake in this project's history.
6. **Do not invent rules from classic Lemmings, and do not halve classic values.** Halving a
   guess leaves a guess: `maxStepUp` and `lemmingHeight` came out 2 and 3, not the halved 3
   and 4, and the blast turned out to be a stored 37-cell stencil where the halved guess
   cleared 81.
7. **Expect provisional things to be the wrong *shape*, not the wrong number.** Three
   "intervals in ticks" were animation cycles (`frames x 0.06 s`, the double at
   `0x004B1940`); a blocker "list scan" was a terrain field; an "explosion radius" was a
   bitmap; a "wait of 2 ticks" was a 14-frame animation. If a gadget looks like it wants a
   constant, check whether it wants a state, a field, or an animation length instead.
8. **A clean negative is a publishable result.** So is "this gadget does not appear in any
   reachable level".
9. **Preserve quirks.** If the original does something that looks wrong, ours does it too.

## Tests

Every behavioural change needs a test that **fails against the old behaviour**. Verify that
by actually reverting the change (or flipping the new field **alone** — not bundled with
others) and watching the test fail on **behaviour**: a tick-by-tick position series, a state
transition, a terrain change. A test whose expectation is derived from the field it is
testing passes at any value; that has happened here and was caught twice. Then restore and
get the whole suite green (16 tests).

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
- `lrstate.attach` returns `(handle, pid)`, **not** `(pid, handle)`. Backwards costs you a
  run and presents as a level-table failure.
- `read_lvl_grid.find_grid` returns `(offset, cells)` — two values.
- Menus are German. **RETURN only works when the highlight is exactly 0.** A short ESC fails
  to leave pause. These are real behaviours; do not route around them by poking memory where
  a keypress is what you need.
- **Skill assignment must go through the panel.** Proof of assignment is the stock count at
  `0x004C91D8` **decrementing** — not a sprite change. `tools/harness/assign_skill.py`
  works. Budget ~3 s per assignment. Panel indices confirmed: 0 climber, 1 parasol, 3
  blocker, 4 builder, 7 digger; 2, 5, 6, 8, 9 unidentified (`0x0045A0E0`'s ten-arm jump
  table at `0x0045A4DD` is unfollowed — do not fill them in from the classic order).
- **Short-lived states need kHz sampling, not sleep-then-read**: the dig state lives 271 ms.
  But **cap your sampling rate** — an uncapped single-lemming run at ~300k
  `ReadProcessMemory`/s killed two game processes; `--hz 2000` is safe.
- If you write to the live collision grid (a legitimate and proven technique for probing the
  engine's own arms — see round 8), **log every cell and restore all of them**.
- Useful re-runnable scripts: `measure_steppers.py`, `measure_stepup.py`,
  `measure_blocker.py`, `measure_dig.py`, `measure_build.py`, `measure_bomb.py`,
  `measure_float_splat.py`, `read_lvl_grid.py`, `read_grid.py`, `assign_skill.py`,
  `goto_level.py`.
- If you cannot launch the game, a **static-only round is still a legitimate result**: read
  the addresses, name the bits, write the predictions, and say clearly that nothing was
  measured live.

## Must not touch

`src/assets/*` and `src/__test__/test_collisiongrid.cpp`, `src/__test__/test_corpus.cpp` and
its expected numbers, and any already-settled `Rules` field listed above. If a finding seems
to require moving a settled field, **stop and report that instead of moving it**.

## Documentation — you now own these

`PLAN.md` section 1.6, `docs/PHASE3_HANDOFF.md`, and `.claude/rules/match-the-original.md`.
Previous rounds had a separate facilitator who owned them; you do not.

**Append** your result; do not rewrite or compress what is there. Those files contain
explicit "Correction." notes and retractions that were expensive to establish — notably that
`compare_tick.py`'s numbers are withdrawn and that billboard quads are not lemmings. Do not
undo them. Update the rounds table status and the `Rules` status table for what you settle.
Park all working prose in `docs/scratch/phase3-round9-<mechanic>.md`.

## Deliverables

1. `docs/scratch/phase3-round9-<mechanic>.md`: the addresses and bytes, the prediction
   written before the run, the live series, the verdict, and what you left open **with
   citations**.
2. The sim changes in `src/sim/World.{h,cpp}` and `src/sim/Terrain.{h,cpp}` for what you
   actually settled, each with a MEASURED or READ comment naming its address.
3. New tests, verified to fail against the old behaviour, plus all 16 green.
4. `PLAN.md` + handoff updated by appending.
5. The state of the game process when you finish, and confirmation that every live write was
   restored.

**Report honestly.** Do not round, smooth, or over-claim; the convention here is that
headline numbers get independently re-verified, and three of the last six rounds had a claim
narrowed or retracted on re-check. Say plainly what you measured, what you only read, and
what you could not settle.

## Still open, if you want a second round after this one

One per round, same discipline.

- **A left-facing turn costs our sim a tick and the original nothing.** Walk matched 27
  ticks then diverged at the turn (live 24, 25, 26; ours 24, 24, 25). Cause read at
  `0x0041C67E`; fixing it perturbs round 8's left-hand expectation, so it needs both taken
  together.
- Animation strips 1, 5 and 14 unnamed.
- Panel indices 2, 5, 6, 8, 9 (jump table at `0x0045A4DD`).
- Dig's `0.75` at `controller+0x1D8` unread; `0x00426DB0` unread; `[+0x4a]` unread.
- `is_lemming` in `measure_dropoff.py` is still a **fit** to one camera (demands `cy >= 220`
  where real boxes sit at y 77–126 on `MM_DIG_000`), left that way deliberately as a
  regression case.
- The `0x2` and `0x200` blast skips are read but not measured: no reachable level puts either
  inside a bombable lemming's footprint.
