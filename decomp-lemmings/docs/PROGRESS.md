# Function coverage benchmark

Baseline recorded on **2026-10-29 UTC**, against source commit `d729b7a`.
This is the fixed starting point for measuring future recovery progress.

| Measure | Functions | Percentage |
| --- | ---: | ---: |
| Documented recovery, including partial routines | **372** | **6.0%** |
| Not yet represented in the recovery ledger | 5,814 | 94.0% |
| Eligible identified functions | **6,186** | 100% |

The unrounded baseline is `372 / 6186 = 6.013579%`.
Separately, **543 functions (8.8%)** have executable byte pins. That set includes
supporting and inspected functions, so byte pins alone do not count as recovery.
The fully recovered / partially recovered / inspected-only / unknown split has
not yet been audited; “not in the ledger” does not prove a function is unknown.

## Counting rules

1. Use the installed `Lemmings Revolution.exe` with SHA-256
   `32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
2. The REA/Ghidra inventory contains **6,322 procedures**. Exclude **130 external
   functions** and **6 internal thunks**, leaving **6,186** eligible functions.
   Compiler/runtime functions remain included. Inventory evidence:
   `ev_3c905cbda0eac7c0404434965cd835246e300d86eca76c2b4e2e352d64462052`.
3. Count unique, exact procedure-entry addresses listed in the first address
   column of [FUNCTIONS.md](FUNCTIONS.md). Count an entry once even when multiple
   source modules or ledger rows cover it. Partial recoveries count, with their
   existing evidence limitations preserved.
4. Interior block addresses and range endpoints are not separate functions.
   The baseline ledger mentions 382 distinct addresses; 10 are interior block
   addresses/endpoints, leaving 372 exact entries.
5. Metadata inspection, pseudocode, tests or byte pins alone do not add a
   function to the recovery numerator. A ledger claim must link recovered
   source and evidence under the repository's recovery rules.
6. Verification strength does not change the count. A routine compared against
   the original bytes in-process through `hooks/` counts the same as one with
   unit tests only; the difference belongs in the ledger's status column, not
   in the numerator.

Recovery switched to the hybrid in-process method described in
[the hybrid runtime notes](HYBRID_RUNTIME.md) after this baseline was recorded.
That changes the cost and the evidence form of each recovered function, not the
counting rules above, so the baseline remains comparable. Future checkpoints
should note how much of any increase came from the harness being available.

This is an unweighted measure of documented function coverage. Small helpers
and large gameplay routines each count once. It is not a measure of playability,
fully completed source recovery, or remaining effort.

## Updating the benchmark

Keep the baseline unchanged and append a dated checkpoint below after each
coverage audit. Reuse the same executable and counting rules. Record the source
commit, numerator, denominator and change from 372; explain any inventory or
classification changes rather than silently changing the denominator. Source
recovery and indexing previously recovered routines should be identified as
different reasons for an increase.

| Checkpoint (UTC) | Source commit | Documented functions | Eligible functions | Coverage | Change from baseline |
| --- | --- | ---: | ---: | ---: | ---: |
| 2026-10-29 baseline | `d729b7a` | 372 | 6,186 | 6.0% | 0 |

## Denominator audit, 2026-10-29

The baseline above is unchanged. This audit reclassifies the denominator
without recovering anything, so it is recorded separately rather than as a
coverage increase. Reproduce with
`python tools/classify_inventory.py`; the full classification is in
[function_classification.json](benchmarks/function_classification.json).

| Class | Count | Recoverable? |
| --- | ---: | --- |
| Unwind/catch funclets | 1,294 | No — not functions |
| Library, Ghidra FID-matched | 57 | No — not game code |
| Named | 19 | Yes |
| Unnamed (`FUN_`/`SUB_`) | 4,816 | Yes |
| **Target denominator** | **4,835** | |

**The 1,294 funclets are the significant finding.** Ghidra synthesises them
from the MSVC exception tables: they are compiler-generated unwind and catch
funclets belonging to a parent routine, named `Unwind@<address>`, spaced 9 to
32 bytes apart. All 1,294 lie in one contiguous region, `0x4A9130` through
`0x4AF400`, which is why they were not obvious in an address histogram of the
whole image. They are not separately recoverable and should never be
scheduled.

Against the 4,835 target denominator, the documented 372 functions are
**7.69%** rather than 6.01%.

Two cautions. Name-based CRT exclusion does **not** work on this build —
Ghidra's Function ID database matched only 57 routines, so the statically
linked Visual C++ 6.0 runtime is still inside the 4,835 and is not separable
by name. And 4,835 remains an upper bound on *work*, not a measure of it: it
counts every recoverable function whether or not the game ever executes it.
The runtime coverage trace below separates those, and supersedes this number
for planning.

## Execution coverage, 2026-10-29

Measured with the harness's `INT3` tracer: every one of the 4,835 recoverable
entries was instrumented, each recording itself the first time it executed.
Two runs, both driven through the input shim — one held at the frontend, one
entering `MM_DIG_000` and playing for 50 seconds. Traces are in
`build/coverage/`; reproduce with
`python tools/analyze_coverage.py build/coverage/frontend.txt build/coverage/level_mm_dig_000.txt`.

| Measure | Functions | Share |
| --- | ---: | ---: |
| Recoverable (upper bound) | 4,835 | 100% |
| **Executed on a playable path** | **2,024** | **41.9%** |
| Never executed | 2,811 | 58.1% |
| — reached in the frontend | 1,126 | |
| — first reached inside a level | 898 | |
| — reached only in the frontend | 0 | |

Of the 367 ledger entries that are in the recoverable set, **239 executed**.
So coverage *of the code a playable build needs* is **11.81%** (239 / 2,024),
roughly double the 6.0% headline, and **1,785 executed functions remain**.

The 128 recovered functions that never executed are not wasted — they are
reached by gadgets, skills, effects and screens this one run did not exercise —
but they are evidence that recovery effort has not been aimed at the playable
path.

A further **208 of the 1,785 sit at or above `0x498000`**, the region holding
the known CRT anchors (`operator new` `0x0049C1A8`, `free` `0x0049C13D`, `ftol`
`0x0049C5AC`, CRT entry `0x0049D518`). A standalone build supplies its own
runtime, so those should not be ported, giving a working target of roughly
**1,577**. The exact boundary is unverified and should be established before
relying on it.

**2,024 is a floor, not a final answer.** It is one level, in one difficulty
group, for 50 seconds, with no explosions — `0x0043F510` never ran. Tracing
several levels with different gadgets and skills will raise it. Saturating that
curve is worth doing before committing to a schedule.

[Machine-readable baseline](benchmarks/function_coverage_baseline.json) retains
the counts, reference identity and inventory evidence ID for future comparisons.
