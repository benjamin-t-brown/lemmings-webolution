# A4g — actor vertex-colour writers: found, but none explains walk/block 152

Queue id `a4g-actor-tint-writer`. Worker note retained at
`build/tmp/a4g-actor-tint-writer/round-note.md`; scanner
`tools/harness/scan_actor_tint_a4g.py`, hits in `scan-hits.json` beside the note.

## Result

The question from A4f stays open, narrowed. The live walk/block actor's controller
(`actor+0x128`) holds RGB 152.0 in all four vertex records; no static writer found so
far produces 152 for those states.

## What is READ (facilitator re-read the bytes)

- **Which object.** `capture_actor_background_a4f.py` reads the 152 records at
  `actor+0x128` + `0x08/0x70/0xD8/0x140` (+`0x38` RGB). The records are on the graphics
  controller, not on the `actor+0x78` draw object as the A4g brief and worker note say.
- **`0x406260`** is a uniform-RGB setter: one double written to R, G and B of all four
  records (e.g. `0x4062AC 894840`, `0x40626D 898878010000`). The worker lists nine
  direct call sites; the facilitator re-read two.
- **`0x404434`** (in `0x404372`, type-`0x23` vtable slot `0x4B03C4`) is the one computed
  caller: `0x4043B9 fld [Y+8]; fsub [[actor+0x58]+0x50]; fdiv [[actor+0x58]+0x98]`, clamp
  to `1.0` (`0x4B02D0`), `fmul 255.0` (`0x4B0400`), `ecx = actor+0x128`. Type `0x23` is
  the **fluid** state (rules: Bubble/Die cues from `0x404372`/`0x404261`), so this is a
  sinking fade. It does not apply to walk/block.
- **`0x40E9EC`** sets the controller of each new `0x43CE00` object (`+0xB0`, built by
  `0x4680f0`) to RGB `i·128/8 = 16·i` (`0x40E9B2 shl ecx,7`; `fdiv 8.0` at `0x4B07F0`).
  152 is not a multiple of 16, so this loop is not the 152 writer directly.
- In that same loop, `0x40E993` calls **`0x4682a6`** with `ecx = actor+0x128` and the new
  object's `+0xB0` as argument, i.e. the actor's controller and a same-class prototype
  are linked there. The copy direction and whether it carries the RGB records are
  **not read**.

## Narrowed / corrected from the worker

- The worker states no `MM_CLIMB_002.LVL` was available. It is at
  `build/lvl_dump/levels/COMPILED/MM_CLIMB_002.LVL` (699117 bytes); the brief named it.
  The fluid formula's `level+0x50/+0x98` were not mapped to file data either way.
- The worker leaves "walk/block use `0x404372`" as unsettled. The type-`0x23` = fluid
  identification settles it negative for these fixtures.
- The worker's `[draw+0x2c]` propagation inference is moot: the 152 is already on
  `actor+0x128`.

## Facilitator check (2026-10-19)

Re-read `0x404372..0x40445F`, `0x40E930..0x40E9F9`, `0x43CE00..0x43CE38`,
`0x4682A6..0x4682D7` and the capture script's record offsets. Static round: no
process, no live writes, no code or test changes, so no suite rerun was needed.

## A4h follow-up: `0x4682a6` is a clean negative

