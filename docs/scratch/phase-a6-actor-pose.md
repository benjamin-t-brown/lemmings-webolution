# Phase A6a: actor `+0x08` to model position `+0x108` and facing basis

Authority: `C:\progs\Lemmings Revolution\Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
Brief `build/tmp/a6a-actor-pose-brief.md`; repairs `build/tmp/a6a-actor-pose-repair1.md`
and `-repair2.md`. Worker packets under `build/tmp/a6a-actor-pose/` (`round-note.md`,
`round-note-repair1.md`, `round-note-repair2.md`, `prediction.md`, `live/report.json`,
`native_samples.json`). Cursor `round-agent` on `composer-2.5-fast`, dispatch
`dc7a6e88-61d3-4aa9-a7ba-04ef8432b4d2`. Three rounds on this question (initial plus two
repairs), so the round cap is reached.

## Result

- **READ:** native `0x43D56C` (`ecx = 0x4C6A10`) maps actor `+0x08` `(X, Y, Z)` to the
  model position by bilinear interpolation of the level cell mesh. X is truncated by
  `0x49C5AC`, bucketed `cdq / and edx,7 / sar 3` and wrapped `& 0x1F` (32 columns of 8
  world units). Y is bucketed `and edx,0xF / sar 4` (16-unit rows), with the row clamped
  against `[this+0x28]`. Z blends shell layer 0 (inner) and layer 1 (outer).
- **READ:** when a `grid0` key is `-1` (`0x43A356 837dfcff`), `0x43A31D` allocates a
  pool vertex and fills it by `0x43D2B8` (`0x43A3AE e8052f0000`). That function
  interpolates the cylinder template with the same X/Y bucketing (`0x43D2CC..0x43D313`).
  `src/app/ActorPose.cpp` uses retail `CylinderMesh::vertices` for this. That the
  runtime `world+0x18` copy is identical to the template is an inference from A4b, not
  a byte comparison.
- **READ:** the basis producer `0x42EEF2` adds facing to the first component of the
  `+0x08` copy (`0x42EF38 db4040` fild `[+0x40]`, `0x42EF3B dc0d40104b00` fmul `0.1`
  from `.rdata 0x4B1040`). It then calls `0x43D56C` (`0x42EF6F`), subtracts the model
  position (`0x42EF76`) and normalizes.
- **MEASURED:** three walkers on `MM_CLIMB_002` (actors `0x2833720`, `0x2834c50`,
  `0x28344c0`, vtable `0x4B10E8`), one stock start. Model `+0x108` matches within 1e-4
  on all nine components; the worst is below 1e-6.
- **Falsifier not met for the basis:** the brief's 1e-6 basis tolerance fails on 5 of 9
  components. The maximum is 2.962e-5, on `0x2834c50` basis[2]. `test_actorpose`
  checks the basis against a labelled **5e-5 regression bound**, not a fidelity
  tolerance.
- **Unsettled:** `sim::Lemming` to `+0x08`. The integer sim X carries no sub-cell
  fraction (native X is fractional, e.g. 59.6448). The Y flip
  `height - 1 - sim.y` and the Z slab (0.75 for walkers on row 32, 0.5 for one on a
  slope) are not measured against a sim/native pair. The worker removed a test whose
  expectation derived from the formula under test.

## Facilitator check

2026-10-20, Cursor facilitator.

- **Re-read bytes:** `0x42EF30..0x42EF7B`, `.rdata 0x4B1040` (`9a99999999 99b93f` =
  0.1), `0x43D56C..0x43D5A8`, `0x43A31D..0x43A3B9`, `0x43D2B8..0x43D313`. All agree with
  the implementation.
- **Float32 inputs:** every native `+0x08`, `+0x108` and basis component in
  `live/report.json` is exactly representable as float32. One float32 ulp at magnitude
  ~50 is ~3.8e-6, and the basis divides a ~0.08-unit difference, so 1e-6 is below the
  inputs' resolution. A facilitator mutation that rounded only the final outputs to
  float32 did not close the residual (6 failures at 1e-6). The native precision chain
  is therefore unreplayed, not explained.
- **First packet rejected:**
  - the position match depended on an invented outer-shell scaling: disabling it alone
    failed 7 checks, including `0x28344c0` model X/Z;
  - the test had been loosened to 3e-5 against the brief's 1e-6;
  - the sim-Y test was tautological;
  - mutation restore hashes differed;
  - the live run swapped `dgVoodoo.conf`, which was on the must-not-touch list
    (restored; before/after hash `9a34b1b3...`).
- **Repair 1:** replaced the invented scaling with the `0x43D2B8` rule; accepted after
  the byte re-reads above.
- **Repair 2:** packaging only. Its worker reported reverting a leftover `facing*0.0`
  from its own mutation run. I therefore re-measured on the final source
  (`ActorPose.cpp` `2CD8B950...`):
  - at 1e-6, 5 basis failures;
  - at a 2.9e-5 bound, only `0x2834c50` basis[2] fails;
  - at 3.0e-5, none fail.
- **Isolated mutation:** dropping `facing * 0.1` alone fails 6 basis checks at the
  5e-5 bound. Restored hash-exact, touched, fresh full build: 21/21 CTest. The worker's
  `0x43D2B8`-disable mutation also fails (model and basis of `0x28344c0`), with an
  identical restore hash.
- **Live side:** no game process and no `LemWeb-*` task left. Install hashes (exe,
  `SavedGame.dat`, `dgVoodoo.conf`) equal the worker's before set. The `enter_fast`
  level-slot restore is logged `slot_restored: true`.
- **Scope:** `ActorPose.cpp` is compiled into `webolution` but has no caller in the app
  yet. Integration is A6d.

## Parked

The basis residual (max 2.962e-5) is parked in `PLAN.md` Phase E. The basis feeds only
the A3 sprite-view chooser, where it can matter only within ~3e-5 rad of a sector
boundary. Resume by replaying the native float32/x87 chain through `0x43D56C`,
`0x401830` and `0x480AC8`, and only if the A6 gate shows a view-index mismatch.
