# B2n — Option A offline recovery decision (2026-10-27)

Directive `offline-recovery-review-a-2026-10-27`, SHA256
`98cef7b883e4865857b93758cce837a603310c5488c66bad78989ffe08514809`.
This is a documentation-only review, **not capture/input acceptance or repair**.
Owner `ses_e80f12432ffebdYi3RdAhbYN8v`, parent `ses_e80f36524ffe7KzSDCj9yVcKCp`.
Packet prefix `build/tmp/offline-recovery-review-a-2026-10-27/`.

## Decision: reuse established seams; repair safety, not the capture engine

| Component | Disposition and source/evidence |
| --- | --- |
| B1b reference and foreground/client crop | **Reuse at its qualified scope.** Saved `build/tmp/b1b-reference-screengrab/capture.py:169-205` brackets the capture, crops actual client geometry and requires 640x480/screen3/highlight0. Its `metadata.json:6-46` records all three 3/0 brackets, 640x480 crop, desktop2560x1440, unchanged before/after hashes. Saved image independently viewed: English, first menu item highlighted. [Landed B1b](phase-b1-reference-screengrab.md) remains valid; later geometry failure does not invalidate it. |
| Existing capture/foreground functions | **Reuse as components, not a currently safe launch chain.** `tools/harness/screengrab.py:33-51` is composited desktop BitBlt, not PrintWindow; it requires unobscured game pixels. `acquire_game_foreground.py:65-112,115-131` checks stock path/window, verifies foreground PID, masks activation input and restores prior shim state. Its use would require new live authority; none was exercised here. The screengrab docstring's reference to `goto_level.focus` is stale: that older queue-attachment path stays banned. No source repair made. |
| B2h scripted navigation | **Reuse the observed sequence and bounded driver seams.** Saved `build/tmp/b2h-live-esc-sweep/capture_b2_sequence.edited.copy.py:295-323,332-346` acquires foreground, drives highlight0, aims and writes shim button0 down/up. `tools/harness/lrinput.py:174-208` writes synthetic motion/buttons/keys to shared memory; `goto_level.py:399-463` refuses blind/nonzero-highlight RETURN and uses two RETURNs then map clicks. B2h `report.json:54-115` and `raw/04-map-node-click.json` retain loaded `Compiled\MM_CLIMB_002.lvl`, counter5, empty frontend head, then play0/pause1. This was an observed successful scripted entry, not a hypothetical route. [B2h](phase-b2h-live-esc-sweep.md) still leaves level-to-map/non-head links and whole B2 acceptance open. |
| B2m executed launcher/helper | **Reject as an executable route; preserve as failure evidence.** Exact frozen source, raw commands and receipts below show safety failures independent of the presentation failure. No rejected launcher was executed or imported, even TestOnly. Its self-recorded hashes are useful provenance, not an approved-source comparison or safety gate. |
| B3k input facts and current app | **Retain conditional READ/DERIVED, not physical-binding/entry acceptance.** [B3k](phase-b3k-native-binding-partial.md) establishes constructed C's enumerated mouse/custom format and separately runtime G→shared S, not C→G storage. Current-state object+0 is queried, not the distinct newpress+10 flag. ANYINSTANCE is first available button; conventional physical-left interpretation remains conditional. Installed enabled shim route can supply synthetic IPC byte+1C→sample+12; no runtime physical sample is claimed. SOURCE map rendering is not actual app map-to-level entry. |

**Reconciliation:** B2h's observed shim-driven click proves entry happened on that
scripted run, including the loaded level path. It does not identify the constructor
object used by the shared map context, certify physical-left/SDL edge semantics,
qualify a future desktop capture, or pass all B2/B3 clauses. Conversely B3k's missing
identity proof does **not** erase B2h or demand universal input-architecture proof
before recommending reuse of its scripted route. The minimum remaining *binding*
question is C→runtime G/shared S identity on the actual map path, with the existing
API-success/route conditions retained. No new executable body or binary was read
to close it in this review.

## Established defects versus hypotheses

All following paths start at
`build/tmp/native-capture-first-entry-2026-10-27/` unless otherwise stated.