Worker note `build/tmp/a4h-controller-copy/round-note.md`. The facilitator re-read the
bytes. `0x455ba0` copies `0x6C` dwords (`0x1B0` bytes, all four vertex records) **from
`this` to the argument**: `0x455BA9 8b75fc` esi←this, `0x455BB1 8b7d08` edi←arg,
`0x455BAC b96c000000`, `0x455BB4 f3a5`. At `0x40E993`, `this = actor+0x128`, so the
actor's controller is the *source*. Eight prototypes (loop `0x40E924 cmp [ebp-0x10],8;
jge`) become copies of the actor's controller and are then re-tinted to
`0, 16, …, 112`. The worker reports `0x40E993` as the only direct caller of `0x4682a6`,
and `0x46836d` touches only `+0x1F8/+0x1FC`. Neither has been re-read independently.
Nothing in this path writes `actor+0x128` RGB, so it cannot produce 152.

Static leads around `0x406260` are exhausted. A per-record writer that bypasses
`0x406260`, or a bulk copy *into* `actor+0x128` from elsewhere, remains possible. The
A4g broad scan's 472 unfiltered hits were not triaged.

## A4i: live write watch — 152 is not rewritten per frame (MEASURED)

Worker note `build/tmp/a4i-tint-write-watch/round-note.md`; the retry is under `retry/`.
- Attempt 1 was inconclusive: the game was not shown to be running, and the `ntdll
  0x77c0a4c1` SIGTRAP was GDB's interrupt thread, not a crash. It is retained.
- Retry: walker `0x2763720` on `Compiled\MM_CLIMB_002.lvl`, `controller+0x40` =
  `0x2763888` = 152.0. `-break-list` shows a `hw watchpoint`. Over 10.04 s with gameplay
  provably running (actor list 4 → 8; `actor+0x108` moved `(-53.99, 32, 19.08)` →
  `(-30.91, 32, 42.46)`), **zero writes**. The colour is set once, not per frame.
- Phase 2 (`0x440A10` hardware breakpoint): no hit in two 30 s windows. Those windows
  opened only *after* the four new spawns had already happened during phase 1, so they
  are not a negative on the constructor.
- Cleanup (facilitator-checked): empty break list before each detach, process alive at
  detach and stopped afterwards, install hashes match; no stock or gdb process remains.

Facilitator static follow-ups, re-read 2026-10-19:
- The controller constructor `0x4680f0` calls `0x4409E0` (`0x468113`), which calls
  `0x440A10` (`0x4409EA`). The constructor path is valid.
- `0x455ba0` has exactly two `E8` callers: `0x4682B4` (A4h) and `0x454AE2`. At `0x454AE2`,
  `this = [obj+0x16D8]+0x128` copies **out** to `obj+0x16E0`, then tints the copy with the
  BSS global double `0x4C6988`. Its only writer, `0x454A6D..7D`, stores the constant
  255.0 (`0x4B1730`). Neither path writes the actor.
- `0x4409E0` has eight direct callers (`0x44C8E6/F8/90A`, `0x452CD1`, `0x45A584`,
  `0x468113`, `0x49046F`, `0x491435`); `0x4680f0` has four (`0x43CE34`, `0x4576BD`,
  `0x4576FE`, `0x47B356`).

## A4j: spawn watch failed; A4i "not rewritten" retracted to inconclusive

Worker note `build/tmp/a4j-spawn-watch/round-note.md` (`run2/`, `run3/`). With a hardware
breakpoint on `0x4409E0` armed before unpause, the first new `ecx` (`0x2793E44`) received
a confirmed `hw watchpoint` on `+0x40` (255.0). **Zero watch triggers**, yet the worker's
memory poll saw 255.0 → 0.0. No actor ever had `+0x128 == ecx`, the actor list stayed
2 → 2, and gameplay did not advance. `0x4409E0` has eight callers, so that `ecx` was very
likely not a lemming controller. The worker used more than the two-game-start cap; this
is recorded as a brief violation. Cleanup was fine: empty break list, hashes match, no
process left.

**Retraction (facilitator, same day).** A4i's "zero writes in 10 s, so the colour is set
once" depended only on watch hits. A4j shows this GDB/Windows hardware watch can miss a
real write, perhaps because debug registers are not applied to every thread. A4i also
never re-read the value after the window. That claim is therefore **inconclusive**, not
measured. The gameplay-advancement proof stands. Do not reuse GDB hardware watches here
until one is shown to trigger on a known write.

## A4k: Frida hooks — fresh walkers go 255 → 152 after construction (MEASURED)

Worker note `build/tmp/a4k-frida-tint/round-note.md`; one game start, REA Frida, no GDB.
- **Measured:** four controllers constructed during the hooked window via the spawn chain
  `0x4409E0` (`ra = 0x468118`, i.e. `0x4680f0`'s call at `0x468113`) — `0x28055E8`,
  `0x2806D78`, `0x28065E8`, `0x2807D78` — are walk (action 13) lemming controllers that
  later read 152.0 at `+0x40`. Since `0x440A10` writes 255.0, the 152 is written **after
  construction**. Walker `draw_position`s differ between the two snapshots, so gameplay
  advanced.
- **Not measured, because of script bugs (facilitator diagnosis, not the worker's):**
  `0x406260` R/G/B logged `null` because the script used the removed legacy
  `Memory.readDouble`/`Memory.readPointer` API inside a `try/catch`. The worker's
  `esp+4` vs `[ebp+8]` explanation is wrong; at entry they are the same slot. The
  `0x455ba0` hook in the monitor script threw on the same API. The `MemoryAccessMonitor`
  saw zero hits because it is one-shot per page and the callback re-armed only after a
  matching write, so the first (read) access spent it. The main run's 5000-event cap was
  filled by `0x4409E0` calls from `0x49C611`, an array-constructor loop.
- The worker's "hooked bytes restored" check used `read_steppers.py`, which reads the
  **file**, not process memory, so it proves nothing. That is moot here because the
  process ended and install hashes match, but it must be read from live memory next time.
- The worker's advancement reference (`ref_snapshot`) decodes as garbage and is
  unchanged. Only the walker positions prove advancement.

## A4l: fixed Frida rerun — 255 → 152 within 16 ms of construction (MEASURED)

Worker note `build/tmp/a4l-frida-tint-fixed/round-note.md`; one game start.
- Positive control works: 21 spawn-chain `0x4409E0` calls (`ra 0x468118`), each reading
  `ecx+0x40 = 255.0` at `onLeave`.
- A 1 ms poll caught four walk controllers (`0x2733848`, `0x2734D78`, `0x27345E8`,
  `0x2735D78`) turning **255 → 152 within 1–16 ms** of their `onLeave`
  (`frida-events.json` seq 327–337, facilitator-read). At the end all four are 152.0, and
  their walkers' `actor+0x108` moved.
- **No** `0x406260` or `0x455ba0` call touched those controllers, so the 152 comes from
  another path in the spawn flow. The page monitor produced no write `from` address
  (guard-page conflicts with the poll); it is not evidence.
- The facilitator read process-memory hook bytes after unload (`hook-process-bytes.json`):
  they match the file at `0x4409E0`, `0x406260` and `0x455BA0`. Install hashes match.
- Static: `0x43CE00` (the draw object whose `+0xB0` is the controller) has four callers,
  `0x40E94E`, `0x42DD4E`, `0x42DEC7` and `0x465992`. `0x42DEC7` is in the actor
  constructor that A4f traced.

## A4m: stack bisect hooked the wrong stack — inconclusive, not negative

Worker note `build/tmp/a4m-tint-bisect/round-note.md`; one game start. Hooks were restored:
15/15 image sites match the file in process memory, and install hashes match.
- MEASURED: 21 `0x4409E0` leaves read 255.0. 166 frame leaves read 255.0, and none read
  152. At the end the four walkers are 152.0 and have advanced.
- The worker concluded that "152 is written after the spawn stack fully returns". The
  facilitator **does not accept** that. Frame hooks were installed from the *first*
  backtrace only (seq 3), and that one is one of 17 level-load constructions through
  `ra 0x47B35B` (`0x47B320` ← `0x46BCA4` ← … ← `0x437E02`). The four walker controllers
  (`0x2793848`, `0x2794D78`, `0x27945E8`, `0x2795D78`) are exactly the seq 232/236/240/244
  backtraces, about 2.9 s apart, and they came through a **different** stack:
  `0x468118` ← `0x4409E0` ← `0x43CE39` ← `0x42DECC` ← `0x425C35` ← `0x442D44` ←
  `0x45DCD9` ← `0x450A58`. None of those frames was hooked. Facilitator-read from
  `frida-events.json`.
- Static (facilitator): the lemming constructor calls `0x46836D(ecx = actor+0x128,
  [0x4E335C + arg5*4])` at `0x42DF89`. `0x46836D` (`0x46836D..0x4683C2`) only stores
  `+0x1FC` (resource id) and `+0x1F8` (handle from `0x42FF90`), so it is not the writer.
  A `.text` scan for stores at actor-relative `+0x168/+0x170/+0x178/+0x1D0..` found nothing
  convincing; `0x427DDD/0x427DF1` are a stream deserialiser.
- Approximate function starts from a prologue scan, which the next round must confirm:
  `0x42DECC` in ~`0x42DE51`, `0x425C35` in ~`0x425A95`, `0x442D44` in ~`0x4423D2`,
  `0x45DCD9` in ~`0x45CFF9`.

Next: A4n repeats the bisect on the **walker** stack.

## A4n: 152 is written by the lemming's first tick, inside the spawner (MEASURED bracket)

Worker note `build/tmp/a4n-walker-stack-bisect/round-note.md`. Two game starts (the first
failed to enter the level). 12/12 image hook sites match the file after unload, install
hashes match, and no process is left. Facilitator-read from `frida-events.json`:
- Frame hooks installed at attach on `0x43CE00`, `0x42DE51`, `0x425A95`, `0x4423D2` and
  `0x45CFF9`, with walker controllers filtered by `0x43CE39` in the backtrace.
- For walkers 2, 3 and 4 (`0x2714D78`, `0x27145E8`, `0x2715D78`) the order is identical:
  `0x4409E0` leaves at 255, then `0x43CE00` leaves at 255, then the lemming constructor
  **`0x42DE51` leaves at 255** (seq 873 / 1722 / 2571). The next event, the enter of
  `0x426180` (call site `0x425CE1` in `0x425A95`), already reads **152** (seq 874 / 1723 /
  2572). The 255 `0x444ABC` events between them are nested inside the constructor: the hook
  is on the target, not the call site. Walker 1 (`0x2713758`) had only frame hooks armed and
  agrees: the poll flipped 4 ms after the constructor, before `0x425A95` returned.
- `0x425A95` returns every ~7 ms (1916 leaves), so it is a per-tick hatch/spawner routine,
  not a one-shot spawner.
- READ, `0x425C35..0x425CE1`, the window after the constructor returns:
  - `call 0x42F5E3` if `[hatch+0xAD]`, and `call 0x42F66B` if `word [hatch+0xB4] == -1`.
    Both were hooked, and neither logged for walkers 2–4 inside the window. The `0x42F66B`
    hook did fire later (seq 2056), so it works.
  - a store to `[hatch+0xB4]`.
  - **`call [vtable+0x14]` at `0x425CCC`** on the new lemming, with the double argument
    `−[hatch+0xC0]`. This call was not hooked.
  So the 152 is written inside that virtual call or nowhere in this window.
- READ: the lemming constructor stores vtable `0x4B10E8` at `0x42DF1D`. `.rdata` slot
  `+0x14` = **`0x42E2BF`**, the lemming update routine (the `dt` argument is compared with
  0.0, then it dispatches through action object `[actor+0x340]` vtable `+4`). This target
  identity is **read only**: a derived class could replace the vtable after `0x42DF1D`.
- **The worker's "writer function `0x425A95`" is too coarse.** The flip happens in the
  first `0x42E2BF` tick, called from the spawner, which is consistent with A4l's 1–16 ms.

Next: A4o traces that one call at the instruction level.

## A4o: the writer is `0x403820`, fed by a position-indexed colour map (MEASURED + READ)

Worker note `build/tmp/a4o-first-tick-trace/round-note.md`. One game start (attached twice).
4/4 hook sites match the file after unload, install hashes match, no process left. The
Stalker trace crashed and was replaced by a subcall bisect, which is enough here because
the writer is a pure copy.
- MEASURED (`frida-events.json` seq 6–10, facilitator-read): on walker `0x2763848`'s first
  update, the call target at `0x425CCC` is `[[actor]+0x14]` = **`0x42E2BF`** (vtable
  `0x4B10E8`), with `dt = 0.0069988`. On entry `ctrl+0x40/+0x48/+0x50` = 255/255/255.
  The subcall `0x403820` enters at 255 and **leaves at 152/152/152**, and later updates
  stay at 152. The worker's `record1_a8: 0` is not `ctrl+0xA8` (that should read 152 after
  the copy); treat it as a mislabelled read.
- READ, the writer: **`0x403820(this = ctrl, src)`** (`0x403820..0x403889`, `ret 4`)
  performs four `rep movsd` of 6 dwords (three doubles, RGB). It copies `src →
  ctrl+0x178`, then `+0x178 → +0x110`, `+0x110 → +0xA8` and `+0xA8 → +0x40`, so all four
  vertex records (`+0x38` in records at `+0x140/+0xD8/+0x70/+0x08`) get the same RGB.
  It stores nothing else.
- READ, the call site in `0x42E2BF`, `0x42E62F..0x42E661`:
  `src = 0x403F40(ecx = 0x4C6A10, a = ftol([actor+0x08]), b = ftol([actor+0x10]))`, then
  `0x403820(ecx = actor+0x128, src)`. `0x49C5AC` is called on each double (assumed to be
  the CRT `_ftol`; not re-read).
- READ, the lookup `0x403F40` → `0x403FC0` → `0x403FA0`:
  `idx = (b << 8) / 2 + (a & 0xFF) / 2` (C signed division, `cdq; sub; sar 1`), and
  `byte = *([0x4C6A10 + 0x664 + 0x0C] + idx)`. The return value is
  `0x4C6A10 + 0x678 + 0x18·byte`, i.e. a **palette of RGB-double triples indexed by a
  byte map**. In other words, the vertex colour is a per-position light/colour map value.
  The 152 is that map's value at the walker's position on `MM_CLIMB_002`.
- **Falsified:** the facilitator's A4o prediction (a per-action/animation table).
- Unsettled: which world axes `actor+0x08` and `actor+0x10` are; who fills the byte map and
  palette (level loader?); and whether the update calls `0x403820` every tick or only on a
  condition (`0x42E62D` is a `je 0x42E666` around it).
- `0x4C6A10` appears in 562 instruction operands (with `0x4C6A08`), so it is a central
  level object.

## A4p: the map and palette are stored LVL data; the lookup reproduces the live tint (MEASURED)

Worker note `build/tmp/a4p-colour-map/round-note.md`. One game start, read-only (no hooks,
no writes), install hashes match, no process left. The facilitator re-checked the dumps:
- **Stored, not computed.** In `MM_CLIMB_002.LVL` the 12,288-byte map is at file offset
  **338319**: the collision grid ends at 240011 + 98304 = 338315, then `u32 B = 12288`.
  The **6,144-byte palette follows immediately at 350607**, with no length prefix. The live
  BSS copies (`live-map.bin`, `live-palette.bin`) are byte-identical to both. This is the
  `u32 B, u8[B]` block already described in `src/assets/CollisionGrid.h`, plus the palette
  after it.
- **The palette is 256 entries × 24 bytes** (3 × f64). The worker's "512 × 24" is wrong:
  `0x1800 = 6144 = 256·24`. On `MM_CLIMB_002` only entries 0–12 are non-zero, and they form
  a grey ramp with R = G = B: 152, 147, 140, 131, 123, 116, 107, 98, 92, 82, 74, 68, 61.
  The map uses bytes 0–12 only. Byte 0 covers 9,381 of 12,288 cells (76%), and byte 12
  covers 641.
- READ (worker, not re-read by the facilitator): `FUN_00439FD3` (called from `0x443A0A` in
  `0x4432C1`) loads the map via `0x43B700` to `+0x664` and the palette via `0x43B5B0`
  (`0x1800` bytes) to `+0x678`. `0x49C5AC` is `_ftol` (`558bec83c4f49bd97dfe…`, a
  control-word truncation). `actor+0x08` is world X and `+0x10` world Y (up), per
  `measure_steppers.py`. In `0x42E2BF` the copy is guarded by a `_strcmp` (`0x49C870`)
  whose result decides `je 0x42E666`; the compared strings are unread. The colour is
  re-applied on every update that passes the guard, so it **follows the lemming's
  position**.
- MEASURED: 20 samples (10 each for 2 lemmings; the other two walkers were not sampled in
  the window). `palette[map[idx]]` equals `ctrl+0x40/+0x48/+0x50` in all 20, with
  `idx = y·128 + (x & 0xFF)/2` (for example x = 32.65, y = 32 gives idx 4112).
- **Weakness:** every sample sat on map byte 0. The test therefore shows agreement but not
  a colour *change* across cells. The prediction for the hatch cell (32, 42) was written
  before the run and gave byte 0 → 152, but byte 0 is the majority value.
- So the "152" of A4f is this level's palette entry 0 at the walkers' cells, and the vertex
  colour is a per-level, per-cell **grey light map** applied to lemmings.

## A4q: production light map; the A4f pixel gate passes (facilitator-verified)

Worker note `build/tmp/a4q-actor-light-map/round-note.md`. Code round, no live game.
- Code:
  - `src/assets/LightMap.{h,cpp}` (registered in `cmake/webolution_sources.cmake`) parses
    the map and palette after the collision grid. It also does the exact lookup
    (`trunc`, then `(b << 8)/2 + (a & 0xFF)/2`, bounds-checked) and
    `modulateActorSpritePixel` (`tex·tint/255`, alpha kept; ASSUMED, the blend state at
    `0x414D80` is unread).
  - `drawActorSprite` now takes `vertexRgb`.
  - `actorfixture` and `verify_actor_sprite_a4f.py` look up the tint from
    `COMPILED\MM_CLIMB_002.LVL`; `--no-light-lookup` gives the old path.
- Facilitator checks:
  - **All 102 levels:** `B == 12288`, and every palette double is a whole number in
    [0, 255], so the `uint8` storage is lossless. The highest map byte used is 24.
  - **CTest 20/20, after a forced rebuild.** The first facilitator run failed
    `actorsprite`. The worker had restored mutated sources with their *old* mtimes, so ninja
    kept the mutated objects. The sources were hash-identical to the worker's `.bak`
    copies, so the code was right and the build state stale. After touching them: 20/20.
  - **Own reverts** (`facilitator_mutations.ps1` → `facilitator-mutations.txt`), each
    restored with a fresh mtime:
    - no `& 0xFF` → `actorsprite` and `collisiongrid` fail;
    - `floor` for X → both fail (the A4f index 4196 becomes 4195);
    - no modulate in the draw → `actorsprite` fails ("draw path applies MODULATE").
    - Restored: pass.
  - **Gate rerun** (`facilitator-gate/`, `facilitator-gate-old/`):

    | Fixture | candidate MAE, tinted | absent-actor MAE | candidate MAE, `--no-light-lookup` |
    | --- | --- | --- | --- |
    | five | 0.0032284 | 0.0061900 | 0.0079708 |
    | eight | 0.0036200 | 0.0068484 | 0.0093424 |

    The worker's numbers are reproduced exactly; both pass with the tint, both fail without.
  - Tube views: the new code has no caller in the tube/app render path (only `actorfixture`
    and the tests), so the nine views are unaffected by construction. The worker re-scored
    all nine; only `initial`'s hash was compared against a recorded value.
- Still open:
  - the actor-specific alpha/blend state; the later
    [A4r static check](phase-a4-actor-render-state.md) supports RGB modulation and
    reads conditional alpha/blend assignments, but leaves actor flags and selector
    unresolved;
  - a live colour *change* across map cells;
  - the `_strcmp` guard strings at `0x42E622`;
  - the ~0.08 sprite-pixel residual;
  - drawing actors in the app at all.

## Superseded next step (done in A4h)

Read `0x4682a6` and its callees (`0x455ba0`, `0x46836d`) to decide whether it copies the
prototype's vertex records into `actor+0x128`. Then find which prototype index or other
writer a walk/block actor on `MM_CLIMB_002` receives. If static reading stalls, the
cheapest live check is a polled read of `actor+0x128+0x40` from spawn, without a debugger.
