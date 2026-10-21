# A6f repair4: fresh pixels pass the restated A6 gate; A6 accepted

2026-10-20 project authorization. OpenCode `round-agent` Task
`ses_ea3976b87ffeEI5AKx1j0611Xa`, plus one same-session offline packet correction.
`opencode debug agent round-agent` resolved the targeted OpenAI Luna/high pin in
[ROLES](../ROLES.md); Task does not independently expose the actual execution model.
Briefs: `build/tmp/a6f-fresh-pause-gate-repair4.md` and
`build/tmp/a6f-fresh-pause-gate-repair4-review1.md`. Packet:
`build/tmp/a6f-fresh-pause-gate/repair4/round-note.md`.

## Disposition

**MEASURED offline:** both restated A6 pixel clauses pass arithmetically on the
retained three-row injection. **A6 acceptance is not established:** acquisition
qualification is incomplete. Do not replace the old A6d3/A6e actor-2 failure with
an unconditional milestone pass or infer its cause from this sample.

One carried-over authorized stock start reached `Popen`, PID 43272. Capture and
task exited 0; the owned game was terminated in the finalizer. No live retry was
made. The earlier A6f start plus repair4 make two consumed stock starts in total;
repair3 reached none. Remaining authorized starts: zero.

## Reproduced pixel results

Reference, actors-on and tube-only are 640x480 PNGs in the packet directory.
Fixed ROI is unchanged: `[100,80,280,410)`.

- ROI actors-on and tube-only error are both **708,159 / 45,441,000 =
  0.015584142074338152**, passing <= 0.02.
- The unchanged three-largest-difference-component box method gives:

| Component rank | Half-open box | Tube-only error | Actors-on error |
| --- | --- | --- | --- |
| 0 | `[460,276,473,294)` | 19,500 / 179,010 | 12,673 / 179,010 |
| 1 | `[423,289,435,307)` | 17,749 / 165,240 | 9,913 / 165,240 |
| 2 | `[399,286,411,304)` | 17,317 / 165,240 | 9,063 / 165,240 |

Every listed component strictly improves. Component rank is **not** a measured
join to actor-list row index. The initial worker packet mislabeled these indices;
the offline correction preserves all numbers and renames them `component_index`.

Full-frame diagnostic remains above 0.02: actors-on **49,870,416 / 235,008,000 =
0.21220731209150326**, tube-only **49,893,333 / 235,008,000 =
0.21230482792075164**. This is reported, not gated, in restated A6.

Three selected rows in `capture.json` have vtable `0x4B10E8`, Y = 32, with X/Z
`48.34524917602539 / 0.25`, `62.77764129638672 / 0.25`, and
`50.790000915527344 / 0.75`; facings -1, -1, +1. They are copied exactly into
`injected-plus8.json`. No non-integer-Y slope sample was retained.

## Qualification limits (not waived by repair4)

1. `same_pause: true` is a literal in the inherited capture script. One camera and
   actor snapshot plus paused-state checks were retained, not the originally
   requested second stability snapshot across image/table/camera acquisition.
2. The script checks at least three actors but persists only `actors[:3]`. No
   full-list count or exhaustive identity is retained, so equality between the
   complete original actor list and the injected three is unestablished.
3. Component ranks are image-difference regions, not native actor rectangles or a
   retained mapping from a region to an actor pointer.

The loaded-path entry log and capture PID agree; entry path is
`Compiled\MM_CLIMB_002.lvl`, paused state 1, yaw 32, zoom/elevation 0. Camera raw
bytes equal the retained A6d3 camera block (SHA-256 `7e6e1a8a...`). These checks
support a bounded captured interval, not the missing repeated stability proof.
The script requests a 14-second sleep; separately measured walking duration is
not retained. No new native-byte finding is claimed.

The stricter raw initial/restore level-slot audit was explicitly waived by repair3
and remains waived. Entry restoration is a boolean helper report, not a raw byte
audit; this is distinct from the unwaived acquisition limitations above.

## Production change

`src/app/main.cpp` adds an optional `--walkers` fixed-schema input for the recorded
`{"walkers":[{"plus8":[x,y,z],"facing":n},...]}` format. It replaces the injected
table, including an empty table, and rejects the tested NaN input. Default
behavior is unchanged. This is input plumbing, not a World adapter, temporal
integration, raster correction, general JSON-parser claim or A6 completion.

Source SHA-256 after independent mutation restoration:
`462fdb6449bd61faf03fa9ecbeb9f06dcbd4237cfbe636f104ca439c26f673f2`.
Renders reproduce these hashes:

- Default: `c4e6cf477e7f803d92329e5b223fe296b014801abb0552132d65133fb1a09ebf`.
- Fresh table: `b0bc40a5cc44ac8d29c399a171e2dfbf9ed03ab587374e7b97050d7822324056`.
- Empty table/tube-only: `66a8ea3b44df2216b64debf9d8f417d2ed923841fc440e99c028b013010d3651`.

## Facilitator check

Independent checker (Pillow histogram sums and separate connected-component
enumeration, no worker scorer imports or live execution):

```text
python build/tmp/a6f-fresh-pause-gate/repair4/facilitator_check.py
```

It reproduces every ROI/component/full-frame integer above; verifies injected rows,
PID/entry path/pause, camera bytes, dependency hashes, script identity against
repair3, unchanged A6d3 entry helper, sidecar equality, self-test logs/exit 3 and
prediction ordering. Own default/fresh/empty renders reproduce the worker PNG
hashes. Retained output is `repair4/facilitator-check.json`.

An isolated removal of **only** `options.injectedWalkers = std::move(loaded)`
changes the fresh-input shot to the old default hash: the fresh-image expectation
fails. Restoring that assignment restores the exact source hash above; recompiling
and rerendering restores the expected fresh hash. No fitted pixel correction was
introduced. Own fresh full build and **24/24 CTest pass** after restoration.
The malformed-input invocation reports the expected format error.

Current stock EXE/save/config hashes independently match the recorded pre/post
values. The copied windowed sidecar equals both its A6d3 source and current config;
the config swap is content-identical here. No complete write ledger is claimed.
Separate process/task checks find zero game/webolution processes and zero
`LemWeb-*` tasks; Explorer remains in session 1. Worker and facilitator hold no
live owner. No commit was authorized or made; prior uncommitted work is preserved.

## Blocker / next decision

The queue remains blocked on acquisition qualification and the exhausted one-start
allowance. Any further live acquisition needs explicit authorization and a bounded
brief permitting the missing full-list/stability capture, rather than another
unchanged-script rerun. No automatic follow-up, actor pixel fit or revival of the
waived raw-slot tooling. The new passing arithmetic and the old failed fixture
both remain retained.

## Acceptance (2026-10-20, Claude Code facilitator, user decision)

The user accepted this frame as passing the A6 exit gate. The three qualification limits
above apply equally to the A6d3 capture, which came from the same script and was accepted as
evidence. Repair 4 is held to that same standard. The blocker above is therefore resolved by
decision, not by new evidence, and no further acquisition is queued.

All three walkers in this sample are flat (Y = 32). The A6d3 slope walker's clause-2 failure
is retained, unexplained, and parked in PLAN Phase E. Every actor-2 lead was already at its
round cap. A7 is next.
