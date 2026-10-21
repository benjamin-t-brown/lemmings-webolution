# A6f: fresh-pause acquisition failed before walking; no new gate result

2026-10-20. OpenCode `round-agent`, dispatch
`ses_ea4570477ffec1vBM0X4zNuNW4`, using the configured targeted pin from
[ROLES](../ROLES.md). Configuration resolved correctly; Task did not independently
expose the actual execution model. Original brief
`build/tmp/a6f-fresh-pause-gate-brief.md`; final corrected packet
`build/tmp/a6f-fresh-pause-gate/round-note-repair2.md`.

## Disposition

**Failed acquisition, not a fresh image failure or acceptance.** The queued
experiment selected a 14-second walk on `MM_CLIMB_002`. Its only authorized stock
start (owned PID 60128) reached the level-entry pause, then the scratch runner
raised `KeyError: 'address'` while selecting memory rows from a mixed config/memory
ledger. It never unpaused for the planned walk and produced no original screenshot,
fresh walker table, stable snapshot bracket, actor-box score or tube-ROI score.

The first dispatch stopped after preflight; repair1 made the single live attempt.
Repair2 was offline disposition only. Three dispatches total; no start remains
authorized, and no automatic retry is queued. [A6e](phase-a6-restated-gate-rescore.md)
still governs the measured gate: tube ROI passes, actor 2 fails clause 2 on the
A6d3 pause. This experiment establishes neither pose specificity nor an actor fix.

## Corrected evidence coverage

- `capture.json` and `enter-log.json` agree on PID and entry-pause state. The planned
  `walk_seconds: 14` is metadata, **not measured walking time**. No
  `walk_started_ns` or `capture_complete` is recorded.
- The sole raw memory event at level slot `0x004E35D4` covers **280 restore bytes**,
  not the initial slot write. `capture_fresh_pause.py` patches `entry.write_mem`;
  the finally restore at `enter_fast_a6f.py:106` uses it. Imported
  `goto_level.select_level` resolves its initial `write_mem` call in its own module
  globals (`tools/harness/goto_level.py:275`) and bypasses that patch.
- The restore event's own preimage, write and immediate readback are identical.
  The initial preimage/write/readback was **not retained**. Repair1's interpretation
  of this as an initial event, and its inference that the slot was already selected,
  are withdrawn by repair2. No full initial-to-restored raw audit is claimed.
- Config write/readback/restore bytes agree with the original. Independent check
  finds the attempted windowed-config write was itself byte-identical to the
  preimage, not a changed config in this run. Current executable, save and config
  hashes match the recorded before/after set.
- The worker corrected the ledger filter after the failed run, but the current
  wrapper still cannot collect the two memory events its assertion expects.
  The helper-only mock tests did not validate the integrated imported write path.
  No claim that this acquisition runner is repaired or safe for live reuse.

## Production disposition

The optional `--walkers` loader was never used on fresh state. Its source is
preserved as scratch `main-cli-candidate.cpp`; it was removed rather than shipped
with a failed experiment. `src/app/main.cpp` is byte-identical to its pre-A6f/HEAD
baseline (SHA-256 `0b3f9152968622efa06ce0017be7ebde38bedc3c90174f175cbc05cef6863a1a`).
No production behavior change or original-game number is accepted.

## Facilitator check

Independent checker, not the worker's mock suite:

```text
python build/tmp/a6f-fresh-pause-gate/facilitator_check.py
```

It reads the retained JSON, reproduces the heterogeneous-ledger `KeyError` offline,
checks missing capture milestones, verifies PID/entry-pause agreement, decodes the
280-byte restore event, checks the separate Python module namespaces, verifies raw
config identity/restoration and current install hashes, and compares main.cpp bytes
directly to `git show HEAD:src/app/main.cpp`. Output is retained in
`facilitator-check.json`. Raw capture SHA-256:
`5c6d7497864f781a5efdc882f0ffd0be08a4c513a56e9f0b183052ac0aa0011f`;
entry-log SHA-256:
`ada95bd1c6c4fe43e1c36d5b761776fc6a8523b81473ab650e85ad65d28ebc57`.

Fresh facilitator build succeeded (no work required) and **24/24 CTest passed**
through `scripts/Invoke-Ucrt64.ps1`. Separate process/task observation found zero
Lemmings/webolution processes and zero `LemWeb-*` tasks. No extra stock start,
native-instruction claim, raster mutation or fresh pixel result was made. Existing
retractions and A6d3/A6e failure remain intact. No commit authorization assumed.