1. **Stale executed source/TestOnly ineffective, established.**
   `capture/repair1/frozen/worker.py:40-65` has no test-only argument branch and
   proceeds from permissive tests to Popen. Its SHA256 is
   `7b8942f52f7f6f97287e85cf8162f0e140ac3cfac89fd99a1c48d6070baf8d1f`;
   helper `4f71d620dee9d0e8becbe5be5e047325dd64b4e818f51898970141cb392d013b`.
   Both actual receipts' frozen manifests match those saved bytes. Later **working**
   `capture/repair1/worker.py:47-61` adds the exact exception assertion and
   `--test-only` early return. Those edits never became the frozen executed worker.
   `capture-test-switch-patches.json` records launcher switch plumbing before the
   second command. Actual saved tool message `msg_17ee7cea8001EtUTvhTA9WSK0x`,
   tool `call_UxXeTSRkMeWlDoPkovGA4L3k`, contains **-TestOnly** and bash
   `Copy-Item: command not found`/hash-command errors. Thus failed refreeze plus
   unguarded old worker, **not omitted flag**, explains why TestOnly could launch.
2. **False forced-exception preflight, established.** Frozen helper
   `capture/repair1/frozen/native_entry_capture.py:108-119` checks geometry before
   `elif force_error`. Frozen worker `:47-55` only asserts nonempty error and prints
   PASS. The saved forced-error receipt has client404x261 and geometry refusal,
   not the injected exception; `run/worker-result.json:44-84` nevertheless reports
   PASS. Later stronger source is not retroactive proof. Prerequisite was unmet.
3. **Missing single-use enforcement, established for these scripts.** Frozen worker
   `:58-65` records its own manifest then Popen; complete saved launcher
   `capture/repair1/run-session1.ps1` has no durable consumed authorization check.
   Separate tool output packets establish PID40140 at19:47:23Z and PID16480
   at19:48:14Z versus one authorized/predebited start, not merely duplicate source
   strings. Original budget violation remains one unauthorized actual start.
4. **Overwritten evidence, established.** Launcher `:13` removes prior result;
   frozen worker reuses constant OUT and `stock-initial` names (`:8,76,96-98`),
   helper uses replaceable receipt/PNG paths (`:95-119`). Both raw receipts name
   the same screenshot paths. First textual receipt was recovered from saved
   tool history; first images remain lost. Retained PID16480 image is unrelated,
   occluding desktop content, not a qualified native game frame. No substitutions.

Saved text source `capture/repair1/run-session1.ps1` is the later surviving launcher,
not a fully hash-bound revision history of every PowerShell execution. The
Python frozen identity and terminal patch/command chronology support the narrower
cause above; do not claim an immutable entire launch chain.

**Hypotheses only:** concurrent user desktop use plausibly explains occlusion/focus
loss, not two starts, stale source or overwrites. Fake-fullscreen, DPI handling and
the reason a client was reported2560x1440 are not diagnosed causes. An owned HWND,
correct crop arithmetic or geometry metadata alone does not establish visible game
pixels. Exit1 is not proof of either normal exit or a crash. No new host/process or
install-hash measurement was made; historical cleanup remains historical evidence.

## Concrete next-pass recommendation — approval required

Recommend **one narrow offline safety-boundary repair/preflight**, not a new capture
engine, observer, native retry or application integration campaign. Reuse the
established BitBlt/foreground/client-crop and scripted navigation components above;
repair only the launch/evidence boundary in a separately owned candidate. Proposed
budget: **90min total, one worker attempt (≤60min), ≤30min independent verification,
zero starts/attaches/foreground/desktop exercises**. Stop on failure, no automatic
repair/retry or budget carry-over. This proposal requires new explicit user approval.

Acceptance/falsifier for that *future* pass, with all external effects mocked and
actual main/launcher boundaries tested rather than a substitute skeleton:

- Bind the approved source/arguments/dependencies by hashes **and actually check
  equality before any launch path**, including stale frozen worker and failed-copy
  cases. A manifest that merely records whatever ran is insufficient.
- TestOnly must fail closed through launcher→worker, with **zero Popen and zero
  desktop/target effects**, including unknown/unsupported flags and stale worker.
- Atomically consume a durable one-use authorization before Popen; second invocation,
  concurrent use or crash/restart must refuse. Never refund a consumed token merely
  because capture/entry failed. Do not test this against the stock process.
- Exclusive unique attempt directories and append-only receipts; retain first bytes
  after second attempts/failures. Bind source/nonce/result identities, include errors
  and actual owned termination outcome; refuse existing evidence paths.
