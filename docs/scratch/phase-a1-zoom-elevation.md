# A1 zoom/elevation branches: all reachable cases pass

This follows the [control mapping](phase-a1-control-mapping.md). The fixed criterion is
unchanged: 640x480, ROI `[100,80,280,410]`, normalized mean absolute RGB error `<= 0.02`.

## Verdict

All four reachable zoom/elevation branches pass, and the static scaling read is confirmed
live. **A1's camera/projection deliverable is complete** for the tested level and branches.

| Case | Input | Zoom | Elev `+0x18` | ROI MAE | Pass |
| --- | --- | ---: | ---: | ---: | --- |
| canonical | none | 0 | 0.0 | 0.015577 | yes |
| zoom1 | SPACE | 1 | 18.5 | 0.011949 | yes |
| elev_up | SPACE + grab + dy=+48 | 1 | 47.5 | 0.011646 | yes |
| elev_down | SPACE + grab + dy=-48 | 1 | -10.5 | 0.012861 | yes |
| elev0_neg | dy=+48 at zoom 0 | 0 | 0.0 | 0.015860 | yes |

The facilitator independently recomputed all five ROI errors from the original/candidate
PNGs; they match the report exactly.

## Static scaling (verified bytes)

- Local eye vector: `(0, (worldRows + 17.392) / 2^(zoom&31), -377.516 / 2^(zoom&31))`.
  `0x45CC61` loads `control+0x10`; `0x45CC64`/`0x45CCB4` are `shl edx,cl` (x86 masks by
  `0x1f`); `0x45CC72`/`0x45CCC2` are `fdivp`.
- Elevation is added to eye **y** only when `zoom != 0` (`0x45CF30` `cmp [edx+0x10],0`;
  `0x45CF34` `je`; `0x45CF48` `fadd [ecx+0x18]`). At zoom 0 it is not applied.
- Fixed pitch `-0.03135770272046` at `0x45CF83/88`.
- Lens builder `0x47B061` never reads `control+0x10`; zoom does not change the projection.
- Elevation sources: the zoom transition sets `control+0x18 = 47.5 - 58*((cursorY-130)/(H-260))`
  clamped to `[-10.5, 47.5]`; grabbed mouse at zoom 1 adds `+1.5*dy` to `+0x18` and
  `-0.125*dy` to the displayed elevation `+0x60`.

## Isolation controls

- Canonical re-render is byte-identical.
- Canonical candidate vs the zoom-1 original: **0.075635** (fails); the zoom-1 pass is not
  trivial.
- Zoom-1 candidate vs the `elev_up` original: **0.049419** (fails); vs its own: **0.011949**
  (passes). The elevation delta is carried by `control+0x18` alone.
- `elev_up` candidate vs the zoom-1 original: **0.046127** (fails).
- Lens is invariant across all five captures: `camera[0x118] = 0.052861109375953674`,
  aspect `1.3333333730697632`, center `(319,239)`, projection `[2.237313, 2.237313,
  0.001563]`.

## Correction

The first capture set was contaminated by the level-start **"Let's Go!"** banner inside the
ROI. It is not lemmings; a 4x1 s play left the ROI pixel-identical. All reported captures are
post-banner, and the canonical original then matches the retained `original-0.png` at
`0.000003`.

## Residual unknowns

- The gameplay **write site of `control+0x10`** remains unlocated; the 0<->1 toggle and its
  one-frame lag behind `+0x14` are measured, not read.
- The wrapper's internal Z bits are not readable (depth/clipping note).
- The camera mapping is verified on `MM_CLIMB_002` and the retained views; no whole-scene or
  all-level claim.
- The entry-point debt (A4) is outside A1.

## Reproduction

```text
python build/tmp/a1-zoom-elevation/read_zoom_elevation.py
python build/tmp/a1-zoom-elevation/score_states.py
```

Artifacts: `build/tmp/a1-zoom-elevation/round-note.md`, `live-captures.json`,
`params-*.json`, `state-pixel-report.json`, `final-live-state.json`, `restore.json`, and the
original/candidate PNGs.

**A1 is complete for the tested scope.** Next: A2 the tube — INN/OUT addressing, draw order,
inner-before-outer, and skipped scenery. The entry-point debt remains A4.
