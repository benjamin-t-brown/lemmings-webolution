# C3s — explicit reference selection in the full-frame scorer

2026-10-25. Offline harness repair; no original-game read/start or render change.
Packet: `build/tmp/c3s-explicit-reference-scorer/round-note.md`.

`score_c3_regions.py` now accepts `--reference PNG`, reports the selected path
and SHA-256 in console/JSON, and supports external paths. Historical A6d3 remains
the explicitly labeled legacy default. A missing candidate fails with exit2;
absent control remains optional. Metric, denominator235008000, threshold0.02,
gate numerator4700160, region rectangles and ownership partition are unchanged.
Floor ring stays included in full frame and diagnostically UNBOUNDED/in rest.
Existing actor boxes remain A6d3-derived diagnostics, not repair4 actor identities.

For the current directive, use an explicit reference, for example:

```text
python tools/harness/score_c3_regions.py --reference build/tmp/a6f-fresh-pause-gate/repair4/original-live.png --candidate build/tmp/c3-facilitator-baseline/current-repair4.png --json build/tmp/c3s-facilitator-check/current-cli.json
```

This returns current baseline **48188934/235008000 = 0.2050523131127451**,
still failing0.02. It is not a new scene render. Repair4 reference SHA-256 is
`39ba74328224a44ab1ae392790dd191303eb399492162d164c104a577fb99053`.
The default candidate/control remain historical diagnostics; explicit reference
selection does not magically match a candidate's camera/actor state.

## Facilitator check

Read the complete diff and regression test. Changes are restricted to CLI input,
identity reporting, path labels and missing-candidate handling; score/region math
was not edited. Original-byte checks do not apply to this source-only repair.

`python -B build/tmp/c3s-facilitator-check.py` invokes the actual CLI and separately
computes Pillow difference histograms using independently listed rectangles,
without importing the scorer or worker tests. A synthetic reference changes one
pixel in each of six regions; expected full numerators **0/534** and every region
sum/pixel count match. Same old C3a candidate against repair4 versus A6d3 yields
**49336286/49297573**; the current-repair4 candidate yields **48188934**.
These different numbers arise from named candidate/reference pairs, not revisions
to the metric. All selected reference hashes and exact totals match.

Isolated regression: temporarily changed only `ref = load(args.reference)` back
to `ref = load(REF)`. Two of the three permanent tests fail: synthetic scores
become80897438/80897438 instead of0/255, and real-reference scores collapse to
49297573/49297573. Restored the line, checked exact scorer SHA-256
`aea29cf74aca9d9fbc3c3637c1fbbc976d2c0f81dbb19907f357960780fc45a3`, and reran
both independent checker and **3/3** permanent tests successfully. Negative log:
`build/tmp/c3s-facilitator-check/isolated-bypass.log`.

Fresh wrapper build and **34/34 CTest passed**. No Lemmings process; zero live
resources/writes. C3q/C3r still own only disjoint offline scratch. No render,
pixel fidelity, runtime device-state or new original measurement is accepted.

Dispatch window14:16:53..14:17:34; worker finished14:20:43; accepted14:24:12 MDT.
Dispatch-to-acceptance **7m19s** from recorded dispatch start (6m38s from confirmed
dispatch). Next values remain scene device state/texture upload, then integration.