- Separate wrong-geometry from injected-exception controls; assert the **exact branch**
  and durable diagnostic artifacts, not merely a truthy error. Individually bypassing
  source identity, TestOnly, single-use or non-overwrite guards must fail its own test.

**Falsifier:** any stale/mismatched/second-use/TestOnly path reaches launch or desktop
effects, wrong-geometry earns exception-test PASS, or later execution can replace
earlier evidence. Any one rejects the repair. No output from this review satisfies
these future checks. Only after a separate acceptance/decision could a different
live brief be proposed: explicit start cap, no retry, exclusive user-agreed desktop
window, fresh ownership and qualified foreground/client/state evidence. No such
start or binding research is authorized here or queued automatically.

## Status corrections and facilitator check

D1n code `1d68ee0`/landing `24ec965` are HEAD ancestors and the opt-in fields remain
in World source. [D1n](phase-d1-continuous-stepper-clock.md) explicitly says landed,
not accepted. [D1o](phase-d1-continuous-live-recheck.md), ancestor `a2bb9e7`, still
fails the fixed gate (retained657/305 off/on) and parks D1. LESSONS queued/“until it
lands” wording corrected; Phase D/D3/digger-clock parks unchanged. PLAN's consumed
native-directive link, not-yet-activated review, stale B2 failure summary and old
unused-start/current-authority wording corrected without changing historical records.

Worker `ses_e80ecea29ffeSgcMCRsauYejLz` returned initial review then one bounded
source-citation repair, both succeeded/idle independently confirmed. Two worker
attempts, **one review round**, no more attempts. Candidate notes remain immutable.
Facilitator narrowed two errors: initial PLAN29-31 never contained the alleged
D1n “until it lands”; repair.md:19 incorrectly located later TestOnly/assertion
lines in the frozen worker. Direct reads show those lines only in the **working**
worker; executed frozen worker has neither. They are not promoted as written.

Independent commands (from repo; safe offline only):

```
python build/tmp/offline-recovery-review-a-2026-10-27/verify-saved.py
python build/tmp/offline-recovery-review-a-2026-10-27/verify-integrity.py
python scripts/check_agent_framework.py --brief build/tmp/offline-recovery-review-a-2026-10-27/brief.md
python scripts/check_agent_framework.py --brief build/tmp/offline-recovery-review-a-2026-10-27/repair-brief.md
git diff --check
git merge-base --is-ancestor 1d68ee0 HEAD
git merge-base --is-ancestor 24ec965 HEAD
git merge-base --is-ancestor a2bb9e7 HEAD
```

`verify-saved.py` is a separate facilitator JSON/AST/PNG-header checker, not a rerun
of a worker/old launcher. It reproduces B1b brackets/dimensions, B2h raw loaded-path
and synthetic route distinction, decodes the two original shell-output receipts,
checks frozen source hashes and absent guards versus later working edits, false
exception PASS and reused paths. Result `independent-saved-result.json` under
packet prefix. Saved images independently viewed. No mutation is appropriate for
this review: existing evidence immutable, no behavioural implementation made.
No builds/CTest/retail app runs or new pixel scoring performed.

Integrity check preserves exact prior standing_run object/all older archive entries,
all16 gate texts, ten untouched dirty baseline files and236 saved evidence files.
Initial API requests failed from MSYS path conversion; reissued successfully with
`MSYS_NO_PATHCONV=1`. One diff display failed cp1252 output, repeated with UTF-8.
Brief lint errors were corrected before dispatch. None launched or changed targets.

**Retained, not fresh measurements:** C3 `14934333/235008000 =
0.06354818984885621` FAIL versus0.02; C1 zoom1 `285083/17721990 =
0.0160863988750699` and elev_down `285700/17721990 = 0.016121214378294987`
PASS. B2/B3 unaccepted; C2 native gate unpassed; B1 conditional. Gates/masks/
exclusions/references unchanged. Prior native breach, all historical counts and
unused frozen pools preserved exactly in queue archive; this review renews none.

**Stop:** verified review/report only. Zero stock starts, attaches, foreground/input,
desktop exercises, screenshots, native sampling, install/save/config writes,
debuggers/Frida/REA/tasks or production/harness changes. Worker terminal; no owned
live resources or restoration owed. Documentation left **uncommitted** for user
review, no push; no follow-on dispatch. Exact final clock/accounting in queue.
