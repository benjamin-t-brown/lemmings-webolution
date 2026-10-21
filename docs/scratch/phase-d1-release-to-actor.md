# D1i: why a released lemming only enters the actor list on a ~1.02 s cycle

Queue id `d1i-release-to-actor`. Round 1 of 3 on this question, following D1a-D1h.
Offline only — no live game process was started, no debugger, no `src/**` change.

## Short answer

**There is no gate in the executable.** The ~1.02 s cycle the brief asked about does not
exist in the original game. It is produced entirely by this project's own
`tools/harness/measure_d1_release_trace.py`'s `capture()` loop, which polls the `RELEASED`
counter (`0x004CBA78`) every pass but only re-scans the actor-list pointer vector
(`0x004CBB18`) when `t - last_enum > 1.0` (the literal source line, read directly, not
paraphrased). Because the loop's passes land on a fixed `1/hz = 0.02 s` grid and the
threshold (`1.0`) is an exact multiple of that grid, the next re-scan after any 1.0 s gap
always lands `0.02 s` later than the gap itself allows — the observed "~1.02 s" period, with
zero exe-side mechanism involved.

## The exe read: release and actor-list insert are the same synchronous call

Full disassembly with bytes for all four addresses below is in
`build/tmp/d1i-release-to-actor/exe_reads.txt` (`read_steppers.py --at`, re-read
independently by the facilitator and byte-identical — see "Facilitator check").

