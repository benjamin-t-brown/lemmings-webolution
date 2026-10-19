# What we already know

Hard reset of 2026-10-18. Everything retired lives under `old/` — `old/PLAN.md`,
`old/docs/` (the previous plan, four handoffs, 42 round notes), `old/src/gfx` and
`old/src/shell`. Nothing was deleted. This file is the carry-forward: the facts worth
knowing before you touch anything, each one with the address or the script behind it.

**Read this before the plan.** It exists so a new agent does not re-derive, re-measure, or
re-litigate what cost previous sessions weeks.

## The one rule

The stock 640x480 `Lemmings Revolution.exe` is the authority — not classic Lemmings, not
plausibility, not a decompiler's prose. A claim needs **an address and the bytes read
there**, or a re-runnable script. A decompiler line is not a measurement.

SHA-256 of the authority: `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x400000`, 32-bit PE.

## Camera checkpoint: A1 remains open

The accepted pose/lens operand reads and their exact addresses are in
[the A1 facilitator check](scratch/phase-a1-camera.md). Its standalone fixture has
independent original-image mutation/restore proofs; it is not a finished renderer.
The initial, grabbed-mouse and right-key regions pass the predeclared criterion. The
left-key frame fails, and numeric near/far planes remain unknown. Do not list either as
settled or restart the completed simulation work.

The scale helper executes **FPTAN**, not sine (`0x0047B095` calls `0x0049D8D4`,
`D9 F2` at `0x0049D8F2`). Projection divides by transformed depth and reverses screen y
(`0x0047D0C0`, `0x0040EAE0`). The captured center was `(319,239)` at stock resolution,
from BSS globals `0x0058D294` / `0x0058D298`; each component has a separate pixel mutation.
The conventional center assumption and earlier sine interpretation were explicitly
corrected. Exact original floating-point intermediate precision is still unestablished.

Control action names do not imply unconditional rotation. The captured default path
moved the virtual cursor without a grab; a grab over empty scenery changed the meaning of
the same inputs. Window activation and selection matter. Preserve the failed view and the
scope limits in the linked note before proposing the next round.

The [active cylinder draw follow-up](scratch/phase-a1-cylinder-draw.md) corroborates a
bounded frame-time inner-bank submission at the retained left camera and matches its
source vertex pool to the stored level. It does not fix the outer-only fixture's failed
pixels. Use the new foreground-acquisition helper from a hidden worker; it masks
activation input from the game and verifies actual focus. Do not require a manual click
for every probe or use the older input-queue attachment helper.

The [cell-fixture replay](scratch/phase-a1-cell-oracle.md) retains the subsequent
failed geometry candidate, isolated mutation survivors and explicit corrections.
Its source/runtime mapping is READ evidence; no new selection or raster value is
pixel-accepted. The next probe follows a selected quad through the original's final
device-vertex emission rather than repeating the camera reads or cell capture.

## Hard-won rules of thumb

1. **One cell is one world unit.** Never multiply a height or distance by
   `PIXELS_PER_UNIT`. The factor-of-2 lattice raster is retired, and doubling a world-unit
   delta is the single most repeated mistake in this project's history — it survived in the
   plan's own prose for weeks after being disproved.
2. **Expect the wrong *shape*, not the wrong number.** Every provisional value that turned
   out wrong was wrong structurally: three "intervals in ticks" were animation cycles; a
   blocker "list scan" was a terrain field; an "explosion radius" was a stored bitmap; a
   "wait of 2 ticks" was a 14-frame animation; an exit "rectangle" was an object AABB.
3. **Halving a classic-Lemmings value leaves a guess.** `maxStepUp` and `lemmingHeight` came
   out 2 and 3, not the halved 3 and 4. The blast clears 37 cells, not the 81 a halved
   radius-5 circle would.
4. **A plausible series can come from the wrong object.** A billboard-quad probe was fitted
   to "a lemming" and produced clean, plausible numbers for **nine rounds** before anyone
   noticed the quad was not a lemming. Two published measurements had to be retracted.
   Read lemmings from the **actor list at `0x004CBB18`**.
5. **Check `.data` rawsize before citing a byte.** `.data` has va `0x004BF000`, vsize
   `0x1185B8`, but **rawsize only `0x7000`** — file bytes exist only to `0x004C6000`. A
   round once asserted a gate byte of `0xFF` that the loader actually zero-fills.
6. **Mutate one thing alone and demand a behavioural failure.** A test whose expectation is
   derived from the field it tests passes at any value. Reverting a whole round at once
   proves only that *something* was tested.
7. **A clean negative is a result.** So is "this does not appear in any reachable level".
8. **Preserve quirks.** German menus, RETURN only when the menu highlight is exactly 0, a
   short ESC that fails to leave pause, the drawn crosshair disagreeing with the tested
   cursor. All real. Do not smooth them out.

## The data formats — solved

- **Container**: `lembox`, LRZ-compressed, base plus patch overlay, read from the user's
  install at runtime. `lrtool` lists, extracts and converts. All 10,376 textures decoded and
  visually verified; all 73 retail `.WAV` decode with a load-time resampler.
