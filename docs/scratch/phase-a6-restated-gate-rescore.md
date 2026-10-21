# A6e: restated A6 gate — clause 1 passes, clause 2 still fails on actor 2

2026-10-20. Offline rescore, no game operation, no source change. Claude Code
`round-agent` (session model, no pin). Brief
`build/tmp/a6e-restated-gate-rescore-brief.md`; packet
`build/tmp/a6e-restated-gate-rescore/round-note.md`.

## Result (MEASURED on retained PNGs)

Inputs are the [A6d24](phase-a6-full-frame-gate.md) PNGs, hashes unchanged:
reference `603f05f6…0e03`, actors-on `c4e6cf47…9ebf`, tube-only `66a8ea3b…3651`.

| Frame | Tube ROI `[100,80,280,410)` numerator / denominator | MAE | Clause 1 (≤ 0.02) |
| --- | ---: | ---: | --- |
| actors-on | 708,159 / 45,441,000 | 0.015584142074338152 | **PASS** |
| tube-only | 708,159 / 45,441,000 | 0.015584142074338152 | PASS |

- The two frames are pixel-identical inside the ROI; no frozen actor box enters it. On
  this camera clause 1 is a tube-regression check only. Actor fidelity rests on clause 2.
- Clause 2 is unchanged from A6d24: actors 0 and 1 improve; actor 2 worsens
  (15,283 → 17,035 of 151,470). **A6 is not accepted.**

## Facilitator check (2026-10-20)

Separate per-pixel integer summer, not `score_a4_render.py`:
`build/tmp/a6e-restated-gate-rescore/facilitator/fcheck.py`, output `fcheck.json`.
Reproduces both ROI numerators, ROI identity, all six A6d24 box numerators, all three
input hashes and the clause decisions with exact `Fraction` comparison.
