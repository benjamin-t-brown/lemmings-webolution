# Phase D1c: hatch door-opening duration (live measurement)

Queue id `d1c-door-open-duration`. Round 3 of the D1 gate question, following
D1a (`docs/scratch/phase-d1-release-walkout-trace.md`) and D1b
(`docs/scratch/phase-d1-release-trigger.md`). Landed 2026-10-21.

## What was measured

One live, read-only capture of `MM_CLIMB_002`'s hatch object (located by its
own vtable `0x004B0F50` via a heap scan, `measure_lever.find_objects`,
verified against every one of D1b's static field reads — `count=10 x=32
y=42 rate=20 min=1 max=50 type=0` — before trusting it), sampling
`hatch+0xAC` (armed) and `hatch+0xC0` (countdown) at 100 Hz for 4 s from the
same `t=0` anchor D1a used.

**The literal `0`→`1` arming flip was missed**: locating the object (a
full-process heap scan) cost ~0.90 s of real time before the first sample,
and every sample from `t = 0.8956 s` onward already reads `armed=1`. This is
reported as a genuine gap against the brief's literal wording, not hidden.

**What the capture measured directly instead:**

1. The countdown's rollover (the release firing) lands in `(2.020 s,
   2.030 s]` — at or within one 10 ms sample of D1a's independently measured
   first release, `t = 2.0200 s`. This is a *second*, independent live
   confirmation of that number, from a different field (`hatch+0xC0`, not
   the global `RELEASED` counter) in a separate capture.
2. A linear fit of the 203 pre-rollover `hatch+0xC0` samples: slope
   `-0.997`, i.e. the countdown falls at very nearly `1.0` unit per real
   second (no doubling/halving). Solving the fit for `countdown == 1.3`
   (D1b's static seed) gives a **derived** `arm_time ≈ 0.723 s`.
   `arm_time + 1.3 s ≈ 2.023 s`, against D1a's `2.020 s` — agreement to
   `~3 ms`, inside the fit's own `0.0058 s` residual band.

**So: `arm_time ≈ 0.723 s`, derived by back-extrapolation of a directly
measured, tightly linear trajectory — not a directly sampled flip.** Whether
`0.723 s` is exactly `12 × 0.06 s` (the project's established animation-frame
convention) is inside tolerance but not discriminated; left open.

## Facilitator check (2026-10-21)

- **Reproduced the linear fit independently**, with fresh least-squares code
  reading only the saved `hatch_raw.jsonl` (not the worker's own
  `analyze_arm_time.py`): slope `-0.997163094969382`, intercept
  `2.0210104385757433`, max residual `0.005843593747880016`, derived
  `arm_time = 0.723061695938398`, `arm_time + 1.3 = 2.023061695938398` — all
  match the round's reported figures exactly.
- **Independently re-read the raw rollover rows**: `t=2.02 → cd=0.010002`,
  `t=2.03 → cd=2.881489` (the jump), matching the round's quote exactly.
- **Confirmed the hatch's identity**: `hatch_raw.meta.json`'s field snapshot
  (`count=10 spawn_x=32 spawn_y=42 rate=20 min=1 max=50 type=0`) matches, byte
  for byte, D1b's own independently-reread `.LVL` hatch record (verified
  separately in that round's facilitator check) — a live/static cross-check,
  not just an internal consistency claim. The post-capture snapshot shows
  `count=9` (one release occurred), consistent with D1b's decrement read.
- **Live-game state, independently checked**: no `Lemmings Revolution`
  process, installed exe hash matches the authority
  (`32675BDB...737A2F`). `git status --short` shows only
  `docs/tasks.json`'s permitted fields changed.
- **An additional observation, not in the worker's own report**: consecutive
  10 ms samples show the countdown changing by small, *uneven* increments
  (alternating roughly `-0.007`/`-0.014` rather than one clean `-0.2` jump
  every 20th sample) — i.e. `hatch+0xC0` is updated far more often than once
  per 200 ms sim tick, at something close to a ~144 Hz render-frame rate,
  the same figure `measure_release.py`'s own docstring already cites for
  this machine's foregrounded frame rate. This is consistent with, not a
  contradiction of, the project's already-established picture (`docs/LESSONS.md`,
  "The simulation") that lemming *position* is rendered continuously between
  200 ms logic ticks — but it has not been established here whether the
  hatch's actual fire decision (the `fcomp` against `0.0` in the same
  function call that does the subtraction) is itself tick-gated or
  genuinely continuous. **Not settled; flagged as a lead for a future round
  if an exact-tick implementation needs it** — this round's own claims
  (the rollover timing, the derived `arm_time`) do not depend on resolving
  it, since both are measured/derived from the field's own value, not from
  an assumption about its update cadence.

Accepted as a landed finding with the "measured vs. derived vs. unsettled"
labels the round itself already used. D1's dominant mechanism (D1a tick-1
divergence → D1b per-hatch countdown trigger → D1c's `~0.723 s` arm delay)
is now numerically closed to within a few milliseconds of D1a's own
measurement. The door-animation's own internal mechanism (`hatch+0x44`) and
the tick-vs-frame clock question above remain open, parked leads — not
blocking an implementation attempt.
