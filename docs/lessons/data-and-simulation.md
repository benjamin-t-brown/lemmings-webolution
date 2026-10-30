# Data formats, simulation, and gadgets

> Dated historical research excerpts, preserved in their original wording and order within each source range.
> Earlier conclusions may be superseded by later entries; this is not current project status or
> authorization. See [STATUS](../STATUS.md) for current scope and [LESSONS](../LESSONS.md)
> for essential startup guidance. Some excerpts refer to findings that appeared earlier in the
> former combined file; use their linked evidence and the source-range comments below.

<!-- Source lines 1146-1288 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## The data formats — solved

- **Container**: `lembox`, LRZ-compressed, base plus patch overlay, read from the user's
  install at runtime. `lrtool` lists, extracts and converts. All 10,376 textures decoded and
  visually verified; all 73 retail `.WAV` decode with a load-time resampler.
- **`.SPR` sprite sheets** carry temporal entries, each containing ordered directional
  images. The former “walk 12 / block 8 / fall 7 views” claim reversed the axes;
  see the A3 checkpoint above. Direction counts are five for walk/fall, eight for
  block/cartwheel, one for shrug across all six lemming resources. Frames reference
  sheets `$LemSheet2`..`$LemSheet10`. Sheet
  pixel size is recovered from the rectangle and its normalised form, and **only the
  Y-then-X reading gives a sheet's real size** — transposed axes were a real bug here.
  Only the six lemming sheets carry a palette, and all six share it.
- **`.LVL` chunk order**, anchored at the literal `"Cylinder"` chunk, lands on the collision
  grid in all 102 levels: `u32 8,"Cylinder"` / `u32[5]` zero / `u32 height` (96 everywhere) /
  `f64 0.36` / `u32 N,u32[N]` surface indices (49858) / `u32 R, R x 12 bytes` / **`u32 C,
  u32[C]` the collision grid, C == 256*height** / `u32 B, u8[B]` (12288).
- **The collision mask is stored, not computed.** 256 x 96 `u32` cells, length-prefixed,
  byte-identical to the grid the running engine collides against (verified: 98,304 bytes at
  offset 240,011 of `MM_CLIMB_002.LVL`, zero bytes differing).
- **The surface lattice, `PolyList` and `SurfaceMesh` are the *render* input, not
  collision.** `rasteriseSurface` must never be in a collision path. This one cost a whole
  phase: our rasteriser's sparse masks were never the terrain.
- **`RELEASERATE.PC` and `LEVELMAP.DAT` hold literal seconds.**

### Cell flags, every one with a reader instruction behind it

