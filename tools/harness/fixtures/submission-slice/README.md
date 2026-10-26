# Synthetic submission-slice fixtures

These JSONL files are synthetic test data, not stock captures. They exercise one
pre-wrapper triangle-fan record per ordered draw.

## Narrow accepted shape

Each slice has explicit capture/boundary identity, coverage counters, ordered draws,
and one per-draw request checkpoint whose sequence matches that draw. The checkpoint
requires typed `blend` (boolean) and `texture` (integer) values. The comparator checks
fan vertex/primitive counts, coordinate space, geometry, resource identity/generation/
content version, viewport and clear shape. It requires the vertex `specular` field to be
exactly the string `"unknown"`; only that explicit unknown sentinel is excluded from
the comparison view. Known words and malformed values are rejected, never silently
discarded. Original JSONL bytes and parsed raw records remain intact.

Run from the repository root:

```sh
python -B tools/harness/compare_submission_slice.py tools/harness/fixtures/submission-slice/baseline-a.jsonl tools/harness/fixtures/submission-slice/baseline-b.jsonl
python -B -m unittest tools.harness.tests.test_compare_submission_slice -v
```

`complete_equal` / `complete_different` compare this supported synthetic schema after
canonicalizing object-key order only; event/list order and typed values remain
significant. Incomplete coverage returns `indeterminate`; malformed or unsupported
schema raises `SliceError` (the CLI prints an error and exits nonzero). Successful
request returns are still request history; `effective` remains unknown. This fixture
contract makes no claim about resource lifecycle, complete native state/history,
capture reachability, runtime ABI, effective device state, forwarding, or
non-interference.