1. **`0x00425A95`** (the hatch step, D1d's known dispatch target). The release countdown
   rolls past `0.0` (`fcomp [0x4B0338]`, confirmed `0.0`), the remaining-count field at
   `hatch+0x98` is decremented, then the Lemming constructor is called directly:
   `00425c30  e8 1c 82 00 00  call 0x0042DE51`. Nothing between the decrement and this call
   reads a clock or an animation frame.
2. **`0x0042DE51`** (Lemming constructor). After sub-object construction, it calls
   `0x004230A2` on the new lemming (`0042dea5  e8 f8 51 ff ff  call 0x0042DE51`'s target),
   and only several hundred bytes of straight-line code *later in the same function* does it
   increment the `RELEASED` per-type counter at `0x004CBA78`:
   ```
   0042dff1  66 8b 04 55 78 ba 4c 00   mov ax, word ptr [edx*2 + 0x4cba78]
   0042dff9  66 05 01 00               add ax, 1
   0042e000  66 89 04 4d 78 ba 4c 00   mov word ptr [ecx*2 + 0x4cba78], ax
   ```
3. **`0x004230A2`** (called from the ctor at `0042dea5`, i.e. *before* the counter
   increment above): near its end, `ecx` is set to `0x004CBB18` — the actor list — and
   `0x004233A0` is called with the lemming pointer:
   ```
   00423146  b9 18 bb 4c 00   mov ecx, 0x4cbb18
   0042314b  e8 50 02 00 00   call 0x4233a0
   ```
4. **`0x004233A0`** is a generic growable-array push_back: grow-if-full, then
   unconditionally store the pointer at `array[count]` and `count += 1`.

**The actor-list publish (step 3/4) executes strictly before the `RELEASED` increment
(step 2's second half), inside one synchronous constructor call with no clock read and no
animation-frame check anywhere on the path.** The brief's own candidate (a 17-frame x
0.06 s hatch-drop animation gating the insert) is withdrawn — ruled out by this read, not
argued away.

## The offline model

`build/tmp/d1i-release-to-actor/model.py` replays `capture()`'s own `last_enum`/`>1.0` gate
logic against the *real* per-pass timestamps already recorded in each capture's own
`live_raw.jsonl` (not an idealized grid, except for the handful of passes before any
lemming exists, where nothing was sampled). It reproduces all twenty saved
`first_seen_seconds` values (ten in D1a, ten in D1g) to floating-point exactness — see
"Facilitator check" for an independently-written reproduction plus a discriminating
control.

## Does `released_count` mean the counter or the actor-list count?

**The counter.** `PLAN.md`'s D1 gate says "the released count and order match," not
"actor-list count." D1a's own `diff_trace.py` names its comparison field
`"released_count"` where it compares the sim's `spawned` counter against the live CSV's
`released` column. The exe read above shows the two are the *same quantity* in the
original at any instant — the actor-list insert and the counter increment are a few
instructions apart in one call — so "release counter" and "actor-list size" never actually
diverge in the game. They diverge only in this project's own measurement: the live CSV's
`released` column is built from the throttled `first_seen`, not from a direct, unthrottled
read of the counter.

## Implication for D1g's spawn-tick-drift residual (new, not yet actioned)

This is the significant follow-on the facilitator found while verifying this round, not a
claim from the D1i worker: `measure_d1_release_trace.py::build_table()` computes the
per-tick `released` column fed into `diff_trace.py`'s comparison as
`released_by_t = sum(1 for p in order if first_seen[p] <= t_t + 1e-9)` — i.e. every
per-tick "released count" and "existence" check in D1e/D1g's own gate recheck was built
from the *throttled* `first_seen`, which this round shows lags a lemming's true,
synchronous appearance by up to ~1.02 s of harness artifact. D1g's characterization of its
161 residual ticks as the sim "drifting by 0-4 ticks from discretizing the original's
continuous accumulator" has not been re-examined against this finding; some or all of that
residual may be this harness throttle rather than a sim timing defect. This is an open
question for a new round (not this one — D1i's brief was read-only, offline, no
`tools/harness/**` change), queued next as D1j.

## What this does and does not settle

- **Settled, with addresses and bytes**: there is no exe-side gate between release and
  actor-list insert; the ~1.02 s period is this project's own harness throttle, not a game
  mechanism. No live read is needed to close this question.
- **Not re-litigated**: D1g/D1h's separate, already-parked single-tick `facing`-label
  residual (Phase E) is untouched by this finding.
- **Opened, not closed**: whether D1g's 161-tick spawn-drift residual survives once the
  harness's own throttle bias is removed from the comparison (D1j).

## Facilitator check

2026-10-21: independently re-read all four cited addresses directly against the installed
exe (`read_steppers.py --at`, not re-running the worker's own dump) — `0x0042dff1`
(`RELEASED` increment, 8 bytes) and `0x00423146` (actor-list `ecx` load + call) both match
the worker's bytes exactly. Confirmed the call-site ordering from the raw addresses alone
(`0x0042dea5 < 0x0042dff1`: the actor-list-insert call precedes the counter increment in
instruction order, inside the same function). Independently grepped
`measure_d1_release_trace.py` and confirmed the literal source line
`if t - last_enum > 1.0:` at its reported location, exactly as quoted. Independently
confirmed `diff_trace.py`'s `"field": "released_count"` citation and `PLAN.md`'s literal D1
gate text ("released count and order match," no "actor-list count" wording).

Wrote a fresh, independent checker (not importing or copying the worker's `model.py`) that
re-derives `first_seen_seconds` from each capture's own `live_trace.meta.json` +
`live_raw.jsonl` using the same gate logic, expressed from scratch: reproduces all twenty
values to floating-point exactness in both captures. Added a mutate-one-alone discriminating
control absent from the worker's own packet: a naive "no throttle" model (`first_seen ==
release_t`) against the same data — it **fails** 9 of 10 points in D1a and 9 of 10 in D1g
(diffs up to +1.00 s), confirming the throttle model is actually required to fit the data,
not merely one plausible story among several.

Confirmed via PowerShell `Get-Process`/`Get-ScheduledTask` that no Lemmings Revolution
process or `LemWeb-*` scheduled task is running. `git status --short` shows no `src/**`,
`tools/harness/**`, or other `docs/**` file touched by the worker — only this task's
permitted `docs/tasks.json` fields and this facilitator's own landing edits. No rebuild was
needed (no source changed this round).