## Concrete remaining blockers

1. Offline integration regression must intercept **both** actual import namespaces,
   distinguish config from memory events, test exception cleanup, and validate the
   full initial/restore chain before any further live attempt. This tooling serves
   the still-unmeasured fresh A6 gate, not a parked pixel fit.
2. The single-start budget is exhausted. A new live attempt requires explicit renewed
   authorization and a justified continuation after this three-dispatch round.
3. Until a matched reference/table/camera capture exists, fresh-pause pass/fail and
   the reason actor 2 worsens on A6d3 remain unknown. Do not alter the gate or
   substitute a fitted offset/phase.

## Repair3: renewed attempt also failed acquisition, before game creation

User-authorized continuation, dispatched by a new OpenCode facilitator as requested;
Task `ses_ea3c26b2affexWMJiy7qMNWeZ2` completed. The configured targeted model/high
resolved via `opencode debug agent round-agent`; actual execution model was not
independently exposed. Brief `build/tmp/a6f-fresh-pause-gate-repair3.md`; packet
`build/tmp/a6f-fresh-pause-gate/repair3/round-note.md`. Its UTC exit marker is
`2026-10-21T02:17:45.0759940Z` (the authorization/project date is 2026-10-20).

This continuation explicitly waived the stricter initial entry-write audit above,
and reused A6d3 with output-path, 14-second-walk and finally-cleanup edits. It did
**not** fail that old audit. The copied script reads
`repair3/dgVoodoo.windowed.conf` before its fallback; the file was not copied.
The scheduled capture exited 1 with no PID, capture, images, table or scores.
The missing-file diagnosis is independently reproduced from the pure file-read
expression and source execution order, **not** a retained full traceback:
PowerShell's error wrapper retained only its first `Traceback` line.

No additional stock process was reached on this attempt. No retry was made:
repair3 says to stop after any failed start/capture invocation. The prior consumed
stock start remains one; zero further automatic attempts are authorized. This is
another failed acquisition, neither a new image failure nor a gate pass.

### Facilitator check — repair3

Independent offline checker (does not import/run the capture or write the install):

```text
python build/tmp/a6f-fresh-pause-gate/repair3/facilitator_check.py
```

Output `repair3/facilitator-check.json` retains the full script diff. It confirms
the identical A6d3 entry helper, acquisition core unchanged except 14-second sleep,
missing required config before fallback/`Popen`, exit 1, no captured state/images,
prediction file preceding capture output, main.cpp byte-identical to HEAD, and
current executable/save/config hashes equal to the packet's before/after values.
Capture JSON SHA-256:
`72709e9c87b9bf3723cf05f8e03ff37d94e3a644ba0087dc6e28f9a39b707470`.

**Narrowing:** `writes: []` is not proof no write occurred: the new finalizer
unconditionally rewrites the saved config preimage even on this prelaunch error,
without adding that event. Exact current hash equality supports restored contents;
there is no complete raw config-write ledger. Missing PID alone is not general
proof that a subprocess never existed; the ordered prelaunch failure supports
this disposition. No original instruction or new pixel claim was made, so no
native-byte or raster-mutation acceptance is implied.

Independent full build succeeded (no work required), **24/24 CTest passed**.
Separate process/task check: zero game/webolution processes, zero `LemWeb-*` tasks,
Explorer present in session 1. No production edit, fitted correction, commit or
additional game start. Prior failed measurements/retractions remain intact.

### Current blockers (supersede the pre-repair3 authorization/tooling requirement)

1. Fresh reference/table/camera and actor-box scores still do not exist; A6e's
   actor-2 failure remains. No evidence tests the pose-specificity hypothesis.
2. Before proposing another live round, split out a bounded **offline** prerequisite
   check: account for the copied capture's data/config dependencies and preserve
   full Python stderr on failure. Do not resurrect the waived repair1/2 ledger.
3. Any further live invocation requires renewed user authorization and round-cap
    justification. Stop-on-failure applies even though repair3 reached no new game
    process. The queue is blocked, with no live owner.

## Repair4 superseding status

The user subsequently carried repair3's unused start over to repair4. That one
start produced a fresh reference/table and passing ROI/component arithmetic;
[the repair4 facilitator check](phase-a6-fresh-pause-pixel-result.md) carries the
result and independent reproduction. Acquisition qualification remains incomplete
(no second stability snapshot, truncated actor list); A6 is not accepted. The
carried-over start is now consumed, no retry remains, and no live owner is held.
Historical failure observations and the raw-slot-audit waiver above are preserved.
