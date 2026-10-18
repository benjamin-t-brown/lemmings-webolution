# Phase 3, round 2 — walk-off fall step

2026-10-14. Question: does a walk-off the y ≈ 48 ring step 4.000 world
units per 200 ms tick, and which fall state/vtable does it enter?

`Rules` was **not** changed. The live series agrees with the static read
`fallSpeed = 4` (20.0 at `0x004B0A18` × 0.2 s). `floatAfter = 10` and
`splatDistance = 30` were not exercised by this short drop (16 world units
ledge to floor) and stay as the binary read.

Script: `tools/harness/measure_dropoff.py` (brought in and tightened for
ledge lock / fall continuity). Height is billboard centroid Y
(`measure_motion`); one cell = one world unit. Do not multiply by
`PIXELS_PER_UNIT`.

Stock exe `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`.

## Observation — climb then walk-off on MM_CLIMB_002

Assigned climber via panel index `0x004C9200` (glide + click; writing
`0x004C36A0` is dead in play — see script docstring). Climber rose on
column 128 with the already-measured +1.000 / 200 ms step, hoisted past
47 to **48.190**, then left the ledge.

### Fall series (world Y)

Re-run, pid 22132, path `Compiled\MM_CLIMB_002.lvl`. Follow seeded on the
climber at `0x026a50d8`. Billboard records recycled during the fall; gaps
between samples are tracker reacquire lag, **not** multi-tick physics holds.
Every observed downward step is exactly 4.000 world units:

| t (ms) | y | column | record | ΔY from prior |
| --- | --- | --- | --- | --- |
| 0.0 | 34.190 | 128.00 | 0x026a50d8 | (climb start) |
| … | … | 128.00 | 0x026a50d8 | +1.000 / ~200–275 ms (climb) |
| 4575.0 | 48.190 | 128.00 | 0x026a50d8 | hoist / top-out |
| 6778.3 | 44.190 | 126.94 | 0x026a6928 | **−4.000** |
| 17654.7 | 40.190 | 128.00 | 0x026a60d8 | **−4.000** (reacquired) |
| 18879.7 | 36.190 | 128.00 | 0x026a4868 | **−4.000** |

Climb excerpt (same run, same record) held column 128 and stepped
34.190 → 46.190 at ~200–275 ms, then 48.190 at 4575 ms (hoist skipped 47,
as before).

Earlier partial run (pid 12916) also saw **48.190 → 44.190** with ΔY =
4.000 before the tracker jumped; that matches this series’ first fall step.

### Measured step

- **ΔY per fall step: 4.000 world units** (three consecutive observed
  edges: 48.190→44.190→40.190→36.190).
- Predicted: 4.0 from `20.0 * 0.2`. **Agrees.** Leave `Rules::fallSpeed`.
- Polling was ~40 Hz in climb/ledge and intended 80 Hz in fall; reacquire
  rescans dropped effective rate, so tick **gaps** in the table are not the
  physics period. The climb on the same run still shows ~200 ms. Do not
  infer fallSpeed from gap/ΔY when the gap is seconds of missed samples.

The old 48.190→32.190 snap with no intermediates is still not this fall
(would be four steps of 4.0 over ~0.8 s). This run shows the intermediates.

Hatch releases at y = 32.190 were not used.

## Vtable / state

**Not confirmed live.** Static read unchanged: ordinary faller vtable
`0x004B0A20`, speed `0x004B0A18` = 20.0, umbrella `0x004B03E8` = 10.0,
splat `0x004B10A8` = 30.0 (verified still those doubles in the live image
this session). A heap scan for dword `0x004B0A20` during play did not yield
a usable state object with a moving `+0x18` fall accumulator on this pass;
image xrefs at `0x00415fb4` / `0x00415ff4` are not heap states.

Hypothesis, not promoted: the walk-off enters that same vtable. The live
step size matches its speed constant; the enter transition was not read.

Sibling 25.0 at `0x004B0C60` was not used.

Umbrella / splat: this drop is 16 world units (48→32). That is past
`floatAfter` 10 if a floater were assigned, and under `splatDistance` 30.
No floater was assigned; landing / splat was not captured in the series
(tracker stopped around y = 36.190).

## Harness notes (observation)

- `goto_level.focus()` crashed the process via dgVoodoo on this machine
  after a cold attach; enter with shim only (no focus) still polled ~144 Hz.
- Panel select + glide-to-lemming works; hit-test statics do not.
- Prefer `.190` height fraction; reject `.000` scenery. After top-out, lock
  ledge phase so a second climber at y≈43 cannot steal the track as a fake
  drop.

## Rules

No edit. Live ΔY = 4.000 agrees with `fallSpeed = 4`. `floatAfter` /
`splatDistance` / `floatSpeed` unchanged (unread by this series beyond the
static constants still present in memory).

## Left the game

pid **22132**, `ui_state` **0**, path **`Compiled\MM_CLIMB_002.lvl`**,
clock was still counting (~208 s when last checked after the fall run).
Nothing further pressed for Round 2 close-out; a later heap probe was
killed without input to the game.