- **`.SPR` sprite sheets** carry, for each action, **several views — not variations**. A
  lemming on the far side of the tube is seen from a different angle, so `walk` has 12
  views, `block` 8, `fall` 7. Frames reference sheets `$LemSheet2`..`$LemSheet10`. Sheet
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

The tick is **5 Hz (200 ms)**, measured three ways and not frame-locked. It had been guessed
at 20 Hz, which made everything timed in ticks run four times too fast.

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

**Known divergence, deliberately unfixed**: a left-facing turn costs our sim a tick and the
original nothing (`0x0041C67E`). Fixing it perturbs the blocker-turn expectation, so the two
must be taken together.

## The tooling

- **Static disassembly**: `python tools/harness/read_steppers.py --at 0x00430BA1 --len 0x90`
  (also `--states` for the 24 animation sites and 46 vtables, `--cell-flags`). Capstone.
- **REA** (Ghidra-backed decompilation) — see `docs/REA_SETUP.md`. This is the new primary
  tool for reading code we cannot measure live, which is most of rendering.
- **Live reads**: 40+ scripts under `tools/harness/`, all re-runnable, driven through
  `tools/dinput-shim`. `measure_steppers.py --compare walker|climber` is the per-tick oracle.
  `read_lvl_grid.py` and `read_grid.py` read the stored and live grids.
- **Screenshots**: `screengrab.py` captures via a **desktop BitBlt** — `PrintWindow` returns
  all black because dgVoodoo presents through a D3D11 swapchain. Needs the game foreground.
- **`compare_tick.py` is superseded** — it fits a billboard quad that is not a lemming. Do
  not quote its numbers.

### Environment traps, all learned the hard way

- Build through `.\scripts\Invoke-Ucrt64.ps1 "<cmd>"` (MSYS2 UCRT64). Test binaries need
  UCRT64 `bin` on `PATH` or they fail to load silently.
- **Do not put UCRT64 `bin` on `PATH` for `tools/harness/*.py`** — it shadows the default
  Python and `capstone` disappears.
- `tools/dinput-shim` is 32-bit mingw32, not UCRT64.
- `lrstate.attach` returns `(handle, pid)`. `read_lvl_grid.find_grid` returns
  `(offset, cells)`.
- **Skill assignment must go through the panel.** Proof is the stock count at `0x004C91D8`
  **decrementing**, not a sprite change. Panel indices known: 0 climber, 1 parasol, 3
  blocker, 4 builder, 7 digger; 2, 5, 6, 8, 9 unidentified.
- **Cap live sampling.** An uncapped single-lemming run at ~300k `ReadProcessMemory`/s killed
  two game processes. `--hz 2000` is safe.
- `goto_level` **can silently fail** — it once reported entering one level while the loaded
  path stayed another, and the dump was of the wrong level. Read back the loaded path.
- `goto_level.focus()` has crashed dgVoodoo on this machine.
- **Correction to the retired `2 = pause` shortcut:** P gameplay pause is `ui_state 1`
  (store at `0x0045D226`), and P resumes `0` (store at `0x0045D508`). The clock was
  independently observed stopped/running/stopped; changing only the harness's pause
  classification to the old value failed, then its exact bytes were restored and the
  suite passed. Do not conflate gameplay pause with the separate menu state.
  Frontend screen id `0x004C416C` remains: 3 menu, 4 confirm, 5 map.
- If you write to the live collision grid (a legitimate, proven probe), **log every cell and
  restore all of them**.

## Useful strings the engine keeps

Cue and UI element names, packed null-terminated in `.data` from `0x004C0C7C` (inside
rawsize, so readable from the image):

`Letsgo`, `Yippee`, `Chink`, `Explode`, `Bubble`, `Scrape`, `Ohno`, `Changeop`, `Splat`,
`ForceF`, `Die`, `FallSplat`, `Lever`, `Mine`, `MineMetal`, `Rumble`, `AntiGrav`, `SpeedPad`,
`Action`, `Balloon`, `Fire`, `Witch`, `Bits`, `GreyIcons`, `BlackIcons`, `AdditiveIcons`,
`Axe`, `Smoke`, then cursors `_curNoLem`, `_curLem`, `_curNoLemLeft`, `_curLemLeft`,
`_curNoLemRight`, `_curLemRight`, `_curEnt`, `_curExit`, `^curGrab`, `_curMseDwn`,
`+shadow_new`, `^tab`, `^tab_selected`, `^out`, `^saved`, `^time`.

`^saved` / `^out` / `^time` are the HUD counters, and `_curExit` means the engine has its own
exit hover test — both matter for Phase C.

**Every cue in that list has now been chased down** (rounds 18–28), and most were clean
negatives. Do not re-investigate them; see the gadget table below.

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

## What the hard reset was for

**Thirty-one research rounds** measured the simulation correctly — physics, all nine skills,
every gadget — and left the *visible* game wrong: it booted into a flat 2D debug view, drew
every lemming from sprite view 0 regardless of viewing angle, and showed a placeholder menu
in our own font. The simulation was never the problem, and none of it is being thrown away.
The plan is now about what you can see — `PLAN.md`.

**Note on paths**: the 42 round notes moved to `old/docs/scratch/`. Comments in
`src/sim/World.h` and elsewhere still cite them as `docs/scratch/phase3-*.md`; read that as
`old/docs/scratch/`.