| bit | meaning |
| --- | --- |
| `0x0001` | solid and destructible (builder's `or 1` at `0x004101F8`) |
| `0x0002` | indestructible (tested first, never cleared) |
| `0x0004` / `0x0008` | no bash right / left |
| `0x0010` / `0x0020` | no dig upright / inverted |
| `0x0080` | not landable — the faller skips it (`0x0041A99A`) |
| `0x0040` | water / acid drown (rounds 14–15) — side-effect `0x2f` tests it at `0x00415386`, attaches drown `0x004B0570`, ~2.0 s. Acid is the **same** path, no separate state |
| `0x0100` | authored-only (round 31) — 3 levels, no runtime setter. Sets `lem[+0x5f]` |
| `0x0400` | door / gate stamp (round 31) — vtable `0x004B0F00`, `|0x403` / `&0xfbfc` |
| `0x0800` | blocker field — **runtime only**, 0 of 2,506,752 shipped cells |
| `0x1000` | trampoline bounce (round 13) — fall-land `0x00416560` attaches bounce `0x004B0810` *before* splat. **0 of 102** authored; painted at runtime by trampoline enter `0x0041B838` |
| `0x8000` | anti-grav pad — exactly the 18 levels embedding `anti_grav.a3d`, audited twice |
| `0x0200` / `0x2000` / `0x4000` | object zones — see below |

Zone bits `0x200` / `0x2000` / `0x4000` are **splat exemptions** (round 29): fluid `+0x128`
0/1/2 stamps them, the hatch sets `[+0x4c]` via `0x004BFFC8`, and land `FUN_00416433` skips
the splat for `0x200` with `[+0x4c]==1` and `0x4000` with `[+0x4c]==2`. Lava has no arm.

### Geometry and coordinates

- File grid is **Y-up**; `sim::Terrain` is Y-down. File row `r` maps to terrain row
  `height - 1 - r`.
- The engine turns a world Y into a file row by **truncation** — its float-to-int helper
  `0x0049C5AC` sets x87 rounding to chop — **not** `floor(worldH - y)`. The two differ by one
  cell for an integer Y, and that one cell has bitten twice.
- Actor `+0x08` is not a 3D position: the model position `+0x108` is the cell-mesh
  interpolation `0x43D56C` of it (8-unit columns `& 0x1F`, 16-unit rows, Z = shell blend).
  See [A6a](../scratch/phase-a6-actor-pose.md). Writers: [A6d10](../scratch/phase-a6-sim-plus8-writer.md).
  `+0x360` added by `0x42F22E` is a facing bias (0 / −1), not a cell origin.
- Hatch spawn is the two int16 at hatch `+0x9A` / `+0x9C` (plus count `+0x98`, type `+0xB0`),
  **not** the entrance model's translation: on all 144 retail hatch records the stored Y is
  the translation's world Y minus 2, and that difference is stored, not computed.
- The exit save test is the **general object-overlap AABB** at `0x00430BA1`, inclusive on
  every edge (mins `test ah,1`/`jne`; maxes `test ah,0x41`/`je`), against the lemming's raw
  `(X,Y,Z)` at `+8`, with Z a fixed `[0,1]` slab. The box is `[round(ang x width)±1] x
  [ty−9..ty−6]` — 3 columns by 4 rows, twelve cells — and round 12 dumped it on a level of
  each of the **four** retail exit models (`Exit.a3d` 84, `Exit_water.a3d` 14,
  `Exit_any.a3d` 9, `Exit_acid.a3d` 1): all the same box.
  **The column centre is `round`, not `floor`** — round 11 said floor, and 46 of 111
  placements disagree, including `MM_FLOAT`.
- `0x00430BA1` is the engine's general gadget-overlap predicate with **19 call sites**.
  Identified: anti-grav `0x0041C1E3`, the exit, teleport `0x00435467` / `0x004354CB`,
  SpeedPad via `FUN_00432742`, lever `0x00436D30`.

## The simulation — measured, and worth keeping

**The "5 Hz, measured three ways" claim below is withdrawn as of D1m (round 2026-10-21,
Phase D) — see [D1m](../scratch/phase-d1-stepper-clock.md).** Two of the three old
measurements rest on the billboard-quad centroid method this project's own guardrails
already flag as unreliable for a closely related claim (the climbSpeed quad-lock
retraction, round 10); re-run against the actor list, the same level shows the *opposite*
pattern (position changes on 97% of consecutive 20 ms samples, not the ~10% a flat 200 ms
step would produce). The third measured the hatch's release gap, not the lemming stepper,
and D1d already found the hatch's own countdown is itself continuous. D1m's own bounded
disassembly read of the walk stepper (`0x0041BDD4`) found a mutable, re-checked,
byref time-budget loop — a shape with no reason to exist under a fixed step — and a
zero-fitted-parameter continuous replay collapsed the fall/landing-tick mismatch count by
76–100% against three live captures. **The conclusion is not yet "the tick rate is
wrong"** — D1m did not re-measure/re-implement the table, and `World::tick()` retains
200 ms bookkeeping; D1n's later opt-in implementation is recorded below — but the premise that lemmings *step* discretely at 5 Hz,
rather than moving continuously and being *sampled* at 200 ms, no longer has supporting
evidence and has active evidence against it. [D1n landed](../scratch/phase-d1-continuous-stepper-clock.md)
(`1d68ee0`, landing `24ec965`): opt-in continuous fall/walk budgets, not D1 acceptance.
[D1o](../scratch/phase-d1-continuous-live-recheck.md) (`a2bb9e7`) still fails the unchanged
gate (retained657/305 divergences off/on); D1 remains user-parked. Read the table's
per-tick arithmetic as 200 ms bookkeeping, not a confirmed discrete hardware step rate.

The tick was **measured three ways** as 5 Hz (200 ms) and not frame-locked — see the
withdrawal above. It had been guessed at 20 Hz, which made everything timed in ticks run
four times too fast; that specific correction (20 Hz → 5 Hz bookkeeping) stands regardless
of D1m, since the alternative is now "continuous, sampled at 5 Hz," not "20 Hz."

**Durations are animation lengths, not tick counts**: `frames x 0.06 s` (the double at
`0x004B1940`), fired when the animation's integer percent reaches a stored frame. A single
"interval in ticks" cannot reproduce bash acting every 7 or 8 ticks averaging 7.2.

Every constant below is a read of the shipped bytes **and** a live series:

| | value | |
| --- | --- | --- |
| `walkSpeed` | 1 column / tick | |
| `climbSpeed` | 1 cell / tick | the 5.0 at `0x004B10B0` |
| `fallSpeed` | 4 | `20.0 * 0.2 s` |
| `floatSpeed` | 2 | **not a constant** — a division of `fallSpeed` by the 2.0 at `0x004B03F8` |
| `floatAfter` | 10 | umbrella latch |
| `floatOpenMs` | 480 | halving is delayed: `umbopen` is strip 12, 8 frames |
| `splatDistance` | 30 | 29 survives, 31 dies |
| `maxStepUp` | 2 | the probe is unrolled exactly twice |
| `lemmingHeight` | 3 | ceiling bias only |
| `climberTopOut` | 2 | `0x00413B22` subtracts `[+0x44]` once where the ceiling arm subtracts three |
| `hoistCycleMs` | 840 | strip 8 `flip`, 14 frames. At a 200 ms tick, 800–999 are indistinguishable |
| `hoistLift` | 2 | X stays on the wall column |
| `brickLength` | 2 | at percent 81; 12 bricks then a shrug |
| `blockerFieldHeight` | 4 | one column x four rows |
| `bomberFuseMs` | 5000 | then Ohno, then a further 1020 ms, then the crater |
| blast | **stored 7x7 stencil** at `0x004B1528`, 37 of 49 cells | |
| anti-grav | cell bit `0x8000` negates `[lemming+0x44]` via cartwheel `0x004B0CA8` type `0x2b` | |

**The walk probe snaps X to an exact integer before every test.** `0x0041BECE` truncates and
`0x0041BEDC` stores the integer back, then `0x0041BEE6` probes; `0x0041BF61` picks the next
boundary as `trunc(X)+1` facing right and `trunc(X)` facing left. Truncation gives a shared
edge to the right-hand cell, so a right-facing walker stops **on** the wall column and a
left-facing one at wall+1. This is why climbers sit at exactly X 232.000 / 128.00 / 64.000.

**Formerly a known divergence, fixed in Phase D (D1f, 2026-10-21)**: a walker that *was*
facing Left, turning to face Right at a wall, used to cost our sim a tick the original
never loses (`0x0041C67E`) — the original's turn arm returns 0 so its slice loop
(`0x0041BDD4`) keeps going in the same tick and the walker steps once more immediately.
`World::stepWalker` now does the same, gated on the facing the walker had *entering* the
call (not the facing a turn just produced), and only for that direction — a walker that
*was* facing Right already consumed the tick moving onto the wall column, so it must not
get the extra step. One `testBlockerTurnsWalkers` literal moved (131→132) as a direct,
justified consequence; the already-good "was Right, turns Left" case is unaffected and
still matches. See `docs/scratch/phase-d1-left-turn-fix.md`.


<!-- Source lines 1414-1434 in the pre-split docs/LESSONS.md; excerpt preserved, relative links rebased. -->
## Gadgets — all settled, rounds 9 and 13–31

Implemented in `src/sim` and not to be re-investigated:

| gadget | verdict |
| --- | --- |
| **Anti-grav** | cell bit `0x8000` + an anti-grav object; negates `[lemming+0x44]` through cartwheel `0x004B0CA8` type `0x2b`. The bit alone does nothing |
| **Trampoline** | cell bit `0x1000`, painted at runtime by trampoline enter `0x0041B838`; bounce `0x004B0810` attaches on fall-land **before** the splat arm. `pressure_pad.a3d` is *not* the source |
| **Water / acid** | the same path — cell bit `0x40`, side-effect `0x2f`, drown `0x004B0570`, ~2.0 s. No separate acid state; type `0x2e` unused in retail. `Entrance_water` / `Entrance_acid` are sheets and zones, not the hazard |
| **Teleport** | object-only, no cell bit (18/18). `teleport.a3d` vtable `0x004B1250`, partner via `+0x260` from id `+0x98`, attach `0x28`→`0x2c` with a 4-tick countdown, relocate `0x0043503C` (Y+2.0). AABB 12x8, facing preserved |
| **SpeedPad** | object-only, no cell bit (9/9). `speedup_two_way.a3d` vtable `0x004B1178` → FASTER `0x004B0A38` for 5.0 s; `0x0042E2BF` doubles the main state while boosted. AABB 2x6 |
| **Lever** | object-only. `lever.a3d` vtable `0x004B1288`; overlap `0x00436D30` facing-gates against `+0xb8`; toggles and fans type 4/5 messages. **No lemming attach** |
| **Splat exemptions** | the zone bits, round 29 — see the cell-flag section |
| **Inverted steppers** | after an anti-grav flip, step-up / climb / dig match upright with gravity inverted; dig refuses on `0x20` when `[+0x44] == -1` (`0x00415A96`) |

Clean negatives — cues that are **not** gadgets, so do not go looking for one:
`Balloon` and `Fire` and `ForceF` have **zero** play references in retail; `Witch` is a pause
easter egg on `e10_kf_1` only; `Axe` is a HUD sprite sheet; `Bubble` is SFX on fluid type
`0x23`; `Scrape` is the dig/bash refusal SFX; `Rumble` is an object state-commit SFX;
`Changeop` is the skill-panel and options-slider SFX; `Die` is SFX on type-`0x23` enter.


