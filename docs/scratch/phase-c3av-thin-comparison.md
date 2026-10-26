# C3av initial/repair1/repair2 — synthetic comparison mechanics

2026-10-25. Packet `build/tmp/c3av-thin-comparator/repair2/round-note.md`;
initial and repair1 code/fixtures/results are preserved with hash manifests.
Three attempts on this question; six attempts total for the bounded offline pass.

## Accepted scope

`tools/harness/compare_submission_slice.py` records supplied JSONL bytes
create-only and compares one explicitly **synthetic** scene-fan slice. Two
distinct draws exercise ordering. Header coverage counts and typed per-draw
request/resource checkpoints must be complete in this declared schema. Missing,
failed, unknown or malformed required fields reject or return indeterminate.
No coordinate fit, tolerance, call sorting/merging or geometry conversion occurs.
Only object-key formatting is canonicalized; typed values and list order matter.

Raw bytes/hash and parsed raw records survive independently of the comparison
view. Specular must be the exact string `unknown`; that explicitly scoped
unknown field alone is excluded. Known/malformed specular values are rejected,
not normalized away. This exclusion is a **synthetic contract**, not proof
native fog/specular behavior is irrelevant.

**Narrowing:** these are per-draw supplied request/resource checkpoints, not a
resource-lifecycle or state-event-history recorder. Successful request returns
do not establish effective device state. This does not decode native call ABI,
capture pointers, attribute original objects, forward a method, or prove capture
compatibility/completeness/non-interference. No production logger was added.

## Facilitator check

Read all code/tests/fixtures and the final schema README. Independently create
single-field variants without importing worker test/expectation generators:
geometry, blend, texture request, resource generation, content version, viewport,
clear, numeric JSON type and valid resequenced draw payload order all return
`complete_different` (**9** controls). Six malformed checkpoint/resource/space
cases reject; dropped/decode-failed evidence is indeterminate even against
itself; known specular is rejected (**9** fail-closed controls in total).
Object-key-only formatting compares equal while raw hashes/records survive.
Baseline CLI and **12/12 Python tests** pass.

The initial eight green tests missed four identical-incomplete false equalities:
empty state, checkpoint after the last draw, empty resource and unknown space.
Repair1 closes those but still discarded known specular. Repair2 restricts the
exclusion domain. Both failed packets remain preserved, not silently relabeled
as accepted. All repair2 preservation-manifest hashes independently match.

Own source mutation sets **only normalized vertex X to0** before equality.
The permanent focused geometry discriminator fails with `complete_equal`, not
a loader/syntax error; other fields were not bundled into the mutation.
Exact restored comparator SHA256
`4c51dc13fc0f3025b2d05c0e80f6532b14dcc02e1c5dc6ac889bef669d25a7d7`;
all12 tests pass again. Logs/checkers under `build/tmp/c3-offline-pivot-startup/`:
`av-isolated-x-bypass.log`, `av-restored-tests.log`, `av-final-check.log`,
`check-av-repair1.py`. The earlier repair1 false-equal result is retained in
`build/tmp/c3av-thin-comparator/repair2/preserved/packet/av-repair1-results.json`.

Reproduce from repository root, ordinary Python standard library:

```
python -B -m unittest tools.harness.tests.test_compare_submission_slice -v
python -B tools/harness/compare_submission_slice.py tools/harness/fixtures/submission-slice/baseline-a.jsonl tools/harness/fixtures/submission-slice/baseline-b.jsonl
```

Fresh wrapper build and **44/44 CTest** pass. Fresh final independent renders
and fixed histogram scorer reproduce repair4 PNG `9a5baba7…` and
**14934330/235008000 = 0.06354817708333334 FAIL**; C1zoom1/elev_down remain
**285083/285700 over17721990 PASS**. Full-frame floor inclusion, references,
footprint and gates unchanged. No production rendering behavior changed, no
gate moved, and no native observation/start/attach/install/game write occurred.

## Next permission boundary

[Frame map](../RENDER_FRAME_MAP.md) and
[conditional boundary](phase-c3au-comparison-boundary.md) are checked. The
[capture proposal](../RENDER_CAPTURE_PROPOSAL.md) names the next small offline
preflight, one separately approved native route observation and a distinct
matched non-interference experiment. Runtime identity/ABI, source attribution,
effective history, coverage and safe observation remain concrete blockers.
No full-frame capture or further renderer work follows automatically.
